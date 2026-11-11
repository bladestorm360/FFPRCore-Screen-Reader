// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual full mountain routes and the separate mandatory combat instruction.
#define main retained_kolts_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace kolts_fixture {
using namespace floating_fixture;
int errors=0,cases=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool journey(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<24;++leg){context(f);auto advice=model(f,at);if(advice.unknown||advice.finished||advice.targets.empty())return false;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.candidates=std::move(advice.targets);obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Kolts map%d from%d,%d,%d goal%d/%d success%d next%d trap%d reason%s error%s %s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap||!(p.goal.entity_id==expected||(expected==1&&p.goal.entity_id==10)))return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
 }return false;
}
}
int main(){using namespace kolts_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=334;context(f);Cell at=entry(334);
 auto advice=model(f,at);check(advice.supported&&!advice.unknown&&advice.targets.size()==2&&advice.puzzle_types==1,"real mountain entry offers both sighting markers as Navigation");
 check(model(f,at,{false,true}).targets.empty(),"Navigation off does not skip the unsolved sighting");check(model(f,at,{true,false}).targets.size()==2,"Other off preserves the navigation stage");
 check(journey(f,at,1),"full mountain entrance-to-sighting route crosses the actual connected floors");
 f.scenario[40]=1;advice=model(f,at);check(advice.targets.size()==1&&advice.targets[0].goal.entity_id==2&&advice.puzzle_types==2&&advice.finish_on_scenario==41&&!advice.finished,"40 advances to the mandatory Other combat operation and only full41 finishes");
 check(advice.targets.size()==1&&advice.targets[0].instruction.find(L"When Sabin fights alone, use Raging Fist from Blitz: Left, Right, Left, then Confirm.")!=std::wstring::npos,"mandatory Blitz solution is supplied before entering battle");
 check(model(f,at,{true,false}).targets.empty(),"Other off withholds combat solution without inventing completion");check(model(f,at,{false,true}).targets.size()==1,"Other combat instruction is independent of Navigation once40 is actual");
 // Native sighting moves the player seven East then seven West,so the final
 // approach position is retained;the boss itself moves to Ground23,32.
 check(journey(f,at,2),"post-sighting player position reaches the actual relocated Vargas");
 {const auto* sm=baked::submap_by_map_id(343);map_state::Resolver state(f,fixture::records);const baked::Object* boss=nullptr;for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::NPC&&o.entity_id()==2)boss=&o;
  check(boss&&state.effective(*sm,*boss).layer==1&&state.effective(*sm,*boss).tx==23&&state.effective(*sm,*boss).ty==32,"actual40 projects native Ground23,32 rather than off-map Upper home");
  auto early=f;early.scenario[40]=0;map_state::Resolver before(early,fixture::records);check(boss&&before.effective(*sm,*boss).layer==boss->layer&&before.effective(*sm,*boss).tx==boss->tx,"native initial off-map pose is preserved before the sighting");
  json::Value invalid;json::parse(R"({"entry":0,"code":[[10,0,2,1,23,32,3],[11,0],[6]]})",invalid);check(!map_state::project(invalid,f).known,"invalid fourth layer cannot grant a projected placement");}
 for(int point:{1,2}){f.current_map_id=343;at=entry(343,point);check(journey(f,at,2),"each actual summit arrival after40 reaches the reloaded boss");}
 f.scenario[40].reset();check(model(f,at).unknown,"unknown sighting receipt cannot certify progress");f.scenario[40]=1;f.scenario[41].reset();check(model(f,at).unknown,"unknown final receipt cannot certify completion");f.scenario[41]=1;check(model(f,at).finished,"actual full41 completes the entire episode");
 f.scenario[41]=0;f.scenario[297]=1;check(!model(f,at).supported,"earlier Vargas guide is not offered for the ruined-world episode");
 for(int map=334;map<=343;++map)check(ffpr::ff6::story::supported_map(map),"every connected mountain map participates in the native guide lifecycle");
 check(fixture::wrong_owner==0,"model and route work use owned state");std::printf("Mt. Kolts:%d assertions/%d failures\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
