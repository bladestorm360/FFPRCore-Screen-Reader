// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual production planner, native flags/actors and complete floor continuations.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0,transfers=0;
 const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,progress,local1,local2,local3,area1,goal_map,entity;const wchar_t* group=nullptr;Cell start{};int hanging=0;bool possible=true;};
 std::vector<Case> cases{{442,1,0,0,0,0,0,442,19},{442,1,1,0,0,0,0,442,20},{442,1,2,0,0,0,0,442,18},{442,1,3,0,0,0,0,442,15},{442,1,4,0,0,0,0,442,13},{442,1,5,0,0,0,0,444,2},
 {443,1,5,0,0,0,0,444,2},{444,1,5,0,0,0,0,444,2},{444,1,6,0,0,0,0,445,-2},
 {445,1,6,0,0,0,0,445,88,L"ev_e_1064"},
 {445,5,6,0,2,0,0,445,56},
 {445,6,6,1,2,0,1,445,1,L"ev_e_0771",{1,6,24},4},
 {445,6,7,1,2,0,1,446,7,nullptr,{1,6,24},4},
 {445,7,7,2,2,0,1,446,7,nullptr,{},4},
 {446,1,7,0,0,0,1,446,7,nullptr,{},4},
 {446,1,8,0,0,0,1,446,8,nullptr,{1,11,14},4},
 {446,1,8,0,0,0,1,446,9,L"ev_e_0774",{},4},
 {446,1,8,0,0,0,1,446,8,nullptr,{1,11,18},4}};
 for(int phase=0;phase<3;++phase)cases.push_back({445,1,6,0,1,phase,0,445,-1,nullptr,{1,30,31}});
 cases.push_back({445,3,6,0,1,0,0,445,55});
 cases.push_back({445,3,6,0,1,1,0,445,55,nullptr,{},0,false});
 cases.push_back({445,3,6,0,1,2,0,445,55,nullptr,{},0,false});
 for(int i=0;i<4;++i)cases.push_back({445,6,6,1,2,0,1,445,43+2*i,i==0?L"ev_e_0767":i==1?L"ev_e_0768":i==2?L"ev_e_0769":L"ev_e_0770",{1,6,24},i});
 cases.push_back({445,1,9,0,0,0,0,445,56});
 for(int i=0;i<4;++i)cases.push_back({445,6,9,1,2,0,1,445,43+2*i,i==0?L"ev_e_0767":i==1?L"ev_e_0768":i==2?L"ev_e_0769":L"ev_e_0770",{1,6,24},i});
 std::set<std::tuple<int,int,std::wstring>> covered;
 for(const auto test:cases){
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_area_id=36;f.global[0]=4;f.global[5]=1;f.scenario[298]=1;
  for(int i=1;i<=test.progress;++i)f.scenario[i==1?379:i==2?577:i==3?380:i==4?381:i==5?382:i==6?384:i==7?389:390]=1;
  if(test.progress>=9)f.scenario[391]=1;
  for(int i=0;i<test.hanging;++i)f.scenario[385+i]=1;
  f.local[1]=test.local1;f.local[2]=test.local2;f.local[3]=test.local3;f.area[1]=test.area1;f.current_map_id=test.map;
  map_state::Resolver state(f,fixture::records);const auto* sm=baked::submap_by_map_id(test.map);const auto* point=point_in(*sm,test.point,state);if(!point)return 2;
  MapGoal goal;goal.map_id=test.goal_map;goal.entity_id=test.entity;goal.avoid_other_events=true;goal.allow_scripted_crossings=true;TargetKind kind=TargetKind::Interact;
  for(const auto& source:baked::grids(*baked::submap_by_map_id(test.goal_map)).objects)if(source.type!=baked::ObjType::PointIn&&source.type!=baked::ObjType::Warp&&source.entity_id()==test.entity&&(!test.group||baked::str(source.group)==test.group)){
   goal.cell={source.layer,source.tx,source.ty};goal.raw_name=baked::object_identifier(source);if(baked_route::step_event(source))kind=TargetKind::Spot;
  }
  if(test.entity<0){goal.entity_id=-1;goal.cell=test.entity==-1?Cell{1,23,23}:Cell{1,30,32};kind=TargetKind::Spot;}
  int map=test.map;Cell at=test.start==Cell{}?Cell{point->layer,point->tx,point->ty}:test.start;bool reached=false;
  for(int leg=0;leg<12;++leg){
   auto current=f;current.current_map_id=map;RouteSnapshot s(map_state::Resolver(current,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.goal=goal;s.kind=kind;
   for(const auto& source:baked::grids(*s.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*s.submap,source,true))continue;
    const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    // Current post-callback capture uses real collider enablement. Initializer
    // replay alone cannot represent a later timer phase. Never remove terrain.
    if(map==445&&test.local2==1&&n.collider&&n.id>=85&&n.id<=87)n.enabled=n.id!=85+test.local3;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
   }
   Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
   std::printf("Owzer from%d point%d progress%d local%d/%d/%d area%d to%d/%d leg%d map%d: success%d next%d %s %s\n",test.map,test.point,test.progress,test.local1,test.local2,test.local3,test.area1,test.goal_map,test.entity,leg,map,p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
   if(!test.possible){expect(!p.path.success,"closed left door remains a real native collider");reached=!p.path.success;break;}
   expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"native stage avoids traps and unrelated encounters");if(!p.path.success)break;
   if(!p.path.next_map_id){reached=true;break;}
   const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"each physical transfer has an owned native arrival");if(!receipt)break;
   if(map==445&&receipt->destination==445){if(receipt->entity_id==55)f.local[2]=2;if(receipt->entity_id==56){f.local[1]=1;f.area[1]=1;}if(receipt->entity_id==57)f.local[1]=2;}
   map=receipt->destination;at=receipt->arrival;++transfers;
  }
  expect(reached,"finite complete floor continuation reaches the actual operation");
  if(reached&&test.possible&&test.entity>=0)covered.emplace(goal.map_id,goal.entity_id,goal.raw_name);
 }
 json::Value recipes;expect(json::parse(read("data/ff6/puzzles.json"),recipes),"actual recipe loads");
 for(const auto& guide:recipes.get(L"story_guides")->arr)if(guide.get(L"id")->s==L"owzers_mansion")for(const auto& rule:guide.get(L"rules")->arr)if(const auto* targets=rule.get(L"targets"))for(const auto& row:targets->arr)if(row.get(L"entity"))expect(covered.count({int(row.get(L"map")->n),int(row.get(L"entity")->n),row.get(L"raw")->s})!=0,"every authored operation has a complete physical witness");
 expect(!fixture::wrong_owner,"native APIs stay off the route worker");std::printf("Owzer routes:%zu cases/%d continuations/%d failures/owner%d\n",cases.size(),transfers,failures,fixture::wrong_owner.load());return failures?1:0;
}
