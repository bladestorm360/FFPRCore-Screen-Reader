// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Full native teleport ends in Baron at6,6. The intermediate world is a scene,
// not a walking leg. Real source/destination grids and actual planner are used.
#define main retained_devil_route_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff4/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff4/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF4;g_loaded=g_prepared=true;
 const auto* source=baked::submap_by_name(L"Map_20061_9");const auto* baron=baked::submap_by_map_id(29);if(!source||!baron)return 2;
 int failures=0,plans=0;auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 for(int enabled:{0,1})for(int arrival_scene:{0,1}){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[29]=enabled;flags.scenario[33]=arrival_scene;
  flags.current_map_id=source->first_map_id;
  std::thread worker([&]{
   map_state::Resolver state(flags,fixture::records);const auto* entry=point_in(*source,1,state);if(!entry){++failures;return;}
   RouteSnapshot request(state);request.map_id=source->first_map_id;request.submap=source;request.layer=entry->layer;request.locale=L"en";
   request.position=world_of_cell(*source,{entry->layer,entry->tx,entry->ty});request.goal.map_id=29;request.goal.cell={0,6,6};request.kind=TargetKind::Spot;
   const auto output=plan_snapshot(request);
   if(!enabled){expect(!output.path.success,"locked native Devil's Road does not grant a route");return;}
   expect(output.path.success&&output.path.next_map_id==29,"actual source entrance routes through the complete native teleport to Baron");
   const auto& receipt=output.pending.incoming_transition;
   expect(receipt&&receipt->destination==29&&receipt->entity_id==5&&receipt->arrival==Cell{0,6,6},"pending receipt uses the final player position, not intermediate world or native6,12");
   plans+=output.path.success&&output.path.next_map_id==29;
   if(!receipt)return;
   flags.scenario[30]=1;flags.current_map_id=29;
   RouteSnapshot arrived(map_state::Resolver(flags,fixture::records));arrived.map_id=29;arrived.submap=baron;arrived.layer=0;arrived.locale=L"en";
   arrived.position=world_of_cell(*baron,{0,6,6});arrived.goal=request.goal;arrived.kind=TargetKind::Spot;arrived.pending=output.pending;
   const auto resumed=plan_snapshot(arrived);expect(resumed.path.success&&resumed.path.step_count==0,"fresh final map capture retains the selected destination without an invented walk");
   const auto* original=point_in(*baron,1,arrived.state);expect(original&&original->tx==6&&original->ty==12,"ordinary Baron PointIn1 remains at its native unchanged position");
  });worker.join();
 }
 expect(plans==2,"both native arrival story branches use the same proven final landing");expect(fixture::wrong_owner==0,"teleport proof uses owned worker state only");
 std::printf("Devil's Road: %d/2 actual plans, %d failures, owner%d; gameplay timing separate\n",plans,failures,fixture::wrong_owner.load());return failures?1:0;
}
