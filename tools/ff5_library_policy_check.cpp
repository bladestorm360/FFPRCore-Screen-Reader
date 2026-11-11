// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Compile ORIGINAL guide decisions from the separately stored primary fixture.
// The shipped table contains observation keys and native destination identities,
// never game bytecode or reconstructed implementations. All solving is offline.
#define FFPR_LIBRARY_POLICY_CHECK
#include "ff5_library_story_check.cpp"
#include <climits>
namespace policy {
using namespace journey;
struct Edge {Operation operation;int to=0;};
struct Vertex {Node node;std::vector<Edge> edges;int distance=INT_MAX;};
std::vector<int> observed(int map){if(map==275)return {9,10,14,15,16,17,21,27};if(map==281)return {13,14,15,16,21};return {};}
int flags(const Node& n){int mask=0;for(size_t i=0;i<bits.size();++i)if(n.state.flags.scenario[bits[i]]==1)mask|=1<<i;return mask;}
int hidden(const Node& n){int mask=0;const auto ids=observed(n.map);for(size_t i=0;i<ids.size();++i)if(!shown(n.state,ids[i]))mask|=1<<i;return mask;}
std::string observation(const Node& n){return std::to_string(n.map)+":"+std::to_string(flags(n))+":"+std::to_string(hidden(n));}
std::string operation_key(const Operation& o){return std::to_string(o.id)+":"+std::to_string(o.cell.layer)+":"+std::to_string(o.cell.x)+":"+std::to_string(o.cell.y);}
bool transition(const Node& node,const Operation& op,Node& next){
 auto captured=snapshot(node,op);const auto route=plan_snapshot(captured);if(!route.path.success||route.path.walk_phases.empty()||route.path.npc_approach)return false;
 auto actual=op;auto end=route.path.walk_phases.back().cells.back();bool contact=false,moved=false;const auto ops=operations(node);
 for(const auto& phase:route.path.walk_phases){for(auto cell:phase.cells){if(cell==node.state.at)continue;moved=true;
  for(const auto& candidate:ops)if(candidate.object&&(candidate.portal||baked_route::step_event(*candidate.object))&&candidate.object->layer==cell.layer&&candidate.object->contains(cell.x,cell.y)){actual=candidate;end=cell;contact=true;break;}if(contact)break;
 }if(contact)break;}
 if(!moved&&(actual.portal||actual.waypoint||(actual.object&&baked_route::step_event(*actual.object))))return false;
 // Every table target must be the actual first native operation.
 if(operation_key(actual)!=operation_key(op))return false;
 next=node;next.actions.clear();return apply(next,actual,end);
}
int compile(const std::function<bool(const std::vector<Vertex>&)>& consumer={}){
 std::vector<Vertex> graph;std::map<std::wstring,int> indices;
 const auto add=[&](Node n){n.actions.clear();const auto k=identity(n);const auto old=indices.find(k);if(old!=indices.end())return old->second;const int id=int(graph.size());indices[k]=id;graph.push_back({std::move(n)});return id;};
 for(auto [map,point,mask]:std::vector<std::array<int,3>>{{275,1,0},{279,1,1|64|128},{278,4,1|8|64},{281,1,1|8|64},{283,1,1|8|64},{283,1,1|2|8|64},{281,3,1|2|4|8}}){
  Node n;n.state.flags.scenario.fill(0);n.state.flags.treasure.fill(0);n.state.flags.global.fill(0);n.state.flags.area.fill(0);n.state.flags.local.fill(0);for(size_t i=0;i<bits.size();++i)if(mask&(1<<i))n.state.flags.scenario[bits[i]]=1;
  if(!enter(n,map,point))return 2;add(std::move(n));
 }
 size_t edge_count=0;
 for(size_t v=0;v<graph.size();++v){
  if(graph.size()>8192||edge_count>65536)return 3;const Node node=graph[v].node;
  if(node.map==272&&node.state.flags.scenario[50]==1){graph[v].distance=0;continue;}
  std::vector<Edge> edges;for(const auto& op:operations(node)){Node next;if(transition(node,op,next)){edges.push_back({op,add(std::move(next))});++edge_count;}}
  graph[v].edges=std::move(edges);
 }
 bool changed=true;while(changed){changed=false;for(auto& v:graph)for(auto e:v.edges)if(graph[e.to].distance!=INT_MAX&&graph[e.to].distance+1<v.distance){v.distance=graph[e.to].distance+1;changed=true;}}
 // The production-consumer check reuses this native-shaped graph instead of
 // duplicating its comparison interpreter or reconstructing a runtime solver.
 if(consumer&&!consumer(graph))return 1;
 struct Choice {Operation operation;int worst=0;};std::map<std::string,std::map<std::string,Choice>> groups;
 int dead=0;for(const auto& v:graph){if(v.distance==INT_MAX){++dead;continue;}if(v.distance==0)continue;auto& group=groups[observation(v.node)];for(const auto& e:v.edges){auto& into=group[operation_key(e.operation)];into.operation=e.operation;into.worst=std::max(into.worst,graph[e.to].distance);}}
 std::map<std::string,std::vector<Choice>> ordered;for(const auto& [key,group]:groups){auto& out=ordered[key];for(const auto& [id,choice]:group)if(choice.worst<INT_MAX)out.push_back(choice);std::stable_sort(out.begin(),out.end(),[](auto a,auto b){return a.worst<b.worst;});}
 int failures=0;for(const auto& v:graph){if(v.distance==INT_MAX||!v.distance)continue;bool picked=false;for(const auto& c:ordered[observation(v.node)]){const auto found=std::find_if(v.edges.begin(),v.edges.end(),[&](auto e){return operation_key(e.operation)==operation_key(c.operation);});if(found==v.edges.end())continue;picked=true;if(graph[found->to].distance>=v.distance){++failures;if(failures<15)std::printf("FAIL policy %s at L%d %d,%d id%d distance%d next%d\n",observation(v.node).c_str(),v.node.state.at.layer,v.node.state.at.x,v.node.state.at.y,c.operation.id,v.distance,graph[found->to].distance);}break;}if(!picked)++failures;}
 std::printf("Library policy closure: %zu states/%zu edges/%zu observations; dead%d greedy failures%d owner%d\n",graph.size(),edge_count,groups.size(),dead,failures,fixture::wrong_owner.load());
 if(dead||failures)return 1;
 std::ofstream out("build_check/ff5_library_policy.json");out<<"{\"version\":1,\"rows\":[";bool first=true;
 for(const auto& [key,choices]:ordered){if(!first)out<<',';first=false;out<<"{\"key\":\""<<key<<"\",\"targets\":[";bool first_choice=true;for(const auto& c:choices){if(!first_choice)out<<',';first_choice=false;const auto& o=c.operation;out<<'['<<o.id<<','<<o.cell.layer<<','<<o.cell.x<<','<<o.cell.y<<','<<int(o.object&&!o.portal&&!baked_route::step_event(*o.object))<<']';}out<<"]}";}out<<"]}\n";return 0;
}
}
#ifndef FFPR_LIBRARY_PRODUCTION_CHECK
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("../tools/ff5_library_20261101/native_programs.json"),programs))return 2;fixture::game_id=game::Id::FF5;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=policy::compile();});worker.join();return result;}
#endif
