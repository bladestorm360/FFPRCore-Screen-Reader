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
struct Opened { bool side_room=false, gloves=false, boss_defeated=false; };
Planned route(int mask,Cell at,const field::baked_nav::GuideTarget& target,int map=270,bool manual_request=false,Opened opened={}){auto state=flags(mask);state.current_map_id=map;state.scenario[43]=1;state.scenario[44]=1;state.scenario[45]=opened.boss_defeated;state.treasure[84]=opened.gloves;RouteSnapshot s(map_state::Resolver(state,fixture::records));s.map_id=map;s.submap=baked::submap_by_map_id(map);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";
 if(manual_request){s.goal=target.goal;s.kind=target.kind;}else s.candidates={target};
 for(const auto& source:baked::grids(*s.submap).objects){if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!s.state.active(*s.submap,source,true))continue;
  if(opened.side_room&&map==256&&(source.entity_id()==13||source.entity_id()==15))continue;
  const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
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
constexpr int ship_rooms[]{250,251,252,253,254,255,256,257,258,259,260,261,262,
                           263,264,265,266,268,269,270,271,634,635,636,637,638};

struct WalkArrival { Cell at; int layout; Opened opened; };

bool finish_walk(int map,Cell at,const GuideTarget& goal,Opened opened={},
                 WalkArrival* arrival=nullptr,int mask=0)
{
 std::set<std::tuple<int,int,int,int,int,bool,bool>> visited;
 for(int step=0;step<48;++step){
  if(!visited.emplace(map,at.layer,at.x,at.y,mask,opened.side_room,opened.boss_defeated).second)return false;
  const auto result=route(mask,at,goal,map,true,opened);
  if(!result.path.success||result.path.npc_approach||result.path.walk_phases.empty()){
   std::printf("Unreachable leg %d/%d,%d to %d/%d: %ls %ls\n",map,at.x,at.y,
       goal.goal.map_id,goal.goal.entity_id,result.path.error.c_str(),result.path.reason.c_str());
   return false;
  }
  const auto& phase=result.path.walk_phases.back();
  if(!result.path.next_map_id){
   if(!result.active->key_approach){
    if(map!=goal.goal.map_id||phase.cells.empty()||
       (goal.kind==TargetKind::Transition&&phase.cells.back()!=goal.goal.cell))return false;
    if(arrival)*arrival={phase.cells.back(),mask,opened};
    return true;
   }
   if(map==271&&!opened.boss_defeated&&result.path.event_on_route&&
      phase.action_after==T(L"Defeat Liquid Flame")&&!phase.cells.empty()&&phase.cells.back()==Cell{1,15,24}){
    // The victory continuation sets flag 45 and positions the controlled
    // player at (15,20), on the far side of the encounter contact.
    opened.boss_defeated=true;at={1,15,20};continue;
   }
   if(!phase.manual_after||phase.cells.empty()){
    std::printf("Unexpected prerequisite %d/%d,%d to %d/%d: %ls\n",map,at.x,at.y,
        goal.goal.map_id,goal.goal.entity_id,phase.action_after.c_str());
    return false;
   }
   const bool side_door=goal.goal.map_id==256&&goal.goal.entity_id==6&&goal.goal.cell==Cell{1,8,8};
   if(map==256&&(goal.goal.map_id==259||side_door)&&!opened.side_room){
    opened.side_room=true;at=phase.cells.back();continue;
   }
   if(map!=270||at.layer!=1||at.x<0||at.y<0||at.x>=width||at.y>=height)return false;
   const auto* guide=fixture::guide_recipes.get(L"fire_ship_guide");
   const auto* rows=guide->get(L"rows");
   if(goal.goal.map_id!=271){
    const int target=goal.goal.map_id==270?goal.goal.entity_id:15;
    if(target!=16)rows=guide->get(L"target_rows")->get(std::to_wstring(target));
   }
   if(!rows||rows->arr[mask].s.size()!=width*height)return false;
   const auto code=std::string_view(alphabet).find(static_cast<char>(rows->arr[mask].s[at.y*width+at.x]));
   if(code==0||code>32||phase.cells.back()!=stand(static_cast<int>(code)))return false;
   const auto& effect=facts.get(L"transitions")->arr[mask*32+code-1];
   mask=static_cast<int>(effect.get(L"after")->n);
   at=phase.cells.back();
   const auto* player=effect.get(L"player");
   if(player->type==json::Value::Type::Array)at={int(player->arr[2].n),int(player->arr[0].n),int(player->arr[1].n)};
   continue;
  }
  const auto& hop=result.pending.incoming_transition;
  if(!hop||hop->destination!=result.path.next_map_id||hop->random_transfer||
     (!result.path.step_count&&!phase.manual_after))return false;
  if(hop->destination!=map)opened.side_room=false;
  map=hop->destination;at=hop->arrival;
  if(map==269||map==271)mask=0;
 }
 return false;
}

void verify_ship_returns()
{
 const auto inside=[](int map){return std::find(std::begin(ship_rooms),std::end(ship_rooms),map)!=std::end(ship_rooms);};
 auto state=flags(0);state.scenario[43]=1;state.scenario[44]=1;
 map_state::Resolver resolver(state,fixture::records);
 const auto hub_chest=destination(253,17,baked::ObjType::TreasureBox,TargetKind::Interact);
 const auto save=destination(269,8,baked::ObjType::SavePoint,TargetKind::Spot);
 auto boss=destination(271,1,baked::ObjType::Event,TargetKind::Infer);
 boss.goal.storyline=true;
 const auto gloves=destination(261,26,baked::ObjType::TreasureBox,TargetKind::Interact);
 const auto gloves_return=route(0,{1,13,26},save,261,true,{false,true});
 const auto& current=gloves_return.pending.incoming_transition;
 check(gloves_return.path.success&&current&&current->entity_id==44&&
       current->entry==Cell{1,13,25}&&current->arrival==Cell{1,13,22}&&
       gloves_return.path.description.find(T(L"Ride the current"))!=std::wstring::npos,
       "The logged Thief's Gloves landing uses the native northbound return current");

 // Audit every playable arrival referenced by a native ship door, lift, pipe
 // or current. Source-only staging points and genuine story exits are excluded.
 std::set<std::tuple<int,int,int,int>> arrivals;
 for(int map:ship_rooms){
  const auto& floor=*baked::submap_by_map_id(map);
  for(const auto& edge:edges_of(baked::grids(floor),map)){
   if(!inside(edge.dest_map_id)||!resolver.active(floor,*edge.object)||resolver.consumed(floor,*edge.object))continue;
   const auto* point=point_in(*baked::submap_by_map_id(edge.dest_map_id),edge.dest_point,resolver);
   if(point)arrivals.emplace(edge.dest_map_id,point->layer,point->tx,point->ty);
  }
 }
 // The outside boarding point and native conveyor landings are usable
 // origins even though the internal door table does not point at them.
 const auto* deck=point_in(*baked::submap_by_map_id(250),1,resolver);
 if(deck)arrivals.emplace(250,deck->layer,deck->tx,deck->ty);
 arrivals.emplace(271,1,11,17);
 arrivals.emplace(271,1,19,17);
 check(arrivals.size()==64,"The replay includes every native ship arrival and both conveyor landings");
 for(const auto& [map,layer,x,y]:arrivals){
  const Cell at{layer,x,y};
  const bool home=finish_walk(map,at,hub_chest);
  const bool onward=finish_walk(map,at,boss);
  if(!home||!onward)std::printf("Arrival %d/%d %d,%d: hub=%d boss=%d\n",map,layer,x,y,home,onward);
  check(home&&onward,"Every native ship landing reaches both earlier rooms and the boss approach");
 }
 std::vector<GuideTarget> objectives;
 std::set<std::tuple<int,int,int,int,int>> unique;
 for(int map:ship_rooms){
  const auto& floor=*baked::submap_by_map_id(map);
  for(const auto& object:baked::grids(floor).objects){
   const bool exit=object.type==baked::ObjType::GotoMap&&inside(object.a);
   const bool lift=object.type==baked::ObjType::Warp&&(object.d&baked::kFireShipElevator);
   const bool chest=object.type==baked::ObjType::TreasureBox;
   if(!(exit||lift||chest)||!resolver.active(floor,object)||resolver.consumed(floor,object))continue;
   if(!unique.emplace(map,object.layer,object.tx,object.ty,static_cast<int>(object.type)).second)continue;
   MapGoal goal{map,{object.layer,object.tx,object.ty},lift,{},0,object.entity_id(),baked::object_identifier(object)};
   objectives.push_back({std::move(goal),exit?TargetKind::Transition:TargetKind::Interact,{}});
  }
 }
 for(const auto& goal:objectives){
  WalkArrival arrival;
  const bool reached=finish_walk(253,{1,16,20},goal,{},&arrival);
  if(!reached)std::printf("Outward %d/%d at %d,%d: no complete route\n",goal.goal.map_id,goal.goal.entity_id,goal.goal.cell.x,goal.goal.cell.y);
  check(reached,"The earlier ship can reach each internal door, lift and treasure chest");
  const auto& floor=*baked::submap_by_map_id(goal.goal.map_id);
  if(reached&&ordinary_chest_goal(baked::grids(floor),goal.goal.cell,goal.goal.entity_id)){
   arrival.opened.gloves=goal.goal.map_id==261&&goal.goal.entity_id==26;
   const bool returned=finish_walk(goal.goal.map_id,arrival.at,hub_chest,arrival.opened,nullptr,arrival.layout);
   if(!returned)std::printf("Chest return %d/%d from %d,%d in layout %d\n",
       goal.goal.map_id,goal.goal.entity_id,arrival.at.x,arrival.at.y,arrival.layout);
   check(returned,"Each chest's actual interaction position has a complete return to the earlier ship");
  }
 }
 std::printf("Ship connectivity audit: %zu playable native arrivals and %zu outward objectives\n",arrivals.size(),objectives.size());
 for(Cell landing:{Cell{1,10,27},Cell{1,16,27},Cell{1,13,26},Cell{1,17,9}}){
  check(finish_walk(261,landing,hub_chest),"A pipe landing has a complete return to the earlier ship");
  check(finish_walk(261,landing,save),"A pipe landing has a complete route back to the engine-room save point");
 }
 check(finish_walk(269,{1,5,11},gloves),"The route into the Thief's Gloves pocket still works");
 check(finish_walk(261,{1,13,26},boss,{false,true}),"Opening the Thief's Gloves chest leaves a complete Storyline route");
 const GuideTarget touching_door{MapGoal{262,{1,3,5}},TargetKind::Transition,{}};
 const auto reopen=route(0,touching_door.goal.cell,touching_door,262,true).path;
 check(reopen.success&&reopen.step_count==2&&!reopen.walk_phases.empty()&&
       reopen.walk_phases.back().cells.back()==touching_door.goal.cell,
       "Directions explicitly step away and back when the party arrives on a return door");
 const auto castle_exit=destination(271,3,baked::ObjType::GotoMap,TargetKind::Transition);
 const auto deck_exit=destination(250,2,baked::ObjType::GotoMap,TargetKind::Transition);
 check(finish_walk(261,{1,13,26},deck_exit,{false,true}),"The opened chest has a full return route to the outside exit");
 for(Cell landing:{Cell{1,11,17},Cell{1,19,17},Cell{1,15,20}})
  check(finish_walk(271,landing,hub_chest,{false,false,true}),
        "The far side and victory landing retain complete return routes with Liquid Flame defeated");
 check(finish_walk(271,{1,15,20},castle_exit,{false,false,true}),
       "Liquid Flame's completion preserves the route through the native castle exit");
 const auto* castle_entry=point_in(*baked::submap_by_map_id(271),2,resolver);
 check(castle_entry&&finish_walk(271,{castle_entry->layer,castle_entry->tx,castle_entry->ty},hub_chest,
       {false,false,true}),"Reentering from the castle reaches the earlier ship after Liquid Flame");
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
 const Cell carried{1,12,21};
 std::set<int> carried_layouts;
 for(const auto& node:nodes)if(node.reach.reachable(carried))carried_layouts.insert(node.mask);
 check(carried_layouts.size()==28,"All native layouts compatible with the logged carry landing are replayed");
 for(const auto& [map,entity] : {std::pair{261,26},std::pair{261,27},std::pair{256,8},std::pair{259,0},std::pair{255,11},std::pair{253,17},std::pair{254,0},std::pair{258,0}}) {
  const auto goal=destination(map,entity,baked::ObjType::TreasureBox,TargetKind::Interact);
  for(int layout:carried_layouts){
   const auto path=route(layout,carried,goal,270,true).path;
   check(path.success&&path.next_map_id==269,"The logged post-carry pose takes the lower exit towards earlier chests");
  }
  for(const auto& [from,at] : {std::pair{269,Cell{1,5,5}},std::pair{268,Cell{1,6,10}},std::pair{261,Cell{1,16,26}}}) {
   const auto path=route(0,at,goal,from,true).path;
   check(path.success,"The chest remains reachable from each floor on the real return route");
  }
 }
 const auto elevator_return=destination(266,14,baked::ObjType::GotoMap,TargetKind::Transition);
 const auto crossing=route(0,{1,6,10},elevator_return,268,true);
 check(crossing.path.success&&crossing.path.next_map_id==268&&crossing.pending.incoming_transition&&
       crossing.pending.incoming_transition->entity_id==15&&
       crossing.pending.incoming_transition->entry==Cell{1,9,10}&&
       crossing.pending.incoming_transition->arrival==Cell{1,9,8},
       "The return current ends at the native trigger and records the exact same-floor landing");
 const auto landed=route(0,{1,9,8},elevator_return,268,true).path;
 check(landed.success&&landed.next_map_id==266,"The actual current landing opens the onward elevator route");
 const auto* current_room=baked::submap_by_map_id(268);
 const auto& currents=baked::grids(*current_room);
 baked_route::Request walking;
 walking.submap=current_room;
 walking.ignore_hazards=true;
 check(!baked_route::edge_allowed(walking,currents,{1,9,11},{1,9,10})&&
       baked::current_pushback(currents,1,9,9),"Ordinary walking cannot cross either water contact");
 walking.goals={{1,9,10}};
 walking.goal_entry_mask=4;
 check(baked_route::edge_allowed(walking,currents,{1,9,11},{1,9,10})&&
       !baked_route::edge_allowed(walking,currents,{1,9,9},{1,9,10}),
       "A deliberate current ride can enter only from the south");
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
 verify_ship_returns();
 std::printf("Ship production:%d assertions/%d physical legs/%d cell observations/%d failures\n",assertions,legs,cells,errors);return !errors;
}
}
int main(){using namespace ship_routes;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)||!json::parse(read("../tools/puzzle_audit_20261030/broader_audit_20261104/ff5_FireShip_Context/physical_facts.json"),facts))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;room=baked::submap_by_map_id(270);consumer=verify;int result=2;std::thread worker([&]{result=compile();});worker.join();return result;}
