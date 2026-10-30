// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual FFIII wind destination, captured parked models and production planner.
#define main retained_vehicle_journey_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff3/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(4);const auto& grid=baked::grids(*world);
 map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(1);flags.global.fill(1);flags.current_map_id=4;
 int failures=0;auto expect=[&](bool value,const char* why){if(!value){++failures;std::printf("FAIL %s\n",why);}};
 const baked::Object* door=nullptr;for(const auto& o:grid.objects)if(o.type==baked::ObjType::GotoMap&&o.entity_id()==158)door=&o;
 expect(door&&door->tx==128&&door->ty==184,"Doga's Manor fixture uses its actual native world entrance");if(!door)return 2;
 const auto parking=[&](int id,Cell origin,int occurrence=0){
  const auto* row=baked::transport(id);baked_route::Request foot;foot.submap=world;foot.start=origin;
  for(int dy=-3;dy<=3;++dy)for(int dx=-3;dx<=3;++dx){Cell at{1,origin.x+dx,origin.y+dy};
   if(!baked_route::open(foot,grid,at)||baked_route::terminal_transition(foot,grid,at))continue;
   if(row->landing_group>0&&!baked::group_lands(row->landing_group,baked::attribute(grid,at.x,at.y)))continue;
   if(occurrence--==0)return at;
  }return Cell{1,-1,-1};
 };
 const auto model=[&](int id,Cell at){VehicleOnMap value;value.transportation_id=id;value.type=baked::transport(id)->type;value.pos=world_of_cell(*world,at);return value;};
 const auto make=[&](int current){auto c=std::make_shared<ExitSnapshot>(true,map_state::Resolver(flags,fixture::records));auto& s=c->route;
  s.map_id=4;s.submap=world;s.layer=1;s.locale=L"en";s.position=world_of_cell(*world,{1,151,74});s.vehicle={current!=0,current,current?baked::transport(current)->type:0,{}};
  s.vehicle_shortcuts=true;s.auto_groups={3};s.goal.map_id=4;s.goal.cell={1,128,184};s.goal.entity_id=158;s.kind=TargetKind::Transition;
  // The actual wind restriction is enforced by three powered native events,
  // not terrain: all three aircraft share the relevant terrain profile. Keep
  // the exact actor/selector observations used by the real capture producer.
  for(const auto& o:grid.objects)if(o.type==baked::ObjType::Event&&(o.entity_id()==896||o.entity_id()==898||o.entity_id()==899)) {
   NativeObstacle wind;wind.type=2;wind.id=o.entity_id();wind.cell={1,o.tx,o.ty};wind.raw_name=baked::object_identifier(o);wind.trigger=true;
   wind.transportation_restricted=wind.transportation_complete=true;
   wind.transportation_ids=baked_route::transportation_bit(o.entity_id()==896?1:o.entity_id()==898?10:6);
   s.obstacles.push_back(std::move(wind));
  }
  s.vehicle_names[9]=L"Nautilus";s.vehicle_names[10]=L"Invincible";return c;};
 const auto boards=[](const Planned& plan,int id){return std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[&](const auto& p){return p.transportation_id==id&&p.action_before.find(L"board")!=std::wstring::npos;});};
 const auto board_cell=[](const Planned& plan,int id){for(const auto& p:plan.path.walk_phases)if(p.transportation_id==id&&p.action_before.find(L"board")!=std::wstring::npos&&!p.cells.empty())return p.cells.front();return Cell{1,-1,-1};};
 const Cell first=parking(9,{1,151,74}),moved=parking(9,{1,151,74},1);
 expect(first.x>=0&&moved.x>=0&&!(first==moved),"Nautilus observations use two distinct native-legal parked cells");
 Planned original,missing,updated,repeated;std::vector<int> listed;
 auto capture=make(10);capture->route.vehicles={model(9,first)};
 auto began=std::chrono::steady_clock::now();
 std::thread worker([&]{original=plan_snapshot(capture->route);listed=exit_distances(*capture,{{world_of_cell(*world,{1,128,184}),4,door->a,{1,128,184},TargetKind::Transition}},{});});worker.join();
 const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
 expect(original.path.success&&boards(original,9),"Invincible route lands at and boards Nautilus for the real wind-protected Manor");
 expect(listed.size()==1&&listed[0]>=0,"vehicle-filter eligibility agrees with the real craft-changing wind route");
 expect(board_cell(original,9)==first,"first route boards the actual captured Nautilus position");
 auto same=make(10);same->route.vehicles={model(9,first)};same->route.previous_active=original.active;
 std::thread repeat_worker([&]{repeated=plan_snapshot(same->route);});repeat_worker.join();
 expect(repeated.path.success&&board_cell(repeated,9)==first,"repeated directions retain the same real boarding target and required craft");
 auto changed=make(10);changed->route.vehicles={model(9,moved)};changed->route.previous_active=original.active;
 std::thread moved_worker([&]{updated=plan_snapshot(changed->route);});moved_worker.join();
 expect(updated.path.success&&board_cell(updated,9)==moved,"a retained route rechecks the parked vehicle's live position");
 auto absent=make(10);absent->route.previous_active=original.active;
 std::thread missing_worker([&]{missing=plan_snapshot(absent->route);});missing_worker.join();
 expect(!missing.path.success,"a missing or locked Nautilus cannot be inferred from its baked capability");
 auto chain=make(0);const auto invincible=parking(10,{1,151,74}),nautilus=parking(9,{1,46,57});
 chain->route.vehicles={model(10,invincible),model(9,nautilus)};Planned two;
 std::thread two_worker([&]{two=plan_snapshot(chain->route);});two_worker.join();
 expect(two.path.success&&boards(two,10)&&boards(two,9),"on-foot route can board Invincible, fly to Nautilus, land, change craft and reach the Manor");
 auto vehicle_target=make(10);vehicle_target->route.position=world_of_cell(*world,{1,197,42});
 vehicle_target->route.vehicles={model(9,{1,46,57})};vehicle_target->route.goal.cell={1,46,57};
 vehicle_target->route.goal.entity_id=-1;vehicle_target->route.goal.transportation_id=9;vehicle_target->route.kind=TargetKind::Spot;
 auto browse_copy=vehicle_target->route;Planned directions;BrowsePlanned browse;
 std::thread vehicle_directions([&]{directions=plan_snapshot(vehicle_target->route);});vehicle_directions.join();
 std::thread vehicle_browse([&]{browse=plan_browse_snapshot(browse_copy);});vehicle_browse.join();
 expect(directions.path.success,"logged Invincible197,42 to parked Nautilus46,57 has an actual selected route");
 expect(browse.route.direct||!browse.route.reason.empty(),"a valid vehicle route's browse label gives distance or its required action, never bare Not directly reachable");
 std::printf("Nautilus browse: direct%d reason=%ls; directions%d phases%zu\n",browse.route.direct,browse.route.reason.c_str(),directions.path.success,directions.path.walk_phases.size());
 expect(fixture::wrong_owner==0,"all journey decisions use owned snapshot data without native calls");
 std::printf("Wind route: success%d board9%d listed%d; moved%d/%d; missing%d; two-craft%d, %.3fms owned selected+list\n",original.path.success,boards(original,9),listed.empty()?-1:listed[0],board_cell(updated,9).x,board_cell(updated,9).y,missing.path.success,two.path.success,elapsed);
 std::printf("Vehicle journeys: %d failures, owner%d\n",failures,fixture::wrong_owner.load());return failures?1:0;
}
