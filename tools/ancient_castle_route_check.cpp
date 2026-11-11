// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact native objective and old construction-waypoint route observations.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0;
 const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,odin,stairs,target_map,entity;};
 for(const auto test:std::array<Case,13>{{{517,1,0,0,517,2},{517,1,0,0,517,12},{517,1,1,0,517,12},{518,1,1,1,518,-1},{518,1,1,1,519,10},{519,1,1,1,519,10},{519,1,0,1,519,10},{517,1,1,1,519,10},{519,1,0,1,517,2},{518,1,0,1,517,2},{517,2,0,0,517,2},{517,3,0,0,517,2},{517,4,0,0,517,2}}}){
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_map_id=test.map;f.global[0]=4;f.global[5]=1;f.scenario[428]=test.odin;f.scenario[429]=test.stairs;
  const auto* sm=baked::submap_by_map_id(test.map);map_state::Resolver state(f,fixture::records);const auto* point=point_in(*sm,test.point,state);if(!point)return 2;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=test.map;s.submap=sm;s.layer=point->layer;s.position=world_of_cell(*sm,{point->layer,point->tx,point->ty});s.vehicle={true,1,baked::transport(1)->type,{}};s.locale=L"en";s.goal.map_id=test.target_map;s.goal.entity_id=test.entity;s.kind=TargetKind::Interact;
  for(const auto& o:baked::grids(*baked::submap_by_map_id(test.target_map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==test.entity){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);}
  if(test.entity<0){s.kind=TargetKind::Spot;s.goal.cell={1,14,32};}
  for(const auto& source:baked::grids(*sm).objects){
   if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*sm,source,true))continue;
   const auto o=s.state.effective(*sm,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
   if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
  }
  Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Ancient map%d point%d odin%d stairs%d to%d/%d: success%d next%d npc%d reason%s directions%s\n",test.map,test.point,test.odin,test.stairs,test.target_map,test.entity,p.path.success,p.path.next_map_id,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  const bool possible=test.entity>=0&&!(test.target_map==519&&!test.odin);
  expect(p.path.success==possible,"real objective succeeds; old construction waypoint and inactive final statue do not");
  int legs=0;
  while(p.path.success&&p.path.next_map_id>0&&legs<12){
   const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"ordinary castle passage retains its real arrival");if(!receipt)break;
   auto flags=f;flags.current_map_id=receipt->destination;flags.local.fill(0);RouteSnapshot next(map_state::Resolver(flags,fixture::records));next.map_id=receipt->destination;next.submap=baked::submap_by_map_id(next.map_id);next.layer=receipt->arrival.layer;next.position=world_of_cell(*next.submap,receipt->arrival);next.vehicle=s.vehicle;next.locale=L"en";next.goal=s.goal;next.kind=s.kind;
   for(const auto& source:baked::grids(*next.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!next.state.active(*next.submap,source,true))continue;
    const auto o=next.state.effective(*next.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}next.obstacles.push_back(n);
   }
   std::thread continued([&]{p=plan_snapshot(next);});continued.join();++legs;
   std::printf("  continuation map%d at%d,%d L%d: success%d next%d npc%d %s %s\n",next.map_id,receipt->arrival.x,receipt->arrival.y,receipt->arrival.layer,p.path.success,p.path.next_map_id,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  }
  if(possible)expect(p.path.success&&!p.path.next_map_id&&!p.path.required_trap&&!p.path.npc_approach&&legs<12,"complete castle route reaches the real operation without a trap or compulsory optional battle");
 }
 expect(!fixture::wrong_owner,"native calls remain off the worker");std::printf("Ancient castle:13 physical/resume/negative cases/%d failures/owner%d\n",failures,fixture::wrong_owner.load());return failures?1:0;
}
