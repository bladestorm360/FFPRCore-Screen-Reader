// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual route-capture/worker integration. Native vehicle Hide is represented
// independently from group/script inactivity; no game or runtime probe is used.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 int failures=0;auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 std::ifstream file("data/ff3/maps.bin",std::ios::binary);std::string blob{std::istreambuf_iterator<char>(file),{}};
 if(!baked::load(blob.data(),blob.size()))return 2;
 fixture::game_id=game::Id::FF3;fixture::capture_ready=true;fixture::capture_entity_alive=true;
 const auto* world=baked::submap_by_map_id(4);if(!world)return 3;
 int actor=0;
 ExitSnapshot capture(true,map_state::Resolver(map_state::Flags{},fixture::records));
 capture.route.submap=world;capture.route.map_id=4;capture.route.layer=1;
 fixture::capture_entity_vehicle_hidden=true;capture_exit_obstacle(capture,&actor);
 expect(capture.route.obstacles.size()==1,"vehicle-hidden native object survives current-map route capture for future vehicle legs");
 capture.route.obstacles.clear();fixture::capture_entity_vehicle_hidden=false;
 capture_exit_obstacle(capture,&actor);
 expect(capture.route.obstacles.empty(),"inactive story/group objects are not resurrected as vehicle-hidden objects");
 // The real capture now sees a typed property and IL2CPP-shaped value list,
 // rather than treating every actor as generic ObjectType0 with no metadata.
 fixture::capture_transport=true;fixture::capture_entity_active=true;
 auto& property=fixture::transport_property;auto& native=fixture::transport_actor;
 property.type=19;property.id=73;fixture::capture_raw_name=L"native transport collider";
 capture.route.vehicle.valid=true;capture.route.vehicle.id=8;
 property.targets=nullptr;auto unrestricted=capture_obstacle(&native,capture.route);
 expect(unrestricted.collider&&unrestricted.enabled&&unrestricted.applies&&unrestricted.id==73,
     "null native target list is unrestricted and retains real collider identity");
 fixture::TransportArray items;fixture::TransportList ids;ids.items=&items;
 property.targets=&ids;
 expect(!capture_obstacle(&native,capture.route).applies,"nonnull empty native target list permits no vehicle");
 ids.count=1;items.count=1;items.values[0]=1;
 expect(!capture_obstacle(&native,capture.route).applies,"foot-only collider does not apply to observed Nautilus8");
 capture.route.vehicle.id=1;
 expect(capture_obstacle(&native,capture.route).applies,"foot-only collider applies to valid native Player1");
 property.type=2;items.values[0]=8;
 auto flight_trigger=capture_obstacle(&native,capture.route);
 expect(flight_trigger.trigger&&!flight_trigger.applies,"powered aircraft-only event retains exact current-mode applicability");
 fixture::capture_entity_active=false;fixture::capture_entity_vehicle_hidden=true;
 capture.route.obstacles.clear();capture_exit_obstacle(capture,&native);
 expect(capture.route.obstacles.size()==1&&capture.route.obstacles.front().trigger&&capture.route.obstacles.front().id==73,
     "vehicle-hidden powered trigger survives capture with its typed native property and identity");
 expect(capture.route.obstacles.front().transportation_ids==(uint64_t(1)<<8),"captured trigger owns copied selector bits, not its native list");
 ids.count=65;items.count=8;
 const auto oversized=capture_obstacle(&native,capture.route);
 expect(!oversized.transportation_complete&&oversized.transportation_ids==~uint64_t(0),"oversized native lists are bounded and conservative");
 ids.count=1;items.count=1;items.values[0]=1;property.type=19;
 capture.route.obstacles.push_back(capture_obstacle(&native,capture.route));
 {
   SnapshotScope scope(capture.route);
   const auto live=live_blockers(*world,baked::grids(*world),{});
   baked_route::Request foot;foot.submap=world;live.apply_transport(foot);
   auto aircraft=foot;const auto* row=baked::transport(8);aircraft.transportation_id=8;aircraft.vehicle_group=row->group;aircraft.vehicle_type=row->type;
   const auto cell=capture.route.obstacles.front().cell;auto query=cell;query.layer=0;
   expect(live.gates.empty()&&live.triggers.empty()&&foot.transport_rules,
       "conditional actors are not frozen into unconditional walking/flight blockers");
   expect(baked_route::transport_rule_at(foot,baked::grids(*world),query,baked_route::TransportRules::Gate)&&
       !baked_route::transport_rule_at(aircraft,baked::grids(*world),query,baked_route::TransportRules::Gate),
       "the same captured foot gate applies to a walking leg and not a later aircraft leg");
   expect(!baked_route::transport_rule_at(foot,baked::grids(*world),query,baked_route::TransportRules::Hazard)&&
       baked_route::transport_rule_at(aircraft,baked::grids(*world),query,baked_route::TransportRules::Hazard),
       "the retained hidden aircraft event activates only in the matching future flight leg");
   auto explanation=foot;live.apply_transport(explanation,false);
   expect(!baked_route::transport_rule_at(explanation,baked::grids(*world),query,baked_route::TransportRules::Gate),
       "gates-free explanations omit conditional gates too");
 }
 fixture::capture_transport=false;fixture::capture_entity_vehicle_hidden=false;
 using Rules=baked_route::TransportRules;
 for(int n=1;n<=6;++n) {
   fixture::game_id=static_cast<game::Id>(n);
   auto read=[](const std::string& path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
   const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");
   expect(baked::load(bytes.data(),bytes.size()),"each actual game bake loads for selector tests");
   json::Value records;expect(json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),records),"actual indexed selector metadata loads");
   map_state::Resolver state(map_state::Flags{},records);
   int native_rules=0,warp_rules=0,unknown_records=0,switches=0,live_switches=0,remote_switches=0;
   const auto began=std::chrono::steady_clock::now();
   for(int i=0;i<baked::submap_count();++i) {
     const auto& map=*baked::submap_at(i);
     for(const auto& object:baked::grids(map).objects)
       if(state.transportation_mask(map,object)!=~uint64_t(0)) {
         if(object.type==baked::ObjType::Warp)++warp_rules;else ++native_rules;
       }
   }
   // All real SwitchEvent records, through actual capture and both consumers.
   // Native Reset/Hide/Exit can rearm a trigger: power is fresh snapshot state,
   // never a permanent consumed flag. No fixture pointer enters the worker.
   for(int i=0;i<baked::submap_count();++i) {
     const auto& map=*baked::submap_at(i);const auto& grid=baked::grids(map);
     for(const auto& object:grid.objects)if(object.type==baked::ObjType::SwitchEvent) {
       ++switches;expect(object.a||object.b,"real switch has a native action/script");
       fixture::capture_transport=true;fixture::capture_entity_active=true;
       fixture::transport_property.type=30;fixture::transport_property.id=object.entity_id();
       fixture::transport_property.targets=nullptr;fixture::capture_raw_name=baked::object_identifier(object);
       const Cell at{object.layer,object.tx,object.ty};fixture::current_position=world_of_cell(map,at);
       for(bool power:{true,false,true}) {
         ExitSnapshot observation(false,map_state::Resolver(map_state::Flags{},records));
         observation.route.submap=&map;observation.route.map_id=map.first_map_id;
         fixture::transport_actor.power=power;capture_exit_obstacle(observation,&fixture::transport_actor);
         expect(observation.route.obstacles.size()==1,"fresh capture retains switch identity even while unpowered");
         const bool typed=observation.route.obstacles.size()==1&&observation.route.obstacles.front().trigger;
         if(power&&typed)++live_switches;
         expect(typed==power,"switch capture follows native power and rearming");
         std::thread worker([&]{
           SnapshotScope scope(observation.route);const auto live=live_blockers(map,grid,{});
           bool footprint=false;for(const auto& cell:live.triggers)if(cell.x==at.x&&cell.y==at.y)footprint=true;
           expect(footprint==power,"owned current-floor switch hazards follow this capture, with no stale power cache");
         });worker.join();
       }
       std::thread worker([&]{
         map_state::Resolver remote(map_state::Flags{},records);baked_route::Request request;request.submap=&map;
         map_state::add_obstacles(remote,map,request);
         const bool hazard=std::any_of(request.hazards.begin(),request.hazards.end(),[&](const auto& cell){return cell.x==at.x&&cell.y==at.y;});
         remote_switches+=hazard;expect(hazard,"unknown future-floor state preserves the real scripted switch hazard");
       });worker.join();
     }
   }
   expect(switches==(n==6?17:0)&&live_switches==switches*2&&remote_switches==switches,
       "all-six exact switch inventory has capture/rearm and future-floor coverage");
   fixture::capture_transport=false;fixture::transport_actor.power=true;
   for(const auto& pair:records.obj)if(const auto* unknown=pair.second.get(L"initializer_unknown");unknown&&unknown->b) {
     expect(!map_state::project(pair.second,map_state::Flags{}).known,"selector-only records preserve missing-initializer uncertainty");++unknown_records;
   }
   expect(native_rules>0,"each game resolves real native selector fingerprints");
   const auto* map=static_cast<const baked::Submap*>(nullptr);
   const baked::TransportRow* air=nullptr;
   baked_route::Cell at{0,0,0};bool found=false;
   baked_route::Request foot,fly;
   // Enterprise flight is over sea; it has no shared walking tile. Choose a
   // real compatible model rather than asserting grass access for that craft.
   // A flooded/underwater world may correctly have no walking cells. Search
   // actual world/model pairs rather than assuming the first world has land.
   for(int i=0;i<baked::submap_count()&&!found;++i) {
     const auto* candidate=baked::submap_at(i);if(!candidate->world)continue;
     const auto& cells=baked::grids(*candidate);foot.submap=candidate;fly=foot;
     for(const auto& row:baked::transports()) {
       if(found)break;if(row.type!=3&&row.type!=8)continue;
       fly.transportation_id=row.id;fly.vehicle_group=row.group;fly.vehicle_type=row.type;
       for(int y=0;y<cells.height&&!found;++y)for(int x=0;x<cells.width;++x) {
         at={0,x,y};if(baked_route::open(foot,cells,at)&&baked_route::open(fly,cells,at)){found=true;air=&row;map=candidate;break;}
       }
     }
   }
   expect(found,"real walking and flight geometry supplies a shared-mode witness");
   if(!air||!map)continue;const auto& grid=baked::grids(*map);
   auto rules=std::make_shared<Rules>();
   baked_route::add_transport_cell(*rules,Rules::Gate,*map,at,uint64_t(1)<<1);
   for(const auto kind:{Rules::Hazard,Rules::Trap,Rules::Terminal})baked_route::add_transport_cell(*rules,kind,*map,at,uint64_t(1)<<air->id);
   foot.transport_rules=fly.transport_rules=rules;
   expect(baked_route::transport_rule_at(foot,grid,at,Rules::Gate)&&!baked_route::transport_rule_at(fly,grid,at,Rules::Gate),"all-six manual flight and foot use distinct selector masks");
   for(const auto kind:{Rules::Hazard,Rules::Trap,Rules::Terminal})expect(!baked_route::transport_rule_at(foot,grid,at,kind)&&baked_route::transport_rule_at(fly,grid,at,kind),"all-six event/trap/terminal selectors follow the actual leg");
   auto direction_rules=std::make_shared<Rules>();
   baked_route::add_transport_collision(*direction_rules,*map,at,2,uint64_t(1)<<1);
   auto expected=foot;expected.transport_rules.reset();baked_route::add_collision(expected.blocked,expected.directional,at,2);
   foot.transport_rules=fly.transport_rules=direction_rules;
   for(const auto& offset:std::vector<std::pair<int,int>>{{0,-1},{1,0},{0,1},{-1,0}}) {
     baked_route::Cell beside{0,(at.x+offset.first+grid.width)%grid.width,(at.y+offset.second+grid.height)%grid.height};
     expect(baked_route::direction_allowed(foot,at,beside)==baked_route::direction_allowed(expected,at,beside)&&
         baked_route::direction_allowed(foot,beside,at)==baked_route::direction_allowed(expected,beside,at),"conditional partial collisions retain native entering and leaving masks");
     expect(baked_route::direction_allowed(fly,at,beside)&&baked_route::direction_allowed(fly,beside,at),"a foot-only partial collider does not constrain flight");
   }
   if(n<=3) {
     const auto* canoe=baked::transport(2);auto automatic=foot;automatic.transport_rules=rules;automatic.auto_groups={canoe->group};
     bool river=false;for(int y=0;y<grid.height&&!river;++y)for(int x=0;x<grid.width;++x)
       if(baked::group_crosses(canoe->group,baked::attribute(grid,x,y))){at={0,x,y};river=true;break;}
     expect(river&&baked_route::transportation_at(automatic,grid,at)==2,"owned canoe terrain selects native ID2 in all applicable games");
     automatic.auto_groups.clear();expect(baked_route::transportation_at(automatic,grid,at)==1,"unowned automatic vehicles are not invented by selector lookup");
   }
   const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
   std::printf("FF%d real selector lookups: %d native, %d derived arcs, %d unknown-only records; %.3fms owned host including preparation\n",n,native_rules,warp_rules,unknown_records,ms);
   std::printf("FF%d pressure switches: %d real, %d powered/rearmed captures, %d future-floor hazards\n",n,switches,live_switches,remote_switches);
 }
 std::printf("Transport capture: %d failures; native-owner violations=%d; no game run\n",failures,fixture::wrong_owner.load());
 return failures?1:0;
}
