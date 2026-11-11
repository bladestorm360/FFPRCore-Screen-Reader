// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_jachol_library_owner_main
#include "ff5_library_owner_check.cpp"
#include "../src/games/ff5/jachol_observation.cpp"
namespace jachol_owner {
int original_calls=0;
void visibility(bool hidden){for(auto& a:owner::actors)if(a.property->id==8)a.own_hidden=hidden;}
void native_show(const void*){fixture::native();++original_calls;visibility(false);fixture::area.values[1]=4;}
void native_hide(const void*){fixture::native();++original_calls;visibility(true);}
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits={}){owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_jachol_library_owner_main())return 1;
 using namespace owner;using namespace jachol_owner;
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);fixture::area.values.fill(0);phase_test_time=10.;
 ff5::jachol::original_set=&native_show;ff5::jachol::original_reset=&native_hide;
 scene(284,{1,12,10});const auto initial_reads=fixture::native_control_reads;step();
 check(targets.size()==2&&targets.front().goal.entity_id==25&&fixture::native_control_reads==initial_reads,"fresh Jachol offers real first controls without an actor scan");respond();
 scene(284,{1,30,28},{407});ff5::jachol::on_reset(nullptr);pulse();drain();
 check(original_calls==1&&!ff5::jachol::phase_fresh(284,1)&&spoken.back()==L"Wait for the skull switch to appear.","native hide finishes before scalar observation;initial unchosen column waits safely");
 const auto before_show=fixture::native_control_reads;const int old_interrupts=interruptions;const int old_routes=routes;
 phase_test_time=16.;ff5::jachol::on_set(nullptr);
 check(original_calls==2&&ff5::jachol::phase_fresh(284,1)&&interruptions==old_interrupts+1&&fixture::native_control_reads==before_show&&tasks.size()==1,"native show copies only scalar phase and coalesces one refresh without inline observation");
 pulse();drain();check(routes==old_routes+1&&targets.size()==1&&targets[0].goal.entity_id==8&&fixture::native_control_reads==before_show+1,"fresh real switch captures one exact hidden value and routes on the owner continuation");
 phase_test_time=17.01;const auto expired=spoken.size();respond();check(spoken.size()==expired+1&&spoken.back()==L"Wait for the skull switch to appear.","late route cannot publish timed movement");
 phase_test_time=25.;ff5::jachol::on_set(nullptr);pulse();drain();auto stale=std::move(publish);publish={};phase_test_time=28.;ff5::jachol::on_reset(nullptr);const auto retired=spoken.size();
 field::PathInfo old;old.success=true;old.guidance_index=0;old.description=L"Stale";if(!stale)return 2;stale(std::move(old));check(spoken.size()==retired&&!beacon,"native hide retires the old pose route,preview and epoch");pulse();drain();
 ready=false;menu=true;const auto blocked_reads=fixture::native_control_reads;for(int i=0;i<25;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<50;++i)pulse();
 check(jobs.empty()&&tasks.size()==1&&fixture::native_control_reads==blocked_reads,"blocked events coalesce without polling actor state");ready=true;menu=false;pulse();drain();
 fixture::missing_control=true;const auto unknown_routes=routes;step();check(routes==unknown_routes&&spoken.back()==L"Story guidance is waiting for current game state.","unresolved real actor cannot become a guessed movement target");fixture::missing_control=false;
 timers_frozen=true;visibility(false);phase_test_time=100.;step();check(routes==unknown_routes+1&&targets.front().goal.entity_id==8,"frozen visible switch retains the actual native route without a clock deadline");respond();visibility(true);step();check(spoken.back()==L"Resume timers until the skull switch appears.","frozen hidden phase asks for native timer resumption");timers_frozen=false;
 scene(284,{1,30,28},{408});const auto door_reads=fixture::native_control_reads;step();check(fixture::native_control_reads==door_reads+1&&targets.front().goal.entity_id==6,"opened skull puzzle observes only the exact flagless chest-door control");respond();
 ready=false;menu=true;actions::run("GameModule.FieldStateChanged");const auto choice_reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");check(spoken.back()==L"Examine the chest. Choose the first option to open the passage."&&fixture::native_control_reads==choice_reads,"ordinary chest choice repeats its own cached answer without a skull timing cue or native capture");ready=true;menu=false;pulse();drain();respond();
 for(auto& actor:actors)if(actor.property->id==6)actor.own_hidden=true;step();check(targets.front().goal.map_id==285&&targets.front().goal.entity_id<0,"actual chest control Hide unlocks the next-floor guide step");respond();
 // The complete native return is armed only after all real chests and a
 // successful route to the real climb control. A blocked new map cannot finish.
 fixture::treasure.values.fill(~0u);scene(285,{1,27,8},{435});step();check(targets.front().goal.entity_id==10,"all source receipts select the actual climb");respond();
 ready=false;event(events::Kind::MapChanging);scene(1,{1,0,0},{435});event(events::Kind::MapChanged);const auto waiting=spoken.size();pulse();check(queries::is("GameModule.PuzzleGuideActive")&&spoken.size()==waiting,"new world arrival waits for full native field readiness");ready=true;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","full ready first-world climb completes the journey");
 scene(285,{1,27,8},{125,435});step();respond();event(events::Kind::MapChanging);scene(173,{1,10,10},{125,435});event(events::Kind::MapChanged);pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","full ready later-world Bal return completes independently");
 scene(284,{1,12,10});nav_guidance=false;other_guidance=true;const auto disabled=routes;step();check(routes==disabled&&spoken.back()==L"Navigation puzzle guidance is off","Jachol respects independent Navigation disable");nav_guidance=true;other_guidance=false;step();check(routes==disabled+1,"Other guidance disable preserves Jachol navigation");respond();nav_guidance=other_guidance=true;
 ff5::library::stop(false);check(fixture::wrong_owner==0,"Jachol performs no native observation or speech on the model worker");std::printf("Jachol native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
