// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
#define FFPR_LIBRARY_OWNER_MAIN retained_gogo_library_owner_main
#include "ff5_library_owner_check.cpp"
namespace gogo_owner {
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits={}){owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_gogo_library_owner_main())return 1;using namespace owner;using namespace gogo_owner;
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);nav_guidance=false;other_guidance=true;scene(485,{0,11,5},{346});const auto observed=fixture::native_control_reads;step();
 check(targets.size()==1&&targets.front().goal.map_id==490&&targets.front().goal.entity_id==3&&fixture::native_control_reads==observed,"Other-owned tower guide starts with the real air supply and no native actor scan");respond();
 scene(490,{0,7,6},{346,347});step();check(targets.front().goal.map_id==494&&targets.front().goal.entity_id==8,"full native air receipt selects the actual Gogo encounter");respond();
 ready=false;menu=true;const auto reads=fixture::native_control_reads;actions::run("GameModule.PuzzleGuide");check(fixture::native_control_reads==reads&&spoken.back().find(L"second option")!=std::wstring::npos&&spoken.back().find(L"take no actions")!=std::wstring::npos,"native warning choice repeats its complete correct answer without a fresh observation");
 battle=true;menu=false;event(events::Kind::BattleEnd);const auto pending=spoken.size();for(int i=0;i<30;++i)pulse();check(queries::is("GameModule.PuzzleGuideActive")&&spoken.size()==pending&&tasks.size()==1,"battle/full job scene cannot finish or poll controls while native readiness is blocked");
 battle=false;scene(494,{0,10,9},{166,346,347});ready=true;pulse();drain();check(targets.front().goal.map_id==485&&targets.front().goal.entity_id==2&&queries::is("GameModule.PuzzleGuideActive"),"full native Mime receipt retains the actual return journey");respond();
 event(events::Kind::MapChanging);scene(485,{0,11,16},{166,346,347});event(events::Kind::MapChanged);pulse();drain();check(targets.front().goal.map_id==485,"tower ascent retains the actual upper exit");respond();
 ready=false;event(events::Kind::MapChanging);scene(622,{1,9,10},{166,346});event(events::Kind::MapChanged);const auto returning=spoken.size();for(int i=0;i<20;++i)pulse();check(queries::is("GameModule.PuzzleGuideActive")&&spoken.size()==returning,"outside timer-finish scene waits for full native readiness without early completion");
 ready=true;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","full safe return completes with Navigation off and Other on");
 scene(494,{0,10,9},{346,347});step();respond();event(events::Kind::MapChanging);scene(29,{1,0,0},{346,347});event(events::Kind::MapChanged);const auto early=spoken.size();pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.size()==early,"leaving before the real Mime receipt never certifies puzzle completion");
 scene(494,{0,10,9},{346,347});nav_guidance=true;other_guidance=false;const auto disabled=routes;step();check(routes==disabled&&spoken.back()==L"Non-navigation puzzle guidance is off","Other disable independently blocks the Gogo solution");nav_guidance=false;other_guidance=true;step();check(routes==disabled+1,"Navigation disable preserves Gogo guidance");respond();
 const auto move_reads=fixture::native_control_reads;for(int i=0;i<100;++i)actions::run("GameModule.PuzzleGuideMove");check(fixture::native_control_reads==move_reads,"ordinary movement adds no actor scan or timer polling");
 ff5::library::stop(false);nav_guidance=other_guidance=true;check(fixture::wrong_owner==0,"Gogo rule/route workers perform no native reads or speech");std::printf("Gogo native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
