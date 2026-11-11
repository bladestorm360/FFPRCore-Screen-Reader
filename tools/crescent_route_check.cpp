// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete real-bake guide legs, with native receipts supplied as owned inputs.
#define main retained_crescent_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace crescent_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
void context(map_state::Flags& f){const auto* m=baked::submap_by_map_id(f.current_map_id);const auto* row=fixture::records.get(baked::str(m->name));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
Cell entrance(int map,int id){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==id)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(const map_state::Flags& f,Cell at){ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";return ffpr::ff6::story::plan(s);}
Planned crossing_probe(map_state::Flags f,int from,int point,int to,int entity){
 f.current_map_id=from;context(f);const auto at=entrance(from,point);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=from;s.submap=baked::submap_by_map_id(from);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.goal.map_id=to;s.goal.entity_id=entity;s.goal.allow_scripted_crossings=true;s.goal.avoid_other_events=true;
 if(entity>=0)for(const auto& o:baked::grids(*baked::submap_by_map_id(to)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==entity){s.goal.raw_name=baked::object_identifier(o);s.goal.cell={o.layer,o.tx,o.ty};s.kind=baked_route::step_event(o)?TargetKind::Spot:TargetKind::Interact;break;}
 if(entity<0){for(const auto& o:baked::grids(*baked::submap_by_map_id(382)).objects)if(o.type==baked::ObjType::Warp&&o.a==96)for(const auto& arrival:baked::grids(*baked::submap_by_map_id(96)).objects)if(arrival.type==baked::ObjType::PointIn&&arrival.a==o.b)s.goal.cell={arrival.layer,arrival.tx,arrival.ty};s.kind=TargetKind::Spot;}
 obstacles(s);Planned result;std::thread worker([&]{result=plan_snapshot(s);});worker.join();
 std::printf("Crescent cross-floor probe%d ->%d/%d at%d,%d,%d raw%s success%d next%d error%s reason%s\n",from,to,entity,s.goal.cell.layer,s.goal.cell.x,s.goal.cell.y,text::to_utf8(s.goal.raw_name).c_str(),result.path.success,result.path.next_map_id,text::to_utf8(result.path.error).c_str(),text::to_utf8(result.path.reason).c_str());
 return result;
}
std::optional<std::pair<int,int>> next(map_state::Flags& f,Cell& at){
 for(int step=0;step<24;++step){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);
  Planned result;ffpr::ff6::story::Plan advice;std::thread worker([&]{advice=model(f,at);if(advice.supported&&!advice.unknown&&!advice.finished&&!advice.targets.empty()){s.candidates=advice.targets;result=plan_snapshot(s);}});worker.join();
  std::printf("Crescent map%d from%d,%d,%d goal%d/%d success%d next%d unknown%d candidates%zu reason%s\n",s.map_id,at.layer,at.x,at.y,result.goal.map_id,result.goal.entity_id,result.path.success,result.path.next_map_id,advice.unknown,advice.targets.size(),text::to_utf8(result.path.reason).c_str());
  if(!result.path.success||result.path.required_trap||result.path.npc_approach)return {};++legs;
  if(!result.path.next_map_id){for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return std::pair{result.goal.map_id,result.goal.entity_id};}
  const auto r=result.pending.incoming_transition;if(!r)return {};
  const bool waypoint=result.goal.entity_id<0&&r->destination==result.goal.map_id&&r->arrival==result.goal.cell;
  const bool transition=result.goal.map_id==r->map_id&&result.goal.entity_id==r->entity_id;
  f.current_map_id=r->destination;at=r->arrival;f.local.fill(0);if(waypoint)return std::pair{result.goal.map_id,-1};if(transition)return std::pair{result.goal.map_id,result.goal.entity_id};
 }
 return {};
}
}
int main(){
 using namespace crescent_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.party_gau=false;f.current_map_id=378;context(f);Cell at=entrance(378,1);
 auto p=model(f,at);check(p.targets.empty()&&p.note==L"Bring Gau in the active party, then request guidance again.","reserve/absent Gau cannot unlock helmet search");
 f.party_gau.reset();check(model(f,at).unknown,"unknown active party cannot certify Gau's physical event");f.party_gau=true;
 auto goal=next(f,at);check(goal==std::optional{std::pair{378,5}},"first native helmet-information region is reachable");f.scenario[129]=1;
 goal=next(f,at);check(goal==std::optional{std::pair{378,10}},"actual helmet is reached without optional wrong-search/prank regions");
 check(!model(f,at).finished,"helmet search cannot finish the complete underwater journey");
 // Real scene owns133 and automatically enters the3D trench; Right then puts
 // the player at the actual first cave arrival, not an invented walking edge.
 f.scenario[133]=1;f.current_map_id=379;context(f);at=entrance(379,1);
 goal=next(f,at);check(goal==std::optional{std::pair{379,4}},"first Right cave routes its actual treasure chest");f.treasure[112]=1;
 goal=next(f,at);check(goal==std::optional{std::pair{379,0}},"first chest is followed by its native exit region");
 // Second Right returns control at the second cave's genuine PointIn1.
 f.current_map_id=380;context(f);at=entrance(380,1);f.area.fill(0);
 goal=next(f,at);check(goal==std::optional{std::pair{381,1}},"second cave reaches the drain control across floors");
 f.scenario[74]=1;check(!model(f,at).targets.empty()&&model(f,at).targets[0].goal.entity_id==1,"old permanent74 cannot stand in for the current area drain");
 f.area[1]=1;goal=next(f,at);check(goal==std::optional{std::pair{382,6}},"drained passage reaches the second actual chest");f.treasure[113]=1;
 p=model(f,at);check(!p.finished&&p.finish_on_exit==96&&p.finish_through==std::vector<int>({573,1}),"last chest still requires full native final callback/arrival");
 goal=next(f,at);check(goal==std::optional{std::pair{382,0}},"second chest routes the true last underwater exit");
 f.current_map_id=96;context(f);at=entrance(96,2);check(!model(f,at).finished,"arrival without actual521 cannot certify full journey completion");f.scenario[521]=1;check(model(f,at).finished,"full521 plus both treasure receipts completes at ready Nikeah");
 f.treasure[112]=0;p=model(f,at);check(p.targets.empty()&&p.note==L"Return after the battle at Narshe.","missed first-pass treasure preserves the native138 repeat gate");f.treasure[112]=1;f.scenario[138]=1;
 f.current_map_id=378;context(f);at=entrance(378,1);check(!model(f,at).finished,"finished repeat inside helmet cave still has an exit step");
 goal=next(f,at);check(goal==std::optional{std::pair{377,-1}},"finished helmet room leads to the outside platform instead of another forced dive");check(model(f,at).finished,"outside platform completes an already finished journey");
 f.treasure[112]=0;f.party_gau=false;goal=next(f,at);check(goal&&(goal->first==377&&(goal->second==1||goal->second==2)),"missed first chest allows repeat dive without requiring Gau again");
 f.current_map_id=380;context(f);at=entrance(380,1);f.area.fill(0);p=model(f,at);check(p.targets.size()==1&&p.targets[0].goal.map_id==381,"repeat second cave reopens current-area drain even with old74");
 f.area[1]=1;f.treasure[113]=1;f.current_map_id=382;context(f);at=entrance(382,1);p=model(f,at);check(!p.finished&&!p.finish_on_exit&&p.targets.size()==1&&p.targets[0].goal.entity_id==0,"missed first chest exits without falsely finishing the full treasure journey");
 f.current_map_id=96;p=model(f,at);check(p.targets.size()==1&&p.targets[0].goal.map_id==378&&p.targets[0].goal.allow_world_transit&&!p.targets[0].goal.avoid_other_events,"missed cave return delegates real world access to ordinary walking/owned-vehicle policy");
 f.treasure[112]=f.treasure[113]=0;
 auto cross=crossing_probe(f,377,1,379,4);check(cross.path.success&&cross.path.next_map_id==379&&cross.pending.incoming_transition&&cross.pending.incoming_transition->destination==379,"generic cross-floor route collapses only the known native first Right journey");
 check(!cross.path.walk_phases.empty()&&cross.path.walk_phases.back().manual_after&&cross.path.walk_phases.back().action_after.find(L"Choose the first option.")!=std::wstring::npos&&cross.path.walk_phases.back().action_after.find(L"Choose Right at the junction.")!=std::wstring::npos,"repeat route preserves explicit player-operated dive and timed Right instruction");
 f.scenario[138]=0;cross=crossing_probe(f,377,1,379,4);check(!cross.path.success,"generic repeat transfer cannot bypass native138");f.scenario[138]=1;
 cross=crossing_probe(f,379,1,382,6);check(cross.path.success&&cross.path.next_map_id==380,"generic second Right returns to actual380 rather than movie573");
 cross=crossing_probe(f,382,1,96,-1);check(cross.path.success&&cross.path.next_map_id==96&&cross.pending.incoming_transition&&cross.pending.incoming_transition->arrival==cross.goal.cell,"generic final return reaches the actual final Nikeah pose");
 check(fixture::wrong_owner==0,"model/routes use no native API or pointer on workers");
 std::printf("Crescent:%d assertions/%d real route legs/%d failures;owned representative poses,not game/audible timing\n",cases,legs,errors);return errors?1:0;
}
