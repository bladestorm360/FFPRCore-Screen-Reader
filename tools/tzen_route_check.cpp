// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Full Tzen start/child/escape, separate from the accepted Thamasa guide.
#define main retained_tzen_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace tzen_fixture {
using namespace floating_fixture;
int errors=0,cases=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool journey(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<24;++leg){context(f);auto advice=model(f,at);if(advice.unknown||advice.finished||advice.targets.empty())return false;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.candidates=std::move(advice.targets);obstacles(s);Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();
  std::printf("Tzen map%d from%d,%d,%d goal%d/%d success%d next%d trap%d reason%s error%s %s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap||!(p.goal.entity_id==expected||(expected==54&&p.goal.entity_id==55)))return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(receipt->map_id==p.goal.map_id&&receipt->entity_id==p.goal.entity_id)return true;
 }return false;
}
}
int main(){using namespace tzen_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;auto f=floating_fixture::flags();f.scenario.fill(0);f.scenario[297]=1;f.current_map_id=201;context(f);Cell at=entry(201);
 auto advice=model(f,at);check(advice.supported&&!advice.unknown&&advice.targets.size()==2&&advice.puzzle_types==1,"actual late Tzen entry offers the two quake markers as Navigation");
 check(journey(f,at,54),"actual town entrance reaches the native quake trigger");f.scenario[309]=1;
 advice=model(f,at);check(advice.targets.size()==1&&advice.targets[0].goal.entity_id==52&&advice.targets[0].instruction==L"Start the rescue. Find the child and leave the house within six minutes.","six-minute information precedes the actual timer-start marker");
 check(journey(f,at,52),"post-quake player position reaches the actual native start marker");f.scenario[310]=1;
 advice=model(f,at);check(advice.targets[0].goal.map_id==434&&advice.targets[0].goal.entity_id==9&&!advice.finished,"actual310 starts the child objective,not completion");
 check(model(f,at,{false,true}).targets.empty(),"Navigation off withholds the timed route");check(!model(f,at,{true,false}).targets.empty(),"the complete navigation rescue is independent of Other guidance");
 check(journey(f,at,9),"full timed town/first-floor/basement journey reaches the actual child");
 f.scenario[311]=1;advice=model(f,at);check(advice.targets[0].goal.map_id==433&&advice.targets[0].goal.entity_id==0&&advice.finish_on_scenario==312&&!advice.finished,"child311 advances to the real house exit and arms only final312");
 check(journey(f,at,0),"actual child pickup position has a complete return to the correct native house exit");
 f.scenario[312]=1;check(model(f,at).finished,"actual full rescue312 completes the whole guide");
 f.scenario[312]=0;f.scenario[311]=0;f.current_map_id=201;at=entry(201,7);advice=model(f,at);check(advice.targets[0].goal.entity_id==9,"leaving without the child neither restarts the timer nor completes the rescue");check(journey(f,at,9),"retreat without the child can resume the full actual route");
 for(int point:{1,2}){f.current_map_id=433;at=entry(433,point);check(journey(f,at,9),"each actual first-floor arrival retains a valid child route");}
 f.current_map_id=201;f.scenario[297]=0;check(!model(f,entry(201)).supported,"late Tzen guide is not selected before the real ruined-world episode");
 check(ffpr::ff6::story::supported_map(201)&&ffpr::ff6::story::supported_map(433)&&ffpr::ff6::story::supported_map(434),"all three native maps participate in the existing guide lifecycle");check(fixture::wrong_owner==0,"all model/route work uses owned state");
 std::printf("Tzen:%d assertions/%d failures;offline routes,not a live timed run\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
