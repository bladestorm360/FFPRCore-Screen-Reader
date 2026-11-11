// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Native-stage witnesses using actual gates, actors, scene endpoints and exits.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0;
 const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,phase,goal_map,entity;int engine=-1;};
 for(const auto test:std::array<Case,16>{{{355,1,0,355,1},{355,1,1,355,2},{357,1,2,357,2},{357,1,3,361,8},{361,1,4,361,6},{624,11,5,624,1},{624,11,6,624,2},{624,11,7,624,3},{362,1,8,362,20},{624,12,9,624,22},{362,1,9,362,20},{362,1,10,371,14},{371,1,10,371,10,7},{371,1,10,371,6,1},{371,1,10,356,5,5},{372,1,11,372,1}}}){
  const int map=test.map,phase=test.phase;
  const auto* sm=baked::submap_by_map_id(map);if(!sm)return 2;
  map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.global[5]=1;f.current_map_id=map;
  const int stages[]{107,108,109,519,110,111,112,113,114,115};for(int i=0;i<std::min(phase,10);++i)f.scenario[stages[i]]=1;f.scenario[935]=phase>=2&&phase<11;f.scenario[936]=phase==11;f.scenario[123]=phase==11;
  if(map==371)f.scenario[119]=1; // native0207_1 entrance receipt
  if(test.engine>=0)for(int i=0;i<3;++i)f.scenario[120+i]=(test.engine>>i)&1;
  map_state::Resolver resolver(f,fixture::records);const auto* point=point_in(*sm,test.point,resolver);if(!point)return 2;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.submap=sm;s.map_id=map;s.layer=point->layer;s.position=world_of_cell(*sm,{point->layer,point->tx,point->ty});s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.kind=TargetKind::Interact;
  if(map==357&&phase==2)s.position=world_of_cell(*sm,{1,29,11}); // final0161_1 player SetPos
  if(map==624&&phase==6)s.position=world_of_cell(*sm,{1,125,2}); //0186 player moves north once from125,3
  if(map==624&&phase==7)s.position=world_of_cell(*sm,{1,97,5}); // actual0187 final player landing, not its old Point11
  for(const auto& o:baked::grids(*baked::submap_by_map_id(test.goal_map)).objects)if(o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.entity_id()==test.entity){s.goal.map_id=test.goal_map;s.goal.entity_id=test.entity;s.goal.raw_name=baked::object_identifier(o);s.goal.cell={o.layer,o.tx,o.ty};s.kind=o.type==baked::ObjType::Entity||o.type==baked::ObjType::NPC?TargetKind::Interact:TargetKind::Spot;}
  for(const auto& source:baked::grids(*sm).objects){
   if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*sm,source,true))continue;
   const auto o=s.state.effective(*sm,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
   if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
  }
  Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"each real native stage has an ordinary physical route without a fabricated encounter/fall");
  if(map==357&&phase==3)expect(p.path.next_map_id==624&&p.pending.incoming_transition&&p.pending.incoming_transition->arrival==Cell{1,151,5},"first-car west exit lands at actual624 Point2");
  if(map==357&&phase==2)expect(p.path.step_count==2,"post-boarding player starts29,11 and must actually reach the departure trigger");
  if(map==624&&phase==6)expect(p.path.step_count==6,"after the real pursuit movement, the next jump trigger is six west");
  if(map==357&&phase==3){
   auto state=s.state;const auto floors=connected_floors(sm,map,state);std::printf("First-car graph:");for(const auto& floor:floors)std::printf(" %d",floor.map_id);std::printf("\n");
   for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::Warp)std::printf("First-car warp id%d pos%d,%d dest%d point%d active%d consumed%d\n",o.entity_id(),o.tx,o.ty,o.a,o.b,state.active(*sm,o),state.consumed(*sm,o));
  }
  std::printf("Phantom map%d phase%d point%d,%d actors%zu to%d/%d: success%d next%d npc%d reason%s directions%s\n",map,phase,point->tx,point->ty,s.obstacles.size(),test.goal_map,test.entity,p.path.success,p.path.next_map_id,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  int legs=0;
  while(p.path.success&&p.path.next_map_id>0&&legs<20){
   const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"ordinary continuation retains its exact native arrival");if(!receipt)break;
   auto next_flags=f;next_flags.current_map_id=receipt->destination;next_flags.local.fill(0);
   RouteSnapshot next(map_state::Resolver(next_flags,fixture::records));next.map_id=receipt->destination;next.submap=baked::submap_by_map_id(next.map_id);next.layer=receipt->arrival.layer;next.position=world_of_cell(*next.submap,receipt->arrival);next.locale=L"en";next.vehicle=s.vehicle;next.goal=s.goal;next.kind=s.kind;
   for(const auto& source:baked::grids(*next.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!next.state.active(*next.submap,source,true))continue;
    const auto o=next.state.effective(*next.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}next.obstacles.push_back(n);
   }
   std::thread continuation([&]{p=plan_snapshot(next);});continuation.join();++legs;
   std::printf("  continuation map%d at%d,%d: success%d next%d npc%d %s %s\n",next.map_id,receipt->arrival.x,receipt->arrival.y,p.path.success,p.path.next_map_id,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
   expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"complete intermediate-floor continuation remains physically usable");
  }
  expect(p.path.success&&p.path.next_map_id==0&&legs<20,"each stage reaches its final objective rather than only its first exit");
 }
 // Transfer109 availability must never hide the native message event. All
 // four entry sides/landings are source-qualified, not a first-branch guess.
 const auto* first=baked::submap_by_map_id(357);
 for(int value:{0,1}){
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_map_id=357;f.scenario[108]=1;f.scenario[109]=value;
  map_state::Resolver state(f,fixture::records);int native=0,transfers=0;
  for(const auto& o:baked::grids(*first).objects)if(o.entity_id()==1||o.entity_id()==13||o.entity_id()==16||o.entity_id()==17){
   if(o.type==baked::ObjType::Event){expect(!state.consumed(*first,o),"message-only native door stays available before train start");++native;}
   if(o.type==baked::ObjType::Warp){expect(state.consumed(*first,o)==!value,"only the derived transfer requires actual109");expect(o.a==624&&o.b==(o.tx>20?1:2)&&((o.d>>8)&15)==(o.tx>20?8:2),"four doors retain their real facing, point and approach");++transfers;}
  }
  expect(native==4&&transfers==4,"exact four physical actors and four derived exits");
 }
 expect(!fixture::wrong_owner,"native ownership preserved");std::printf("Phantom stages:16 actual route witnesses/four conditional exits/%d failures/owner%d\n",failures,fixture::wrong_owner.load());return failures?1:0;
}
