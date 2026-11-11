// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/mechanics_model.cpp"
namespace gohn_routes {
using namespace field;using namespace baked_nav;
int cases=0,errors=0,legs=0;
void check(bool v,const char* why){++cases;if(!v){++errors;if(errors<16)std::printf("FAIL %s\n",why);}}
map_state::Flags flags(int mask){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.current_map_id=287;f.current_area_id=37;f.scenario[59]=1;constexpr int bits[]{60,365,366,367,368};for(int i=0;i<5;++i)f.scenario[bits[i]]=(mask>>i)&1;return f;}
ff5::library::Plan advice(int mask,Cell at){ff5::mechanics::Snapshot s;s.map=287;s.flags=flags(mask);s.player=at;s.locale=L"en";return ff5::mechanics::plan(s,fixture::guide_recipes);}
Planned route(int mask,Cell at,const std::vector<GuideTarget>& targets){RouteSnapshot s(map_state::Resolver(flags(mask),fixture::records));s.map_id=287;s.submap=baked::submap_by_map_id(287);s.layer=at.layer;s.position=world_of_cell(*s.submap,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates=targets;
 for(const auto& source:baked::grids(*s.submap).objects){if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!s.state.active(*s.submap,source,true))continue;const auto o=s.state.effective(*s.submap,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
 ++legs;return plan_snapshot(s);}
int verify(){
 int states=0;
 for(int mask=0;mask<32;++mask){const bool start=mask&1,second=mask&2,east=mask&4,west=mask&8,last=mask&16;if((!start&&mask)||(!second&&(mask&28))||(last&&!east&&!west))continue;++states;
  const auto p=advice(mask,{1,36,49});check(p.supported&&!p.unknown&&!p.finished&&!p.targets.empty(),"every native reachable search state has complete guidance");if(p.targets.empty())return 2;
  const int expected=!start?5:!second?6:last?10:east||west?17:0;
  check(expected?p.targets.size()==1&&p.targets[0].goal.entity_id==expected:p.targets.size()==5,"initializer order and alternative middle sightings select real controls");
  for(Cell at:{Cell{1,36,49},Cell{1,36,45},Cell{1,35,37},Cell{1,49,45},Cell{1,49,46},Cell{1,22,32},Cell{1,22,33},Cell{1,22,34},Cell{1,35,34}}){
   const auto r=route(mask,at,p.targets);check(r.path.success&&!r.path.npc_approach,"actual strict route reaches an eligible sighting from entry/native scene returns");if(!r.path.success){std::printf("mask%d at%d,%d: %ls/%ls\n",mask,at.x,at.y,r.path.error.c_str(),r.path.reason.c_str());return 3;}
   const int selected=r.path.guidance_index;check(selected>=0&&size_t(selected)<p.targets.size(),"actual planner reports the selected physical alternative");if(selected<0||size_t(selected)>=p.targets.size())return 4;
   const auto& goal=p.targets[selected].goal;check(!r.path.walk_phases.empty()&&r.path.walk_phases.back().cells.back()==goal.cell,"guide reaches the selected real foot trigger");
  }
 }
 check(states==9,"all nine lawful persistent search states were exercised");
 for(int middle:{4,8,12}){const int mask=3|middle;const auto p=advice(mask,{1,35,37});check(p.targets.size()==1&&p.targets[0].goal.entity_id==17,"either middle sighting suffices;the guide does not require an extra inactive sighting");}
 auto p=advice(31,{1,35,34});check(p.targets.size()==1&&p.targets[0].goal.entity_id==10&&!p.finish_map,"complete city search includes the actual collapse trigger rather than stopping early");
 GuideTarget destination;destination.kind=TargetKind::Spot;destination.goal.map_id=288;destination.goal.cell={1,17,10};const auto crossing=route(31,{1,35,34},{destination});check(crossing.path.success,"ordinary cross-floor routing reaches the actual underground scene continuation");
 check(crossing.pending.incoming_transition&&crossing.pending.incoming_transition->destination==288&&crossing.pending.incoming_transition->arrival==Cell{1,17,10},"full callback landing replaces its cinematic starting point without opening terrain");
 ff5::mechanics::Snapshot s;s.map=288;s.flags=flags(31);s.flags.scenario[61]=1;s.player={1,17,10};s.locale=L"en";check(ff5::mechanics::plan(s,fixture::guide_recipes).finished,"only full native61 at the underground landing finishes the guide");
 s.map=287;s.flags.scenario[61]=0;s.policy={false,true};check(ff5::mechanics::plan(s,fixture::guide_recipes).targets.empty(),"Navigation toggle suppresses this independent navigation solution");s.policy={true,false};check(!ff5::mechanics::plan(s,fixture::guide_recipes).targets.empty(),"Other toggle preserves city navigation");s.flags.scenario[368].reset();check(ff5::mechanics::plan(s,fixture::guide_recipes).unknown,"unknown final sighting never fabricates the collapse route");
 std::printf("Gohn production:%d assertions/%d physical legs/%d native states/%d failures\n",cases,legs,states,errors);return errors?5:fixture::wrong_owner?6:0;
}
}
int main(){using namespace gohn_routes;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;int result=2;std::thread worker([&]{result=verify();});worker.join();return result;}
