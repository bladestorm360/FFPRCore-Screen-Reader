// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_desert_policy_main
#include "ff5_desert_policy_check.cpp"
#undef main
#include "../src/games/ff5/mechanics_model.cpp"
#include "../src/core/field/puzzle_entry_notice.h"
namespace desert_routes {
using namespace desert_policy;
int assertions=0,errors=0,legs=0;
void check(bool v,const char* why){++assertions;if(!v){++errors;if(errors<12)std::printf("FAIL %s\n",why);}}
ff5::library::Plan advice(map_state::Flags f,Cell at,field::puzzle_policy::Policy policy={}){ff5::mechanics::Snapshot s;s.flags=f;s.map=286;s.player=at;s.locale=L"en";s.policy=policy;return ff5::mechanics::plan(s,fixture::guide_recipes);}
Planned route(map_state::Flags f,Cell at,const field::baked_nav::GuideTarget& target){RouteSnapshot s(map_state::Resolver(f,fixture::records));s.map_id=286;s.submap=room;s.layer=at.layer;s.position=world_of_cell(*room,at);s.vehicle={true,1,baked::transport(1)->type,{}};s.vehicle_shortcuts=false;s.locale=L"en";s.candidates={target};
 for(const auto& source:baked::grids(*room).objects){if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||source.type==baked::ObjType::CurrentArea||!s.state.active(*room,source,true))continue;const auto o=s.state.effective(*room,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.trigger=baked_route::step_event(o);n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;s.obstacles.push_back(n);}
 ++legs;return plan_snapshot(s);}
bool verify(int later,const std::string& row){
 const auto* shipped=fixture::guide_recipes.get(L"desert_guide");check(shipped&&text::to_utf8(shipped->get(L"rows")->arr[later].s)==row,"shipped policy matches independent native macro graph");if(!shipped)return false;
 for(int at=0;at<size;++at)if(desert_policy::distance[at]!=INT_MAX){
  auto p=advice(flags(later),cell(at));check(p.targets.size()==1&&!p.unknown&&!p.observe_waypoint_arrival&&p.guide_id==L"desert_crossing","every lawful resting cell has one native-state guide target");if(p.targets.size()!=1)return false;
  const auto& t=p.targets.front();check(index(t.goal.cell)==next[at]&&t.goal.avoid_other_events&&!t.goal.allow_scripted_crossings,"guide selects the independent first automatic entry");
  const auto r=route(flags(later),cell(at),t);check(r.path.success&&!r.path.required_trap&&!r.path.npc_approach,"actual strict route reaches every authored current entry");
  if(!r.path.success){std::printf("No route layout%d at%d,%d ->%d,%d: %ls / %ls\n",later,at%64,at/64,t.goal.cell.x,t.goal.cell.y,r.path.error.c_str(),r.path.reason.c_str());return false;}
  check(!r.path.walk_phases.empty()&&r.path.walk_phases.back().cells.back()==t.goal.cell,"strict leg stops exactly at its real entry");
  for(const auto& phase:r.path.walk_phases)for(size_t i=0;i+1<phase.cells.size();++i){const int c=index(phase.cells[i]);check(c>=0&&!directions[c]&&scripted[c]<0,"no unrelated automatic current can precede the guide goal");}
  if(next[at]!=58*64+49){int end=next[at];std::set<int> seen;while(directions[end]||scripted[end]>=0){if(!seen.insert(end).second)return false;if(scripted[end]>=0)end=scripted[end];else{const int d=direction(directions[end]);constexpr int delta[]{-64,1,64,-1};end+=delta[d];}}check(desert_policy::distance[end]<desert_policy::distance[at],"every guided native carry strictly advances toward the south exit");}
 }
 if(later)return !errors;
 auto f=flags(0);f.scenario[59]=0;f.scenario[287]=0;const Cell north{2,53,8};auto p=advice(f,north);check(p.targets.size()==1&&p.targets[0].goal.cell==Cell{2,53,13}&&p.targets[0].instruction.find(L"first option")!=std::wstring::npos,"initial encounter gives real safe standing point and correct choice");if(p.targets.size()!=1)return false;check(route(f,north,p.targets[0]).path.success,"actual north entry reaches the pre-intro standing point");
 p=advice(f,north,{true,false});check(p.targets.size()==1&&p.targets[0].instruction.find(L"first option")==std::wstring::npos,"Other off retains navigation without disclosing the encounter answer");
 p=advice(f,north,{false,true});check(p.targets.size()==1&&p.puzzle_types==2,"Other solution remains independently usable with Navigation off");
 f.scenario[287]=1;p=advice(f,{2,53,13});check(p.targets.size()==1&&p.targets[0].goal.entity_id==61&&p.targets[0].goal.cell==Cell{2,55,13},"declining or reloading retries the real moved story NPC");if(p.targets.size()!=1)return false;check(route(f,{2,53,13},p.targets[0]).path.success,"native post-intro pose has a legal interaction approach");
 for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Warp&&(o.d&baked::kAutomaticCurrent)){map_state::Resolver before(f,fixture::records),after(flags(0),fixture::records);check(before.consumed(*room,o)&&!after.consumed(*room,o),"scripted current is unavailable until full native callback59");auto unknown=f;unknown.scenario[59].reset();map_state::Resolver missing(unknown,fixture::records);check(missing.consumed(*room,o),"unknown completion never invents a current crossing");}
 f.scenario[58]=0;p=advice(f,north);check(p.targets.size()==1&&p.targets[0].goal.entity_id==63,"missing prerequisite guides back through the real north exit");if(p.targets.size()!=1)return false;check(route(f,north,p.targets[0]).path.success,"story gate does not block leaving the desert");
 check(advice(f,north,{false,true}).note==L"Story gated","enabled Other guidance identifies a missing story prerequisite without claiming the toggle is off");
 int barriers=0;map_state::Resolver gated(f,fixture::records),opened(flags(0),fixture::records);
 for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Event){barriers+=gated.story_barrier(*room,o);check(!opened.story_barrier(*room,o),"complete native story state opens every former pushback barrier");}
 check(barriers==19,"all exact pushback actors retain the prerequisite gate");
 const auto* exit=fixture::guide_recipes.get(L"desert_guide")->get(L"south");auto destination=ff5::mechanics::target(*exit);if(!destination)return false;destination->goal.avoid_other_events=false;
 check(!route(f,north,*destination).path.success,"ordinary full route cannot cross the native prerequisite barrier early");
 auto ordinary=route(flags(0),{2,53,13},*destination);check(ordinary.path.success&&ordinary.path.full_description.find(L"Ride the current")!=std::wstring::npos,"ordinary directions distinguish forced current travel from manual steps");
 auto missing=flags(0);missing.scenario[63].reset();check(advice(missing,north).unknown,"unknown current layout cancels advice");check(advice(flags(0),north,{false,true}).targets.empty(),"completed encounter never leaks Navigation with it disabled");
 const auto notice=[&](map_state::Flags state,puzzle_policy::Policy policy){return puzzle_guide::entry_guide_available(fixture::guide_recipes,286,true,[&](int bit){return state.scenario[bit];},policy);};
 check(notice(flags(0),{true,false})&&!notice(flags(0),{false,true}),"entry notice respects completed encounter and independent navigation toggle");f.scenario[58]=1;check(notice(f,{false,true})&&!notice(f,{false,false}),"eligible encounter keeps Minimal entry notice and both-off remains silent");
 std::printf("Desert production:%d assertions/%d physical route legs/%d failures\n",assertions,legs,errors);return !errors;
}
}
int main(){using namespace desert_routes;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),fixture::guide_recipes)||!json::parse(read("docs/re/ff5_desert_native_20261105.json"),proof))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;room=baked::submap_by_map_id(286);consumer=verify;int result=2;std::thread worker([&]{result=compile();});worker.join();std::printf("Desert complete production:%d assertions/%d physical route legs/%d failures\n",assertions,legs,errors);return result;}
