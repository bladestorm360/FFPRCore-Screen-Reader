// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Six real FFV underwater entries must retain mounted submarine access when
// reclassified as ordinary exits. Native scripts own disembarking inside.
#define main retained_context_submarine_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff5/maps.bin");
 const auto before_bytes=read("build_check/silent_context_20261029/before/ff5_maps.bin");json::Value reviews;
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("tools/world_event_reviews.json"),reviews))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 int failures=0,cases=0;double total=0,peak=0;std::map<int,bool> before_foot;
 auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 for(bool baseline:{true,false}){
  const auto& selected_bytes=baseline?before_bytes:bytes;if(!baked::load(selected_bytes.data(),selected_bytes.size()))return 2;
 for(const auto& row:reviews.arr){
  const auto* entry=row.get(L"vehicle_entry");if(!entry||entry->s!=L"submarine")continue;if(!baseline)++cases;
  const auto* map=baked::submap_by_name(row.get(L"submap")->s);if(!map)return 2;const auto& grid=baked::grids(*map);
  const baked::Object* warp=nullptr;for(const auto& o:grid.objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==int(row.get(L"entity_id")->n)&&baked::object_identifier(o)==row.get(L"raw")->s)warp=&o;
  if(!baseline)expect(warp&&baked::world_exit(*warp)&&baked::submarine_entry(*warp),"exact reviewed underwater entry is indexed as a mounted map exit");if(!warp)continue;
  const int id=baked::name_of(*map)==L"Map_10030"?8:13;const auto* model=baked::transport(id);
  expect(model&&model->type==6,"native submarine model and group are available");if(!model)continue;
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=map->first_map_id;
  std::thread worker([&]{
   const auto started=std::chrono::steady_clock::now();RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
   snapshot.submap=map;snapshot.map_id=map->first_map_id;snapshot.layer=1;snapshot.locale=L"en";
   snapshot.vehicle={true,id,model->type,{}};snapshot.goal.map_id=map->first_map_id;snapshot.goal.cell={1,warp->tx,warp->ty};
   snapshot.goal.entity_id=warp->entity_id();snapshot.goal.raw_name=baked::object_identifier(*warp);snapshot.kind=TargetKind::Transition;
   baked_route::Request movement;movement.submap=map;movement.transportation_id=id;movement.vehicle_group=model->group;movement.vehicle_type=model->type;
   std::optional<Cell> start;
   for(const auto& delta:std::vector<std::pair<int,int>>{{0,-1},{1,0},{0,1},{-1,0}}){
    Cell at{1,(warp->tx+delta.first+grid.width)%grid.width,(warp->ty+delta.second+grid.height)%grid.height};if(baked_route::open(movement,grid,at)){start=at;break;}
   }
   expect(start.has_value(),"real submarine movement has a neighboring water approach");if(!start)return;
   snapshot.position=world_of_cell(*map,*start);
   auto walking=snapshot;walking.vehicle={};const auto foot=plan_snapshot(walking);
   if(baseline){before_foot[warp->entity_id()]=foot.path.success;return;}
   // Player1 movement layers also exist in these underwater bakes. A forced
   // hypothetical on-foot capture is not proof of an illegal native state.
   // Preserve its actual previous routing result rather than invent a new
   // vehicle requirement; normally the native entry is reached in submarine.
   expect(before_foot.count(warp->entity_id())&&foot.path.success==before_foot[warp->entity_id()],"reclassification preserves the actual existing on-foot route result");
   const auto selected=plan_snapshot(snapshot);
   expect(selected.path.success,"selected real underwater exit is reachable aboard the native submarine");
   for(const auto& phase:selected.path.walk_phases)expect(phase.action_after!=L"land","entry script owns dismounting; planner does not order a submarine landing");
   ExitSnapshot captured(true,snapshot.state);auto& copy=captured.route;copy.submap=map;copy.map_id=snapshot.map_id;copy.layer=1;
   copy.position=snapshot.position;copy.vehicle=snapshot.vehicle;copy.locale=L"en";
   const auto eligibility=exit_distances(captured,{{world_of_cell(*map,snapshot.goal.cell),0,warp->a,snapshot.goal.cell}},{});
   expect(eligibility.size()==1&&eligibility[0]>=0,"actual exit batch retains mounted access for destination publication");
   const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();total+=ms;peak=std::max(peak,ms);
  });worker.join();
 }
 }
 expect(cases==6,"all six real underwater entries are covered");expect(fixture::wrong_owner==0,"all route/batch decisions use owned worker data");
 std::printf("FFV contextual submarine entries: %d cases, %d failures, owner%d, total%.3fms peak%.3fms owned compute; no frame/audio claim\n",cases,failures,fixture::wrong_owner.load(),total,peak);
 return failures?1:0;
}
