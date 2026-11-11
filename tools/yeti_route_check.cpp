// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual complete pit arrivals, production routes and native Mog requirement.
#define main retained_yeti_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace yeti_fixture {
using namespace floating_fixture;
int errors=0,cases=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
Planned route(map_state::Flags f,Cell at,int map=0,int id=-1){
 context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};obstacles(s);
 if(map){s.goal.map_id=map;s.goal.entity_id=id;s.kind=TargetKind::Interact;for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type!=baked::ObjType::PointIn&&o.type!=baked::ObjType::Warp&&o.entity_id()==id){s.goal.cell={o.layer,o.tx,o.ty};s.goal.raw_name=baked::object_identifier(o);break;}}
 else {auto advice=model(f,at);if(!advice.supported||advice.unknown||advice.finished)return {};s.candidates=std::move(advice.targets);}
 Planned p;std::thread worker([&]{p=plan_snapshot(s);});worker.join();return p;
}
bool journey(map_state::Flags& f,Cell& at,int id){
 for(int leg=0;leg<32;++leg){auto p=route(f,at);std::printf("Yeti map%d from%d,%d,%d to%d/%d success%d next%d trap%d reason%s\n",f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str());
  if(!p.path.success||p.goal.entity_id!=id)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition?p.pending.incoming_transition:p.pending.incoming_trap;if(!receipt)return false;
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(receipt->map_id==p.goal.map_id&&receipt->entity_id==p.goal.entity_id)return true;
 }return false;
}
Cell operate(int source,int id,int destination){const auto* sm=baked::submap_by_map_id(source);for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==id&&o.a==destination){check((o.d&255)==2,"Yeti switch is a real manual interaction,not a step-on trigger");return entry(destination,o.b);}std::abort();}
}
int main(){using namespace yeti_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;auto f=floating_fixture::flags();f.scenario.fill(0);f.scenario[297]=f.scenario[353]=1;f.current_map_id=20;context(f);Cell at=entry(20);f.party_mog=false;
 check(model(f,at).targets[0].goal.entity_id==1,"actual frozen-esper battle precedes the cave opening");
 check(journey(f,at,1),"native late-story temple arrival reaches the actual prerequisite battle");f.scenario[395]=1;
 auto advice=model(f,at);check(advice.targets[0].goal.entity_id==26&&advice.puzzle_types==1,"native395 opens the real first-option jump as Navigation");check(journey(f,at,26),"guided entrance reaches the actual jump control");
 // Simulate the completed native entry using its reviewed graph PointIn.
 const auto* temple=baked::submap_by_map_id(20);for(const auto& o:baked::grids(*temple).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==26){at=entry(493,o.b);break;}f.current_map_id=493;f.scenario[960]=1;
 check(journey(f,at,13),"complete first/second/third-floor journey reaches the correct switch through actual passages");
 check(model(f,at,{false,true}).targets.empty(),"Navigation off does not skip the actual fall switch to a later choice");
 at=operate(495,13,496);f.current_map_id=496;advice=model(f,at);check(advice.targets[0].goal.entity_id==6&&advice.puzzle_types==2,"full correct-switch landing selects the real magicite choice and battle as Other");
 check(model(f,at,{true,false}).targets.empty(),"Other off withholds the unsolved magicite choice without advancing to recruitment");
 check(journey(f,at,6),"actual final chamber landing reaches the skull interaction");
 f.scenario[397]=1;at={1,12,13};advice=model(f,at);check(advice.note==L"Bring Mog in the active party before talking to the yeti.","actual battle397 is not sufficient to recruit without active Mog");
 check(route(f,at,496,1).path.success,"ordinary conversation remains reachable without Mog;only recruitment is gated");
 f.party_mog.reset();check(model(f,at).unknown,"unknown Mog state cannot certify recruitment");f.party_mog=true;advice=model(f,at);check(advice.finish_on_scenario==398&&advice.targets[0].goal.entity_id==1,"active Mog advances to the actual recruitment interaction and native398");
 check(journey(f,at,1),"post-battle native pose reaches the initializer-positioned yeti");f.scenario[398]=1;check(model(f,at).finished,"only actual completed recruitment398 finishes the whole guide");
 const auto wrong=operate(495,15,494);f.scenario[398]=0;f.scenario[397]=0;f.current_map_id=494;at=wrong;check(journey(f,at,13),"wrong-switch return has a valid complete recovery to the correct switch");
 for(int point:{1,2,5,6}){f.current_map_id=494;at=entry(494,point);check(journey(f,at,13),"each original automatic-pit landing retains a complete recovery on its actual native layer");}
 f.current_map_id=20;f.scenario[353]=0;check(!model(f,entry(20)).supported,"new late-story guide does not replace earlier Narshe guidance");
 check(fixture::wrong_owner==0,"all route/model calculations use owned state");std::printf("Yeti:%d assertions/%d failures;offline fixture,not game response timing\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
