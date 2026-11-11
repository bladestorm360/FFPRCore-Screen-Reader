// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/library_model.cpp
// Physical entry/return evidence, independent of the prototype's collapsed272.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/library_model.h"
namespace {
using namespace field;using namespace baked_nav;
int cases=0,failures=0,hops=0;
void check(bool value,const char*why){if(!value){++failures;std::printf("FAIL Library entry %s\n",why);}}
RouteSnapshot capture(int map,Cell at,const map_state::Flags& flags,const MapGoal& goal) {
 RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);
 s.position=world_of_cell(*s.submap,at);s.layer=at.layer;s.vehicle={true,1,1,{}};s.locale=L"en";s.goal=goal;s.kind=TargetKind::Spot;
 for(const auto& original:baked::grids(*s.submap).objects) {
  if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::Warp||!s.state.active(*s.submap,original))continue;
  const auto object=s.state.effective(*s.submap,original);NativeObstacle seen;
  seen.id=object.entity_id();seen.type=int(object.type);seen.raw_name=baked::object_identifier(original);seen.cell={object.layer,object.tx,object.ty};
  seen.enabled=seen.applies=true;seen.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;
  seen.chest=object.type==baked::ObjType::TreasureBox;seen.trigger=baked_route::step_event(object);
  seen.collider=object.type==baked::ObjType::CollisionEntity;seen.collision=object.a;
  seen.transportation_ids=s.state.transportation_mask(*s.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);
  s.obstacles.push_back(std::move(seen));
 }return s;
}
void journey(int root,const baked::Object& entry,bool done) {
 ++cases;map_state::Flags flags;flags.scenario.fill(0);flags.local.fill(0);flags.area.fill(0);flags.global.fill(0);flags.treasure.fill(0);
 if(done)for(int bit:{48,49,50,163})flags.scenario[bit]=1;
 flags.current_map_id=root;
 const int destination=done?272:275;const auto* room=baked::submap_by_map_id(destination);map_state::Resolver state(flags,fixture::records);
 const auto* endpoint=point_in(*room,1,state);check(endpoint,"actual destination PointIn1");if(!endpoint)return;
 ff5::library::Snapshot observed;observed.map=root;observed.player={entry.layer,entry.tx,entry.ty};observed.locale=L"en";
 for(size_t i=0;i<ff5::library::flags.size();++i)observed.values[i]=flags.scenario[ff5::library::flags[i]];
 const auto advice=ff5::library::plan(observed,json::Value{});
 if(root==272&&done){check(advice.finished&&advice.targets.empty(),"actual production final receipt at exterior");return;}
 check(!advice.unknown&&advice.targets.size()==1,"actual production entry/return target");if(advice.targets.size()!=1)return;
 const auto goal=advice.targets.front().goal;
 check(goal.map_id==destination&&goal.cell==Cell{endpoint->layer,endpoint->tx,endpoint->ty}&&goal.avoid_other_events,"production goal is the real safe PointIn1");
 Cell at{entry.layer,entry.tx,entry.ty};int map=root;Pending pending;bool reached=false;
 std::printf("Library physical root%d point%d done%d:",root,entry.a,done);
 for(int count=0;count<12;++count) {
  flags.current_map_id=map;auto snapshot=capture(map,at,flags,goal);snapshot.pending=pending;
  const auto route=plan_snapshot(snapshot);
  check(route.path.success&&!route.path.walk_phases.empty()&&!route.path.npc_approach,"actual safe physical leg");
  if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach){std::printf(" FAILED %ls",route.path.reason.c_str());break;}
  for(const auto& phase:route.path.walk_phases)for(const auto cell:phase.cells) {
   const auto& objects=baked::grids(*snapshot.submap).objects;
   for(const auto& object:objects)if(baked_route::step_event(object)&&snapshot.state.active(*snapshot.submap,object)&&object.layer==cell.layer&&object.contains(cell.x,cell.y))check(false,"entry path crosses unrelated contact event");
  }
  if(map==destination){reached=true;break;}
  const auto receipt=route.pending.incoming_transition;check(receipt.has_value(),"native door receipt");if(!receipt)break;
  check(receipt->destination>=272&&receipt->destination<=275,"only physical Library entrance floors");
  std::printf(" %d->%d",map,receipt->destination);++hops;pending=route.pending;
  {SnapshotScope scope(snapshot);snapshot.pending=pending;record_map_arrival(receipt->destination,receipt->arrival);pending=snapshot.pending;}
  map=receipt->destination;at=receipt->arrival;flags.local.fill(0);
 }
 std::printf(" reached%d\n",reached);check(reached,"complete physical entry or return");
}
}
int main(){
 std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto data=read("data/ff5/maps.bin");if(!baked::load(data.data(),data.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 for(int map:{272,273,274}) {
  const auto* room=baked::submap_by_map_id(map);check(room!=nullptr,"entry floor exists");if(!room)continue;
  for(const auto& object:baked::grids(*room).objects)if(object.type==baked::ObjType::PointIn)
   for(bool done:{false,true}){std::thread worker([&]{journey(map,object,done);});worker.join();}
 }
 check(fixture::wrong_owner==0,"owned worker makes no native call");
 std::printf("Library physical entrances: %d profiles/%d native hops; failures%d worker-native%d\n",cases,hops,failures,fixture::wrong_owner.load());return failures?1:0;
}
