// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/library_model.cpp
// Actual shipped policy/model/ordered route consumer versus the native-shaped
// comparison graph. Native programs remain external; no game is executed.
#define FFPR_LIBRARY_PRODUCTION_CHECK
#include "ff5_library_policy_check.cpp"
#include "../src/games/ff5/library_model.h"
namespace production {
using namespace field;using namespace baked_nav;
int failures=0,profiles=0,loaded_controls=0,regions=0,covered_cells=0,resume_steps=0;
void check(bool pass,const char* why,int map=0,int id=0){if(!pass){++failures;if(failures<30)std::printf("FAIL Library production %s map%d id%d\n",why,map,id);}}
ff5::library::Snapshot observe(const journey::Node& node) {
 ff5::library::Snapshot s;s.map=node.map;s.player=node.state.at;s.locale=L"en";
 for(size_t i=0;i<journey::bits.size();++i)s.values[i]=node.state.flags.scenario[journey::bits[i]];
 const auto ids=ff5::library::controls(node.map);const auto* map=field::baked::submap_by_map_id(node.map);
 const auto selection=journey::base(node);
 for(size_t i=0;i<ids.size();++i) {
  bool exists=false;
  for(const auto& object:field::baked::grids(*map).objects)if(object.entity_id()==ids[i].id&&field::baked::object_identifier(object)==ids[i].raw&&
      (field::baked::str(object.group)==L"entity_default"||selection.groups.count(field::baked::str(object.group)))){exists=true;break;}
  check(exists,"watched exact control exists in the actual initialized native group",node.map,ids[i].id);if(exists)++loaded_controls;
  s.hidden[i]=!shown(node.state,ids[i].id);
 }return s;
}
bool verify(const std::vector<policy::Vertex>& graph,const json::Value& puzzles) {
 using namespace field;using namespace baked_nav;
 std::map<std::wstring,int> indices;for(size_t i=0;i<graph.size();++i)indices[journey::identity(graph[i].node)]=int(i);
 for(const auto& vertex:graph) {
  ++profiles;const auto observed=observe(vertex.node);const auto advice=ff5::library::plan(observed,puzzles);
  if(vertex.distance==0){check(advice.finished,"exact final Mid/exterior receipt",vertex.node.map);continue;}
  check(advice.supported&&!advice.unknown&&!advice.targets.empty(),"complete bounded model decision",vertex.node.map);
  if(advice.targets.empty())continue;
  auto captured=journey::snapshot(vertex.node,journey::operations(vertex.node).front());captured.candidates=advice.targets;
  const auto result=plan_snapshot(captured);
  check(result.path.success&&!result.path.npc_approach&&result.path.guidance_index>=0,"actual ordered guide consumer",vertex.node.map);
  if(!result.path.success||result.path.guidance_index<0)continue;
  const auto& goal=advice.targets[size_t(result.path.guidance_index)].goal;
  const auto options=journey::operations(vertex.node);
  const auto op=std::find_if(options.begin(),options.end(),[&](const auto& candidate){return candidate.id==(goal.entity_id<0?0:goal.entity_id)&&candidate.cell==goal.cell;});
  check(op!=options.end(),"selected target is a real current operation",vertex.node.map,goal.entity_id);if(op==options.end())continue;
  journey::Node after;
  check(policy::transition(vertex.node,*op,after),"target is the first native contact, never a crossed control",vertex.node.map,op->id);
  const auto next=indices.find(journey::identity(after));check(next!=indices.end(),"native continuation stays in the closed graph",vertex.node.map,op->id);
  if(next!=indices.end())check(graph[size_t(next->second)].distance<vertex.distance,"strict progress prevents every policy loop",vertex.node.map,op->id);
 }
 std::printf("Library production policy: %d profiles/%d exact observed controls; failures%d worker-native%d\n",profiles,loaded_controls,failures,fixture::wrong_owner.load());
 return !failures&&!fixture::wrong_owner;
}

// Ordinary walking cannot change a shelf state. Strongly connected free-floor
// regions therefore have the same available native operations. Enumerate the
// actual directed neighbour graph (including native stairs and collisions),
// then check one resume per reachable region, every occupied contact, and all
// valid manual-operation approach ends. This avoids pretending that the one
// shortest approach chosen by the original graph covers every legal endpoint.
struct Floor {
 int width=0,height=0;std::vector<int> component;std::vector<Cell> representatives;
 int index(Cell c)const{return (c.layer*height+c.y)*width+c.x;}
 Cell cell(int n)const{const int x=n%width;n/=width;return {n/height,x,n%height};}
};
std::wstring geometry(journey::Node n){n.state.at={1,0,0};return journey::identity(n);}
baked_route::Request movement(journey::Node const& node,RouteSnapshot& capture){
 auto request=walk_request(*capture.submap,node.state.at);const auto live=live_blockers(*capture.submap,baked::grids(*capture.submap),{});
 request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
 request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;return request;
}
Floor floor_regions(const baked_route::Request& request){
 Floor out;const auto& grid=baked::grids(*request.submap);out.width=grid.width;out.height=grid.height;
 const int count=3*grid.width*grid.height;check(count<=32768,"finite Library geometry budget");if(count>32768)return out;
 out.component.assign(size_t(count),-1);std::vector<bool> free(size_t(count),false);
 for(int i=0;i<count;++i){const auto at=out.cell(i);
  free[size_t(i)]=baked_route::open(request,grid,at)&&!baked_route::trap_cell(request,grid,at)&&!baked_route::terminal_transition(request,grid,at)&&
   !baked_route::transport_rule_at(request,grid,at,baked_route::TransportRules::Hazard)&&
   std::none_of(request.hazards.begin(),request.hazards.end(),[&](Cell h){return h.x==at.x&&h.y==at.y&&(h.layer<0||h.layer==at.layer);});
 }
 auto edges=std::vector<std::vector<int>>(size_t(count));auto reverse=edges;
 for(int i=0;i<count;++i)if(free[size_t(i)]){std::array<Cell,8> next;const auto n=baked_route::successors(request,out.cell(i),next);
  for(size_t j=0;j<n;++j){const int to=out.index(next[j]);if(to>=0&&to<count&&free[size_t(to)]){edges[size_t(i)].push_back(to);reverse[size_t(to)].push_back(i);}}
 }
 // Iterative Kosaraju: finite storage and no recursion proportional to a room.
 std::vector<bool> visited(size_t(count),false);std::vector<int> order;
 for(int root=0;root<count;++root)if(free[size_t(root)]&&!visited[size_t(root)]){
  std::vector<std::pair<int,size_t>> stack{{root,0}};visited[size_t(root)]=true;
  while(!stack.empty()){auto& [at,edge]=stack.back();if(edge<edges[size_t(at)].size()){int to=edges[size_t(at)][edge++];if(!visited[size_t(to)]){visited[size_t(to)]=true;stack.push_back({to,0});}}
   else{order.push_back(at);stack.pop_back();}}
 }
 for(auto it=order.rbegin();it!=order.rend();++it)if(out.component[size_t(*it)]<0){
  const int id=int(out.representatives.size());out.representatives.push_back(out.cell(*it));std::vector<int> stack{*it};out.component[size_t(*it)]=id;
  while(!stack.empty()){const int at=stack.back();stack.pop_back();for(int to:reverse[size_t(at)])if(out.component[size_t(to)]<0){out.component[size_t(to)]=id;stack.push_back(to);}}
 }return out;
}
bool episode(journey::Node node,const json::Value& puzzles){
 std::set<std::wstring> visited;
 for(int step=0;step<64;++step){
  const auto advice=ff5::library::plan(observe(node),puzzles);
  if(advice.finished)return node.map==272&&node.state.flags.scenario[50]==1;
  if(!visited.insert(journey::identity(node)).second){check(false,"arbitrary-position native episode loops",node.map);return false;}
  if(advice.unknown||advice.targets.empty()){check(false,"arbitrary-position native episode has complete guidance",node.map);return false;}
  const auto options=journey::operations(node);if(options.empty())return false;
  auto captured=journey::snapshot(node,options.front());captured.candidates=advice.targets;const auto result=plan_snapshot(captured);
  if(!result.path.success||result.path.npc_approach||result.path.guidance_index<0){check(false,"arbitrary-position native episode physically routes",node.map);return false;}
  const auto& goal=advice.targets[size_t(result.path.guidance_index)].goal;
  const auto found=std::find_if(options.begin(),options.end(),[&](const auto& op){return op.id==(goal.entity_id<0?0:goal.entity_id)&&op.cell==goal.cell;});
  if(found==options.end()){check(false,"resume does not invent a step-off on inactive contact",node.map,goal.entity_id);return false;}
  journey::Node next;if(!policy::transition(node,*found,next)){check(false,"resume stops at first actual native contact",node.map,found->id);return false;}
  node=std::move(next);++resume_steps;
 }check(false,"finite native episode budget",node.map);return false;
}
bool resumes(const std::vector<policy::Vertex>& graph,const json::Value& puzzles){
 std::map<std::wstring,Floor> floors;std::map<std::wstring,journey::Node> seeds;std::set<std::pair<std::wstring,int>> reached_regions;
 const auto seed=[&](journey::Node n){n.actions.clear();const auto key=journey::identity(n);seeds.emplace(key,std::move(n));};
 for(const auto& vertex:graph){const auto& n=vertex.node;if(!vertex.distance)continue;
  const auto options=journey::operations(n);if(options.empty())continue;auto captured=journey::snapshot(n,options.front());SnapshotScope scope(captured);
  const auto request=movement(n,captured);const auto name=geometry(n);
  auto existing=floors.find(name);if(existing==floors.end())existing=floors.emplace(name,floor_regions(request)).first;
  const auto& floor=existing->second;const auto reach=baked_route::flood(request);
  for(size_t i=0;i<floor.component.size();++i)if(floor.component[i]>=0&&reach.reachable(floor.cell(int(i)))){
   const int id=floor.component[i];if(reached_regions.insert({name,id}).second){auto resume=n;resume.state.at=floor.representatives[size_t(id)];seed(std::move(resume));++regions;}
   // The model's step-off predicate depends on exact position even inside a
   // single walking region. Include inactive baked contacts too: the player
   // can legitimately stand there after the native actor has hidden itself.
   const auto at=floor.cell(int(i));
   for(const auto& object:baked::grids(*captured.submap).objects)if(object.layer==at.layer&&object.contains(at.x,at.y)&&
      (object.type==baked::ObjType::GotoMap||baked_route::step_event(object))){auto resume=n;resume.state.at=at;seed(std::move(resume));break;}
  }
  // Every actual manual endpoint is an independently reachable native action;
  // its resulting cell may lie in a different region after shelves move.
  for(const auto& op:options)if(op.object&&!op.portal&&!baked_route::step_event(*op.object)){
   const auto goals=baked_route::goals_for(request,op.cell,baked_route::GoalKind::Interact,op.object);
   for(Cell end:goals.cells)if(reach.reachable(end)){auto after=n;if(journey::apply(after,op,end))seed(std::move(after));else check(false,"native manual endpoint effect",n.map,op.id);}
  }
 }
 for(const auto& [name,floor]:floors)for(int id:floor.component)if(id>=0&&reached_regions.count({name,id}))++covered_cells;
 check(seeds.size()<=4096,"finite arbitrary-position seed budget");if(seeds.size()>4096)return false;
 int episodes=0;for(const auto& [key,node]:seeds){if(!episode(node,puzzles)&&failures<25)std::printf("Resume failed from%d L%d %d,%d flags%d hidden%d\n",node.map,node.state.at.layer,node.state.at.x,node.state.at.y,policy::flags(node),policy::hidden(node));++episodes;}
 std::printf("Library arbitrary resumes: %zu native layouts/%d walking regions/%d covered cells/%d complete episodes/%d native operations; failures%d\n",floors.size(),regions,covered_cells,episodes,resume_steps,failures);
 return !failures;
}
}
int main(){
 std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto data=read("data/ff5/maps.bin");json::Value puzzles;
 if(!field::baked::load(data.data(),data.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||
    !json::parse(read("data/ff5/puzzles.json"),puzzles)||!json::parse(read("../tools/ff5_library_20261101/native_programs.json"),programs))return 2;
 fixture::game_id=game::Id::FF5;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;
 int result=2;std::thread worker([&]{result=policy::compile([&](const auto& graph){return production::verify(graph,puzzles)&&production::resumes(graph,puzzles);});});worker.join();return result;
}
