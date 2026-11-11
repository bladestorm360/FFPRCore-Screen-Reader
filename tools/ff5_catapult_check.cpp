// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/catapult_model.cpp
// Production model + ordered route against external primary-script effects.
// All current actors retain native groups, visibility, collision and position.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/catapult_model.h"
#include <sstream>
namespace catapult_check {
using namespace field;using namespace baked_nav;
json::Value programs,puzzles;int failures=0,episodes=0,operations=0,poses=0,book_routes=0;
struct State {map_state::Flags flags;map_state::Selection actors;Cell at{1,18,22};};
const wchar_t* scripts[]={L"sc_e_0224_1",L"sc_e_0224_2",L"sc_e_0224_3",L"sc_e_0224_4",L"sc_e_0224_5"};
void check(bool pass,const char* why){if(!pass&&++failures<25)std::printf("FAIL Catapult %s\n",why);}
const json::Value* program(const wchar_t* name){for(const auto& row:programs.arr)if(row.get(L"name")->s==name)return &row;return nullptr;}
bool shown(const State& n,int id){const auto it=n.actors.entities.find(id);return it==n.actors.entities.end()||it->second.visible!=false;}
bool active(const State& n,const baked::Object& o){return (baked::str(o.group)==L"entity_default"||n.actors.groups.count(baked::str(o.group)))&&shown(n,o.entity_id());}
bool effect(State& n,const wchar_t* name,int face=0,bool yes=true){
 const auto* source=program(name);if(!source)return false;const json::Value* record=source->get(yes?L"record":L"no");
 if(source->get(L"directions"))record=source->get(L"directions")->get(std::to_wstring(face));
 if(!record)return false;const auto value=map_state::project(*record,n.flags);if(!value.known||!value.uncertain_entities.empty())return false;
 for(const auto& [id,e]:value.entities){auto& out=n.actors.entities[id];if(e.visible)out.visible=e.visible;if(e.x)out.x=e.x;if(e.y)out.y=e.y;}
 for(const auto& group:value.groups)n.actors.groups.insert(group);
 const bool completes=(!source->get(L"directions")||(face==0||face==2))&&(yes||std::wstring_view(name)!=L"sc_e_0224_5");
 if(completes)for(const auto& row:source->get(L"flag_updates")->arr)n.flags.scenario[int(row.get(L"index")->n)]=int(row.get(L"value")->n);
 return true;
}
bool enter(State& n){n.actors={};n.flags.current_map_id=293;n.flags.local.fill(0);return effect(n,L"sc_map_20231_4");}
State fresh(int variant){State n;n.flags.scenario.fill(0);n.flags.global.fill(0);n.flags.local.fill(0);n.flags.area.fill(0);n.flags.treasure.fill(0);
 // All six actual initializer branches: Cid/Mid, staged books, later letter,
 // absent occupants and returned occupants. Notepad125 also gets its variant.
 n.flags.scenario[65]=1;if(variant>=1)n.flags.scenario[69]=1;if(variant>=2)n.flags.scenario[74]=1;if(variant>=3)n.flags.scenario[75]=1;
 if(variant>=4)n.flags.scenario[76]=n.flags.scenario[77]=1;if(variant>=5)n.flags.scenario[146]=n.flags.scenario[125]=1;
 check(enter(n),"exact native initializer");return n;}
ff5::catapult::Snapshot observe(const State& n){ff5::catapult::Snapshot s;s.map=293;s.player=n.at;s.locale=L"en";
 for(size_t i=0;i<5;++i){s.values[i]=n.flags.scenario[ff5::catapult::flags[i]];s.hidden[i]=!shown(n,ff5::catapult::controls(293)[i].id);}return s;}
RouteSnapshot snapshot(const State& n){const auto* sm=baked::submap_by_map_id(293);RouteSnapshot s(map_state::Resolver(n.flags,fixture::records));
 s.map_id=293;s.submap=sm;s.layer=n.at.layer;s.position=world_of_cell(*sm,n.at);s.vehicle={true,1,1,{}};s.locale=L"en";s.kind=TargetKind::Interact;
 std::set<std::pair<int,std::wstring>> seen;
 for(const auto& o:baked::grids(*sm).objects){if(!active(n,o)||o.type==baked::ObjType::PointIn||o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::Warp)continue;
  const auto raw=baked::object_identifier(o);if(!seen.insert({o.entity_id(),raw}).second)continue;
  NativeObstacle a;a.id=o.entity_id();a.type=int(o.type);a.raw_name=raw;a.cell={o.layer,o.tx,o.ty};a.enabled=a.applies=true;
  if(const auto e=n.actors.entities.find(a.id);e!=n.actors.entities.end()){if(e->second.x)a.cell.x=*e->second.x;if(e->second.y)a.cell.y=*e->second.y;}
  a.trigger=baked_route::step_event(o);a.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;a.chest=o.type==baked::ObjType::TreasureBox;
  a.collider=o.type==baked::ObjType::CollisionEntity;a.collision=o.a;a.transportation_ids=s.state.transportation_mask(*sm,o);a.transportation_restricted=a.transportation_ids!=~uint64_t(0);s.obstacles.push_back(std::move(a));
 }return s;
}
std::wstring key(const State& n){std::wstringstream s;s<<n.at.layer<<L':'<<n.at.x<<L':'<<n.at.y;for(int b:ff5::catapult::flags)s<<L':'<<n.flags.scenario[b].value_or(-1);
 for(const auto& [id,e]:n.actors.entities)s<<L';'<<id<<L','<<e.visible.value_or(true)<<L','<<e.x.value_or(-999)<<L','<<e.y.value_or(-999);return s.str();}
bool episode(State n){++episodes;std::set<std::wstring> visited;
 for(int step=0;step<24;++step){const auto advice=ff5::catapult::plan(observe(n),puzzles);if(advice.finished)return n.flags.scenario[313]==1;
  if(!visited.insert(key(n)).second){check(false,"native receipt/visibility episode loops");return false;}
  check(advice.supported&&!advice.unknown&&!advice.targets.empty(),"complete real-state decision");if(advice.targets.empty())return false;
  auto s=snapshot(n);s.candidates=advice.targets;const auto result=plan_snapshot(s);
  check(result.path.success&&!result.path.npc_approach&&result.path.guidance_index>=0,"ordered route reaches real control");
  if(!result.path.success||result.path.guidance_index<0||result.path.walk_phases.empty()){
   std::printf("Catapult stopped at L%d %d,%d receipts",n.at.layer,n.at.x,n.at.y);for(int b:ff5::catapult::flags)std::printf(" %d",n.flags.scenario[b].value_or(-1));
   std::printf(" targets");for(const auto& t:advice.targets)std::printf(" %d",t.goal.entity_id);std::printf(" reason %ls\n",result.path.reason.c_str());return false;}
  const auto& goal=advice.targets[size_t(result.path.guidance_index)].goal;size_t i=0;while(i<5&&ff5::catapult::controls(293)[i].id!=goal.entity_id)++i;
  const auto& phase=result.path.walk_phases.back();if(phase.cells.empty())return false;n.at=phase.cells.back();
  if(goal.entity_id==17){check(!goal.examine&&n.at==Cell{1,18,12},"entry transit stops at the exact native rest contact");check(effect(n,L"sc_e_0432",2,false),"native No response returns control without invented progress");++operations;continue;}
  if(i==5){check(false,"native control identity");return false;}
  const int face=phase.arrival_direction;check(i!=3||face==0||face==2,"book approached from a native working direction");
  check(effect(n,scripts[i],face),"exact native operation projects");++operations;
 }check(false,"finite end-to-end episode");return false;
}
void verify(){
 const auto* sm=baked::submap_by_map_id(293);std::vector<Cell> starts;
 for(const auto& o:baked::grids(*sm).objects)if(o.type==baked::ObjType::PointIn)starts.push_back({o.layer,o.tx,o.ty});
 check(!starts.empty(),"native entrances available");
 for(int variant=0;variant<6;++variant){State n=fresh(variant);
  for(int prefix=0;prefix<=5;++prefix){
   auto advice=ff5::catapult::plan(observe(n),puzzles);check(advice.finished==(prefix==5),"persistent receipt owns final completion");
   if(prefix==3){check(n.actors.entities[23].x==10&&n.actors.entities[23].y==14&&shown(n,23),"real book moved from off-map home to10,14");++poses;
    for(Cell at:std::array<Cell,2>{{{1,10,13},{1,10,15}}}){auto resume=n;resume.at=at;episode(resume);}
    for(Cell at:std::array<Cell,3>{{starts.front(),{1,10,13},{1,10,15}}}){auto resume=n;resume.at=at;auto ordinary=snapshot(resume);
     const auto advice=ff5::catapult::plan(observe(resume),puzzles);const auto book=std::find_if(advice.targets.begin(),advice.targets.end(),[](const auto& target){return target.goal.entity_id==23;});
     if(book==advice.targets.end()){check(false,"actual relocated book remains available");continue;}ordinary.goal=book->goal;ordinary.kind=book->kind;
     const auto result=plan_snapshot(ordinary);const auto browse=plan_browse_snapshot(ordinary);++book_routes;
     check(result.path.success&&result.goal.cell==Cell{1,10,14}&&browse.route.direct,"ordinary directions and browsing use the actual relocated book");
     if(!result.path.walk_phases.empty()){const int face=result.path.walk_phases.back().arrival_direction;check(face==0||face==2,"ordinary book route obeys native successful sides");}
    }
    auto captured=snapshot(n);SnapshotScope scope(captured);auto request=walk_request(*sm,starts.front());const auto live=live_blockers(*sm,baked::grids(*sm),{});
    request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);const auto reach=baked_route::flood(request);
    check(!reach.reachable({1,11,14})&&!reach.reachable({1,12,14}),"east book pocket is disconnected after the native urn effect, not a valid resume");
   }
   for(bool reload:{false,true}){auto resume=n;if(reload)check(enter(resume),"native re-entry projects");
    for(Cell at:starts){resume.at=at;check(episode(resume),"full guide from native entrance and clue prefix");}
   }
   if(prefix<5)check(effect(n,scripts[prefix]),"actual required clue advances prefix");
  }
 }
 auto n=fresh(0);for(int i=0;i<4;++i)check(effect(n,scripts[i]),"prepare final native choice");
 check(effect(n,scripts[4],0,false)&&n.flags.scenario[313]==0,"No preserves unfinished receipt");n.at=starts.front();check(episode(n),"guide recovers after No");
 auto s=observe(fresh(0));s.values[0].reset();check(ff5::catapult::plan(s,puzzles).unknown,"unknown flag cannot invent progress");
 s=observe(fresh(0));s.hidden[0].reset();check(ff5::catapult::plan(s,puzzles).unknown,"unknown actor cannot invent a control");
 s.values[4]=1;check(ff5::catapult::plan(s,puzzles).finished,"final receipt retires earlier missing actors");
 s=observe(fresh(0));s.cancelled=[]{return true;};check(ff5::catapult::plan(s,puzzles).targets.empty(),"cancellation suppresses worker advice");
 auto bad=puzzles;bad.obj[L"catapult_guide"].obj[L"steps"].arr[3].obj[L"raw"].s=L"unrelated book";
 check(ff5::catapult::plan(observe(fresh(0)),bad).unknown,"resource must match exact staged actor");
 for(int done:{0,1}){auto chest=fresh(0);if(done)for(int bit:ff5::catapult::flags)chest.flags.scenario[bit]=1;check(enter(chest),"chest-access initializer");chest.at={1,14,21};auto captured=snapshot(chest);
  const auto& objects=baked::grids(*sm).objects;const auto found=std::find_if(objects.begin(),objects.end(),[](const auto& o){return o.type==baked::ObjType::TreasureBox&&o.entity_id()==13;});
  if(found==objects.end()){check(false,"exact type-only chest target");continue;}
  captured.goal.map_id=293;captured.goal.cell={found->layer,found->tx,found->ty};captured.goal.entity_id=13;captured.goal.raw_name=baked::object_identifier(*found);captured.goal.examine=true;captured.goal.avoid_other_events=true;
  const auto result=plan_snapshot(captured);check(result.path.success==bool(done),"final313 owns actual chest passage, without exposing contents");
 }
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);using namespace catapult_check;
 const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("data/ff5/puzzles.json"),puzzles)||!json::parse(read("../tools/ff5_catapult_20261101/puzzle_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;std::thread worker([]{verify();});worker.join();
 std::printf("Catapult: %d native-prefix/re-entry episodes/%d actual operations/%d relocated-book profiles/%d ordinary book routes and browse checks; failures%d worker-native%d\n",episodes,operations,poses,book_routes,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
