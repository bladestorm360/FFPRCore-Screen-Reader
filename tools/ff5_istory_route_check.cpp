// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_istory_floating_main
#include "floating_continent_route_check.cpp"
#undef main
#include "../src/games/ff5/mechanics_model.cpp"
namespace istory_fixture {
using namespace floating_fixture;
int cases=0,errors=0,legs=0;
void check(bool ok,const char* why){++cases;if(!ok){++errors;std::printf("FAIL %s\n",why);}}
ff5::library::Plan advice(const map_state::Flags& f,Cell at,field::puzzle_policy::Policy policy={},std::wstring id={}){ff5::mechanics::Snapshot s;s.map=f.current_map_id;s.flags=f;s.player=at;s.locale=L"en";s.policy=policy;s.guide_id=std::move(id);return ff5::mechanics::plan(s,fixture::guide_recipes);}
bool reach(map_state::Flags& f,Cell& at,int expected){
 std::wstring id;
 for(int leg=0;leg<16;++leg){const auto p=advice(f,at,{},id);if(!p.supported||p.unknown||p.targets.size()!=1||(p.targets.front().goal.entity_id!=expected&&p.targets.front().goal.entity_id!=11))return false;id=p.guide_id;
  RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=f.current_map_id;s.submap=baked::submap_by_map_id(s.map_id);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates=p.targets;obstacles(s);Planned got;std::thread worker([&]{got=plan_snapshot(s);});worker.join();++legs;
  if(!got.path.success||got.path.required_trap){std::printf("Istory map%d from%d,%d,%d target%d success%d trap%d %s %s\n",s.map_id,at.layer,at.x,at.y,got.goal.entity_id,got.path.success,got.path.required_trap,text::to_utf8(got.path.reason).c_str(),text::to_utf8(got.path.full_description).c_str());return false;}
  if(!got.path.next_map_id){for(const auto& phase:got.path.walk_phases)if(!phase.cells.empty())at=phase.cells.back();if(got.goal.entity_id==expected)return true;
   // Exact native GotoMap11 -> world1/point32. Shift+G then routes back
   // through the real town entrance;no teleport or invented landing.
   if(s.map_id!=118||got.goal.entity_id!=11||at!=Cell{2,51,11})return false;
   const auto* world=baked::submap_by_map_id(1);const baked::Object* landing=nullptr;for(const auto& o:baked::grids(*world).objects)if(o.type==baked::ObjType::PointIn&&o.a==32)landing=&o;if(!landing)return false;
   // The production player_cell and cross-map planner normalize every world
   // landing to Ground;the source PointIn's drawing layer is not player mode.
   f.current_map_id=1;at={1,landing->tx,landing->ty};continue;
  }
  if(!got.pending.incoming_transition)return false;f.current_map_id=got.pending.incoming_transition->destination;at=got.pending.incoming_transition->arrival;
 }return false;
}
}
int main(){using namespace istory_fixture;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 std::vector<Cell> entries;for(const auto& o:baked::grids(*baked::submap_by_map_id(118)).objects)if(o.type==baked::ObjType::PointIn)entries.push_back({o.layer,o.tx,o.ty});check(!entries.empty(),"actual town entrances exist");
 for(const auto entry:entries)for(int mask=0;mask<8;++mask){auto f=flags();f.scenario.fill(0);f.current_map_id=118;f.current_area_id=29;for(int i=0;i<3;++i)f.scenario[size_t(296+i)]=(mask>>i)&1;Cell at=entry;bool success=true;
  for(int i=0;i<3;++i)if(f.scenario[size_t(296+i)]==0){const int id=i==0?31:i==1?28:33;if(!reach(f,at,id)){success=false;break;}f.scenario[size_t(296+i)]=1;}
  if(success)success=reach(f,at,26);check(success,"every saved subset completes the four real contacts from each actual entrance");
  f.scenario[299]=1;check(advice(f,at).finished,"only full final299 finishes after its entire native scene");
 }
 auto f=flags();f.scenario.fill(0);f.current_map_id=118;f.current_area_id=29;
 check(advice(f,entries.front(),{false,true}).targets.empty(),"Navigation off independently blocks the navigation puzzle");check(!advice(f,entries.front(),{true,false}).targets.empty(),"Other off preserves the flower solution");
 f.scenario[296].reset();const auto unknown=advice(f,entries.front());check(unknown.unknown&&unknown.targets.empty(),"unknown first receipt cannot expose a later contact");f.scenario[296]=1;f.scenario[297].reset();check(advice(f,entries.front()).unknown,"unknown middle receipt cannot skip its contact");f.scenario[299].reset();check(advice(f,entries.front()).unknown,"missing final receipt cannot certify completion");
 f.scenario.fill(0);f.current_map_id=1;check(!advice(f,{1,0,0}).supported,"world map never starts an unowned Istory guide");check(!advice(f,{1,0,0},{},L"unrelated").supported,"another guide cannot borrow Istory transit ownership");check(advice(f,{1,0,0},{},L"istory_flowers").targets.front().goal.entity_id==31,"only the owned Istory journey can guide the town re-entry");
 check(fixture::wrong_owner==0,"all rule selection and physical routing stay on owned data");std::printf("Istory:%d assertions/%d route legs/%d failures\n",cases,legs,errors);return errors||fixture::wrong_owner?1:0;
}
