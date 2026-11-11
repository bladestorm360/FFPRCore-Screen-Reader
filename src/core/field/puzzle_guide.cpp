// Guided puzzle solutions (user decision, 2026-10-10: "guided puzzle solutions with
// step-by-step directions"). A puzzle is a list of steps authored per game in
// data/<game>/puzzles.json (embedded as PUZZLES_FFN): each step is a destination on some
// sub-map (a switch to step on, an object to examine from beside it, a floor to reach) with a
// short instruction. The guide speaks the current step with live directions from the player's
// position, across maps when needed (baked_nav::find_across: exits and script warps), and
// advances at a waypoint or after the control's exact native operation receipt.
// Standing beside a control never establishes that it was operated.
//
// Keys: Shift+G starts the guide for this map or repeats the current step; Ctrl+G stops it.
// Pad: mod mode + RB / LB. Events: a map change re-announces the step after the map name;
// each movement input (FieldPlayerKeyController.OnTouchPadCallback, an input event) checks
// the arrival. No polling.
//
// puzzles.json:
//   {"puzzles": [{"id": "citadel_of_trials", "maps": [93, 94, 95],
//                 "steps": [{"map": 95, "x": 29, "y": 29, "layer": 0, "examine": false,
//                            "text": "Go to {0}", "arg": "map:95"}]}]}
// "text" is a mod-text key; "arg" fills {0}: "map:<id>" = that map's name through the game,
// anything else = the literal. Map ids are the map master's (tools/mapdump/<game>_master/map.txt).
#include <string>
#include <vector>
#include <climits>
#include "baked_nav.h"
#include "field_api.h"
#include "field_internal.h"
#include "map_names.h"
#include "map_state.h"
#include "puzzle_entry_notice.h"
#include "scene_events.h"
#include "../core.h"
#include "../frame.h"
#include "../hooks.h"
#include "../json.h"
#include "../log.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../ui/preferences.h"

namespace field::puzzle_guide {
namespace {

struct Step {
    int map_id = 0;
    baked_route::Cell cell;
    bool examine = false;
    int entity_id = -1;
    std::wstring raw_name;
    int done_scenario = -1, done_local = -1, done_value = 1;
    int done_keyword = -1; // FFII learned-word receipt, explicit repeat only
    int before_scenario = -1; // unfinished operation expires at this native story receipt
    bool done_hidden = false;
    bool reach_map = false; // a maze whose actual end is entry into its final floor
    bool allow_world_transit = false; // explicit fetch/return leg, not a speculative dungeon detour
    std::wstring text;   // mod-text key
    std::wstring arg;    // "map:<id>" or a literal
    std::wstring arg2;   // second owned localized term for Ask/Learn instructions
};

struct Puzzle {
    std::wstring id;
    int requires_scenario=-1,done_scenario=-1;
    std::vector<int> maps;
    std::vector<Step> steps;
};

std::vector<Puzzle>* g_puzzles = nullptr;
bool g_loaded = false;
int g_active = -1;         // index into g_puzzles
int g_step = 0;
int g_arrived_step = -1;   // the step whose arrival was already announced
uint64_t g_context = 0;
uint64_t g_entry_context = 0;
puzzle_policy::Policy policy(){return {ui::prefs::navigation_puzzle_guidance(),ui::prefs::other_puzzle_guidance()};}
bool enabled(const Puzzle& p,const Step& step) {
    return policy().allows(puzzle_policy::kind(data::game_json("puzzles"),p.id,step.text));
}
bool field_ready() {
    return !queries::is("GameStatePatches.IsInputTransitionBlocked") &&
        queries::is("GameStatePatches.IsFieldNavigationReady") &&
        !queries::is("BattleStateHelper.IsInBattle");
}

void load()
{
    if (g_loaded || !data::game_data_ready()) return;
    g_loaded = true;
    g_puzzles = new std::vector<Puzzle>();
    const json::Value& root = data::game_json("puzzles");
    const json::Value* list = root.get(L"puzzles");
    if (!list || list->type != json::Value::Type::Array) {
        LOGF("[PuzzleGuide] no puzzles for this game");
        return;
    }
    for (const json::Value& p : list->arr) {
        Puzzle pz;
        if (const json::Value* v = p.get(L"id")) pz.id = v->s;
        if (const auto* v=p.get(L"requires_scenario"))pz.requires_scenario=static_cast<int>(v->n);
        if (const auto* v=p.get(L"done_scenario"))pz.done_scenario=static_cast<int>(v->n);
        if (const json::Value* v = p.get(L"maps"))
            for (const json::Value& m : v->arr) pz.maps.push_back(static_cast<int>(m.n));
        if (const json::Value* v = p.get(L"steps")) {
            for (const json::Value& s : v->arr) {
                Step st;
                if (const json::Value* x = s.get(L"map")) st.map_id = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"x")) st.cell.x = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"y")) st.cell.y = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"layer")) st.cell.layer = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"examine")) st.examine = x->b;
                if (const json::Value* x = s.get(L"entity")) st.entity_id = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"raw")) st.raw_name = x->s;
                if (const json::Value* x = s.get(L"done_scenario")) st.done_scenario = static_cast<int>(x->n);
                if (const auto* x=s.get(L"before_scenario"))st.before_scenario=static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"done_local")) st.done_local = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"done_value")) st.done_value = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"done_hidden")) st.done_hidden = x->b;
                if (const json::Value* x = s.get(L"reach_map")) st.reach_map = x->b;
                if (const json::Value* x = s.get(L"allow_world_transit")) st.allow_world_transit = x->b;
                if (const json::Value* x = s.get(L"done_keyword")) st.done_keyword = static_cast<int>(x->n);
                if (const json::Value* x = s.get(L"text")) st.text = x->s;
                if (const json::Value* x = s.get(L"arg")) st.arg = x->s;
                if (const json::Value* x = s.get(L"arg2")) st.arg2 = x->s;
                if (st.map_id > 0 && !st.text.empty()) pz.steps.push_back(st);
            }
        }
        // Authored resources are finite. Do not silently execute a partial
        // recipe or allow an examined control to fall back to proximity.
        bool valid=!pz.steps.empty()&&pz.steps.size()<=32&&pz.requires_scenario>=-1&&pz.requires_scenario<1024&&pz.done_scenario>=-1&&pz.done_scenario<1024;
        for(const auto& s:pz.steps) {
            valid&=s.done_scenario>=-1&&s.done_scenario<1024&&s.done_local>=-1&&s.done_local<8;
            valid&=s.before_scenario>=-1&&s.before_scenario<1024;
            valid&=s.done_keyword==-1||(game::is(game::Id::FF2)&&s.done_keyword>0&&s.done_keyword<=65535);
            valid&=s.arg2.empty()||!s.arg.empty();
            if(s.examine)valid&=s.done_scenario>=0||s.done_local>=0||s.done_hidden||s.done_keyword>0;
            if(s.done_hidden)valid&=s.entity_id>=0&&!s.raw_name.empty();
        }
        if(valid&&g_puzzles->size()<64) g_puzzles->push_back(std::move(pz));
    }
    LOGF("[PuzzleGuide] %d puzzle guide(s) loaded", static_cast<int>(g_puzzles->size()));
}

const Puzzle* active() { return g_active >= 0 && g_active < static_cast<int>(g_puzzles->size()) ? &(*g_puzzles)[static_cast<size_t>(g_active)] : nullptr; }
bool can_repeat_during_event() {
    return active()&&!queries::is("BattleStateHelper.IsInBattle")&&
        (queries::is("MessageChoice.IsOpen")||queries::is("MessageWindowPatches.IsInDialogue")||
         (game::is(game::Id::FF3)&&menu_state::is_active(menu_state::EVENT_ITEM_SELECT))||
         (game::is(game::Id::FF4)&&menu_state::any_active())||
         (game::is(game::Id::FF2)&&queries::is("KeywordMenuState.IsActive")));
}

std::wstring resolve_arg(const std::wstring& arg)
{
    // Official keyword/item terms are baked into the owned localization
    // catalog. Repeating an answer in a native choice performs no game lookup.
    if(text::starts_with(arg,L"modtext:"))return T(arg.substr(8));
    if (text::starts_with(arg, L"map:")) {
        int id = 0;
        for (size_t i = 4; i < arg.size(); ++i)
            if (arg[i] >= L'0' && arg[i] <= L'9') id = id * 10 + (arg[i] - L'0');
        std::wstring name = baked_nav::map_name_of(id);
        return name.empty() ? TF(L"Map {0}", {text::num(id)}) : name;
    }
    return arg;
}

std::wstring step_text(const Step& s) {
    if(!s.arg2.empty())return TF(s.text,{resolve_arg(s.arg),resolve_arg(s.arg2)});
    return s.arg.empty()?T(s.text):TF(s.text,{resolve_arg(s.arg)});
}

std::wstring puzzle_name(const Puzzle& p)
{
    const int id = p.maps.empty() ? p.steps.front().map_id : p.maps.front();
    std::wstring name = baked_nav::map_name_of(id);
    return name.empty() ? p.id : name;
}

/// "Step 2 of 4: Step on switch 2. North 3, East 1."
void announce_step(bool interrupt)
{
    const Puzzle* p = active();
    if (!p) return;
    if (g_step >= static_cast<int>(p->steps.size())) return;
    const Step& s = p->steps[static_cast<size_t>(g_step)];
    if(!enabled(*p,s)) {
        baked_nav::MapGoal old;old.map_id=s.map_id;old.cell=s.cell;
        baked_nav::clear_preview_destination(old);++g_context;
        if(interrupt)ffpr::say(T(puzzle_policy::disabled_key(puzzle_policy::kind(data::game_json("puzzles"),p->id,s.text))),true);
        return;
    }
    const auto caption = [step = g_step, count = p->steps.size(), verbose = ui::prefs::puzzle_guide_verbose()](std::wstring line) {
        return verbose ? TF(L"Step {0} of {1}: {2}", {text::num(step + 1), text::num(static_cast<long long>(count)),line}) : line;
    };
    if(can_repeat_during_event()) {
        // Choice/dialogue repeat speaks only the owned authored instruction.
        // No flag read, route/capture request or native pointer is involved.
        ffpr::say(caption(step_text(s)),interrupt);
        return;
    }
    if(!field_ready())return;

    baked_nav::MapGoal goal;
    goal.map_id = s.map_id;
    goal.allow_world_transit = s.allow_world_transit;
    goal.cell = s.cell;
    goal.examine = s.examine;
    goal.entity_id = s.entity_id;
    goal.raw_name = s.raw_name;
    const bool treasury = p->id == L"troia_castle_treasury" && current_map_id() == s.map_id;
    goal.avoid_other_events = treasury;
    if(s.before_scenario>=0) {
        // A native story variant can retire a still-unfinished optional clue.
        // Test only this current operation on explicit guidance; a completed
        // clue was already skipped by its own receipt. Never route to its old
        // actor after the window closes, or read this extra flag on movement.
        const auto expired=map_state::scenario_flag(s.before_scenario);
        if(expired!=0) {
            baked_nav::clear_preview_destination(goal);++g_context;
            ffpr::say(T(expired.has_value()?L"Story gated":L"Navigation data is loading"),interrupt);return;
        }
    }
    const int step = g_step;
    const auto context = g_context;
    auto publish = [p, step, context, caption, treasury, interrupt](PathInfo info) {
        if (!field_ready() || context != g_context || active() != p || g_step != step || !enabled(*p,p->steps[static_cast<size_t>(step)])) return;
        auto instruction = step_text(p->steps[static_cast<size_t>(step)]);
        if (treasury && info.success && info.step_count == 0)
            instruction = T(L"Step off and back onto the switch");
        else if (treasury && info.success && info.guidance_index == 1)
            instruction = T(L"Step on the entrance switch");
        auto line = caption(std::move(instruction));
        if (info.success) {
            if (!info.description.empty() && info.description != T(L"No movement needed")) line += L". " + info.description;
        } else line += L". " + (!info.reason.empty() ? info.reason : T(L"No path to the next step"));
        if (ui::prefs::mod_warnings_enabled() && !info.warning.empty())
            ffpr::say_sequence({info.warning, std::move(line)}, interrupt);
        else ffpr::say(line, interrupt);
    };
    if (treasury && step > 0) {
        // The live callback opens the gate and leaves the next contact usable.
        // Reloading B1 can hide that contact. The entrance switch remains live
        // and runs the same receipt-driven script. Try both in one capture.
        const auto& entrance = p->steps.front();
        auto alternate = goal;
        alternate.cell = entrance.cell;
        alternate.entity_id = entrance.entity_id;
        alternate.raw_name = entrance.raw_name;
        baked_nav::request_guidance({{std::move(goal), TargetKind::Spot, {}},
                                     {std::move(alternate), TargetKind::Spot, {}}}, std::move(publish));
    } else {
        baked_nav::request_route(std::move(goal), s.examine?TargetKind::Interact:TargetKind::Spot, L"", std::move(publish), false);
    }
}

void on_move(bool interrupt = true,bool observe_controls = false);
void start_or_repeat()
{
    if(!policy().any()){ffpr::say(T(L"Puzzle guidance is off"),true);return;}
    if(can_repeat_during_event()){announce_step(true);return;}
    if(!field_ready()) {
        if(!queries::is("GameModule.PuzzleGuideDuringEvent")&&!queries::is("GameModule.PuzzleGuideReadyDuringEvent")&&
           !queries::is("GameModule.PuzzleGuideRepeatDuringEvent"))return;
    }
    if(queries::is("GameModule.HasPuzzleGuide")) {
        if(const auto* old=active()) {
            baked_nav::MapGoal goal;goal.map_id=old->steps[static_cast<size_t>(g_step)].map_id;
            goal.cell=old->steps[static_cast<size_t>(g_step)].cell;
            baked_nav::clear_preview_destination(goal);
        }
        ++g_context;g_active=-1;g_step=0;g_arrived_step=-1;
        actions::run("GameModule.PuzzleGuide");return;
    }
    load();
    if (!g_puzzles) { ffpr::say(T(L"Navigation data is loading")); return; }
    if (!active()) {
        const int map_id = current_map_id();
        for (size_t i = 0; i < g_puzzles->size(); ++i) {
            const Puzzle& p = (*g_puzzles)[i];
            bool here = false;
            for (int m : p.maps) here = here || m == map_id;
            for (const Step& s : p.steps) here = here || s.map_id == map_id;
            if (!here) continue;
            if(!puzzle_policy::any_enabled(data::game_json("puzzles"),p.id,policy())) {
                ffpr::say(T(puzzle_policy::disabled_key(puzzle_policy::kind(data::game_json("puzzles"),p.id))),true);return;
            }
            if(p.done_scenario>=0&&map_state::scenario_flag(p.done_scenario)==1){ffpr::say(T(L"Puzzle guide finished"),true);return;}
            if(p.requires_scenario>=0) {
                const auto allowed=map_state::scenario_flag(p.requires_scenario);
                if(allowed!=1){ffpr::say(T(allowed.has_value()?L"Story gated":L"Navigation data is loading"),true);return;}
            }
            g_active = static_cast<int>(i);
            ++g_context;
            g_step = 0;
            g_arrived_step = -1;
            break;
        }
        if (!active()) {
            ffpr::say(T(L"No puzzle guide for this map"), true);
            return;
        }
        // Starting a different authored guide retires an older game-specific
        // continuation; two producers must not speak competing objectives.
        if(queries::is("GameModule.PuzzleGuideActive"))actions::run("GameModule.StopPuzzleGuide");
        const bool verbose=ui::prefs::puzzle_guide_verbose();
        if(verbose)ffpr::say(TF(L"Puzzle guide: {0}", {puzzle_name(*active())}), true);
        const int initial_step=g_step;
        on_move(!verbose,true);
        if(!active()||g_step!=initial_step)return;
        announce_step(!verbose);
        return;
    }
    const int initial_step=g_step;
    on_move(true,true);
    if(active()&&g_step==initial_step)announce_step(true);
}

void stop()
{
    // Retirement touches owned state only, including during native loading,
    // dialogue or a movie. Readiness must never trap an already-owned guide.
    const bool module=queries::is("GameModule.PuzzleGuideActive");
    if(module)actions::run("GameModule.StopPuzzleGuide");
    if (!active()) return;
    const auto& step = active()->steps[static_cast<size_t>(g_step)];
    baked_nav::MapGoal goal; goal.map_id=step.map_id;goal.cell=step.cell;
    baked_nav::clear_preview_destination(goal);
    ++g_context;
    g_active = -1;
    g_step = 0;
    g_arrived_step = -1;
    if(!module)ffpr::say(T(L"Puzzle guide stopped"), true);
}

void policy_changed() {
    // Settings changes retire guide-owned continuations and previews only.
    // Ordinary selected-object routes and native menu readers retain ownership.
    if(const auto* p=active()) {
        const auto& s=p->steps[static_cast<size_t>(g_step)];baked_nav::MapGoal goal;
        goal.map_id=s.map_id;goal.cell=s.cell;baked_nav::clear_preview_destination(goal);
    }
    ++g_context;++g_entry_context;g_active=-1;g_step=0;g_arrived_step=-1;
    frame::cancel_label("PuzzleGuide.MapChanged");
    if(actions::defined("GameModule.PuzzleGuidePolicyChanged"))actions::run("GameModule.PuzzleGuidePolicyChanged");
}

// Only the exact current-map native bank or validated actor can establish a
// temporary operation. Persistent scenario bits also support a resumed guide.
bool operation_completed(const Step& s,bool observe_controls)
{
    if(s.reach_map)return current_map_id()==s.map_id;
    if(s.done_keyword>0)return observe_controls&&map_state::keyword_learned(s.done_keyword)==true;
    if(s.done_scenario>=0)return map_state::scenario_flag(s.done_scenario)==s.done_value;
    if(s.done_local>=0)return map_state::local_value(s.map_id,s.done_local)==s.done_value;
    if(s.done_hidden&&observe_controls)return map_state::entity_hidden(s.map_id,s.entity_id,s.raw_name)==true;
    return false;
}

/// Native movement/ready events and explicit repeat: observe, never run scripts.
void on_move(bool interrupt,bool observe_controls)
{
    if (!field_ready()) return;
    if ((game::is(game::Id::FF3)||game::is(game::Id::FF5)) && queries::is("GameModule.PuzzleGuideActive")) { actions::run("GameModule.PuzzleGuideMove"); return; }
    const Puzzle* p = active();
    if (!p || g_step >= static_cast<int>(p->steps.size())) return;
    const Step& s = p->steps[static_cast<size_t>(g_step)];
    if(!enabled(*p,s))return; // never skip an unsolved disabled operation
    // The native end receipt outranks an optional earlier dependency or an
    // unavailable learned-word observation on a completed/resumed family.
    if(p->done_scenario>=0&&map_state::scenario_flag(p->done_scenario)==1) {
        baked_nav::MapGoal previous;previous.map_id=s.map_id;previous.cell=s.cell;
        baked_nav::clear_preview_destination(previous);++g_context;
        g_active=-1;g_step=0;g_arrived_step=-1;ffpr::say(T(L"Puzzle guide finished"),interrupt);return;
    }
    const bool receipt=s.done_scenario>=0||s.done_local>=0||s.done_hidden||s.reach_map||s.done_keyword>0;
    if(receipt) {
        if(!operation_completed(s,observe_controls))return;
    } else {
        if (s.examine||g_arrived_step==g_step||current_map_id() != s.map_id) return;
        baked_route::Cell at;
        if (!baked_nav::player_cell(map_handle(), player(), at)) return;
        // Bridge/ground walkways can share XY. A coordinate waypoint must
        // retain its actual logical layer, separately from floor-entry receipts.
        if (at.layer != s.cell.layer||at.x!=s.cell.x||at.y!=s.cell.y) return;
    }

    g_arrived_step = g_step;
    baked_nav::MapGoal previous;previous.map_id=s.map_id;previous.cell=s.cell;
    baked_nav::clear_preview_destination(previous);
    ++g_context; // an old route cannot speak after its operation completed
    if (g_step + 1 >= static_cast<int>(p->steps.size())) {
        ffpr::say(T(L"Puzzle guide finished"), interrupt);
        g_active = -1;
        g_step = 0;
        g_arrived_step = -1;
        return;
    }
    const bool verbose=ui::prefs::puzzle_guide_verbose();
    if(verbose)ffpr::say(T(L"Step done"), interrupt);
    ++g_step;
    // At most32 authored steps, and only one cheap receipt per completed step.
    // This is input/event-driven; there is no full flag/party snapshot or poll.
    while(g_step<static_cast<int>(p->steps.size())&&operation_completed(p->steps[static_cast<size_t>(g_step)],observe_controls)) {
        ++g_step;
    }
    if(g_step==static_cast<int>(p->steps.size())) {
        ffpr::say(T(L"Puzzle guide finished"),interrupt&&!verbose);
        g_active=-1;g_step=0;g_arrived_step=-1;return;
    }
    announce_step(interrupt&&!verbose);
}

void on_map_changed()
{
    // One entry receipt serves inactive availability and existing active guide
    // progression. It never starts a guide or computes a route merely to notify.
    frame::cancel_label("PuzzleGuide.MapChanged");
    const auto entry=++g_entry_context;
    const int map=current_map_id();
    const auto context = g_context;
    frame::every_frame([context,entry,map] {
        if(entry!=g_entry_context||map!=current_map_id())return false;
        if (!field_ready()||!data::game_data_ready()) return true;
        const bool module=(game::is(game::Id::FF3)||game::is(game::Id::FF5))&&queries::is("GameModule.HasPuzzleGuide");
        if(entry_guide_available(data::game_json("puzzles"),map,module,[](int bit){return map_state::scenario_flag(bit);},policy()))
            ffpr::say(T(L"Puzzle guide active"),false);
        if(context!=g_context||!active())return false;
        const int step = g_step;
        on_move(false);
        if (!active() || g_step != step) return false;
        announce_step(false);
        return false;
    }, "PuzzleGuide.MapChanged", INT_MAX);
}

int install()
{
    actions::define("Settings.Changed.NavigationPuzzleGuidance",[]{policy_changed();});
    actions::define("Settings.Changed.OtherPuzzleGuidance",[]{policy_changed();});
    actions::define("PuzzleGuide.Next", [] { hooks::guarded("PuzzleGuide.Next", [] { start_or_repeat(); }); });
    actions::define("PuzzleGuide.Stop", [] { hooks::guarded("PuzzleGuide.Stop", [] { stop(); }); });
    actions::define("PuzzleGuide.OnMove", [] { hooks::guarded("PuzzleGuide.OnMove", [] { on_move(); }); });
    queries::define("PuzzleGuide.IsActive", [] { return active() != nullptr || queries::is("GameModule.PuzzleGuideActive"); });
    queries::define("PuzzleGuide.CanRepeatDuringEvent", [] {
        return can_repeat_during_event()||(game::is(game::Id::FF6)&&(queries::is("GameModule.PuzzleGuideReadyDuringEvent")||
            queries::is("GameModule.PuzzleGuideRepeatDuringEvent")))||((game::is(game::Id::FF3)||game::is(game::Id::FF5))&&
            queries::is("GameModule.PuzzleGuideActive")&&queries::is("GameModule.PuzzleGuideDuringEvent"));
    });
    events::on(events::Kind::MapChanged, [] { hooks::guarded("PuzzleGuide.MapChanged", [] { on_map_changed(); }); });
    for(auto event:{events::Kind::DialogueClosed,events::Kind::MenuClosed,events::Kind::BattleEnd})
        events::on(event, [] {if(active())hooks::guarded("PuzzleGuide.NativeReady", [] {on_move(false);});});
    events::on(events::Kind::MapChanging, [] { ++g_context;++g_entry_context; });
    events::on(events::Kind::BattleStart, [] { ++g_context;++g_entry_context; frame::cancel_label("PuzzleGuide.MapChanged"); });
    events::on(events::Kind::SceneLoaded, [] {
        if (scene_name() != L"Title") return;
        ++g_context;++g_entry_context; g_active=-1; g_step=0; g_arrived_step=-1;
        frame::cancel_label("PuzzleGuide.MapChanged");
    });
    return 0;
}
core::Install s_install{"PuzzleGuide", &install, 130};

} // namespace
} // namespace field::puzzle_guide
