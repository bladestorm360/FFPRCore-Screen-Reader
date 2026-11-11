// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
// Actual native-reader + Library adapter. Native-shaped fields only; no game.
#define FFPR_LIBRARY_OWNER_CHECK
#define main retained_mapstate_native_main
#include "mapstate_native_check.cpp"
#undef main
#include "../src/games/ff5/library_guide.cpp"
namespace owner {
bool ready=true,menu=false,battle=false,busy=false,position_valid=true;
bool nav_guidance=true,other_guidance=true;
bool timers_frozen=false;double phase_test_time=1.;int interruptions=0;
std::map<std::string,actions::Fn> actions;
std::map<std::string,std::function<bool()>> queries;
std::map<int,std::pair<std::string,frame::Step>> tasks;int next_task=0;
std::vector<frame::Task> posts,jobs;
std::vector<std::wstring> spoken;
std::vector<field::baked_nav::GuideTarget> targets;
std::function<void(field::PathInfo)> publish;
std::optional<field::baked_nav::MapGoal> beacon;
field::baked_route::Cell position{1,8,12};int routes=0,clears=0,cases=0;
std::vector<std::wstring> names;std::vector<fixture::ControlProperty> properties;std::vector<fixture::Control> actors;
thread_local std::wstring locale=L"en";
void check(bool value,const char* why){++cases;fixture::expect(value,why);}
void event(events::Kind kind){for(const auto& fn:fixture::event_handlers[kind])fn();}
void pulse(){std::vector<int> ids;for(const auto& [id,task]:tasks)ids.push_back(id);for(int id:ids){auto it=tasks.find(id);if(it!=tasks.end()&&!it->second.second())tasks.erase(id);}}
void drain(){auto work=std::move(jobs);jobs.clear();std::thread worker([&]{for(auto& fn:work)fn();});worker.join();auto ready_posts=std::move(posts);posts.clear();for(auto& fn:ready_posts)fn();}
bool same(const field::baked_nav::MapGoal& a,const field::baked_nav::MapGoal& b){return a.map_id==b.map_id&&a.cell==b.cell&&a.entity_id==b.entity_id&&a.raw_name==b.raw_name;}
void respond(size_t index=0){if(!publish||index>=targets.size()){check(false,"a real pending route is required for publication");return;}field::PathInfo path;path.success=true;path.guidance_index=int(index);path.description=L"Owned route";path.guidance=targets[index].instruction;
 beacon=targets[index].goal;auto callback=std::move(publish);publish={};callback(std::move(path));}
void native_scene(int map) {
 fixture::current_map=map;fixture::running_game=game::Id::FF5;fixture::hidden_control=fixture::hidden_parent=fixture::bad_control=fixture::missing_control=fixture::dead_control=false;
 names.clear();properties.clear();actors.clear();fixture::controls.clear();names.reserve(64);properties.reserve(64);actors.reserve(64);
 json::Value records;json::parse(fixture::read("data/ff5/mapstate.json"),records);field::map_state::Flags values;values.scenario.fill(0);values.global.fill(0);values.local.fill(0);values.area.fill(0);values.treasure.fill(0);
 for(int bit=0;bit<1024;++bit)values.scenario[size_t(bit)]=(fixture::scenario.values[size_t(bit)/32]>>(bit%32))&1;
 for(size_t i=0;i<values.area.size()&&i<fixture::area.values.size();++i)values.area[i]=fixture::area.values[i];
 const auto* room=field::baked::submap_by_map_id(map);if(!room)return;
 const auto* record=records.get(field::baked::name_of(*room));const auto selected=record?field::map_state::project(*record,values):field::map_state::Selection{};
 // Build native-shaped actors from the independently baked identities and
 // actual initializer group/Hide facts, never the guide's watched-ID list.
 for(const auto& object:field::baked::grids(*room).objects) {
  if(object.entity_id()<=0||(object.type!=field::baked::ObjType::Event&&object.type!=field::baked::ObjType::Entity&&object.type!=field::baked::ObjType::NPC))continue;
  const auto group=field::baked::str(object.group);if(group!=L"entity_default"&&!selected.groups.count(group))continue;
  names.push_back(field::baked::object_identifier(object));properties.push_back({object.entity_id(),&names.back()});
  const auto entry=selected.entities.find(object.entity_id());const bool hidden=entry!=selected.entities.end()&&entry->second.visible==false;
  actors.push_back({&properties.back(),hidden});fixture::controls.push_back(&actors.back());
 }
}
void flags(std::initializer_list<int> set){fixture::scenario.values.fill(0);for(int bit:set)fixture::scenario.values[size_t(bit)/32]|=1u<<(bit%32);}
}
namespace modtext {
LocaleScope::LocaleScope(std::wstring language){previous_=owner::locale;owner::locale=std::move(language);}
LocaleScope::~LocaleScope(){owner::locale=*previous_;}
const std::wstring& lang(){return owner::locale;}
std::wstring T(std::wstring_view value){return std::wstring(value);}
std::wstring TF(std::wstring_view key,std::initializer_list<std::wstring> args){std::wstring out(key);int i=0;for(const auto& arg:args){auto token=L"{"+std::to_wstring(i++)+L"}";auto at=out.find(token);if(at!=out.npos)out.replace(at,token.size(),arg);}return out;}
}
namespace data {bool game_data_ready(){return true;}}
namespace settings {bool get_bool(const char* key,bool fallback){return std::string(key)==ui::prefs::kNavigationPuzzleGuidance?owner::nav_guidance:std::string(key)==ui::prefs::kOtherPuzzleGuidance?owner::other_guidance:fallback;}}
namespace field {
void* player(){fixture::native();return reinterpret_cast<void*>(1);}
std::wstring translate_entity_name_snapshot(const std::wstring& value){return value;}
const std::wstring& scene_name(){static std::wstring scene=L"Field";return scene;}
}
namespace field::baked_nav {
bool prepared(){return true;}
void interrupt_requests(){++owner::interruptions;}
bool player_cell(void*,void*,baked_route::Cell& cell){fixture::native();cell=owner::position;return owner::position_valid;}
void clear_preview_destination(const MapGoal& goal){++owner::clears;if(owner::beacon&&owner::same(goal,*owner::beacon))owner::beacon.reset();}
void request_guidance(std::vector<GuideTarget> targets,std::function<void(PathInfo)> publish,const map_state::Flags*,const DefenseContext*){
 fixture::native();++owner::routes;owner::targets=std::move(targets);owner::publish=std::move(publish);}
}
namespace ffpr {
void say(std::wstring_view value,bool){fixture::native();owner::spoken.emplace_back(value);}
void say(std::wstring_view value,bool interrupt,speech::Timing){say(value,interrupt);}
void say_sequence(std::vector<std::wstring> values,bool interrupt,speech::Timing){for(const auto& value:values)say(value,interrupt);}
}
namespace menu_state {bool any_active(){return owner::menu;}}
namespace actions {
void define(const char* name,Fn fn){owner::actions[name]=std::move(fn);}
bool defined(const char* name){return owner::actions.count(name)>0;}
bool run(const char* name){const auto it=owner::actions.find(name);if(it==owner::actions.end())return false;it->second();return true;}
}
namespace queries {
void define(const char* name,std::function<bool()> fn){owner::queries[name]=std::move(fn);}
bool is(const char* name){if(std::string(name)=="GameStatePatches.IsFieldNavigationReady")return owner::ready;
 if(std::string(name)=="GameStatePatches.IsInputTransitionBlocked")return !owner::ready;
 if(std::string(name)=="BattleStateHelper.IsInBattle")return owner::battle;
 if(std::string(name)=="TimerHelper.TimersFrozen")return owner::timers_frozen;
 const auto it=owner::queries.find(name);return it!=owner::queries.end()&&it->second();}
}
namespace frame {
double now(){return owner::phase_test_time;}
int every_frame(Step step,const char* label,int){const int id=++owner::next_task;owner::tasks[id]={label,std::move(step)};return id;}
void cancel_label(const char* prefix){for(auto it=owner::tasks.begin();it!=owner::tasks.end();)if(it->second.first.rfind(prefix,0)==0)it=owner::tasks.erase(it);else ++it;}
void post(Task task){owner::posts.push_back(std::move(task));}
}
namespace background {bool post(std::function<void()> job,Lane lane){owner::check(lane==Lane::Navigation,"Library work stays on Navigation");if(owner::busy)return false;owner::jobs.push_back(std::move(job));return true;}}
#ifndef FFPR_LIBRARY_OWNER_MAIN
#define FFPR_LIBRARY_OWNER_MAIN main
#endif
int FFPR_LIBRARY_OWNER_MAIN(){
 std::setvbuf(stdout,nullptr,_IONBF,0);
 using namespace owner;
 if(retained_mapstate_native_main())return 1;fixture::wrong_owner=0;
 auto data=fixture::read("data/ff5/maps.bin");if(!field::baked::load(data.data(),data.size())||!json::parse(fixture::read("data/ff5/puzzles.json"),fixture::records))return 2;
 fixture::current=&fixture::storage;fixture::storage.scenario=&fixture::scenario_flags;fixture::scenario_flags.bits=&fixture::scenario;fixture::scenario_flags.length=1024;fixture::scenario.length=128;
 ff5::library::installed.fn();flags({});native_scene(275);
 const int before=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads-before==8,"one explicit upper capture reads exactly eight controls");
 check(jobs.size()==1&&routes==0,"no layout or route work runs inline on native owner");
 drain();check(routes==1&&!targets.empty(),"actual worker model produces native-owner route request");respond();check(beacon.has_value(),"first published guide retains beacon preview");
 const auto first_speech=spoken.size();actions::run("GameModule.PuzzleGuide");drain();respond();
 check(spoken.size()==first_speech+1&&beacon.has_value(),"identical explicit repeat speaks and does not erase its new preview");
 menu=true;ready=false;const int before_repeat=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads==before_repeat&&spoken.size()==first_speech+2,"menu repeat uses owned instruction without a native layout read");menu=false;
 for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");
 check(tasks.size()==1,"one ready continuation coalesces a native event burst");
 for(int i=0;i<100;++i)pulse();check(fixture::native_control_reads==before_repeat,"blocked frames never poll the layout");
 ready=true;pulse();check(fixture::native_control_reads==before_repeat+8,"one ready capture after the complete event burst");drain();respond();
 const auto before_move=fixture::native_control_reads;for(int i=0;i<100;++i)actions::run("GameModule.PuzzleGuideMove");
 check(fixture::native_control_reads==before_move&&tasks.empty(),"ordinary movement never scans controls or treats contact proximity as completion");
 actions::run("GameModule.PuzzleGuide");const auto before_cancel=spoken.size();event(events::Kind::MapChanging);drain();
 check(spoken.size()==before_cancel,"map epoch rejects queued stale advice");
 const auto watched=std::find_if(properties.begin(),properties.end(),[](const auto& value){return value.id==9;});
 if(watched==properties.end())return 2;
 const auto original=watched->name;std::wstring wrong=L"unrelated control";watched->name=&wrong;
 const auto incomplete=ff5::library::capture();bool missing=false;for(size_t i=0;i<8;++i)missing|=!incomplete.hidden[i];
 check(missing,"exact native raw identity mismatch stays unknown");watched->name=original;
 fixture::controls.assign(65,&fixture::control);const auto over=fixture::native_control_reads;const auto malformed=ff5::library::capture();
 check(fixture::native_control_reads==over,"over-budget native list is rejected before any control lookup");check(!malformed.hidden[0],"over-budget actor list never establishes a completed operation");native_scene(275);
 position_valid=false;actions::run("GameModule.PuzzleGuide");check(spoken.back()==L"Navigation data is loading","missing actual player position gives a loading answer");position_valid=true;
 flags({48,49,163,396});native_scene(283);position={1,5,10};actions::run("GameModule.PuzzleGuide");drain();
 check(!targets.empty()&&targets.front().goal.entity_id==1,"Byblos49 continues to actual Mid interaction instead of declaring completion");respond();
 flags({50});native_scene(272);actions::run("GameModule.PuzzleGuide");drain();
 check(spoken.back()==L"Puzzle guide finished"&&!queries::is("GameModule.PuzzleGuideActive"),"Mid50 plus actual exterior ends guide");
 native_scene(256);check(!queries::is("GameModule.HasPuzzleGuide"),"independent engine-room authored guide remains available");
 flags({});native_scene(275);position={1,8,12};actions::run("GameModule.PuzzleGuide");actions::run("GameModule.StopPuzzleGuide");drain();
 check(!queries::is("GameModule.PuzzleGuideActive")&&tasks.empty(),"stop retires callbacks and ready work");
 flags({48,163,396});native_scene(281);position={1,5,13};
 const auto lower_before=fixture::native_control_reads;auto lower=ff5::library::capture();
 check(lower.map==281&&fixture::native_control_reads==lower_before+5,"lower observation reads exactly five controls");
 check(std::all_of(lower.hidden.begin(),lower.hidden.begin()+5,[](auto value){return value.has_value();}),"every exact lower control resolves in the native initialized group");
 const auto lower_actor=std::find_if(actors.begin(),actors.end(),[](const auto& actor){return actor.property->id==13;});
 if(lower_actor==actors.end())return 2;
 lower_actor->own_hidden=!lower_actor->own_hidden;auto shifted=ff5::library::capture();
 check(shifted.hidden[0]!=lower.hidden[0],"per-control native visibility distinguishes the flagless lower layout");
 fixture::hidden_parent=true;const auto parent=ff5::library::capture();
 check(!parent.hidden[0],"an inactive parent cannot establish a lower-layout receipt");fixture::hidden_parent=false;
 actions::run("GameModule.StopPuzzleGuide");flags({});native_scene(275);position={1,8,12};busy=true;
 const auto busy_before=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");const auto busy_routes=routes;
 for(int i=0;i<40;++i)pulse();
 check(jobs.empty()&&routes==busy_routes&&fixture::native_control_reads==busy_before+8,"busy Navigation retries the owned job without inline work or repeated native capture");
 busy=false;pulse();drain();check(routes==busy_routes+1,"queued Library advice resumes when Navigation accepts it");respond();
 actions::run("GameModule.PuzzleGuide");drain();auto stale_route=std::move(publish);publish={};
 const auto route_speech=spoken.size();event(events::Kind::MapChanging);field::PathInfo stale;stale.success=true;stale.guidance_index=0;stale.description=L"Stale";
 if(!stale_route)return 2;stale_route(stale);check(spoken.size()==route_speech,"map cancellation rejects an already-requested route continuation");
 actions::run("GameModule.StopPuzzleGuide");
 // Native-shaped post-operation witness275:0:131 is independently present in
 // the closed comparison graph. Its occupied next contact is14, not9: native9
 // has already hidden itself. A ready position on9 with fresh-entry visibility
 // was not a valid witness for the table's step-off operation.
 for(size_t i=0;i<ff5::library::controls(275).size();++i){const auto& watched=ff5::library::controls(275)[i];
  for(auto& actor:actors)if(actor.property->id==watched.id&&*static_cast<const std::wstring*>(actor.property->name)==watched.raw)actor.own_hidden=bool((131>>i)&1);
 }
 const auto* upper=field::baked::submap_by_map_id(275);const auto contact=std::find_if(field::baked::grids(*upper).objects.begin(),field::baked::grids(*upper).objects.end(),[](const auto& object){return object.entity_id()==14&&field::baked_route::step_event(object);});
 if(contact==field::baked::grids(*upper).objects.end())return 2;position={contact->layer,contact->tx,contact->ty};
 actions::run("GameModule.PuzzleGuide");drain();const auto waypoint=std::find_if(targets.begin(),targets.end(),[](const auto& target){return target.goal.entity_id<0;});
 check(waypoint!=targets.end(),"standing on a contact offers the actual policy step-off alternative");
 if(waypoint!=targets.end()){
  const auto index=size_t(waypoint-targets.begin());const auto destination=waypoint->goal.cell;respond(index);
  const auto step_reads=fixture::native_control_reads;position=destination;actions::run("GameModule.PuzzleGuideMove");
  check(!beacon&&tasks.size()==1&&fixture::native_control_reads==step_reads,"step-off arrival retires its preview and schedules native-ready continuation without scanning controls");
  pulse();check(fixture::native_control_reads==step_reads+8,"one ready continuation captures the layout after the physical step-off");drain();respond();
 }
 event(events::Kind::TitleScreen);check(!queries::is("GameModule.PuzzleGuideActive")&&tasks.empty(),"title retirement cancels all Library ownership");
 flags({});native_scene(293);check(queries::is("GameModule.HasPuzzleGuide"),"same FFV owner dispatches the Catapult leaf");
 const auto cat_reads=fixture::native_control_reads;const auto cat_routes=routes;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads==cat_reads+5&&jobs.size()==1&&routes==cat_routes,"Catapult captures exactly five controls without inline policy or routing");
 drain();check(!targets.empty()&&targets.front().goal.entity_id==30,"Catapult starts at the required planter, not the optional hint switch");respond();
 const auto cat_repeat=fixture::native_control_reads;const auto cat_speech=spoken.size();menu=true;ready=false;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads==cat_repeat&&spoken.size()==cat_speech+1,"Catapult native-menu repeat uses owned advice");
 for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<30;++i)pulse();
 check(tasks.size()==1&&fixture::native_control_reads==cat_repeat,"Catapult blocked event burst coalesces without observation polling");
 menu=false;ready=true;pulse();drain();respond();
 const auto cat_moves=fixture::native_control_reads;for(int i=0;i<100;++i)actions::run("GameModule.PuzzleGuideMove");
 check(fixture::native_control_reads==cat_moves,"Catapult movement never scans controls");
 position_valid=false;actions::run("GameModule.PuzzleGuide");check(spoken.back()==L"Navigation data is loading"&&jobs.empty()&&!beacon,"Catapult missing current position retires the preview and never guesses entry contact");position_valid=true;
 flags({309,310,311});native_scene(293);actions::run("GameModule.PuzzleGuide");drain();
 check(!targets.empty()&&targets.front().goal.entity_id==23&&targets.front().goal.cell.x==-2,"Catapult retains exact staged book identity for authoritative live-pose routing");respond();
 flags({309,310,311,312});native_scene(293);actions::run("GameModule.PuzzleGuide");drain();
 check(!targets.empty()&&targets.front().goal.entity_id==18,"Catapult native book receipt selects final switch");respond();
 flags({313});native_scene(293);fixture::controls.clear();const auto done_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();
 check(spoken.back()==L"Puzzle guide finished"&&!queries::is("GameModule.PuzzleGuideActive")&&fixture::native_control_reads==done_reads,"Catapult final native receipt finishes without obsolete actor dependencies");
 flags({});native_scene(293);actions::run("GameModule.PuzzleGuide");const auto before_switch=spoken.size();event(events::Kind::MapChanging);native_scene(275);drain();
 check(spoken.size()==before_switch,"map change rejects queued advice from the other FFV leaf");actions::run("GameModule.StopPuzzleGuide");
 flags({});native_scene(371);check(queries::is("GameModule.HasPuzzleGuide"),"same FFV owner supports Ghido");
 const auto stone_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();
 check(fixture::native_control_reads==stone_reads&&!targets.empty()&&targets.front().goal.entity_id==28,"Ghido reads seven story bits without scanning chest actors");respond();
 flags({176,179});native_scene(371);actions::run("GameModule.PuzzleGuide");drain();check(!targets.empty()&&targets.front().goal.entity_id==30,"carried stone selects native northwest chest");respond();
 flags({174,176});native_scene(371);actions::run("GameModule.PuzzleGuide");drain();check(!targets.empty()&&targets.front().goal.map_id==370&&targets.front().goal.entity_id==15,"NW receipt selects the actual first switch");respond();
 native_scene(372);const auto gate_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();
 check(fixture::native_control_reads==gate_reads+1&&!targets.empty()&&targets.front().goal.entity_id==15,"secret-stair floor observes only exact current gate11");respond();
 const auto gate=std::find_if(actors.begin(),actors.end(),[](const auto& actor){return actor.property->id==11;});if(gate==actors.end())return 2;gate->own_hidden=true;
 actions::run("GameModule.FieldStateChanged");pulse();drain();check(!targets.empty()&&targets.front().goal.map_id==373,"native hidden-gate receipt selects the final physical exit");respond();
 native_scene(373);fixture::controls.clear();const auto final_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();
 check(spoken.back()==L"Puzzle guide finished"&&fixture::native_control_reads==final_reads&&!queries::is("GameModule.PuzzleGuideActive"),"actual final floor retires Ghido observations and owner");
 flags({});native_scene(160);check(queries::is("GameModule.HasPuzzleGuide"),"same FFV owner supports Surgate");
 const auto book_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");check(jobs.size()==1&&fixture::native_control_reads==book_reads+3,"Surgate observes exactly three outstanding floor books before worker dispatch");drain();
 check(!targets.empty()&&targets.front().goal.entity_id==17,"empty arrangement selects first native book");respond();
 const auto book_repeat=fixture::native_control_reads;const auto book_speech=spoken.size();menu=true;ready=false;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads==book_repeat&&spoken.size()==book_speech+1,"Surgate menu repeat uses the current owned instruction");
 for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<30;++i)pulse();check(tasks.size()==1&&fixture::native_control_reads==book_repeat,"Surgate native-ready invalidations coalesce without blocked-frame actor scans");menu=false;ready=true;pulse();drain();respond();
 const auto book_moves=fixture::native_control_reads;for(int i=0;i<100;++i)actions::run("GameModule.PuzzleGuideMove");check(fixture::native_control_reads==book_moves,"Surgate ordinary movement never scans books");
 flags({524});native_scene(160);const auto held_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();check(fixture::native_control_reads==held_reads+2&&targets.size()==3&&targets.front().goal.entity_id==21,"held book retires its floor actor and selects exact native shelf alternatives");respond();
 flags({});native_scene(160);const auto lost=std::find_if(actors.begin(),actors.end(),[](const auto& a){return a.property->id==17;});if(lost==actors.end())return 2;lost->own_hidden=true;
 actions::run("GameModule.PuzzleGuide");drain();check(!targets.empty()&&targets.front().goal.entity_id==1,"actual missing book selects the real reset exit");respond();
 native_scene(159);const auto corridor_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();check(fixture::native_control_reads==corridor_reads&&!targets.empty()&&targets.front().goal.map_id==160&&targets.front().goal.entity_id<0,"return corridor uses a safe future-floor waypoint with no actor reads");respond();
 flags({521,522,523});native_scene(160);const auto final_wall=std::find_if(actors.begin(),actors.end(),[](const auto& a){return a.property->id==58;});if(final_wall==actors.end())return 2;final_wall->own_hidden=false;
 const auto librarian_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();check(fixture::native_control_reads==librarian_reads+1&&!targets.empty()&&targets.front().goal.entity_id==20,"complete arrangement observes only wall58 before the librarian action");respond();
 final_wall->own_hidden=true;actions::run("GameModule.FieldStateChanged");pulse();drain();check(spoken.back()==L"Puzzle guide finished"&&!queries::is("GameModule.PuzzleGuideActive"),"actual native wall receipt completes Surgate and retires preview");
 native_scene(159);fixture::controls.clear();const auto outside_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");drain();check(spoken.back()==L"Puzzle guide finished"&&fixture::native_control_reads==outside_reads,"completed corridor retires all earlier actor dependencies");
 ff5::library::stop(false);flags({});native_scene(293);actions::run("GameModule.PuzzleGuide");drain();auto held_type_route=publish;
 other_guidance=false;actions::run("GameModule.PuzzleGuidePolicyChanged");const auto quiet=spoken.size();
 if(held_type_route){field::PathInfo p;p.success=true;p.guidance_index=0;p.guidance=L"Stale clue";held_type_route(p);}
 check(!ff5::library::active&&spoken.size()==quiet,"non-navigation toggle quietly retires held Catapult work");
 const int old_calls=fixture::native_control_reads,old_routes=routes;actions::run("GameModule.PuzzleGuide");
 check(fixture::native_control_reads==old_calls&&routes==old_routes&&spoken.back()==L"Non-navigation puzzle guidance is off","disabled clue solution does not scan actors or route");
 nav_guidance=false;other_guidance=true;native_scene(272);actions::run("GameModule.PuzzleGuide");
 check(!ff5::library::active&&spoken.back()==L"Navigation puzzle guidance is off","library maze has independent navigation ownership");
 nav_guidance=other_guidance=true;
 check(fixture::wrong_owner==0,"no native read or speech on layout worker");
 std::printf("FFV native guide owner: %d cases/%d failures; bounded controls, no blocked-frame/movement scans, cancellation/repeat/preview/native completion verified\n",cases,fixture::failures);return fixture::failures?1:0;
}
