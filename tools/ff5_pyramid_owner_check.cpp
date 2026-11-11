// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
// Real five-leaf owner/native-shaped storage and exact static phase producers.
// Retain every accepted four-leaf test before exercising the Pyramid addition.
#define FFPR_LIBRARY_OWNER_MAIN retained_ff5_library_owner_main
#include "ff5_library_owner_check.cpp"
#include "../src/games/ff5/pyramid_observation.cpp"
namespace pyramid_owner {
int original_calls=0;
void native0(const void*){fixture::native();++original_calls;fixture::area.values[0]=0;}
void native1(const void*){fixture::native();++original_calls;fixture::area.values[0]=1;}
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits={}){
 owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;
}
void explicit_step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_ff5_library_owner_main())return 1;
 using namespace owner;using namespace pyramid_owner;
 // baked::load retains its caller-owned bytes. The retained main's local
 // storage has ended;give the extension its own complete-lifetime backing.
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 std::puts("Pyramid owner:entrance");
 ff5::library::stop(false);fixture::area.values.fill(0);phase_test_time=10.;
 ff5::pyramid::original0=&native0;ff5::pyramid::original1=&native1;
 scene(410,{2,25,53},{135});const auto reads=fixture::native_control_reads;
 explicit_step();check(fixture::native_control_reads==reads&&targets.size()==1&&targets[0].goal.entity_id==17,"Pyramid entrance uses copied flags and position with zero actor scans");respond();
 const auto count=spoken.size();ready=false;menu=true;actions::run("GameModule.PuzzleGuide");
 check(spoken.size()==count+1&&fixture::native_control_reads==reads,"Pyramid menu repeat uses owned instruction without native observation");
 for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<100;++i)pulse();
 check(tasks.size()==1&&jobs.empty()&&fixture::native_control_reads==reads,"blocked Pyramid events coalesce without polling actors or flags");ready=true;menu=false;pulse();drain();respond();
 std::puts("Pyramid owner:live/reloaded controls");
 ff5::library::stop(false);scene(421,{1,13,7},{135,136,390,391});
 const auto lookup=fixture::native_control_reads;explicit_step();
 check(fixture::native_control_reads==lookup+1&&targets.front().goal.entity_id==5,"reloaded native391 hides the third switch and observes that exact control once");respond();
 const auto third=std::find_if(actors.begin(),actors.end(),[](const auto& actor){return actor.property->id==12;});
 if(third==actors.end())return 2;third->own_hidden=false;explicit_step();
 check(targets.front().goal.entity_id==12,"live visible third switch remains required before392");respond();
 fixture::missing_control=true;const auto before_unknown=routes;explicit_step();
 check(routes==before_unknown&&spoken.back()==L"Story guidance is waiting for current game state.","unresolved actual third control cannot skip its operation");fixture::missing_control=false;
 std::puts("Pyramid owner:flagless snake corridor");
 ff5::library::stop(false);scene(413,{1,5,11},{135,136});const auto corridor_reads=fixture::native_control_reads;explicit_step();
 check(fixture::native_control_reads==corridor_reads+1&&targets.front().goal.entity_id==1,"unactivated flagless corridor captures only its exact trigger");respond();
 const auto contact=std::find_if(actors.begin(),actors.end(),[](const auto& actor){return actor.property->id==1;});if(contact==actors.end())return 2;contact->own_hidden=true;position={1,11,7};
 const auto released_reads=fixture::native_control_reads;explicit_step();check(fixture::native_control_reads==released_reads+3&&targets.size()==3&&targets.front().goal.map_id==411,"native hidden contact observes the two actual snake receipts only after activation");respond();
 const auto snake=std::find_if(properties.begin(),properties.end(),[](const auto& p){return p.id==117;});if(snake==properties.end())return 2;
 const auto snake_name=snake->name;std::wstring wrong_snake=L"Unrelated actor";snake->name=&wrong_snake;const auto partial_routes=routes;explicit_step();
 check(routes==partial_routes&&spoken.back()==L"Story guidance is waiting for current game state.","a missing later snake receipt retires earlier candidate targets instead of publishing a partial unknown plan");snake->name=snake_name;
 fixture::missing_control=true;const auto corridor_routes=routes;explicit_step();check(routes==corridor_routes&&spoken.back()==L"Story guidance is waiting for current game state.","unknown flagless corridor receipt cannot skip the activation");fixture::missing_control=false;
 std::puts("Pyramid owner:timed floor");
 ff5::library::stop(false);scene(424,{1,12,16},{135,136});phase_test_time=20.;ff5::pyramid::phase0(nullptr);
 check(original_calls==1&&fixture::area.values[0]==0&&ff5::pyramid::phase_fresh(424,0)&&!ff5::pyramid::phase_fresh(424,1),"phase0 original completes before its scalar observation");
 auto before_route=routes;explicit_step();check(routes==before_route&&spoken.back()==L"Wait for the floor to change.","lower entry never routes in the wrong native phase");
 const auto native_reads=fixture::native_control_reads;const auto interrupted=interruptions;int autowalk_cancel=0;actions::define("Autowalk.Cancel",[&]{++autowalk_cancel;});
 phase_test_time=23.;ff5::pyramid::phase1(nullptr);
 check(original_calls==2&&fixture::area.values[0]==1&&interruptions==interrupted+1&&autowalk_cancel==1&&tasks.size()==1&&fixture::native_control_reads==native_reads,"native phase1 cancels old route/autowalk and schedules one bounded refresh");
 pulse();drain();check(routes==before_route+1&&targets.front().goal.cell==field::baked_route::Cell{1,11,13},"fresh phase1 publishes the exact invariant hold waypoint");respond();
 check(beacon.has_value(),"fresh crossing retains only its own destination preview");
 position={1,11,13};const auto move_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuideMove");
 check(!beacon&&tasks.size()==1&&fixture::native_control_reads==move_reads,"hold arrival cancels old crossing and schedules state refresh without actor scans");pulse();drain();check(spoken.back()==L"Wait for the floor to change.","invariant hold waits for the opposite phase");
 phase_test_time=26.;ff5::pyramid::phase0(nullptr);pulse();
 // An owned model completion can arrive after its short publication window.
 phase_test_time=27.01;before_route=routes;drain();
 check(routes==before_route&&spoken.back()==L"Wait for the floor to change.","late model completion cannot publish an expired timed crossing");
 phase_test_time=29.;ff5::pyramid::phase0(nullptr);pulse();drain();
 check(routes==before_route+1,"new native boundary permits the next crossing");
 const auto expired_speech=spoken.size();phase_test_time=30.01;respond();
 check(!beacon&&spoken.size()==expired_speech+1&&spoken.back()==L"Wait for the floor to change.","late route completion clears its preview and cannot speak obsolete directions");
 phase_test_time=32.;ff5::pyramid::phase0(nullptr);pulse();drain();auto held=std::move(publish);publish={};
 phase_test_time=35.;ff5::pyramid::phase1(nullptr);const auto obsolete=spoken.size();
 if(!held)return 2;field::PathInfo stale;stale.success=true;stale.guidance_index=0;stale.description=L"Expired crossing";held(stale);
 check(spoken.size()==obsolete&&!beacon,"native opposite phase rejects an already queued crossing");
 menu=true;ready=false;actions::run("GameModule.PuzzleGuide");check(spoken.back()==L"Wait for the floor to change.","menu repeat cannot repeat an obsolete Go instruction after a native phase change");menu=false;ready=true;pulse();drain();
 timers_frozen=true;before_route=routes;explicit_step();check(routes==before_route&&spoken.back()==L"Resume timers until the floor changes.","frozen wrong phase explains how to continue safely");
 fixture::area.values[0]=0;phase_test_time=100.;explicit_step();check(routes==before_route+1,"explicit frozen matching phase supports the safe cardinal leg without a deadline");respond();timers_frozen=false;
 check(!ff5::pyramid::phase_fresh(423,0)&&!ff5::pyramid::phase_fresh(424,2),"phase witness cannot validate another floor or invalid phase");
 std::puts("Pyramid owner:full tablet return");
 ff5::library::stop(false);scene(426,{1,11,24},{135,136});explicit_step();respond();
 ready=false;const auto scene_routes=routes;const auto scene_speech=spoken.size();
 for(int map:{410,19,410,19,410,19,410}){event(events::Kind::MapChanging);fixture::storage.map_id=map;native_scene(map);event(events::Kind::MapChanged);for(int i=0;i<5;++i)pulse();}
 check(ff5::library::active&&routes==scene_routes&&spoken.size()==scene_speech&&jobs.empty(),"all seven tablet-return maps preserve the owned guide until full native readiness");
 flags({135,136,137});pulse();check(ff5::library::active&&spoken.size()==scene_speech,"even full137 cannot finish during its blocked native scene");
 ready=true;pulse();drain();check(!ff5::library::active&&spoken.back()==L"Puzzle guide finished"&&!beacon,"full137 plus ready final exterior completes and retires the entire guide");
 scene(410,{2,25,21},{135,136});nav_guidance=false;other_guidance=true;const auto disabled=routes;explicit_step();
 check(routes==disabled&&!ff5::library::active&&spoken.back()==L"Navigation puzzle guidance is off","Pyramid obeys its independent Navigation toggle");nav_guidance=true;other_guidance=false;explicit_step();
 check(routes==disabled+1,"Pyramid navigation remains available when non-navigation guidance is off");respond();
 nav_guidance=other_guidance=true;ff5::library::stop(false);check(fixture::wrong_owner==0,"no Pyramid native read or speech runs on the policy worker");
 std::printf("Pyramid native owner:%d total cases/%d failures;accepted leaves retained;phase deadline/cancel/full return verified\n",cases,fixture::failures);return fixture::failures?1:0;
}
