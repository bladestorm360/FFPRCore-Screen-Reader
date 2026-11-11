// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual guides/geometry/state on owned route workers, no game execution.
#define main retained_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace mobliz_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0;
void check(bool pass,const char* why){++cases;if(!pass){++errors;std::printf("FAIL %s\n",why);}}
void context(map_state::Flags& f){const auto* room=baked::submap_by_map_id(f.current_map_id);const auto* row=fixture::records.get(baked::str(room->name));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
Cell entrance(int map,int id){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==id)return{o.layer,o.tx,o.ty};std::abort();}
ffpr::ff6::story::Plan model(const map_state::Flags& f,Cell at){ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";return ffpr::ff6::story::plan(s);}
std::optional<std::pair<int,int>> next(map_state::Flags& f,Cell& at){
 for(int leg=0;leg<32;++leg){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);
  Planned result;ffpr::ff6::story::Plan guide;std::thread worker([&]{guide=model(f,at);if(guide.supported&&!guide.unknown&&!guide.finished&&!guide.targets.empty()){s.candidates=guide.targets;result=plan_snapshot(s);}});worker.join();
  std::printf("Mobliz map%d from%d,%d,%d goal%d/%d success%d next%d unknown%d supported%d candidates%zu worker%.3fms note%s reason%s\n",s.map_id,at.layer,at.x,at.y,result.goal.map_id,result.goal.entity_id,result.path.success,result.path.next_map_id,guide.unknown,guide.supported,guide.targets.size(),result.compute_ms,text::to_utf8(guide.note).c_str(),text::to_utf8(result.path.reason).c_str());
  if(!result.path.success||result.path.required_trap||result.path.npc_approach)return {};++legs;
  if(!result.path.next_map_id){for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return std::pair{result.goal.map_id,result.goal.entity_id};}
  const auto receipt=result.pending.incoming_transition;if(!receipt)return {};
  const bool waypoint=result.goal.entity_id<0&&receipt->destination==result.goal.map_id&&receipt->arrival==result.goal.cell;
  const bool transition=result.goal.map_id==receipt->map_id&&result.goal.entity_id==receipt->entity_id;
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);if(waypoint)return std::pair{result.goal.map_id,-1};if(transition)return std::pair{result.goal.map_id,result.goal.entity_id};
 }
 return {};
}
}
int main(){
 using namespace mobliz_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=90;context(f);Cell at=entrance(90,1);
 auto goal=next(f,at);check(goal==std::optional{std::pair{90,9}},"first wounded-soldier interaction is reachable");f.scenario[502]=1;
 for(const auto row:{std::array{503,504,505,8},std::array{506,507,508,7},std::array{509,510,511,6},std::array{512,513,514,5},std::array{515,516,-1,4}}){
  goal=next(f,at);check(goal==std::optional{std::pair{90,row[3]}},"each current animated letter has a valid trap-free path");if(!goal)break;
  f.scenario[size_t(row[0])]=1;goal=next(f,at);check(goal==std::optional{std::pair{88,10}},"read letter routes to the real postman across floors");if(!goal)break;
  auto p=model(f,at);check(p.targets.size()==1&&p.targets[0].instruction==L"Choose the first option.","native refusal/insufficient funds leaves the same payment objective");
  f.scenario[size_t(row[1])]=1;
  if(row[2]>=0){goal=next(f,at);check(goal&&(goal->first==86||goal->first==87||goal->first==91),"native shop-close reply has a reachable physical shopkeeper");if(!goal)break;f.scenario[size_t(row[2])]=1;}
 }
 if(f.scenario[516]==1){goal=next(f,at);check(goal==std::optional{std::pair{90,9}},"last payment returns to the soldier reward");f.scenario[517]=1;check(model(f,at).finished,"actual final517 completes the full letters");}
 // The later visit follows the completed Solitary Island raft voyage (306).
 // Otherwise the island guide correctly still owns the shared World2 map.
 f=floating_fixture::flags();f.scenario.fill(0);f.scenario[297]=f.scenario[306]=f.scenario[336]=f.scenario[353]=1;f.current_map_id=207;context(f);at=entrance(207,1);
 for(auto [receipt,map,id]:{std::tuple{316,207,57},std::tuple{317,212,46},std::tuple{318,212,66},std::tuple{319,212,44},std::tuple{320,212,43}}){
  goal=next(f,at);check(goal&&goal->first==map&&(goal->second==id||(receipt==316&&goal->second==58)),"later visit preserves and routes any missing native first-visit prerequisite");if(!goal)break;f.scenario[size_t(receipt)]=1;
 }
 if(f.scenario[320]==1){
  f.area[1]=1;goal=next(f,at);check(goal==std::optional{std::pair{207,6}},"gift's local guard routes a real village exit");
  // Native area change resets the actual area bank. This is an owned fixture
  // input, not a production state write or a timer/proximity receipt.
  f.current_map_id=2;at=entrance(2,4);f.area.fill(0);context(f);goal=next(f,at);check(goal==std::optional{std::pair{207,-1}},"world2 return reaches the real village PointIn");
  goal=next(f,at);check(goal&&goal->first==210&&goal->second>=7&&goal->second<=9,"dog cue is reached only after true re-entry");
  f.scenario[354]=1;goal=next(f,at);check(goal==std::optional{std::pair{213,1}},"actual hidden passage reaches the discovery region");
  f.scenario[355]=1;goal=next(f,at);check(goal==std::optional{std::pair{213,9}},"native discovery routes the final battle region");
  auto p=model(f,at);check(!p.finished&&p.finish_on_scenario==356,"battle entry is not final native completion");f.scenario[356]=1;check(model(f,at).finished,"only final356 completes the journey");
 }
 check(fixture::wrong_owner==0,"route/model work owns no native pointer or API");
 std::printf("Mobliz:%d assertions/%d real route legs/%d failures;representative fixture poses,not native movement/speech timing\n",cases,legs,errors);return errors?1:0;
}
