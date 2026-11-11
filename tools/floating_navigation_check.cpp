// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual Floating Continent planner/list/browse episode with its real border
// actors. Timings cover owned computation, not fake native waits or live speech.
#define main retained_floating_route_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 auto bytes=read("data/ff3/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(1);if(!world)return 2;const auto& grid=baked::grids(*world);
 // Production does this on Data before readiness. The offline owner performs
 // the same immutable preparation before starting any planning worker.
 prepare_floating_connections();
 map_state::Flags flags;flags.scenario.fill(1);flags.global.fill(1);flags.treasure.fill(1);flags.current_map_id=1;flags.current_area_id=1;
 int failures=0;const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 expect(g_floating_connections&&g_floating_connections->world==world,"actual world1 connector facts prepare without native work");
 if(g_floating_connections) {
  baked_route::Reach proof;proof.submap=world;proof.width=grid.width;proof.height=grid.height;
  proof.dist[0].assign(static_cast<size_t>(grid.width)*grid.height,0xffff);
  int complete=0;
  for(const auto& [index,connection]:g_floating_connections->rooms) {
   const auto* room=baked::submap_at(index);expect(room&&!connection.unknown,"every current interior closure is complete");
   if(!room||connection.unknown)continue;++complete;
   std::fill(proof.dist[0].begin(),proof.dist[0].end(),uint16_t(0xffff));
   if(!connection.returns.empty()) {
    expect(!floating_connection_dominated(*room,proof,0),"an unknown/unreached return pocket always preserves its connector");
    for(const auto& cell:connection.returns)proof.dist[0][static_cast<size_t>(cell.y)*grid.width+cell.x]=1;
    expect(floating_connection_dominated(*room,proof,0),"all returns already reached at lower/equal cost prove a detour redundant");
    const auto& first=connection.returns.front();proof.dist[0][static_cast<size_t>(first.y)*grid.width+first.x]=2;
    expect(!floating_connection_dominated(*room,proof,0),"a potentially shorter interior shortcut is preserved even when both pockets are reachable");
   }
  }
  std::printf("Floating connections: %d complete real interior closures; missing pocket and cheaper shortcut controls retained\n",complete);
 }
 std::vector<const baked::Object*> doors;
 for(const auto& object:grid.objects)if(object.type==baked::ObjType::GotoMap&&object.a>0)doors.push_back(&object);
 expect(doors.size()>=3,"actual Floating Continent entrances are present");if(doors.empty())return 2;
 for(const int id:{0,8,11}){
  std::vector<size_t> reference_order;std::vector<std::wstring> reference_routes;
  for(const bool indexed:{false,true}){
  // Offline reference strategy changes only lookup metadata, never terrain,
  // objects, gates or routes. Both branches run the actual production consumer.
  const_cast<baked::Grids&>(grid).entity_index_prepared=indexed;
  auto source=std::make_shared<ExitSnapshot>(true,map_state::Resolver(flags,fixture::records));auto& s=source->route;
  s.map_id=1;s.submap=world;s.layer=1;s.position=world_of_cell(*world,{1,37,64});s.locale=L"en";
  // Native on-foot state is valid Player1, not a missing transportation model.
  s.vehicle={true,id?id:1,id?baked::transport(id)->type:1,{}};s.goal.map_id=1;s.kind=TargetKind::Transition;
  if(!id){
   auto geometry=s;SnapshotScope scope(geometry);auto foot=walk_request(*world,{1,37,64});world_mode(foot);Cell origin=foot.start;int best=INT_MAX;
   for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x){const int d=std::abs(x-37)+std::abs(y-64);Cell at{1,x,y};
    if(d<best&&baked_route::open(foot,grid,at)&&!baked_route::terminal_transition(foot,grid,at)){best=d;origin=at;}}
   expect(best<INT_MAX,"foot benchmark starts on a real standable world tile");s.position=world_of_cell(*world,origin);
   for(int model_id:{8,11})for(const auto& side:std::array<Cell,4>{{{1,0,1},{1,0,-1},{1,1,0},{1,-1,0}}}){
    const auto* row=baked::transport(model_id);Cell at{1,origin.x+side.x,origin.y+side.y};
    if(!baked_route::open(foot,grid,at)||!baked::group_lands(row->landing_group,baked::attribute(grid,at.x,at.y)))continue;
    const auto position=world_of_cell(*world,at);
    if(std::any_of(s.vehicles.begin(),s.vehicles.end(),[&](const auto& model){return model.pos.x==position.x&&model.pos.y==position.y;}))continue;
    VehicleOnMap model;model.transportation_id=model_id;model.type=row->type;model.pos=world_of_cell(*world,at);s.vehicles.push_back(model);break;
   }
   std::printf("Foot origin%d,%d parked%zu\n",origin.x,origin.y,s.vehicles.size());
  }
  expect(!exit_snapshot_stationary_inventory(*source),"an empty actor capture cannot certify a stationary inventory");
  for(const auto& object:grid.objects)if(const auto* rule=air_transitions::match(3,1,object)){
   NativeObstacle actor;actor.type=2;actor.id=object.entity_id();actor.cell={-1,object.tx,object.ty};actor.raw_name=baked::object_identifier(object);
   actor.trigger=true;actor.transportation_restricted=actor.transportation_complete=true;actor.transportation_ids=rule->mask;s.obstacles.push_back(std::move(actor));
  }
  expect(exit_snapshot_stationary_inventory(*source),"current world border capture qualifies only while no native NPC/party actor is observed");
  for(int map:{3,4}){s.map_id=map;s.submap=baked::submap_by_map_id(map);expect(exit_snapshot_stationary_inventory(*source),"qualified surface/underwater capture also reuses a stationary inventory");}
  s.map_id=2;s.submap=baked::submap_by_map_id(2);expect(!exit_snapshot_stationary_inventory(*source),"other world identities retain ordinary refresh behavior");
  s.map_id=1;s.submap=world;
  {NativeObstacle actor;actor.npc=true;s.obstacles.push_back(actor);expect(!exit_snapshot_stationary_inventory(*source),"even an unlisted live NPC disables stationary inventory reuse");s.obstacles.pop_back();}
  {NativeObstacle actor;actor.npc=true;actor.navigation_available=false;s.obstacles.push_back(actor);expect(exit_snapshot_stationary_inventory(*source),"native non-actionable world puppet does not disable stationary view reuse");s.obstacles.pop_back();}
  if(indexed) {
   const auto original=world_origin_key(s,false,false);auto changed=original;
   changed.position.x+=16;expect(!(changed==original),"world origin cache rejects movement");changed=original;
   ++changed.vehicle.id;expect(!(changed==original),"world origin cache rejects boarding/mode changes");changed=original;
   changed.gates=true;expect(!(changed==original),"world origin cache separates actual gate policies");changed=original;
   changed.npcs=true;expect(!(changed==original),"world origin cache separates actual NPC policies");changed=original;
   changed.shortcuts=!changed.shortcuts;expect(!(changed==original),"world origin cache separates vehicle-assistance policy");changed=original;
   changed.owner=nullptr;expect(!(changed==original),"another owned capture cannot borrow cached reach");
   if(!original.models.empty()){changed=original;changed.models.front().position.y+=16;expect(!(changed==original),"world origin cache rejects changed parked craft positions");}
  }
  filter_worker::FilterInput input;input.current_map=1;input.world=input.sort=input.exits=true;input.reachable_only=false;
  input.navigation=source;input.state=std::make_shared<map_state::Resolver>(flags,fixture::records);input.player=s.position;
  for(const auto* object:doors){filter_worker::FilterRow row;row.index=input.rows.size();row.kind=Kind::MapExit;row.category=Category::MapExits;
   row.position=row.numbering_position=world_of_cell(*world,{1,object->tx,object->ty});row.name=std::to_wstring(object->entity_id());
   row.destination=object->a;row.entity_id=object->entity_id();row.raw_name=baked::object_identifier(*object);row.cell_valid=true;row.cell_layer=1;row.cell_x=object->tx;row.cell_y=object->ty;
   input.rows.push_back(std::move(row));
  }
  std::thread worker([&]{
   const auto cold=s; // category work must not warm a later explicit request's resolver
   if(indexed) {
    auto profile=cold;SnapshotScope scope(profile);
    const auto start=std::chrono::steady_clock::now();
    const auto live=live_blockers(*world,grid,{});const auto blocked=std::chrono::steady_clock::now();
    auto movement=walk_request(*world,cell_of(*world,profile.position,1));world_mode(movement);
    movement.blocked=live.hard();movement.directional=live.directional;live.apply_transport(movement);
    movement.live_hazards=movement.live_traps=true;movement.hazards=live.triggers;movement.traps=live.traps;
    const auto prepared=std::chrono::steady_clock::now();const auto reach=baked_route::flood(movement);
    const auto flooded=std::chrono::steady_clock::now();
    movement.goals={{1,doors.front()->tx,doors.front()->ty}};const auto route=baked_route::find(movement);
    const auto found=std::chrono::steady_clock::now();
    std::printf("Floating components mode%d blockers%.3fms prepare%.3fms flood%.3fms Astar%.3fms route%d reachable%zu\n",id,
      std::chrono::duration<double,std::milli>(blocked-start).count(),std::chrono::duration<double,std::milli>(prepared-blocked).count(),
      std::chrono::duration<double,std::milli>(flooded-prepared).count(),std::chrono::duration<double,std::milli>(found-flooded).count(),route.found,reach.dist[1].size());
    auto group_capture=*source;
    std::map<int,int> counts;for(const auto* door:doors)++counts[door->a];
    std::vector<ExitTarget> repeated;for(const auto* door:doors)if(counts[door->a]>1)
      repeated.push_back({world_of_cell(*world,{1,door->tx,door->ty}),0,door->a,{1,door->tx,door->ty},TargetKind::Transition});
    const auto grouping=std::chrono::steady_clock::now();const auto distances=exit_distances(group_capture,repeated,{});
    const auto grouped=std::chrono::steady_clock::now();
    auto air_capture=*source;const auto air=air_transition_target(air_capture,{},{},{},false,false);
    const auto aired=std::chrono::steady_clock::now();
    std::printf("Floating category stages mode%d repeated%zu grouping%.3fms provider%.3fms\n",id,distances.size(),
      std::chrono::duration<double,std::milli>(grouped-grouping).count(),std::chrono::duration<double,std::milli>(aired-grouped).count());
   }
   const auto begin=std::chrono::steady_clock::now();const auto categories=filter_worker::build_categories(input);
   const auto list_end=std::chrono::steady_clock::now();
   if(!indexed)reference_order=categories.views[0].order;else expect(reference_order==categories.views[0].order,"prepared ID index preserves the exact complete category order");
   std::printf("Floating indexed%d mode%d actors%zu objects%zu category%.3fms rows",indexed,id,s.obstacles.size(),grid.objects.size(),std::chrono::duration<double,std::milli>(list_end-begin).count());
   for(auto n:categories.views[0].order)std::printf("/%zu",n);std::printf("\n");
   for(size_t index=0;index<doors.size();++index){
    s.goal.cell={1,doors[index]->tx,doors[index]->ty};s.goal.entity_id=doors[index]->entity_id();s.goal.raw_name=baked::object_identifier(*doors[index]);
    auto selected=cold,browsed=cold;selected.goal=s.goal;browsed.goal=s.goal;
    const auto start=std::chrono::steady_clock::now();const auto route=plan_snapshot(selected);
    const auto middle=std::chrono::steady_clock::now();const auto browse=plan_browse_snapshot(browsed);
    std::wstring fingerprint=std::to_wstring(route.path.success)+L":"+std::to_wstring(route.path.step_count)+L":"+std::to_wstring(browse.route.direct)+L":"+std::to_wstring(browse.route.via_map_id)+L":"+browse.route.reason;
    for(const auto& phase:route.path.walk_phases){fingerprint+=L"/"+std::to_wstring(phase.transportation_id)+L":"+phase.action_before+L":"+phase.action_after;for(const auto& cell:phase.cells)fingerprint+=L"/"+std::to_wstring(cell.layer)+L","+std::to_wstring(cell.x)+L","+std::to_wstring(cell.y);}
    if(!indexed)reference_routes.push_back(fingerprint);else expect(reference_routes[index]==fingerprint,"prepared ID index preserves selected phases, every tile and complete browse result");
    std::printf("Floating mode%d target%zu selected%.3fms browse%.3fms result%d/%d steps%d phases%zu via%d reason%ls\n",id,index,
      std::chrono::duration<double,std::milli>(middle-start).count(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-middle).count(),
      route.path.success,browse.route.direct,route.path.step_count,route.path.walk_phases.size(),browse.route.via_map_id,browse.route.reason.c_str());
   }
  });worker.join();
  }
 }
 expect(fixture::wrong_owner==0,"Floating Continent list/selected/browse compute uses no native API");
 std::printf("Floating navigation: %d failures; owner%d\n",failures,fixture::wrong_owner.load());return failures?1:0;
}
