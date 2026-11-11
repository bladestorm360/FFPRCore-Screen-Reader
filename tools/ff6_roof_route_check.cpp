// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Physical primary-data witnesses for the Zone Eater roof-room safe areas.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff6/zone_eater_roof_model.cpp"
#include <limits>
int main(){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records))return 2;
 json::Value puzzles;if(!json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;const json::Value* guide=nullptr;
 for(const auto& g:puzzles.get(L"story_guides")->arr)if(g.get(L"id")->s==L"zone_eater_roof")guide=&g;if(!guide)return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;const auto* sm=baked::submap_by_map_id(504);if(!sm)return 2;
 map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.global[0]=4;flags.area[1]=1;flags.current_map_id=504;
 const std::array<Cell,10> points{{{1,37,18},{1,37,12},{1,33,9},{1,29,14},{1,24,10},{1,18,13},{1,13,9},{1,10,15},{1,9,19},{1,9,20}}};
 int failures=0;const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
 const std::array<size_t,9> expected_steps{{6,7,9,9,9,9,9,5,1}};
 for(size_t i=1;i<points.size();++i){
  RouteSnapshot s(map_state::Resolver(flags,fixture::records));s.map_id=504;s.submap=sm;s.layer=1;s.position=world_of_cell(*sm,points[i-1]);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
  s.goal.map_id=504;s.goal.cell=points[i];s.kind=i+1==points.size()?TargetKind::Transition:TargetKind::Spot;s.name=L"Safe area";
  Planned result;std::thread worker([&]{result=plan_snapshot(s);});worker.join();size_t steps=0;for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())steps+=phase.cells.size()-1;
  if(!result.path.success)++failures;
  expect(steps==expected_steps[i-1],"independently reviewed physical safe-area leg length");
  ffpr::ff6::story::Snapshot state;state.map=504;state.player=points[i-1];state.flags=flags;ffpr::ff6::story::Plan advice;
  std::thread model([&]{advice=ffpr::ff6::story::roof_plan(state,*guide);});model.join();
  expect(advice.supported&&!advice.unknown&&advice.targets.size()==1&&advice.targets[0].goal.cell==points[i],"actual roof model selects the next physical station");
  expect(advice.roof_timed==(i+1<points.size())&&advice.finish_on_exit==(i+1==points.size()?505:0),"only final safe-area exit arms completion; other legs require preview/readiness");
  // Match the production request's one recognized destination, including the
  // native exit-trigger cell at the border; do not grant other wall openings.
  baked_route::Request edge;edge.submap=sm;edge.goals={points[i]};
  for(const auto& phase:result.path.walk_phases)for(size_t j=1;j<phase.cells.size();++j){const auto a=phase.cells[j-1],b=phase.cells[j];expect(a.layer==1&&b.layer==1&&std::abs(a.x-b.x)+std::abs(a.y-b.y)==1&&baked_route::edge_allowed(edge,baked::grids(*sm),a,b),"every timed leg retains a legal cardinal native edge");}
  std::printf("Roof leg%zu %d,%d to%d,%d: success%d steps%zu nextmap%d reason%ls directions%ls\n",i,points[i-1].x,points[i-1].y,points[i].x,points[i].y,result.path.success,steps,result.path.next_map_id,result.path.reason.c_str(),result.path.description.c_str());
 }
 // Real automatic floor boundaries, not just guessed local arrival coordinates.
 for(int origin:{503,505}) {
  const auto* from=baked::submap_by_map_id(origin);map_state::Resolver resolver(flags,fixture::records);
  const auto* start=from?point_in(*from,1,resolver):nullptr;
  expect(start!=nullptr,"ordinary neighboring floor has its real entry point");if(!start)continue;
  auto current=flags;current.current_map_id=origin;
  RouteSnapshot s(map_state::Resolver(current,fixture::records));s.map_id=origin;s.submap=from;s.layer=start->layer;s.position=world_of_cell(*from,{start->layer,start->tx,start->ty});s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
  s.goal.map_id=504;s.goal.cell=origin==503?points.front():points[8];s.kind=TargetKind::Spot;Planned p;
  std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  expect(p.path.success&&p.path.next_map_id==504,"automatic entry reaches the roof through its actual source floor");
  std::printf("Roof incoming from%d: success%d next%d reason%ls\n",origin,p.path.success,p.path.next_map_id,p.path.reason.c_str());
 }
 size_t recoveries=0,resumes=0;double peak=0;
 std::thread recovery([&]{
  using namespace ffpr::ff6::story;
  baked_route::Request movement;movement.submap=sm;movement.start=points[0];movement.blocked={{1,29,13},{1,24,9},{1,10,10},{1,37,19},{1,9,20}};movement.live_hazards=true;movement.live_traps=true;
  const auto reach=baked_route::flood(movement);const auto& g=baked::grids(*sm);
  for(int y=0;y<g.height;++y)for(int x=0;x<g.width;++x)if(reach.dist[1][static_cast<size_t>(y)*g.width+x]!=0xffff){
   Snapshot s;s.map=504;s.player={1,x,y};s.flags=flags;const auto begin=std::chrono::steady_clock::now();const auto p=roof_plan(s,*guide);peak=std::max(peak,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
   expect(!p.unknown&&p.targets.size()==1,"every physically connected floor cell has a real safe-area recovery");++recoveries;
  }
  for(const auto& safe:guide->get(L"safe_cells")->arr){
   Snapshot s;s.map=504;s.flags=flags;s.player={1,int(safe.arr[1].n),int(safe.arr[2].n)};bool finished=false;
   for(int step=0;step<12;++step){auto p=roof_plan(s,*guide);if(p.unknown||p.targets.size()!=1)break;s.player=p.targets[0].goal.cell;if(p.finish_on_exit==505){finished=s.player==points.back();break;}}
   expect(finished,"all34 safe markers resume to the actual505 exit without looping");++resumes;
  }
  Snapshot s;s.map=504;s.player=points[0];s.roof_seconds=4.75f;expect(roof_fresh(s),"fresh native cycle accepted");s.roof_seconds=4.74f;expect(!roof_fresh(s),"late cycle rejected");s.roof_frozen=true;expect(roof_fresh(s),"existing user-frozen timer remains usable");s.roof_seconds=std::numeric_limits<float>::quiet_NaN();expect(!roof_fresh(s),"nonfinite timer cannot validate a leg");s.roof_seconds=6;expect(!roof_fresh(s),"wrong timer duration rejected");s.roof_seconds.reset();expect(!roof_fresh(s),"unknown timer never fabricates a window");s.player.layer=2;expect(roof_plan(s,*guide).unknown,"temporary upper-layer animation is not a ground route");
  s.player=points[0];s.cancelled=[] {return true;};expect(roof_plan(s,*guide).targets.empty(),"cancelled snapshot produces no goal");
 });recovery.join();
 std::printf("Roof connected-cell recoveries%zu safe resumes%zu peak model%.3fms (capped fixture, not game latency)\n",recoveries,resumes,peak);
 if(fixture::wrong_owner)++failures;std::printf("Roof physical geometry: %d failures; owner%d\n",failures,fixture::wrong_owner.load());return failures?1:0;
}
