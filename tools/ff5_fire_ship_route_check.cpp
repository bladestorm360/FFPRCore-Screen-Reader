// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_ship_policy_main
#include "ff5_fire_ship_policy_check.cpp"
#undef main
#include "../src/games/ff5/mechanics_model.cpp"
namespace ship_routes {
using namespace ship_policy;
int assertions=0,errors=0,legs=0,cells=0;
void check(bool v,const char* why){++assertions;if(!v){++errors;if(errors<12)std::printf("FAIL %s\n",why);}}
ff5::library::Plan advice(int mask,Cell at,int map=270,std::wstring id={}){ff5::mechanics::Snapshot s;s.flags=flags(mask);s.map=map;s.player=at;s.guide_id=id;s.locale=L"en";return ff5::mechanics::plan(s,fixture::guide_recipes);}
Planned route(int mask,Cell at,const field::baked_nav::GuideTarget& target,int map=270){auto state=flags(mask);state.current_map_id=map;state.scenario[44]=1;RouteSnapshot s(map_state::Resolver(state,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates={target};
 for(const auto& source:baked::grids(*s.submap).objects){if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!s.state.active(*s.submap,source,true))continue;const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
 ++legs;return plan_snapshot(s);}
GuideTarget destination(int map, int entity, baked::ObjType type, TargetKind kind)
{
 const auto& grid=baked::grids(*baked::submap_by_map_id(map));
 for(const auto& object:grid.objects)if(object.entity_id()==entity&&object.type==type){
  MapGoal goal;goal.map_id=map;goal.cell={object.layer,object.tx,object.ty};
  goal.entity_id=entity;goal.raw_name=baked::object_identifier(object);
  return {std::move(goal),kind,{}};
 }
 std::abort();
}
void verify_pathfinder()
{
 auto boss=destination(271,1,baked::ObjType::Event,TargetKind::Infer);
 boss.goal.storyline=true;
 const auto entrance=route(0,{1,5,11},boss,269).path;
 check(entrance.success&&entrance.next_map_id==270,"Storyline from the save room reaches the real passage entrance");
 const auto approach=route(0,{1,10,21},boss).path;
 check(approach.success&&!approach.walk_phases.empty()&&approach.walk_phases.back().manual_after&&
       approach.walk_phases.back().action_after.find(T(L"Operate the switch."))!=std::wstring::npos,
       "Storyline from the lower platform gives the first reachable switch and native facing");
 const auto chest=destination(270,17,baked::ObjType::TreasureBox,TargetKind::Interact);
 check(route(0,{1,5,11},chest,269).path.success,"The engine room chest is reachable through the passage entrance");
 check(route(0,{1,10,21},chest).path.success,"The engine room chest receives the passage prerequisite");
 const auto switch_two=destination(270,19,baked::ObjType::Entity,TargetKind::Interact);
 check(route(0,{1,5,11},switch_two,269).path.success,"A specific upper switch is reachable through the passage entrance");
 check(route(0,{1,10,21},switch_two).path.success,"A selected switch has a verified route of its own");
 const auto earlier=route(0,{1,13,20},boss,261).path;
 check(earlier.success,"Storyline from the earlier engine room reaches a passage prerequisite");
 const auto hidden_chest=destination(259,0,baked::ObjType::TreasureBox,TargetKind::Interact);
 const auto locked=route(0,{1,8,11},hidden_chest,256).path;
 check(locked.success&&!locked.walk_phases.empty()&&
       locked.walk_phases.back().action_after==T(L"Examine the switch"),
       "The one-switch side room explains which control opens the chest route");
 struct Lift { int lower, from_x, lower_exit; };
 constexpr Lift lifts[]{{257,5,16},{260,4,14},{263,4,14},{264,4,17},{266,5,14}};
 for(int i=0;i<5;++i){
  const auto& lift=lifts[i];
  const int upper=634+i;
  const auto check_lift=[&](int from, Cell at, int to, int exit){
   const auto goal=destination(to,exit,baked::ObjType::GotoMap,TargetKind::Transition);
   const auto elevator=route(0,at,goal,from).path;
   const bool usable=elevator.success&&elevator.next_map_id==to&&!elevator.walk_phases.empty()&&
       elevator.walk_phases.back().manual_after&&elevator.description.find(T(L"Elevator"))!=std::wstring::npos;
   if(!usable)std::printf("Elevator %d -> %d: %ls / %ls / %ls\n",from,to,elevator.reason.c_str(),
       elevator.error.c_str(),elevator.description.c_str());
   check(usable,"The lift is a reachable, player-operated elevator from its native landing");
  };
  check_lift(lift.lower,{2,lift.from_x,8},upper,16);
  check_lift(upper,{2,5,8},lift.lower,lift.lower_exit);
 }
 const auto* pipe_room=baked::submap_by_map_id(265);
 const auto& grid=baked::grids(*pipe_room);
 constexpr Cell landings[]{{1,10,27},{1,16,27},{1,13,26},{1,17,9}};
 for(int i=0;i<4;++i){
  const auto warp=destination(265,11+i,baked::ObjType::Warp,TargetKind::Transition);
  const auto* contact=baked::world_exit_at(grid,11+i,warp.goal.raw_name,warp.goal.cell.x,warp.goal.cell.y);
  check(contact&&contact->a==261&&(contact->d&baked::kAutomaticCurrent),"The exact pipe actor owns a native transfer");
  if(!contact)continue;
  auto state=map_state::Resolver(flags(0),fixture::records);
  const auto* arrival=point_in(*baked::submap_by_map_id(261),contact->b,state);
  check(arrival&&Cell{arrival->layer,arrival->tx,arrival->ty}==landings[i],"The pipe arrives on its exact post-script tile");
 }
}
bool verify(){
 const auto* guide=fixture::guide_recipes.get(L"fire_ship_guide");if(!guide)return false;
 for(const auto& n:nodes){const auto p=advice(n.mask,n.at);check(p.supported&&!p.unknown&&p.targets.size()==1&&!p.observe_waypoint_arrival,"every native state has one operation-owned production goal");if(p.targets.size()!=1)return false;
  const int code=int(std::string_view(alphabet).find(char(guide->get(L"rows")->arr[n.mask].s[n.at.y*25+n.at.x])));const auto& t=p.targets.front();
  check(code<=32?t.kind==TargetKind::Spot&&t.goal.entity_id==-1&&t.goal.cell==stand(code):t.kind==TargetKind::Transition&&t.goal.entity_id==(code==33?16:15),"production goal matches the independently computed native action and side");
  if(code==33)check(n.distance==0,"production exit is chosen only from its real reachable walking region");
  else{const auto edge=std::find_if(n.edges.begin(),n.edges.end(),[&](auto e){return e.code==code;});check(edge!=n.edges.end()&&nodes[edge->to].distance<n.distance,"every shipped policy choice strictly advances the independent native journey,including recovery");}
  const auto r=route(n.mask,n.at,t);check(r.path.success&&!r.path.required_trap&&!r.path.npc_approach,"every native state reaches its exact goal using the complete production planner");if(!r.path.success){std::printf("mask%d from%d,%d code%d %s %s\n",n.mask,n.at.x,n.at.y,code,text::to_utf8(r.path.reason).c_str(),text::to_utf8(r.path.error).c_str());return false;}
  if(code<=32){const auto& end=r.path.walk_phases.back().cells.back();check(end==stand(code)&&!r.path.next_map_id,"switch legs end at the authored cardinal standing point without leaving the room");}
  else if(code==33)check(p.finish_map==271,"only the real onward exit arms full271 completion");
  else{const auto back=advice(0,{1,5,5},269,L"fire_ship_passages");check(back.targets.size()==1&&back.targets.front().goal.entity_id==6&&!back.finish_map,"native reset room returns through its actual exit without completing");if(back.targets.size()!=1)return false;check(route(0,{1,5,5},back.targets.front(),269).path.success,"actual native reset landing reaches the real return entrance");}
  // Current position is an observation dimension,not a persistent step count.
  for(int y=0;y<27;++y)for(int x=0;x<25;++x)if(n.reach.reachable({1,x,y})){++cells;const auto cell=advice(n.mask,{1,x,y});if(cell.targets.size()!=1||cell.targets.front().goal.cell!=t.goal.cell||cell.targets.front().instruction!=t.instruction)return false;}
 }
 ff5::mechanics::Snapshot s;s.map=270;s.player={1,10,21};s.flags=flags(0);s.policy={false,true};check(ff5::mechanics::plan(s,fixture::guide_recipes).targets.empty(),"Navigation disable independently suppresses the passage solution");s.policy={true,false};check(!ff5::mechanics::plan(s,fixture::guide_recipes).targets.empty(),"Other disable preserves this navigation puzzle");s.flags.scenario[304].reset();check(ff5::mechanics::plan(s,fixture::guide_recipes).unknown,"unknown layout bit never fabricates a solution");s.flags=flags(0);s.cancelled=[]{return true;};check(ff5::mechanics::plan(s,fixture::guide_recipes).targets.empty(),"cancelled policy never publishes a target");
 check(!advice(0,{1,5,5},269).supported,"ordinary adjacent room cannot start or advertise an unowned guide");
 verify_pathfinder();
 std::printf("Ship production:%d assertions/%d physical legs/%d cell observations/%d failures\n",assertions,legs,cells,errors);return !errors;
}
}
int main(){using namespace ship_routes;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)||!json::parse(read("../tools/puzzle_audit_20261030/broader_audit_20261104/ff5_FireShip_Context/physical_facts.json"),facts))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;room=baked::submap_by_map_id(270);consumer=verify;int result=2;std::thread worker([&]{result=compile();});worker.join();return result;}
