// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Native Player versus invalid-record control; Giant's Cave must precede a
// parked Ship detour on the walking journey to the Sage's Cave world entrance.
#define main retained_giant_route_main
#include "route_worker_check.cpp"
#undef main
namespace giant {
using namespace field;using namespace baked_nav;
RouteSnapshot capture(const baked::Submap& room,const map_state::Flags& flags,Cell at,Cell boat,const baked::Object& target,bool native,bool shortcuts,TargetKind kind) {
 RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=room.first_map_id;s.submap=&room;s.layer=at.layer;
 s.position=world_of_cell(room,at);if(native)s.vehicle={true,1,1,{}};s.locale=L"en";s.vehicle_shortcuts=shortcuts;
 s.goal.map_id=1;s.goal.cell={1,target.tx,target.ty};s.goal.entity_id=target.entity_id();s.goal.raw_name=baked::object_identifier(target);s.kind=kind;
 if(room.world){VehicleOnMap ship;ship.transportation_id=4;ship.type=2;ship.pos=world_of_cell(room,boat);s.vehicles.push_back(ship);s.vehicle_names[4]=L"Ship";}
 for(const auto& source:baked::grids(room).objects) {
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(room,source))continue;
  const auto object=s.state.effective(room,source);NativeObstacle n;n.type=int(object.type);n.id=object.entity_id();n.raw_name=baked::object_identifier(source);
  n.cell={room.world?-1:object.layer,object.tx,object.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(object);
  n.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;n.chest=object.type==baked::ObjType::TreasureBox;
  n.collider=object.type==baked::ObjType::CollisionEntity;n.collision=object.a;
  n.transportation_ids=s.state.transportation_mask(room,source);n.transportation_restricted=n.transportation_ids!=~uint64_t(0);
  if(object.type==baked::ObjType::Warp){if((object.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(std::move(n));
 }return s;
}
bool walking(const PathInfo& path) {
 return std::none_of(path.walk_phases.begin(),path.walk_phases.end(),[](const auto& phase){return phase.transportation_id>0||phase.vehicle_before!=route_progress::VehicleAction::None;});
}
int run() {
 const auto* world=baked::submap_by_map_id(1);const auto* model=baked::transport(4);if(!world||!model)return 2;const auto& grid=baked::grids(*world);
 const Cell start{1,58,180};Cell boat;bool dock=false;int closest=INT_MAX;
 // The saved Ship pose is unavailable. Choose a real dock reachable on foot
 // from this logged origin; never invent water at the quoted relative offset.
 baked_route::Request foot;foot.submap=world;foot.transportation_id=1;
 foot.start=start;const auto shore=baked_route::flood(foot);
 baked_route::Request ride=foot;ride.transportation_id=4;ride.vehicle_group=model->group;ride.vehicle_type=2;ride.landing_group=model->landing_group;
 for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x) {
  const Cell water{1,x,y};if(!baked_route::open(ride,grid,water))continue;
  for(const auto side:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}) {
   const Cell land{1,x+side.x,y+side.y};const int distance=shore.distance(land);
   if(distance>=0&&distance<closest&&baked_route::open(foot,grid,land)&&baked::group_lands(model->landing_group,baked::attribute(grid,land.x,land.y))){boat=water;dock=true;closest=distance;}
  }
 }
 if(!dock){std::printf("FAIL no physical dock in logged walking component\n");return 2;}
 std::printf("Giant witness: native log origin%d,%d; legal unconfirmed Ship dock%d,%d\n",start.x,start.y,boat.x,boat.y);
 int failures=0,cases=0,journeys=0,hops=0;std::optional<Cell> eastern_arrival;
 const auto expect=[&](bool okay,const char* why,int native,int shortcuts,int kind,int id) {
  ++cases;if(!okay){++failures;std::printf("FAIL Giant %s native%d shortcuts%d kind%d target%d\n",why,native,shortcuts,kind,id);}
 };
 for(const auto& target:grid.objects)if(target.type==baked::ObjType::GotoMap&&target.a==40)for(int native:{0,1})for(int shortcuts:{0,1})for(const auto kind:{TargetKind::Transition,TargetKind::Infer}) {
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.current_map_id=1;
  for(int bit=1;bit<=18;++bit)flags.scenario[bit]=1;flags.scenario[517]=1;flags.current_area_id=1;
  auto s=capture(*world,flags,start,boat,target,native!=0,shortcuts!=0,kind);const auto plan=plan_snapshot(s);
  std::printf("Giant target%d at%d,%d native%d shortcuts%d kind%d: success%d next%d steps%d walking%d reason[%ls]\n",target.entity_id(),target.tx,target.ty,native,shortcuts,int(kind),plan.path.success,plan.path.next_map_id,plan.path.step_count,walking(plan.path),plan.path.reason.c_str());
  expect(plan.path.success&&plan.path.next_map_id==81&&walking(plan.path),"first hop must walk to Giant's Cave",native,shortcuts,int(kind),target.entity_id());
  auto b=capture(*world,flags,start,boat,target,native!=0,shortcuts!=0,kind);const auto browse=plan_browse_snapshot(b);
  expect(!browse.route.direct&&browse.route.via_map_id==81,"browse agrees with cave first hop",native,shortcuts,int(kind),target.entity_id());
  if(!plan.path.success||plan.path.next_map_id!=81||!walking(plan.path))continue;
  if(plan.pending.incoming_transition&&plan.pending.incoming_transition->destination==81)eastern_arrival=plan.pending.incoming_transition->arrival;
  Pending pending=plan.pending;const auto* room=world;Cell at=start;bool done=false;
  for(int hop=0;hop<8;++hop) {
   flags.current_map_id=room->first_map_id;
   if(const auto* record=fixture::records.get(baked::name_of(*room)))if(const auto* area=record->get(L"area_id"))flags.current_area_id=int(area->n);
   auto n=capture(*room,flags,at,boat,target,native!=0,shortcuts!=0,kind);n.pending=pending;const auto next=plan_snapshot(n);
   expect(next.path.success&&walking(next.path),"continued journey must remain walking",native,shortcuts,int(kind),target.entity_id());
   if(!next.path.success||!walking(next.path)||next.path.walk_phases.empty())break;
   if(next.path.next_map_id==0){done=room==world&&next.path.walk_phases.back().cells.back()==Cell{1,target.tx,target.ty};break;}
   const auto receipt=next.pending.incoming_transition?next.pending.incoming_transition:next.pending.incoming_trap;
   expect(receipt&&receipt->destination==next.path.next_map_id,"exact native continuation receipt",native,shortcuts,int(kind),target.entity_id());if(!receipt)break;
   pending=next.pending;room=baked::submap_by_map_id(receipt->destination);at=receipt->arrival;++hops;if(!room)break;
  }
  expect(done,"complete cave and west-world journey",native,shortcuts,int(kind),target.entity_id());++journeys;
 }
 // An unopened Giant passage cannot become a certified through-dungeon route
 // merely because the native Player record is valid. Remove parked models to
 // isolate that actual source gate from genuine alternative vehicle journeys.
 for(const auto& target:grid.objects)if(target.type==baked::ObjType::GotoMap&&target.a==40)for(int ruby:{0,1}) {
  map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.current_map_id=1;
  for(int bit=1;bit<=16;++bit)f.scenario[bit]=1;f.scenario[17]=ruby;f.current_area_id=1;
  auto closed=capture(*world,f,start,boat,target,true,false,TargetKind::Transition);closed.vehicles.clear();
  CrossLeg proof;{SnapshotScope scope(closed);proof=find_across(nullptr,nullptr,closed.goal,true);}
  std::printf("Giant no opening18, Ruby17=%d: prospective foot journey%d\n",ruby,proof.found);
  if(ruby)expect(proof.found,"owned Ruby permits a prospective manual opening",1,0,2,target.entity_id());
  else {
   const bool before=std::getenv("FFPR_GIANT_GATE_BEFORE")!=nullptr;
   expect(before?proof.found:!proof.found,before?"before source reproduces omitted no-Ruby barrier":"without Ruby the closed Giant passage cannot validate a foot journey",1,0,2,target.entity_id());
  }
  if(!std::getenv("FFPR_GIANT_GATE_BEFORE")) {
   auto named=capture(*world,f,start,boat,target,true,false,TargetKind::Transition);named.vehicles.clear();
   const auto directions=plan_snapshot(named);
   expect(ruby?directions.path.success:(!directions.path.success&&directions.path.reason==L"Story gated"),
       "world directions distinguish the Ruby story gate from a vehicle requirement",1,0,2,target.entity_id());
   auto listed=capture(*world,f,start,boat,target,true,false,TargetKind::Transition);listed.vehicles.clear();
   const auto browse=plan_browse_snapshot(listed);
   expect(ruby?browse.route.via_map_id==81:browse.route.reason==L"Story gated",
       "world browse reports the same Giant/Ruby requirement",1,0,2,target.entity_id());
   const auto* cave=baked::submap_by_map_id(81);if(!cave||!eastern_arrival)return 2;
   f.current_map_id=81;f.current_area_id=12;map_state::Resolver state(f,fixture::records);
   bool early=false,ready=false;
   for(const auto& actor:baked::grids(*cave).objects)if(actor.type==baked::ObjType::NPC&&state.active(*cave,actor)) {
    early|=actor.entity_id()==13;ready|=actor.entity_id()==11;
   }
   expect(!early&&ready,"native initializer keeps Giant11 until opening18",1,0,2,target.entity_id());
   auto inside=capture(*cave,f,*eastern_arrival,boat,target,true,false,TargetKind::Transition);
   const auto local=plan_snapshot(inside);
   expect(ruby?(local.path.success&&local.path.npc_approach):(!local.path.success&&local.path.reason==L"Story gated"),
       "current floor requires the actual story item or manual Giant interaction",1,0,2,target.entity_id());
  }
 }
 expect(cases>0,"Sage's Cave native entrance exists",0,0,0,0);
 std::printf("FF1 Giant/Sage route consumers: %d assertions/%d complete journeys/%d native hops; failures%d owner%d\n",cases,journeys,hops,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
#ifndef FFPR_GIANT_ROUTE_FIXTURE_ONLY
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=giant::run();});worker.join();return result;}
#endif
