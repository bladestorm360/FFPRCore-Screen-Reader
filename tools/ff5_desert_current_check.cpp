// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Finite physical investigation only. Native areas are separate primary input;
// no production current graph/guide is certified by this diagnostic alone.
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
namespace desert_probe {
using namespace field;using namespace baked_nav;
int investigate(const json::Value& proof){
 const auto* room=baked::submap_by_map_id(286);if(!room)return 2;const int width=room->width,height=room->height;std::printf("Desert baked shape:%dx%d\n",width,height);if(width!=64||height!=64)return 3;
 const auto* areas=proof.get(L"sliding_areas");if(!areas||areas->arr.size()!=274)return 4;
 std::vector<int> directions(width*height),owners(width*height),next(width*height,-1);int overlaps=0,cells=0;
 for(const auto& a:areas->arr){const auto& c=a.get(L"cell")->arr;const auto& z=a.get(L"size")->arr;const int x=int(c[0].n),y=int(c[1].n),w=int(z[0].n)/16,h=int(z[1].n)/16,d=int(a.get(L"direction")->n),id=int(a.get(L"entity")->n);
  if(!a.get(L"enabled")->b||a.get(L"seconds")->n||a.get(L"all_layers")->b||a.get(L"layer")->s!=L"UpperObject"||a.get(L"group")->s!=L"entity_default"||!w||!h||x<0||y<0||x+w>width||y+h>height)return 5;
  for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx){const int at=(y+dy)*width+x+dx;if(directions[at]){++overlaps;if(directions[at]!=d)return 6;}else ++cells;directions[at]=d;owners[at]=id;}
 }
 map_state::Flags f;f.scenario.fill(0);f.treasure.fill(0);f.global.fill(0);f.area.fill(0);f.local.fill(0);f.current_map_id=286;f.scenario[58]=f.scenario[59]=f.scenario[287]=1;map_state::Resolver state(f,fixture::records);
 baked_route::Request q;q.submap=room;q.passages=true;q.transportation_id=1;q.transitions=state.transitions(*room);q.ignore_hazards=true;q.goals_open=false;q.max_expansions=8;map_state::add_obstacles(state,*room,q);map_state::add_relays(state,*room,q);
 int blocked=0,bad_terrain=0;for(int at=0;at<width*height;++at)if(directions[at]){const int d=directions[at],dx=d==2?1:d==8?-1:0,dy=d==1?-1:d==4?1:0;if(d!=1&&d!=2&&d!=4&&d!=8)return 7;Cell from{2,at%width,at/width},to{2,from.x+dx,from.y+dy};q.start=from;q.goals={to};
  if(!baked::walkable(baked::grids(*room),2,from.x,from.y,true))++bad_terrain;const auto r=baked_route::find(q);if(!r.found||r.steps()!=1){++blocked;if(blocked<=16)std::printf("Blocked native current area%d cell%d,%d d%d ->%d,%d\n",owners[at],from.x,from.y,d,to.x,to.y);}else next[at]=to.y*width+to.x;
 }
 int cycles=0,longest=0,stuck=0;std::set<int> ends;for(int start=0;start<width*height;++start)if(directions[start]){std::set<int> seen;int at=start,steps=0;while(directions[at]){if(next[at]<0){++stuck;break;}if(!seen.insert(at).second){++cycles;break;}at=next[at];if(++steps>4096)return 8;}if(!directions[at])ends.insert(at);longest=std::max(longest,steps);}
 std::printf("Native currents:274 areas/%d cells/overlaps%d/blocked%d/closed source tiles%d/cyclic starts%d/stuck starts%d/%zu stable landings/longest%d cells\n",cells,overlaps,blocked,bad_terrain,cycles,stuck,ends.size(),longest);for(int end:ends)std::printf("Landing %d,%d\n",end%width,end/width);
 // A current's interior is not a place where another manual step begins.
 // Over-approximate lawful resting-position input using all native collision
 // edges,even omitting NPC blockers. If a wall-stop cell is unreachable here,
 // no ordinary actor can make it reachable in the narrower actual layout.
 std::map<int,int> scripts;
 for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::Event&&state.active(*room,o)){
  const int id=o.entity_id();int x=-1;
  if(id==13||id==15||id==17||id==20||id==21||id==829||id==502||id==504)x=52;
  if(id==14||id==16||id==22||id==23||id==24||id==25||id==828||id==503||id==505)x=53;
  if(x>=0)scripts[o.ty*width+o.tx]=14*width+x;
 }
 q.blocked.clear();q.directional.clear();q.goals_open=true;q.goals.clear();std::set<int> exits;
 for(const auto& o:baked::grids(*room).objects)if(o.type==baked::ObjType::GotoMap){q.goals.push_back({o.layer,o.tx,o.ty});exits.insert(o.ty*width+o.tx);}
 std::set<int> legal;std::vector<int> pending;const auto add=[&](int at){if(legal.insert(at).second)pending.push_back(at);};for(Cell c:{Cell{2,53,8},Cell{2,49,57},Cell{2,53,13},Cell{2,52,14},Cell{2,53,14}})add(c.y*width+c.x);
 int unsafe_inputs=0,current_entries=0;std::set<int> entered_currents;
 for(size_t i=0;i<pending.size();++i){const int from=pending[i];if(exits.count(from))continue;if(directions[from])return 10;std::array<Cell,8> options;const auto count=baked_route::successors(q,{2,from%width,from/width},options);
  for(size_t k=0;k<count;++k){int at=options[k].y*width+options[k].x;std::set<int> chain;
   while(true){const auto scripted=scripts.find(at);if(scripted!=scripts.end()){at=scripted->second;continue;}if(!directions[at])break;entered_currents.insert(at);++current_entries;if(next[at]<0){++unsafe_inputs;at=-1;break;}if(!chain.insert(at).second)return 11;at=next[at];}
   if(at>=0)add(at);
  }
 }
 std::printf("Rest-position over-approximation:%zu rests/%zu actually entered current cells/%d current traversals/%d reachable blocked-current entries;exits",legal.size(),entered_currents.size(),current_entries,unsafe_inputs);for(int at:exits)std::printf(" %d,%d=%d",at%width,at/width,int(legal.count(at)));std::puts("");
 RouteSnapshot captured(map_state::Resolver(f,fixture::records));captured.map_id=286;captured.submap=room;captured.layer=2;captured.position=world_of_cell(*room,{2,53,13});captured.vehicle={true,1,baked::transport(1)->type,{}};captured.locale=L"en";captured.vehicle_shortcuts=false;captured.goal.map_id=286;captured.goal.entity_id=64;captured.kind=TargetKind::Transition;
 for(const auto& source:baked::grids(*room).objects){
  if(source.type==baked::ObjType::GotoMap&&source.entity_id()==64){captured.goal.raw_name=baked::object_identifier(source);captured.goal.cell={source.layer,source.tx,source.ty};}
  if(source.type==baked::ObjType::PointIn||source.type==baked::ObjType::GotoMap||source.type==baked::ObjType::Warp||!captured.state.active(*room,source,true))continue;
  const auto o=captured.state.effective(*room,source);NativeObstacle n;n.type=int(o.type);n.id=o.entity_id();n.raw_name=baked::object_identifier(source);n.cell={o.layer,o.tx,o.ty};if(n.id==27)n.cell={2,55,13};if(n.id==28)n.cell={2,51,13};n.npc=o.type==baked::ObjType::NPC;n.trigger=baked_route::step_event(o);n.collision=o.a;captured.obstacles.push_back(n);
 }
 const auto planned=plan_snapshot(captured);int contrary=0,checked=0;
 for(const auto& phase:planned.path.walk_phases)for(size_t i=1;i<phase.cells.size();++i){const auto a=phase.cells[i-1],b=phase.cells[i];if(a.layer!=2||!baked::in_bounds(baked::grids(*room),a.x,a.y))continue;const int at=a.y*width+a.x;if(!directions[at])continue;++checked;if(next[at]<0||b.layer!=2||b.y*width+b.x!=next[at])++contrary;}
 std::printf("Full production southern-exit route:success%d/%d current edges/%d contrary native displacements;required trap%d\n",planned.path.success,checked,contrary,planned.path.required_trap);
 // Diagnostic evidence,not a release pass:the missing native mechanisms
 // remain active work even when their extracted area geometry is coherent.
 return fixture::wrong_owner?9:0;
}
}
int main(){using namespace desert_probe;std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff5/maps.bin");json::Value proof;if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records)||!json::parse(read("docs/re/ff5_desert_native_20261105.json"),proof))return 2;fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;int result=2;std::thread worker([&]{result=investigate(proof);});worker.join();return result;}
