// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production planner with exact sealed-gate states and physical actors.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int diagnostic(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0;
 const auto* sm=baked::submap_by_map_id(407);
 for(int mode=0;mode<=2;++mode)for(int a=0;a<=1;++a)for(int b=0;b<=1;++b){
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_area_id=63;f.current_map_id=407;f.global[0]=2;f.global[5]=1;f.local[1]=mode;f.local[2]=a;f.local[3]=b;f.local[4]=mode==2;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=407;s.submap=sm;s.layer=1;s.position=world_of_cell(*sm,{1,6,3});s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
  s.goal.map_id=407;s.goal.entity_id=3;s.goal.cell={1,18,15};s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;s.kind=TargetKind::Transition;
  for(const auto& source:baked::grids(*sm).objects){
   if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap)continue;
   const int id=source.entity_id();bool active=s.state.active(*sm,source,true);
   // Exact native callback tables, independently verified against installed
   // metadata. Initializers hide the fall triggers until the first timer tick;
   // a post-tick capture must include the actual new phase's live triggers.
   const std::vector<int> a0{23,24,29,30,31,32,34,35,37,87},a1{20,21,22,25,26,27,28,33,36,86,90};
   const std::vector<int> b0{37,40,41,42,43,49,50,89},b1{36,38,39,44,45,46,47,48};
   const auto has=[&](const auto& v){return std::find(v.begin(),v.end(),id)!=v.end();};
   if(has(a0)||has(a1)||has(b0)||has(b1))active=mode==1?(a?has(a1):has(a0)):mode==2?(b?has(b1):has(b0)):false;
   const std::vector<int> ac0{19,73,74,75,76,77,78,79,80,81},ac1{18,64,65,66,67,68,69,70,71,72,91};
   const std::vector<int> bc0{19,58,59,60,61,62,63,88},bc1{18,51,52,53,54,55,56,57};
   // The native active callback writes shared18/19 after initialization too.
   // Project their actual post-tick Show/Hide, including the active timer's
   // conditional groups; do not inherit the other bridge's last initializer.
   if(mode==1&&(has(ac0)||has(ac1)))active=a?has(ac1):has(ac0);
   if(mode==2&&(has(bc0)||has(bc1)))active=b?has(bc1):has(bc0);
   if(!active)continue;
   const auto o=s.state.effective(*sm,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
   if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
  }
  Planned p;std::thread worker([&]{
   // The preflight already recorded the whole-floor static-phase failures.
   // This pass exports exact per-phase edges; timed crossings are separate
   // short legs, not repeated expensive impossible whole-floor requests.
   if((mode==0&&a==0&&b==0)||mode>0){
    SnapshotScope scope(s);const auto& grid=baked::grids(*sm);auto req=walk_request(*sm,{1,6,3});
    const auto live=live_blockers(*sm,grid,{});req.blocked=live.hard();req.directional=live.directional;live.apply_transport(req);
    req.live_hazards=req.live_traps=true;req.hazards=live.triggers;req.traps=live.traps;
    req.goals={{1,15,4},{1,18,13},{1,18,15}};
    std::printf("GRID mode%d phase%d/%d %dx%d traps%zu triggers%zu walls%zu\n",mode,a,b,grid.width,grid.height,live.traps.size(),live.triggers.size(),req.blocked.size());
    for(int y=0;y<grid.height;++y){
     std::printf("%02d ",y);
     for(int x=0;x<grid.width;++x){
      const Cell at{1,x,y};char ch=baked_route::open(req,grid,at)?'.':'#';
      for(auto c:req.blocked)if(c==at)ch='X';for(auto c:req.hazards)if(c==at)ch='e';for(auto c:req.traps)if(c==at)ch='!';
      std::printf("%c",ch);
     }
     std::printf("\n");
    }
    for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x)for(auto delta:std::array<std::pair<int,int>,4>{{{0,-1},{1,0},{0,1},{-1,0}}}){
     Cell from{1,x,y},to{1,x+delta.first,y+delta.second};
     if(baked_route::open(req,grid,from)&&baked_route::edge_allowed(req,grid,from,to))
      std::printf("EDGE %d %d %d %d %d %d %d\n",mode,a,b,x,y,to.x,to.y);
    }
   }
  });worker.join();
  std::printf("Sealed bridge phase capture mode%d phase%d/%d complete\n",mode,a,b);
 }
 for(int collapse=0;collapse<=1;++collapse)for(int left=0;left<=1;++left)for(int right=0;right<=1;++right)for(int bridge=0;bridge<=2;++bridge)for(int entity:{38,36,34,17}){
  if(!collapse&&entity!=34)continue;
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_area_id=63;f.current_map_id=408;f.global[0]=2;f.global[5]=1;
  f.scenario[218]=f.scenario[219]=f.scenario[224]=1;f.local[1]=left;f.local[2]=right;f.local[3]=bridge;f.local[4]=f.local[5]=1;f.local[6]=collapse;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=408;s.submap=baked::submap_by_map_id(408);s.layer=1;s.position=world_of_cell(*s.submap,collapse?Cell{1,87,18}:Cell{1,83,18});s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
  s.goal.map_id=408;s.goal.entity_id=entity;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;s.kind=TargetKind::Interact;
  for(const auto& source:baked::grids(*s.submap).objects){
   if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::Warp||source.type==baked::ObjType::GotoMap)continue;
   if(source.entity_id()==entity){s.goal.cell={source.layer,source.tx,source.ty};s.goal.raw_name=baked::object_identifier(source);}
   if(!s.state.active(*s.submap,source,true))continue;
   const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
  }
  Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  if(entity==34&&left==1&&right==0&&bridge==0){
   const auto& grid=baked::grids(*s.submap);
   std::printf("Island toggle native cardinal approaches:");
   for(int layer:{0,1,2})for(auto d:std::array<std::pair<int,int>,4>{{{0,-1},{1,0},{0,1},{-1,0}}})
    std::printf(" L%d/%d,%d=%d",layer,87+d.first,13+d.second,baked::walkable(grid,layer,87+d.first,13+d.second,true));
   std::printf("\n");
  }
  std::printf("Sealed islands collapsed%d local%d/%d/%d to%d: success%d trap%d %s %s\n",collapse,left,right,bridge,entity,p.path.success,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
 }
 if(fixture::wrong_owner)++failures;
 std::printf("Initial sealed-gate physical census: owner%d failures%d; timed/step recipe qualification pending\n",fixture::wrong_owner.load(),failures);return failures?1:0;
}
namespace sealed_fixture {
using namespace field;using namespace baked_nav;
int failures=0,journeys=0,transfers=0;
void expect(bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}}
bool contains(const std::vector<int>& values,int id){return std::find(values.begin(),values.end(),id)!=values.end();}
void obstacles(RouteSnapshot& s,int mode=0,int a=0,int b=0){
 const std::vector<int> af0{23,24,29,30,31,32,34,35,37,87},af1{20,21,22,25,26,27,28,33,36,86,90},bf0{37,40,41,42,43,49,50,89},bf1{36,38,39,44,45,46,47,48};
 const std::vector<int> ac0{19,73,74,75,76,77,78,79,80,81},ac1{18,64,65,66,67,68,69,70,71,72,91},bc0{19,58,59,60,61,62,63,88},bc1{18,51,52,53,54,55,56,57};
 for(const auto& source:baked::grids(*s.submap).objects){
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp)continue;
  const int id=source.entity_id();bool active=s.state.active(*s.submap,source,true);
  if(s.map_id==407&&mode){
   const auto has=[&](const auto& v){return contains(v,id);};
   if(has(af0)||has(af1)||has(bf0)||has(bf1))active=mode==1?(a?has(af1):has(af0)):(b?has(bf1):has(bf0));
   if(mode==1&&(has(ac0)||has(ac1)))active=a?has(ac1):has(ac0);
   if(mode==2&&(has(bc0)||has(bc1)))active=b?has(bc1):has(bc0);
  }
  if(!active)continue;
  const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=id;n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
 }
}
map_state::Flags flags(){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.local.fill(0);f.area.fill(0);f.global.fill(0);f.global[0]=2;f.global[5]=1;f.current_area_id=63;f.current_map_id=408;return f;}
MapGoal actor(int map,int id){
 MapGoal goal;goal.map_id=map;goal.entity_id=id;goal.avoid_other_events=true;goal.allow_scripted_crossings=true;
 for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==id){goal.cell={o.layer,o.tx,o.ty};goal.raw_name=baked::object_identifier(o);return goal;}
 expect(false,"exact native actor exists");return goal;
}
MapGoal entrance(int map,int point,map_state::Flags f){
 map_state::Resolver state(f,fixture::records);const auto* in=point_in(*baked::submap_by_map_id(map),point,state);expect(in!=nullptr,"exact native floor entry exists");
 MapGoal goal;goal.map_id=map;goal.cell=in?Cell{in->layer,in->tx,in->ty}:Cell{};goal.avoid_other_events=true;goal.allow_scripted_crossings=true;return goal;
}
bool follow(map_state::Flags& f,Cell& at,const MapGoal& goal,TargetKind kind,bool expected=true){
 ++journeys;
 for(int leg=0;leg<12;++leg){
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.goal=goal;s.kind=kind;
  obstacles(s,s.map_id==407?f.local[1].value_or(0):0,f.local[2].value_or(0),f.local[3].value_or(0));Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Sealed route%d leg%d from%d/%d,%d,%d to%d/%d: success%d next%d trap%d %s %s\n",journeys,leg,f.current_map_id,at.layer,at.x,at.y,goal.map_id,goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!expected){expect(!p.path.success,"closed native bridge phase/approach remains unavailable");return !p.path.success;}
  expect(p.path.success&&!p.path.required_trap,"real route reaches its objective without an unrelated trap");if(!p.path.success)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"floor/manual continuation supplies a real owned arrival");if(!receipt)return false;
  if(receipt->destination==f.current_map_id&&receipt->entity_id==69)f.local[4]=1;
  else if(receipt->destination!=f.current_map_id)f.local.fill(0);
  f.current_map_id=receipt->destination;at=receipt->arrival;++transfers;
  if(f.current_map_id==411)f.area[1]=1;
 }
 expect(false,"complete journey terminates within twelve floor continuations");return false;
}
int qualify(){
 std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 const std::vector<std::vector<Cell>> paths{{{1,8,4},{1,9,4},{1,10,4},{1,10,5},{1,10,6}},{{1,10,6},{1,10,7},{1,11,7},{1,12,7}},{{1,12,7},{1,12,6},{1,12,5},{1,13,5},{1,13,4}},{{1,13,4},{1,14,4},{1,15,4}},{{1,16,5},{1,16,6},{1,16,7},{1,16,8}},{{1,16,8},{1,16,9},{1,15,9}},{{1,15,9},{1,15,10},{1,16,10},{1,17,10},{1,18,10},{1,19,10},{1,19,11}},{{1,19,11},{1,19,12},{1,19,13},{1,18,13}}};
 for(size_t i=0;i<paths.size();++i)for(int held:{0,1}){
  const int mode=i<4?1:2,phase=int(i%4%2);auto f=flags();f.current_map_id=407;f.local[1]=mode;f.local[2]=mode==1?phase:held;f.local[3]=mode==2?phase:held;f.local[4]=mode==2;
  auto at=paths[i].front();MapGoal goal;goal.map_id=407;goal.cell=paths[i].back();goal.avoid_other_events=true;follow(f,at,goal,TargetKind::Spot);
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=407;s.submap=baked::submap_by_map_id(407);s.vehicle={true,1,baked::transport(1)->type,{}};obstacles(s,mode,f.local[2].value_or(0),f.local[3].value_or(0));
  std::thread edge_worker([&]{SnapshotScope scope(s);const auto& grid=baked::grids(*s.submap);const auto live=live_blockers(*s.submap,grid,{});auto r=walk_request(*s.submap,paths[i].front());r.blocked=live.hard();r.directional=live.directional;live.apply_transport(r);r.live_hazards=r.live_traps=true;r.hazards=live.triggers;r.traps=live.traps;r.goals={paths[i].back()};for(size_t j=1;j<paths[i].size();++j)expect(baked_route::edge_allowed(r,grid,paths[i][j-1],paths[i][j]),"every spoken preview step retains its exact legal native cardinal edge");});edge_worker.join();
 }
 auto f=flags();f.current_map_id=405;map_state::Resolver first(f,fixture::records);const auto* in=point_in(*baked::submap_by_map_id(405),1,first);Cell at{in->layer,in->tx,in->ty};follow(f,at,entrance(407,1,f),TargetKind::Spot);
 f=flags();at={2,13,9};follow(f,at,actor(408,69),TargetKind::Interact);
 f.local[4]=1;at={1,45,24};follow(f,at,actor(408,65),TargetKind::Interact);f.scenario[218]=1;
 follow(f,at,actor(408,60),TargetKind::Interact);f.scenario[219]=1;
 for(const auto pair:std::vector<std::pair<int,int>>{{58,220},{57,221},{56,222},{55,223}}){follow(f,at,actor(408,pair.first),TargetKind::Interact);f.scenario[pair.second]=1;}
 follow(f,at,actor(408,40),TargetKind::Spot);f.scenario[224]=1;
 follow(f,at,actor(408,48),TargetKind::Spot);f.local[5]=1;
 auto before_collapse=f;Cell before_at=at;before_collapse.local[3]=1;follow(before_collapse,before_at,actor(408,34),TargetKind::Interact);before_collapse.local[3]=2;follow(before_collapse,before_at,actor(408,46),TargetKind::Spot);
 follow(f,at,actor(408,46),TargetKind::Spot);f.local[6]=1;
 follow(f,at,actor(408,38),TargetKind::Interact);f.local[1]=1;
 follow(f,at,actor(408,17),TargetKind::Interact);f.treasure[140]=1;
 auto right=f;auto right_at=at;right.local[2]=1;follow(right,right_at,entrance(411,1,right),TargetKind::Spot);
 follow(f,at,entrance(411,1,f),TargetKind::Spot);follow(f,at,actor(411,5),TargetKind::Spot);
 auto reset=flags();reset.local[4]=reset.local[5]=reset.local[6]=1;reset.local[3]=1;reset.scenario[218]=reset.scenario[219]=reset.scenario[224]=1;Cell reset_at{1,87,18};follow(reset,reset_at,actor(408,34),TargetKind::Interact,false);follow(reset,reset_at,entrance(410,1,reset),TargetKind::Spot);follow(reset,reset_at,entrance(408,4,reset),TargetKind::Spot);expect(reset.local[3]==0,"native side-room round trip clears the closed bridge state");
 expect(!fixture::wrong_owner,"no native API enters the route worker");std::printf("Sealed Gate physical qualification: %d journeys/%d continuations/%d failures/owner%d\n",journeys,transfers,failures,fixture::wrong_owner.load());return failures?1:0;
}
}
int main(int argc,char** argv){return argc>1&&std::string(argv[1])=="--diagnostic"?diagnostic():sealed_fixture::qualify();}
