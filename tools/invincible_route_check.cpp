// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real FFIII bakes and actual owned planner/filter consumers; no game execution.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include "../src/core/fieldnav/autowalk_route.h"
int main(){
 using namespace field;using namespace baked_nav;
 int failures=0;auto expect=[&](bool yes,const char* why){if(!yes){++failures;std::printf("FAIL %s\n",why);}};
 std::ifstream file("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(file),{}};
 if(!baked::load(bytes.data(),bytes.size()))return 2;
 std::ifstream metadata("data/ff3/mapstate.json",std::ios::binary);std::string text{std::istreambuf_iterator<char>(metadata),{}};
 if(!json::parse(text,fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 const auto* world=baked::submap_by_map_id(4);const auto& grid=baked::grids(*world);
 auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
 flags->scenario[73]=flags->scenario[905]=1;
 for(int id:{8,9,10,11}){
  const auto* row=baked::transport(id);
  auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
  snapshot->map_id=4;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";
  snapshot->vehicle={true,id,row->type,L""};snapshot->position=world_of_cell(*world,{1,83,62});
  snapshot->kind=TargetKind::Transition;snapshot->goal.map_id=4;snapshot->goal.cell={1,36,101};
  baked_route::Route path;baked_route::Reach reach;Planned plan;
  const auto began=std::chrono::steady_clock::now();
  std::thread work([&]{SnapshotScope scope(*snapshot);auto request=walk_request(*world,{1,83,62});
   request.transportation_id=id;request.vehicle_group=row->group;request.vehicle_type=row->type;request.landing_group=row->landing_group;
   request.goals={{1,36,100}};request.goals_open=false;request.live_hazards=request.live_traps=true;
   path=baked_route::find(request);reach=baked_route::flood(request);plan=plan_snapshot(*snapshot);
  });work.join();
  const bool invincible=id==10||id==11;
  expect(path.found==invincible,"actual valley route requires the Invincible mountain action, not Nautilus flight");
  expect(reach.reachable({1,36,100})==invincible,"actual reachability flood agrees with the Invincible valley route");
  if(invincible){
   expect(path.steps()==87&&reach.distance({1,36,100})==87,"fewest-action valley route retains its real87-tile displacement instead of the old85-tile shortcut");
   expect(plan.path.success,"full Invincible family route reaches the Falgabard entrance");
   expect(std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){return phase.manual_after&&!phase.action_after.empty();}),"full flight directions expose manual mountain crossing boundaries");
   expect(std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){return phase.vehicle_before==route_progress::VehicleAction::ConfirmLand;}),"Falgabard route lands before the ordinary entrance");
   expect(!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back()==Cell{1,36,101},"full route ends at the exact Falgabard entrance");
   expect(plan.path.step_count>=85&&plan.path.step_count<120,"published route distance includes manual mountain displacement");
   if(id==10){
    auto jump_at=std::find_if(path.cells.begin()+1,path.cells.end(),[&](const Cell& cell){
     const size_t i=size_t(&cell-path.cells.data());return baked_route::step_direction(path.cells[i-1],cell,world->width,world->height)<0;
    });
    expect(jump_at!=path.cells.end(),"real shortest route contains a native mountain edge");
    if(jump_at!=path.cells.end()){
     const auto from=*(jump_at-1),to=*jump_at;const int dir=baked_route::direction_towards(from,to,world->width,world->height);
     constexpr int delta[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
     // Actual owner capture normalizes world obstacle layers to wildcard -1.
     const Cell middle{-1,(from.x+delta[dir][0]+world->width)%world->width,(from.y+delta[dir][1]+world->height)%world->height};
     for(int kind=0;kind<6;++kind){
      baked_route::Request guarded;guarded.submap=world;guarded.start=from;guarded.goals={to};guarded.goals_open=false;
      guarded.vehicle_group=row->group;guarded.vehicle_type=row->type;guarded.transportation_id=id;guarded.live_hazards=guarded.live_traps=true;
      guarded.transitions=std::make_shared<baked::TransitionGrid>();guarded.max_expansions=256;
      if(kind==0)guarded.blocked={middle};
      if(kind==1||kind==2){guarded.hazards={middle};guarded.ignore_hazards=kind==2;}
      if(kind==3)guarded.traps={middle};
      if(kind==4){auto rules=std::make_shared<baked_route::TransportRules>();baked_route::add_transport_cell(*rules,baked_route::TransportRules::Terminal,*world,middle,baked_route::transportation_bit(id));guarded.transport_rules=rules;}
      if(kind==5){constexpr int bits[]={1,4,8,2};guarded.directional.push_back({middle,0,15&~bits[dir]});}
      baked_route::Route alternate;std::thread constraint([&]{alternate=baked_route::find(guarded);});constraint.join();
      expect(!alternate.found||alternate.cells.size()!=2,"native mountain action cannot skip a blocker, event, trap, terminal or directional wall");
     }
    }
   }
   int actions=0;
   std::thread boundaries([&]{SnapshotScope scope(*snapshot);
    const auto index=build_beacon_corner_index(*world,plan.path.walk_phases,false,&world_of_cell);
    for(const auto& phase:plan.path.walk_phases){
     for(size_t i=1;i<phase.cells.size();++i)expect(baked_route::step_direction(phase.cells[i-1],phase.cells[i],world->width,world->height)>=0,
      "published movement phases contain only ordinary native unit edges");
     if(!phase.manual_displacement_steps)continue;
     ++actions;expect(phase.manual_after&&phase.manual_displacement_steps>=2&&phase.manual_displacement_steps<=3,
      "each skipped ridge has one bounded native manual action");
     const auto from=phase.cells.back();std::vector<Phase> remaining;
     expect(remaining_from(*plan.active,from,remaining)&&!remaining.empty(),"retained route finds the current mountain action boundary");
     Corner reference;corner_of(*world,remaining,from,reference);const auto* cached=index.find(from);
     expect(reference.found&&!reference.arrived&&reference.steps_to_corner==0&&cached&&cached->steps_to_corner==0&&cached->steps_left==reference.steps_left,
      "beacon stops at the manual mountain action and retains complete displacement distance");
     ffpr::fieldnav::autowalk::Route follower;follower.set(remaining,world->width,world->height,true);
     const auto next=follower.next(from);
     expect(next.result==ffpr::fieldnav::autowalk::Result::Action&&next.vehicle_action==route_progress::VehicleAction::None&&
      next.face_direction==phase.arrival_direction,"autowalk stops with native-facing guidance instead of driving across mountain cells");
    }
   });boundaries.join();expect(actions>0,"real full route exposes at least one mountain action");
  }
  std::printf("Invincible geometry id%d: route%d distance%d/flood%d, full route%d, %.3fms owned compute\n",id,path.found,path.steps(),reach.distance({1,36,100}),plan.path.success,
   std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
 }
 // Actual surface ownership uses Invincible10;11 is its floating-continent
 // identity. Above,11 deliberately checks the same geometric family on this
 // fixed surface witness rather than claiming a native11 ownership state here.
 for(bool owned:{false,true}){
  auto capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*flags,fixture::records));auto& snapshot=capture->route;
  snapshot.map_id=4;snapshot.submap=world;snapshot.layer=1;snapshot.locale=L"en";snapshot.auto_groups={3};snapshot.vehicle_shortcuts=true;
  snapshot.vehicle={true,1,1,L""};snapshot.position=world_of_cell(*world,{1,83,62});
  snapshot.goal.map_id=4;snapshot.goal.cell={1,36,101};snapshot.goal.entity_id=888;snapshot.goal.raw_name=L"ファルガバード入口";snapshot.kind=TargetKind::Transition;
  if(owned){VehicleOnMap vehicle;vehicle.transportation_id=10;vehicle.type=8;vehicle.pos=world_of_cell(*world,{1,83,63});snapshot.vehicles={vehicle};snapshot.vehicle_names[10]=L"Invincible";}
  Planned plan;std::vector<int> distances;filter_worker::Categories reachable,literal;
  std::thread pipeline([&]{plan=plan_snapshot(snapshot);
   ExitTarget target{world_of_cell(*world,snapshot.goal.cell),0,144,snapshot.goal.cell};target.kind=TargetKind::Transition;
   distances=exit_distances(*capture,{target},{});
   filter_worker::FilterInput input;input.current_map=4;input.world=true;input.navigation=capture;
   filter_worker::FilterRow marker;marker.kind=Kind::MapExit;marker.category=Category::MapExits;marker.name=L"Exit to Falgabard";
   marker.entity_id=888;marker.raw_name=snapshot.goal.raw_name;marker.position=target.position;marker.numbering_position=marker.position;marker.destination=144;
   input.rows={marker};reachable=filter_worker::build_categories(input);input.reachable_only=false;literal=filter_worker::build_categories(input);
  });pipeline.join();
  expect(plan.path.success==owned&&!distances.empty()&&(distances[0]>=0)==owned,"parked unlocked Invincible contributes the full Falgabard route before boarding, and missing ownership stays unreachable");
  // Category views contain source row indices; only All owns the name vector.
  auto contains_falgabard=[](const auto& view){return std::find(view.order.begin(),view.order.end(),size_t(0))!=view.order.end();};
  expect(contains_falgabard(reachable.views[static_cast<int>(Category::MapExits)])==owned&&contains_falgabard(literal.views[static_cast<int>(Category::MapExits)])&&
   !contains_falgabard(literal.views[static_cast<int>(Category::Events)]),
   "actual category pipeline publishes Falgabard under Map Exits for owned reachability and literal discovery");
  if(owned)expect(std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){return !phase.action_before.empty()&&phase.transportation_id==10&&phase.action_cell;}),
   "before-boarding directions retain the actual Invincible boarding approach");
 }
 // Synthetic copied-grid semantics use the actual game's group masks; they
 // never modify a bake or any game object. This tests wrap and native thickness.
 {
  auto copy=grid;const auto* row=baked::transport(10);baked_route::Request request;
  request.submap=world;request.transportation_id=10;request.vehicle_group=row->group;request.vehicle_type=row->type;
  const Cell start{1,255,100};const auto at=[&](int x){return size_t(100)*copy.width+(x+copy.width)%copy.width;};
  for(int x:{255,0,1,2,3}){copy.attribute[at(x)]=4;copy.transport[row->group-1][at(x)]=0;}
  copy.attribute[at(0)]=19;
  auto one=baked_route::mountain_edge(request,copy,start,1);
  expect(one&&one->to==Cell{1,1,100}&&one->distance==2,"one mountain crosses2 physical tiles across the wrapped world boundary");
  copy.attribute[at(1)]=19;auto two=baked_route::mountain_edge(request,copy,start,1);
  expect(two&&two->to==Cell{1,2,100}&&two->distance==3,"two mountains cross3 physical tiles");
  copy.attribute[at(2)]=19;expect(!baked_route::mountain_edge(request,copy,start,1),"three-thick mountain ridges stay closed");
  copy.attribute[at(1)]=4;copy.attribute[at(2)]=4;copy.transport[row->group-1][at(1)]=1;
  expect(!baked_route::mountain_edge(request,copy,start,1),"closed first far side cannot be skipped to a later legal tile");
  copy.transport[row->group-1][at(1)]=0;copy.attribute[at(1)]=0;
  expect(!baked_route::mountain_edge(request,copy,start,1),"unknown/forbidden first far-side terrain remains closed");
  request.transportation_id=9;expect(!baked_route::mountain_edge(request,copy,start,1),"same-group Nautilus cannot acquire Invincible's action");
 }
 // The second real model belongs to floating-continent world1. Exercise its
 // actual world/identity rather than only the shared surface geometry above.
 {
  const auto* floating=baked::submap_by_map_id(1);const auto& terrain=baked::grids(*floating);const auto* row=baked::transport(11);
  baked_route::Request movement;movement.submap=floating;movement.vehicle_group=row->group;movement.vehicle_type=row->type;movement.transportation_id=11;
  std::optional<baked_route::MountainEdge> edge;Cell origin;
  for(int y=0;y<terrain.height&&!edge;++y)for(int x=0;x<terrain.width&&!edge;++x){
   origin={1,x,y};if(!baked_route::open(movement,terrain,origin))continue;
   for(int dir=0;dir<4&&!edge;++dir)edge=baked_route::mountain_edge(movement,terrain,origin,dir);
  }
  expect(edge.has_value(),"floating continent contains a real native Invincible11 mountain crossing");
  if(edge){
   auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));snapshot->map_id=1;snapshot->submap=floating;
   snapshot->layer=1;snapshot->locale=L"en";snapshot->vehicle={true,11,row->type,L""};snapshot->position=world_of_cell(*floating,origin);
   snapshot->kind=TargetKind::Spot;snapshot->goal.map_id=1;snapshot->goal.cell=edge->to;
   // Isolate this actual native edge from optional ordinary detours. The
   // separate preference check now requires an available ordinary detour to win.
   constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
   for(int direction=0;direction<4;++direction)if(direction!=edge->direction){
    NativeObstacle wall;wall.type=19;wall.id=-100-direction;wall.collider=true;wall.collision=1;
    wall.cell={-1,(origin.x+sides[direction][0]+terrain.width)%terrain.width,(origin.y+sides[direction][1]+terrain.height)%terrain.height};
    snapshot->obstacles.push_back(wall);
   }
   Planned plan;std::thread real_model([&]{plan=plan_snapshot(*snapshot);});real_model.join();
   expect(plan.path.success&&plan.path.step_count==edge->distance&&std::any_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& phase){return phase.manual_displacement_steps>0;}),
    "actual floating-continent model11 publishes native manual crossing and exact distance");
  }
 }
 {
  const auto* row=baked::transport(10);baked_route::Request movement;movement.submap=world;movement.start={1,83,62};movement.goals={{1,36,100}};
  movement.vehicle_group=row->group;movement.vehicle_type=row->type;movement.transportation_id=10;movement.live_hazards=movement.live_traps=true;
  // An observed empty terminal plane, rather than the conservative all-story
  // fallback, lets this cancellation-only fixture actually start searching.
  movement.transitions=std::make_shared<baked::TransitionGrid>();
  int tokens=0;movement.cancelled=[&]{return ++tokens>50;};expect(!baked_route::find(movement).found&&tokens>50&&tokens<100,"mountain A* cancels without completing obsolete work");
  tokens=0;expect(!baked_route::flood(movement).submap&&tokens>50&&tokens<100,"weighted mountain flood cancels without publishing partial reachability");
 }
 for(int n:{1,2,4,5,6}){
  std::ifstream foreign("data/ff"+std::to_string(n)+"/maps.bin",std::ios::binary);std::string data{std::istreambuf_iterator<char>(foreign),{}};
  expect(baked::load(data.data(),data.size()),"other game's real bake loads for capability abstention");
  const baked::Submap* other=nullptr;for(int i=0;i<baked::submap_count()&&!other;++i)if(baked::submap_at(i)->world)other=baked::submap_at(i);
  baked_route::Request movement;movement.submap=other;movement.vehicle_group=6;movement.vehicle_type=8;movement.transportation_id=10;
  expect(!baked_route::mountain_vehicle(movement),"other five games cannot inherit the FFIII Invincible capability");
 }
 expect(fixture::wrong_owner==0,"Invincible route work never reads native objects");
 std::printf("Invincible crossing: %d failures\n",failures);return failures?1:0;
}
