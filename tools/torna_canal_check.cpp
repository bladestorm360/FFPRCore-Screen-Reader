// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Code-only tester report: real Ship2, real canal arc, actual route/filter worker.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 auto read=[](const char* path){std::ifstream file(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(file),{});};
 auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);const baked::Object* entry=nullptr;
 for(const auto& object:grid.objects)if(baked::world_exit(object)&&object.entity_id()==63&&object.a==210&&object.b==1&&baked::object_identifier(object)==L"船のままトルナ運河")entry=&object;
 expect(entry!=nullptr,"actual Torna Canal script is already an indexed ordinary Map Exit");if(!entry)return 2;
 expect(baked::ship_entry(*entry),"reviewed ship-preserving Torna entrance permits native sailing entry");
 const auto target=Cell{1,entry->tx,entry->ty};
 auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);flags->current_map_id=1;
 auto capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*flags,fixture::records));auto& snapshot=capture->route;
 snapshot.map_id=1;snapshot.submap=world;snapshot.layer=1;snapshot.locale=L"en";snapshot.vehicle={true,2,2,L""};
 snapshot.position=world_of_cell(*world,{1,169,88});snapshot.goal.map_id=1;snapshot.goal.cell=target;
 snapshot.goal.entity_id=63;snapshot.goal.raw_name=baked::object_identifier(*entry);snapshot.kind=TargetKind::Transition;
 Planned plan;std::vector<int> distances;filter_worker::Categories views,literal;
 std::thread work([&]{plan=plan_snapshot(snapshot);
  ExitTarget exit{world_of_cell(*world,target),0,210,target};exit.kind=TargetKind::Transition;distances=exit_distances(*capture,{exit},{});
  filter_worker::FilterInput input;input.current_map=1;input.world=true;input.navigation=capture;
  filter_worker::FilterRow row;row.kind=Kind::MapExit;row.category=Category::MapExits;row.name=L"Exit to Torna Canal";
  row.entity_id=63;row.raw_name=snapshot.goal.raw_name;row.position=exit.position;row.numbering_position=row.position;row.destination=210;
  input.rows={row};views=filter_worker::build_categories(input);input.reachable_only=false;literal=filter_worker::build_categories(input);
 });work.join();
 expect(plan.path.success&&plan.path.step_count==1,"boarded Ship2 sails directly through the exact Torna entry");
 expect(!distances.empty()&&distances[0]==1,"actual bulk destination eligibility agrees with selected Ship2 routing");
 auto contains_entry=[](const auto& view){return std::find(view.order.begin(),view.order.end(),size_t(0))!=view.order.end();};
 expect(contains_entry(views.views[int(Category::MapExits)])&&contains_entry(literal.views[int(Category::MapExits)]),"reachable and literal category publication retain Torna under Map Exits");
 expect(!contains_entry(literal.views[int(Category::Events)]),"a plain ship-preserving map transfer is not duplicated as a storyline event");
 if(plan.path.success){
  expect(!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back().x==target.x&&plan.path.walk_phases.back().cells.back().y==target.y,"directions end at the exact native canal entrance");
  expect(std::none_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand||phase.vehicle_before==route_progress::VehicleAction::BackLand||phase.vehicle_before==route_progress::VehicleAction::WalkOff;}),"canal route does not invent a landing/disembark action");
 }
 for(int id:{15,16}){
  const baked::Object* event=nullptr;
  for(const auto& object:grid.objects)if(object.type==baked::ObjType::Event&&object.entity_id()==id&&object.a==37&&baked::str(object.group)==L"ev_e_0014")event=&object;
  expect(event!=nullptr,"real Ship2-only helm scene exists in the world bake");if(!event)continue;
  const Cell goal{1,event->tx,event->ty};
  auto unknown=std::make_unique<map_state::Flags>();unknown->current_map_id=1;
  for(int vehicle:{2,10}){
   auto scene=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*unknown,fixture::records));auto& route=scene->route;
   route.map_id=1;route.submap=world;route.layer=1;route.locale=L"en";route.vehicle={true,vehicle,2,L""};
   route.position=world_of_cell(*world,{1,goal.x+(id==15?-1:1),goal.y});route.kind=TargetKind::Infer;
   route.goal.map_id=1;route.goal.cell=goal;route.goal.entity_id=id;route.goal.raw_name=baked::object_identifier(*event);
   NativeObstacle actor;actor.type=2;actor.id=id;actor.raw_name=route.goal.raw_name;actor.cell={-1,goal.x,goal.y};
   actor.trigger=true;actor.transportation_restricted=true;actor.transportation_ids=uint64_t(1)<<2;actor.applies=vehicle==2;
   route.obstacles={actor};Planned selected;std::vector<int> reachable;
   std::thread scene_worker([&]{selected=plan_snapshot(route);ExitTarget target{world_of_cell(*world,goal),0,0,goal};target.kind=TargetKind::Infer;
    reachable=exit_distances(*scene,{target},{});
   });scene_worker.join();
   expect(selected.path.success==(vehicle==2),"real scene accepts its native Ship2 and rejects a different ship identity");
   expect(!reachable.empty()&&(reachable[0]>=0)==(vehicle==2),"event browsing follows the native ship selector without a town landing requirement");
   std::printf("Helm event%d vehicle%d: selected%d, eligibility%d\n",id,vehicle,selected.path.success,reachable.empty()?-99:reachable[0]);
  }
 }
 // The same destinations must be offered before boarding an owned Ship2.
 // Select an actual coastal docking pair, never a made-up open terrain cell.
 for(int id:{63,15,16}){
  const baked::Object* object=id==63?entry:nullptr;
  if(!object)for(const auto& candidate:grid.objects)if(candidate.type==baked::ObjType::Event&&candidate.entity_id()==id&&candidate.a==37)object=&candidate;
  const Cell goal{1,object->tx,object->ty};baked_route::Request foot;foot.submap=world;
  auto ship=foot;ship.vehicle_group=2;ship.vehicle_type=2;ship.transportation_id=2;
  Cell stand{},dock{};int best=INT_MAX;
  constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
  for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x){
   const Cell at{1,x,y};if(!baked_route::open(foot,grid,at))continue;
   for(const auto& side:sides){
    const Cell beside{1,(x+side[0]+grid.width)%grid.width,(y+side[1]+grid.height)%grid.height};
    if(!baked_route::open(ship,grid,beside))continue;
    if(std::any_of(grid.objects.begin(),grid.objects.end(),[&](const auto& event){return (baked::transition_bits(event)||baked_route::step_event(event))&&event.contains(beside.x,beside.y);}))continue;
    const int dx=std::abs(beside.x-goal.x),dy=std::abs(beside.y-goal.y),distance=std::min(dx,grid.width-dx)+std::min(dy,grid.height-dy);
    if(distance<best){best=distance;stand=at;dock=beside;}
   }
  }
  expect(best<INT_MAX,"a real non-trigger coastline can hold the unlocked ship");if(best==INT_MAX)continue;
  for(bool owned:{false,true}){
   auto unknown=std::make_unique<map_state::Flags>();unknown->current_map_id=1;
   auto before=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*unknown,fixture::records));auto& route=before->route;
   route.map_id=1;route.submap=world;route.layer=1;route.locale=L"en";route.vehicle={true,1,1,L""};route.vehicle_shortcuts=true;
   route.position=world_of_cell(*world,stand);route.goal.map_id=1;route.goal.cell=goal;route.goal.entity_id=id;route.goal.raw_name=baked::object_identifier(*object);
   route.kind=id==63?TargetKind::Transition:TargetKind::Infer;
   if(owned){VehicleOnMap model;model.transportation_id=2;model.type=2;model.pos=world_of_cell(*world,dock);route.vehicles={model};route.vehicle_names[2]=L"Ship";}
   if(id!=63)for(const auto& event:grid.objects)if(event.type==baked::ObjType::Event&&event.a==37&&(event.entity_id()==15||event.entity_id()==16)){
    NativeObstacle actor;actor.type=2;actor.id=event.entity_id();actor.cell={-1,event.tx,event.ty};actor.raw_name=baked::object_identifier(event);
    actor.trigger=true;actor.transportation_restricted=true;actor.transportation_ids=uint64_t(1)<<2;actor.applies=false;route.obstacles.push_back(actor);
   }
   Planned selected;std::vector<int> reachable;
   std::thread before_worker([&]{selected=plan_snapshot(route);ExitTarget target{world_of_cell(*world,goal),0,id==63?210:0,goal};target.kind=route.kind;reachable=exit_distances(*before,{target},{});});before_worker.join();
   expect(selected.path.success==owned&&!reachable.empty()&&(reachable[0]>=0)==owned,"before-boarding route and browse require a real owned ship and use its native goal mode");
   if(owned)expect(std::any_of(selected.path.walk_phases.begin(),selected.path.walk_phases.end(),[](const auto& phase){return phase.transportation_id==2&&phase.vehicle_before==route_progress::VehicleAction::WalkOn;}),"before-boarding directions retain native walk-on Ship2 boarding");
   std::printf("Before-board event%d owned%d: selected%d, eligibility%d\n",id,owned,selected.path.success,reachable.empty()?-99:reachable[0]);
  }
 }
 expect(fixture::wrong_owner==0,"Torna route/publication work makes no native calls from workers");
 std::printf("Torna Canal: route%d steps%d eligibility%d, %d failures\n",plan.path.success,plan.path.step_count,distances.empty()?-99:distances[0],failures);
 return failures?1:0;
}
