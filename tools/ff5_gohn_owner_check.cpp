// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_gohn_library_owner_main
#include "ff5_library_owner_check.cpp"
namespace gohn_owner {
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits){owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_gohn_library_owner_main())return 1;using namespace owner;using namespace gohn_owner;const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);scene(287,{1,36,49},{59});const auto observed=fixture::native_control_reads;step();check(targets.size()==1&&targets[0].goal.entity_id==5,"Gohn starts at actual first sighting");respond();
 ready=false;for(int i=0;i<20;++i)actions::run("GameModule.FieldStateChanged");for(int i=0;i<80;++i)pulse();check(tasks.size()==1&&fixture::native_control_reads==observed,"full sighting waits in one continuation without native actor polling");
 scene(287,{1,36,45},{59,60});ready=true;pulse();drain();check(targets.size()==1&&targets[0].goal.entity_id==6,"native60 and actual player return advance to the second sighting");respond();
 scene(287,{1,35,37},{59,60,365});step();check(targets.size()==5,"native365 offers all five equivalent physical middle contacts");respond();
 scene(287,{1,22,34},{59,60,365,367});step();check(targets.size()==1&&targets[0].goal.entity_id==17,"western middle receipt advances without requiring eastern sighting");respond();
 scene(287,{1,35,34},{59,60,365,367,368});step();check(targets.size()==1&&targets[0].goal.entity_id==10,"last sighting still routes to the final collapse trigger");respond();const auto routes_before=routes;
 ready=false;event(events::Kind::MapChanging);scene(288,{1,23,4},{59,60,365,367,368});event(events::Kind::MapChanged);for(int i=0;i<80;++i)pulse();check(queries::is("GameModule.PuzzleGuideActive")&&routes==routes_before,"underground loading/cinematic starting point cannot complete or re-route the guide early");
 scene(288,{1,17,10},{59,60,61,365,367,368});ready=true;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","only native-ready full61 at final underground landing completes the search");check(fixture::native_control_reads==observed,"all Gohn stages use copied flags/position without actor scans");
 scene(287,{1,36,49},{59});nav_guidance=false;other_guidance=true;const auto count=routes;step();check(routes==count&&spoken.back()==L"Navigation puzzle guidance is off","Navigation independently disables city guidance");nav_guidance=true;other_guidance=false;step();check(routes==count+1,"Other disable retains city navigation");respond();nav_guidance=other_guidance=true;
 check(fixture::wrong_owner==0,"native reads and speech retain their original owner");std::printf("Gohn native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
