// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Offline policy compiler. Native comparison input stays outside every Git tree.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include <climits>
namespace ship_policy {
using namespace field;using namespace baked_nav;
constexpr int width=25,height=27;constexpr char alphabet[]=".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh";
constexpr int ids[]{21,19,20,23,24,26,25,22},dx[]{0,1,0,-1},dy[]{-1,0,1,0};
json::Value facts;const baked::Submap* room=nullptr;
struct Edge{int code,to;};struct Node{int mask;Cell at;baked_route::Reach reach;std::vector<Edge> edges;int distance=INT_MAX;};
std::vector<Node> nodes;std::map<std::string,int> seen;
std::function<bool()> consumer;
map_state::Flags flags(int mask){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.current_map_id=270;for(int i=0;i<7;++i)f.scenario[301+i]=(mask>>i)&1;return f;}
const baked::Object& actor(int id){for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Entity&&o.entity_id()==id)return o;std::abort();}
Cell stand(int code){const int c=(code-1)/4,d=(code-1)%4;const auto& o=actor(ids[c]);return{o.layer,o.tx-dx[d],o.ty-dy[d]};}
baked_route::Request request(int mask,Cell at){map_state::Resolver r(flags(mask),fixture::records);baked_route::Request q;q.submap=room;q.start=at;q.passages=true;q.transportation_id=1;q.transitions=r.transitions(*room);map_state::add_obstacles(r,*room,q);map_state::add_relays(r,*room,q);
 // A native exit ends this room. Never use it as an indoor walking passage.
 for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::GotoMap)q.blocked.push_back({o.layer,o.tx,o.ty});return q;}
int add(int mask,Cell at){auto q=request(mask,at);auto reach=baked_route::flood(q);std::string key=std::to_string(mask)+":";for(int y=0;y<height;++y)for(int x=0;x<width;++x)key+=reach.reachable({1,x,y})?'1':'0';const auto old=seen.find(key);if(old!=seen.end())return old->second;const int index=int(nodes.size());seen[key]=index;nodes.push_back({mask,at,std::move(reach)});return index;}
// Back-propagate the exact switch outcomes from each selected object's
// reachable approaches. A zero-operation row lets ordinary routing own it.
bool write_target_rows(std::ostream& out)
{
 const auto& objects=baked::grids(*room).objects;
 out << "\"target_rows\":{";
 for(int entity:{15,17,19,20,21,22,23,24,25,26}){
  const auto type=entity==15?baked::ObjType::GotoMap:entity==17?baked::ObjType::TreasureBox:baked::ObjType::Entity;
  const auto target=std::find_if(objects.begin(),objects.end(),[&](const auto& o){return o.entity_id()==entity&&o.type==type;});
  if(target==objects.end())return false;
  std::vector<int> distances(nodes.size(),INT_MAX);
  for(size_t i=0;i<nodes.size();++i){
   const auto& node=nodes[i];
   if(entity==15){if(node.reach.reachable({1,10,21}))distances[i]=0;continue;}
   const auto movement=request(node.mask,node.at);
   const auto goals=baked_route::goals_for(movement,{target->layer,target->tx,target->ty},
       baked_route::GoalKind::Interact,&*target);
   if(std::any_of(goals.cells.begin(),goals.cells.end(),[&](Cell at){return node.reach.reachable(at);}))distances[i]=0;
  }
  bool changed=true;
  while(changed){
   changed=false;
   for(size_t i=0;i<nodes.size();++i)for(auto edge:nodes[i].edges){
    if(distances[edge.to]==INT_MAX||distances[edge.to]+1>=distances[i])continue;
    distances[i]=distances[edge.to]+1;changed=true;
   }
  }
  std::vector<std::string> rows(128,std::string(width*height,'.'));
  int direct=0,staged=0;
  for(size_t i=0;i<nodes.size();++i){
   if(distances[i]==0){++direct;continue;}
   if(distances[i]==INT_MAX)continue;
   const auto& node=nodes[i];
   const auto step=std::find_if(node.edges.begin(),node.edges.end(),[&](auto edge){return distances[edge.to]==distances[i]-1;});
   if(step==node.edges.end())return false;
   ++staged;
   for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(node.reach.reachable({1,x,y})){
    auto& code=rows[node.mask][y*width+x];
    if(code!='.'&&code!=alphabet[step->code])return false;
    code=alphabet[step->code];
   }
  }
  std::printf("Ship target %d: %d direct, %d staged, %zu without a verified route\n",entity,direct,staged,nodes.size()-direct-staged);
  if(!direct)return false;
  if(entity!=15)out << ',';
  out << '"' << entity << "\":[";
  for(size_t i=0;i<rows.size();++i){
   if(i)out<<',';
   out << '"';
   if(rows[i].find_first_not_of('.')!=std::string::npos)out<<rows[i];
   out << '"';
  }
  out << ']';
 }
 out << '}';
 return true;
}
int compile(){
 const auto* layouts=facts.get(L"layouts"),*effects=facts.get(L"transitions");if(!layouts||layouts->arr.size()!=128||!effects||effects->arr.size()!=4096)return 2;
 int comparisons=0;
 for(int mask=0;mask<128;++mask){map_state::Resolver r(flags(mask),fixture::records);if(!r.known(*room))return 3;
  for(const auto& [key,a]:layouts->arr[mask].obj){const int id=std::stoi(key);const auto& source=actor(id);const auto effective=r.effective(*room,source);const bool visible=r.active(*room,source,true);
   if(visible!=a.get(L"visible")->b||(visible&&(effective.tx!=a.get(L"x")->n||effective.ty!=a.get(L"y")->n||effective.layer!=int(a.get(L"layer")->n)))){std::printf("FAIL source layout %d actor%d projected%d/%d,%d,%d native%d/%g,%g,%g\n",mask,id,visible,effective.layer,effective.tx,effective.ty,a.get(L"visible")->b,a.get(L"layer")->n,a.get(L"x")->n,a.get(L"y")->n);return 4;}++comparisons;
  }
 }
 const int first=add(0,{1,10,21}),back=add(0,{1,18,8});size_t edges=0;
 for(size_t i=0;i<nodes.size();++i){if(nodes.size()>4096||edges>65536)return 5;const auto mask=nodes[i].mask;const auto reach=nodes[i].reach;std::vector<Edge> out;
  if(reach.reachable({1,18,8}))nodes[i].distance=0;
  // The full adjacent269 initializer resets all seven bits. Leaving through
  // the real lower exit and returning is a native recovery,not a flag write.
  if(reach.reachable({1,10,21})&&int(i)!=first){out.push_back({34,first});++edges;}
  for(int code=1;code<=32;++code){const auto at=stand(code);if(!reach.reachable(at))continue;const auto& e=effects->arr[mask*32+code-1];if(int(e.get(L"mask")->n)!=mask||int(e.get(L"control")->n)!=(code-1)/4+1||int(e.get(L"direction")->n)!=(code-1)%4)return 6;
   Cell end=at;const auto* moved=e.get(L"player");if(moved->type==json::Value::Type::Array)end={int(moved->arr[2].n),int(moved->arr[0].n),int(moved->arr[1].n)};
   const int to=add(int(e.get(L"after")->n),end);if(to!=int(i)){out.push_back({code,to});++edges;}
  }nodes[i].edges=std::move(out);
 }
 bool changed=true;while(changed){changed=false;for(auto& n:nodes)for(auto e:n.edges)if(nodes[e.to].distance<INT_MAX&&nodes[e.to].distance+1<n.distance){n.distance=nodes[e.to].distance+1;changed=true;}}
 int dead=0;std::vector<std::string> rows(128,std::string(width*height,'.'));std::vector<int> action(nodes.size());
 for(size_t i=0;i<nodes.size();++i){const auto& n=nodes[i];if(n.distance==INT_MAX){++dead;if(dead<=12)std::printf("Unsolved mask%d at%d,%d\n",n.mask,n.at.x,n.at.y);continue;}int code=33;if(n.distance){code=0;for(auto e:n.edges)if(nodes[e.to].distance+1==n.distance){code=e.code;break;}if(!code)return 7;}action[i]=code;
  for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(n.reach.reachable({1,x,y})){auto& c=rows[n.mask][y*width+x];if(c!='.'&&c!=alphabet[code])return 8;c=alphabet[code];}
 }
 std::printf("Ship policy:%zu legal states/%zu edges/%d native actor comparisons/dead%d;initial%d reverse%d\n",nodes.size(),edges,comparisons,dead,nodes[first].distance,nodes[back].distance);
 if(dead||nodes[first].distance==INT_MAX||nodes[back].distance==INT_MAX)return 9;
 // Every reachable node must strictly decrease native remaining operations;
 // this includes unchanged-flag carriers into a different walking region.
 for(size_t i=0;i<nodes.size();++i){size_t at=i;for(size_t k=0;k<=nodes.size();++k){if(!nodes[at].distance)break;const auto& n=nodes[at];const auto e=std::find_if(n.edges.begin(),n.edges.end(),[&](auto e){return e.code==action[at];});if(e==n.edges.end()||nodes[e->to].distance>=n.distance)return 10;at=e->to;if(k==nodes.size())return 11;}}
 if(consumer&&!consumer())return 13;
 std::ofstream output("build_check/ff5_fire_ship_policy.json");output<<"{\"rows\":[";for(int m=0;m<128;++m){if(m)output<<',';output<<'\"'<<rows[m]<<'\"';}output<<"],\"states\":"<<nodes.size()<<",\"edges\":"<<edges<<",\"comparisons\":"<<comparisons<<',';
 if(!write_target_rows(output))return 14;
 output<<"}\n";
 return fixture::wrong_owner?12:0;
}
}
int main(){using namespace ship_policy;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("../tools/puzzle_audit_20261030/broader_audit_20261104/ff5_FireShip_Context/physical_facts.json"),facts))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;room=baked::submap_by_map_id(270);int result=2;std::thread worker([&]{result=compile();});worker.join();return result;}
