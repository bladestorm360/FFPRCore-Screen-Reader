// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact FFIII scripted village entrance, through production route/filter paths.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string{std::istreambuf_iterator<char>(f),{}};};
 const auto bytes=read("data/ff3/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(4);const auto& grid=baked::grids(*world);
 int count=0;
 for(const auto& o:grid.objects)if(o.type==baked::ObjType::Warp&&o.entity_id()>=888&&o.entity_id()<=890&&baked::object_identifier(o)==L"ファルガバード入口"){
  ++count;expect(o.type==baked::ObjType::Warp&&o.a==144&&o.b==1,"native Falgabard transfer retains its actual destination and arrival");
  expect(baked::world_exit(o),"pure scripted village entrance is indexed as a named map destination");
  for(int vehicle:{0,8,9,10,11}){
   map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=4;
   RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=4;snapshot.submap=world;snapshot.layer=1;
   snapshot.position=world_of_cell(*world,{1,83,62});snapshot.locale=L"en";snapshot.auto_groups={3};snapshot.vehicle_shortcuts=true;
   if(vehicle){const auto* row=baked::transport(vehicle);snapshot.vehicle={true,vehicle,row->type,L""};}
   snapshot.goal.map_id=4;snapshot.goal.cell={1,o.tx,o.ty};snapshot.goal.entity_id=o.entity_id();snapshot.goal.raw_name=baked::object_identifier(o);snapshot.kind=TargetKind::Transition;
   Planned route;std::vector<int> eligible;
   std::thread worker([&]{route=plan_snapshot(snapshot);ExitSnapshot capture(true,snapshot.state);
    auto& copy=capture.route;copy.submap=world;copy.map_id=4;copy.layer=1;copy.position=snapshot.position;copy.vehicle=snapshot.vehicle;
    copy.auto_groups=snapshot.auto_groups;copy.goal=snapshot.goal;copy.kind=snapshot.kind;
    ExitTarget target{world_of_cell(*world,snapshot.goal.cell),0,144,snapshot.goal.cell};target.kind=TargetKind::Transition;eligible=exit_distances(capture,{target},{});});worker.join();
   std::printf("Falgabard entity%d (%d,%d) vehicle%d: route%d steps%d eligibility%d error=%ls\n",o.entity_id(),o.tx,o.ty,vehicle,route.path.success,route.path.step_count,eligible.empty()?-99:eligible[0],route.path.error.c_str());
  }
  map_state::Flags local_flags;local_flags.scenario.fill(1);local_flags.treasure.fill(0);local_flags.global.fill(0);local_flags.current_map_id=4;
  auto capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(local_flags,fixture::records));auto& local=capture->route;
  local.map_id=4;local.submap=world;local.layer=1;local.position=world_of_cell(*world,{1,36,100});local.locale=L"en";
  local.goal.map_id=4;local.goal.cell={1,o.tx,o.ty};local.goal.entity_id=o.entity_id();local.goal.raw_name=baked::object_identifier(o);local.kind=TargetKind::Transition;
  Planned arrived;filter_worker::Categories views;
  std::thread valley([&]{arrived=plan_snapshot(local);filter_worker::FilterInput input;input.current_map=4;input.world=true;input.navigation=capture;
    filter_worker::FilterRow row;row.kind=Kind::MapExit;row.category=Category::MapExits;row.name=L"Exit to Falgabard";row.entity_id=o.entity_id();
    row.raw_name=baked::object_identifier(o);row.position=world_of_cell(*world,local.goal.cell);row.numbering_position=row.position;row.destination=144;
    input.rows={row};views=filter_worker::build_categories(input);});valley.join();
  expect(arrived.path.success,"actual valley walking position reaches the native village entry");
  expect(views.views[static_cast<int>(Category::MapExits)].order.size()==1&&views.views[static_cast<int>(Category::Events)].order.empty(),
    "production reachability/category publication lists reachable Falgabard only in Map Exits");
 }
 expect(count==3,"all three native direction-sensitive entrance tiles are audited");
 expect(fixture::wrong_owner==0,"all planning uses owned data without native reads");
 std::printf("World entrance audit: %d failures\n",failures);return failures?1:0;
}
