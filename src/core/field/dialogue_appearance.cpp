#include "field_internal.h"
#include "field_api.h"
#include "object_cache.h"
#include "../core.h"
#include "../dialogue/dialogue.h"
#include "../displayed_ui.h"
#include "../fieldnav/map_transition.h"
#include "../frame.h"
#include "../modtext.h"
#include "../say.h"
#include <algorithm>
#include <span>

namespace ffpr::dialogue {
namespace {
namespace r = ::field::r;

ref::Class kManager{"Last.Message", "MessageWindowManager"};
ref::Field fFieldWindow{kManager, "fieldWindowController"};
ref::Field fCurrentWindow{kManager, "currentWindowController"};
ref::Class kWindow{"Last.Message", "MessageWindowController"};
ref::Field fView{kWindow, "view"};
ref::Class kView{"Last.Message", "MessageWindowView"};
ref::Field fMessageText{kView, "messageText"};
ref::Class kClient{"Last.Management", "MessageWindowClient"};
ref::Method mClient{kClient, "Instance", 0};
ref::Field fMessageId{kClient, "<CurrentMessageId>k__BackingField"};

ref::Field fEventOwner{r::kFieldController, "eventHandle"};
ref::Class kEventProcedure{"Last.Map", "EventProcedure"};
ref::Field fRunAction{kEventProcedure, "runAction"};
ref::Class kEventAction{"Last.Map", "EventAction"};
ref::Class kScriptAction{"Last.Map", "EventActionScript"};
ref::Field fScriptName{kScriptAction, "scriptName"};
ref::Field fObserver{r::kFieldController, "interactiveTargetObserver"};
ref::Class kObserver{"Last.Map", "InteractiveTargetObserver"};
ref::Field fTarget{kObserver, "interactiveTargetEntity"};

ref::Class kSpriteEntity{"Last.Entity.Field", "FieldSpriteEntity"};
ref::Field fRenderer{kSpriteEntity, "spriteRenderer"};
ref::Class kRenderer{"UnityEngine", "SpriteRenderer"};
ref::Method mSprite{kRenderer, "get_sprite", 0};
ref::Class kSprite{"UnityEngine", "Sprite"};
ref::Method mTexture{kSprite, "get_texture", 0};

std::wstring field_message_key()
{
    auto* manager = ref::instance(kManager);
    auto* window = ref::rd<void*>(manager, fCurrentWindow);
    if (!window || window != ref::rd<void*>(manager, fFieldWindow) ||
        !displayed::visible(ref::rd<void*>(ref::rd<void*>(window, fView), fMessageText))) return {};
    // The six installed SetContent implementations have one native caller.
    // MessageWindowClient.Play records the corresponding ID before setting content.
    auto* client = ref::call_static<void*>(mClient);
    return ref::rd_str_bounded(client, fMessageId, 96);
}

bool matches_direct_talk(void* actor, const std::wstring& message)
{
    if (!r::kFieldNonPlayer.is(actor) || !::field::live_entity(actor)) return false;
    auto* property = ::field::entity_property(actor);
    return r::kPropertyTalk.is(property) && r::fEventScriptId &&
           ref::rd<int>(property, r::fEventScriptId) == 0 &&
           ref::rd_str_bounded(property, r::fTalkMessageKey, 96) == message &&
           displayed::renderer_visible(ref::rd<void*>(actor, fRenderer));
}

void* unique_actor(void* controller, const auto& matches)
{
    auto* actors = ref::rd<void*>(controller, r::fEntityList);
    const int count = unity::list_count(actors);
    if (count < 1 || count > 64) return nullptr;
    void* result = nullptr;
    for (int i = 0; i < count; ++i) {
        auto* actor = unity::list_at(actors, i);
        if (!matches(actor)) continue;
        if (result && result != actor) return nullptr;
        result = actor;
    }
    return result;
}

void* direct_talk_actor(void* controller, const std::wstring& message)
{
    auto* target = ref::rd<void*>(ref::rd<void*>(controller, fObserver), fTarget);
    if (matches_direct_talk(target, message)) return target;
    // When the indicator clears, require a unique visible actor with this key.
    return unique_actor(controller, [&](void* actor) { return matches_direct_talk(actor, message); });
}

void* scripted_talk_actor(void* controller, void* action, const std::wstring& message)
{
    struct Speaker { int map; std::wstring_view script, message; int entity; };
    static constexpr Speaker cornelians[] = {
        {72, L"sc_e_0003", L"MSG_HELPSERA_01", 32},
        {72, L"sc_e_0003", L"MSG_HELPSERA_02", 32},
        {3, L"sc_e_0002", L"MSG_MAP0A01_10", 14},
        {3, L"sc_e_0002_1", L"MSG_EKKEN_02", 12},
        {3, L"sc_e_0004_1", L"MSG_LUTE_02", 7},
        {3, L"sc_e_0004_2", L"MSG_LUTE_01", 7},
        {3, L"sc_e_0004_2", L"MSG_LUTE_02", 7},
        {13, L"sc_e_0009", L"MSG_SHIP_01", 38},
        {13, L"sc_e_0009", L"MSG_PBK_05", 38},
        {13, L"sc_e_0009_2", L"MSG_SHIP_03", 38},
        {33, L"sc_e_0011", L"MSG_ASTOS_01", 105},
        {33, L"sc_e_0011", L"MSG_ASTOS_02", 106},
        {33, L"sc_e_0011", L"MSG_ASTOS_03", 106},
    };
    static constexpr Speaker altair[] = {
        {6, L"sc_e_0001_5", L"E0001_04_006_a_07", 49},
        {6, L"sc_e_0001_5", L"E0001_04_006_e_09", 49},
        {7, L"sc_e_0001_3", L"E0001_03_007_a_01", 14},
        {7, L"sc_e_0001_3", L"E0001_03_007_a_02", 15},
        {7, L"sc_e_0001_3", L"E0001_03_007_a_03", 14},
        {7, L"sc_e_0001_3", L"E0001_03_007_a_04", 15},
        {7, L"sc_e_0001_3", L"E0001_03_007_a_05", 14},
        {13, L"sc_e_0016", L"E0016_00_013_a_01", 9},
        {13, L"sc_e_0016", L"E0016_00_013_a_02", 9},
        {13, L"sc_e_0016", L"E0016_00_013_d_03", 9},
        {13, L"sc_e_0016", L"E0016_00_013_f_04", 9},
        {13, L"sc_e_0016", L"E0016_00_013_f_05", 9},
        {13, L"sc_e_0016", L"E0016_00_013_f_06", 6},
        {13, L"sc_e_0016", L"N022_C00_013_02_01", 6},
        {104, L"sc_e_0016_1", L"N022_C00_013_02_01", 8},
    };
    static constexpr Speaker baron[] = {
        {8, L"sc_e_0001_11", L"E0001_11_008_a_01", 19},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_02", 18},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_03", 19},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_04", 19},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_05", 18},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_08", 19},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_09", 18},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_11", 18},
        {8, L"sc_e_0001_11", L"E0001_11_008_a_12", 19},
    };
    static constexpr Speaker ur[] = {
        {12, L"sc_e_0010", L"E0010_00_020_a_03", 6},
        {12, L"sc_e_0010", L"E0010_00_020_a_04", 6},
        {12, L"sc_e_0010", L"E0010_00_020_a_05", 6},
    };
    static constexpr Speaker tycoon[] = {
        {71, L"sc_e_0001", L"E0001_00_071_a_03", 32},
        {71, L"sc_e_0001", L"E0001_00_071_a_05", 32},
        {71, L"sc_e_0001", L"E0001_00_071_a_07", 32},
    };
    static constexpr Speaker narshe[] = {
        {17, L"sc_e_0093", L"E0093_00_017_a_01", 14},
        {17, L"sc_e_0093", L"E0093_00_017_a_03", 14},
        {17, L"sc_e_0093", L"E0093_00_017_a_05", 14},
        {17, L"sc_e_0093", L"E0093_00_017_a_07", 14},
    };
    std::span<const Speaker> speakers;
    switch (game::current()) {
        case game::Id::FF1: speakers = cornelians; break;
        case game::Id::FF2: speakers = altair; break;
        case game::Id::FF3: speakers = ur; break;
        case game::Id::FF4: speakers = baron; break;
        case game::Id::FF5: speakers = tycoon; break;
        case game::Id::FF6: speakers = narshe; break;
        default: return nullptr;
    }
    const auto script = ref::rd_str_bounded(action, fScriptName, 48);
    const auto map = ::field::current_map_id();
    const auto speaker = std::find_if(speakers.begin(), speakers.end(), [&](const Speaker& row) {
        return row.map == map && row.script == script && row.message == message;
    });
    if (speaker == speakers.end()) return nullptr;
    // The stage reserves the entity ID for this utterance. The live renderer
    // still determines the costume and can reject a hidden or unloaded actor.
    return unique_actor(controller, [speaker](void* actor) {
        return kSpriteEntity.is(actor) && ::field::live_entity(actor) &&
               ref::rd<int>(::field::entity_property(actor), r::fPropEntityId) == speaker->entity &&
               displayed::renderer_visible(ref::rd<void*>(actor, fRenderer));
    });
}

void* actor_sprite(void* actor)
{
    return actor ? ref::call<void*>(mSprite, ref::rd<void*>(actor, fRenderer)) : nullptr;
}

std::wstring atlas_art(void* sprite)
{
    if (!unity::alive(sprite)) return {};
    // All six packed field atlases place the authored costume key immediately
    // before the texture's final hash. Pose sprite names are shared by outfits.
    const auto texture = unity::name(ref::call<void*>(mTexture, sprite));
    if (texture.size() > 128 || !texture.starts_with(L"sactx-")) return {};
    const auto start = texture.find(L"-MO_FF");
    if (start == std::wstring::npos) return {};
    const auto end = texture.find(L'-', start + 1);
    if (end == std::wstring::npos || end - start > 49) return {};
    auto art = texture.substr(start + 1, end - start - 1);
    for (wchar_t& c : art) {
        if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
        if (!((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9') || c == L'_')) return {};
    }
    return art;
}

std::wstring visible_art(void* actor) { return atlas_art(actor_sprite(actor)); }

struct StagedCue {
    game::Id title;
    int map;
    std::wstring_view script, message, text;
    int actor = 0;
    std::wstring_view art{};
    std::wstring_view pose{};
};

// The first page follows the verified visible action on these exact event
// paths. Some use the native player or mounted party, which has no NPC slot.
constexpr StagedCue staged_cues[] = {
    {game::Id::FF3, 153, L"sc_e_0001_1", L"E0001_01_153_a_01", L"Altar Cave opening cue"},
    {game::Id::FF4, 342, L"sc_e_0001_1", L"E0001_01_342_a_03", L"Red Wings deck cue", 5, L"mo_ff4_n101_c02"},
    {game::Id::FF4, 325, L"sc_e_0054_1", L"E0055_01_004_a_01", L"Enterprise workshop cue", 3, L"mo_ff4_n005_c01"},
    {game::Id::FF5, 198, L"sc_e_0001_1", L"E0001_01_198_a_01", L"Pirate ship opening cue", 44, L"mo_ff5_n111_c00"},
    {game::Id::FF5, 209, L"sc_e_0001_4", L"E0001_04_209_a_01", L"Wind Shrine opening cue", 30, L"mo_ff5_b002_c00"},
    {game::Id::FF6, 3, L"sc_e_0001_5", L"E0001_04_003_a_01", L"Narshe armored entry cue"},
    {game::Id::FF6, 122, L"sc_e_1033_1", L"E1033_00_122_a_03", L"Setzer engine cue", 6, L"mo_ff6_p010"},
    {game::Id::FF2, 31, L"sc_e_0057", L"E0057_00_031_a_01", L"Leila raised arm cue", 98, L"mo_ff2_p006_c00", L"righthandup_00"},
    {game::Id::FF3, 106, L"sc_e_0076_2", L"E0076_02_106_a_06", L"Saronia king down cue", 33, L"mo_ff3_n009_c01", L"down_00"},
    {game::Id::FF5, 93, L"sc_e_0052_1", L"E0052_00_093_a_08", L"Mid raised hand cue", 17, L"mo_ff5_n017_c00", L"righthandup_00"},
    {game::Id::FF5, 136, L"sc_e_0096_7", L"E0096_05_136_a_01", L"Moogle raised paw cue", 17, L"mo_ff5_n119_c00", L"righthandup_00"},
    {game::Id::FF5, 298, L"sc_e_0066", L"E0066_00_298_a_05", L"Cid celebrates cue", 5, L"mo_ff5_n016_c00", L"handsup_00"},
    {game::Id::FF5, 306, L"sc_e_0075_2", L"E0075_00_306_a_08", L"Krile reunion cue", 21, L"mo_ff5_p005_c00", L"handsup_00"},
    {game::Id::FF5, 582, L"sc_e_0100_2", L"E0100_02_580_a_01", L"Exdeath fallen armor cue", 28, L"mo_ff5_n001_c00", L"down_00"},
    {game::Id::FF6, 378, L"sc_e_0246", L"E0246_00_378_a_01", L"Gau bends forward cue", 15, L"mo_ff6_p012", L"dying_00"},
    {game::Id::FF6, 425, L"sc_e_0546", L"E0546_00_425_a_03", L"Locke statue cue", 205, L"mo_ff6_p002", L"dying_00"},
    {game::Id::FF6, 545, L"sc_e_1023", L"E1023_00_542_a_01", L"Strago stoops cue", 16, L"mo_ff6_p008", L"dying_00"},
    {game::Id::FF6, 555, L"sc_e_1020", L"E1020_00_542_a_04", L"Celes bandana cue", 30, L"mo_ff6_p007", L"dying_00"},
    {game::Id::FF6, 560, L"sc_e_1016", L"E1016_00_542_a_01", L"Gogo button cue", 41, L"mo_ff6_p013", L"dyingr_00"},
    {game::Id::FF6, 585, L"sc_e_0461_b", L"E0461_02_582_a_16", L"Locke fallen cue", 2, L"mo_ff6_p002", L"down_00"},
    {game::Id::FF6, 596, L"sc_e_1020", L"E1020_00_542_a_04", L"Celes bandana cue", 30, L"mo_ff6_p007", L"dying_00"},
};

std::wstring staged_cue(void* controller, const StagedCue& cue)
{
    auto* owner = ref::rd<void*>(controller, fEventOwner);
    auto* action = kEventProcedure.is(owner) ? ref::rd<void*>(owner, fRunAction) : nullptr;
    if (!kScriptAction.is(action) || ref::rd_str_bounded(action, fScriptName, 48) != cue.script) return {};
    if (cue.actor) {
        auto* actor = unique_actor(controller, [&](void* candidate) {
            return kSpriteEntity.is(candidate) && ::field::live_entity(candidate) &&
                   ref::rd<int>(::field::entity_property(candidate), r::fPropEntityId) == cue.actor;
        });
        if (!actor || !displayed::renderer_visible(ref::rd<void*>(actor, fRenderer))) return {};
        auto* sprite = actor_sprite(actor);
        if (atlas_art(sprite) != cue.art) return {};
        if (!cue.pose.empty()) {
            auto pose = unity::name(sprite);
            for (auto& ch : pose) if (ch >= L'A' && ch <= L'Z') ch += L'a' - L'A';
            if (pose.ends_with(L"(clone)")) pose.resize(pose.size() - 7);
            if (pose != cue.pose) return {};
        }
    }
    return T(cue.text);
}
} // namespace

bool read_dialogue_appearance()
{
    if (!frame::on_main_thread() || !in_dialogue() || queries::is("GameStatePatches.IsInputTransitionBlocked")) return false;
    const auto message = field_message_key();
    if (message.empty()) return false;
    auto* map = ::field::cache::get(r::kFieldMap);
    auto* controller = ref::rd<void*>(map, r::fFieldController);
    if (!r::kFieldController.is(controller)) return false;
    auto* event_owner = ref::rd<void*>(controller, fEventOwner);
    if (!kEventProcedure.is(event_owner)) return false;
    auto* action = ref::rd<void*>(event_owner, fRunAction);
    if (!kEventAction.is(action) || !kScriptAction) return false;
    const bool scripted = kScriptAction.is(action);
    const auto art = visible_art(scripted ? scripted_talk_actor(controller, action, message)
                                          : direct_talk_actor(controller, message));
    if (art.empty()) return false;
    const auto key = L"Field appearance " + art;
    if (!modtext::has(key)) return false;
    say(T(key), true);
    return true;
}

std::wstring action_cue_at_dialogue()
{
    const auto game_id = game::current();
    if (!frame::on_main_thread() || !in_dialogue() ||
        queries::is("GameStatePatches.IsInputTransitionBlocked")) return {};
    const int map_id = ::field::current_map_id();
    const bool construction = game_id == game::Id::FF1 && map_id == 121;
    const bool audience = game_id == game::Id::FF2 && map_id == 6;
    const bool surrender = game_id == game::Id::FF1 && map_id == 13;
    const bool reveal = game_id == game::Id::FF1 && map_id == 33;
    const auto belongs_here = [&](const StagedCue& cue) { return cue.title == game_id && cue.map == map_id; };
    if (!construction && !audience && !surrender && !reveal &&
        std::none_of(std::begin(staged_cues), std::end(staged_cues), belongs_here)) return {};
    const auto message = field_message_key();
    if (construction && message != L"MSG_BRIDGE_01") return {};
    if (audience && message != L"E0001_04_003_a_05") return {};
    if (surrender && message != L"MSG_SHIP_03") return {};
    if (reveal && message != L"MSG_ASTOS_02") return {};
    auto* map = ::field::cache::get(r::kFieldMap);
    auto* controller = ref::rd<void*>(map, r::fFieldController);
    if (!r::kFieldController.is(controller)) return {};
    for (const auto& cue : staged_cues)
        if (belongs_here(cue) && message == cue.message) return staged_cue(controller, cue);
    if (!construction && !audience && !surrender && !reveal) return {};
    if (surrender || reveal) {
        // These scenes finish the animated change before the next page.
        // Require that page's exact script, speaker and currently visible art.
        auto* owner = ref::rd<void*>(controller, fEventOwner);
        auto* action = kEventProcedure.is(owner) ? ref::rd<void*>(owner, fRunAction) : nullptr;
        if (!kScriptAction.is(action)) return {};
        const auto art = visible_art(scripted_talk_actor(controller, action, message));
        if (surrender && art == L"mo_ff1_n017_c00") return T(L"Pravoka pirate defeat cue");
        if (reveal && art == L"mo_ff1_n035_c00") return T(L"Western Keep transformation cue");
        return {};
    }
    if (audience) {
        // The audience's first page follows the completed head-lowering pose.
        // The later repetition accompanies a different message in this script.
        auto* owner = ref::rd<void*>(controller, fEventOwner);
        auto* action = kEventProcedure.is(owner) ? ref::rd<void*>(owner, fRunAction) : nullptr;
        if (!kScriptAction.is(action) || ref::rd_str_bounded(action, fScriptName, 48) != L"sc_e_0001_5") return {};
        auto* hilda = unique_actor(controller, [](void* actor) {
            return kSpriteEntity.is(actor) && ::field::live_entity(actor) &&
                   ref::rd<int>(::field::entity_property(actor), r::fPropEntityId) == 49;
        });
        if (hilda && displayed::renderer_visible(ref::rd<void*>(hilda, fRenderer)) &&
            visible_art(hilda) == L"mo_ff2_n005_c00") return T(L"Altair audience bow cue");
        return {};
    }
    // Both bridge branches show the first construction pan before this
    // dialogue. The later work montage has no speech producer.
    auto* actors = ref::rd<void*>(controller, r::fEntityList);
    const int count = unity::list_count(actors);
    if (count < 1 || count > 64) return {};
    for (int i = 0; i < count; ++i) {
        auto* actor = unity::list_at(actors, i);
        if (!kSpriteEntity.is(actor) || !::field::live_entity(actor)) continue;
        const int id = ref::rd<int>(::field::entity_property(actor), r::fPropEntityId);
        if (id < 1 || id > 10 || !displayed::renderer_visible(ref::rd<void*>(actor, fRenderer))) continue;
        if (visible_art(actor) == L"mo_ff1_n101_c00") return T(L"Cornelia bridge construction cue");
    }
    return {};
}

std::wstring action_cue_at_scroll(int type, float duration, int font_size)
{
    // The FF1 bridge prologue's first native scroll follows the complete
    // bird track. This scroll producer has a unique type, duration and size
    // in that script. Its sibling credits use a different event owner.
    if (!game::is(game::Id::FF1) || type != 0 || duration != 90 || font_size != 52 ||
        !frame::on_main_thread() || fieldnav::map_transition::is_state_changing() ||
        ::field::current_map_id() != 124) return {};
    auto* map = ::field::cache::get(r::kFieldMap);
    auto* controller = ref::rd<void*>(map, r::fFieldController);
    if (!r::kFieldController.is(controller)) return {};
    auto* owner = ref::rd<void*>(controller, fEventOwner);
    auto* action = kEventProcedure.is(owner) ? ref::rd<void*>(owner, fRunAction) : nullptr;
    if (!kScriptAction.is(action) || ref::rd_str_bounded(action, fScriptName, 48) != L"sc_e_0006_1" ||
        fieldnav::map_transition::is_screen_fading()) return {};
    return T(L"Cornelia prologue birds cue");
}
} // namespace ffpr::dialogue
