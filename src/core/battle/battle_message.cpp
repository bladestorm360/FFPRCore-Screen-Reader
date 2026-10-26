// Ports FF1 Patches/BattleMessagePatches.cs: actor and action ("Gordan: Fire"), damage and
// healing, separately rendered hit counters (setting "MultiHitDamage"), misses, status added
// ("X: Poison"), status removed ("X: Poison removed"), and the game's battle system messages
// ("The party was defeated", "Preemptive strike!").
//
// Status removals observe the native function reconciliation in every game.
// Both bulk reconciliation and the direct RemoveFunction path are covered;
// persistent conditions do not require an Add announcement in this battle.
// Battle-end and KO/Stone cleanup retain the established silent policy.
// Exact ABI/phase evidence: docs/re/STATUS_REMOVALS_20261024.md.
//
// Per-game signatures: BattleBasicFunction.CreateDamageView has 4 parameters in FF1/FF3/FF4/FF6,
// 5 in FF2 (+bool isPlaySe) and FF5 (+MissType); BattleConditionController.Add has 2 in FF1-FF4 and
// FF6, 3 in FF5 (+bool isCallBack); BattleCommandMessageController.SetMessage (KeyInput) and
// SetSystemMessage (Touch) take (string message, bool isLeft) in FF6, (string) in FF1-FF5. Every
// hooked RVA is unique in every game.
//
// FF6 also keeps its C# mod's one existing check on the message line (BattleMenuController
// .SetCommadnMessage: Libra/Scan results repeat the same text 40+ times); see ff6_* below.
// FF6 adds HpAbs (8), MpAbs (9) and Golem (10). The shared resource producer
// preserves explicit MpAbs as MP; HP absorption/Golem keep ordinary damage feedback.
#include <algorithm>
#include <array>
#include <cstdint>
#include "../ui/preferences.h"
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "battle_internal.h"
#include "battle_message_line.h"
#include "rendered_feedback.h"
#include "special_feedback.h"
#include "stat_feedback.h"
#include "battle.h"
#include "battle_util.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../dedup.h"
#include "../frame.h"
#include "../game.h"
#include "../hooks.h"
#include "../modtext.h"
#include "../say.h"
#include "../settings.h"
#include "../text.h"
#include "../unity.h"
#include "../ui/preferences.h"

namespace ffpr::battle {
namespace {

constexpr uint8_t kFF134 = game::kFF1 | game::kFF3 | game::kFF4;
constexpr uint8_t kFF1234 = game::kFF1 | game::kFF2 | game::kFF3 | game::kFF4;
constexpr uint8_t kFF345 = game::kFF3 | game::kFF4 | game::kFF5;
constexpr uint8_t kFF12345 = kFF1234 | game::kFF5;
// FF6 shares FF1's CreateDamageView, Add and removal predicate (signatures checked in FF6's dump.cs).
constexpr uint8_t kFF1346 = kFF134 | game::kFF6;
constexpr uint8_t kFF12346 = kFF1234 | game::kFF6;
constexpr uint8_t kFF16 = game::kFF1 | game::kFF6;

ref::Class kActExec{"Last.Battle", "BattleActExection"};
ref::Method mInitActionState{kActExec, "InitActionState", 0};
ref::Method mEndFinishingState{kActExec, "EndFinishingState", 0};
ref::Field fInStagingActData{kActExec, "inStagingBattleActData"};
ref::Class kActData{"Last.Battle", "BattleActData"};
ref::Field fAttackUnitData{kActData, "<AttackUnitData>k__BackingField"};
ref::Field fStatTargets{kActData, "<TargetUnitDatas>k__BackingField"};

ref::Class kBasicFn{"Last.Battle.Function", "BattleBasicFunction", kFF1346};
ref::Method mCreateDamageView{kBasicFn, "CreateDamageView", 4};
ref::Class kBasicFnFF2{"Last.Battle.Function", "BattleBasicFunction", game::kFF2};
ref::Method mCreateDamageViewFF2{kBasicFnFF2, "CreateDamageView", 5};
ref::Class kBasicFnFF5{"Last.Battle.Function", "BattleBasicFunction", game::kFF5};
ref::Method mCreateDamageViewFF5{kBasicFnFF5, "CreateDamageView", 5};

ref::Class kDamageViewUI{"Last.UI", "DamageViewUIManager"};
ref::Method mCreateHitCount{kDamageViewUI, "CreateHitCount", 3};
ref::Class kViewDamage12345{"Last.Battle", "ViewDamageEntity", kFF12345};
ref::Method mCreateDamageValue{kViewDamage12345, "CreateDamageValue", 4};
ref::Class kViewDamage6{"Last.Battle", "ViewDamageEntity", game::kFF6};
ref::Method mCreateDamageValue6{kViewDamage6, "CreateDamageValue", 5};
ref::Class kBattleUtility{"Last.Battle", "BattleUtility", kFF1346 | game::kFF5};
ref::Method mUtilityDamage{kBattleUtility, "CreateDamageView", 4};
ref::Class kBattleUtility2{"Last.Battle", "BattleUtility", game::kFF2};

ref::Class kCmdMsgKey{"Last.UI.KeyInput", "BattleCommandMessageController", kFF12345};
ref::Method mSetMessage{kCmdMsgKey, "SetMessage", 1};
ref::Class kCmdMsgTouch{"Last.UI.Touch", "BattleCommandMessageController", kFF12345};
ref::Method mSetSystemMessage{kCmdMsgTouch, "SetSystemMessage", 1};
ref::Class kCmdMsgKeyFF6{"Last.UI.KeyInput", "BattleCommandMessageController", game::kFF6};
ref::Method mSetMessageFF6{kCmdMsgKeyFF6, "SetMessage", 2};
ref::Class kCmdMsgTouchFF6{"Last.UI.Touch", "BattleCommandMessageController", game::kFF6};
ref::Method mSetSystemMessageFF6{kCmdMsgTouchFF6, "SetSystemMessage", 2};
ref::Class kBattleMenuKeyFF6{"Last.UI.KeyInput", "BattleMenuController", game::kFF6};
ref::Method mSetCommadnMessageFF6{kBattleMenuKeyFF6, "SetCommadnMessage", 2};

ref::Class kCondCtl{"Last.Battle", "BattleConditionController"};
ref::Method mBattleEndRecoveryCondition{kCondCtl, "BattleEndRecoveryCondition", 0};
ref::Class kCondCtl12346{"Last.Battle", "BattleConditionController", kFF12346};
ref::Method mAdd{kCondCtl12346, "Add", 2};
ref::Class kCondCtlFF5{"Last.Battle", "BattleConditionController", game::kFF5};
ref::Method mAddFF5{kCondCtlFF5, "Add", 3};
ref::Method mRemoveFunction{kCondCtl, "RemoveFunction", 2};
ref::Method mRemoveConditionFunction{kCondCtl, "RemoveConditionFunction", 3};
ref::Class kStatusCtlFF2{"Last.Battle", "BattleStatusControl", game::kFF2};
ref::Method mUpdateTempParamter{kStatusCtlFF2, "UpdateTempParamter", 1};
ref::Class kBattleCtlFF5{"Last.Battle", "BattleController", game::kFF5};
ref::Method mStateChange{kBattleCtlFF5, "StateChange", 1};

// HitType values (identical in all five dump.cs).
constexpr int kHitMiss = 2;
constexpr int kHitZero = 3;         // a genuine 0-value result
constexpr int kHitMpHit = 5;        // MP damage
constexpr int kHitMpRecovery = 6;   // MP recovery

// Context is carried through the native call stack, never through a global frame-wide
// multiplier or HP-difference inference. The innermost renderer owns the spoken event.
struct DamageContext { void* unit = nullptr; int type = -1; };
thread_local DamageContext g_damage_context;
struct DamageScope {
    DamageContext previous = g_damage_context;
    ~DamageScope() { g_damage_context = previous; }
};

// --- status removal state ---------------------------------------------------------------------
// True during the battle-end cleanup. FF1 and FF6: only while BattleEndRecoveryCondition runs.
// FF2-FF5: from BattleEndRecoveryCondition (FF5 also StateChange into an end state) until the next
// battle starts.
bool g_battle_end_cleanup = false;
// Same-frame (unit, condition) guard.
int64_t g_spoken_frame = -1;
std::set<std::pair<void*, int>> g_spoken_this_frame;

bool first_this_frame(void* unit, int id)
{
    int64_t frame_now = static_cast<int64_t>(frame::count());
    if (frame_now != g_spoken_frame) {
        g_spoken_frame = frame_now;
        g_spoken_this_frame.clear();
    }
    return g_spoken_this_frame.insert({unit, id}).second;
}

void speak_removed(void* unit, const std::wstring& condition)
{
    ffpr::say(TF(L"{0}: {1} removed", {unit_name(unit), condition}), false);
}

// --- The action line: game text first (user rule, 2026-10-06) -------------------------------------
// "Read game results where available; keep mod-side results only for information the game shows
// as icons or pictures." When an action starts, the game writes its name in the battle message box
// (BattleCommandMessageController.SetMessage: "Steal", "Fire"), and the mod's InitActionState hook
// knows the actor, which the game shows only by animation. Speaking both doubled every spell and
// ability ("Steal", then "Edge: Steal"). So the two are paired: whichever arrives first waits up to
// kPairFrames for the other, and one line is spoken - the actor (picture-only information) plus
// the GAME's text. An action the game names nowhere (a basic attack) keeps the mod's own
// "Actor: Action"; a game message with no action starting (results, "Stole Potion!",
// "Couldn't escape") is spoken as the game wrote it. This pairs two halves of one event; it is
// not a text comparison.
using message_line::action_started;
using message_line::game_message;

// --- InitActionState (postfix): the action's actor ------------------------------------------------
using Void_t = void (*)(void* self, const void* mi);
Void_t o_InitActionState;

void d_InitActionState(void* self, const void* mi)
{
    timing::mark("Battle action begin");
    { timing::Scope native("Battle InitActionState",timing::Kind::Original);
      o_InitActionState(self, mi); }
    hooks::guarded("BattleActExection.InitActionState", [&] {
        if (!self) return;
        void* act = ref::rd<void*>(self, fInStagingActData);
        if (!act) return;
        void* unit = ref::rd<void*>(act, fAttackUnitData);
        action_actor_started(unit);
        std::wstring actor = unit_name(unit);
        stat_feedback::observe_action(unit,actor,ref::rd<void*>(act,fStatTargets));
        action_started(actor, action_name(act));
    });
    timing::mark("Battle action initialized");
}
Void_t o_EndFinishingState;
void d_EndFinishingState(void* self, const void* mi)
{
    // Exact native action completion catches unique/inlined base-stat writes too.
    // Its currently rooted participants are still alive before native teardown.
    hooks::guarded("Battle.StatActionComplete",[&] {
        void* act=self?ref::rd<void*>(self,fInStagingActData):nullptr;
        if(act)stat_feedback::finish_action(ref::rd<void*>(act,fAttackUnitData),ref::rd<void*>(act,fStatTargets));
    });
    // Finish before native completion callbacks can initialize the next command/action.
    hooks::guarded("Battle.ActionFinished", [] { action_actor_finished(); });
    { timing::Scope native("Battle EndFinishingState",timing::Kind::Original);
      o_EndFinishingState(self, mi); }
    timing::mark("Battle action finished");
}

// --- CreateDamageView (postfix): damage, healing, misses -----------------------------------------
void damage_view(const std::wstring& target, int value, int hit_type, bool is_recovery)
{
    const auto prefix = target.empty() ? std::wstring() : target + L": ";
    std::wstring msg;
    if (hit_type == kHitMiss) {
        msg = prefix + T(L"Miss");
    } else if (hit_type == kHitMpRecovery) {
        msg = prefix + TF(L"Recovered {0} MP", {text::num(value)});
    } else if (hit_type == kHitMpHit) {
        msg = prefix + TF(L"{0} MP damage", {text::num(value)});
    } else if (is_recovery) {
        // The innermost renderer flag controls the displayed recovery colour.
        // A healing function's nominal Recovery hit type can still render HP
        // damage (Cure against undead); it must not override that presentation.
        msg = prefix + TF(L"Recovered {0} HP", {text::num(value)});
    } else {
        msg = prefix + TF(L"{0} damage", {text::num(value)});
    }
    // Damage/healing doesn't interrupt: it queues after the action announcement.
    ffpr::say(msg, false);
}

using DamageView_t = void (*)(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, const void* mi);
DamageView_t o_CreateDamageView;
void d_CreateDamageView(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, const void* mi)
{
    DamageScope scope;
    g_damage_context = {data, special_feedback::owned_damage_type(self,hit_type,is_recovery)};
    timing::Scope native("Battle CreateDamageView",timing::Kind::Original);
    o_CreateDamageView(self, data, value, hit_type, is_recovery, mi);
}

using DamageViewFF2_t = void (*)(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, bool is_play_se, const void* mi);
DamageViewFF2_t o_CreateDamageViewFF2;
void d_CreateDamageViewFF2(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, bool is_play_se, const void* mi)
{
    DamageScope scope;
    g_damage_context = {data, special_feedback::owned_damage_type(self,hit_type,is_recovery)};
    timing::Scope native("Battle CreateDamageView FF2",timing::Kind::Original);
    o_CreateDamageViewFF2(self, data, value, hit_type, is_recovery, is_play_se, mi);
}

using DamageViewFF5_t = void (*)(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, int32_t miss_type, const void* mi);
DamageViewFF5_t o_CreateDamageViewFF5;
void d_CreateDamageViewFF5(void* self, void* data, int32_t value, int32_t hit_type, bool is_recovery, int32_t miss_type, const void* mi)
{
    DamageScope scope;
    g_damage_context = {data, special_feedback::owned_damage_type(self,hit_type,is_recovery)};
    timing::Scope native("Battle CreateDamageView FF5",timing::Kind::Original);
    o_CreateDamageViewFF5(self, data, value, hit_type, is_recovery, miss_type, mi);
}

using UtilityDamage = void* (*)(void*, int32_t, bool, bool, const void*);
UtilityDamage o_utility_damage;
void* d_utility_damage(void* unit, int32_t value, bool recovery, bool miss, const void* mi)
{
    DamageScope scope;
    g_damage_context.unit = unit;
    return o_utility_damage(unit, value, recovery, miss, mi);
}
using UtilityDamage2 = void* (*)(void*, int32_t, bool, bool, bool, const void*);
UtilityDamage2 o_utility_damage2;
void* d_utility_damage2(void* unit, int32_t value, bool recovery, bool miss, bool se, const void* mi)
{
    DamageScope scope;
    g_damage_context.unit = unit;
    return o_utility_damage2(unit, value, recovery, miss, se, mi);
}
using RenderDamage = void (*)(void*, void*, int32_t, bool, bool, const void*);
RenderDamage o_render_damage;
void rendered_damage(void* self, bool recovery, bool miss)
{
    hooks::guarded("Battle.DisplayedDamage", [=] {
        const auto name = unit_name(g_damage_context.unit);
        const int type = miss ? kHitMiss : special_feedback::damage_type(g_damage_context.type,recovery);
        rendered::damage(self, miss, [name, type, recovery](int digits) {
            damage_view(name, digits, type < 0 ? kHitZero : type, recovery);
        });
    });
}
void d_render_damage(void* self, void* target, int32_t value, bool recovery, bool miss, const void* mi)
{
    o_render_damage(self, target, value, recovery, miss, mi);
    rendered_damage(self, recovery, miss);
}
using RenderDamage6 = void (*)(void*, void*, int32_t, unity::Vec2, bool, bool, const void*);
RenderDamage6 o_render_damage6;
void d_render_damage6(void* self, void* target, int32_t value, unity::Vec2 offset, bool recovery, bool miss, const void* mi)
{
    o_render_damage6(self, target, value, offset, recovery, miss, mi);
    rendered_damage(self, recovery, miss);
}

// --- CreateHitCount (postfix): read the visible digit counter --------------------------------------------
using CreateHitCount_t = void* (*)(void* self, int32_t hit_count_value, void* attack, void* target, const void* mi);
CreateHitCount_t o_CreateHitCount;
void* d_CreateHitCount(void* self, int32_t hit_count_value, void* attack, void* target, const void* mi)
{
    void* result = o_CreateHitCount(self, hit_count_value, attack, target, mi);
    hooks::guarded("Battle.RenderedHitCount", [result] { rendered::hit_count(result); });
    return result;
}

// --- BattleCommandMessageController.SetMessage / SetSystemMessage (postfix) ----------------------
void battle_system_message(void* message_str)
{
    std::wstring message_text = ref::str(message_str);
    if (message_text.empty()) return;
    std::wstring clean = strip_icon(message_text);
    clean = text::replace_all(clean, L"\n", L" ");
    clean = text::replace_all(clean, L"\r", L" ");
    clean = text::trim(clean);
    while (text::contains(clean, L"  ")) clean = text::replace_all(clean, L"  ", L" ");
    if (clean.empty()) return;
    // The defeat message interrupts.
    bool is_defeat = text::contains(text::lower(clean), L"defeated");
    game_message(clean, is_defeat);
}

using SetMessage_t = void (*)(void* self, void* message_str, const void* mi);
SetMessage_t o_SetMessage;
void d_SetMessage(void* self, void* message_str, const void* mi)
{
    o_SetMessage(self, message_str, mi);
    hooks::guarded("BattleCommandMessageController.SetMessage", [&] { battle_system_message(message_str); });
}

SetMessage_t o_SetSystemMessage;
void d_SetSystemMessage(void* self, void* message_str, const void* mi)
{
    o_SetSystemMessage(self, message_str, mi);
    hooks::guarded("BattleCommandMessageController.SetSystemMessage", [&] { battle_system_message(message_str); });
}

// --- FF6: the same two hooks, (string message, bool isLeft) --------------------------------------
// In FF6 the KeyInput SetMessage has exactly two callers (GameAssembly.dll call sites, 2026-10-07):
// BattleMenuController.SetCommadnMessage (BattleUIManager.SetCommadnMessage / SetSystemMessage:
// "Stole X!", Libra results, system lines) and BattleMenuController.SetCommandText (BattleUIManager
// .SetCommandText: the action's name). FF6's C# mod hooked those two; hooking SetMessage covers both.
//
// FF6's C# check on the SetCommadnMessage path (BattleMenuController_KeyInput_SetCommadnMessage_Patch),
// kept as it was: Libra/Scan call it with the same text 40+ times, so the same text within 2.5 s is
// dropped, and an empty message (the box cleared) forgets the last one. The SetCommadnMessage
// wrapper below marks which SetMessage calls came through that path; the action-name path
// (SetCommandText) is unchecked, as in the core. Its other check (SetCommandText never repeating
// the previous text) is not kept: the core pairs the action name with its actor instead.
constexpr const char* kCtxFF6CommandMessage = "Battle.SetCommandMessage";
constexpr double kFF6ThrottleSeconds = 2.5;   // MESSAGE_THROTTLE_SECONDS
bool g_ff6_in_command_message = false;
double g_ff6_last_message_time = -1.0e9;      // lastMessageTime

/// True when this message (arriving through SetCommadnMessage) is to be spoken.
bool ff6_command_message_is_new(void* message_str)
{
    const std::wstring message = text::trim(ref::str(message_str));
    if (blank(message)) {
        // Message cleared: reset tracking.
        dedup::reset(kCtxFF6CommandMessage);
        g_ff6_last_message_time = -1.0e9;
        return false;
    }
    const double now = frame::now();
    if (now - g_ff6_last_message_time >= kFF6ThrottleSeconds) dedup::reset(kCtxFF6CommandMessage);
    if (!dedup::text(kCtxFF6CommandMessage, message, dedup::Scope::Battle)) return false;
    g_ff6_last_message_time = now;
    return true;
}

using SetMessageFF6_t = void (*)(void* self, void* message_str, bool is_left, const void* mi);
SetMessageFF6_t o_SetMessageFF6;
void d_SetMessageFF6(void* self, void* message_str, bool is_left, const void* mi)
{
    o_SetMessageFF6(self, message_str, is_left, mi);
    hooks::guarded("BattleCommandMessageController.SetMessage", [&] {
        if (g_ff6_in_command_message && !ff6_command_message_is_new(message_str)) return;
        battle_system_message(message_str);
    });
}

SetMessageFF6_t o_SetSystemMessageFF6;
void d_SetSystemMessageFF6(void* self, void* message_str, bool is_left, const void* mi)
{
    o_SetSystemMessageFF6(self, message_str, is_left, mi);
    hooks::guarded("BattleCommandMessageController.SetSystemMessage", [&] { battle_system_message(message_str); });
}

/// Restores the path flag however the original returns (a managed exception unwinds under /EHa).
struct CommandMessageScope {
    bool outer;
    CommandMessageScope() : outer(g_ff6_in_command_message) { g_ff6_in_command_message = true; }
    ~CommandMessageScope() { g_ff6_in_command_message = outer; }
    CommandMessageScope(const CommandMessageScope&) = delete;
    CommandMessageScope& operator=(const CommandMessageScope&) = delete;
};

// KeyInput BattleMenuController.SetCommadnMessage: only marks the path; SetMessage speaks.
SetMessageFF6_t o_SetCommadnMessageFF6;
void d_SetCommadnMessageFF6(void* self, void* message_str, bool is_left, const void* mi)
{
    CommandMessageScope scope;
    o_SetCommadnMessageFF6(self, message_str, is_left, mi);
}

// --- BattleConditionController.Add (postfix): "X: Poison" -----------------------------------------
void condition_added(void* unit, int id)
{
    if (!unit) return;
    // The condition from the unit's confirmed list.
    void* confirmed = nullptr;
    if (void* param = unit_parameter(unit)) {
        for (void* c : confirmed_conditions(param)) {
            if (c && condition_id(c) == id) {
                confirmed = c;
                break;
            }
        }
    }
    int type=0;
    std::wstring name = resolve_condition_name(confirmed, id, &type);
    // Generic enemy art has no critical-health pose/icon. This separate,
    // default-off disclosure never mutes party critical health or critical hits.
    if(type==4&&kBattleEnemyData.is(unit)&&!ui::prefs::enemy_critical_status_enabled())return;
    if (stat_feedback::announce_condition(unit,name,type)) return;
    if (!name.empty()) ffpr::say(unit_name(unit) + L": " + name, false);
}

using Add_t = void (*)(void* self, void* unit, int32_t id, const void* mi);
Add_t o_Add;
void d_Add(void* self, void* unit, int32_t id, const void* mi)
{
    stat_feedback::AddScope applied(unit);
    o_Add(self, unit, id, mi);
    hooks::guarded("BattleConditionController.Add", [&] { condition_added(unit, id); });
}

using AddFF5_t = void (*)(void* self, void* unit, int32_t id, bool is_call_back, const void* mi);
AddFF5_t o_AddFF5;
void d_AddFF5(void* self, void* unit, int32_t id, bool is_call_back, const void* mi)
{
    stat_feedback::AddScope applied(unit);
    o_AddFF5(self, unit, id, is_call_back, mi);
    hooks::guarded("BattleConditionController.Add", [&] { condition_added(unit, id); });
}

// --- BattleEndRecoveryCondition (prefix, FF1 also postfix) ---------------------------------------
Void_t o_BattleEndRecoveryCondition;
void d_BattleEndRecoveryCondition(void* self, const void* mi)
{
    g_battle_end_cleanup = true;
    stat_feedback::cleanup();
    o_BattleEndRecoveryCondition(self, mi);
    // FF1 and FF6: the scope ends with the cleanup. FF2-FF5 keep the latch until the next battle starts.
    if (game::in(kFF16)) g_battle_end_cleanup = false;
}

// --- Native function removal: direct and bulk, all six games -----------------------
// Recovery functions first edit CurrentConditionList, then native reconciliation
// removes the obsolete BattleConditionFunctions. Capture the live old functions
// on this stack, rather than relying on an Add seen in this battle. Master
// Condition objects are immutable/rooted by the master manager throughout this
// synchronous call; no function/condition pointer is retained for later frames.
constexpr int kRemovalLimit = 128;
struct RemovalSnapshot {
    struct Entry { void* condition = nullptr; int id = 0; int type = 0; };
    std::array<Entry,kRemovalLimit> entries{};
    int count = 0;
    void* parameter = nullptr;
    bool valid = false;
};
RemovalSnapshot removal_before(void* unit)
{
    RemovalSnapshot result;
    if (!unit || g_battle_end_cleanup || !is_in_battle()) return result;
    result.parameter = unit_parameter(unit);
    void* list = info_functions(unit_info(unit));
    const int count = unity::list_count(list);
    if (!result.parameter || count < 0 || count > kRemovalLimit) return result;
    for (int i=0;i<count;++i) {
        void* c = function_condition(unity::list_at(list,i));
        if (c) result.entries[result.count++] = {c,condition_id(c),condition_type(c)};
    }
    result.valid = true;
    return result;
}
void removal_after(void* unit, const RemovalSnapshot& before)
{
    if (!before.valid || g_battle_end_cleanup || !is_in_battle() ||
        unit_parameter(unit) != before.parameter) return;
    void* functions = info_functions(unit_info(unit));
    void* current = current_condition_list(before.parameter);
    const int nf=unity::list_count(functions), nc=unity::list_count(current);
    if (nf<0 || nf>kRemovalLimit || nc<0 || nc>kRemovalLimit) return;
    std::array<int,kRemovalLimit*2> remaining{}; int count=0;
    bool incapacitated=false;
    for (int i=0;i<nf;++i) if (void* c=function_condition(unity::list_at(functions,i)))
        remaining[count++]=condition_id(c);
    for (int i=0;i<nc;++i) if (void* c=unity::list_at(current,i)) {
        remaining[count++]=condition_id(c);
        const int type=condition_type(c);
        incapacitated |= type==kConditionTypeKO || type==kConditionTypeStone;
    }
    for (int i=0;i<before.count;++i) {
        const auto& entry=before.entries[i];
        if(entry.type==4&&kBattleEnemyData.is(unit)&&!ui::prefs::enemy_critical_status_enabled())continue;
        if (std::find(remaining.begin(),remaining.begin()+count,entry.id)!=remaining.begin()+count) continue;
        // Keep native stacked/permanent conditions and prior KO/Stone cleanup
        // policy. A successful revive can still announce KO removed.
        if (incapacitated && entry.type!=kConditionTypeKO && entry.type!=kConditionTypeStone) continue;
        if (entry.type==kConditionTypeKO && param_current_hp(before.parameter)<=0) continue;
        const auto name=resolve_condition_name(entry.condition,entry.id);
        if (name.empty() || !first_this_frame(unit,entry.id)) continue;
        speak_removed(unit,name.substr(0,256));
    }
}
using RemoveFunction_t = void (*)(void*,void*,int32_t,const void*);
RemoveFunction_t o_RemoveFunction;
void d_RemoveFunction(void* self,void* unit,int32_t id,const void* mi)
{
    RemovalSnapshot before;
    hooks::guarded("Battle.RemovalBefore",[&]{before=removal_before(unit);});
    o_RemoveFunction(self,unit,id,mi);
    hooks::guarded("Battle.RemovalAfter",[&]{removal_after(unit,before);});
}
using RemoveConditions_t = void (*)(void*,void*,void*,void*,const void*);
RemoveConditions_t o_RemoveConditionFunction;
void d_RemoveConditionFunction(void* self,void* unit,void* conditions,void* functions,const void* mi)
{
    RemovalSnapshot before;
    hooks::guarded("Battle.RemovalsBefore",[&]{before=removal_before(unit);});
    o_RemoveConditionFunction(self,unit,conditions,functions,mi);
    hooks::guarded("Battle.RemovalsAfter",[&]{removal_after(unit,before);});
}

// --- FF5: BattleController.StateChange (prefix) ---------------------------------------------------
// Init (1) .. Event (6) are a live battle; WinWait (7) .. End (20) are victory, defeat, escape and
// their fades. Transitions only.
using StateChange_t = void (*)(void* self, int32_t state, const void* mi);
StateChange_t o_StateChange;
void d_StateChange(void* self, int32_t state, const void* mi)
{
    if (state >= 1 && state <= 6) {
        g_battle_end_cleanup = false;
        g_spoken_this_frame.clear();
    } else if (state >= 7 && state <= 20) {
        g_battle_end_cleanup = true;
    }
    o_StateChange(self, state, mi);
}

// FFII's existing stat-refresh adapter; removal is owned by native function
// reconciliation above, independently of Add history or temporary-stat refresh.
using UpdateTemp_t = void (*)(void* self, void* unit, const void* mi);
UpdateTemp_t o_UpdateTempParamter;
void d_UpdateTempParamter(void* self, void* unit, const void* mi)
{
    stat_feedback::Snapshot before;
    hooks::guarded("Battle.StatBefore", [&] { before=stat_feedback::capture(unit); });
    o_UpdateTempParamter(self, unit, mi);
    hooks::guarded("Battle.StatAfter", [&] { stat_feedback::updated(unit,before); });
}

int install()
{
    events::on(events::Kind::BattleEnd, [] {
        frame::cancel_label("Battle.HitCount");
        frame::cancel_label("Battle.DamageDisplay");
    });
    int n = 0;
    n += hooks::attach(mInitActionState, &d_InitActionState, &o_InitActionState) ? 1 : 0;
    n += hooks::attach(mEndFinishingState, &d_EndFinishingState, &o_EndFinishingState) ? 1 : 0;
    if (game::in(kFF1346)) n += hooks::attach(mCreateDamageView, &d_CreateDamageView, &o_CreateDamageView) ? 1 : 0;
    if (game::in(game::kFF2)) n += hooks::attach(mCreateDamageViewFF2, &d_CreateDamageViewFF2, &o_CreateDamageViewFF2) ? 1 : 0;
    if (game::in(game::kFF5)) n += hooks::attach(mCreateDamageViewFF5, &d_CreateDamageViewFF5, &o_CreateDamageViewFF5) ? 1 : 0;
    n += hooks::attach(mCreateHitCount, &d_CreateHitCount, &o_CreateHitCount) ? 1 : 0;
    n += hooks::attach(mCreateDamageValue, &d_render_damage, &o_render_damage) ? 1 : 0;
    n += hooks::attach(mCreateDamageValue6, &d_render_damage6, &o_render_damage6) ? 1 : 0;
    if (game::is(game::Id::FF2)) {
        auto* method = find_method_param_type(kBattleUtility2.get(), "CreateDamageView", 5, 0, "BattleUnitData");
        n += hooks::attach_addr(il2cpp::method_pointer(method), "BattleUtility.CreateDamageView.unit", reinterpret_cast<void*>(&d_utility_damage2), reinterpret_cast<void**>(&o_utility_damage2)) ? 1 : 0;
    } else n += hooks::attach(mUtilityDamage, &d_utility_damage, &o_utility_damage) ? 1 : 0;
    if (game::in(kFF12345)) {
        n += hooks::attach(mSetMessage, &d_SetMessage, &o_SetMessage) ? 1 : 0;
        n += hooks::attach(mSetSystemMessage, &d_SetSystemMessage, &o_SetSystemMessage) ? 1 : 0;
    }
    if (game::in(game::kFF6)) {
        n += hooks::attach(mSetMessageFF6, &d_SetMessageFF6, &o_SetMessageFF6) ? 1 : 0;
        n += hooks::attach(mSetSystemMessageFF6, &d_SetSystemMessageFF6, &o_SetSystemMessageFF6) ? 1 : 0;
        n += hooks::attach(mSetCommadnMessageFF6, &d_SetCommadnMessageFF6, &o_SetCommadnMessageFF6) ? 1 : 0;
    }

    if (game::in(kFF12346)) n += hooks::attach(mAdd, &d_Add, &o_Add) ? 1 : 0;
    if (game::in(game::kFF5)) n += hooks::attach(mAddFF5, &d_AddFF5, &o_AddFF5) ? 1 : 0;
    n += hooks::attach(mBattleEndRecoveryCondition, &d_BattleEndRecoveryCondition, &o_BattleEndRecoveryCondition) ? 1 : 0;

    n += hooks::attach(mRemoveFunction, &d_RemoveFunction, &o_RemoveFunction) ? 1 : 0;
    n += hooks::attach(mRemoveConditionFunction, &d_RemoveConditionFunction, &o_RemoveConditionFunction) ? 1 : 0;
    if (game::in(game::kFF2)) n += hooks::attach(mUpdateTempParamter, &d_UpdateTempParamter, &o_UpdateTempParamter) ? 1 : 0;
    if (game::in(game::kFF5)) n += hooks::attach(mStateChange, &d_StateChange, &o_StateChange) ? 1 : 0;
    return n;
}
core::Install s_install{"Battle.Message", &install, 110};

} // namespace

void removal_reset_state()
{
    frame::cancel_label("Battle.HitCount");
    frame::cancel_label("Battle.DamageDisplay");
    // Battle start: a missed postfix can never silence the next battle.
    g_battle_end_cleanup = false;
    g_spoken_this_frame.clear();
    g_spoken_frame = -1;

}

} // namespace ffpr::battle
