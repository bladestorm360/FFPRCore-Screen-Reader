// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_desert_library_owner_main
#include "ff5_library_owner_check.cpp"
namespace desert_owner {
void scene(field::baked_route::Cell at,std::initializer_list<int> bits){owner::flags(bits);fixture::storage.map_id=286;owner::native_scene(286);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_desert_library_owner_main())return 1;using namespace owner;using namespace desert_owner;
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);scene({2,53,8},{58});const auto reads=fixture::native_control_reads;step();
 check(targets.size()==1&&targets.front().goal.cell==field::baked_route::Cell{2,53,13}&&targets.front().instruction.find(L"first option")!=std::wstring::npos,"native north entry offers actual Sandworm staging and first choice");check(fixture::native_control_reads==reads,"desert captures copied flags/position without native actor scans");respond();
 position={2,53,13};const auto old_routes=routes;const auto quiet=spoken.size();for(int i=0;i<200;++i)actions::run("GameModule.PuzzleGuideMove");check(routes==old_routes&&spoken.size()==quiet&&tasks.empty(),"arrival and movement cannot trigger the encounter or advance guidance");
 ready=false;for(int i=0;i<30;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<100;++i)pulse();check(tasks.size()==1&&routes==old_routes&&fixture::native_control_reads==reads,"intro/battle preparation waits for full native readiness without actor polling");
 scene({2,53,13},{58,287});ready=true;pulse();drain();check(targets.size()==1&&targets.front().goal.entity_id==61&&targets.front().goal.cell==field::baked_route::Cell{2,55,13},"declined first choice exposes actual moved retry NPC");respond();
 battle=true;ready=false;event(events::Kind::BattleEntering);scene({2,53,13},{58,59,287});event(events::Kind::BattleEnd);for(int i=0;i<30;++i)pulse();check(routes==old_routes+1,"early completion receipt cannot publish inside battle/callback");battle=false;ready=true;pulse();drain();check(targets.size()==1&&ff5::library::rule_id==L"desert_crossing"&&targets.front().instruction==L"Let the current carry you, then request guidance again.","complete native-ready callback changes from encounter to actual current navigation");respond();
 const auto after=routes;for(int i=0;i<200;++i)actions::run("GameModule.PuzzleGuideMove");check(routes==after&&fixture::native_control_reads==reads,"current motion does not solve,scan or advance on every movement event");
 other_guidance=false;actions::run("GameModule.PuzzleGuidePolicyChanged");step();check(routes==after+1,"Other disable retains Navigation currents");respond();nav_guidance=false;other_guidance=true;actions::run("GameModule.PuzzleGuidePolicyChanged");const auto stopped=routes;step();check(routes==stopped,"Navigation disable suppresses completed-encounter current directions");
 nav_guidance=other_guidance=true;ff5::library::stop(false);scene({2,49,57},{58,59,287});step();check(targets.size()==1&&targets.front().goal.entity_id==64,"final leg is the real south exit");respond();ready=false;event(events::Kind::MapChanging);fixture::storage.map_id=1;native_scene(1);event(events::Kind::MapChanged);ready=true;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive"),"ordinary world arrival retires the guide without requiring a guessed world variant");
 check(fixture::wrong_owner==0,"all native observation and speech stay on their actual owner");std::printf("Desert native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
