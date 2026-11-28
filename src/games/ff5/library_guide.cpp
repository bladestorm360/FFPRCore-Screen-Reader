// One FFV native guide owner. Each leaf captures only its bounded story bits,
// controls and position dependencies at explicit/native-ready boundaries.
// Policy/target work and routing stay on Navigation; movement never scans actors.
#include "library_model.h"
#include "catapult_model.h"
#include "stone_model.h"
#include "book_model.h"
#include "pyramid_model.h"
#include "jachol_model.h"
#include "mechanics_model.h"
#include "ff5_internal.h"
#include <algorithm>
#include <climits>
#include <type_traits>
#include <variant>
#include "../../core/core.h"
#include "../../core/frame.h"
#include "../../core/hooks.h"
#include "../../core/modtext.h"
#include "../../core/say.h"
#include "../../core/worker_job.h"
#include "../../core/field/map_state.h"
#include "../../core/field/scene_events.h"
#include "../../core/field/puzzle_policy.h"
#include "../../core/fieldnav/autowalk.h"
#include "../../core/ui/preferences.h"

namespace ff5::library {
namespace {
namespace autowalk = ffpr::fieldnav::autowalk;
worker_job::Owner work;
bool active=false,pending=false;
uint64_t context=0;
std::optional<field::baked_nav::MapGoal> preview;
std::wstring instruction;
std::wstring rule_id;
int finish_map=0,finish_source=0;bool finish_departed=false;
unsigned finish_types=0;
bool observe_waypoint_arrival=true;
bool follow_karnak=false;
bool karnak_replan=false;
int karnak_chest_receipt=-1, karnak_story_receipt=-1;
unsigned rule_types(int map){return mechanics::types(map)|mechanics::context_types(map,rule_id);}
bool supports(int map){return supported(map)||catapult::supported(map)||stone::supported(map)||book::supported(map)||pyramid::supported(map)||jachol::supported(map)||rule_types(map);}
field::puzzle_policy::Kind kind() {
 const auto types=finish_departed&&finish_map==field::current_map_id()&&finish_types?finish_types:rule_types(field::current_map_id());if(types==2)return field::puzzle_policy::Kind::Other;if(types==3)return field::puzzle_policy::Kind::Unknown;
 return book::supported(field::current_map_id())||catapult::supported(field::current_map_id())?
  field::puzzle_policy::Kind::Other:field::puzzle_policy::Kind::Navigation;
}
bool enabled(){const field::puzzle_policy::Policy policy{ui::prefs::navigation_puzzle_guidance(),ui::prefs::other_puzzle_guidance()};return rule_types(field::current_map_id())==3?policy.any():policy.allows(kind());}
bool publication_enabled(unsigned types){return types?field::puzzle_policy::Policy{ui::prefs::navigation_puzzle_guidance(),ui::prefs::other_puzzle_guidance()}.allows_mask(types):enabled();}
bool ready(){return !menu_state::any_active()&&!queries::is("ModMenu.IsOpen")&&!queries::is("GameStatePatches.IsInputTransitionBlocked")&&
 queries::is("GameStatePatches.IsFieldNavigationReady")&&!queries::is("BattleStateHelper.IsInBattle");}
bool phase_ready(int phase,int map){return phase<0||(field::current_map_id()==map&&((map==284?jachol::phase_fresh(map,phase):map==424&&pyramid::phase_fresh(map,phase))||queries::is("TimerHelper.TimersFrozen")));}
std::wstring wait_note(int map){return T(map==284?L"Wait for the skull switch to appear.":L"Wait for the floor to change.");}
bool during_event(){return active&&!queries::is("BattleStateHelper.IsInBattle")&&
 (menu_state::any_active()||queries::is("MessageChoice.IsOpen")||queries::is("MessageWindowPatches.IsInDialogue"));}
void retire(){
 frame::cancel_label("FF5.Library.KarnakCutoff");
 if(preview)field::baked_nav::clear_preview_destination(*preview);
 preview.reset();
}
void stop(bool announce) {
 const bool was=active;active=pending=false;++context;work.cancel();retire();instruction.clear();rule_id.clear();
 if(follow_karnak&&autowalk::following_guide())autowalk::cancel();
 follow_karnak=karnak_replan=false;karnak_chest_receipt=karnak_story_receipt=-1;
 finish_map=finish_source=0;finish_departed=false;finish_types=0;
 observe_waypoint_arrival=true;
 frame::cancel_label("FF5.Library.Ready");if(was&&announce)ffpr::say(T(L"Puzzle guide stopped"),true);
}
Snapshot capture() {
 Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();
 if(!supported(s.map))return s;
 for(size_t i=0;i<flags.size();++i)s.values[i]=field::map_state::scenario_flag(flags[i]);
 const auto observed=controls(s.map);
 for(size_t i=0;i<observed.size();++i)s.hidden[i]=field::map_state::entity_hidden(s.map,observed[i].id,observed[i].raw);
 if(!field::baked_nav::player_cell(field::map_handle(),field::player(),s.player))s.map=0;
 return s;
}
using CurrentSnapshot=std::variant<Snapshot,catapult::Snapshot,stone::Snapshot,book::Snapshot,pyramid::Snapshot,jachol::Snapshot,mechanics::Snapshot>;
CurrentSnapshot capture_current(){
 if(rule_types(field::current_map_id())){
  mechanics::Snapshot s;s.map=field::current_map_id();s.flags=field::map_state::snapshot();s.locale=modtext::lang();s.policy={ui::prefs::navigation_puzzle_guidance(),ui::prefs::other_puzzle_guidance()};s.guide_id=rule_id;
  if(!field::baked_nav::player_cell(field::map_handle(),field::player(),s.player))s.map=0;
  if(karnak::is_castle(s.map) && s.flags.scenario[42]==1 && s.flags.scenario[43]==1 &&
     s.flags.scenario[46]==1 && s.flags.scenario[47]==0) {
   s.karnak_seconds=ffpr::ff5::karnak_seconds_remaining();
   s.karnak_route=karnak::Route(std::clamp(settings::get_int(karnak::preference,0),0,int(karnak::Route::AllChests)));
  }
  if(s.map==397&&s.flags.area[0]==0&&s.player==field::baked_route::Cell{2,6,10}) {
   const auto hidden=field::map_state::entity_hidden(397,27,L"床を止める");
   if(hidden)s.exdeath_stop_visible=!*hidden;
  }
  return s;
 }
 if(jachol::supported(field::current_map_id())){
  jachol::Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();s.flags=field::map_state::snapshot();
  if(s.map==284){
   if(s.flags.scenario[407]==1){s.skull_hidden=field::map_state::entity_hidden(284,8,L"sc_e_0226_3:壁が移動し、道ができる");s.fresh=jachol::phase_fresh(284,1);s.timers_frozen=queries::is("TimerHelper.TimersFrozen");}
   else if(s.flags.scenario[407]==0&&s.flags.scenario[408]==1)s.chest_door_hidden=field::map_state::entity_hidden(284,6,L"sc_e_0433:扉が開く");
  }
  if(!field::baked_nav::player_cell(field::map_handle(),field::player(),s.player))s.map=0;return s;
 }
 if(pyramid::supported(field::current_map_id())){
  pyramid::Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();s.flags=field::map_state::snapshot();
  if(s.map==413){s.corridor_hidden[0]=field::map_state::entity_hidden(413,1,L"sc_e_0293:壁から蛇(2F通路)");
   if(s.corridor_hidden[0]==true){s.corridor_hidden[1]=field::map_state::entity_hidden(413,117,L"白い蛇A");s.corridor_hidden[2]=field::map_state::entity_hidden(413,118,L"白い蛇B");}}
  if(s.map==421&&s.flags.scenario[391]==1&&s.flags.scenario[392]==0)s.last_sand_switch_hidden=field::map_state::entity_hidden(421,12,L"sc_e_0520:③の流砂が止まる");
  if(s.map==424&&s.flags.area[0]){s.phase_fresh=pyramid::phase_fresh(s.map,*s.flags.area[0]);s.timers_frozen=queries::is("TimerHelper.TimersFrozen");}
  if(!field::baked_nav::player_cell(field::map_handle(),field::player(),s.player))s.map=0;return s;
 }
 if(book::supported(field::current_map_id())){
  book::Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();
  for(size_t i=0;i<book::flags.size();++i)s.values[i]=field::map_state::scenario_flag(book::flags[i]);
  if(s.map==159)return s;
  if(s.values[0]==1&&s.values[1]==1&&s.values[2]==1){s.gate_hidden=field::map_state::entity_hidden(160,58,L"壁のコリジョン");return s;}
  const auto observed=book::pickups();
  for(size_t i=0;i<observed.size();++i)if(s.values[i]==0&&s.values[i+3]==0)s.hidden[i]=field::map_state::entity_hidden(160,observed[i].id,observed[i].raw);
  return s;
 }
 if(stone::supported(field::current_map_id())){
  stone::Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();
  if(s.map==373)return s;
  if(s.map==372){s.stairs_hidden=field::map_state::entity_hidden(372,11,L"壁のコリジョン");return s;}
  for(size_t i=0;i<stone::flags.size();++i)s.values[i]=field::map_state::scenario_flag(stone::flags[i]);return s;
 }
 if(!catapult::supported(field::current_map_id()))return capture();
 catapult::Snapshot s;s.map=field::current_map_id();s.locale=modtext::lang();
 for(size_t i=0;i<catapult::flags.size();++i)s.values[i]=field::map_state::scenario_flag(catapult::flags[i]);
 if(s.values[4]==1)return s; // native final receipt retires earlier actor dependencies
 const auto observed=catapult::controls(s.map);
 for(size_t i=0;i<observed.size();++i)s.hidden[i]=field::map_state::entity_hidden(s.map,observed[i].id,observed[i].raw);
 if(!field::baked_nav::player_cell(field::map_handle(),field::player(),s.player))s.map=0;
 return s;
}
void invalidate();

void replan_karnak()
{
 if (!active || rule_id!=L"karnak_escape") return;
 if (autowalk::following_guide()) autowalk::cancel();
 karnak_replan=true;
 invalidate();
}

bool watch_karnak_cutoff(int seconds, double requested)
{
 auto check_at=requested+std::max(0,seconds-karnak::exit_seconds-1);
 // The native clock can pause or battle can take control before the estimated
 // crossing. Verify the live clock on the ready field before changing goals.
 if (check_at<=frame::now()) {
  const auto current=ffpr::ff5::karnak_seconds_remaining();
  if (current && *current<=karnak::exit_seconds) {replan_karnak();return false;}
 }
 frame::every_frame([check_at]() mutable {
  if (!active || rule_id!=L"karnak_escape" || !preview) return false;
  const auto now=frame::now();
  if (now<check_at || !ready()) return true;
  const auto current=ffpr::ff5::karnak_seconds_remaining();
  if (!current) return false;
  if (*current>karnak::exit_seconds) {
   check_at=now+std::max(.10,double(*current-karnak::exit_seconds-1));
   return true;
  }
  // A manually stopped walk may still have a spoken guide preview.
  if (!autowalk::following_guide()) follow_karnak=false;
  replan_karnak();
  return false;
 }, "FF5.Library.KarnakCutoff", INT_MAX);
 return true;
}

void refresh(bool explicit_request) {
 if(!enabled()){stop(false);if(explicit_request)ffpr::say(T(field::puzzle_policy::disabled_key(kind())),true);return;}
 if(!ready())return;
 if(!explicit_request&&follow_karnak&&autowalk::active())return;
 if(!data::game_data_ready()||!field::baked_nav::prepared()){if(explicit_request)ffpr::say(T(L"Navigation data is loading"),true);return;}
 if(!supports(field::current_map_id())){const bool complete=finish_map==field::current_map_id()&&finish_departed;stop(false);if(complete)ffpr::say(T(L"Puzzle guide finished"),explicit_request);return;}
 finish_map=finish_source=0;finish_departed=false;finish_types=0;
 const double requested=frame::now();auto snapshot=capture_current();
 if(std::visit([](const auto& s){return s.map==0;},snapshot)){++context;work.cancel();retire();if(explicit_request)ffpr::say(T(L"Navigation data is loading"),true);return;}
 const auto* mechanics_state=std::get_if<mechanics::Snapshot>(&snapshot);
 const auto cutoff_clock=mechanics_state ? mechanics_state->karnak_seconds : std::optional<int>{};
 if(!explicit_request&&follow_karnak&&!karnak_replan) {
  const auto* state=mechanics_state;
  if(!state || (karnak_chest_receipt>=0 && state->flags.treasure[size_t(karnak_chest_receipt)]!=1) ||
     (karnak_story_receipt>=0 && state->flags.scenario[size_t(karnak_story_receipt)]!=1))return;
 }
 karnak_replan=false;
 active=true;const auto token=++context;work.cancel();const auto epoch=work.epoch();const auto generation=epoch->load();
 std::visit([epoch,generation](auto& s){s.cancelled=[epoch,generation]{return epoch->load()!=generation;};},snapshot);
 worker_job::start(work,std::move(snapshot),[](const CurrentSnapshot& s){return std::visit([](const auto& input){
   if constexpr(std::is_same_v<std::decay_t<decltype(input)>,Snapshot>)return plan(input,data::game_json("puzzles"));
   else if constexpr(std::is_same_v<std::decay_t<decltype(input)>,catapult::Snapshot>)return catapult::plan(input,data::game_json("puzzles"));
   else if constexpr(std::is_same_v<std::decay_t<decltype(input)>,stone::Snapshot>)return stone::plan(input,data::game_json("puzzles"));
   else if constexpr(std::is_same_v<std::decay_t<decltype(input)>,book::Snapshot>)return book::plan(input,data::game_json("puzzles"));
   else if constexpr(std::is_same_v<std::decay_t<decltype(input)>,pyramid::Snapshot>)return pyramid::plan(input,data::game_json("puzzles"));
   else if constexpr(std::is_same_v<std::decay_t<decltype(input)>,jachol::Snapshot>)return jachol::plan(input,data::game_json("puzzles"));
   else return mechanics::plan(input,data::game_json("puzzles"));
  },s);},
  [token,requested,explicit_request,cutoff_clock](Plan result) {
   if(!active||context!=token||!ready()||result.map!=field::current_map_id()||!publication_enabled(result.puzzle_types))return;
   if(!result.supported){stop(false);if(explicit_request)ffpr::say(T(L"No puzzle guide for this map"),true);return;}
   rule_id=result.guide_id;
   if(explicit_request)follow_karnak=rule_id==L"karnak_escape"&&ui::prefs::autowalk_enabled();
   observe_waypoint_arrival=result.observe_waypoint_arrival;
   if(!phase_ready(result.floor_phase,result.phase_map)){retire();instruction=wait_note(result.phase_map);ffpr::say(instruction,explicit_request);return;}
   if(result.finished){stop(false);ffpr::say(T(L"Puzzle guide finished"),explicit_request);return;}
   if(result.targets.empty()){retire();instruction=result.note;if(!instruction.empty())ffpr::say(instruction,explicit_request);return;}
   retire();
   auto targets=std::make_shared<std::vector<field::baked_nav::GuideTarget>>(result.targets);
   field::baked_nav::request_guidance(std::move(result.targets),[targets,token,requested,explicit_request,phase=result.floor_phase,phase_map=result.phase_map,finish=result.finish_map,source=result.map,types=result.puzzle_types,chest=result.karnak_treasure_flag,cutoff_clock](field::PathInfo path) {
    if(!active||context!=token||!ready()||!publication_enabled(types))return;
    if(!phase_ready(phase,phase_map)){for(const auto& target:*targets)field::baked_nav::clear_preview_destination(target.goal);retire();instruction=wait_note(phase_map);ffpr::say(instruction,explicit_request);return;}
    if(path.success&&finish){finish_map=finish;finish_source=source;finish_departed=false;finish_types=types;}
    preview.reset();instruction.clear();
    if(path.guidance_index>=0&&path.guidance_index<int(targets->size())) {
     preview=(*targets)[size_t(path.guidance_index)].goal;instruction=(*targets)[size_t(path.guidance_index)].instruction;
    }
    if(path.success&&preview&&chest>=0&&cutoff_clock&&!watch_karnak_cutoff(*cutoff_clock,requested))return;
    if(follow_karnak&&ui::prefs::autowalk_enabled()&&path.success&&preview&&!path.walk_phases.empty()) {
     karnak_chest_receipt=chest;
     karnak_story_receipt=chest>=0 ? -1 : preview->entity_id==29 ? 245 : 47;
     const auto& chosen=(*targets)[size_t(path.guidance_index)];
     autowalk::start(path,&chosen.goal,chosen.kind,{},false);
     if(autowalk::following_guide()) {
      // The native chest interaction supplies its own audio. A manual chest
      // or a rapid-escape instruction still needs the guide's cue.
      if(chest<0||!ui::prefs::auto_interact_enabled())ffpr::say(instruction,false);
      return;
     }
    }
    auto line=path.success?path.description:(!path.reason.empty()?path.reason:T(L"No path to the next step"));
    if(path.success&&!path.guidance.empty()){if(!line.empty())line+=L". ";line+=path.guidance;}
    const speech::Timing timing{speech::Feature::Directions,requested,requested};
    if(ui::prefs::mod_warnings_enabled()&&!path.warning.empty())ffpr::say_sequence({path.warning,std::move(line)},explicit_request,timing);
    else ffpr::say(line,explicit_request,timing);
   });
  },background::Lane::Navigation);
}
void schedule() {
 if(!active||pending)return;pending=true;
 frame::every_frame([]{if(!active||!pending)return false;if(!ready())return true;pending=false;refresh(false);return false;},"FF5.Library.Ready",INT_MAX);
}
void invalidate(){if(!active||(follow_karnak&&autowalk::following_guide()))return;++context;work.interrupt();if(field::current_map_id()==424){field::baked_nav::interrupt_requests();retire();instruction=wait_note(424);actions::run("Autowalk.Cancel");}schedule();}
void jachol_phase_changed(){if(!active||field::current_map_id()!=284)return;field::baked_nav::interrupt_requests();retire();instruction=wait_note(284);actions::run("Autowalk.Cancel");invalidate();}
void next() {
 if(!enabled()){stop(false);ffpr::say(T(field::puzzle_policy::disabled_key(kind())),true);return;}
 if(during_event()&&!ready()){if(!instruction.empty())ffpr::say(instruction,true);return;}
 if(ready())refresh(true);
}
void move() {
 // Only the policy's one-cell step-off waypoint needs movement observation.
 // Script controls advance on native ready events, never proximity alone.
 if(!active||!observe_waypoint_arrival||!preview||preview->entity_id>=0||preview->map_id!=field::current_map_id()||!ready())return;
 field::baked_route::Cell at;if(field::baked_nav::player_cell(field::map_handle(),field::player(),at)&&at==preview->cell){retire();invalidate();}
}
int install() {
 actions::define("GameModule.PuzzleGuidePolicyChanged",[]{stop(false);});
 actions::define("GameModule.PuzzleGuide",[]{hooks::guarded("FF5.Library.Next",[]{next();});});
 actions::define("GameModule.StopPuzzleGuide",[]{stop(true);});
 actions::define("GameModule.PuzzleGuideMove",[]{if(active)move();});
 actions::define("GameModule.FieldStateChanged",[]{invalidate();});
 actions::define("GameModule.GuidedWalkReturnedAfterBattle",[]{if(follow_karnak)replan_karnak();});
 actions::define("Settings.Changed.KarnakEscapeRoute",[]{replan_karnak();});
 actions::define("FF5.Jachol.PhaseChanged",[]{jachol_phase_changed();});
 queries::define("GameModule.HasPuzzleGuide",[]{return supports(field::current_map_id());});
 queries::define("GameModule.PuzzleGuideActive",[]{return active;});
 queries::define("GameModule.PuzzleGuideDuringEvent",[]{return during_event();});
 events::on(events::Kind::MapChanging,[]{if(active){finish_departed=finish_map&&finish_source==field::current_map_id();++context;work.interrupt();}});
 for(auto event:{events::Kind::MapChanged,events::Kind::BattleEnd,events::Kind::DialogueClosed,events::Kind::MenuClosed})
  events::on(event,[]{invalidate();});
 events::on(events::Kind::BattleEntering,[]{if(active){++context;work.interrupt();}});
 events::on(events::Kind::TitleScreen,[]{stop(false);});
 events::on(events::Kind::SceneLoaded,[]{if(field::scene_name()==L"Title")stop(false);});
 return 0;
}
core::Install installed{"FF5.LibraryGuide",&install,516,game::kFF5};
}
}
