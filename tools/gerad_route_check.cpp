// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production guide/planner on exact native Gerad rooms and both turtle banks.
#define main floating_fixture_entry
#include "floating_continent_route_check.cpp"
#undef main
namespace gerad_fixture {
using namespace floating_fixture;
int assertions=0,failures=0,hops=0;
void expect(bool ok,const char* text){++assertions;if(!ok){++failures;std::printf("FAIL %s\n",text);}}
void context(map_state::Flags& f){const auto* room=baked::submap_by_map_id(f.current_map_id);const auto* row=fixture::records.get(baked::str(room->name));if(const auto* area=row?row->get(L"area_id"):nullptr)f.current_area_id=int(area->n);}
map_state::Flags flags(int map,int stage=332){auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=map;f.scenario[297]=1;for(int i=322;i<=stage;++i)f.scenario[size_t(i)]=1;f.scenario[575]=f.scenario[576]=stage>=328;context(f);return f;}
Cell entrance(int map,int point){for(const auto&o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(const map_state::Flags& f,Cell at){ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";return ffpr::ff6::story::plan(s);}
Planned route(map_state::Flags f,Cell from,Cell to,bool npc=false,int pose=9){
 const auto* room=baked::submap_by_map_id(f.current_map_id);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=room;s.layer=from.layer;s.position=world_of_cell(*room,from);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.goal.map_id=s.map_id;s.goal.cell=to;s.kind=npc?TargetKind::Interact:TargetKind::Spot;s.goal.avoid_other_events=true;s.goal.allow_scripted_crossings=true;obstacles(s);
 for(auto& a:s.obstacles)if(a.npc&&a.id==16){a.cell={1,pose,10};a.movement_known=true;a.wanders=f.scenario[332]==1;if(npc){s.goal.entity_id=16;s.goal.raw_name=a.raw_name;}}
 Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();return p;
}
bool walk_guide(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<24;++leg){context(f);const auto* room=baked::submap_by_map_id(f.current_map_id);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=room;s.layer=at.layer;s.position=world_of_cell(*room,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);Planned p;ffpr::ff6::story::Plan advice;
  std::thread worker([&]{advice=model(f,at);if(advice.supported&&!advice.unknown&&!advice.finished&&!advice.targets.empty()){s.candidates=advice.targets;p=plan_snapshot(s);}});worker.join();
  std::printf("Gerad map%d from%d,%d,%d expect%d chose%d map%d success%d next%d unknown%d reason%s note%s\n",s.map_id,at.layer,at.x,at.y,expected,p.goal.entity_id,p.goal.map_id,p.path.success,p.path.next_map_id,advice.unknown,text::to_utf8(p.path.reason).c_str(),text::to_utf8(advice.note).c_str());
  if(!p.path.success||p.path.required_trap||p.path.npc_approach)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return p.goal.entity_id==expected;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;
  const bool waypoint=p.goal.entity_id<0&&receipt->destination==p.goal.map_id&&receipt->arrival==p.goal.cell;
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);++hops;
  if(receipt->map_id==449&&f.current_map_id==448&&f.scenario[331]==1){f.scenario[332]=1;at={1,17,13};}
  // An arrival continuation can move the player before input is returned.
  // The real door receipt owns entry; the next request uses that final pose.
  if(expected<0&&waypoint)return true;
 }
 return false;
}
}
#ifndef FFPR_GERAD_FIXTURE_ONLY
int main(){using namespace gerad_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char*p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 for(int pose:{7,9,11})for(bool north:{true,false}){
  const Cell bank{1,9,north?12:8},landing{1,9,north?8:12};auto f=flags(448);auto p=route(f,bank,landing,false,pose);
  expect(p.path.success&&p.path.next_map_id==448&&!p.path.required_trap,"both turtle directions work at every real patrol pose");
  const auto receipt=p.pending.incoming_transition;
  expect(receipt&&receipt->entity_id==16&&receipt->entry==bank&&receipt->arrival==landing,"manual receipt retains the actual bank and native landing");
  expect(!p.path.walk_phases.empty()&&p.path.walk_phases.back().manual_after&&p.path.walk_phases.back().arrival_direction==(north?0:2)&&p.path.walk_phases.back().action_after.find(L"keep interacting")!=std::wstring::npos,"manual action faces the accessible bank and explains waiting interaction");
  auto talk=route(f,bank,{1,7,10},true,pose);expect(talk.path.success==(pose==9),"ordinary interaction still requires the actual current turtle pose");
 }
 for(int state:{-1,0}){auto f=flags(448);if(state<0)f.scenario[332].reset();else f.scenario[332]=0;expect(!route(f,{1,9,12},{1,9,8}).path.success,"unstarted or unknown332 cannot grant the crossing");}
 expect(!route(flags(448),{2,9,12},{1,9,8}).path.success,"a wrong player layer cannot borrow the turtle interaction plane");
 {auto f=flags(217,321);Cell at=entrance(217,1);for(auto [id,receipt]:{std::pair{3,322},std::pair{2,323},std::pair{1,324},std::pair{4,325}}){expect(walk_guide(f,at,id),"each actual bandit is reachable");f.scenario[size_t(receipt)]=1;}f.scenario[326]=1;expect(walk_guide(f,at,-1),"departure uses the real pub-to-town entrance");f.scenario[327]=1;expect(walk_guide(f,at,1),"first Gerad talk follows crew departure");f.local[1]=1;auto p=model(f,at);expect(p.targets.empty()&&p.note.find(L"Wait for Gerad")!=std::wstring::npos,"first follow waits for575");f.scenario[575]=1;expect(walk_guide(f,at,72),"native reload resumes the visible alternate actor");f.local[1]=2;p=model(f,at);expect(p.targets.empty()&&p.note.find(L"Wait for Gerad")!=std::wstring::npos,"second follow waits for576");f.scenario[576]=1;expect(walk_guide(f,at,72),"final conversation uses the stopped dock pose");f.scenario[328]=1;at={2,14,58};expect(walk_guide(f,at,6),"first ferry boarding follows Gerad's final scene");}
 {auto f=flags(246,329);auto at=entrance(246,2);expect(walk_guide(f,at,11),"South Figaro arrival reaches Gerad at the inn");f.scenario[330]=1;expect(walk_guide(f,at,6),"inn conversation leads through the world to cave Siegfried");f.scenario[331]=1;expect(walk_guide(f,at,-1),"Siegfried leads to the native turtle entry scene");expect(f.scenario[332]==1,"actual449-to448 continuation owns332");expect(walk_guide(f,at,1),"crossing reaches fleeing Siegfried");f.scenario[333]=1;expect(walk_guide(f,at,14),"cave exit reaches castle jail");f.scenario[334]=1;expect(walk_guide(f,at,1),"jail passage reaches the engine room");f.scenario[335]=1;expect(walk_guide(f,at,6),"Tentacles completion leads to surfacing");expect(model(f,at).finish_on_scenario==336,"only surfacing arms final336");}
 {auto f=flags(218,329);auto p=model(f,entrance(218,2));expect(p.targets.size()==1&&p.targets[0].goal.entity_id==5&&p.targets[0].instruction.find(L"second option")!=std::wstring::npos,"repeat ferry uses the second choice");f.scenario[329].reset();expect(model(f,entrance(218,2)).unknown,"unknown first ferry cannot skip boarding");}
 expect(fixture::wrong_owner==0,"workers own no native API or pointer");
 std::printf("Gerad:%d assertions/%d continuations/%d failures/owner%d\n",assertions,hops,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}


#endif
