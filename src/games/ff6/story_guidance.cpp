// Native-owner boundary for FF6 story guidance. The rule model and all route
// searches use owned worker snapshots. Native hooks only mark cancellation or
// schedule one readiness continuation; they never wait for guide work.
#include "story_model.h"
#include "zone_eater_roof.h"
#include "owzer_doors.h"
#include "sealed_gate_bridges.h"
#include "palace/choices.h"
#include "serpent_choices.h"
#include <algorithm>
#include <iterator>
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/game.h"
#include "../../core/hooks.h"
#include "../../core/modtext.h"
#include "../../core/say.h"
#include "../../core/text.h"
#include "../../core/worker_job.h"
#include "../../core/field/field_internal.h"
#include "../../core/field/scene_events.h"
#include "../../core/field/map_names.h"
#include "../../core/fieldnav/audio_loops.h"
#include "../../core/ui/preferences.h"
#include "../../core/input/input.h"

namespace ffpr::ff6::story {
namespace {
ref::Class kUser{"Last.Management", "UserDataManager",game::kFF6};
ref::Method mParty{kUser,"set_CorpsSlotIndex",1};
ref::Class kSaveManager{"Last.Map","SaveMapManager",game::kFF6};
ref::Class kSaveData{"Last.Map","SaveMapData",game::kFF6};
ref::Class kPartyData{"Last.Map","SaveMapPartyData",game::kFF6};
ref::Method mSaveInstance{kSaveManager,"get_Instance",0};
ref::Field fSaveData{kSaveManager,"<SaveData>k__BackingField"};
ref::Field fOtherParties{kSaveData,"otherPartyDataList"};
ref::Field fPartyMap{kPartyData,"mapId"};
ref::Field fSavedPlayer{kPartyData,"<PlayerEntity>k__BackingField"},fSavedJson{kPartyData,"playerEntity"};
ref::Class kSavedEntity{"Last.Map","SaveEntityData",game::kFF6};
ref::Field fSavedPosition{kSavedEntity,"position"};
using Party=void(*)(void*,int32_t,const void*);
Party o_party=nullptr;
worker_job::Owner g_work;
uint64_t g_context=0;
bool g_active=false,g_refresh=false,g_party_cleanup=false;
int g_finish_on_exit=0;
std::vector<int> g_finish_through;
bool g_check_exit=false;
int g_finish_on_scenario=-1;
int g_slot=-1,g_pending_slot=-1,g_count=1;
std::optional<field::baked_nav::MapGoal> g_last_goal;
std::wstring g_repeat_instruction;
unsigned g_repeat_types=0;
field::puzzle_policy::Policy policy(){return {ui::prefs::navigation_puzzle_guidance(),ui::prefs::other_puzzle_guidance()};}
bool repeat_solution_goal(const field::baked_nav::MapGoal& goal) {
    return (goal.map_id==466&&goal.entity_id==0)||(goal.map_id==386&&goal.entity_id==11)||(goal.map_id==408&&goal.entity_id==34)||
        ((goal.map_id==431||goal.map_id==631)&&(goal.entity_id==155||goal.entity_id==11))||
        (goal.map_id==540&&goal.entity_id==2)||(goal.map_id==60&&goal.entity_id==1)||(goal.map_id==41&&goal.entity_id==12)||
        (goal.map_id==50&&(goal.entity_id==11||goal.entity_id==6))||(goal.map_id==52&&goal.entity_id==1)||
        (goal.map_id==48&&goal.entity_id==19)||(goal.map_id==218&&goal.entity_id==5)||(goal.map_id==88&&goal.entity_id==10)||
        (goal.map_id==377&&(goal.entity_id==1||goal.entity_id==2))||(goal.map_id==143&&goal.entity_id==61);
}
bool epitaph_repeat_available() {
    // Native tomb/clock/bridge/airship/coral choices own their focus. Repeat only the last
    // published instruction for these exact controls, never a route,
    // snapshot or remembered instruction from another field context.
    return g_active&&policy().allows_mask(g_repeat_types)&&!g_repeat_instruction.empty()&&g_last_goal&&
        repeat_solution_goal(*g_last_goal)&&field::current_map_id()==g_last_goal->map_id&&
        !queries::is("BattleStateHelper.IsInBattle")&&
        (queries::is("MessageChoice.IsOpen")||queries::is("MessageWindowPatches.IsInDialogue"));
}
int lethe_choice_option() {
    // Complete345/347 source has only these raft choices. Existing native
    // MessageChoice ownership clears its scalar on close/map/battle/title;
    // no new hook,pointer,cursor scan or per-frame observer is introduced.
    const int map=field::current_map_id();
    if((map!=345&&map!=347)||!queries::is("MessageChoice.IsOpen")||queries::is("BattleStateHelper.IsInBattle"))return 0;
    return map==345?1:2; // Yes/Straight;Left exits the Up loop on347.
}
std::array<std::optional<field::baked_nav::MapGoal>,3> g_party_goals;
struct RoofPending {
    field::baked_route::Cell start;
    field::baked_nav::GuideTarget target;
    std::vector<field::baked_route::Cell> cells;
    bool armed=false;
};
std::optional<RoofPending> g_roof_pending;
bool g_roof_resume=false;
bool g_owzer_pending=false,g_owzer_armed=false,g_owzer_posted=false;
bool g_owzer_available=false;
struct BridgePending {Plan::BridgeCrossing crossing;bool armed=false;};
std::optional<BridgePending> g_bridge_pending;
bool g_bridge_available=false,g_bridge_posted=false;
void cancel_owzer(){g_owzer_pending=g_owzer_armed=false;}
bool owzer_position() {
    field::baked_route::Cell at;
    return field::current_map_id()==445&&field::baked_nav::player_cell(field::map_handle(),field::player(),at)&&
        at==field::baked_route::Cell{1,23,23};
}

// Native SaveMapData.AddOtherPartyData indexes this array by the same zero-based
// corps slot used by SwitchCorps. Read at most three ready records, copy names,
// and discard every native pointer before submitting the guide worker. A missing
// slot is unknown; never invent a parked party or restore a stale mod position.
void capture_parties(Snapshot& snapshot) {
    if(snapshot.map!=318&&snapshot.map!=19)return;
    void* manager=ref::call_static<void*>(mSaveInstance);
    void* saved=ref::rd<void*>(manager,fSaveData);
    void* parties=ref::rd<void*>(saved,fOtherParties);
    const int length=unity::array_length(parties);
    if(length!=3)return;
    for(int i=0;i<3;++i)if(void* row=unity::array_at(parties,i)) {
        auto& party=snapshot.parties[static_cast<size_t>(i)];party.map=ref::rd<int32_t>(row,fPartyMap);
        if(party.map!=snapshot.map)continue;
        if(void* player=ref::rd<void*>(row,fSavedPlayer))party.position=ref::rd<unity::Vec3>(player,fSavedPosition);
        else party.serialized=ref::rd_str_bounded(row,fSavedJson,16384);
    }
}

bool ready() {
    return queries::is("GameStatePatches.IsFieldNavigationReady") &&
        !queries::is("GameStatePatches.IsInputTransitionBlocked") &&
        !queries::is("BattleStateHelper.IsInBattle") && !menu_state::any_active();
}
void next(bool explicit_request);
void stop(bool announce);
void release_roof(bool explicit_request);
void retire_preview() {
    if(g_last_goal)field::baked_nav::clear_preview_destination(*g_last_goal);
    g_last_goal.reset();
}
// One native event may produce several ready callbacks. Coalesce that pending
// observation, not speech text. A new explicit guide request always speaks.
void schedule_refresh() {
    if(g_refresh)return;
    g_refresh=true;
    frame::every_frame([] {
        if(!g_refresh)return false;
        if(!ready())return true;
        g_refresh=false;
        if(g_party_cleanup) {
            g_party_cleanup=false;
            field::baked_nav::clear_pending(true);
            field::baked_nav::invalidate();
            field::scanner::invalidate_map();
            actions::run("Autowalk.Cancel");
            actions::run("NavigationTargetTracker.Clear");
            ffpr::fieldnav::audio_loops::stop_all();
            ffpr::fieldnav::audio_loops::start_if_enabled();
        }
        if(g_check_exit) {
            g_check_exit=false;
            const int arrived=field::current_map_id();
            const bool completed=g_finish_on_exit>0&&arrived==g_finish_on_exit;
            // A long return can pass normal playable floors. Preserve only
            // explicitly reviewed continuations; every older immediate-exit
            // guide retains its wrong-exit cancellation behavior.
            if(completed||std::find(g_finish_through.begin(),g_finish_through.end(),arrived)==g_finish_through.end()) {
                g_finish_on_exit=0;g_finish_through.clear();
            }
            if(completed) {
                stop(false);
                ffpr::say(T(L"Puzzle guide finished"),false);
                return false;
            }
        }
        const auto flags=field::map_state::snapshot();
        // Native persistent receipts (Opera177, Dreamscape train449) survive
        // their battle/scene boundaries. Reuse this
        // existing ready-state snapshot; add no native read, hook or callback.
        if(g_finish_on_scenario>=0&&g_finish_on_scenario<1024&&
           flags.scenario[static_cast<size_t>(g_finish_on_scenario)]==1) {
            stop(false);ffpr::say(T(L"Puzzle guide finished"),false);return false;
        }
        g_count=flags.global[5].value_or(1);
        if(g_count>1&&g_pending_slot>=0&&g_pending_slot<3&&g_pending_slot!=g_slot) {
            if(g_slot>=0&&g_slot<3)g_party_goals[static_cast<size_t>(g_slot)]=std::move(g_last_goal);
            g_slot=g_pending_slot;
            g_last_goal=std::move(g_party_goals[static_cast<size_t>(g_slot)]);
            ffpr::say(TF(L"Party {0}",{text::num(g_slot+1)}),false);
        }
        g_pending_slot=-1;
        if(g_roof_resume) {
            g_roof_resume=false;
            if(g_active&&g_roof_pending&&g_roof_pending->armed)release_roof(false);
        }
        // Script/battle/party changes invalidate advice, but ordinary directions
        // are requested with Shift+G. Only the defense watcher speaks urgent alerts.
        return false;
    },"FF6.StoryGuide.Ready",INT_MAX);
}
void stop(bool announce) {
    g_active=false;++g_context;g_work.cancel();retire_preview();
    g_bridge_pending.reset();
    g_repeat_instruction.clear();
    g_repeat_types=0;
    cancel_owzer();
    g_roof_pending.reset();g_roof_resume=false;
    g_finish_on_exit=0;g_finish_through.clear();g_check_exit=false;g_finish_on_scenario=-1;
    for(auto& goal:g_party_goals)goal.reset();
    if(announce)ffpr::say(T(L"Puzzle guide stopped"),true);
}
bool roof_position(field::baked_route::Cell& out) {
    return field::current_map_id()==504&&field::baked_nav::player_cell(field::map_handle(),field::player(),out);
}
// Compare physical steps, never speech text. A changed native path needs a new
// preview; the short timed cue must never release different memorized directions.
bool roof_cells(const field::PathInfo& path,std::vector<field::baked_route::Cell>& out) {
    if(!path.success||path.walk_phases.empty()||path.walk_phases.size()>3)return false;
    for(const auto& phase:path.walk_phases) {
        if(phase.manual_after||phase.wait_for_npc||!phase.precision.empty()||!phase.action_before.empty()||!phase.action_after.empty()||
           phase.vehicle_before!=field::route_progress::VehicleAction::None||phase.manual_displacement_steps)return false;
        for(const auto& cell:phase.cells) {
            if(cell.layer!=1)return false;
            if(!out.empty()&&out.back()==cell)continue;
            if(out.size()>=128||(!out.empty()&&std::abs(out.back().x-cell.x)+std::abs(out.back().y-cell.y)!=1))return false;
            out.push_back(cell);
        }
    }
    return out.size()>1;
}
void roof_wait(bool interrupt) {ffpr::say(T(L"Wait here for Go now."),interrupt);}
void roof_changed(bool interrupt) {
    g_roof_pending.reset();g_roof_resume=false;
    ffpr::say(T(L"Your position changed. Request the roof guide again."),interrupt);
}
bool roof_attention() {
    return input::game_window_focused()&&!queries::is("ModMenu.IsOpen")&&!queries::is("ReviewBuffer.IsActive")&&
        !queries::is("TextInputWindow.IsOpen")&&!queries::is("ConfirmationDialog.IsOpen");
}
void release_roof(bool explicit_request) {
    if(!g_active||!g_roof_pending||!g_roof_pending->armed||!ready()||!ui::prefs::navigation_puzzle_guidance())return;
    if(!roof_attention()){g_roof_pending->armed=false;g_roof_resume=false;return;}
    field::baked_route::Cell at;
    if(!roof_position(at)||!(at==g_roof_pending->start)){roof_changed(explicit_request);return;}
    Snapshot timing;timing.map=504;capture_roof_timer(timing);
    if(!timing.roof_seconds) {
        g_roof_pending->armed=false;
        ffpr::say(T(L"Roof timing is not ready. Wait in a safe area, then request the guide again."),explicit_request);return;
    }
    if(!roof_fresh(timing)) {if(explicit_request)roof_wait(true);return;}
    const auto expected=*g_roof_pending;
    const auto context=++g_context;g_work.cancel();const double requested=frame::now();
    field::baked_nav::request_guidance({expected.target},[context,expected,explicit_request,requested](field::PathInfo path) {
        if(context!=g_context||!g_active||!ready()||!g_roof_pending||!ui::prefs::navigation_puzzle_guidance())return;
        const auto discard=[&] {field::baked_nav::clear_preview_destination(expected.target.goal);field::baked_nav::invalidate();};
        if(!roof_attention()){discard();g_roof_pending->armed=false;g_roof_resume=false;return;}
        field::baked_route::Cell here;
        if(!roof_position(here)||!(here==expected.start)){discard();roof_changed(true);return;}
        std::vector<field::baked_route::Cell> current;
        if(!roof_cells(path,current)||current!=expected.cells) {
            discard();g_roof_pending.reset();
            // Changed geometry is not a failure to memorize. Rebuild the route
            // through the normal explicit-preview path before another Go cue.
            next(true);return;
        }
        Snapshot now;now.map=504;capture_roof_timer(now);
        if(!roof_fresh(now)){discard();if(explicit_request)roof_wait(true);return;}
        g_last_goal=expected.target.goal;g_roof_pending.reset();g_roof_resume=false;
        ffpr::say(T(L"Go now."),true,{speech::Feature::Directions,requested,requested});
    });
}
void next(bool explicit_request) {
    if(!policy().any()){stop(false);if(explicit_request)ffpr::say(T(L"Puzzle guidance is off"),true);return;}
    if(const int option=lethe_choice_option()) {
        if(!ui::prefs::navigation_puzzle_guidance()){stop(false);ffpr::say(T(L"Navigation puzzle guidance is off"),explicit_request);return;}
        // One explicit single-bit read arms the first journey only. Replayed
        // native choices remain answerable without a premature completion.
        const auto completed=field::map_state::scenario_flag(50);stop(false);
        if(completed==0){g_active=true;g_finish_on_scenario=50;}
        ffpr::say(T(option==1?L"Choose the first option.":L"Choose the second option."),explicit_request);return;
    }
    if(serpent::choice_available()) {
        auto instruction=serpent::choice_instruction();
        if(ui::prefs::navigation_puzzle_guidance()){stop(false);g_active=true;}
        if(!instruction.empty())ffpr::say(instruction,explicit_request);
        return;
    }
    if(palace::choice_available()) {
        // Current native question outranks a remembered field objective. This
        // copies no actor/party snapshot and never changes the native choice.
        auto instruction=palace::choice_instruction();
        if(palace::choice_enabled()) {stop(false);g_active=true;g_finish_on_scenario=258;}
        if(!instruction.empty())ffpr::say(instruction,explicit_request);
        return;
    }
    if(epitaph_repeat_available()) {ffpr::say(g_repeat_instruction,explicit_request);return;}
    if(g_active&&g_bridge_pending) {
        field::baked_route::Cell at;
        if(roof_attention()&&field::current_map_id()==407&&
            field::baked_nav::player_cell(field::map_handle(),field::player(),at)&&
            !g_bridge_pending->crossing.cells.empty()&&at==g_bridge_pending->crossing.cells.front()) {
            g_bridge_pending->armed=true;ffpr::say(T(L"Wait here for Go now."),explicit_request);return;
        }
        g_bridge_pending.reset();
    }
    if(g_active&&g_owzer_pending) {
        if(ready()&&roof_attention()&&owzer_position()) {
            g_owzer_armed=true;ffpr::say(T(L"Wait here for Go now."),explicit_request);return;
        }
        cancel_owzer();
    }
    if(g_active&&g_roof_pending) {
        // Readiness can be confirmed while the native ceiling animation owns
        // input. Change only the mod's pending intent; observe no native actor.
        if(!ready()){g_roof_pending->armed=true;roof_wait(explicit_request);return;}
        field::baked_route::Cell at;
        if(roof_position(at)&&at==g_roof_pending->start) {
            g_roof_pending->armed=true;release_roof(explicit_request);return;
        }
        g_roof_pending.reset();g_roof_resume=false;
    }
    if(queries::is("GameModule.PuzzleGuideDuringEvent")) {
        if(!ui::prefs::other_puzzle_guidance()){stop(false);if(explicit_request)ffpr::say(T(L"Non-navigation puzzle guidance is off"),true);return;}
        g_active=true;++g_context;g_work.cancel();retire_preview();
        actions::run("Opera.Guide");return;
    }
    if(!ready())return;
    const double requested_at=frame::now();
    const int map=field::current_map_id();
    if(!supported_map(map)){stop(false);return;}
    if(!data::game_data_ready()||!field::baked_nav::prepared()) {
        if(explicit_request)ffpr::say(T(L"Navigation data is loading"));return;
    }
    g_repeat_instruction.clear();
    const bool starting=!g_active;
    Snapshot snapshot=capture_snapshot();
    if(snapshot.flags.global[4]&&*snapshot.flags.global[4]>=0&&*snapshot.flags.global[4]<3)g_slot=*snapshot.flags.global[4];
    g_count=snapshot.flags.global[5].value_or(1);
    g_active=true;const auto context=++g_context;g_work.cancel();
    const auto epoch=g_work.epoch();const auto generation=epoch->load();
    snapshot.cancelled=[epoch,generation]{return epoch->load()!=generation;};
    worker_job::start(g_work,std::move(snapshot),&plan,[context,explicit_request,requested_at,starting](Plan result) {
        if(context!=g_context||!g_active||!ready()||result.map!=field::current_map_id()||!policy().allows_mask(result.puzzle_types))return;
        if(!result.supported||result.finished) {
            stop(false);
            if(explicit_request&&!result.note.empty())ffpr::say(result.note,true);
            return;
        }
        // Only a source-verified final stage may arm completion. Native map
        // arrival, after field readiness, owns the actual completion message.
        g_finish_on_exit=result.finish_on_exit;
        g_finish_through=std::move(result.finish_through);
        g_finish_on_scenario=result.finish_on_scenario;
        std::wstring prefix;
        if(starting&&ui::prefs::puzzle_guide_verbose()&&(!result.bridge||!result.bridge->immediate)) {
            prefix=TF(L"Puzzle guide: {0}",{field::baked_nav::map_name_of(result.map)})+L". ";
            if(!result.introduction.empty())prefix+=result.introduction+L" ";
        }
        if(result.bridge) {
            retire_preview();g_bridge_pending.reset();
            field::baked_route::Cell at;
            if(!g_bridge_available||!roof_attention()||result.bridge->cells.size()<2||
                result.bridge->cells.size()>7||!field::baked_nav::player_cell(field::map_handle(),field::player(),at)||
                !(at==result.bridge->cells.front())) {
                ffpr::say(T(L"Story guidance is waiting for current game state."),explicit_request);return;
            }
            if(result.bridge->immediate) {
                int mode=0,phase=-1;
                if(!sealed_bridge_phase(mode,phase)||mode!=result.bridge->mode||phase!=result.bridge->phase)return;
            } else g_bridge_pending=BridgePending{std::move(*result.bridge)};
            ffpr::say(prefix+result.note,explicit_request,{speech::Feature::Directions,requested_at,requested_at});return;
        }
        if(result.targets.empty()) {
            retire_preview();
            if(result.owzer_door_wait) {
                cancel_owzer();
                if(!g_owzer_available||!roof_attention()||!owzer_position()) {
                    ffpr::say(T(L"Story guidance is waiting for current game state."),explicit_request);return;
                }
                g_owzer_pending=true;
            }
            if(!result.note.empty())ffpr::say(prefix+result.note,explicit_request,
                {speech::Feature::Directions,requested_at,requested_at});
            return;
        }
        // A release script can make the plate just left eligible again. Keep
        // the still-pending objective across that native refresh, or guidance
        // would send the same party straight back to the old plate.
        if(g_last_goal&&!result.defense) {
            const auto same=[&](const field::baked_nav::GuideTarget& target) {
                const auto& goal=target.goal;
                return goal.map_id==g_last_goal->map_id &&
                    (goal.entity_id>=0 ? goal.entity_id==g_last_goal->entity_id&&goal.raw_name==g_last_goal->raw_name : goal.cell==g_last_goal->cell);
            };
            const auto pending=std::find_if(result.targets.begin(),result.targets.end(),same);
            if(pending!=result.targets.end())std::rotate(result.targets.begin(),pending,std::next(pending));
        }
        auto goals=std::make_shared<std::vector<field::baked_nav::GuideTarget>>(result.targets);
        field::baked_nav::request_guidance(std::move(result.targets),[context,explicit_request,requested_at,types=result.puzzle_types,prefix=std::move(prefix),goals,hold_note=std::move(result.hold_note),roof_timed=result.roof_timed,roof_start=result.roof_start](field::PathInfo path) {
            if(context!=g_context||!g_active||!ready()||!policy().allows_mask(types))return;
            if(roof_timed) {
                const auto discard=[&] {
                    retire_preview();for(const auto& goal:*goals)field::baked_nav::clear_preview_destination(goal.goal);
                    field::baked_nav::invalidate();
                };
                if(!roof_attention()){discard();return;}
                field::baked_route::Cell here;std::vector<field::baked_route::Cell> cells;
                if(!roof_position(here)||!(here==roof_start)){discard();roof_changed(true);return;}
                if(path.guidance_index>=0&&path.guidance_index<static_cast<int>(goals->size())&&roof_cells(path,cells)) {
                    RoofPending pending;pending.start=here;pending.target=(*goals)[static_cast<size_t>(path.guidance_index)];pending.cells=std::move(cells);
                    discard();
                    g_roof_pending=std::move(pending);
                    ffpr::say(TF(L"Next safe area: {0}. Request the guide again when ready, then wait for Go now.",{path.description}),explicit_request,
                        {speech::Feature::Directions,requested_at,requested_at});return;
                }
                discard();
                ffpr::say(!path.reason.empty()?path.reason:T(L"Story guidance is waiting for current game state."),explicit_request,
                    {speech::Feature::Directions,requested_at,requested_at});return;
            }
            if(!path.success&&!hold_note.empty()) {
                retire_preview();
                ffpr::say(prefix+hold_note,explicit_request,{speech::Feature::Directions,requested_at,requested_at});
                return;
            }
            if(path.guidance_index>=0&&path.guidance_index<static_cast<int>(goals->size())) {
                g_last_goal=(*goals)[static_cast<size_t>(path.guidance_index)].goal;
                const auto& instruction=(*goals)[static_cast<size_t>(path.guidance_index)].instruction;
                if(path.success&&repeat_solution_goal(*g_last_goal)&&instruction.size()<=512)
                    {g_repeat_instruction=instruction;g_repeat_types=types;}
            }
            auto line=path.defense_warning;
            const auto append=[&](const std::wstring& text){if(text.empty())return;if(!line.empty())line+=L" ";line+=text;};
            append(prefix);
            if(path.success){append(path.description);append(path.guidance);}
            else append(!path.reason.empty()?path.reason:T(L"No path to the next step"));
            const speech::Timing timing{speech::Feature::Directions,requested_at,requested_at};
            if(ui::prefs::mod_warnings_enabled()&&!path.warning.empty())ffpr::say_sequence({path.warning,std::move(line)},explicit_request,timing);
            else ffpr::say(line,explicit_request,timing);
        },result.anticipated_release?&*result.anticipated_release:nullptr,result.defense?&*result.defense:nullptr);
    },background::Lane::Navigation);
}
void d_party(void* self,int32_t slot,const void* mi) {
    // This producer may run inside a map/battle continuation. Only epochs and
    // flags change before the original; all cleanup/readout waits for readiness.
    hooks::guarded("FF6.StoryGuide.PartyChanging",[] {
        ++g_context;g_work.interrupt();events::fire(events::Kind::PartyChanging);
    });
    o_party(self,slot,mi);
    hooks::guarded("FF6.StoryGuide.PartyChanged",[slot] {
        g_party_cleanup=true;g_pending_slot=slot;schedule_refresh();
    });
}
int install() {
    actions::define("GameModule.PuzzleGuidePolicyChanged",[]{stop(false);});
    actions::define("GameModule.PuzzleGuide",[]{hooks::guarded("FF6.StoryGuide.Next",[]{next(true);});});
    actions::define("GameModule.StopPuzzleGuide",[]{stop(true);});
    actions::define("GameModule.FieldStateChanged",[]{
        if(g_active){++g_context;g_work.interrupt();schedule_refresh();}
    });
    queries::define("GameModule.HasPuzzleGuide",[]{return lethe_choice_option()!=0||serpent::choice_available()||palace::choice_available()||queries::is("GameModule.PuzzleGuideDuringEvent")||supported_map(field::current_map_id());});
    queries::define("GameModule.PuzzleGuideActive",[]{return g_active;});
    queries::define("GameModule.PuzzleGuideRepeatDuringEvent",[]{return epitaph_repeat_available();});
    queries::define("GameModule.PuzzleGuideReadyDuringEvent",[]{return lethe_choice_option()!=0||serpent::choice_available()||palace::choice_available()||(g_active&&g_roof_pending.has_value());});
    events::on(events::Kind::MapChanging,[]{++g_context;g_work.interrupt();g_roof_pending.reset();g_roof_resume=false;g_repeat_instruction.clear();cancel_owzer();});
    events::on(events::Kind::BattleEntering,[]{++g_context;g_work.interrupt();g_finish_on_exit=0;g_finish_through.clear();g_check_exit=false;g_roof_pending.reset();g_roof_resume=false;cancel_owzer();});
    events::on(events::Kind::BattleEnd,[]{if(g_active)schedule_refresh();});
    events::on(events::Kind::MapChanged,[]{if(g_active){g_check_exit=g_finish_on_exit>0;schedule_refresh();}});
    events::on(events::Kind::SceneLoaded,[]{
        if(field::scene_name()!=L"Title")return;
        stop(false);g_slot=g_pending_slot=-1;g_refresh=g_party_cleanup=false;
        frame::cancel_label("FF6.StoryGuide.Ready");
    });
    events::on(events::Kind::MapChanging,[]{g_bridge_pending.reset();});
    events::on(events::Kind::BattleEntering,[]{g_bridge_pending.reset();});
    g_bridge_available=install_sealed_bridges(+[]{return g_active&&g_bridge_pending&&g_bridge_pending->armed;},+[](int mode,int phase){
        if(g_bridge_posted||!g_bridge_pending||mode!=g_bridge_pending->crossing.mode||phase!=g_bridge_pending->crossing.phase)return;
        g_bridge_posted=true;const auto context=g_context;const auto opened=frame::now();
        frame::next_frame([context,opened,mode,phase]{
            g_bridge_posted=false;
            if(context!=g_context||!g_active||!g_bridge_pending||!g_bridge_pending->armed||!ui::prefs::navigation_puzzle_guidance())return;
            field::baked_route::Cell at;int current_mode=0,current_phase=-1;
            if(!ready()||!roof_attention()||!field::baked_nav::player_cell(field::map_handle(),field::player(),at)||
                g_bridge_pending->crossing.cells.empty()||!(at==g_bridge_pending->crossing.cells.front())||
                !sealed_bridge_phase(current_mode,current_phase)||current_mode!=mode||current_phase!=phase) {
                g_bridge_pending.reset();return;
            }
            const auto age=frame::now()-opened;if(age<0||age>.25)return;
            g_bridge_pending.reset();ffpr::say(T(L"Go now."),true);
        },"FF6.SealedGate.Bridge");
    });
    const bool roof=install_roof_timer(+[]{g_roof_resume=true;schedule_refresh();},+[]{
        return g_active&&g_roof_pending&&g_roof_pending->armed&&field::current_map_id()==504;
    });
    g_owzer_available=install_owzer_doors(+[]{return g_active&&g_owzer_pending&&g_owzer_armed;},+[]{
        if(g_owzer_posted)return;
        g_owzer_posted=true;const auto context=g_context;const auto opened=frame::now();
        frame::next_frame([context,opened]{
            g_owzer_posted=false;
            if(context!=g_context||!g_active||!g_owzer_pending||!g_owzer_armed||!ui::prefs::navigation_puzzle_guidance())return;
            if(!ready()||!roof_attention()||!owzer_position()){cancel_owzer();return;}
            // A delayed scheduler must wait for another native opening. The
            // one-cell cue uses only the first quarter of the one-second window.
            const auto age=frame::now()-opened;if(age<0||age>.25)return;
            cancel_owzer();ffpr::say(T(L"Go now."),true);
        },"FF6.Owzer.LeftDoor");
    });
    return (hooks::attach(mParty,&d_party,&o_party)?1:0)+(roof?1:0)+(g_owzer_available?1:0)+(g_bridge_available?2:0)+palace::install_choices()+serpent::install_choices();
}
core::Install s_install{"FF6.StoryGuide",&install,515,game::kFF6};
}
Snapshot capture_snapshot() {
    Snapshot snapshot;snapshot.map=field::current_map_id();snapshot.flags=field::map_state::snapshot();snapshot.locale=modtext::lang();
    snapshot.guidance=policy();
    if(!field::baked_nav::player_cell(field::map_handle(),field::player(),snapshot.player))snapshot.map=0;
    capture_parties(snapshot);capture_roof_timer(snapshot);return snapshot;
}
}
