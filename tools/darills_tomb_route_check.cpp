// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real Darill floors, water state, native turtle travel and story objectives.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0,transfers=0;
 const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,progress,area,target_map,entity;const wchar_t* raw=nullptr;int memory=0;bool possible=true;};
 const std::vector<Case> cases{{456,1,0,0,458,1},{458,1,1,0,459,1},{459,1,2,0,463,24},{463,1,2,0,463,24},{463,1,3,0,463,1},{463,1,4,0,468,4},{465,1,4,1,468,4},{468,1,4,1,468,4},{468,1,5,1,468,5},{463,2,3,0,463,1},{465,1,4,1,457,-1},{463,1,2,0,464,-1},{463,1,4,0,465,-1},{463,1,4,0,463,22},{463,1,4,1,468,4},{463,4,4,1,468,4},
 {464,1,2,0,464,0,L"石碑を調べる1"},{464,1,2,0,464,0,L"石碑を調べる2"},{464,1,2,0,464,0,L"石碑を調べる3"},{464,1,2,0,464,0,L"石碑を調べる4"},{464,1,2,0,466,0},{466,1,2,0,461,9},{467,1,2,0,461,9},
 {577,1,5,1,577,2,nullptr,1},{577,1,5,1,577,2,nullptr,2},{577,1,5,1,577,2,nullptr,3},
 {458,1,0,0,459,1,nullptr,0,false},{463,1,3,0,468,4,nullptr,0,false}};
 std::set<std::tuple<int,int,std::wstring>> covered;
 for(const auto test:cases){
  map_state::Flags f;f.scenario.fill(0);f.global.fill(0);f.local.fill(0);f.area.fill(0);f.treasure.fill(0);f.current_map_id=test.map;f.current_area_id=47;f.global[0]=4;f.global[5]=1;f.area[1]=test.area;
  for(int i=1;i<=test.progress;++i)f.scenario[i==5?352:340+i]=1;
  if(test.target_map==466||test.entity==9)for(int i=346;i<=350;++i)f.scenario[i]=1;
  if(test.memory){f.scenario[526]=1;f.scenario[527]=test.memory>1;f.scenario[528]=test.memory>2;}
  auto make=[&](int map,Cell cell){
   auto current=f;current.current_map_id=map;RouteSnapshot s(map_state::Resolver(current,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=cell.layer;s.position=world_of_cell(*s.submap,cell);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
   for(const auto& source:baked::grids(*s.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*s.submap,source,true))continue;
    const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}
    if(map==577&&n.id==2)n.cell=test.memory==1?Cell{1,21,12}:Cell{1,8,24};
    s.obstacles.push_back(n);
   }
   return s;
  };
  map_state::Resolver state(f,fixture::records);const auto* sm=baked::submap_by_map_id(test.map);const auto* point=point_in(*sm,test.point,state);if(!point)return 2;
  MapGoal goal;goal.map_id=test.target_map;goal.entity_id=test.entity;goal.avoid_other_events=true;goal.allow_scripted_crossings=true;TargetKind kind=TargetKind::Interact;
  for(const auto& source:baked::grids(*baked::submap_by_map_id(test.target_map)).objects)if(source.type!=baked::ObjType::PointIn&&source.type!=baked::ObjType::Warp&&source.entity_id()==test.entity&&(!test.raw||baked::object_identifier(source)==test.raw)&&(test.entity!=22||baked::str(source.group)==L"entity_default")){const auto o=state.effective(*baked::submap_by_map_id(test.target_map),source);goal.cell={o.layer,o.tx,o.ty};goal.raw_name=baked::object_identifier(source);if(baked_route::step_event(o))kind=TargetKind::Spot;}
  if(test.entity<0){const auto* dest=point_in(*baked::submap_by_map_id(test.target_map),1,state);if(!dest)return 2;goal.cell={dest->layer,dest->tx,dest->ty};kind=TargetKind::Spot;}
  int map=test.map;Cell at{point->layer,point->tx,point->ty};bool reached=false;
  if(test.memory)at=test.memory==1?Cell{1,29,5}:Cell{1,12,21};
  if(test.possible&&test.entity>=0)covered.emplace(goal.map_id,goal.entity_id,goal.raw_name);
  for(int leg=0;leg<24;++leg){
   auto s=make(map,at);s.goal=goal;s.kind=kind;
   Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
   std::printf("Darill from%d point%d progress%d area%d to%d/%d leg%d map%d at%d,%d L%d: success%d next%d npc%d %s %s\n",test.map,test.point,test.progress,test.area,test.target_map,test.entity,leg,map,at.x,at.y,at.layer,p.path.success,p.path.next_map_id,p.path.npc_approach,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
   if(!test.possible){expect(!p.path.success,"native closed-water/stone prerequisite remains a real gate");reached=!p.path.success;break;}
   expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"native objective uses actual gates/actors without trap or optional enemy");if(!p.path.success)break;
   if(!p.path.next_map_id){reached=true;break;}
   const auto receipt=p.pending.incoming_transition;expect(receipt.has_value(),"actual passage/turtle has an owned arrival");if(!receipt)break;
   if(map==463&&receipt->destination==463){if(receipt->entity_id==22||receipt->entity_id==23)f.area[1]=1;if(receipt->entity_id==20)f.area[1]=0;}
   map=receipt->destination;at=receipt->arrival;++transfers;
  }
  expect(reached,"finite complete water/turtle continuation reaches the operation");
 }
 json::Value recipes;expect(json::parse(read("data/ff6/puzzles.json"),recipes),"actual recipe loads");
 for(const auto& guide:recipes.get(L"story_guides")->arr)if(guide.get(L"id")->s==L"darills_tomb")for(const auto& rule:guide.get(L"rules")->arr)for(const auto& row:rule.get(L"targets")->arr)expect(covered.count({int(row.get(L"map")->n),int(row.get(L"entity")->n),row.get(L"raw")->s})!=0,"every actual recipe objective has a complete physical witness");
 expect(!fixture::wrong_owner,"native calls remain off the route worker");std::printf("Darill routes:%zu objectives/%d continuations/%d failures/owner%d\n",cases.size(),transfers,failures,fixture::wrong_owner.load());return failures?1:0;
}
