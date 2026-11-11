// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Original finite macro graph. Native direction rectangles are primary input,
// independent of the router's forced-edge and spoken-current implementation.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include <climits>
namespace desert_policy {
using namespace field;using namespace baked_nav;
constexpr int size=4096,width=64;constexpr char alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const baked::Submap* room=nullptr;json::Value proof;std::array<int,size> directions{},scripted{};
struct Edge{int entry,to;};
std::array<std::vector<Edge>,size> graph;std::array<int,size> distance{},next{};
std::function<bool(int,const std::string&)> consumer;
map_state::Flags flags(int later){map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.scenario[58]=f.scenario[59]=f.scenario[287]=1;f.scenario[63]=later;f.current_map_id=286;f.current_area_id=35;return f;}
Cell cell(int at){return {2,at%width,at/width};}
int index(Cell c){return c.layer==2&&c.x>=0&&c.x<64&&c.y>=0&&c.y<64?c.y*width+c.x:-1;}
int direction(int a){return a==1?0:a==2?1:a==4?2:a==8?3:-1;}
int compile(){
 const auto* areas=proof.get(L"sliding_areas");if(!areas||areas->arr.size()!=274)return 2;
 int total=0;for(const auto& a:areas->arr){const auto& c=a.get(L"cell")->arr;const auto& z=a.get(L"size")->arr;for(int y=int(c[1].n);y<int(c[1].n)+int(z[1].n)/16;++y)for(int x=int(c[0].n);x<int(c[0].n)+int(z[0].n)/16;++x){const int at=y*64+x;if(at<0||at>=size||directions[at]||direction(int(a.get(L"direction")->n))<0)return 3;directions[at]=int(a.get(L"direction")->n);++total;}}
 if(total!=957)return 4;scripted.fill(-1);
 for(const auto& a:proof.get(L"actors")->arr)if(a.get(L"type")->s==L"Event"){
  const int id=int(a.get(L"entity")->n);if(id==65||id==26)continue;const auto& c=a.get(L"cell")->arr;const int at=(int(c[2].n)-1)*64+int(c[1].n);
  const bool left=id==13||id==15||id==17||id==20||id==21||id==829||id==502||id==504;scripted[at]=14*64+(left?52:53);
 }
 const auto& g=baked::grids(*room);for(int at=0;at<size;++at)if((baked::current_bits(g,2,at%64,at/64)&15)!=directions[at])return 5;
 constexpr int dx[]{0,1,0,-1},dy[]{-1,0,1,0},bits[]{4,1,8,2},reverse[]{2,3,0,1};
 std::ofstream output("build_check/ff5_desert_policy.json",std::ios::binary);output<<"{\"rows\":[";
 for(int later=0;later<2;++later){
  map_state::Resolver state(flags(later),fixture::records);if(!state.known(*room))return 6;
  baked_route::Request q;q.submap=room;q.passages=true;q.transportation_id=1;q.transitions=state.transitions(*room);q.ignore_hazards=true;q.goals_open=true;
  map_state::add_obstacles(state,*room,q,true,false,true);map_state::add_relays(state,*room,q);
  const int goal=58*64+49;q.goals={cell(goal)};for(int at=0;at<size;++at)if(scripted[at]>=0)q.goals.push_back(cell(at));
  std::array<std::vector<int>,size> reverse_edges;int edges=0,automatic=0;
  for(auto& row:graph)row.clear();
  for(int from=0;from<size;++from){
   if(directions[from]||scripted[from]>=0||!baked::walkable(g,2,from%64,from/64,true)||from==goal)continue;
   if(from==7*64+53)continue; // Native north exit ends the room.
   std::array<Cell,8> options;const auto count=baked_route::successors(q,cell(from),options);
   for(size_t k=0;k<count;++k){const int entry=index(options[k]);if(entry<0)continue;int at=entry;std::set<int> seen;bool valid=true;
    while(directions[at]||scripted[at]>=0){
     if(!seen.insert(at).second){valid=false;break;}
     if(scripted[at]>=0){at=scripted[at];continue;}
     const int d=direction(directions[at]);const Cell a=cell(at),b{2,a.x+dx[d],a.y+dy[d]};const int to=index(b);
     if(to<0||!(g.collision[2][at]&bits[d])||!(g.collision[2][to]&bits[reverse[d]])){valid=false;break;}
     at=to;
    }
    if(!valid||at==from)continue;graph[from].push_back({entry,at});reverse_edges[at].push_back(from);++edges;if(entry!=at)++automatic;
   }
  }
  distance.fill(INT_MAX);next.fill(-1);distance[goal]=0;std::vector<int> queue{goal};
  for(size_t i=0;i<queue.size();++i)for(int from:reverse_edges[queue[i]])if(distance[from]==INT_MAX){distance[from]=distance[queue[i]]+1;queue.push_back(from);}
  std::string row(size*2,'.');int solved=0;
  for(int from=0;from<size;++from)if(distance[from]!=INT_MAX){
   int at=from,target=goal;
   while(distance[at]>0){const auto e=std::find_if(graph[at].begin(),graph[at].end(),[&](auto e){return distance[e.to]!=INT_MAX&&distance[e.to]+1==distance[at];});if(e==graph[at].end())return 7;
    if(e->entry!=e->to){target=e->entry;break;}at=e->to;
   }
   next[from]=target;row[from*2]=alphabet[target>>6];row[from*2+1]=alphabet[target&63];++solved;
  }
  for(int at:{8*64+53,57*64+49,13*64+53,14*64+52,14*64+53})if(distance[at]==INT_MAX){std::printf("FAIL native landing%d,%d later%d\n",at%64,at/64,later);return 8;}
  std::printf("Desert policy layout%d:%d solved rests/%d edges/%d automatic actions;callback remaining%d\n",later,solved,edges,automatic,distance[13*64+53]);
  if(consumer&&!consumer(later,row))return 9;
  if(later)output<<',';output<<'\"'<<row<<'\"';
 }
 output<<"]}\n";return fixture::wrong_owner?10:0;
}
}
int main(){using namespace desert_policy;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("docs/re/ff5_desert_native_20261105.json"),proof))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;room=baked::submap_by_map_id(286);int result=2;std::thread worker([&]{result=compile();});worker.join();return result;}
