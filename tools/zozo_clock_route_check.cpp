// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real clock walls, native entry points, hidden reward and ordered guide choices.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0,transfers=0;
 const auto expect=[&](bool value,const char* why){if(!value){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,score,entity;bool possible=true;bool candidates=false;};
 const std::vector<Case> cases{{386,1,0,11},{386,1,1,11},{386,1,2,11},{386,1,3,14},{386,2,0,14},{386,2,3,14},{387,1,0,-1},
 {386,1,0,14,false},{386,1,3,11,false},{386,1,0,11,true,true},{386,1,3,14,true,true},{386,2,0,14,true,true}};
 for(const auto test:cases){
  map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);flags.current_area_id=38;flags.current_map_id=test.map;flags.local[1]=test.score;flags.global[5]=1;
  map_state::Resolver state(flags,fixture::records);const auto* map=baked::submap_by_map_id(test.map);const auto* point=point_in(*map,test.point,state);if(!point)return 2;
  const auto goal=[&](int entity){GuideTarget t;t.kind=entity<0?TargetKind::Spot:TargetKind::Interact;t.goal.map_id=386;t.goal.entity_id=entity;t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;
   if(entity<0){t.goal.cell={1,7,23};return t;}
   for(const auto& o:baked::grids(*baked::submap_by_map_id(386)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==entity){t.goal.cell={o.layer,o.tx,o.ty};t.goal.raw_name=baked::object_identifier(o);}return t;};
  int mid=test.map;Cell at{point->layer,point->tx,point->ty};bool reached=false;
  for(int leg=0;leg<12;++leg){
   auto current=flags;current.current_map_id=mid;RouteSnapshot s(map_state::Resolver(current,fixture::records));s.map_id=mid;s.submap=baked::submap_by_map_id(mid);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
   auto selected=goal(test.entity);s.goal=selected.goal;s.kind=selected.kind;if(test.candidates)s.candidates={goal(11),goal(14)};
   for(const auto& source:baked::grids(*s.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*s.submap,source,true))continue;
    const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
   }
   Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
   std::printf("Zozo from%d point%d local%d to%d leg%d map%d: success%d next%d index%d %s %s\n",test.map,test.point,test.score,test.entity,leg,mid,p.path.success,p.path.next_map_id,p.path.guidance_index,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
   if(!test.possible){expect(!p.path.success,"closed wall and hidden clock remain unavailable");reached=!p.path.success;break;}
   expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"real native gate/collision/event semantics reach the correct operation");if(!p.path.success)break;
   if(test.candidates)expect(p.path.guidance_index==(test.entity==11?0:1),"actual ordered planner selects the proper side's pending clock/reward");
   if(!p.path.next_map_id){reached=true;break;}
   const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"real floor continuation has an owned arrival");if(!receipt)break;mid=receipt->destination;at=receipt->arrival;++transfers;
  }
  expect(reached,"finite complete native-floor route reaches its exact objective");
 }
 expect(!fixture::wrong_owner,"no native API runs on the route worker");std::printf("Zozo clock routes:%zu cases/%d continuations/%d failures/owner%d\n",cases.size(),transfers,failures,fixture::wrong_owner.load());return failures?1:0;
}
