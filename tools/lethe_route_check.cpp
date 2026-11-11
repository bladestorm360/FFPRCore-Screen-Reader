// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real model/planner on controllable raft landings;fork scripts stay native.
#define main retained_lethe_floating_main
#include "floating_continent_route_check.cpp"
#undef main
namespace lethe_fixture {
using namespace floating_fixture;int cases=0,errors=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
Cell entry(int map){for(const auto& o:baked::grids(*baked::submap_by_map_id(map)).objects)if(o.type==baked::ObjType::PointIn&&o.a==1)return{o.layer,o.tx,o.ty};std::abort();}
void context(map_state::Flags& f){const auto* row=fixture::records.get(baked::name_of(*baked::submap_by_map_id(f.current_map_id)));if(const auto* a=row?row->get(L"area_id"):nullptr)f.current_area_id=int(a->n);}
ffpr::ff6::story::Plan advice(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){context(f);ffpr::ff6::story::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.guidance=policy;return ffpr::ff6::story::plan(s);}
bool route(map_state::Flags f,Cell at,int expected){context(f);auto p=advice(f,at);if(p.unknown||p.targets.size()!=1||p.targets[0].goal.map_id!=f.current_map_id||p.targets[0].goal.entity_id!=expected||p.finish_on_scenario!=50)return false;
 RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.locale=L"en";s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.candidates=std::move(p.targets);obstacles(s);Planned got;std::thread worker([&]{got=plan_snapshot(s);});worker.join();
 std::printf("Lethe map%d at%d,%d,%d ->%d success%d next%d trap%d %s %s\n",f.current_map_id,at.layer,at.x,at.y,expected,got.path.success,got.path.next_map_id,got.path.required_trap,text::to_utf8(got.path.reason).c_str(),text::to_utf8(got.path.full_description).c_str());return got.path.success&&!got.path.required_trap&&!got.path.next_map_id&&got.goal.entity_id==expected;
}
}
int main(){using namespace lethe_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff6/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||!json::parse(read("data/ff6/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 auto f=floating_fixture::flags();f.scenario.fill(0);f.scenario[47]=1;f.current_map_id=345;Cell at=entry(345);
 check(route(f,at,12),"actual river entrance reaches the real initial raft trigger");
 f.current_map_id=346;f.local.fill(0);f.local[1]=1;f.scenario[48]=1;at={1,14,13};
 check(route(f,at,1),"full0063_1 landing20,22 minus native crop6,9 reaches the actual second raft");
 check(route(f,entry(346),1),"ordinary re-entry to the first rest floor also reaches its raft");
 f.current_map_id=348;f.local.fill(0);f.local[1]=1;at={1,6,13};
 check(route(f,at,1),"full0064_2 landing6,13 after tutorial reaches the actual final raft");
 check(route(f,entry(348),1),"ordinary final-rest entrance reaches its native raft trigger");
 auto p=advice(f,at);check(p.targets.front().instruction.find(L"Banon")!=std::wstring::npos&&p.finish_on_scenario==50,"final instruction retains native protect-Banon objective and full50 readiness");
 check(advice(f,at,{false,true}).targets.empty(),"Navigation off independently withholds the raft route");check(!advice(f,at,{true,false}).targets.empty(),"Other guidance off preserves the navigation choice puzzle");
 f.scenario[50].reset();check(advice(f,at).unknown,"unknown final50 cannot certify a complete journey");f.scenario[50]=1;check(advice(f,at).finished,"actual full50 is the only persistent completion");f.scenario[50]=0;f.scenario[47]=0;check(!advice(f,at).supported,"unrecruited Banon cannot expose an early raft solution");f.scenario[47].reset();check(advice(f,at).unknown,"unknown eligibility never becomes an early route");
 check(!ffpr::ff6::story::supported_map(347),"movie-only river route is owned by its actual choice UI rather than a free foot route");check(fixture::wrong_owner==0,"all model/planner work stays on owned state");std::printf("Lethe:%d assertions/%d failures\n",cases,errors);return errors||fixture::wrong_owner?1:0;
}
