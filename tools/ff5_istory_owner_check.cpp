// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp ../games/ff5/pyramid_model.cpp ../games/ff5/pyramid_observation.cpp ../games/ff5/jachol_model.cpp ../games/ff5/jachol_observation.cpp ../games/ff5/mechanics_model.cpp
// Actual native-ready owner/worker boundary,including the sheep-yard return.
#define FFPR_LIBRARY_OWNER_MAIN retained_istory_library_owner_main
#include "ff5_library_owner_check.cpp"
namespace istory_owner {
void scene(int map,field::baked_route::Cell at,std::initializer_list<int> bits={}){owner::flags(bits);fixture::storage.map_id=map;owner::native_scene(map);owner::position=at;}
void step(){actions::run("GameModule.PuzzleGuide");owner::drain();}
}
int main(){
 if(retained_istory_library_owner_main())return 1;
 using namespace owner;using namespace istory_owner;
 const auto bytes=fixture::read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size()))return 2;
 ff5::library::stop(false);scene(118,{1,32,47});const auto observed=fixture::native_control_reads;step();
 check(targets.size()==1&&targets.front().goal.entity_id==31&&fixture::native_control_reads==observed,"Istory captures copied flags and position without an actor scan");respond();
 scene(118,{1,26,20},{296});step();check(targets.front().goal.entity_id==28,"first native contact receipt advances to the second actual trigger");respond();
 scene(118,{1,28,20},{296,297});step();check(targets.front().goal.entity_id==33,"two native receipts select the remaining trigger");respond();
 scene(118,{1,26,22},{296,297,298});step();check(targets.front().goal.entity_id==26,"all three receipts expose the final actual contact");respond();
 ready=false;menu=true;event(events::Kind::DialogueClosed);const auto waiting=spoken.size();for(int i=0;i<40;++i)pulse();
 check(queries::is("GameModule.PuzzleGuideActive")&&spoken.size()==waiting,"mid-scene spell grant cannot finish before full native299 and readiness");
 scene(118,{1,28,22},{296,297,298,299});ready=true;menu=false;pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive")&&spoken.back()==L"Puzzle guide finished","full final299 retires the guide after native readiness");
 scene(118,{1,33,19});step();check(targets.front().goal.entity_id==11&&targets.front().goal.cell==field::baked_route::Cell{2,51,11},"actual sheep landing routes to its native upper exit");respond();
 event(events::Kind::MapChanging);ready=false;scene(1,{1,0,0});event(events::Kind::MapChanged);const auto blocked=fixture::native_control_reads;for(int i=0;i<30;++i)pulse();check(tasks.size()==1&&fixture::native_control_reads==blocked,"world transition waits without polling native actors");ready=true;pulse();drain();check(targets.front().goal.map_id==118&&targets.front().goal.entity_id==31,"owned guide continues through the real world-to-town entrance");respond();
 event(events::Kind::MapChanging);scene(118,{1,32,47});event(events::Kind::MapChanged);pulse();drain();check(targets.front().goal.entity_id==31,"town return resumes unchanged saved progress");auto stale=std::move(publish);publish={};
 nav_guidance=false;other_guidance=true;actions::run("GameModule.PuzzleGuidePolicyChanged");const auto retired=spoken.size();field::PathInfo old;old.success=true;old.guidance_index=0;old.description=L"Stale";stale(std::move(old));check(spoken.size()==retired&&!beacon,"policy change retires the pending route and preview");const auto disabled=routes;step();check(routes==disabled&&spoken.back()==L"Navigation puzzle guidance is off","Navigation disable blocks the flower solution independently");
 nav_guidance=true;other_guidance=false;step();check(routes==disabled+1,"Other disable leaves flower navigation available");respond();nav_guidance=other_guidance=true;
 ff5::library::stop(false);scene(1,{1,0,0});check(!queries::is("GameModule.HasPuzzleGuide"),"world map alone cannot advertise or start an Istory guide");
 scene(118,{1,32,47});step();respond();event(events::Kind::MapChanging);scene(119,{1,0,0});event(events::Kind::MapChanged);pulse();drain();check(!queries::is("GameModule.PuzzleGuideActive"),"unrelated house transition clears unlisted transit ownership");
 check(fixture::wrong_owner==0,"no rule selection or route worker performs native reads or speech");std::printf("Istory native owner:%d total cases/%d failures\n",cases,fixture::failures);return fixture::failures?1:0;
}
