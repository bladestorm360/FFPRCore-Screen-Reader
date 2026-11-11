// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real native-shaped FFIV stage resumes through the production route worker.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "ordinary_interaction_fixtures.h"
namespace {
int failures=0,episodes=0,legs=0;json::Value guides;
void observe(field::baked_nav::RouteSnapshot& snapshot,int live_contact=-1) {
 using namespace field;using namespace baked_nav;
 for(const auto& original:baked::grids(*snapshot.submap).objects) {
  if(original.type==baked::ObjType::PointIn||original.type==baked::ObjType::GotoMap||
     original.type==baked::ObjType::Warp||
     (original.entity_id()!=live_contact&&!snapshot.state.active(*snapshot.submap,original)))continue;
  const auto actor=snapshot.state.effective(*snapshot.submap,original);NativeObstacle seen;seen.id=actor.entity_id();seen.type=int(actor.type);seen.raw_name=baked::object_identifier(original);
  seen.cell={actor.layer,actor.tx,actor.ty};seen.enabled=seen.applies=true;seen.trigger=baked_route::step_event(actor);seen.chest=actor.type==baked::ObjType::TreasureBox;
  seen.npc=actor.type==baked::ObjType::NPC||actor.type==baked::ObjType::ShopNPC;seen.collider=actor.type==baked::ObjType::CollisionEntity;seen.collision=actor.a;
  seen.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);seen.transportation_restricted=seen.transportation_ids!=~uint64_t(0);snapshot.obstacles.push_back(std::move(seen));
 }
}
void episode(const char* label,const wchar_t* id,int map,std::initializer_list<int> completed,int stop_receipt=-1,std::optional<field::baked_route::Cell> pose={},bool reloaded=false) {
 using namespace field;using namespace baked_nav;
 map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);
 for(int bit:completed)flags.scenario[bit]=1;
 const auto& definitions=guides.get(L"puzzles")->arr;const auto p=std::find_if(definitions.begin(),definitions.end(),[&](const auto& p){return p.get(L"id")->s==id;});
 if(p==definitions.end()){++failures;return;}
 const auto* room=baked::submap_by_map_id(map);map_state::Resolver state(flags,fixture::records);const auto* entry=point_in(*room,1,state);
 if(!entry){std::printf("FAIL %s no native entrance\n",label);++failures;return;}
 Cell at=pose.value_or(Cell{entry->layer,entry->tx,entry->ty});bool complete=true;
 const bool treasury=p->get(L"id")->s==L"troia_castle_treasury";
 const int end=p->get(L"done_scenario")?int(p->get(L"done_scenario")->n):-1;
 for(const auto& step:p->get(L"steps")->arr) {
  if(end>=0&&flags.scenario[end]==1)break;
  const int bit=int(step.get(L"done_scenario")->n);if(flags.scenario[bit]==1)continue;
  MapGoal goal;goal.map_id=int(step.get(L"map")->n);goal.entity_id=int(step.get(L"entity")->n);goal.raw_name=step.get(L"raw")->s;
  goal.cell={int(step.get(L"layer")->n),int(step.get(L"x")->n),int(step.get(L"y")->n)};goal.examine=step.get(L"examine")->b;
  goal.allow_world_transit=step.get(L"allow_world_transit")&&step.get(L"allow_world_transit")->b;Pending pending;bool reached=false;
  for(int hop=0;hop<64;++hop) {
   flags.current_map_id=map;RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);
   snapshot.layer=at.layer;snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";snapshot.goal=goal;snapshot.pending=pending;
   snapshot.kind=goal.examine?TargetKind::Interact:TargetKind::Spot;
   // The native event callback leaves the next contact visible in the live
   // room, even when the initializer would hide it on load with the same flags.
   observe(snapshot,treasury&&!reloaded?goal.entity_id:-1);
   Cell expected=goal.cell;
   if(treasury) {
    snapshot.goal.avoid_other_events=true;
    if(bit!=274) {
     const auto& first=p->get(L"steps")->arr.front();
     MapGoal entrance=snapshot.goal;
     entrance.cell={int(first.get(L"layer")->n),int(first.get(L"x")->n),int(first.get(L"y")->n)};
     entrance.entity_id=int(first.get(L"entity")->n);entrance.raw_name=first.get(L"raw")->s;
     snapshot.candidates={{snapshot.goal,TargetKind::Spot,{}},{entrance,TargetKind::Spot,{}}};
     if(reloaded)expected=entrance.cell;
    }
   }
   Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();++legs;
   if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach) {
    std::printf("FAIL %s bit%d from%d L%d %d,%d to%d/%d NPC%d: %ls / %ls\n",label,bit,map,at.layer,at.x,at.y,goal.map_id,goal.entity_id,route.path.npc_approach,route.path.reason.c_str(),route.path.error.c_str());break;
   }
   if(map==goal.map_id){
    at=route.path.walk_phases.back().cells.back();
    reached=goal.examine||at==expected;
    if(treasury)std::printf("Treasury bit%d: contact%d, %d steps, reload%d\n",bit,route.goal.entity_id,route.path.step_count,reloaded);
    if(!reached)std::printf("FAIL %s bit%d stopped short of the contact at %d,%d\n",label,bit,at.x,at.y);
    break;
   }
   const auto arrival=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;
   if(!arrival){std::printf("FAIL %s missing arrival\n",label);break;}
   pending=route.pending;{SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(arrival->map_id,arrival->entry);record_map_arrival(arrival->destination,arrival->arrival);pending=snapshot.pending;}
   map=arrival->destination;at=arrival->arrival;flags.local.fill(0);
  }
  if(!reached){complete=false;break;}
  flags.scenario[bit]=1;
  reloaded=false;
  // Source-authored proxy/leader positions, never the boss's occupied12,13
  // actor cell. Gameplay reads its actual native player on each new request.
  if(bit==97)at={0,12,15};
  if(bit==36)at={0,12,14};
  if(bit==44)at={1,15,11}; //0045_2 SetPos15,11, native layer2 -> Ground1
  if(bit==stop_receipt)break;
 }
 std::printf("FFIV %s: complete%d final%d\n",label,complete,map);if(complete)++episodes;else ++failures;
}
void key_door(int map,int entity,int destination,int receipt,unsigned key, std::initializer_list<int> progress) {
 using namespace field;using namespace baked_nav;
 auto check=[&](bool ok,const char* what) {
  if(!ok){++failures;std::printf("FAIL key door %d/%d: %s\n",map,entity,what);}
 };
 map_state::Flags flags;flags.scenario.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.treasure.fill(0);
 flags.current_map_id=map;
 for(int flag:progress)flags.scenario[flag]=1;
 const auto& room=*baked::submap_by_map_id(map);
 const auto& grid=baked::grids(room);
 const auto door=std::find_if(grid.objects.begin(),grid.objects.end(),[entity](const auto& object){return object.type==baked::ObjType::Entity&&object.entity_id()==entity;});
 const auto exit=std::find_if(grid.objects.begin(),grid.objects.end(),[destination](const auto& object){return object.type==baked::ObjType::GotoMap&&object.a==destination;});
 if(door==grid.objects.end()||exit==grid.objects.end()){check(false,"real door and exit must exist");return;}
 const Cell door_cell{door->layer,door->tx,door->ty};
 map_state::Resolver initial(flags,fixture::records);
 const auto* point=point_in(room,1,initial);
 if(!point){check(false,"native floor entrance must exist");return;}
 const Cell at{point->layer,point->tx,point->ty};
 auto run=[&](MapGoal goal, TargetKind kind) {
  RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=&room;
  snapshot.layer=at.layer;snapshot.position=world_of_cell(room,at);snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";
  snapshot.goal=std::move(goal);snapshot.kind=kind;observe(snapshot);
  Planned result;std::thread worker([&]{result=plan_snapshot(snapshot);});worker.join();
  return result;
 };
 MapGoal control;control.map_id=map;control.cell=door_cell;control.entity_id=entity;control.raw_name=baked::object_identifier(*door);
 check(run(control,TargetKind::Interact).path.success,"the door itself remains approachable without the key");
 MapGoal through;through.map_id=map;through.cell={exit->layer,exit->tx,exit->ty};
 check(!run(through,TargetKind::Transition).path.success,"the onward exit is closed without known inventory");
 flags.ff4_door_keys=7u^key;
 check(!run(through,TargetKind::Transition).path.success,"other key items cannot qualify this door");
 flags.ff4_door_keys=key;
 const auto prefix=run(through,TargetKind::Transition);
 check(prefix.path.success&&prefix.active&&prefix.active->key_approach&&!prefix.path.walk_phases.empty()&&
       prefix.path.walk_phases.back().manual_after&&prefix.path.walk_phases.back().cells.back()!=door_cell,
       "owning the right item routes to a safe manual unlock before the closed exit");
 if(map==24) {
  MapGoal remote;remote.map_id=destination;
  const auto* dest=baked::submap_by_map_id(destination);
  const auto* inside=point_in(*dest,1,initial);
  remote.cell={inside->layer,inside->tx,inside->ty};
  const auto onward=run(remote,TargetKind::Spot);
  check(onward.path.success&&onward.active&&onward.active->key_approach&&onward.pending.goal.map_id==destination,
        "a destination beyond a Baron door keeps the final goal and stops for the key interaction");
 }
 flags.scenario[receipt]=1;
 const auto opened=run(through,TargetKind::Transition);
 check(opened.path.success&&opened.active&&!opened.active->key_approach,
       "the native opened receipt restores ordinary travel without an unlock instruction");
}
}
void troia_passages() {
 using namespace field;using namespace baked_nav;
 struct Case {int map;Cell start;int destination;Cell target;TargetKind kind;int receipt;};
 const Case cases[]={
  {94,{0,3,3},94,{0,3,13},TargetKind::Transition,283},
  {94,{0,3,10},94,{0,3,3},TargetKind::Transition,283},
  {95,{0,3,6},95,{0,7,7},TargetKind::Spot,280},
  {93,{0,29,5},93,{0,4,3},TargetKind::Transition,-1},
  {91,{0,7,9},91,{0,32,9},TargetKind::Transition,283},
 };
 for(const auto& example:cases) {
  map_state::Flags flags;flags.scenario.fill(0);flags.current_map_id=example.map;flags.current_area_id=12;
  RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));
  snapshot.map_id=example.map;snapshot.submap=baked::submap_by_map_id(example.map);
  snapshot.layer=0;snapshot.position=world_of_cell(*snapshot.submap,example.start);snapshot.locale=L"en";
  snapshot.kind=example.kind;snapshot.goal.map_id=example.destination;snapshot.goal.cell=example.target;
  observe(snapshot);
  Planned blocked,opened;
  std::thread worker([&]{blocked=plan_snapshot(snapshot);});worker.join();
  const bool prefix=blocked.path.success&&blocked.path.event_on_route&&blocked.path.reason==L"Event required"&&
   !blocked.path.walk_phases.empty()&&blocked.path.walk_phases.back().manual_after&&blocked.pending.valid&&
   blocked.pending.goal.map_id==example.destination;
  std::printf("Troia map%d to%d from%d,%d prefix%d steps%d reason[%ls] action[%ls]\n",example.map,
   example.destination,example.start.x,example.start.y,prefix,blocked.path.step_count,blocked.path.reason.c_str(),
   blocked.path.walk_phases.empty()?L"":blocked.path.walk_phases.back().action_after.c_str());
  if(!prefix)++failures;
  if(example.map==91) {
   auto crowded=snapshot;crowded.position=world_of_cell(*crowded.submap,{0,7,6});
   Planned wait;std::thread waiting([&]{wait=plan_snapshot(crowded);});waiting.join();
   if(!wait.path.success||!wait.path.npc_approach||!wait.pending.valid)++failures;
  }
  if(example.receipt>=0)flags.scenario[example.receipt]=1;
  RouteSnapshot after(map_state::Resolver(flags,fixture::records));
  after.map_id=example.map;after.submap=snapshot.submap;after.layer=0;after.locale=L"en";
  after.position=snapshot.position;after.goal=snapshot.goal;after.kind=snapshot.kind;
  observe(after);
  if(example.map==93)std::erase_if(after.obstacles,[](const auto& actor){return actor.id==22||actor.id==20;});
  std::thread resumed([&]{opened=plan_snapshot(after);});resumed.join();
  if(!opened.path.success||opened.path.reason==L"Story gated")++failures;
 }
 std::puts("Troia ordinary passage prefixes and native opened states replayed.");
}

int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff4/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff4/mapstate.json"),fixture::records)||
    !json::parse(read("data/ff4/puzzles.json"),guides)||!append_ordinary_interactions(guides,4))return 2;
 fixture::game_id=game::Id::FF4;g_loaded=g_prepared=true;
 troia_passages();
 episode("Troia treasury fresh entry",L"troia_castle_treasury",102,{});
 episode("Troia treasury after entrance switch",L"troia_castle_treasury",102,{274},-1,Cell{0,7,10});
 episode("Troia treasury after first door switch",L"troia_castle_treasury",102,{274,275},-1,Cell{0,2,6});
 episode("Troia treasury after second door switch",L"troia_castle_treasury",102,{274,275,276},-1,Cell{0,5,6});
 episode("Troia treasury reload after entrance",L"troia_castle_treasury",102,{274},-1,{},true);
 episode("Troia treasury reload after first door",L"troia_castle_treasury",102,{274,275},-1,{},true);
 episode("Troia treasury reload on the entrance contact",L"troia_castle_treasury",102,{274,275,276},-1,Cell{0,7,10},true);
 episode("Baron locked",L"baron_key_doors",24,{30,31});
 episode("Baron shop open",L"baron_key_doors",24,{30,31,271});
 episode("Baron waterway first",L"baron_key_doors",24,{30,31,272});
 episode("Baron both open",L"baron_key_doors",24,{30,31,271,272});
 episode("Baron later",L"baron_key_doors",24,{33});
 episode("Twin Harp ally",L"magnetic_cave",100,{},34);
 episode("Magnetic entrance",L"magnetic_cave",192,{34});
 episode("Magnetic middle",L"magnetic_cave",196,{34,100});
 episode("Magnetic first encounter",L"magnetic_cave",201,{34,100});
 episode("Magnetic harp restored",L"magnetic_cave",201,{34,97,100});
 episode("Magnetic harp native proxy pose",L"magnetic_cave",201,{34,97,100},-1,Cell{0,12,15});
 episode("Magnetic guardian defeated",L"magnetic_cave",201,{34,36,97,100});
 episode("Magnetic victory native leader pose",L"magnetic_cave",201,{34,36,97,100},-1,Cell{0,12,14});
 episode("Magnetic crystal recovered",L"magnetic_cave",201,{98});
 episode("Magma well",L"agart_magma_well",109,{43});
 episode("Magma complete",L"agart_magma_well",109,{44});
 episode("Babel key",L"babel_key_door",220,{47});
 episode("Babel open",L"babel_key_door",220,{47,273});
 episode("Babel retired",L"babel_key_door",220,{48});
 episode("Sealed key",L"sealed_cavern_door",268,{56,57});
 episode("Dwarf passage closed",L"dwarven_castle_passage",125,{});
 episode("Dwarf passage open",L"dwarven_castle_passage",125,{278});
 episode("Sealed open",L"sealed_cavern_door",268,{56,57,58});
 key_door(24,66,31,271,1u,{30,31});
 key_door(24,67,318,272,1u,{30,31});
 key_door(220,28,221,273,4u,{47});
 key_door(268,6,269,58,2u,{56,57});
 std::printf("FFIV puzzle journeys: %d episodes/%d production legs, failures%d owner%d\n",episodes,legs,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
