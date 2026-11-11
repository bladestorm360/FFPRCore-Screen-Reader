// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Full real-bake tour through every native soldier, with real current gates.
#define FFPR_GERAD_FIXTURE_ONLY
#include "gerad_route_check.cpp"
namespace palace_route {
using namespace gerad_fixture;
constexpr int soldiers[][3]={{230,165,24},{231,165,25},{232,166,44},{233,166,45},{234,166,46},{235,166,47},
 {236,168,15},{237,168,14},{238,168,13},{239,168,11},{240,168,10},{241,170,1},{242,173,9},{243,173,7},{244,174,8},
 {245,178,20},{246,178,19},{247,178,18},{248,178,17},{249,178,15},{250,168,12},{251,169,4},{252,179,9},{253,165,26}};
int legs=0,cases=0,failures=0;double steps=0;
void check(bool ok,const char* name){++cases;if(!ok){++failures;std::printf("FAIL %s\n",name);}}
std::optional<std::pair<int,int>> follow(map_state::Flags& f,Cell& at){
 for(int leg=0;leg<32;++leg){context(f);RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;obstacles(s);
  Planned result;ffpr::ff6::story::Plan guide;
  std::thread worker([&]{guide=model(f,at);if(guide.supported&&!guide.finished&&!guide.unknown&&!guide.targets.empty()){s.candidates=guide.targets;result=plan_snapshot(s);}});worker.join();
  std::printf("Palace route map%d from%d,%d,%d target%d/%d success%d next%d unknown%d note%s reason%s\n",s.map_id,at.layer,at.x,at.y,result.goal.map_id,result.goal.entity_id,result.path.success,result.path.next_map_id,guide.unknown,text::to_utf8(guide.note).c_str(),text::to_utf8(result.path.reason).c_str());
  if(!result.path.success||result.path.required_trap||result.path.npc_approach)return {};
  ++legs;for(const auto& phase:result.path.walk_phases)steps+=phase.cells.empty()?0:phase.cells.size()-1;
  if(!result.path.next_map_id){for(const auto& phase:result.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();return std::pair{result.goal.map_id,result.goal.entity_id};}
  const auto receipt=result.pending.incoming_transition;if(!receipt)return {};
  f.current_map_id=receipt->destination;at=receipt->arrival;f.local.fill(0);
 }
 return {};
}
}
int main(){
 using namespace palace_route;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* name){std::ifstream f(name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.current_map_id=165;f.scenario[226]=1;context(f);Cell at=entrance(165,1);
 auto goal=follow(f,at);check(goal==std::optional{std::pair{165,27}},"invited palace entry reaches the real escort region");
 f.scenario[227]=f.scenario[228]=1;goal=follow(f,at);check(goal==std::optional{std::pair{176,15}},"escort receipt reaches the real Emperor start");
 // The native start scene returns playable input on this actual floor. The
 // tour always recomputes against the resulting cell, not a fixed baked route.
 f.scenario[229]=1;
 for(int n=0;n<24;++n){goal=follow(f,at);if(!goal){check(false,"every unvisited soldier remains reachable through actual floor exits");break;}
  int flag=-1;for(const auto& row:soldiers)if(row[1]==goal->first&&row[2]==goal->second)flag=row[0];
  check(flag>=230&&flag<=253&&f.scenario[size_t(flag)]==0,"nearest-guide alternative is one still-unvisited native soldier");
  if(flag<230||flag>253)break;f.scenario[size_t(flag)]=1;
 }
 bool all=true;for(int i=230;i<=253;++i)all&=f.scenario[size_t(i)]==1;check(all,"full journey visits each of the24 distinct native receipts");
 goal=follow(f,at);check(goal==std::optional{std::pair{176,17}},"only the complete tour guides the real finish-tour region");
 // Timer expiry also legitimately ends an unfinished tour. It must advance to
 // current banquet/break choices, never pretend the missed soldiers were met.
 f.scenario[255]=1;f.scenario[240]=0;f.current_map_id=177;at=entrance(177,1);context(f);
 auto p=model(f,at);check(!p.finished&&p.targets.size()==4&&p.puzzle_types==2,"expired tour offers actual optional-break soldiers without fabricating missed receipt240");
 goal=follow(f,at);check(goal&&goal->first==177&&(goal->second==3||goal->second==4||goal->second==6||goal->second==7),"break battle reaches an actual visible conversation");
 f.scenario[256]=1;goal=follow(f,at);check(goal==std::optional{std::pair{177,13}},"actual break receipt leads to native banquet-resume region");
 f.scenario[257]=1;at=entrance(177,3);goal=follow(f,at);check(goal==std::optional{std::pair{166,41}},"full banquet return reaches the real reward region");
 p=model(f,at);check(!p.finished&&p.finish_on_scenario==258,"banquet257 alone cannot finish before the final reward scene");
 f.scenario[258]=1;p=model(f,at);check(p.finished&&p.targets.empty(),"final ready258 completes the guide");
 f.scenario[258]=0;f.scenario[257]=0;f.scenario[256].reset();p=model(f,at);check(p.unknown&&p.targets.empty(),"unknown break result cannot skip its actual operation");
 check(fixture::wrong_owner==0,"native state/worker ownership remains intact");
 std::printf("Palace route:%d assertions/%d actual legs/%.0f cardinal cells/%d failures; fixture not native timer or audible speed\n",cases,legs,steps,palace_route::failures);return palace_route::failures?1:0;
}
