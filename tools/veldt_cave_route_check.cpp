// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Complete real dog/rock-switch/rescue trigger for both native outcomes.
#define main retained_veldt_floating_main
#include "floating_continent_route_check.cpp"
#undef main
#include "ordinary_interaction_fixtures.h"
namespace veldt_fixture {
using namespace floating_fixture;int cases=0,errors=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map,int point=1){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==point)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan model(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool journey(map_state::Flags& f,Cell& at,int expected){
 for(int leg=0;leg<20;++leg){context(f);auto advice=model(f,at);if(advice.unknown||advice.finished||advice.targets.empty())return false;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.candidates=std::move(advice.targets);obstacles(s);Planned p;std::thread w([&]{p=plan_snapshot(s);});w.join();
  std::printf("Veldt branch%d map%d from%d,%d,%d goal%d/%d success%d next%d trap%d reason%s error%s %s\n",f.scenario[298].value_or(-1),f.current_map_id,at.layer,at.x,at.y,p.goal.map_id,p.goal.entity_id,p.path.success,p.path.next_map_id,p.path.required_trap,text::to_utf8(p.path.reason).c_str(),text::to_utf8(p.path.error).c_str(),text::to_utf8(p.path.full_description).c_str());
  if(!p.path.success||p.path.required_trap||p.goal.entity_id!=expected)return false;
  if(!p.path.next_map_id){for(const auto& phase:p.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return true;}
  const auto receipt=p.pending.incoming_transition;if(!receipt)return false;f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
  if(receipt->map_id==p.goal.map_id&&receipt->entity_id==p.goal.entity_id)return true;
 }return false;
}
bool repeat_return(map_state::Flags f){f.current_map_id=492;context(f);const auto at=entry(492),goal=entry(491,3);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=492;s.submap=baked::submap_by_map_id(492);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.goal.map_id=491;s.goal.cell=goal;s.goal.allow_scripted_crossings=true;s.kind=TargetKind::Spot;obstacles(s);Planned p;std::thread w([&]{p=plan_snapshot(s);});w.join();std::printf("Veldt repeat branch%d done372=%d/373=%d success%d next%d reason%s\n",f.scenario[298].value_or(-1),f.scenario[372].value_or(-1),f.scenario[373].value_or(-1),p.path.success,p.path.next_map_id,text::to_utf8(p.path.reason).c_str());return p.path.success;}
}
int main(){using namespace veldt_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes)||
  !append_ordinary_interactions(fixture::guide_recipes,6,L"story_guides"))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;const auto* chamber=baked::submap_by_map_id(492);const baked::Object* arc=nullptr,*event=nullptr;for(const auto& o:baked::grids(*chamber).objects)if(o.entity_id()==4){if(o.type==baked::ObjType::Warp)arc=&o;else if(o.type==baked::ObjType::Event)event=&o;}if(!arc||!event)return 2;check(baked::world_exit(*arc)&&arc->a==491&&arc->b==3&&entry(491,3).layer==2,"repeat passage retains the actual native491 PointIn3 Upper landing");
 for(int shadow:{0,1}){auto f=floating_fixture::flags();f.scenario[286]=0;f.scenario[297]=1;f.scenario[298]=shadow;f.current_map_id=488;Cell at=entry(488);const int done=shadow?372:373,other=shadow?373:372;
  for(int mode=0;mode<3;++mode){auto g=f;g.scenario[size_t(done)]=mode==0?std::optional<int>{}:std::optional<int>(mode==2);map_state::Resolver state(g,fixture::records);check(state.consumed(*chamber,*arc)==(mode!=2),"missing/incomplete selected rescue receipt cannot certify the ordinary repeat edge");check(!state.consumed(*chamber,*event),"the actual rescue trigger stays physically available before completion");}
  check(!repeat_return(f),"the one-time rescue cannot be used as free travel to its intermediate floor");{auto g=f;g.scenario[size_t(done)]=1;check(repeat_return(g),"actual completed rescue enables the real repeat cross-floor return");}
  auto p=model(f,at);check(p.supported&&!p.finished&&!p.unknown&&p.targets.size()==1&&p.targets[0].goal.entity_id==1,"both actual story branches start with Interceptor");check(model(f,at,{false,true}).targets.empty(),"Navigation off withholds the complete cave route");check(!model(f,at,{true,false}).targets.empty(),"Other guidance is independent of this navigation puzzle");
  check(journey(f,at,1),"actual cave entrance reaches the native Interceptor event");f.scenario[371]=1;
  check(journey(f,at,21),"post-Interceptor route reaches the real rock switch across the cave floors");f.scenario[533]=1;
  {const auto* room=baked::submap_by_map_id(491);map_state::Resolver state(f,fixture::records);for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Entity&&o.entity_id()==21)check(!state.active(*room,o),"actual533 retires the operated switch");}
  p=model(f,at);check(p.targets.size()==1&&p.targets[0].goal.map_id==492&&p.targets[0].goal.entity_id==4&&p.finish_on_scenario==done,"real branch-specific rescue trigger arms only its final inn receipt");f.scenario[926]=1;check(!model(f,at).finished,"ordinary Thamasa visit926 cannot finish the rescue");f.scenario[other]=1;check(!model(f,at).finished,"the other rescue outcome cannot finish this branch");f.scenario[other]=0;
  check(journey(f,at,4),"actual rock-switch position reaches the correct rescue/battle trigger without requiring the optional trapped chest");
  f.current_map_id=311;f.scenario[size_t(done)]=1;check(model(f,entry(311)).finished,"only the actual completed branch receipt finishes on the inn return");
  f.scenario[size_t(done)]=0;for(int map:{488,489,490,491}){f.current_map_id=map;Cell here=entry(map);check(journey(f,here,4),"each playable cave-floor entry can resume after the real rock receipt");}
 }
 auto f=floating_fixture::flags();f.current_map_id=488;f.scenario[297]=1;f.scenario[298].reset();check(model(f,entry(488)).unknown,"unknown native outcome cannot select Shadow or Relm");f.scenario[298]=0;f.scenario[297]=0;check(!model(f,entry(488)).supported,"earlier chapter does not acquire the later cave rescue guide");check(fixture::wrong_owner==0,"route work uses owned data without native pointer access");
 std::printf("Veldt cave:%d assertions/%d failures\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
