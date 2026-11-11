#include "battle.h"
#include "battle_util.h"
#include "party_status.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../unity.h"
#include "../input/input.h"
#include "../ui/preferences.h"
#include "../field/object_cache.h"
#include <algorithm>

namespace ffpr::battle {
namespace {
ref::Class kParty{"Last.UI.KeyInput", "BattlePlayerInfomationController"};
ref::Field fContents{kParty, "contentList"};
ref::Field fTargets{kParty, "targetSelectContentList"};
ref::Field fRoster{kParty, "<PlayerList>k__BackingField"};
ref::Class kCommands{"Last.UI.KeyInput", "BattleCommandSelectController"};
ref::Field fActor{kCommands, "targetData"};
void* g_party = nullptr;
void* g_ordinary_party = nullptr;
// Attack and spell/item targeting can own separate party controllers. A live
// controller is not proof that its rows are the ones currently rendered.
// Keep only owners observed by their native producers, never search the scene.
std::vector<void*> g_party_owners;
void remember_party(void* owner) {
    if(!unity::alive(owner)||!kParty.is(owner))return;
    std::erase_if(g_party_owners,[=](void* p){return p==owner||!unity::alive(p);});
    g_party_owners.insert(g_party_owners.begin(),owner);
    if(g_party_owners.size()>16)g_party_owners.resize(16); // defensive native-instance bound
    g_party=owner;
}
ref::Method mInitialize{kParty, "Initalize", 0};
ref::Method mUpdate{kParty, "UpdateView", 2}, mFocus{kParty, "SetFocus", 2};
void* g_focused_player = nullptr;
using Update = void (*)(void*, void*, bool, const void*);
Update o_update;
void d_update(void* self, void* players, bool target, const void* mi) {
    o_update(self, players, target, mi);
    hooks::guarded("Battle.PartyViewOwner", [self, target] {
        if (!kParty.is(self)) return;
        remember_party(self);
        if (!target) g_ordinary_party = self;
        field::cache::register_object(kParty, self);
    });
}
Update o_focus;
void d_focus(void* self, void* player, bool focused, const void* mi) {
    o_focus(self, player, focused, mi);
    hooks::guarded("Battle.PartyFocus", [=] {
        if (!kParty.is(self)) return;
        remember_party(self); field::cache::register_object(kParty, self);
        if (focused) g_focused_player = player;
        else if (g_focused_player == player) g_focused_player = nullptr;
    });
}
using Initialize = void (*)(void*, const void*);
Initialize o_initialize;
void d_initialize(void* self, const void* mi) {
    o_initialize(self, mi);
    hooks::guarded("Battle.PartyOwner", [self] {
        if (kParty.is(self)) {remember_party(self);field::cache::register_object(kParty, self);}
    });
}
int g_review_slot = 0;
void* g_execution_player = nullptr;
std::wstring g_execution_name;
bool g_action_ongoing=false; // action context only; no enemy identity or turn schedule

std::vector<void*> rows_for(void* party)
{
    std::vector<void*> out;
    if (!in_battle()||!unity::alive(party)) return out;
    // The game swaps its ordinary and target-select panels. Read whichever it displays.
    // Controllers and their serialized views need not share a GameObject. The
    // rendered row owns visibility; a hidden controller must not veto its panel.
    for (void* list : {ref::rd<void*>(party, fTargets), ref::rd<void*>(party, fContents)}) {
        const int count = unity::list_count(list);
        if (count <= 0 || count > 9) continue;
        std::vector<void*> slots(static_cast<size_t>(count));
        bool any = false;
        for (int i = 0; i < count; ++i) {
            void* row = unity::list_at(list, i);
            void* view = displayed::field(row, "view");
            if (displayed::container_visible(view) && !displayed::text_field(view, "nameText").empty()) { slots[i] = row; any = true; }
        }
        if (any) { out = std::move(slots); break; }
    }
    return out;
}
std::vector<void*> rows() {
    if(g_party_owners.empty())remember_party(field::cache::get(kParty));
    for(auto* owner:g_party_owners)if(auto content=rows_for(owner);!content.empty())return content;
    return {};
}

std::wstring read_row(void* row, bool include_command = false)
{
    auto line = displayed::character(displayed::field(row, "view"));
    // Target selection reads the full rendered row. H/party review reads
    // status only; the queued command column is not a character condition.
    if(include_command) {
        const auto command=displayed::text_field(displayed::field(row,"view"),"commandText");
        if(!command.empty())line+=L", "+command;
    }
    // The native rotating condition text/icons can lag after a removal and can
    // be hidden by commands. All three readers use the same current ally state,
    // without borrowing an old label or requiring a transient effect animation.
    const auto extra = party_status::current_lines(
        displayed::field(row,"<TargetData>k__BackingField"), displayed::field(row,"view"));
    if (!extra.empty()) line += L", " + text::join(extra,L", ");
    if (ui::prefs::auto_detail_enabled()) {
        const auto charges = displayed::charge_lines(displayed::field(row, "view"));
        if (!charges.empty()) line += L", " + text::join(charges, L", ");
    }
    return line;
}

bool choosing_command()
{
    return menu_state::is_active(menu_state::BATTLE_COMMAND)||menu_state::is_active(menu_state::BATTLE_ITEM)||
           menu_state::is_active(menu_state::BATTLE_MAGIC)||menu_state::is_active(menu_state::BATTLE_TARGET);
}

void* command_actor()
{
    if(!choosing_command())return nullptr;
    // Read rooted native command data afresh; opening Magic/Items must not clear its owner.
    void* command = field::cache::get(kCommands);
    return unity::alive(command)?ref::rd<void*>(command,fActor):nullptr;
}

void* command_row(const std::vector<void*>& content)
{
    if(!choosing_command())return nullptr;
    void* actor=command_actor();
    if (actor) {
        const int id = character_id(actor);
        for (void* row : content) {
            void* player = displayed::field(row, "<TargetData>k__BackingField");
            void* owned = player_owned_character(player);
            if (owned && (owned == actor || character_id(owned) == id)) return row;
        }
    }
    // The command panel may be inactive while its Magic/Item child owns input.
    // Match the preserved actor by identity only; never dereference an unrooted stale pointer.
    for (void* row : content) {
        void* owned = player_owned_character(displayed::field(row, "<TargetData>k__BackingField"));
        if (owned && owned == current_actor()) return row;
    }
    return nullptr;
}

void* active_row(const std::vector<void*>& content)
{
    // ATB permits another action while this character is choosing a command.
    // The input owner takes precedence over the action currently executing.
    if(void* row=command_row(content))return row;
    for(void* row:content)
        if(g_execution_player&&displayed::field(row,"<TargetData>k__BackingField")==g_execution_player)return row;
    // Preserve H's existing no-active-character result during a non-party
    // action when no command context owns input. A stale focus is not its actor.
    if(g_action_ongoing&&!g_execution_player&&!choosing_command())return nullptr;
    for (void* row : content)
        if (g_focused_player && displayed::field(row, "<TargetData>k__BackingField") == g_focused_player) return row;
    for (void* row : content)
        if (displayed::visible(displayed::field(displayed::field(row, "view"), "focusImage"))) return row;
    return nullptr;
}

std::wstring submenu_slot(int index)
{
    if (!in_battle() || (!menu_state::is_active(menu_state::BATTLE_ITEM) &&
                        !menu_state::is_active(menu_state::BATTLE_MAGIC) &&
                        !menu_state::is_active(menu_state::BATTLE_TARGET))) return {};
    // The normal native UpdateView binds contentList in HUD order and roots its
    // roster. Battle submenus can hide the HUD without clearing those bindings.
    void* owner = g_ordinary_party;
    if (!unity::alive(owner) || !ref::rd<void*>(owner, fRoster)) return {};
    void* row = unity::list_at(ref::rd<void*>(owner, fContents), index);
    void* view = displayed::field(row, "view");
    void* player = displayed::field(row, "<TargetData>k__BackingField");
    void* owned = player_owned_character(player);
    if (!view || !owned) return {};
    void* parameter = unit_parameter(player);
    auto line = character_name(owned);
    if (line.empty() || !parameter) return {};

    line += L", " + T(L"HP") + L" " + text::num(param_current_hp(parameter));
    const int maximum = param_confirmed_or_base(parameter, Stat::MaxHp);
    if (maximum > 0) line += L"/" + text::num(maximum);
    if (displayed::field(view, "currentMpText"))
        line += L", " + T(L"MP") + L" " + text::num(param_current_mp(parameter));
    const auto conditions = party_status::current_lines(player, view);
    if (!conditions.empty()) line += L", " + text::join(conditions, L", ");
    return line;
}

void announce_slot(int slot)
{
    if (slot < 1 || slot > 5) return;
    const auto content = rows();
    void* visible = slot <= static_cast<int>(content.size()) ? content[slot - 1] : nullptr;
    const auto line = visible ? read_row(visible) : submenu_slot(slot - 1);
    if (!line.empty()) ffpr::say(line, true);
}

void cycle_party(int delta)
{
    const auto content = rows();
    if (content.empty()) return;
    const int count = static_cast<int>(content.size());
    for (int i = 0; i < count; ++i) {
        g_review_slot = g_review_slot < 0 ? (delta < 0 ? count - 1 : 0) : (g_review_slot + delta + count) % count;
        if (content[g_review_slot]) break;
    }
    ffpr::say(read_row(content[g_review_slot]), true);
}

int install()
{
    queries::define_text("BattleParty.ChargeDetails", [] {
        const bool target = menu_state::is_active(menu_state::BATTLE_TARGET);
        if (!in_battle() || (!target && !menu_state::is_active(menu_state::BATTLE_COMMAND))) return std::wstring();
        void* selected = nullptr;
        auto content = rows();
        if (target) {
            for (void* row : content)
                if (g_focused_player && displayed::field(row, "<TargetData>k__BackingField") == g_focused_player)
                    selected = row;
        } else selected = active_row(content);
        // I reads only the optional charge tail. Name/vitals/command belong
        // to ordinary row speech, so repeating them here duplicates that read.
        return text::join(displayed::charge_lines(displayed::field(selected, "view")), L", ");
    });
    // Separate battle-read scope also works while a native spell/item submenu
    // owns the information buffer. New scope IDs retire the old number-row
    // bindings without changing shop number keys or native L/R on2/3.
    constexpr input::Key keys[]{input::Key::Slash,input::Key::Period,input::Key::Comma,input::Key::M,input::Key::N};
    constexpr const char* labels[]{"Battle: party slot 1 statistics","Battle: party slot 2 statistics",
        "Battle: party slot 3 statistics","Battle: party slot 4 statistics","Battle: party slot 5 statistics"};
    for (int i = 0; i < 5; ++i) {
        const std::string action = "BattleParty.Slot" + std::to_string(i + 1);
        actions::define(action.c_str(), [i] { announce_slot(i + 1); });
        input::add_binding(keys[i], input::Mod::None, input::Context::BattleParty,
                          action.c_str(), labels[i]);
    }
    actions::define("BattleParty.ActiveTurn", [] { announce_active_turn(); });
    input::add_binding(input::Key::T, input::Mod::None, input::Context::Battle,
                       "BattleParty.ActiveTurn", "Battle: announce active turn");
    actions::define("BattleParty.Next", [] { cycle_party(1); });
    actions::define("BattleParty.Previous", [] { cycle_party(-1); });
    // The ordinary panel can already have been bound when battle readiness fires.
    events::on(events::Kind::BattleStart, [] { g_party_owners.clear();g_party = g_focused_player = nullptr; g_review_slot = -1; action_actor_finished(); });
    events::on(events::Kind::BattleEnd, [] { g_party_owners.clear();g_ordinary_party = g_party = g_focused_player = nullptr; g_review_slot = -1; action_actor_finished(); });
    events::on(events::Kind::TitleScreen, [] { g_ordinary_party = nullptr; });
    return (hooks::attach(mInitialize, &d_initialize, &o_initialize) ? 1 : 0) +
        (hooks::attach(mUpdate, &d_update, &o_update) ? 1 : 0) + (hooks::attach(mFocus, &d_focus, &o_focus) ? 1 : 0);
}
core::Install s_install{"Battle.Party", &install, 110};
}

std::wstring displayed_player(void* player)
{
    if (!player) return {};
    void* owned = player_owned_character(player);
    const int id = owned ? character_id(owned) : 0;
    if(g_party_owners.empty())remember_party(field::cache::get(kParty));
    for(auto* owner:g_party_owners)for (void* row : rows_for(owner)) {
        void* data = displayed::field(row, "<TargetData>k__BackingField");
        if (!data) continue;
        if (data == player) return read_row(row, true);
        // Target enumerables can carry another BattlePlayerData for the same
        // character. Identity selects the rendered row, never hidden statistics.
        void* row_owned = player_owned_character(data);
        if (owned && row_owned && (owned == row_owned || (id > 0 && id == character_id(row_owned)))) return read_row(row, true);
    }
    return {}; // A parameter object alone is not renderer evidence.
}

void observe_target_party(void* owner)
{
    remember_party(owner);
}

void announce_active_status()
{
    if (!in_battle()) { ffpr::say(T(L"Party status only available in battle"), true); return; }
    void* row = active_row(rows());
    ffpr::say(row ? read_row(row) : T(L"No active character"), true);
}

void announce_active_turn()
{
    if (!in_battle()) return;
    const bool choosing=choosing_command();
    // A target/animation panel may temporarily hide the commander's party row.
    // Its rooted native actor, matched to the admitted input owner by identity,
    // still owns the same already-announced name. Never dereference the saved
    // pointer itself or substitute an executing unit for this input context.
    if(choosing)if(void* actor=command_actor();actor&&actor==current_actor()) {
        const auto name=character_name(actor);
        if(!name.empty()){ffpr::say(TF(L"{0}'s turn",{name}),true);return;}
    }
    const auto content=rows();
    void* row=choosing?command_row(content):g_execution_player?active_row(content):nullptr;
    auto name=row?displayed::text_field(displayed::field(row,"view"),"nameText"):std::wstring();
    if(name.empty()&&!choosing&&g_execution_player)name=g_execution_name;
    ffpr::say(name.empty() ? T(L"No active character") : TF(L"{0}'s turn", {name}), true);
}
void action_actor_started(void* unit)
{
    g_action_ongoing=unit!=nullptr;
    g_execution_player = kBattlePlayerData.is(unit) ? unit : nullptr;
    // This observer belongs to party information, not enemy turn tracking.
    // Enemy action/damage readers retain their own native producers.
    g_execution_name=g_execution_player?unit_name(unit):std::wstring();
}
void action_actor_finished() { g_action_ongoing=false;g_execution_player = nullptr; g_execution_name.clear(); }
}
