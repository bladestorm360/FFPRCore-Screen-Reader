// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual production planning and filtering, real bake, copied native collision.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(int argc,char** argv) {
 using namespace field;using namespace baked_nav;
 std::ifstream file("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(file),{}};
 if(!baked::load(bytes.data(),bytes.size()))return 2;
 std::ifstream sf("data/ff3/mapstate.json",std::ios::binary);std::string state_bytes{std::istreambuf_iterator<char>(sf),{}};
 if(!json::parse(state_bytes,fixture::records))return 3;
 fixture::game_id=game::Id::FF3;fixture::current_map=4;g_loaded=g_prepared=true;
 const auto* sm=baked::submap_by_map_id(4);if(!sm)return 4;
 map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=4;
 // Optional read-only saved-state evidence stays outside Git. Canonical builds
 // retain the deterministic synthetic cases; investigations can exercise all
 // copied real scenario/treasure/global/local words without embedding a save.
 if(argc>1) {
  std::ifstream saved(argv[1],std::ios::binary);std::string text{std::istreambuf_iterator<char>(saved),{}};json::Value data;
  if(!json::parse(text,data))return 6;
  auto bits=[&](const wchar_t* key,auto& bank){const auto* a=data.get(key);if(!a)return;
   for(size_t i=0;i<bank.size()&&i/32<a->arr.size();++i)bank[i]=(static_cast<uint32_t>(a->arr[i/32].n)>>(i%32))&1;};
  auto values=[&](const wchar_t* key,auto& bank){const auto* a=data.get(key);if(!a)return;
   for(size_t i=0;i<bank.size()&&i<a->arr.size();++i)bank[i]=static_cast<int>(a->arr[i].n);};
  bits(L"scenario",flags.scenario);bits(L"treasure",flags.treasure);values(L"global",flags.global);
  values(L"area",flags.area);values(L"map",flags.local);
  std::printf("External saved flags: scenario73=%d\n",flags.scenario[73].value_or(-1));
 }
 // The native default resolver can cache only a later observed additive group.
 // Reproduce that exact selection shape, while preserving copied prerequisite
 // flags, rather than testing only the complete initializer projection again.
 json::Value observed;json::parse(R"({"Map_10040":{"code":[[1,"ev_e_0185"],[6]]}})",observed);
 map_state::Resolver state(flags,observed),projected(flags,fixture::records);int failures=0;
 auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 const baked::Object* entry=nullptr;for(const auto& object:baked::grids(*sm).objects)
  if(air_transitions::saronia_entry(3,4,object)&&object.entity_id()==213)entry=&object;
 if(!entry)return 5;
 expect(projected.active(*sm,*entry)&&!state.active(*sm,*entry)&&state.scenario_value(73)==0,
     "later observed-group cache omits the scene despite its copied native prerequisite remaining satisfied");
 expect(!air_transitions::saronia_entry(2,4,*entry)&&!air_transitions::saronia_entry(3,1,*entry),"other games/maps excluded");
 auto wrong=*entry;wrong.a=121;expect(!air_transitions::saronia_entry(3,4,wrong),"unreviewed script excluded");
 const auto start=std::chrono::steady_clock::now();
 for(int id:{6,13,15,16}) {
  const int air=(id==15||id==16)?15:6;
  RouteSnapshot snapshot(state);snapshot.map_id=4;snapshot.submap=sm;snapshot.layer=1;snapshot.locale=L"en";
  snapshot.position=world_of_cell(*sm,{1,114,108});snapshot.vehicle={true,id,baked::transport(id)->type,L"",4};
  snapshot.goal.map_id=4;snapshot.goal.cell={1,entry->tx,entry->ty};snapshot.goal.entity_id=entry->entity_id();snapshot.goal.raw_name=baked::object_identifier(*entry);
  snapshot.kind=TargetKind::Transition;
  NativeObstacle gate;gate.type=19;gate.id=892;gate.cell={-1,83,77};gate.collider=true;gate.collision=1;gate.applies=id!=6;snapshot.obstacles.push_back(gate);
  {
   snapshot.obstacles.clear();
   for(const auto& original:baked::grids(*sm).objects) {
    if(!projected.active(*sm,original))continue;
    const auto object=projected.effective(*sm,original);NativeObstacle raw;
    raw.cell={-1,object.tx,object.ty};raw.id=object.entity_id();raw.raw_name=baked::object_identifier(object);
    raw.type=static_cast<int>(object.type);
    if(object.type==baked::ObjType::CollisionEntity) {
     raw.collider=true;raw.collision=object.a;
     const auto targets=baked::str(object.name);raw.applies=targets.empty()||(L","+targets+L",").find(L","+std::to_wstring(id)+L",")!=targets.npos;
    } else if(object.type==baked::ObjType::Warp) {raw.type=2;raw.trigger=true;}
    else if(object.type==baked::ObjType::Event||object.type==baked::ObjType::RandomEvent)raw.trigger=true;
    snapshot.obstacles.push_back(std::move(raw));
   }
   SnapshotScope scope(snapshot);const auto captured=live_blockers(*sm,baked::grids(*sm),{});
   std::printf("External projection: obstacles=%zu gates=%zu triggers=%zu\n",snapshot.obstacles.size(),captured.gates.size(),captured.triggers.size());
   if(id==6&&argc>2) {
    auto movement=walk_request(*sm,{1,114,108});world_mode(movement);movement.blocked=captured.hard();movement.directional=captured.directional;
    movement.live_hazards=movement.live_traps=true;movement.hazards=captured.triggers;movement.traps=captured.traps;
    const auto reach=baked_route::flood(movement);
    for(const auto& object:baked::grids(*sm).objects)if(air_transitions::saronia_entry(3,4,object)) {
     Cell at{1,object.tx,object.ty};int neighbor=-1;for(const auto& side:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}) {
      const int d=reach.distance({1,at.x+side.x,at.y+side.y});if(d>=0&&(neighbor<0||d<neighbor))neighbor=d;
     }
     std::printf("target%d (%d,%d) active=%d used=%d open=%d reach=%d neighbor=%d\n",object.entity_id(),at.x,at.y,planning_object_active(*sm,object),state.consumed(*sm,object),baked_route::open(movement,baked::grids(*sm),at),reach.distance(at),neighbor);
    }
    auto altered=snapshot;altered.obstacles.erase(std::remove_if(altered.obstacles.begin(),altered.obstacles.end(),[](const auto& o){return o.trigger;}),altered.obstacles.end());
    ExitSnapshot no_triggers(true,state);no_triggers.route.map_id=4;no_triggers.route.submap=sm;
    no_triggers.route.layer=1;no_triggers.route.position=altered.position;no_triggers.route.vehicle=altered.vehicle;no_triggers.route.obstacles=altered.obstacles;
    std::printf("diagnostic without triggers story=%d\n",!!air_transition_target(no_triggers,{},{},{},true));
   }
  }
  Planned plan;std::thread worker([&]{plan=plan_snapshot(snapshot);});worker.join();
  if(!plan.path.success)std::printf("vehicle%d route rejection: %ls / %ls\n",id,plan.path.reason.c_str(),plan.path.error.c_str());
  expect(plan.path.success&&!plan.path.walk_phases.empty(),"Enterprise reaches native Saronia scene");
  if(!plan.path.walk_phases.empty()) {
   const auto& last=plan.path.walk_phases.back();expect(last.transportation_id==air&&last.cells.back()==Cell{1,entry->tx,entry->ty},"route ends in its actual Enterprise flight mode at exact scene, without landing");
   for(const auto& phase:plan.path.walk_phases)expect(phase.vehicle_before!=route_progress::VehicleAction::WalkOff,"no false shore disembark");
  }
  ExitSnapshot capture(true,state);capture.route.map_id=4;capture.route.submap=sm;capture.route.layer=1;
  capture.route.position=snapshot.position;capture.route.vehicle=snapshot.vehicle;capture.route.obstacles=snapshot.obstacles;
  const auto distance=exit_distances(capture,{{world_of_cell(*sm,{1,entry->tx,entry->ty}),4,105,{1,entry->tx,entry->ty},TargetKind::Transition}},{});
  expect(distance.size()==1&&distance[0]>=0,"native story entrance survives world exit distance filtering");
  field::filter_worker::FilterInput input;input.current_map=4;input.world=true;input.exits=true;input.events=true;input.story_gates=false;
  input.state=std::make_shared<map_state::Resolver>(state);input.navigation=std::make_shared<ExitSnapshot>(true,state);
  auto& captured=input.navigation->route;captured.map_id=4;captured.submap=sm;captured.layer=1;
  captured.position=snapshot.position;captured.vehicle=snapshot.vehicle;captured.obstacles=snapshot.obstacles;
  field::filter_worker::FilterRow door;door.index=0;door.kind=Kind::MapExit;door.category=Category::MapExits;door.destination=105;
  door.destination_name=L"Castle Saronia";door.name=L"Exit to Castle Saronia";door.position=world_of_cell(*sm,{1,83,67});
  door.entity_id=24;door.cell_x=83;door.cell_y=67;door.cell_layer=1;door.cell_valid=true;input.rows.push_back(door);
  field::filter_worker::Categories categories;std::thread filtering([&]{categories=field::filter_worker::build_categories(input);});filtering.join();
  expect(categories.story_target.has_value(),"actual filter discovers Saronia event without a native event entity");
  expect(categories.views[static_cast<size_t>(Category::Events)].order.size()==1,"event is published with the user's current-map-with-vehicles and story-filter-off settings");
  if(categories.story_target) {
   const auto& target=*categories.story_target;
   expect(target.name==L"Saronia Airspace","event uses the requested destination name independently of castle exit listing");
   auto selected=snapshot;selected.goal=target.goal;selected.kind=TargetKind::AirshipTransition;selected.previous_active.reset();
   Planned event;std::thread event_worker([&]{event=plan_snapshot(selected);});event_worker.join();
   expect(event.path.success&&!event.path.walk_phases.empty()&&event.path.walk_phases.back().transportation_id==air,
       "selecting the actual Events row reaches the native trigger while airborne");
   if(event.path.success&&!event.path.walk_phases.empty()) {
    expect(event.path.walk_phases.back().cells.back()==target.goal.cell,"selected row ends at its exact trigger");
    for(const auto& phase:event.path.walk_phases)expect(phase.vehicle_before!=route_progress::VehicleAction::WalkOff,"selected row never asks to land");
    SnapshotScope selected_scope(selected);const auto live=live_blockers(*sm,baked::grids(*sm),{});
    for(const auto& phase:event.path.walk_phases)for(size_t j=1;j+1<phase.cells.size();++j)
     expect(!baked_route::crosses_event({phase.cells[j]},live.triggers),"selected row never crosses another trigger before arrival");
   }
  }
  auto complete_flags=flags;complete_flags.scenario[73]=1;
  ExitSnapshot complete(true,map_state::Resolver(complete_flags,fixture::records));complete.route.map_id=4;complete.route.submap=sm;
  complete.route.position=snapshot.position;complete.route.vehicle=snapshot.vehicle;
  expect(!air_transition_target(complete,{},{},{},true),"completed scene is absent from Events");
  auto castle=snapshot;castle.goal={};castle.goal.map_id=4;castle.goal.cell={1,83,67};castle.goal.entity_id=24;castle.kind=TargetKind::Transition;
  Planned ordinary;std::thread castle_worker([&]{ordinary=plan_snapshot(castle);});castle_worker.join();
  expect(ordinary.goal.cell==Cell{1,83,67}&&ordinary.goal.entity_id==24,
      "ordinary Castle Saronia target identity is not rerouted");
  if(ordinary.path.success)expect(!ordinary.path.walk_phases.empty()&&ordinary.path.walk_phases.back().cells.back()==Cell{1,83,67},
      "ordinary route retains the native ground endpoint");
  std::printf("Events view=%zu story=%d native rows=1 (castle only), ordinary endpoint=(83,67)\n",categories.views[static_cast<size_t>(Category::Events)].order.size(),!!categories.story_target);
  std::printf("vehicle%d scene=(%d,%d) route=%d steps=%d filter=%d\n",id,entry->tx,entry->ty,plan.path.success,plan.path.step_count,distance.empty()?-1:distance[0]);
 }
 RouteSnapshot foot(state);foot.map_id=4;foot.submap=sm;foot.layer=1;foot.locale=L"en";foot.position=world_of_cell(*sm,{1,92,78});foot.vehicle={true,1,1,L"",4};
 foot.goal.map_id=4;foot.goal.cell={1,87,74};foot.kind=TargetKind::Transition;
 NativeObstacle gate;gate.type=19;gate.id=892;gate.cell={-1,83,77};gate.collider=true;gate.collision=1;foot.obstacles.push_back(gate);
 Planned blocked;std::thread worker([&]{blocked=plan_snapshot(foot);});worker.join();
 expect(!blocked.path.success&&blocked.path.reason==L"Story gated","real pre-entry ground gate is preserved");
 std::printf("Saronia: %d failures, %.3fms aggregate offline, wrong-owner=%d\n",failures,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count(),fixture::wrong_owner.load());
 return failures||fixture::wrong_owner.load()?1:0;
}
