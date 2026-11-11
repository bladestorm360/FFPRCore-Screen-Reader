// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/book_model.cpp
// Production book guide/physical planner versus external native choices/reset.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/book_model.h"
#include <sstream>
namespace books_check {
using namespace field;using namespace baked_nav;
json::Value programs,puzzles;int failures=0,episodes=0,operations=0,hops=0,resets=0,shelf_contacts=0,wrong_choices=0;
const wchar_t* pickups[]={L"sc_e_0241",L"sc_e_0383",L"sc_e_0385"};
const wchar_t* shelves[]={L"sc_e_0382",L"sc_e_0384",L"sc_e_0386"};
struct State {map_state::Flags flags;map_state::Selection actors;int map=160;Cell at{1,12,17};};
void check(bool pass,const char* why){if(!pass&&++failures<30)std::printf("FAIL Surgate %s\n",why);}
bool shown(const State& n,int id){const auto i=n.actors.entities.find(id);return i==n.actors.entities.end()||i->second.visible!=false;}
bool active(const State& n,const baked::Object& o){return(baked::str(o.group)==L"entity_default"||n.actors.groups.count(baked::str(o.group)))&&shown(n,o.entity_id());}
bool effect(State& n,const wchar_t* name,bool yes=true){const json::Value* p=nullptr;for(const auto& r:programs.arr)if(r.get(L"name")->s==name){p=&r;break;}if(!p)return false;
 const auto selected=map_state::project(*p->get(yes?L"record":L"no"),n.flags);if(!selected.known)return false;for(int id:selected.uncertain_entities)if(id>0)return false;
 const json::Value* writes=nullptr;for(const auto& group:selected.groups){if(group.rfind(L"mod_fixture_exit_",0)==0){if(writes)return false;writes=p->get(L"outcomes")->get(group.substr(17));}else n.actors.groups.insert(group);}
 if(!writes)return false;for(const auto& row:writes->arr)n.flags.scenario[int(row.get(L"index")->n)]=int(row.get(L"value")->n);
 for(const auto& [id,e]:selected.entities)if(id>0){auto& out=n.actors.entities[id];if(e.visible)out.visible=e.visible;if(e.x)out.x=e.x;if(e.y)out.y=e.y;}return true;
}
bool enter(State& n,int map,int point){if(map!=159&&map!=160)return false;n.map=map;n.flags.current_map_id=map;n.flags.local.fill(0);n.actors={};
 if(!effect(n,map==159?L"sc_map_20171_6":L"sc_map_20171_7"))return false;map_state::Resolver state(n.flags,fixture::records);
 const auto* sm=baked::submap_by_map_id(map);const auto* arrival=point_in(*sm,point,state);if(!arrival)return false;n.at={arrival->layer,arrival->tx,arrival->ty};return true;
}
State fresh(int variant){State n;n.flags.scenario.fill(0);n.flags.global.fill(0);n.flags.local.fill(0);n.flags.area.fill(0);n.flags.treasure.fill(0);n.flags.scenario[125]=variant&1;n.flags.scenario[432]=(variant>>1)&1;check(enter(n,160,1),"actual native initializer and entrance");return n;}
RouteSnapshot snapshot(const State& n){const auto* sm=baked::submap_by_map_id(n.map);RouteSnapshot s(map_state::Resolver(n.flags,fixture::records));s.map_id=n.map;s.submap=sm;s.layer=n.at.layer;s.position=world_of_cell(*sm,n.at);s.vehicle={true,1,1,{}};s.locale=L"en";
 std::set<std::pair<int,std::wstring>> seen;for(const auto& o:baked::grids(*sm).objects){if(!active(n,o)||o.type==baked::ObjType::PointIn||o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::Warp)continue;
  const auto raw=baked::object_identifier(o);if(!seen.insert({o.entity_id(),raw}).second)continue;NativeObstacle a;a.id=o.entity_id();a.type=int(o.type);a.raw_name=raw;a.cell={o.layer,o.tx,o.ty};a.enabled=a.applies=true;
  if(const auto e=n.actors.entities.find(a.id);e!=n.actors.entities.end()){if(e->second.x)a.cell.x=*e->second.x;if(e->second.y)a.cell.y=*e->second.y;}
  a.trigger=baked_route::step_event(o);a.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;a.chest=o.type==baked::ObjType::TreasureBox;a.collider=o.type==baked::ObjType::CollisionEntity;a.collision=o.a;
  a.transportation_ids=s.state.transportation_mask(*sm,o);a.transportation_restricted=a.transportation_ids!=~uint64_t(0);s.obstacles.push_back(std::move(a));
 }return s;
}
ff5::book::Snapshot observe(const State& n){ff5::book::Snapshot s;s.map=n.map;s.locale=L"en";for(size_t i=0;i<6;++i)s.values[i]=n.flags.scenario[ff5::book::flags[i]];for(size_t i=0;i<3;++i)s.hidden[i]=!shown(n,17+int(i));s.gate_hidden=!shown(n,58);return s;}
std::wstring key(const State& n){std::wstringstream s;s<<n.map<<L':'<<n.at.layer<<L':'<<n.at.x<<L':'<<n.at.y;for(int b:ff5::book::flags)s<<L':'<<n.flags.scenario[b].value_or(-1);for(const auto& [id,e]:n.actors.entities)if(e.visible)s<<L';'<<id<<L':'<<*e.visible;return s.str();}
const wchar_t* script(int id){if(id>=17&&id<=19)return pickups[id-17];if(id==20)return L"sc_e_0387_1";if(id==21||id==35||id==36)return shelves[0];if(id==22||id==50)return shelves[1];if(id==23||id==51||id==52)return shelves[2];if(id==68)return L"sc_e_0387_2";if(id>=69&&id<=84)return id<=71?L"sc_e_0387_3":id<=73?L"sc_e_0387_4":id<=76?L"sc_e_0387_5":id<=79?L"sc_e_0387_6":id<=81?L"sc_e_0387_7":L"sc_e_0387_8";return nullptr;}
bool advance(State& n,const ff5::library::Plan& advice,const Planned& route,bool yes=true){
 if(!route.path.success||route.path.npc_approach||route.path.guidance_index<0||route.path.walk_phases.empty())return false;
 const auto* sm=baked::submap_by_map_id(n.map);const baked::Object* exit=nullptr;
 for(const auto& phase:route.path.walk_phases){for(Cell at:phase.cells){if(at==n.at)continue;for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::GotoMap&&active(n,o)&&o.layer==at.layer&&o.contains(at.x,at.y)){exit=&o;break;}if(exit)break;}if(exit)break;}
 if(exit){const int old=n.map;if(!enter(n,exit->a,exit->b))return false;++hops;if(old==159&&n.map==160)++resets;return true;}
 const auto& goal=advice.targets[size_t(route.path.guidance_index)].goal;n.at=route.path.walk_phases.back().cells.back();if(goal.map_id!=n.map)return false;if(goal.entity_id<0)return true;
 const auto* name=script(goal.entity_id);if(!name)return false;const int face=route.path.walk_phases.back().arrival_direction;
 if(goal.entity_id!=20&&(goal.entity_id<17||goal.entity_id>19)){check(face==0,"native shelf accepts only north-facing interaction");++shelf_contacts;}
 ++operations;if(!effect(n,name,yes))return false;if(goal.entity_id==20)n.at={1,22,15};return true;
}
bool manual(State& n,int id,bool yes=true){const auto* sm=baked::submap_by_map_id(160);const auto& objects=baked::grids(*sm).objects;const auto o=std::find_if(objects.begin(),objects.end(),[id](const auto& o){return o.entity_id()==id&&(o.type==baked::ObjType::Entity||o.type==baked::ObjType::NPC);});if(o==objects.end())return false;
 ff5::library::Plan advice;GuideTarget t;t.goal.map_id=160;t.goal.cell={o->layer,o->tx,o->ty};t.goal.entity_id=id;t.goal.raw_name=baked::object_identifier(*o);t.goal.examine=t.goal.avoid_other_events=true;t.kind=TargetKind::Interact;advice.targets.push_back(t);
 auto captured=snapshot(n);captured.candidates=advice.targets;return advance(n,advice,plan_snapshot(captured),yes);
}
bool episode(State n){++episodes;std::set<std::wstring> visited;
 for(int step=0;step<24;++step){const auto advice=ff5::book::plan(observe(n),puzzles);if(advice.finished)return n.flags.scenario[521]==1&&n.flags.scenario[522]==1&&n.flags.scenario[523]==1&&!shown(n,58);
  if(!visited.insert(key(n)).second){check(false,"guide repeats native state");return false;}if(advice.unknown||advice.targets.empty()){check(false,"complete native-state decision");return false;}
  auto captured=snapshot(n);captured.candidates=advice.targets;const auto route=plan_snapshot(captured);
  if(!advance(n,advice,route)){std::printf("Surgate stopped map%d at%d,%d reason%ls targets",n.map,n.at.x,n.at.y,route.path.reason.c_str());for(const auto& t:advice.targets)std::printf(" %d/%d",t.goal.map_id,t.goal.entity_id);std::putchar('\n');check(false,"physical/native transition");return false;}
 }check(false,"finite full-episode bound");return false;
}
void verify(){
 // Every placement subset and carried remainder is constructed by actual
 // pickup/shelf interactions in the same loaded room, never a fake re-entry.
 for(int variant=0;variant<4;++variant)for(int mask=0;mask<8;++mask){auto n=fresh(variant);for(int i=0;i<3;++i)if(mask&(1<<i))check(manual(n,17+i)&&manual(n,21+i),"construct native correct arrangement");check(episode(n),"all placement subsets and room variants finish");
  for(int i=0;i<3;++i)if(!(mask&(1<<i))){auto held=n;check(manual(held,17+i),"actual carried-book resume");check(episode(held),"each held book with every native placed subset");}
  auto reload=n;check(enter(reload,160,1),"native partial/complete re-entry");for(int i=0;i<6;++i)check(reload.flags.scenario[521+i]==(mask==7&&i<3?1:0),"re-entry resets all unfinished bits and preserves complete placement");check(episode(reload),"re-entry guide finishes from actual native reset");
 }
 const int ids[3][3]={{21,35,36},{22,50,0},{23,51,52}};
 for(int i=0;i<3;++i)for(int id:ids[i])if(id){auto n=fresh(0);check(manual(n,17+i),"prepare correct shelf alternative");const auto before=key(n);check(manual(n,id,false)&&n.flags.scenario[524+i]==1&&n.flags.scenario[521+i]==0,"No preserves held and placed receipts");check(manual(n,id)&&n.flags.scenario[521+i]==1&&n.flags.scenario[524+i]==0,"every exact shelf contact accepts its native book");check(episode(n),"resume every shelf contact");}
 for(int book=0;book<3;++book)for(int target:{21,22,23,68,69,72,74,77,80,82}){if(target==21+book)continue;auto n=fresh(0);check(manual(n,17+book)&&manual(n,target),"actual wrong shelf accepted");++wrong_choices;
  check(n.flags.scenario[521+book]==0&&n.flags.scenario[524+book]==0&&!shown(n,17+book),"wrong shelf loses book without placement receipt");const auto advice=ff5::book::plan(observe(n),puzzles);check(!advice.targets.empty()&&advice.targets.front().goal.entity_id==1,"lost book selects real reset exit");check(episode(n),"wrong-placement recovery leaves/re-enters then completes");}
 auto outside=fresh(0);check(enter(outside,159,2)&&episode(outside),"actual entrance corridor to full puzzle");
 auto invalid=observe(fresh(0));invalid.values[3]=invalid.values[4]=1;check(ff5::book::plan(invalid,puzzles).unknown,"multiple carried books remain unknown");
 auto missing=observe(fresh(0));missing.hidden[0].reset();check(ff5::book::plan(missing,puzzles).unknown,"unobserved lost-book dependency remains unknown");
 auto cancel=observe(fresh(0));cancel.cancelled=[]{return true;};check(ff5::book::plan(cancel,puzzles).targets.empty(),"cancelled work gives no stale advice");
 auto done=observe(fresh(0));done.values={1,1,1,0,0,0};done.gate_hidden=true;done.hidden.fill(std::nullopt);check(ff5::book::plan(done,json::Value{}).finished,"actual final wall receipt retires old actors/resource");
 check(resets>=wrong_choices&&hops>=2*wrong_choices,"all wrong choices use both real reset map legs");
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);using namespace books_check;const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),puzzles)||!json::parse(read("../tools/ff5_surgate_20261102/comparison_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;std::thread worker([]{verify();});worker.join();
 std::printf("Surgate: %d complete/resumed episodes/%d manual operations/%d shelf contacts/%d wrong choices/%d native hops/%d resets; failures%d worker-native%d\n",episodes,operations,shelf_contacts,wrong_choices,hops,resets,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;}
