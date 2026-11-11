// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/stone_model.cpp
// Actual guide/physical route versus external native branch exits and resets.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/stone_model.h"
#include <sstream>
namespace stone_check {
using namespace field;using namespace baked_nav;
json::Value programs,puzzles;int failures=0,episodes=0,operations=0,hops=0,resets=0;
struct State {map_state::Flags flags;map_state::Selection actors;int map=371;Cell at{0,9,13};};
std::array<int,2> return_counts{};
void check(bool pass,const char* why){if(!pass&&++failures<30)std::printf("FAIL Ghido %s\n",why);}
const json::Value* program(const wchar_t* name){for(const auto& r:programs.arr)if(r.get(L"name")->s==name)return &r;return nullptr;}
bool effect(State& n,const wchar_t* name,bool yes=true){const auto* p=program(name);if(!p)return false;const auto selected=map_state::project(*p->get(yes?L"record":L"no"),n.flags);
 if(!selected.known)return false;for(int id:selected.uncertain_entities)if(id>0)return false;const json::Value* writes=nullptr;
 for(const auto& group:selected.groups){if(group.rfind(L"mod_fixture_exit_",0)==0){if(writes)return false;writes=p->get(L"outcomes")->get(group.substr(17));}else n.actors.groups.insert(group);}
 if(!writes)return false;for(const auto& row:writes->arr)n.flags.scenario[int(row.get(L"index")->n)]=int(row.get(L"value")->n);
 for(const auto& [id,e]:selected.entities)if(id>0){auto& out=n.actors.entities[id];if(e.visible)out.visible=e.visible;if(e.x)out.x=e.x;if(e.y)out.y=e.y;}return true;
}
bool enter(State& n,int map,int point){n.map=map;n.flags.current_map_id=map;n.flags.local.fill(0);n.actors={};
 if(map<=372&&!effect(n,map==370?L"sc_map_30381_2":map==371?L"sc_map_30381_3":L"sc_map_30381_4"))return false;
 map_state::Resolver resolver(n.flags,fixture::records);const auto* sm=baked::submap_by_map_id(map);const auto* arrival=point_in(*sm,point,resolver);if(!arrival)return false;n.at={arrival->layer,arrival->tx,arrival->ty};return true;
}
State initial(int position,int opened){State n;n.flags.scenario.fill(0);n.flags.global.fill(0);n.flags.local.fill(0);n.flags.area.fill(0);n.flags.treasure.fill(0);n.flags.scenario[176]=position==2?0:1;
 if(position<5&&position!=2)n.flags.scenario[174+position]=1;n.flags.scenario[179]=position==5?1:0;n.flags.scenario[294]=opened;check(enter(n,371,1),"exact native initial scene");n.at={0,9,13};return n;}
bool shown(const State& n,int id){const auto i=n.actors.entities.find(id);return i==n.actors.entities.end()||i->second.visible!=false;}
bool active(const State& n,const baked::Object& o){return(baked::str(o.group)==L"entity_default"||n.actors.groups.count(baked::str(o.group)))&&shown(n,o.entity_id());}
RouteSnapshot snapshot(const State& n){const auto* sm=baked::submap_by_map_id(n.map);RouteSnapshot s(map_state::Resolver(n.flags,fixture::records));s.map_id=n.map;s.submap=sm;s.layer=n.at.layer;s.position=world_of_cell(*sm,n.at);s.vehicle={true,1,1,{}};s.locale=L"en";
 std::set<std::pair<int,std::wstring>> seen;
 for(const auto& o:baked::grids(*sm).objects){if(!active(n,o)||o.type==baked::ObjType::PointIn||o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::Warp)continue;
  const auto raw=baked::object_identifier(o);if(!seen.insert({o.entity_id(),raw}).second)continue;
  NativeObstacle a;a.id=o.entity_id();a.type=int(o.type);a.raw_name=raw;a.cell={o.layer,o.tx,o.ty};a.enabled=a.applies=true;
  if(const auto e=n.actors.entities.find(a.id);e!=n.actors.entities.end()){if(e->second.x)a.cell.x=*e->second.x;if(e->second.y)a.cell.y=*e->second.y;}
  a.trigger=baked_route::step_event(o);a.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;a.chest=o.type==baked::ObjType::TreasureBox;a.collider=o.type==baked::ObjType::CollisionEntity;a.collision=o.a;
  a.transportation_ids=s.state.transportation_mask(*sm,o);a.transportation_restricted=a.transportation_ids!=~uint64_t(0);s.obstacles.push_back(std::move(a));
 }return s;
}
ff5::stone::Snapshot observe(const State& n){ff5::stone::Snapshot s;s.map=n.map;s.locale=L"en";for(size_t i=0;i<7;++i)s.values[i]=n.flags.scenario[ff5::stone::flags[i]];if(n.map==372)s.stairs_hidden=!shown(n,11);return s;}
std::wstring key(const State& n){std::wstringstream s;s<<n.map<<L':'<<n.at.layer<<L':'<<n.at.x<<L':'<<n.at.y;for(int b:ff5::stone::flags)s<<L':'<<n.flags.scenario[b].value_or(-1);for(const auto& [id,e]:n.actors.entities)if(e.visible)s<<L';'<<id<<L':'<<*e.visible;return s.str();}
const wchar_t* chest_script(int id){return id==30?L"sc_e_0252_1":id==29?L"sc_e_0252_2":id==28?L"sc_e_0252_3":id==31?L"sc_e_0252_4":id==32?L"sc_e_0252_5":nullptr;}
bool advance(State& n,const ff5::library::Plan& advice,const Planned& route){
 if(!route.path.success||route.path.npc_approach||route.path.guidance_index<0||route.path.walk_phases.empty())return false;
 const auto* sm=baked::submap_by_map_id(n.map);const baked::Object* exit=nullptr;
 for(const auto& phase:route.path.walk_phases){for(Cell at:phase.cells){if(at==n.at)continue;
   for(const auto& o:baked::grids(*sm).objects)if((o.type==baked::ObjType::GotoMap||(o.type==baked::ObjType::Warp&&(o.d&255)==1))&&active(n,o)&&o.layer==at.layer&&o.contains(at.x,at.y)){exit=&o;break;}
   if(exit)break;
  }if(exit)break;}
 if(exit){const int old=n.map,id=exit->entity_id(),destination=exit->a,point=exit->b;
  if(destination<370||destination>373||!enter(n,destination,point))return false;++hops;
  if(old==370&&(id==29||id==30)){if(!effect(n,id==29?L"sc_e_0252_9":L"sc_e_0252_7"))return false;++resets;++return_counts[id==29?1:0];}return true;
 }
 n.at=route.path.walk_phases.back().cells.back();const auto& goal=advice.targets[size_t(route.path.guidance_index)].goal;
 if(goal.map_id!=n.map)return false;if(goal.entity_id<0)return true;
 const auto* name=n.map==371?chest_script(goal.entity_id):n.map==370&&goal.entity_id==15?L"sc_e_0254":n.map==372&&goal.entity_id==15?L"sc_e_0255":nullptr;
 if(!name)return false;++operations;return effect(n,name);
}
bool episode(State n){++episodes;std::set<std::wstring> visited;
 for(int step=0;step<64;++step){const auto advice=ff5::stone::plan(observe(n),puzzles);if(advice.finished)return n.map==373;
  if(!visited.insert(key(n)).second){check(false,"guide repeats a native state");return false;}
  if(advice.unknown||advice.targets.empty()){std::printf("Ghido model incomplete map%d\n",n.map);check(false,"complete native-state decision");return false;}
  auto s=snapshot(n);s.candidates=advice.targets;const auto result=plan_snapshot(s);
  if(!advance(n,advice,result)){std::printf("Ghido route stopped map%d at%d,%d reason%ls targets",n.map,n.at.x,n.at.y,result.path.reason.c_str());for(const auto& t:advice.targets)std::printf(" %d/%d",t.goal.map_id,t.goal.entity_id);std::putchar('\n');check(false,"physical/native operation transition");return false;}
 }check(false,"finite full-episode bound");return false;
}
void verify(){
 for(int opened:{0,1})for(int position=0;position<6;++position){auto n=initial(position,opened);check(episode(n),"all six stone locations and both switch stages complete");}
 auto entrance=initial(2,0);check(enter(entrance,370,1)&&episode(entrance),"complete actual dungeon entrance");
 auto lower=initial(3,1);check(enter(lower,372,1)&&episode(lower),"flagless hidden-stair floor");
 auto used=lower;check(effect(used,L"sc_e_0255")&&episode(used),"already-operated current stairs resume");
 {auto back=initial(3,0);const auto* room=baked::submap_by_map_id(370);const auto& objects=baked::grids(*room).objects;
  // Route to the actual event and replay each real map leg. A presumed
  // PointIn number is not a substitute for the worker's selected passage.
  const auto portal=std::find_if(objects.begin(),objects.end(),[](const auto& o){return o.type==baked::ObjType::Event&&o.entity_id()==29;});
  if(portal==objects.end())check(false,"exact native SW return marker");else{
   ff5::library::Plan advice;GuideTarget target;target.goal.map_id=370;target.goal.cell={portal->layer,portal->tx,portal->ty};target.goal.entity_id=29;target.goal.raw_name=baked::object_identifier(*portal);target.goal.avoid_other_events=true;target.kind=TargetKind::Spot;advice.targets.push_back(target);
   const int before=return_counts[1];bool returned=true;
   for(int leg=0;leg<8&&return_counts[1]==before;++leg){auto captured=snapshot(back);captured.candidates=advice.targets;const auto path=plan_snapshot(captured);if(!advance(back,advice,path)){returned=false;break;}}
   check(returned&&return_counts[1]>before&&back.map==371&&back.flags.scenario[177]==1&&back.flags.scenario[176]==1&&back.flags.scenario[174]==0,"actual return continuation restores SW stone before fresh guidance");check(episode(back),"guide resumes after native SW-return reset");
  }
 }
 check(return_counts[0]>0&&return_counts[1]>0,"both native return reset paths exercised");
 for(int id:{30,29,28,31,32}){auto n=initial(id==30?0:id==29?1:id==28?2:id==31?3:4,0);const auto before=key(n);check(effect(n,chest_script(id),false)&&key(n)==before,"No choice preserves actual stone/gate state");}
 auto unknown=observe(initial(2,0));unknown.values[0].reset();check(ff5::stone::plan(unknown,puzzles).unknown,"unavailable stone receipt stays unknown");
 auto invalid=observe(initial(2,0));invalid.values[0]=1;check(ff5::stone::plan(invalid,puzzles).unknown,"two simultaneous stones cannot invent progress");
 ff5::stone::Snapshot done;done.map=373;done.locale=L"en";check(ff5::stone::plan(done,puzzles).finished,"actual final floor retires prior observations");
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);using namespace stone_check;const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),puzzles)||!json::parse(read("../tools/ff5_stone_20261102/comparison_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;std::thread worker([]{verify();});worker.join();
 std::printf("Ghido: %d complete/resumed episodes/%d manual operations/%d native hops/%d return resets; failures%d worker-native%d\n",episodes,operations,hops,resets,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;}
