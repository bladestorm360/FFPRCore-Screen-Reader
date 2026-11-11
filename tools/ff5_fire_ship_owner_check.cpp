// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
// Actual owner:standing points never substitute for operating the native switch.
#define FFPR_LIBRARY_OWNER_MAIN retained_ship_library_owner_main
#include "ff5_library_owner_check.cpp"
namespace ship_owner {
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits={}){owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_ship_library_owner_main())return 1;using namespace owner;using namespace ship_owner;
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);scene(270,{1,10,21});const auto observed=fixture::native_control_reads;step();
 check(targets.size()==1&&targets.front().goal.entity_id==-1&&targets.front().goal.cell==field::baked_route::Cell{1,10,13}&&targets.front().instruction==L"Face West. Operate the switch.","fresh ship requests the actual rider-side standing point and cardinal facing");check(fixture::native_control_reads==observed,"ship captures copied flags and position without scanning controls");respond();
 position=targets.front().goal.cell;const auto old_reads=fixture::native_control_reads,old_routes=routes;const auto quiet=spoken.size();for(int i=0;i<200;++i)actions::run("GameModule.PuzzleGuideMove");
 check(fixture::native_control_reads==old_reads&&routes==old_routes&&spoken.size()==quiet&&tasks.empty(),"standing/facing input never advances,scans or repeats guidance before native switch use");
 ready=false;busy=false;for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<100;++i)pulse();check(tasks.size()==1&&routes==old_routes,"native switch animation waits in one bounded continuation without layout work");
 scene(270,{1,10,9},{301});ready=true;pulse();drain();check(targets.size()==1&&targets.front().goal.cell==field::baked_route::Cell{1,5,14},"full actual carrier position and receipt advance to the next reachable switch side");respond();
 // Use a legally reachable lower platform state. A307=1 lower standing
 // point is obstructed by that layout and correctly has no authored policy.
 scene(270,{1,15,18},{304});step();check(targets.front().goal.cell==field::baked_route::Cell{1,15,18},"native carrier retains its lower rider-side objective");respond();scene(270,{1,15,12},{304,307});actions::run("GameModule.FieldStateChanged");pulse();drain();check(targets.front().goal.entity_id==16,"native ready observes the carried player and exposes the real exit");respond();
 ready=false;event(events::Kind::MapChanging);scene(271,{1,15,22});event(events::Kind::MapChanged);pulse();check(queries::is("GameModule.PuzzleGuideActive"),"departure alone cannot finish before full destination readiness");ready=true;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","full native271 landing finishes the passage journey");
 scene(270,{1,10,21});step();auto stale=std::move(publish);publish={};nav_guidance=false;other_guidance=true;actions::run("GameModule.PuzzleGuidePolicyChanged");const auto stopped=spoken.size();field::PathInfo p;p.success=true;p.guidance_index=0;stale(std::move(p));check(spoken.size()==stopped&&!beacon,"Navigation disable retires held route and preview");const auto count=routes;step();check(routes==count&&spoken.back()==L"Navigation puzzle guidance is off","Navigation disable blocks ship independently");nav_guidance=true;other_guidance=false;step();check(routes==count+1,"Other disable keeps the ship navigation solution");respond();nav_guidance=other_guidance=true;
 ff5::library::stop(false);scene(269,{1,5,5});check(!queries::is("GameModule.HasPuzzleGuide"),"ordinary reset room does not advertise unowned passage guidance");
 check(fixture::wrong_owner==0,"native observation and speech retain their actual thread owner");std::printf("Ship native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
