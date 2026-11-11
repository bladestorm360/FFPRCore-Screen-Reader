// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Replay actual recipe goals through every native manual jump/landing.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 json::Value puzzles;if(!json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;int failures=0,jumps=0,completed=0;
 const auto expect=[&](bool pass,const char* message){if(!pass){++failures;std::printf("FAIL %s\n",message);}};
 map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.scenario[967]=1;flags.global[0]=4;
 const auto make=[&](int map,Cell at,bool guards){
  auto f=flags;f.current_map_id=map;RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=at.layer;
  s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
  // Real primary initializer poses stand in for the copied live actor list.
  // Never prove a jump or a closed gate on an empty-obstacle floor.
  for(const auto& source:baked::grids(*s.submap).objects){
   if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||!s.state.active(*s.submap,source,true))continue;
   const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.cell={o.layer,o.tx,o.ty};n.raw_name=baked::object_identifier(source);
   n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;if(n.npc&&!guards)continue;
   n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);
   if(o.type==baked::ObjType::Warp){if((o.d&255)!=1)continue;n.type=2;}
   n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);
  }
  return s;
 };
 const auto point=[&](int map,int id){auto f=flags;f.current_map_id=map;map_state::Resolver r(f,fixture::records);const auto* sm=baked::submap_by_map_id(map);const auto* p=point_in(*sm,id,r);if(!p)std::abort();return Cell{p->layer,p->tx,p->ty};};
 const auto find_goal=[&](int map,int entity){
  GuideTarget t;bool found=false;
  for(const auto& guide:puzzles.get(L"story_guides")->arr)if(guide.get(L"id")->s==L"zone_eater_bridges"||guide.get(L"id")->s==L"zone_eater_stepping_stones")
   for(const auto& rule:guide.get(L"rules")->arr)for(const auto& row:rule.get(L"targets")->arr)if(row.get(L"map")->n==map&&row.get(L"entity")&&row.get(L"entity")->n==entity){
    expect(!found,"one recipe owns each physical objective");found=true;t.goal.map_id=map;t.goal.entity_id=entity;t.goal.raw_name=row.get(L"raw")->s;
    t.goal.avoid_other_events=row.get(L"avoid_other_events")->b;t.goal.allow_scripted_crossings=row.get(L"allow_scripted_crossings")->b;
    t.kind=row.get(L"kind")->s==L"spot"?TargetKind::Spot:row.get(L"kind")->s==L"transition"?TargetKind::Transition:TargetKind::Interact;
    for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::Warp&&o.type!=baked::ObjType::PointIn&&o.entity_id()==entity&&baked::object_identifier(o)==t.goal.raw_name)t.goal.cell={o.layer,o.tx,o.ty};
   }
  expect(found,"actual recipe objective exists");return t;
 };
 const auto run=[&](RouteSnapshot& s,const GuideTarget& t){s.goal=t.goal;s.kind=t.kind;Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();return p;};
 const auto replay=[&](int map,Cell at,const GuideTarget& t,bool guards){
  std::set<std::tuple<int,int,int>> seen;bool reached=false;
  for(int leg=0;leg<32;++leg){
   const bool fresh=seen.emplace(at.layer,at.x,at.y).second;expect(fresh,"manual landing makes progress without looping");if(!fresh)break;
   auto s=make(map,at,guards);auto p=run(s,t);
   std::printf("Zone map%d leg%d from%d,%d goal%d: success%d next%d steps%d %ls %ls\n",map,leg,at.x,at.y,t.goal.entity_id,p.path.success,p.path.next_map_id,p.path.step_count,p.path.reason.c_str(),p.path.full_description.c_str());
   expect(p.path.success&&!p.path.required_trap,"next objective needs no deliberate fall");if(!p.path.success)break;
   if(p.path.npc_approach){expect(guards,"only a live guard causes temporary waiting");return;}
   if(p.path.next_map_id>0){
    const auto& transfer=p.pending.incoming_transition;
    expect(transfer.has_value()&&!p.pending.incoming_trap,"manual jump retains real non-trap landing receipt");if(!transfer)break;
    expect(transfer->destination==map,"jump stays on its native floor");
    expect(!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after,"jump stops for manual native operation");
    at=transfer->arrival;++jumps;continue;
   }
   const bool pose=!p.path.walk_phases.empty()&&!p.path.walk_phases.back().cells.empty();expect(pose,"completion has a physical final pose");if(!pose)break;
   at=p.path.walk_phases.back().cells.back();expect(t.kind==TargetKind::Interact||at==t.goal.cell,"switch/exit reaches its exact tile");reached=true;++completed;break;
  }
  expect(reached,"finite manual-jump journey reaches its final objective");
 };
 auto bridge=find_goal(501,6);for(int p:{1,2,3})replay(501,point(501,p),bridge,false);replay(501,point(501,1),bridge,true);
 auto recovery=find_goal(502,8),return_exit=find_goal(502,2);
 auto closed=make(502,point(502,2),false);expect(!run(closed,return_exit).path.success,"return is story gated before actual switch receipt");
 replay(502,point(502,2),recovery,false);flags.local[1]=1;replay(502,recovery.goal.cell,return_exit,false);flags.local[1]=0;
 auto stones=find_goal(506,25),final_exit=find_goal(506,2);for(int p:{1,2})replay(506,point(506,p),stones,false);
 flags.scenario[409]=1;replay(506,stones.goal.cell,final_exit,false);replay(506,point(506,1),final_exit,false);
 replay(507,point(507,1),find_goal(507,1),true);
 for(auto test:std::array<std::array<int,3>,3>{{{{500,501,1}},{{503,504,1}},{{505,506,1}}}}){
  auto s=make(test[0],point(test[0],1),false);GuideTarget t;t.goal.map_id=test[1];t.goal.cell=point(test[1],test[2]);t.kind=TargetKind::Spot;t.goal.avoid_other_events=true;t.goal.allow_scripted_crossings=true;
  const auto p=run(s,t);expect(p.path.success&&p.path.next_map_id==test[1],"corridor uses its actual next-floor connection");
 }
 expect(!fixture::wrong_owner,"native access stays off the worker");
 std::printf("Zone crossings: %d manual legs/%d complete objectives/%d failures/owner%d (capped fixture, not game timing)\n",jumps,completed,failures,fixture::wrong_owner.load());return failures?1:0;
}
