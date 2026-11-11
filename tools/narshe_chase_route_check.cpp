// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Reuse the existing actual-model/planner fixture facade, never its simulated
// receipt choices. This fixture owns Narshe's independent native state profile.
#define main floating_fixture_entry
#include "floating_continent_route_check.cpp"
#undef main
namespace narshe_chase_fixture {
using namespace floating_fixture;
map_state::Flags initial(){auto f=flags();f.scenario.fill(0);for(int i=0;i<=208;++i)f.scenario[size_t(i)]=1;f.current_map_id=3;f.current_area_id=7;return f;}
void map_context(map_state::Flags& f){
 const auto* sm=baked::submap_by_map_id(f.current_map_id);if(!sm)return;
 if(const auto* row=fixture::records.get(baked::str(sm->name)))if(const auto* area=row->get(L"area_id"))f.current_area_id=int(area->n);
}
bool route(map_state::Flags& f,Cell& at,int wanted,int& hops){
 for(int leg=0;leg<16;++leg){
  map_context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;
  obstacles(s);Planned result;ffpr::ff6::story::Plan advice;
  std::thread worker([&]{ffpr::ff6::story::Snapshot state;state.map=s.map_id;state.flags=f;state.player=at;state.locale=L"en";advice=ffpr::ff6::story::plan(state);s.candidates=advice.targets;if(!s.candidates.empty())result=plan_snapshot(s);});worker.join();
  std::printf("Narshe guide map%d at%d,%d,%d expected%d selected%d success%d next%d trap%d unknown%d note%s reason%s\n",s.map_id,at.layer,at.x,at.y,wanted,result.goal.entity_id,result.path.success,result.path.next_map_id,result.path.required_trap,advice.unknown,text::to_utf8(advice.note).c_str(),text::to_utf8(result.path.reason).c_str());
  if(!advice.supported||advice.unknown||advice.finished||advice.targets.empty()||!result.path.success||result.path.required_trap||result.goal.entity_id!=wanted)return false;
  if(!result.path.next_map_id){for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=result.pending.incoming_transition;if(!receipt)return false;++hops;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(wanted<0&&f.current_map_id==result.goal.map_id)return at==result.goal.cell;
 }
 return false;
}
}
int main(){using namespace narshe_chase_fixture;int failures=0,cases=0,hops=0;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* path){std::ifstream stream(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(stream),{});};
 const auto expect=[&](bool pass,const char* name){++cases;if(!pass){++failures;std::printf("FAIL %s\n",name);}};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto model=[&](map_state::Flags f,int map,Cell at={1,19,18}){ffpr::ff6::story::Snapshot state;state.map=map;state.flags=f;state.flags.current_map_id=map;state.player=at;state.locale=L"en";return ffpr::ff6::story::plan(state);};
 {auto f=initial();f.scenario[208]=0;auto at=Cell{1,38,61};expect(route(f,at,-1,hops),"native treasure-room entry is reachable from town");auto p=model(f,16);expect(p.supported&&!p.unknown&&p.targets.empty()&&p.note==L"Wait for the treasure-room scene to finish, then use Shift+G.","treasure-room timer keeps a native wait instruction");}
 {auto f=initial();f.current_map_id=16;auto at=Cell{1,8,18};for(int id:{19,124,120,-1,18,35,29}){const bool reached=route(f,at,id,hops);expect(reached,"sequential actual chase objective is physically reachable");if(!reached)break;if(id>=0)f.scenario[size_t(id==19?209:id==124?210:id==120?211:id==18?212:id==35?213:214)]=1;}
  f.current_map_id=20;f.scenario[214]=0;f.local[1]=1;auto p=model(f,20);expect(p.supported&&!p.unknown&&p.targets.empty()&&p.note==L"Stay still until the ledge scene finishes, then use Shift+G.","running native timer never gives a movement goal");
  f.local[1]=0;f.scenario[214]=1;at={1,19,18};expect(route(f,at,28,hops),"after native ledge scene the guide reaches Mog rather than the alternate actor");f.scenario[215]=1;p=model(f,20);expect(p.finished&&!p.unknown,"native rescue215 completes the guide");}
 {auto f=initial();f.scenario[216]=1;const auto p=model(f,20);expect(p.finished&&p.targets.empty(),"alternate native result never offers the missed rescue again");f.scenario[216]=0;f.scenario[214].reset();expect(model(f,20).unknown,"missing native progress cannot certify a rescue step");}
 {auto f=initial();f.scenario[207]=0;f.scenario[137]=1;f.scenario[138]=0;auto p=model(f,19);expect(p.supported&&!p.unknown&&!p.finished&&p.targets.size()==13,"earlier Closed Narshe defense retains its original selection");f.scenario[207].reset();expect(model(f,19).unknown,"unknown phase cannot fall through to an unrelated episode");}
 std::printf("Narshe actual chase:%d assertions/%d native continuations/%d failures/owner%d\n",cases,hops,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
