// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Compare reachable native shelf layouts using EXTERNAL source-bearing programs.
// This offline prototype never loads a game; no native program is embedded here.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include <deque>
#include <sstream>
namespace {
using namespace field;using namespace baked_nav;
json::Value programs;int redirected_controls=0;
struct State {map_state::Flags flags;std::map<int,map_state::EntityState> entities;Cell at;std::vector<int> operations;};
const json::Value* program(const wchar_t* name){for(const auto& r:programs.arr)if(r.get(L"name")->s==name)return &r;return nullptr;}
std::wstring key(const State& s){std::wstringstream out;out<<s.at.layer<<L','<<s.at.x<<L','<<s.at.y<<L':'<<s.flags.scenario[395].value_or(-1);for(const auto& [id,e]:s.entities)if(e.visible)out<<L';'<<id<<L'='<<*e.visible;return out.str();}
bool shown(const State& s,int id){auto i=s.entities.find(id);return i==s.entities.end()||i->second.visible!=false;}
const baked::Object* control(const baked::Submap& sm,int id){
 const auto& objects=baked::grids(sm).objects;
 const auto found=std::find_if(objects.begin(),objects.end(),[&](const auto& o){const auto group=baked::str(o.group);return o.entity_id()==id&&o.type!=baked::ObjType::Warp&&(group==L"ev_e_0220"||group==L"ev_e_0223"||group==L"ev_e_0431");});
 return found==objects.end()?nullptr:&*found;
}
bool search(const baked::Submap& sm,Cell start,int old_flag){
 const int map=sm.first_map_id,goal_id=map==275?20:14;const std::vector<int> ids=map==275?std::vector<int>{9,10,14,15,16,17,21,27,20}:std::vector<int>{13,16,15,14};
 State initial;initial.flags.scenario.fill(0);initial.flags.local.fill(0);initial.flags.area.fill(0);initial.flags.global.fill(0);initial.flags.treasure.fill(0);initial.flags.current_map_id=map;initial.flags.scenario[395]=old_flag;
 initial.flags.scenario[48]=1;initial.flags.scenario[163]=1;
 const auto* record=fixture::records.get(baked::name_of(sm));if(!record)return false;
 const auto base=map_state::project(*record,initial.flags);if(!base.known){std::printf("FAIL native initializer unknown%d\n",map);return false;}
 initial.entities=base.entities;initial.at=start;std::deque<State> pending{initial};std::set<std::wstring> seen{key(initial)};int edges=0;
 while(!pending.empty()&&seen.size()<=512&&edges<4096){
  const auto current=std::move(pending.front());pending.pop_front();
  for(int desired:ids){
   if(!shown(current,desired))continue;const auto* target=control(sm,desired);if(!target)continue;
   RouteSnapshot snapshot(map_state::Resolver(current.flags,fixture::records));snapshot.map_id=map;snapshot.submap=&sm;snapshot.position=world_of_cell(sm,current.at);snapshot.layer=current.at.layer;
   snapshot.vehicle={true,1,1,{}};snapshot.locale=L"en";snapshot.goal.map_id=map;snapshot.goal.entity_id=desired;snapshot.goal.raw_name=baked::object_identifier(*target);snapshot.goal.cell={target->layer,target->tx,target->ty};snapshot.goal.examine=!baked_route::step_event(*target);
   snapshot.kind=snapshot.goal.examine?TargetKind::Interact:TargetKind::Spot;snapshot.goal.avoid_other_events=true;
   for(const auto& o:baked::grids(sm).objects){
    const auto group=baked::str(o.group);if(group!=L"entity_default"&&!base.groups.count(group))continue;
    if(o.type==baked::ObjType::Warp||o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::PointIn||!shown(current,o.entity_id()))continue;
    NativeObstacle n;n.id=o.entity_id();n.type=int(o.type);n.raw_name=baked::object_identifier(o);n.cell={o.layer,o.tx,o.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(o);
    n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;
    n.transportation_ids=snapshot.state.transportation_mask(sm,o);n.transportation_restricted=n.transportation_ids!=~uint64_t(0);snapshot.obstacles.push_back(std::move(n));
   }
   const auto route=plan_snapshot(snapshot);++edges;if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach)continue;
   int actual=desired;Cell end=route.path.walk_phases.back().cells.back();bool triggered=false;
   // Native contact controls run before a farther requested destination. Stop
   // at the first one and replay its ready resulting visibility, never walk
   // through a layout-changing script as though it were ordinary floor.
   for(const auto& phase:route.path.walk_phases){for(const auto cell:phase.cells){
    if(cell==current.at)continue;
    for(int id:ids){const auto* o=control(sm,id);if(o&&shown(current,id)&&baked_route::step_event(*o)&&o->layer==cell.layer&&o->contains(cell.x,cell.y)){actual=id;end=cell;triggered=true;break;}}
    if(triggered)break;
   }if(triggered)break;}
   if(actual!=desired){
    ++redirected_controls;const auto* prefix=control(sm,actual);
    auto redirected=snapshot;redirected.goal.entity_id=actual;redirected.goal.raw_name=baked::object_identifier(*prefix);redirected.goal.cell=end;redirected.goal.examine=false;redirected.kind=TargetKind::Spot;
    const auto direct=plan_snapshot(redirected);
    std::printf("Library prefix map%d flag%d desired%d actual%d L%d %d,%d direct%d\n",map,old_flag,desired,actual,end.layer,end.x,end.y,direct.path.success);
    if(!direct.path.success)return false;
   }
   State next=current;next.at=end;next.operations.push_back(actual);
   if(actual==goal_id){std::printf("Library native layout%d flag%d startL%d %d,%d:",map,old_flag,start.layer,start.x,start.y);for(int id:next.operations)std::printf(" %d",id);std::printf("; states%zu edges%d\n",seen.size(),edges);return true;}
   const auto* actor=control(sm,actual);const auto script=map==275?(actual==20?L"sc_e_0220":actual==17?L"sc_e_0220_1":actual==9?L"sc_e_0220_2":actual==10?L"sc_e_0220_3":actual==14?L"sc_e_0220_4":actual==15?L"sc_e_0220_5":actual==16?L"sc_e_0220_6":actual==27?L"sc_e_0220_7":L"sc_e_0220_8"):(actual==13?L"sc_e_0223":actual==16?L"sc_e_0223_1":L"sc_e_0223_2");
   const auto* native=program(script);if(!actor||!native)continue;
   // The external fixture qualifies only three unchanged-cell FFV placements;
   // it preserves the global conservative non-VI projection contract.
   const auto effect=map_state::project(*native->get(L"record"),next.flags);if(!effect.known)continue;
   for(const auto& [id,e]:effect.entities){auto& into=next.entities[id];if(e.visible)into.visible=e.visible;if(e.x)into.x=e.x;if(e.y)into.y=e.y;}
   for(const auto& f:native->get(L"flag_updates")->arr)next.flags.scenario[int(f.get(L"index")->n)]=int(f.get(L"value")->n);
   if(seen.insert(key(next)).second)pending.push_back(std::move(next));
  }
 }
 std::printf("FAIL Library layout%d flag%d startL%d %d,%d: states%zu edges%d\n",map,old_flag,start.layer,start.x,start.y,seen.size(),edges);return false;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("../tools/ff5_library_20261101/native_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;int cases=0,failures=0;
 for(int map:{275,281}){const auto* room=baked::submap_by_map_id(map);for(const auto& entry:baked::grids(*room).objects)if(entry.type==baked::ObjType::PointIn){for(int flag:{0,1}){bool pass=false;std::thread worker([&]{pass=search(*room,{entry.layer,entry.tx,entry.ty},flag);});worker.join();++cases;if(!pass)++failures;}}}
 std::printf("Library external-native layouts: %d entrance/flag cases, failures%d owner%d redirects%d\n",cases,failures,fixture::wrong_owner.load(),redirected_controls);return failures||fixture::wrong_owner?1:0;
}
