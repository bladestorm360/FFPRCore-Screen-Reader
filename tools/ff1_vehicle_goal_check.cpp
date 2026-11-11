// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real logged Citadel/Ship positions, actual owned planner and legal boarding.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff1/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF1;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
 int failures=0,cases=0;const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 expect(baked::transport(4)->type==kTransportShip&&baked::transport(2)->type==kTransportContent,"FF1 native Ship4/Canoe2 identities match the actual masters");
 const Cell foot_at{1,122,40},air_at{1,160,36},ship_at{1,223,224};
 const auto model=[&](int id,Cell at){VehicleOnMap v;v.transportation_id=id;v.type=baked::transport(id)->type;v.pos=world_of_cell(*world,at);return v;};
 for(int canal:{0,1})for(int current:{1,3}){
  map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(1);flags.global.fill(1);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=1;flags.scenario[15]=canal;
  RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=1;s.submap=world;s.layer=1;s.locale=L"en";s.position=world_of_cell(*world,current==3?air_at:foot_at);
  s.vehicle={true,current,baked::transport(current)->type,{}};s.vehicle_shortcuts=true;s.auto_groups={baked::transport(2)->group};s.vehicles={model(4,ship_at),model(3,air_at)};
  s.vehicle_names[4]=L"Ship";s.vehicle_names[3]=L"Airship";s.goal.map_id=1;s.goal.cell=ship_at;s.goal.entity_id=-1;s.goal.transportation_id=4;s.kind=TargetKind::Spot;s.name=L"Ship";
  auto browse_snapshot=s;Planned plan;BrowsePlanned browse;
  const auto began=std::chrono::steady_clock::now();
  std::thread worker([&]{plan=plan_snapshot(s);});worker.join();
  std::thread browse_worker([&]{browse=plan_browse_snapshot(browse_snapshot);});browse_worker.join();
  ++cases;expect(plan.path.success,"available Ship is reachable through the captured Airship");
  std::printf("Captured world%d size%d,%d shipModel%.1f,%.1f goal%d,%d start%.1f,%.1f current%d/%d auto%d steps%d\n",world->first_map_id,grid.width,grid.height,s.vehicles[0].pos.x,s.vehicles[0].pos.y,s.goal.cell.x,s.goal.cell.y,s.position.x,s.position.y,s.vehicle.id,s.vehicle.type,s.auto_groups[0],plan.path.step_count);
  for(const auto& p:plan.path.walk_phases)if(!p.cells.empty())std::printf(" phase movement%d craft%d action%d cells%zu from%d,%d to%d,%d\n",p.movement_transportation_id,p.transportation_id,int(p.vehicle_before),p.cells.size(),p.cells.front().x,p.cells.front().y,p.cells.back().x,p.cells.back().y);
  expect(browse.route.direct||(!browse.route.reason.empty()&&browse.route.reason!=T(L"Story gated")),"Ship browse has a usable route/action instead of an incorrect story gate");
  if(plan.path.success){
   bool flew=false,boarded=false;
   for(const auto& p:plan.path.walk_phases){
    flew|=p.movement_transportation_id==3;
    if(p.transportation_id!=4||p.vehicle_before!=route_progress::VehicleAction::WalkOn)continue;
    boarded=true;expect(p.action_cell.has_value()&&!p.cells.empty()&&p.cells.back()==ship_at,"Ship pickup ends at its actual captured cell with a shore action");
    if(p.action_cell){baked_route::Request walking;walking.submap=world;walking.start=*p.action_cell;walking.auto_groups=s.auto_groups;
     expect(boarding_approach::allowed(walking,*baked::transport(4),*p.action_cell,ship_at),"final boarding approach obeys the native qualified FF1 direction contract");
     expect(p.vehicle_direction==baked_route::step_direction(*p.action_cell,ship_at,grid.width,grid.height),"spoken boarding direction matches the actual shore-to-Ship step");}
   }
   expect(flew&&boarded,"route really flies, lands and boards the Ship instead of stopping over its water tile");
  }
  std::printf("FF1 ship canal%d current%d: route%d browse%d reason=%ls phases%zu %.3fms\n",canal,current,plan.path.success,browse.route.direct,browse.route.reason.c_str(),plan.path.walk_phases.size(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
  if(canal==1&&current==3){
   const auto boards_ship=[](const Planned& p){return std::any_of(p.path.walk_phases.begin(),p.path.walk_phases.end(),[](const auto& phase){return phase.transportation_id==4&&phase.vehicle_before==route_progress::VehicleAction::WalkOn;});};
   auto no_assistance=s;no_assistance.vehicle_shortcuts=false;Planned current_craft;
   std::thread a([&]{current_craft=plan_snapshot(no_assistance);});a.join();++cases;
   expect(current_craft.path.success&&boards_ship(current_craft),"already-ridden Airship still lands and boards the explicit Ship with optional assistance disabled");
   auto missing_ship=s;missing_ship.vehicles={model(3,air_at)};Planned unavailable;
   std::thread b([&]{unavailable=plan_snapshot(missing_ship);});b.join();++cases;
   expect(!unavailable.path.success&&unavailable.path.reason==T(L"Destination is no longer available"),"absent Ship is never invented from its baked capabilities");
   auto missing_air=s;missing_air.position=world_of_cell(*world,foot_at);missing_air.vehicle={true,1,1,{}};missing_air.vehicles={model(4,ship_at)};Planned no_air;
   std::thread c([&]{no_air=plan_snapshot(missing_air);});c.join();++cases;
   expect(!no_air.path.success,"remote Ship cannot borrow an unowned Airship");
   auto walk_only=missing_air;walk_only.vehicles.push_back(model(3,air_at));walk_only.vehicle_shortcuts=false;Planned no_shortcut;
   std::thread d([&]{no_shortcut=plan_snapshot(walk_only);});d.join();++cases;
   expect(!no_shortcut.path.success,"walking-only origin does not auto-board a parked Airship");
   for(const auto& phase:plan.path.walk_phases)if(phase.transportation_id==4&&phase.action_cell){
    auto landed=missing_air;landed.position=world_of_cell(*world,*phase.action_cell);landed.previous_active=plan.active;landed.pending=plan.pending;Planned remainder;
    std::thread e([&]{remainder=plan_snapshot(landed);});e.join();++cases;
    expect(remainder.path.success&&boards_ship(remainder)&&std::none_of(remainder.path.walk_phases.begin(),remainder.path.walk_phases.end(),[](const auto& p){return p.movement_transportation_id==3;}),"after landing, the retained goal finishes at the Ship without returning to the Airship");break;
   }
  }
 }
 expect(fixture::wrong_owner==0,"all selected-vehicle planning remains owned data without native worker calls");
 std::printf("FF1 selected-vehicle goals: %d profiles, failures%d owner%d\n",cases,failures,fixture::wrong_owner.load());return failures?1:0;
}
