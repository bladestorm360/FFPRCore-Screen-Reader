// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Melmond before/after native canal15, owning only the real ship. Pure owned
// snapshots replay walking and already-boarded routes, browse and filtering.
#define main retained_trench_route_main
#include "route_worker_check.cpp"
#undef main
namespace trench {
using namespace field;using namespace baked_nav;
RouteSnapshot make(const baked::Submap& world,const map_state::Flags& flags,Cell start,Cell boat,const baked::Object& target,bool riding,TargetKind kind){
 RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=1;s.submap=&world;s.layer=1;s.position=world_of_cell(world,start);s.vehicle={true,riding?4:1,riding?2:1,{}};s.vehicle_shortcuts=true;s.locale=L"en";
 VehicleOnMap ship;ship.transportation_id=4;ship.type=2;ship.pos=world_of_cell(world,boat);s.vehicles.push_back(ship);s.vehicle_names[4]=L"Ship";
 s.goal.map_id=1;s.goal.cell={1,target.tx,target.ty};s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);s.kind=kind;
 for(const auto& original:baked::grids(world).objects){
  if(!s.state.active(world,original))continue;const auto o=s.state.effective(world,original);
  if(o.type!=baked::ObjType::CollisionEntity&&o.type!=baked::ObjType::Entity&&o.type!=baked::ObjType::AnimEntity&&o.type!=baked::ObjType::NPC&&o.type!=baked::ObjType::Event&&o.type!=baked::ObjType::Warp)continue;
  NativeObstacle n;n.id=o.entity_id();n.type=int(o.type);n.raw_name=baked::object_identifier(o);n.cell={-1,o.tx,o.ty};n.collider=o.type==baked::ObjType::CollisionEntity;n.enabled=true;n.collision=o.a;
  n.transportation_ids=s.state.transportation_mask(world,original);n.transportation_restricted=n.transportation_ids!=~uint64_t(0);n.applies=(n.transportation_ids&baked_route::transportation_bit(s.vehicle.id))!=0;n.trigger=baked_route::step_event(o);n.npc=o.type==baked::ObjType::NPC;
  if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}
  s.obstacles.push_back(std::move(n));
 }
 return s;
}
int run(){const auto* world=baked::submap_by_map_id(1);const auto* ship=baked::transport(4);if(!world||!world->world||!ship||ship->type!=2){std::printf("Invalid fixture world%d worldflag%d ship%d type%d\n",world!=nullptr,world?world->world:0,ship!=nullptr,ship?ship->type:-1);return 2;}const auto& grid=baked::grids(*world);
 const Cell walking{1,145,153};Cell boat;bool dock=false;
 // A legal dock beside the reported Cornelia-area player pose. This is an
 // owned-ship gate witness, not a claim about the user's uncaptured ship pose.
 for(int radius=0;radius<=32&&!dock;++radius)for(int y=walking.y-radius;y<=walking.y+radius&&!dock;++y)for(int x=walking.x-radius;x<=walking.x+radius&&!dock;++x){
  if(std::max(std::abs(x-walking.x),std::abs(y-walking.y))!=radius)continue;
  baked_route::Request foot;foot.submap=world;foot.transportation_id=1;const Cell landing{1,x,y};
  if(!baked_route::open(foot,grid,landing)||!baked::group_lands(ship->landing_group,baked::attribute(grid,x,y))||baked_route::terminal_transition(foot,grid,landing))continue;
  for(auto d:std::array<Cell,4>{{{1,0,1},{1,1,0},{1,0,-1},{1,-1,0}}}){
   const Cell candidate{1,x+d.x,y+d.y};baked_route::Request ride;ride.submap=world;ride.transportation_id=4;ride.vehicle_group=ship->group;ride.vehicle_type=2;ride.landing_group=ship->landing_group;
   if(baked_route::open(ride,grid,candidate)){boat=candidate;dock=true;break;}
  }
 }
 if(!dock){std::printf("No native legal dock near witness\n");return 2;}std::printf("Trench witness walking %d,%d ship %d,%d; real ship4/type2/group%d\n",walking.x,walking.y,boat.x,boat.y,ship->group);
 int failures=0,cases=0;const auto expect=[&](bool okay,const char* mode,int stage,int riding,int id,int kind){++cases;if(!okay){++failures;std::printf("FAIL Melmond %s stage%d riding%d entity%d kind%d\n",mode,stage,riding,id,kind);}};
 for(int stage=0;stage<3;++stage){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=1;
  for(int flag=1;flag<=(stage?14:12);++flag)flags.scenario[flag]=1;flags.scenario[15]=stage==2;flags.scenario[49]=1;flags.scenario[517]=flags.scenario[519]=1;
  for(const auto& target:grid.objects)if(target.type==baked::ObjType::GotoMap&&target.a==34){
   // An absent ship must stay a vehicle requirement, even when a closed story
   // collision exists elsewhere. No hypothetical ownership in the gate proof.
   auto no_ship_flags=flags;no_ship_flags.scenario[517]=0;
   for(auto kind:{TargetKind::Transition,TargetKind::Infer}){
    auto missing=make(*world,no_ship_flags,walking,boat,target,false,kind);missing.vehicles.clear();
    const auto route=plan_snapshot(missing).path;
    expect(!route.success&&route.reason.find(L"Requires ")==0,"missing-ship",stage,0,target.entity_id(),int(kind));
   }
   for(int riding:{0,1})for(auto kind:{TargetKind::Transition,TargetKind::Infer}){
    const Cell start=riding?boat:walking;auto s=make(*world,flags,start,boat,target,riding!=0,kind);const auto path=plan_snapshot(s).path;
    std::printf("Melmond stage%d riding%d kind%d exit%d: success%d reason%ls\n",stage,riding,int(kind),target.entity_id(),path.success,path.reason.c_str());
    expect(stage==2?path.success:!path.success&&path.reason==L"Story gated","directions",stage,riding,target.entity_id(),int(kind));
    auto browse=make(*world,flags,start,boat,target,riding!=0,kind);const auto result=plan_browse_snapshot(browse).route;
    // Owned boarding/landing is an indirect but usable route. The action label
    // is expected after opening; it must never claim a missing vehicle/gate.
    expect(stage==2?!result.reason.empty()&&result.reason.find(L"Requires ")==std::wstring::npos&&result.reason!=L"Story gated":!result.direct&&result.reason==L"Story gated","browse",stage,riding,target.entity_id(),int(kind));
    ExitSnapshot capture(true,map_state::Resolver(flags,fixture::records));auto copied=make(*world,flags,start,boat,target,riding!=0,kind);
    capture.route.map_id=1;capture.route.submap=world;capture.route.position=copied.position;capture.route.layer=1;capture.route.vehicle=copied.vehicle;capture.route.vehicles=std::move(copied.vehicles);capture.route.vehicle_names=std::move(copied.vehicle_names);capture.route.obstacles=std::move(copied.obstacles);capture.route.goal=copied.goal;capture.route.vehicle_shortcuts=true;
    const auto mask=story_gate_mask(capture,{{capture.route.goal,kind}},[] {return false;});
    expect(mask.size()==1&&mask[0]==(stage!=2),"story-filter",stage,riding,target.entity_id(),int(kind));
   }
  }
 }
 std::printf("Melmond trench/owned-ship consumers: %d cases/%d failures; owner%d\n",cases,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=trench::run();});worker.join();return result;}
