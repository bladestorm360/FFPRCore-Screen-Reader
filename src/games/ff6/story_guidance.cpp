// Native-owner boundary for FF6 story guidance. The rule model and all route
// searches use owned worker snapshots. Native hooks only mark cancellation or
// schedule one readiness continuation; they never wait for guide work.
#include "story_model.h"
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
int g_slot=-1,g_pending_slot=-1,g_count=1;
std::optional<field::baked_nav::MapGoal> g_last_goal;
std::array<std::optional<field::baked_nav::MapGoal>,3> g_party_goals;

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
        const auto flags=field::map_state::snapshot();
        g_count=flags.global[5].value_or(1);
        if(g_count>1&&g_pending_slot>=0&&g_pending_slot<3&&g_pending_slot!=g_slot) {
            if(g_slot>=0&&g_slot<3)g_party_goals[static_cast<size_t>(g_slot)]=std::move(g_last_goal);
            g_slot=g_pending_slot;
            g_last_goal=std::move(g_party_goals[static_cast<size_t>(g_slot)]);
            ffpr::say(TF(L"Party {0}",{text::num(g_slot+1)}),false);
        }
        g_pending_slot=-1;
        // Script/battle/party changes invalidate advice, but ordinary directions
        // are requested with Shift+G. Only the defense watcher speaks urgent alerts.
        return false;
    },"FF6.StoryGuide.Ready",INT_MAX);
}
void stop(bool announce) {
    g_active=false;++g_context;g_work.cancel();retire_preview();
    for(auto& goal:g_party_goals)goal.reset();
    if(announce)ffpr::say(T(L"Puzzle guide stopped"),true);
}
void next(bool explicit_request) {
    if(!ready())return;
    const double requested_at=frame::now();
    const int map=field::current_map_id();
    if(!supported_map(map)){stop(false);return;}
    if(!data::game_data_ready()||!field::baked_nav::prepared()) {
        if(explicit_request)ffpr::say(T(L"Navigation data is loading"));return;
    }
    Snapshot snapshot=capture_snapshot();
    if(snapshot.flags.global[4]&&*snapshot.flags.global[4]>=0&&*snapshot.flags.global[4]<3)g_slot=*snapshot.flags.global[4];
    g_count=snapshot.flags.global[5].value_or(1);
    g_active=true;const auto context=++g_context;g_work.cancel();
    worker_job::start(g_work,std::move(snapshot),&plan,[context,explicit_request,requested_at](Plan result) {
        if(context!=g_context||!g_active||!ready()||result.map!=field::current_map_id())return;
        if(!result.supported||result.finished) {
            stop(false);
            if(explicit_request&&!result.note.empty())ffpr::say(result.note,true);
            return;
        }
        std::wstring prefix;
        if(result.targets.empty()) {
            retire_preview();
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
        field::baked_nav::request_guidance(std::move(result.targets),[context,explicit_request,requested_at,prefix=std::move(prefix),goals,hold_note=std::move(result.hold_note)](field::PathInfo path) {
            if(context!=g_context||!g_active||!ready())return;
            if(!path.success&&!hold_note.empty()) {
                retire_preview();
                ffpr::say(prefix+hold_note,explicit_request,{speech::Feature::Directions,requested_at,requested_at});
                return;
            }
            if(path.guidance_index>=0&&path.guidance_index<static_cast<int>(goals->size()))
                g_last_goal=(*goals)[static_cast<size_t>(path.guidance_index)].goal;
            auto line=path.defense_warning;
            const auto append=[&](const std::wstring& text){if(text.empty())return;if(!line.empty())line+=L" ";line+=text;};
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
    actions::define("GameModule.PuzzleGuide",[]{hooks::guarded("FF6.StoryGuide.Next",[]{next(true);});});
    actions::define("GameModule.StopPuzzleGuide",[]{stop(true);});
    actions::define("GameModule.FieldStateChanged",[]{
        if(g_active){++g_context;g_work.interrupt();schedule_refresh();}
    });
    queries::define("GameModule.HasPuzzleGuide",[]{return supported_map(field::current_map_id());});
    queries::define("GameModule.PuzzleGuideActive",[]{return g_active;});
    events::on(events::Kind::MapChanging,[]{++g_context;g_work.interrupt();});
    events::on(events::Kind::BattleEntering,[]{++g_context;g_work.interrupt();});
    events::on(events::Kind::BattleEnd,[]{if(g_active)schedule_refresh();});
    events::on(events::Kind::MapChanged,[]{if(g_active)schedule_refresh();});
    events::on(events::Kind::SceneLoaded,[]{
        if(field::scene_name()!=L"Title")return;
        stop(false);g_slot=g_pending_slot=-1;g_refresh=g_party_cleanup=false;
        frame::cancel_label("FF6.StoryGuide.Ready");
    });
    return hooks::attach(mParty,&d_party,&o_party)?1:0;
}
core::Install s_install{"FF6.StoryGuide",&install,515,game::kFF6};
}
Snapshot capture_snapshot() {
    Snapshot snapshot;snapshot.map=field::current_map_id();snapshot.flags=field::map_state::snapshot();snapshot.locale=modtext::lang();
    if(!field::baked_nav::player_cell(field::map_handle(),field::player(),snapshot.player))snapshot.map=0;
    capture_parties(snapshot);return snapshot;
}
}
