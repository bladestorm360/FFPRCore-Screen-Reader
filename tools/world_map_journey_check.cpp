// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production source-to-output checks: real surface/underwater terrain and entrances.
#define main world_journey_retained_main
#include "route_worker_check.cpp"
#undef main
int main() {
 using namespace field;using namespace baked_nav;
 const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff3/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 map_state::Flags flags;flags.scenario.fill(1);flags.global.fill(1);flags.treasure.fill(1);
 int failures=0;auto expect=[&](bool value,const char* why){if(!value){++failures;std::printf("FAIL %s\n",why);}};
 const auto make=[&](int map,int id,Cell at,int destination,Cell target){
  auto copied=flags;copied.current_map_id=map;copied.current_area_id=1;
  auto owned=std::make_shared<ExitSnapshot>(true,map_state::Resolver(copied,fixture::records));auto& s=owned->route;
  s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=1;s.position=world_of_cell(*s.submap,at);
  s.vehicle={id!=0,id,id?baked::transport(id)->type:0,{}};s.locale=L"en";
  s.goal.map_id=destination;s.goal.cell=target;s.kind=TargetKind::Transition;s.vehicle_names[9]=L"Nautilus";
  for(const auto& object:baked::grids(*s.submap).objects)if(const auto* rule=air_transitions::match(3,map,object)) {
   NativeObstacle actor;actor.type=2;actor.id=object.entity_id();actor.cell={1,object.tx,object.ty};actor.raw_name=baked::object_identifier(object);
   actor.trigger=true;actor.transportation_restricted=actor.transportation_complete=true;actor.transportation_ids=rule->mask;s.obstacles.push_back(std::move(actor));
  }
  return owned;
 };
 const auto started=std::chrono::steady_clock::now();
 expect(make(4,9,{1,151,74},3,{1,95,77})->route.vehicle_shortcuts,"real exit capture constructor retains the requested vehicle policy");
 auto state=map_state::Resolver(flags,fixture::records);std::vector<RemoteObject> inventory;
 std::thread inventory_worker([&]{inventory=world_objects_snapshot(4,state,{});});inventory_worker.join();
 int submerged=0;
 for(const auto& row:inventory) {
  const auto* map=baked::submap_by_map_id(row.map_id);
  expect(map&&map->world,"World map inventory never enumerates town/dungeon interior objects");
  if(row.map_id==3&&row.world_exit)++submerged;
 }
 expect(submerged==4,"surface World map filter includes all four real underwater entrance tiles");
 expect(inventory.size()<512,"continent borders do not restore the 1535 redundant trigger population");
 auto list_capture=make(4,9,{1,151,74},3,{1,95,77});filter_worker::FilterInput listing;
 listing.current_map=4;listing.world=listing.exits=listing.events=listing.sort=true;listing.reachable_only=false;
 listing.navigation=list_capture;listing.player=list_capture->route.position;listing.state=std::make_shared<map_state::Resolver>(flags,fixture::records);
 for(const auto& object:inventory) {
  filter_worker::FilterRow row;row.index=listing.rows.size();row.remote_map=object.map_id;row.cell_valid=true;
  row.cell_layer=1;row.cell_x=object.cell.x;row.cell_y=object.cell.y;row.destination=object.dest_map_id;row.entity_id=object.entity_id;row.raw_name=object.raw_name;
  row.kind=object.world_exit||object.type==baked::ObjType::GotoMap?Kind::MapExit:Kind::Event;
  row.category=row.kind==Kind::MapExit?Category::MapExits:Category::Events;row.name=std::to_wstring(object.entity_id);row.event_type=object.event_type;
  listing.rows.push_back(std::move(row));
 }
 filter_worker::Categories categories;const auto list_started=std::chrono::steady_clock::now();
 std::thread category_worker([&]{categories=filter_worker::build_categories(listing);});category_worker.join();
 std::printf("World category: %zu input, %zu retained, %.3fms owned worker\n",inventory.size(),categories.views[0].order.size(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-list_started).count());
 expect(ui::prefs::kFilterWorldMap==3&&ui::prefs::kPathfindingFilterModeMax==3&&ui::prefs::kPathfindingFilterModeDefault==2,"fourth filter preserves existing saved mode numbers and default");
 auto dive=make(4,9,{1,151,74},3,{1,95,77});Planned d;
 std::thread t([&]{d=plan_snapshot(dive->route);});t.join();
 expect(d.path.success&&d.path.next_map_id==3,"surface route reaches an actual dive tile and retains the underwater dungeon destination");
 expect(!d.path.walk_phases.empty()&&d.path.walk_phases.back().action_after==L"dive here"&&d.path.walk_phases.back().manual_after,"dive is an explicit manual action, not an ordinary landing");
 if(d.path.success) {
  const auto at=d.path.walk_phases.back().cells.back();
  expect(baked::attribute(baked::grids(*baked::submap_by_map_id(4)),at.x,at.y)==16,"dive endpoint is real surface dive terrain");
  auto continued=make(3,14,at,3,{1,95,77});continued->route.pending=d.pending;Planned p;
  std::thread c([&]{p=plan_snapshot(continued->route);});c.join();
  expect(p.path.success&&!p.path.next_map_id,"native same-position dive continues to the actual submarine dungeon entrance");
  std::printf("Dive at %d,%d; surface steps %d; underwater steps %d\n",at.x,at.y,d.path.step_count,p.path.step_count);
  expect(p.path.step_count<=2,"route flies near the dungeon instead of prescribing a long underwater sail");
  auto repeated=make(4,9,{1,151,74},3,{1,95,77});repeated->route.pending=d.pending;
  repeated->route.pending.completed_transitions.push_back({4,-300020,3,at.x,at.y});Planned alternative;
  std::thread avoid([&]{alternative=plan_snapshot(repeated->route);});avoid.join();
  expect(!alternative.path.success||alternative.path.walk_phases.back().cells.back()!=at,"completed manual dive is not proposed again for the retained destination");
 }
 auto village=make(3,14,{1,113,118},4,{1,150,203});Planned v;
 std::thread u([&]{v=plan_snapshot(village->route);});u.join();
 expect(v.path.success&&v.path.next_map_id==4,"underwater origin reaches Doga's Village through a legal surfacing basin");
 if(v.path.success) {
  const auto at=v.path.walk_phases.back().cells.back();
  expect(at.x>=149&&at.x<=151&&at.y>=203&&at.y<=205,"Village surfacing stays inside the real isolated basin");
  auto landed=make(4,9,at,4,{1,150,203});Planned l;
  std::thread s([&]{l=plan_snapshot(landed->route);});s.join();
  expect(l.path.success&&!l.path.next_map_id,"surfacing continues with native landing and walking into the Village");
 }
 auto tunnel=make(4,9,{1,113,118},4,{1,150,203});Planned n;
 std::thread q([&]{n=plan_snapshot(tunnel->route);});q.join();
 expect(n.path.success&&n.path.next_map_id==3,"same-surface destination can use the real underwater tunnel when flight alone fails");
 std::vector<int> village_list;
 std::thread eligible([&]{village_list=exit_distances(*tunnel,{{world_of_cell(*tunnel->route.submap,{1,150,203}),4,151,{1,150,203},TargetKind::Transition}},{});});eligible.join();
 expect(village_list.size()==1&&village_list[0]>=0,"Current map with vehicles retains the Village entrance reachable through the underwater tunnel");
 auto locked=make(4,8,{1,151,74},3,{1,95,77});Planned no;
 std::thread z([&]{no=plan_snapshot(locked->route);});z.join();
 expect(!no.path.success,"original Nautilus cannot invent the diving upgrade");
 auto parked_other_world=make(1,11,{1,64,64},3,{1,95,77});
 parked_other_world->route.saved_vehicles={{9,4,{151,74,0}}};
 Planned change_world_craft;
 std::thread stored([&]{change_world_craft=plan_snapshot(parked_other_world->route);});stored.join();
 expect(change_world_craft.path.success&&change_world_craft.path.next_map_id==4,"Invincible can reach the surface and change to the enabled Nautilus at its saved real position");
 auto manor=make(1,11,{1,64,64},4,{1,128,184});manor->route.saved_vehicles={{9,4,{151,74,0}}};Planned wind_change;
 std::thread distant([&]{wind_change=plan_snapshot(manor->route);});distant.join();
 expect(wind_change.path.success&&wind_change.path.next_map_id==4,"saved Nautilus also proves the required future-map wind passage after Invincible travel");
 auto absent_manor=make(1,11,{1,64,64},4,{1,128,184});Planned absent_wind;
 std::thread without([&]{absent_wind=plan_snapshot(absent_manor->route);});without.join();
 expect(!absent_wind.path.success,"future-map native wind guards reject Invincible when no enabled Nautilus position is captured");
 auto continent=make(1,8,{1,64,64},3,{1,95,77});Planned outward;
 std::thread leave([&]{outward=plan_snapshot(continent->route);});leave.join();
 expect(outward.path.success&&outward.path.next_map_id==4,"floating-continent route carries the script-qualified upgraded Nautilus to the surface before diving");
 if(outward.path.success) {
  expect(outward.pending.incoming_transition&&outward.pending.incoming_transition->arrival==Cell{1,32,216},"continent exit retains the actual SetVehicle arrival instead of its fallback PointIn");
  auto after=make(4,9,{1,32,216},3,{1,95,77});after->route.pending=outward.pending;Planned onward;
  std::thread carry([&]{onward=plan_snapshot(after->route);});carry.join();
  expect(onward.path.success&&onward.path.next_map_id==3,"native continent arrival recaptures and continues the original underwater objective");
 }
 const auto* island=baked::submap_by_map_id(1);const baked::Object* island_door=nullptr;
 for(const auto& object:baked::grids(*island).objects)if(object.type==baked::ObjType::GotoMap&&object.a>0){island_door=&object;break;}
 expect(island_door!=nullptr,"real floating-continent entrance fixture exists");
 if(island_door) {
  auto back=make(4,9,{1,113,118},1,{1,island_door->tx,island_door->ty});Planned return_trip;
  std::thread home([&]{return_trip=plan_snapshot(back->route);});home.join();
  expect(return_trip.path.success&&return_trip.path.next_map_id==1,"surface route uses the actual airborne continent return before its island entrance");
  auto submerged=make(3,14,{1,113,118},1,{1,island_door->tx,island_door->ty});Planned return_sea;
  std::thread shore([&]{return_sea=plan_snapshot(submerged->route);});shore.join();
  expect(return_sea.path.success&&return_sea.path.next_map_id==4,"underwater route surfaces before flying back to the floating continent");
 }
 auto cancelled=make(4,9,{1,151,74},3,{1,95,77});cancelled->route.cancelled=[] {return true;};Planned stop;
 std::thread a([&]{stop=plan_snapshot(cancelled->route);});a.join();
 expect(!stop.path.success,"cancelled cross-world search publishes no route");
 expect(fixture::wrong_owner==0,"cross-world planner never calls native owners from workers");
 std::printf("World inventory: %zu rows, %d underwater entrances\n",inventory.size(),submerged);
 std::printf("World journeys: %d failures, owner%d, %.3fms owned compute\n",failures,fixture::wrong_owner.load(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count());
 return failures?1:0;
}
