// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real baked Amur fixture and independent tile/portal shortest-path oracle.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <queue>
#include <unordered_map>
using namespace field;using namespace field::baked_nav;using namespace field::baked_route;
namespace {
struct Floor { const baked::Submap* sm;Request req;int offset=0; };
struct Link {int to,cost;};
struct Oracle {
 std::vector<Floor> floors;std::unordered_map<const baked::Submap*,int> lookup;
 std::vector<std::vector<Link>> reverse;std::vector<int> distance;size_t links=0;
 int node(int f,const Cell& c)const {const auto& s=*floors[f].sm;return floors[f].offset+(c.layer*s.height+c.y)*s.width+c.x;}
 bool valid(int f,const Cell& c)const {const auto& g=baked::grids(*floors[f].sm);return c.layer>=0&&c.layer<3&&baked::in_bounds(g,c.x,c.y);}
 bool blocked(const Request& req,const Cell& c)const {for(const auto& b:req.blocked)if(b.x==c.x&&b.y==c.y&&(b.layer<0||b.layer==c.layer))return true;return false;}
 void edge(int from,int to,int cost){reverse[to].push_back({from,cost});++links;}
 bool build(RouteSnapshot& snapshot,const baked::Submap& destination,const std::vector<Cell>& goals) {
  SnapshotScope scope(snapshot);auto state=planning_state();auto add=[&](const baked::Submap* s){if(!s||s->world||lookup.count(s))return;lookup[s]=static_cast<int>(floors.size());floors.push_back({s,{}});};
  add(snapshot.submap);
  for(size_t i=0;i<floors.size();++i) {
   if(floors.size()>80){std::printf("oracle component exceeds 80 floors\n");return false;}
   const auto* sm=floors[i].sm;
   for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::GotoMap&&o.a>0&&state.active(*sm,o)&&!state.consumed(*sm,o)) {
    const auto* dest=baked::submap_by_map_id(o.a);if(dest&&!dest->world&&point_in(*dest,o.b,state))add(dest);
   }
  }
  if(!lookup.count(&destination))return false;
  int nodes=0;const auto live=live_blockers(*snapshot.submap,baked::grids(*snapshot.submap),{});
  for(auto& floor:floors) {
   const auto& g=baked::grids(*floor.sm);floor.offset=nodes;nodes+=3*g.width*g.height;
   floor.req=walk_request(*floor.sm,{1,0,0});floor.req.ignore_hazards=true;
   if(floor.sm==snapshot.submap){floor.req.blocked=live.hard();floor.req.directional=live.directional;floor.req.live_hazards=true;floor.req.hazards=live.triggers;floor.req.live_traps=true;floor.req.traps=live.traps;}
   else map_state::add_obstacles(state,*floor.sm,floor.req,true);
  }
  if(nodes>1000000){std::printf("oracle component exceeds one million tile-layer states\n");return false;}
  reverse.resize(nodes);static const int dx[]={0,1,0,-1,1,1,-1,-1},dy[]={-1,0,1,0,-1,1,1,-1};
  for(int fi=0;fi<static_cast<int>(floors.size());++fi) {
   const auto& floor=floors[fi];const auto& g=baked::grids(*floor.sm);
   for(int l=0;l<3;++l)for(int y=0;y<g.height;++y)for(int x=0;x<g.width;++x) {
    Cell from{l,x,y};if(blocked(floor.req,from))continue;
    for(int d=0;d<8;++d){const int nx=x+dx[d],ny=y+dy[d];if(!baked::in_bounds(g,nx,ny))continue;
     Cell next{layer_after(g,l,nx,ny),nx,ny};if(!valid(fi,next)||!edge_allowed(floor.req,g,from,next)||blocked(floor.req,next)||trap_cell(floor.req,g,next))continue;
     if(!open(floor.req,g,next)&&!(next.layer!=l&&open(floor.req,g,{l,nx,ny})))continue;
     edge(node(fi,from),node(fi,next),1);
    }
   }
   for(const auto& o:g.objects) {
    if(o.type!=baked::ObjType::GotoMap||o.a<=0||!state.active(*floor.sm,o)||state.consumed(*floor.sm,o))continue;
    const auto* dest=baked::submap_by_map_id(o.a);if(!dest||!lookup.count(dest))continue;
    const auto* arrival=point_in(*dest,o.b,state);if(!arrival)continue;
    const int di=lookup[dest];Cell to{arrival->layer,arrival->tx,arrival->ty};if(!valid(di,to))continue;
    for(int y=o.ty;y<o.ty+o.th;++y)for(int x=o.tx;x<o.tx+o.tw;++x)for(int l=0;l<3;++l) {
     Cell door{l,x,y};if(l!=o.layer||!valid(fi,door)||blocked(floor.req,door))continue;
     edge(node(fi,door),node(di,to),1);
     for(int d=0;d<8;++d){Cell from{l,x-dx[d],y-dy[d]};if(!valid(fi,from)||blocked(floor.req,from)||!open(floor.req,g,from))continue;
      const int target_layer=layer_after(g,l,x,y);Cell entered{target_layer,x,y};if(!(entered==door)||trap_cell(floor.req,g,door))continue;
      auto goal_req=floor.req;goal_req.goals={door};goal_req.goals_open=true;
      if(edge_allowed(goal_req,g,from,door))edge(node(fi,from),node(di,to),2);
     }
    }
   }
  }
  const int target=lookup[&destination];distance.assign(nodes,INT_MAX);
  using Item=std::pair<int,int>;std::priority_queue<Item,std::vector<Item>,std::greater<Item>> queue;
  for(const auto& c:goals)if(valid(target,c)){const int n=node(target,c);distance[n]=0;queue.push({0,n});}
  while(!queue.empty()){auto [cost,n]=queue.top();queue.pop();if(cost!=distance[n])continue;for(const auto& e:reverse[n])if(cost+e.cost<distance[e.to]){distance[e.to]=cost+e.cost;queue.push({distance[e.to],e.to});}}
  return true;
 }
 int at(const baked::Submap* map,const Cell& c)const {auto it=lookup.find(map);if(it==lookup.end()||!valid(it->second,c))return -1;const int n=distance[node(it->second,c)];return n==INT_MAX?-1:n;}
};
int verify_retention(RouteSnapshot& base) {
 const auto* town=base.submap;const auto& grid=baked::grids(*town);int failures=0;
 auto expect=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
 auto actor=std::find_if(base.obstacles.begin(),base.obstacles.end(),[](const NativeObstacle& a){return a.id==39;});
 if(actor==base.obstacles.end())return 1;const size_t index=static_cast<size_t>(actor-base.obstacles.begin());
 const Cell p{2,16,19},p1{2,16,20},oldnext{2,17,20};auto initial=base;
 initial.obstacles[index].cell={2,14,20};initial.live_transitions.reset();initial.position=world_of_cell(*town,p);
 initial.previous_active.reset();initial.pending={};auto first=plan_snapshot(initial);
 expect(first.path.success&&!first.path.walk_phases.empty()&&first.path.walk_phases.front().cells.size()>3,"witness initial path exists");
 if(!first.path.success||first.path.walk_phases.empty()||first.path.walk_phases.front().cells.size()<4)return failures+1;
 if(!first.pending.incoming_transition){std::puts("FAIL witness lacks incoming transition receipt");return failures+1;}
 const auto old=first.path.walk_phases.front().cells;expect(old[0]==p&&old[1]==p1&&old[2]==oldnext,"real witness initial first steps");
 auto changed=initial;changed.obstacles[index].cell={2,14,21};changed.live_transitions.reset();changed.position=world_of_cell(*town,p1);
 changed.previous_active=first.active;changed.pending=first.pending;auto kept=plan_snapshot(changed);
 expect(kept.path.success&&!kept.path.walk_phases.empty()&&kept.path.walk_phases.front().cells.size()>1&&kept.path.walk_phases.front().cells[1]==oldnext,"patrol movement preserves the clear forward suffix");
 expect(kept.path.next_map_id==first.path.next_map_id&&kept.pending.incoming_transition.has_value()&&
    first.pending.incoming_transition.has_value()&&kept.pending.incoming_transition->entry==first.pending.incoming_transition->entry&&
    kept.path.walk_phases.front().action_after==first.path.walk_phases.front().action_after,"kept leg preserves native action and exact incoming receipt");
 auto third=changed;third.obstacles[index].cell={2,14,20};third.live_transitions.reset();third.position=world_of_cell(*town,oldnext);
 third.previous_active=kept.active;third.pending=kept.pending;auto forward=plan_snapshot(third);
 expect(forward.path.success&&!forward.path.walk_phases.empty()&&forward.path.walk_phases.front().cells.size()>1&&forward.path.walk_phases.front().cells[1]==old[3],"returning patrol leaves next forward step unchanged");
 bool uninterrupted=true;auto prior=forward;size_t continued_steps=0;
 for(size_t k=3;k+1<old.size();++k){auto step=base;step.obstacles[index].cell=(k%2)?Cell{2,14,21}:Cell{2,14,20};
    step.live_transitions.reset();step.position=world_of_cell(*town,old[k]);step.previous_active=prior.active;step.pending=prior.pending;
    auto next=plan_snapshot(step);if(!next.path.success||next.path.walk_phases.empty()||next.path.walk_phases.front().cells.size()<2||
       !(next.path.walk_phases.front().cells[1]==old[k+1])){uninterrupted=false;break;}
    ++continued_steps;prior=std::move(next);
 }
 expect(uninterrupted&&continued_steps==old.size()-4,"alternating patrol preserves every remaining forward step to the planned door");
 std::printf("witness continued steps=%zu of %zu, no reversal=%d\n",continued_steps,old.size()-4,uninterrupted);
 changed.pending=first.pending; // plan_snapshot moves its copied pending receipt into the returned plan
 auto attempt=[&](RouteSnapshot& candidate,const MapGoal& goal,TargetKind kind){candidate.active=*candidate.previous_active;SnapshotScope scope(candidate);PathInfo out;return keep_remote_foot_leg(*town,goal,kind,out);};
 auto candidate=changed;candidate.active={};expect(attempt(candidate,candidate.goal,TargetKind::Interact),"unchanged receipt and clear suffix are reusable");
 auto off=changed;off.position=world_of_cell(*town,{2,0,0});expect(!attempt(off,off.goal,TargetKind::Interact),"off-route player rejects suffix");
 auto raw=changed;auto other_goal=raw.goal;other_goal.raw_name+=L"different";expect(!attempt(raw,other_goal,TargetKind::Interact),"changed raw target rejects suffix");
 auto identity=changed;other_goal=identity.goal;++other_goal.entity_id;expect(!attempt(identity,other_goal,TargetKind::Interact),"changed target identity rejects suffix");
 auto kind=changed;expect(!attempt(kind,kind.goal,TargetKind::Spot),"changed goal kind rejects suffix");
 auto vehicle=changed;vehicle.vehicle={true,13,2,{}};expect(!attempt(vehicle,vehicle.goal,TargetKind::Interact),"changed vehicle rejects suffix");
 auto policy=changed;policy.vehicle_shortcuts=!policy.vehicle_shortcuts;expect(!attempt(policy,policy.goal,TargetKind::Interact),"changed vehicle policy rejects suffix");
 auto map=changed;map.map_id=98;expect(!attempt(map,map.goal,TargetKind::Interact),"changed current map rejects suffix");
 auto receipt=changed;receipt.pending.incoming_transition.reset();receipt.pending.incoming_trap.reset();expect(!attempt(receipt,receipt.goal,TargetKind::Interact),"missing first-hop receipt rejects suffix");
 auto wrong_receipt=changed;wrong_receipt.pending.incoming_transition->entry.x++;expect(!attempt(wrong_receipt,wrong_receipt.goal,TargetKind::Interact),"changed first-hop cell rejects suffix");
 auto trap=changed;trap.pending.incoming_trap=trap.pending.incoming_transition;trap.pending.incoming_transition.reset();expect(!attempt(trap,trap.goal,TargetKind::Interact),"trap receipt stays on existing planner");
 auto required_trap=changed;auto trap_active=std::make_shared<ActiveRoute>(*required_trap.previous_active);trap_active->requires_trap=true;required_trap.previous_active=trap_active;
 expect(!attempt(required_trap,required_trap.goal,TargetKind::Interact),"trap-dependent route stays on existing planner");
 auto wrong_goal=changed;wrong_goal.goal.raw_name+=L"not present";auto unavailable=plan_snapshot(wrong_goal);
 expect(!unavailable.path.success&&(!unavailable.active||!unavailable.active->valid),"unavailable remote target retires old active leg");
 map_state::Flags altered;altered.scenario.fill(0);altered.treasure.fill(0);altered.global.fill(0);altered.current_map_id=97;altered.scenario[0]=1;
 RouteSnapshot state_changed(map_state::Resolver(altered,fixture::records));state_changed.submap=town;state_changed.map_id=97;state_changed.layer=2;
 state_changed.locale=L"en";state_changed.position=changed.position;state_changed.obstacles=changed.obstacles;state_changed.goal=changed.goal;state_changed.kind=TargetKind::Interact;
 state_changed.pending=first.pending;auto marked_active=std::make_shared<ActiveRoute>(*first.active);
 marked_active->phases.front().action_after=L"old route marker";state_changed.previous_active=marked_active;auto reprojected=plan_snapshot(state_changed);
 expect(!reprojected.path.success||reprojected.path.walk_phases.empty()||reprojected.path.walk_phases.front().action_after!=L"old route marker","changed state signature forces fresh first-hop planning");
 auto warned=changed;NativeObstacle trigger;trigger.type=2;trigger.trigger=true;trigger.cell=old[3];trigger.id=90002;warned.obstacles.push_back(trigger);
 warned.active=*warned.previous_active;{SnapshotScope scope(warned);PathInfo out;const bool kept_warning=keep_remote_foot_leg(*town,warned.goal,TargetKind::Interact,out);
  expect(kept_warning&&out.event_on_route,"kept suffix recomputes warnings for current intervening events");}
 auto blocker=changed;NativeObstacle obstacle;obstacle.type=6;obstacle.id=90001;obstacle.npc=true;obstacle.solidity_known=true;obstacle.solid=true;obstacle.cell=oldnext;blocker.obstacles.push_back(obstacle);
 expect(!attempt(blocker,blocker.goal,TargetKind::Interact),"new solid actor on suffix rejects old leg");
 auto fresh=plan_snapshot(blocker);bool crossed=false;for(const auto& phase:fresh.path.walk_phases)for(const auto& c:phase.cells)crossed|=c==oldnext;
 expect(!crossed,"fresh plan or wait never crosses the new actor");
 auto gate=changed;NativeObstacle closed;closed.type=19;closed.collider=true;closed.enabled=true;closed.applies=true;closed.collision=1;closed.cell=first.pending.incoming_transition->entry;gate.obstacles.push_back(closed);
 expect(!attempt(gate,gate.goal,TargetKind::Interact),"closed first-hop gate rejects old leg");
 auto obsolete=changed;obsolete.cancelled=[]{return true;};auto cancelled=plan_snapshot(obsolete);
 expect(!cancelled.path.success&&(!cancelled.active||!cancelled.active->valid),"cancelled stale route cannot publish retained active leg");
 auto stale=changed;stale.epoch=std::make_shared<std::atomic<uint64_t>>(2);stale.generation=1;auto retired=plan_snapshot(stale);
 expect(!retired.path.success&&(!retired.active||!retired.active->valid),"stale request epoch cannot publish retained active leg");
 const auto begin=std::chrono::steady_clock::now();int accepted=0;
 for(int i=0;i<100;++i){auto repeat=changed;accepted+=attempt(repeat,repeat.goal,TargetKind::Interact);}
 const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
 std::printf("retention witness initial=(%d,%d)->(%d,%d), changed next=(%d,%d); 100 validation requests=%.3fms accepted=%d; offline only\n",p.x,p.y,p1.x,p1.y,oldnext.x,oldnext.y,ms,accepted);
 expect(accepted==100,"repeated clear validation retains every leg");
 int fresh_count=0,cancel_count=0;const auto fresh_start=std::chrono::steady_clock::now();
 for(int i=0;i<20;++i){auto request=base;request.obstacles[index].cell={2,14,20};request.live_transitions.reset();
    request.position=world_of_cell(*town,p);request.previous_active.reset();request.pending={};fresh_count+=plan_snapshot(request).path.success;}
 const auto fresh_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-fresh_start).count();
 const auto cancel_start=std::chrono::steady_clock::now();
 for(int i=0;i<100;++i){auto request=changed;request.cancelled=[]{return true;};auto stopped=plan_snapshot(request);cancel_count+=!stopped.path.success&&(!stopped.active||!stopped.active->valid);}
 const auto cancel_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-cancel_start).count();
 std::printf("20 fresh production requests=%.3fms success=%d; 100 cancelled requests=%.3fms retired=%d; offline only\n",fresh_ms,fresh_count,cancel_ms,cancel_count);
 expect(fresh_count==20&&cancel_count==100,"fresh and cancelled episodes are bounded and valid");
 for(const VehicleState vehicle: {VehicleState{true,1,1,L"",97},VehicleState{true,13,2,L"",4}}) {
    auto start=base;start.obstacles[index].cell={2,14,20};start.live_transitions.reset();start.position=world_of_cell(*town,p);start.vehicle=vehicle;
    start.previous_active.reset();start.pending={};auto initial_plan=plan_snapshot(start);
    auto moved=base;moved.obstacles[index].cell={2,14,21};moved.live_transitions.reset();moved.position=world_of_cell(*town,p1);moved.vehicle=vehicle;
    moved.previous_active=initial_plan.active;moved.pending=initial_plan.pending;auto result=plan_snapshot(moved);
    expect(initial_plan.path.success&&result.path.success&&!result.path.walk_phases.empty()&&result.path.walk_phases.front().cells.size()>1&&
       result.path.walk_phases.front().cells[1]==oldnext,"native Player or remembered other-map vehicle retains walking suffix");
 }
 auto native_vehicle=changed;native_vehicle.vehicle={true,13,2,L"",97};native_vehicle.active=*native_vehicle.previous_active;
 {SnapshotScope scope(native_vehicle);expect(!remote_door_candidate(*town),"actual local vehicle stays on established planner");}
 auto warp_leg=changed;warp_leg.pending.incoming_trap=warp_leg.pending.incoming_transition;warp_leg.pending.incoming_transition.reset();warp_leg.active=*warp_leg.previous_active;
 {SnapshotScope scope(warp_leg);expect(!remote_door_candidate(*town),"trap continuation stays on established planner");}
 std::printf("remote NPC retention: %d failures\n",failures);return failures;
}

// Enumerate only verified interior patrol cells and one-cardinal-step changes.
int patrol_probe(RouteSnapshot& snapshot,const baked::Submap& inn,const std::vector<Cell>& goal_cells) {
 const auto* town=snapshot.submap;const auto& grid=baked::grids(*town);const auto begin=std::chrono::steady_clock::now();
 auto ai=std::find_if(snapshot.obstacles.begin(),snapshot.obstacles.end(),[](const NativeObstacle& a){return a.id==39;});
 auto bi=std::find_if(snapshot.obstacles.begin(),snapshot.obstacles.end(),[](const NativeObstacle& a){return a.id==38;});
 if(ai==snapshot.obstacles.end()||bi==snapshot.obstacles.end()){std::puts("missing exact actors 38/39");return 1;}
 const size_t moving=static_cast<size_t>(ai-snapshot.obstacles.begin());const Cell home=ai->cell;
 auto live=live_blockers(*town,grid,{});auto movement=walk_request(*town,home);movement.ignore_hazards=true;
 movement.blocked=live.hard();movement.blocked.erase(std::remove_if(movement.blocked.begin(),movement.blocked.end(),[&](const Cell& c){return c.x==home.x&&c.y==home.y&&(c.layer<0||c.layer==home.layer);}),movement.blocked.end());
 movement.directional=live.directional;movement.live_traps=true;movement.traps=live.traps;movement.goals_open=false;
 auto inside=[](const Cell& c){return c.layer==2&&c.x>=7&&c.x<=22&&c.y>=11&&c.y<=31;};
 auto blocked=[&](const Request& r,const Cell& c){return std::any_of(r.blocked.begin(),r.blocked.end(),[&](const Cell& b){return b.x==c.x&&b.y==c.y&&(b.layer<0||b.layer==c.layer);});};
 static const int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
 std::vector<Cell> area;std::queue<Cell> queue;std::vector<uint8_t> seen(static_cast<size_t>(grid.width)*grid.height);
 if(!inside(home)||!open(movement,grid,home)){std::puts("NPC39 home outside validated open interior");return 1;}
 queue.push(home);seen[home.y*grid.width+home.x]=1;
 while(!queue.empty()){const auto c=queue.front();queue.pop();area.push_back(c);
  for(int d=0;d<4;++d){Cell next{home.layer,c.x+dx[d],c.y+dy[d]};if(!inside(next)||seen[next.y*grid.width+next.x]||
    !open(movement,grid,next)||blocked(movement,next)||trap_cell(movement,grid,next)||!edge_allowed(movement,grid,c,next)||
    layer_after(grid,c.layer,next.x,next.y)!=next.layer)continue;
   seen[next.y*grid.width+next.x]=1;queue.push(next);}
 }
 auto state=planning_state();const auto* arrival=point_in(*town,3,state);
 if(!arrival){std::puts("missing map97 PointIn3 arrival");return 1;}
 std::vector<Cell> starts={{2,16,19},{2,16,20},{2,16,21},{2,16,22},{2,18,21},{2,22,20},
     {arrival->layer,arrival->tx,arrival->ty}};
 for(int d=0;d<4;++d){Cell adjacent{arrival->layer,arrival->tx+dx[d],arrival->ty+dy[d]};if(baked::in_bounds(grid,adjacent.x,adjacent.y)&&open(movement,grid,adjacent)&&!blocked(movement,adjacent)&&!terminal_transition(movement,grid,adjacent)){starts.push_back(adjacent);break;}}
 if(starts.size()!=8){std::puts("arrival has no usable adjacent start");return 1;}
 std::printf("patrol NPC39 home=(%d,%d,L%d), interior flood=%zu, NPC38 fixed=(%d,%d,L%d), arrival=(%d,%d,L%d), starts=%zu\n",home.x,home.y,home.layer,area.size(),bi->cell.x,bi->cell.y,bi->cell.layer,arrival->tx,arrival->ty,arrival->layer,starts.size());
 int trials=0,planned=0,legal=0,changed_count=0,reversals=0,confirmed=0,examples=0;bool stop=false;
 for(const auto& actor_at:area){if(stop)break;
  for(const auto& origin:starts){if(stop)break;
   if(!baked::in_bounds(grid,origin.x,origin.y)||!open(movement,grid,origin)||blocked(movement,origin)||origin==actor_at)continue;
   auto initial=snapshot;initial.obstacles[moving].cell=actor_at;initial.live_transitions.reset();initial.position=world_of_cell(*town,origin);
   initial.previous_active.reset();initial.pending={};auto first=plan_snapshot(initial);++planned;
   if(!first.path.success||first.path.walk_phases.empty()||first.path.walk_phases.front().cells.size()<3)continue;
   const auto old=first.path.walk_phases.front().cells;const Cell after=old[1];
   for(int d=0;d<4;++d){if(++trials>8192){stop=true;break;}
    Cell moved{home.layer,actor_at.x+dx[d],actor_at.y+dy[d]};
    if(!inside(moved)||!seen[moved.y*grid.width+moved.x]||moved==after||moved==origin||blocked(movement,moved)||
       !open(movement,grid,moved)||!edge_allowed(movement,grid,actor_at,moved))continue;
    auto changed=initial;changed.obstacles[moving].cell=moved;changed.live_transitions.reset();changed.position=world_of_cell(*town,after);
    changed.previous_active=first.active;changed.pending=first.pending;SnapshotScope changed_scope(changed);
    auto nowlive=live_blockers(*town,grid,{});auto validation=walk_request(*town,after);validation.blocked=nowlive.hard();validation.directional=nowlive.directional;
    validation.live_traps=true;validation.traps=nowlive.traps;validation.ignore_hazards=true;validation.goals={old.back()};validation.goals_open=true;
    bool suffix=true;for(size_t k=2;k<old.size();++k){const auto& c=old[k];if(blocked(validation,c)||trap_cell(validation,grid,c)||
       !edge_allowed(validation,grid,old[k-1],c)||(!open(validation,grid,c)&&!(c==old.back()))){suffix=false;break;}}
    if(!suffix)continue;++legal;auto second=plan_snapshot(changed);
    if(!second.path.success||second.path.walk_phases.empty()||second.path.walk_phases.front().cells.size()<2)continue;
    const Cell next=second.path.walk_phases.front().cells[1];const bool reverse=next==origin,same=next==old[2];
    if(!same)++changed_count;
    if(!same&&examples<8){++examples;std::printf("changed origin=(%d,%d) after=(%d,%d) NPC=(%d,%d)->(%d,%d) next=(%d,%d) oldnext=(%d,%d) reverse=%d\n",origin.x,origin.y,after.x,after.y,actor_at.x,actor_at.y,moved.x,moved.y,next.x,next.y,old[2].x,old[2].y,reverse);}
    if(!reverse)continue;++reversals;
    auto third=changed;third.obstacles[moving].cell=actor_at;third.live_transitions.reset();third.position=world_of_cell(*town,origin);
    third.previous_active=second.active;third.pending=second.pending;auto again=plan_snapshot(third);
    const bool repeat=again.path.success&&!again.path.walk_phases.empty()&&again.path.walk_phases.front().cells.size()>=2&&again.path.walk_phases.front().cells[1]==after;
    std::printf("REVERSAL origin=(%d,%d,L%d) after=(%d,%d) NPC39=(%d,%d)->(%d,%d)->(%d,%d) third_returns=%d old_suffix_cells=%zu\n",origin.x,origin.y,origin.layer,after.x,after.y,actor_at.x,actor_at.y,moved.x,moved.y,actor_at.x,actor_at.y,repeat,old.size());
    for(size_t k=0;k<std::min<size_t>(6,old.size());++k)std::printf("old[%zu]=(%d,%d,L%d)\n",k,old[k].x,old[k].y,old[k].layer);
    for(size_t k=0;k<std::min<size_t>(6,second.path.walk_phases.front().cells.size());++k){const auto c=second.path.walk_phases.front().cells[k];std::printf("new[%zu]=(%d,%d,L%d)\n",k,c.x,c.y,c.layer);}
    Oracle oracle;if(oracle.build(changed,inn,goal_cells))std::printf("changed-pose whole-route oracle P=%d Pprime=%d\n",oracle.at(town,origin),oracle.at(town,after));
    else std::puts("witness whole-route oracle unavailable");
    if(repeat){++confirmed;stop=true;break;}
   }
  }
 }
 std::printf("patrol search actor39 cells=%zu starts=%zu initial-plans=%d candidate-pairs=%d legal-suffix=%d changed-next=%d reversals=%d confirmed-toggle=%d elapsed=%.3fms wrong-owner=%d\n",area.size(),starts.size(),planned,std::min(trials,8192),legal,changed_count,reversals,confirmed,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count(),fixture::wrong_owner.load());
 return confirmed?0:1;
}
}
int main(int argc,char** argv) {
 const bool baseline=argc>1&&std::string(argv[1])=="--baseline";
 const bool patrol=argc>1&&std::string(argv[1])=="--patrol";
 const bool verify=argc>1&&std::string(argv[1])=="--verify";
 std::ifstream f("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(f),{}};
 if(!baked::load(bytes.data(),bytes.size()))return 2;
 std::ifstream j("data/ff3/mapstate.json",std::ios::binary);std::string state_bytes{std::istreambuf_iterator<char>(j),{}};
 if(!json::parse(state_bytes,fixture::records))return 3;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;fixture::current_map=97;
 const auto* town=baked::submap_by_map_id(97);const auto* inn=baked::submap_by_map_id(101);
 if(!town||!inn){std::puts("missing Amur maps");return 4;}
 map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.current_map_id=97;
 auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));snapshot->submap=town;snapshot->map_id=97;snapshot->layer=1;snapshot->locale=L"en";
 const baked::Object* target=nullptr;for(const auto& o:baked::grids(*inn).objects)
  if((o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC)&&o.contains(17,12)){target=&o;break;}
 if(!target){std::puts("missing real map101 NPC at (17,12)");return 5;}
 snapshot->goal.map_id=101;snapshot->goal.cell={target->layer,17,12};snapshot->goal.entity_id=target->entity_id();snapshot->goal.raw_name=baked::object_identifier(*target);snapshot->kind=TargetKind::Interact;
 for(const auto& o:baked::grids(*town).objects)if((o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC)&&(o.d&1)&&snapshot->state.active(*town,o)){
  NativeObstacle actor;actor.type=o.type==baked::ObjType::NPC?6:17;actor.id=o.entity_id();actor.npc=true;actor.cell={o.layer,o.tx,o.ty};actor.raw_name=baked::object_identifier(o);actor.solidity_known=true;actor.solid=true;actor.movement_known=true;actor.wanders=(o.d&16)!=0;
  snapshot->obstacles.push_back(std::move(actor));if(snapshot->obstacles.size()==2)break;
 }
 if(snapshot->obstacles.size()<2){std::puts("missing two active solid map97 NPCs");return 6;}
 snapshot->layer=snapshot->obstacles.front().cell.layer;
 std::printf("neutral flags, current map97, goal map101 (%d,%d) id=%d, actors=%zu\n",target->tx,target->ty,target->entity_id(),snapshot->obstacles.size());
 auto raw_hash=[](const std::wstring& value){unsigned long long h=1469598103934665603ull;for(wchar_t c:value)h=(h^static_cast<unsigned>(c))*1099511628211ull;return h;};
 std::printf("goal raw fingerprint=%llu layer=%d\n",raw_hash(snapshot->goal.raw_name),target->layer);
 for(const auto& actor:snapshot->obstacles)std::printf("actor id=%d type=%d cell=(%d,%d,L%d) wanders=%d rawhash=%llu\n",actor.id,actor.type,actor.cell.x,actor.cell.y,actor.cell.layer,actor.wanders,raw_hash(actor.raw_name));
 const auto started=std::chrono::steady_clock::now();int failures=0;std::thread worker([&]{
  SnapshotScope scope(*snapshot);auto goals=goals_for(*inn,snapshot->goal.cell,TargetKind::Interact);snapshot->goal.cells=goals.cells;
  if(goals.cells.empty()){std::puts("missing interaction footprint");++failures;return;}
  if(patrol){failures+=patrol_probe(*snapshot,*inn,goals.cells);return;}
  if(verify){failures+=verify_retention(*snapshot);return;}
  Oracle oracle;if(!oracle.build(*snapshot,*inn,goals.cells)){std::puts("oracle component/goal unavailable");++failures;return;}
  std::printf("oracle floors=%zu states=%zu edges=%zu goals=%zu\n",oracle.floors.size(),oracle.distance.size(),oracle.links,goals.cells.size());
  const auto& grid=baked::grids(*town);auto req=walk_request(*town,{snapshot->layer,0,0});auto blockers=live_blockers(*town,grid,{});req.blocked=blockers.hard();req.directional=blockers.directional;req.live_traps=true;req.traps=blockers.traps;
  std::vector<Cell> anchors;for(const auto& a:snapshot->obstacles)anchors.push_back(a.cell);
  for(const auto& o:grid.objects)if(o.type==baked::ObjType::GotoMap&&o.a>0){anchors.push_back({o.layer,o.tx,o.ty});std::printf("map97 door id=%d at (%d,%d,L%d) to=%d point=%d\n",o.entity_id(),o.tx,o.ty,o.layer,o.a,o.b);}
  int entries=0;for(const auto& floor:oracle.floors)for(const auto& o:baked::grids(*floor.sm).objects)
   if(o.type==baked::ObjType::GotoMap&&baked::submap_by_map_id(o.a)==inn&&snapshot->state.active(*floor.sm,o)){
    const auto* arrival=point_in(*inn,o.b,snapshot->state);if(arrival){++entries;std::printf("inn inbound from map%d door=(%d,%d) arrival=(%d,%d,L%d)\n",floor.sm->first_map_id,o.tx,o.ty,arrival->tx,arrival->ty,arrival->layer);}
   }
  std::printf("inn inbound portals=%d\n",entries);
  std::vector<Cell> all;for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x){Cell c{snapshot->layer,x,y};if(open(req,grid,c)&&!oracle.blocked(req,c)&&!terminal_transition(req,grid,c)&&oracle.at(town,c)>=0)all.push_back(c);}
  auto distance=[&](const Cell& c){int best=INT_MAX;for(const auto& a:anchors)best=std::min(best,std::abs(a.x-c.x)+std::abs(a.y-c.y));return best;};
  std::stable_sort(all.begin(),all.end(),[&](const Cell& a,const Cell& b){return distance(a)<distance(b);});
  std::vector<Cell> starts;for(size_t i=0;i<std::min<size_t>(96,all.size());++i)starts.push_back(all[i]);
  for(size_t n=0;n<32&&starts.size()<128&&n<all.size();++n){const auto& c=all[(n*all.size())/32];if(std::find(starts.begin(),starts.end(),c)==starts.end())starts.push_back(c);}
  for(const auto& c:all){if(starts.size()>=128)break;if(std::find(starts.begin(),starts.end(),c)==starts.end())starts.push_back(c);}
  int compared=0,suboptimal=0;Cell witness{};int old_cost=-1,best_cost=-1;CrossLeg witness_leg;
  for(const auto& start:starts){snapshot->position=world_of_cell(*town,start);auto leg=find_across(nullptr,nullptr,snapshot->goal,true,false,true);const int optimal=oracle.at(town,start);if(!leg.found)continue;++compared;
   if(leg.steps_total>optimal){++suboptimal;if(old_cost<0){witness=start;old_cost=leg.steps_total;best_cost=optimal;witness_leg=leg;}}
  }
  std::printf("compared=%d/%zu suboptimal=%d first start=(%d,%d,L%d) actual=%d oracle=%d nextmap=%d\n",compared,starts.size(),suboptimal,witness.x,witness.y,witness.layer,old_cost,best_cost,witness_leg.next_map_id);
  if(!compared){std::puts("no comparable source paths");++failures;return;}
  if(old_cost<0){std::puts("no suboptimal witness within the bounded sample");witness=starts.front();if(baseline)++failures;}
  Cell here=witness;std::vector<Cell> history{here};auto previous=std::shared_ptr<ActiveRoute>();Pending pending;int static_bad=0;
  for(int step=0;step<256;++step){snapshot->position=world_of_cell(*town,here);snapshot->previous_active=previous;snapshot->pending=pending;
   auto result=plan_snapshot(*snapshot);if(!result.path.success||result.path.walk_phases.empty())break;
   const auto& cells=result.path.walk_phases.front().cells;if(cells.size()<2)break;const Cell next=cells[1];
   if(oracle.at(town,here)<0||oracle.at(town,next)!=oracle.at(town,here)-1)++static_bad;
   if(step<6)std::printf("step%d (%d,%d)->(%d,%d) oracle=%d->%d nextmap=%d\n",step,here.x,here.y,next.x,next.y,oracle.at(town,here),oracle.at(town,next),result.path.next_map_id);
   if(history.size()>=2&&next==history[history.size()-2]){std::printf("static reversal at step%d\n",step);break;}
   history.push_back(next);here=next;previous=result.active;pending=result.pending;
   if(terminal_transition(req,grid,here))break;
  }
  std::printf("static moves=%zu nondecreasing-oracle=%d reached-terminal=%d\n",history.size()-1,static_bad,terminal_transition(req,grid,here));
  // Controlled one-cell actor changes near authored actor homes; these are
  // synthetic snapshots and are not the unrecorded live session poses.
  auto actor_distance=[&](const Cell& c){int best=INT_MAX;for(const auto& a:snapshot->obstacles)best=std::min(best,std::abs(c.x-a.cell.x)+std::abs(c.y-a.cell.y));return best;};
  auto near_starts=all;std::stable_sort(near_starts.begin(),near_starts.end(),[&](const Cell& a,const Cell& b){return actor_distance(a)<actor_distance(b);});
  if(near_starts.size()>16)near_starts.resize(16);int pairs=0,both_pairs=0,retained=0,reversals=0,changed_next=0,examples=0;
  static const int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
  for(const Cell origin:near_starts) {
   snapshot->position=world_of_cell(*town,origin);snapshot->previous_active.reset();snapshot->pending={};
   auto initial=plan_snapshot(*snapshot);if(!initial.path.success||initial.path.walk_phases.empty()||initial.path.walk_phases.front().cells.size()<3)continue;
   const auto old=initial.path.walk_phases.front().cells;const Cell after=old[1];
   for(int variant=0;variant<3;++variant)for(int d=0;d<4;++d)for(int e=0;e<(variant==2?4:1);++e) {
    auto changed=*snapshot;
    if(variant==0||variant==2){changed.obstacles[0].cell.x+=dx[d];changed.obstacles[0].cell.y+=dy[d];}
    if(variant==1){changed.obstacles[1].cell.x+=dx[d];changed.obstacles[1].cell.y+=dy[d];}
    if(variant==2){changed.obstacles[1].cell.x+=dx[e];changed.obstacles[1].cell.y+=dy[e];}
    bool invalid=changed.obstacles[0].cell==changed.obstacles[1].cell;
    for(size_t i=0;i<changed.obstacles.size();++i){const auto& actor=changed.obstacles[i];const auto& before=snapshot->obstacles[i];
     if(!baked::in_bounds(grid,actor.cell.x,actor.cell.y)||!open(req,grid,actor.cell)||actor.cell==after||actor.cell==origin)invalid=true;
     if(!(actor.cell==before.cell)&&!edge_allowed(req,grid,before.cell,actor.cell))invalid=true;
    }
    if(invalid)continue;
    changed.position=world_of_cell(*town,after);changed.previous_active=initial.active;changed.pending=initial.pending;changed.live_transitions.reset();
    SnapshotScope nested(changed);auto live=live_blockers(*town,grid,{});auto movement=walk_request(*town,after);
    movement.blocked=live.hard();movement.directional=live.directional;movement.live_traps=true;movement.traps=live.traps;
    movement.ignore_hazards=true;movement.goals={old.back()};movement.goals_open=true;
    bool legal=true;for(size_t k=2;k<old.size();++k) {
     const auto& c=old[k];if(oracle.blocked(movement,c)||trap_cell(movement,grid,c)||!edge_allowed(movement,grid,old[k-1],c)||
       (!open(movement,grid,c)&&!(c==old.back()))) {legal=false;break;}
    }
    if(!legal)continue;++pairs;if(variant==2)++both_pairs;auto replanned=plan_snapshot(changed);if(!replanned.path.success||replanned.path.walk_phases.empty())continue;
    const auto& now=replanned.path.walk_phases.front().cells;if(now.size()<2)continue;
    const bool backward=now[1]==origin,same=now[1]==old[2];reversals+=backward;retained+=same;changed_next+=!same;
    if(backward||(!same&&examples<8)) {++examples;
     std::printf("dynamic origin=(%d,%d) after=(%d,%d) variant=%d actors=(%d,%d)/(%d,%d) old-legal=1 next=(%d,%d) oldnext=(%d,%d) reverse=%d\n",origin.x,origin.y,after.x,after.y,variant,changed.obstacles[0].cell.x,changed.obstacles[0].cell.y,changed.obstacles[1].cell.x,changed.obstacles[1].cell.y,now[1].x,now[1].y,old[2].x,old[2].y,backward);
    }
   }
  }
  std::printf("controlled actor-near origins=%zu legal pairs=%d both-moved=%d same-next=%d changed-next=%d immediate reversals=%d\n",near_starts.size(),pairs,both_pairs,retained,changed_next,reversals);
  if(!baseline&&suboptimal){std::puts("FAIL production whole-route cost exceeds oracle");++failures;}
 });worker.join();
 std::printf("offline fixture %.3fms wrong-owner=%d failures=%d baseline=%d\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count(),fixture::wrong_owner.load(),failures,baseline);
 return failures||fixture::wrong_owner?1:0;
}
