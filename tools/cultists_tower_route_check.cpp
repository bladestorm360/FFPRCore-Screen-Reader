// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Full real staircase/hidden-room/summit/battle and actual return-floor routes.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0,continuations=0;
 const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 struct Case{int map,point,progress,goal_map,entity;bool possible=true;int recruited=1;};std::vector<Case> cases;
 for(int map=476;map<=487;++map)if(map!=479)cases.push_back({map,1,0,478,5});
 for(const auto test:std::vector<Case>{{478,1,1,479,3},{479,1,1,479,3},{486,1,1,479,3},{479,1,2,487,3},{487,1,2,487,3},{487,1,3,486,1},{486,2,3,486,1},{486,2,4,476,-1},{487,1,4,476,-1},{477,1,4,476,-1},{478,1,0,479,3,false},{487,1,2,486,1,false}})cases.push_back(test);
 cases.push_back({476,1,0,478,5,true,0});
 for(const auto test:cases){
  map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.current_map_id=test.map;f.current_area_id=59;f.global[0]=4;f.global[5]=1;f.scenario[392]=test.recruited;
  if(test.progress>=1)f.scenario[421]=1;if(test.progress>=2)f.treasure[179]=1;if(test.progress>=3){f.scenario[423]=1;f.treasure[183]=1;}if(test.progress>=4)f.scenario[424]=1;
  auto make=[&](int map,Cell at){
   auto flags=f;flags.current_map_id=map;RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
   for(const auto& source:baked::grids(*s.submap).objects){
    if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*s.submap,source,true))continue;
    // A real current-floor snapshot contains only native instantiated actors.
    // This additional profile has no Relm: exact native branch5 skips0776.
    // Do not turn a conservative remote unknown into phantom live triggers.
    if(map==476&&!test.recruited&&baked::str(source.group)==L"ev_e_0776")continue;
    const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}s.obstacles.push_back(n);
   }return s;
  };
  map_state::Resolver state(f,fixture::records);const auto* sm=baked::submap_by_map_id(test.map);const auto* point=point_in(*sm,test.point,state);if(!point)return 2;
  MapGoal goal;goal.map_id=test.goal_map;goal.entity_id=test.entity;goal.avoid_other_events=true;goal.allow_scripted_crossings=true;TargetKind kind=TargetKind::Interact;
  for(const auto& o:baked::grids(*baked::submap_by_map_id(test.goal_map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==test.entity){goal.cell={o.layer,o.tx,o.ty};goal.raw_name=baked::object_identifier(o);if(baked_route::step_event(o))kind=TargetKind::Spot;}
  if(test.entity<0){const auto* end=point_in(*baked::submap_by_map_id(test.goal_map),2,state);if(!end)return 2;goal.cell={end->layer,end->tx,end->ty};kind=TargetKind::Spot;}
  int map=test.map;Cell at{point->layer,point->tx,point->ty};bool complete=false;
  for(int leg=0;leg<32;++leg){
   auto s=make(map,at);s.goal=goal;s.kind=kind;Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
   std::printf("Cultists from%d point%d phase%d to%d/%d leg%d map%d at%d,%d L%d success%d next%d: %s %s\n",test.map,test.point,test.progress,test.goal_map,test.entity,leg,map,at.x,at.y,at.layer,p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.full_description).c_str());
   if(!test.possible){expect(!p.path.success,"native hidden-door and pre-chest battle remain unavailable");complete=!p.path.success;break;}
   expect(p.path.success&&!p.path.required_trap&&!p.path.npc_approach,"real tower objective avoids unrelated events/optional dragon");if(!p.path.success)break;
   if(!p.path.next_map_id){complete=true;break;}const auto arrival=p.pending.incoming_transition;expect(arrival.has_value(),"floor route retains the actual native arrival");if(!arrival)break;map=arrival->destination;at=arrival->arrival;++continuations;
  }expect(complete,"complete finite staircase/hidden/summit/battle/return journey reaches its objective");
 }
 expect(!fixture::wrong_owner,"native calls stay off the worker");std::printf("Cultists Tower:%zu physical cases/%d floor continuations/%d failures/owner%d\n",cases.size(),continuations,failures,fixture::wrong_owner.load());return failures?1:0;
}
