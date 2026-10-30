// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact world/maze/Tower route witnesses, without a game launch or native probe.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const char*p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto data=read("data/ff3/maps.bin");if(!baked::load(data.data(),data.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;const auto* world=baked::submap_by_map_id(4);if(!world)return 2;
 auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(1);flags->treasure.fill(1);flags->global.fill(1);flags->current_map_id=4;flags->current_area_id=1;
 int failures=0;const auto expect=[&](bool ok,const char*why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 for(const auto start:std::vector<baked_route::Cell>{{1,151,74},{1,46,57},{1,197,42},{1,197,38}})for(int model:{0,9,10}){
  auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
  snapshot->map_id=4;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
  snapshot->position=world_of_cell(*world,start);snapshot->vehicle={model!=0,model,model?baked::transport(model)->type:0,{}};
  snapshot->goal.map_id=4;snapshot->goal.cell={1,197,37};snapshot->goal.entity_id=175;snapshot->kind=TargetKind::Transition;
  Planned tower,maze;CrossLeg graph;
  std::thread worker([&]{tower=plan_snapshot(*snapshot);SnapshotScope scope(*snapshot);graph=find_across(nullptr,nullptr,snapshot->goal);
   snapshot->goal.cell={1,197,41};snapshot->goal.entity_id=173;maze=plan_snapshot(*snapshot);});worker.join();
  std::printf("FFIII start(%d,%d) model%d: Tower%d next%d reason=%ls; graph%d next%d visited%d; Maze%d next%d\n",start.x,start.y,model,
   tower.path.success,tower.path.next_map_id,tower.path.reason.c_str(),graph.found,graph.next_map_id,graph.visited_maps,maze.path.success,maze.path.next_map_id);
  if(model==10&&start.x==151)expect(tower.path.success,"Invincible at the cleared-statue approach must route through the real maze to Crystal Tower");
  if(model==10&&start.x==151){
   expect(tower.path.next_map_id==299&&!tower.path.walk_phases.empty(),"first leg enters the actual southern maze entrance");
   expect(tower.pending.valid&&tower.pending.goal.map_id==4&&tower.pending.goal.cell==baked_route::Cell{1,197,37}&&tower.pending.goal.entity_id==175,
    "landing/maze first leg retains the original Tower destination rather than substituting the entry door");
   bool land=false;for(const auto& phase:tower.path.walk_phases)land|=phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;
   expect(land&&tower.path.walk_phases.back().cells.back()==baked_route::Cell{1,197,41},"world detour retains native landing and exact first door");
  }
 }
 auto make_capture=[&](int model,baked_route::Cell start){
  auto capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*flags,fixture::records));auto& route=capture->route;
  route.map_id=4;route.submap=world;route.layer=1;route.locale=L"en";route.position=world_of_cell(*world,start);
  route.vehicle={model!=0,model,model?baked::transport(model)->type:0,{}};return capture;
 };
 for(int model:{9,10}){
  auto capture=make_capture(model,{1,151,74});std::vector<ExitTarget> targets;
  for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::GotoMap&&capture->route.state.active(*world,object))
   targets.push_back({world_of_cell(*world,{1,object.tx,object.ty}),4,object.a,{1,object.tx,object.ty},TargetKind::Transition});
  std::vector<int> distances;const auto start=std::chrono::steady_clock::now();
  std::thread list_worker([&]{distances=exit_distances(*capture,targets,{});});list_worker.join();
  int tower=-1,maze=-1;for(size_t i=0;i<targets.size();++i){if(targets[i].destination==305)tower=distances[i];if(targets[i].destination==299)maze=distances[i];}
  std::printf("World%d full%d-exit eligibility: Tower%d Maze%d %.3fms owned worker\n",model,static_cast<int>(targets.size()),tower,maze,
   std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
  expect(model==10?tower>=0:tower<0,"list eligibility matches the real Invincible-only initial mountain approach");
  filter_worker::FilterInput input;input.navigation=capture;input.state=std::make_shared<map_state::Resolver>(capture->route.state);
  input.world=true;input.current_map=4;input.reachable_only=true;input.story_gates=true;input.events=input.exits=true;
  input.sort=true;input.player=capture->route.position;
  for(const auto& object:baked::grids(*world).objects)if(object.type==baked::ObjType::GotoMap&&object.a==305){
   filter_worker::FilterRow row;row.index=input.rows.size();row.kind=Kind::MapExit;row.category=Category::MapExits;
   row.position=row.numbering_position=world_of_cell(*world,{1,object.tx,object.ty});row.cell_valid=true;
   row.cell_x=object.tx;row.cell_y=object.ty;row.cell_layer=1;row.entity_id=object.entity_id();row.raw_name=baked::object_identifier(object);
   row.destination=object.a;row.name=L"Exit to Crystal Tower";input.rows.push_back(std::move(row));
  }
  filter_worker::Categories list;std::thread categories([&]{list=filter_worker::build_categories(input);});categories.join();
  bool tower_listed=false;for(size_t index:list.views[static_cast<size_t>(Category::MapExits)].order)tower_listed|=input.rows[index].destination==305;
  expect(tower_listed==(model==10),"actual Current map with vehicles retains Crystal Tower after story and reachability checks");
 }
 // After entering the first maze floor, the ordinary cross-floor planner keeps
 // the original world Tower goal. Returning north of the maze is then local.
 for(int map:{299,304}){
  const auto* maze_map=baked::submap_by_map_id(map);const baked::Object* arrival=nullptr;
  for(const auto& object:baked::grids(*maze_map).objects)if(object.type==baked::ObjType::PointIn&&object.a==1){arrival=&object;break;}
  expect(arrival!=nullptr,"real maze continuation has its native PointIn");if(!arrival)continue;
  auto copy=*flags;copy.current_map_id=map;auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(copy,fixture::records));
  snapshot->map_id=map;snapshot->submap=maze_map;snapshot->layer=arrival->layer;snapshot->locale=L"en";
  snapshot->position=world_of_cell(*maze_map,{arrival->layer,arrival->tx,arrival->ty});snapshot->goal.map_id=4;
  snapshot->goal.cell={1,197,37};snapshot->goal.entity_id=175;snapshot->kind=TargetKind::Transition;
  Planned next;std::thread continuation([&]{next=plan_snapshot(*snapshot);});continuation.join();
  expect(next.path.success&&next.path.next_map_id>0,"native maze-floor entry retains the final Tower destination and produces the next real leg");
 }
 auto cancel_capture=make_capture(10,{1,151,74});int calls=0;std::vector<ExitTarget> cancel_targets;
 for(const auto& o:baked::grids(*world).objects)if(o.type==baked::ObjType::GotoMap)cancel_targets.push_back({world_of_cell(*world,{1,o.tx,o.ty}),4,o.a,{1,o.tx,o.ty},TargetKind::Transition});
 std::vector<int> cancelled;std::thread cancellation([&]{cancelled=exit_distances(*cancel_capture,cancel_targets,[&]{return ++calls>40;});});cancellation.join();
 expect(calls>40,"whole world eligibility observes cancellation inside its existing graph/flood work");
 auto walking=make_capture(0,{1,197,42});walking->route.vehicle_shortcuts=true;walking->route.goal.map_id=4;
 walking->route.goal.cell={1,197,37};walking->route.goal.entity_id=175;walking->route.kind=TargetKind::Transition;
 Planned walking_path;std::thread walking_worker([&]{walking_path=plan_snapshot(walking->route);});walking_worker.join();
 expect(walking_path.path.success&&walking_path.path.next_map_id==299,"with-vehicles policy and no parked craft preserve ordinary walking into the maze");
 auto cleared=*flags;cleared.scenario.fill(0);for(int bit:{88,89,90,91})cleared.scenario[bit]=1;
 map_state::Resolver clear_state(cleared,fixture::records);int hidden_statues=0,relocated=0;
 for(const auto& object:baked::grids(*world).objects){
  if(object.type==baked::ObjType::Entity&&object.entity_id()>=187&&object.entity_id()<=200)hidden_statues+=!clear_state.active(*world,object);
  if(object.type==baked::ObjType::Event&&object.entity_id()>=202&&object.entity_id()<=205){const auto current=clear_state.effective(*world,object);relocated+=current.tx==145&&current.ty==59;}
 }
 std::printf("Cleared statue flags88..91: hidden%d relocated%d\n",hidden_statues,relocated);
 expect(hidden_statues==8&&relocated==4,"exact cleared flags already hide all eight statues and relocate all four native triggers");
 auto exact=std::make_unique<RouteSnapshot>(clear_state);exact->map_id=4;exact->submap=world;exact->layer=1;exact->locale=L"en";
 exact->position=world_of_cell(*world,{1,151,74});exact->vehicle={true,10,baked::transport(10)->type,{}};
 exact->goal.map_id=4;exact->goal.cell={1,197,37};exact->goal.entity_id=175;exact->kind=TargetKind::Transition;
 Planned exact_path;std::thread exact_worker([&]{exact_path=plan_snapshot(*exact);});exact_worker.join();
 expect(exact_path.path.success&&exact_path.path.next_map_id==299,"cleared-statue witness does not require arbitrary later flags to reach the first maze leg");
 expect(fixture::wrong_owner==0,"world/maze route work retains native pointer ownership");
 std::printf("Crystal Tower: %d failures\n",failures);return failures?1:0;
}
