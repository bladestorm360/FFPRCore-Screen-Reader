// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Current guide/native receipts/geometry, including player-operated pit falls.
#define main retained_esper_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace esper_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0,falls=0,manuals=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
void context(map_state::Flags& f){const auto* m=baked::submap_by_map_id(f.current_map_id);const auto* row=fixture::records.get(baked::str(m->name));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
Cell entrance(int map,int id){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==id)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(const map_state::Flags& f,Cell at,field::puzzle_policy::Policy policy={}){ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool jump(map_state::Flags f,const baked::Object& arc,Cell landing){
 f.current_map_id=424;context(f);const auto* sm=baked::submap_by_map_id(424);const int delta[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
 for(int side=0;side<4;++side){const Cell from{arc.layer,arc.tx+delta[side][0],arc.ty+delta[side][1]};if(!baked::walkable(baked::grids(*sm),from.layer,from.x,from.y,true))continue;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=424;s.submap=sm;s.layer=from.layer;s.position=world_of_cell(*sm,from);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.goal.map_id=424;s.goal.cell=landing;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;s.kind=TargetKind::Spot;obstacles(s);
  Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();const auto r=p.pending.incoming_transition;
  if(p.path.success&&r&&r->entity_id==arc.entity_id()&&r->arrival==landing&&!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after){++legs;++manuals;return true;}
 }
 return false;
}
std::optional<std::pair<int,int>> next(map_state::Flags& f,Cell& at){
 for(int step=0;step<32;++step){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);
  Planned result;ffpr::ff6::story::Plan advice;std::thread worker([&]{advice=model(f,at);if(advice.supported&&!advice.unknown&&!advice.finished&&!advice.targets.empty()){s.candidates=advice.targets;result=plan_snapshot(s);}});worker.join();
  std::printf("Esper map%d from%d,%d,%d goal%d/%d success%d next%d trap%d unknown%d candidates%zu error%s reason%s\n",s.map_id,at.layer,at.x,at.y,result.goal.map_id,result.goal.entity_id,result.path.success,result.path.next_map_id,result.path.required_trap,advice.unknown,advice.targets.size(),text::to_utf8(result.path.error).c_str(),text::to_utf8(result.path.reason).c_str());
  if(!result.path.success||result.path.npc_approach)return {};++legs;if(result.path.required_trap)++falls;
  for(const auto& phase:result.path.walk_phases)if(phase.manual_after)++manuals;
  if(!result.path.next_map_id){for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return std::pair{result.goal.map_id,result.goal.entity_id};}
  auto receipt=result.pending.incoming_transition;if(!receipt)receipt=result.pending.incoming_trap;if(!receipt)return {};
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
 }
 return {};
}
}
int main(){
 using namespace esper_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=422;context(f);Cell at=entrance(422,1);
 auto p=model(f,at);check(!p.supported&&p.note==L"No puzzle guide for this map","ordinary earlier cave access does not select the later story guide");f.scenario[276]=1;
 for(auto [flag,map,id]:{std::tuple{277,422,12},std::tuple{278,424,19},std::tuple{279,425,10},std::tuple{280,425,2}}){
  auto goal=next(f,at);check(goal==std::optional{std::pair{map,id}},"initial sightings/statues/actual Ultros trigger have complete physical routes");if(!goal)break;
  p=model(f,at);if(flag==280){check(p.puzzle_types==2&&p.targets[0].instruction.find(L"Sketch")!=std::wstring::npos,"native portrait mechanic is a non-navigation instruction");check(model(f,at,{true,false}).targets.empty(),"non-navigation off suppresses the Sketch solution without skipping the fight");check(!model(f,at,{false,true}).targets.empty(),"Other can supply the current Sketch solution with navigation guidance off");}
  f.scenario[size_t(flag)]=1;
 }
 if(f.scenario[280]==1){
  check(!model(f,at).finished,"native battle280 does not finish before the actual Esper meeting");
  for(auto [flag,map]:{std::pair{148,427},std::pair{149,428}}){auto goal=next(f,at);check(goal==std::optional{std::pair{map,4}},"both pit-branch treasure chests are included before the meeting");if(!goal)break;f.treasure[size_t(flag)]=1;}
  if(f.treasure[149]==1){auto goal=next(f,at);check(goal==std::optional{std::pair{430,24}},"third branch reaches the actual Espers rather than stopping at a pit");p=model(f,at);check(!p.finished&&p.finish_on_scenario==281,"full native return281 owns completion");f.scenario[281]=1;check(model(f,at).finished,"actual full Esper-return receipt completes the journey");}
 }
 f.scenario[281]=0;f.scenario[277]=f.scenario[278]=0;f.scenario[279]=1;f.scenario[280]=0;p=model(f,at);check(p.targets.size()==1&&p.targets[0].goal.map_id==425&&p.targets[0].goal.entity_id==2,"missed optional sightings cannot send a later player backwards after statue279");
 for(int entity:{16,17,18}){
  const auto* sm=baked::submap_by_map_id(424);const auto& grid=baked::grids(*sm);std::vector<const baked::Object*> arcs;for(const auto& o:grid.objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==entity)arcs.push_back(&o);
  check(arcs.size()==1&&(arcs[0]->d&255)==2&&arcs[0]->a==-1,"each manual bridge control has one same-floor native crossing");
  if(arcs.size()==1){const auto land=entrance(424,arcs[0]->b);check(land.layer==1&&baked::walkable(grid,land.layer,land.x,land.y,true),"native final jump landing is on the current ground geometry");check(jump(f,*arcs[0],land),"each real bridge jump is routed from an accessible side through its own manual receipt to the exact native landing");}
 }
 for(int entity:{10,11,12}){const auto& grid=baked::grids(*baked::submap_by_map_id(426));std::vector<const baked::Object*> arcs;for(const auto& o:grid.objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==entity)arcs.push_back(&o);check(arcs.size()==1&&arcs[0]->a==424,"each real return pit has exactly one destination");if(arcs.size()==1){auto land=entrance(424,arcs[0]->b);check(baked::walkable(baked::grids(*baked::submap_by_map_id(424)),land.layer,land.x,land.y,true),"post-fall eight-step landing is on real geometry");}}
 check(fixture::wrong_owner==0,"guide and route workers own no native API/pointer");
 std::printf("Esper:%d assertions/%d real legs/%d trap legs/%d manual phases/%d failures;representative owned poses,not actual movement/audible timing\n",cases,legs,falls,manuals,errors);return errors?1:0;
}
