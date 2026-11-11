// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Native Library ordinary-gate and physical-operation witnesses. The former
// fixture assumed every target was reachable before its shelf was operated.
// Keep those nine static profiles as explicit gate/route checks, then require
// every same target on a physical native journey from the fresh entrance.
// Source-bearing comparison programs remain external to every Git tree.
#define FFPR_LIBRARY_PRODUCTION_CHECK
#include "ff5_library_policy_check.cpp"
namespace {
int failures=0,cases=0;
struct Witness {const char* name;int map,target_map,entity;std::vector<int> completed;bool gated;};
const std::vector<Witness> witnesses{
 {"initial shelf",275,275,20,{},true},
 {"Ifrit after shelf",275,279,4,{396},true},
 {"Ifrit room",279,279,4,{396},false},
 {"return fire seal",279,278,17,{48,396},false},
 {"lower shelf",278,277,31,{48,163,396},false},
 {"lower maze",277,281,16,{48,163,396,397},true},
 {"maze initial control",281,281,16,{48,163,396,397},true},
 {"maze blocking book",281,281,14,{48,163,396,397},true},
 {"Byblos approach",282,283,7,{48,163,396,397},false}
};
void witness(const Witness& spec){
 const auto [name,initial_map,target_map,entity,completed,expected_gate]=spec;int map=initial_map;
 using namespace field;using namespace baked_nav;map_state::Flags flags;flags.scenario.fill(0);flags.local.fill(0);flags.area.fill(0);flags.global.fill(0);flags.treasure.fill(0);
 for(int bit:completed)flags.scenario[bit]=1;flags.current_map_id=map;
 const auto* room=baked::submap_by_map_id(map);const auto* dest=baked::submap_by_map_id(target_map);map_state::Resolver state(flags,fixture::records);
 const auto* entry=point_in(*room,1,state);if(!entry){++failures;return;}
 const auto& objects=baked::grids(*dest).objects;const auto actor=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.entity_id()==entity&&o.type!=baked::ObjType::Warp;});
 if(actor==objects.end()){++failures;return;}
 MapGoal goal;goal.map_id=target_map;goal.entity_id=entity;goal.raw_name=baked::object_identifier(*actor);goal.cell={actor->layer,actor->tx,actor->ty};goal.examine=!baked_route::step_event(*actor);
 Cell at{entry->layer,entry->tx,entry->ty};Pending pending;bool reached=false,gated=false;
 for(int hop=0;hop<32;++hop){
  flags.current_map_id=map;RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);snapshot.position=world_of_cell(*snapshot.submap,at);snapshot.layer=at.layer;
  snapshot.vehicle={true,1,1,{}};snapshot.goal=goal;snapshot.kind=goal.examine?TargetKind::Interact:TargetKind::Spot;snapshot.locale=L"en";snapshot.pending=pending;
  for(const auto& original:baked::grids(*snapshot.submap).objects){
   if(original.type==baked::ObjType::Warp||original.type==baked::ObjType::GotoMap||original.type==baked::ObjType::PointIn||!snapshot.state.active(*snapshot.submap,original))continue;
   const auto o=snapshot.state.effective(*snapshot.submap,original);NativeObstacle n;n.id=o.entity_id();n.type=int(o.type);n.raw_name=baked::object_identifier(original);n.cell={o.layer,o.tx,o.ty};n.enabled=n.applies=true;n.trigger=baked_route::step_event(o);
   n.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;n.chest=o.type==baked::ObjType::TreasureBox;n.collider=o.type==baked::ObjType::CollisionEntity;n.collision=o.a;n.transportation_ids=snapshot.state.transportation_mask(*snapshot.submap,original);n.transportation_restricted=n.transportation_ids!=~uint64_t(0);snapshot.obstacles.push_back(std::move(n));
  }
  Planned route;std::thread worker([&]{route=plan_snapshot(snapshot);});worker.join();
  if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach){
   gated=!route.path.success&&route.path.reason==T(L"Story gated");
   if(!expected_gate||!gated)std::printf("FAIL %s from%d L%d %d,%d to%d/%d NPC%d: %ls / %ls\n",name,map,at.layer,at.x,at.y,target_map,entity,route.path.npc_approach,route.path.reason.c_str(),route.path.error.c_str());break;
  }
  if(map==target_map){reached=true;break;}
  const auto receipt=route.pending.incoming_transition?route.pending.incoming_transition:route.pending.incoming_trap;if(!receipt)break;
  pending=route.pending;{SnapshotScope scope(snapshot);snapshot.pending=pending;note_trap_start(receipt->map_id,receipt->entry);record_map_arrival(receipt->destination,receipt->arrival);pending=snapshot.pending;}
  map=receipt->destination;at=receipt->arrival;flags.local.fill(0);
 }
 const bool pass=expected_gate?!reached&&gated:reached;
 std::printf("Library ordinary %s reached%d gated%d expected-gate%d\n",name,reached,gated,expected_gate);++cases;if(!pass)++failures;
}
bool physical_witnesses(const std::vector<policy::Vertex>& graph){
 using namespace field;using namespace baked_nav;journey::Node fresh;
 fresh.state.flags.scenario.fill(0);fresh.state.flags.local.fill(0);fresh.state.flags.area.fill(0);fresh.state.flags.global.fill(0);fresh.state.flags.treasure.fill(0);
 if(!journey::enter(fresh,275,1))return false;
 const auto identity=journey::identity(fresh);
 const auto root=std::find_if(graph.begin(),graph.end(),[&](const auto& v){return journey::identity(v.node)==identity;});
 if(root==graph.end())return false;
 std::vector<int> depth(graph.size(),-1);std::deque<int> pending{int(root-graph.begin())};depth[size_t(pending.front())]=0;
 std::vector<int> found(witnesses.size(),-1);
 // Only states connected to the genuine fresh entrance qualify. A separately
 // seeded retry state cannot make an otherwise impossible target pass.
 while(!pending.empty()){
  const int index=pending.front();pending.pop_front();const auto& vertex=graph[size_t(index)];
  for(const auto& edge:vertex.edges){
   for(size_t i=0;i<witnesses.size();++i)if(found[i]<0&&vertex.node.map==witnesses[i].target_map&&edge.operation.id==witnesses[i].entity)found[i]=depth[size_t(index)]+1;
   if(depth[size_t(edge.to)]<0){depth[size_t(edge.to)]=depth[size_t(index)]+1;pending.push_back(edge.to);}
  }
 }
 for(size_t i=0;i<witnesses.size();++i){++cases;if(found[i]<0)++failures;std::printf("Library native %s target%d/%d physical-operations%d\n",witnesses[i].name,witnesses[i].target_map,witnesses[i].entity,found[i]);}
 return failures==0;
}
}
int main(int argc,char**argv){
 using namespace field;using namespace baked_nav;std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("../tools/ff5_library_20261101/native_programs.json"),programs))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 // Diagnostic only: reproduce the former cover-marker override on the owned
 // offline grids, never native memory. Distinguishes stale witnesses from a
 // regression in the shared physical-wall correction.
 if(argc==2&&std::string(argv[1])=="--legacy-cover") {
  for(int i=0;i<baked::submap_count();++i) {
   auto& grid=const_cast<baked::Grids&>(baked::grids(*baked::submap_at(i)));
   for(int l=0;l<3;++l)if(!grid.passage[l].empty()&&!grid.collision[l].empty())
    for(size_t n=0;n<grid.passage[l].size();++n)if(grid.passage[l][n])grid.collision[l][n]=15;
  }
  std::puts("Diagnostic legacy hidden-cover override enabled on owned grids");
 }else if(argc!=1)return 2;
 for(const auto& spec:witnesses)witness(spec);
 int native_result=2;std::thread worker([&]{native_result=policy::compile(physical_witnesses);});worker.join();
 std::printf("FFV Library witnesses: %d cases, failures%d native%d owner%d\n",cases,failures,native_result,fixture::wrong_owner.load());return failures||native_result||fixture::wrong_owner?1:0;
}
