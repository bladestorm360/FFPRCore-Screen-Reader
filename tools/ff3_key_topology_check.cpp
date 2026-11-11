// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real floor floods/transition semantics prove the consumable budget before
// routing may treat the13 exact locked doors as manually unlockable.
#define main retained_key_topology_route_main
#include "route_worker_check.cpp"
#undef main
namespace keys {
using namespace field;using namespace baked_nav;
struct Coverage {std::map<int,std::vector<uint8_t>> cells;int arrivals=0;bool valid=true;};
Coverage reach_from(int root,const map_state::Flags& flags){
 Coverage result;map_state::Resolver state(flags,fixture::records);const auto* first=baked::submap_by_map_id(root);if(!first){result.valid=false;return result;}
 const auto* record=fixture::records.get(baked::name_of(*first));const auto* area=record?record->get(L"area_id"):nullptr;const auto* entry=point_in(*first,1,state);if(!area||!entry){result.valid=false;return result;}
 struct Arrival {const baked::Submap* map;Cell at;};std::vector<Arrival> queue{{first,{entry->layer,entry->tx,entry->ty}}};
 for(size_t next=0;next<queue.size();++next){
  if(queue.size()>128){result.valid=false;return result;}const auto [room,at]=queue[next];const auto& grid=baked::grids(*room);
  baked_route::Request request;request.submap=room;request.start=at;request.passages=true;request.ignore_hazards=true;request.transportation_id=1;
  // NPCs do not determine the number of keys. Preserve every other authored
  // geometric gate, collision mask, stair layer and automatic transition.
  map_state::add_obstacles(state,*room,request,true,false,false);
  const auto flood=baked_route::flood(request);auto& bits=result.cells[room->first_map_id];if(bits.empty())bits.assign(size_t(grid.width)*grid.height*3,0);
  for(int layer=0;layer<3;++layer)for(int y=0;y<grid.height;++y)for(int x=0;x<grid.width;++x)if(flood.distance({layer,x,y})>=0)bits[(size_t(layer)*grid.height+y)*grid.width+x]=1;
  for(const auto& edge:edges_of(grid,room->first_map_id)){
   const auto* destination=baked::submap_by_map_id(edge.dest_map_id);if(!destination||destination->world)continue;
   const auto* dr=fixture::records.get(baked::name_of(*destination));const auto* da=dr?dr->get(L"area_id"):nullptr;
   // Floors without an initializer have no mapstate record, but their baked
   // stairs/arrivals are still real. Never discard them as an unknown area.
   if((da&&da->n!=area->n)||!state.active(*room,*edge.object)||state.consumed(*room,*edge.object))continue;
   if(distance_to_object(request,flood,*edge.object,at.layer,false)<0)continue;
   const auto* point=point_in(*destination,edge.dest_point,state);if(!point)continue;
   const Cell arrival{point->layer,point->tx,point->ty};if(std::none_of(queue.begin(),queue.end(),[&](const auto& old){return old.map==destination&&old.at==arrival;}))queue.push_back({destination,arrival});
  }
 }
 result.arrivals=int(queue.size());return result;
}
size_t count(const Coverage& coverage){size_t n=0;for(const auto& [map,bits]:coverage.cells)for(auto bit:bits)n+=bit!=0;return n;}
bool can_approach(const Coverage& coverage,const json::Value& door){
 const int map=int(door.get(L"maps")->arr.front().n),id=int(door.get(L"id")->n);const auto* room=baked::submap_by_map_id(map);if(!room)return false;
 const auto found=coverage.cells.find(room->first_map_id);if(found==coverage.cells.end())return false;
 const auto& grid=baked::grids(*room);baked_route::Request movement;movement.submap=room;movement.passages=true;
 for(const auto& actor:grid.objects)if(actor.type==baked::ObjType::Entity&&actor.entity_id()==id){
  const auto goals=baked_route::goals_for(movement,{actor.layer,actor.tx,actor.ty},baked_route::GoalKind::Interact,&actor);
  for(const auto& cell:goals.cells)if(cell.layer>=0&&cell.layer<3&&baked::in_bounds(grid,cell.x,cell.y)&&found->second[(size_t(cell.layer)*grid.height+cell.y)*grid.width+cell.x])return true;
 }
 return false;
}
int run(const json::Value& facts){
 int failures=0,doors=0;size_t total=0;std::map<int,std::vector<int>> dependencies;
 std::map<int,map_state::Flags> all_states;std::map<int,Coverage> all_current;
 for(int root:{67,219,242}){
  const auto* first=baked::submap_by_map_id(root);if(!first)return 2;const auto* record=fixture::records.get(baked::name_of(*first));if(!record||!record->get(L"area_id"))return 2;
  const int area=int(record->get(L"area_id")->n);map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);flags.current_map_id=root;flags.current_area_id=area;flags.party_mini=0;flags.magic_keys=0;flags.party_thief=false;
  for(int bit=205;bit<=217;++bit)flags.scenario[bit]=0;
  std::vector<int> receipts;
  for(const auto& door:facts.get(L"doors")->arr){const auto* sm=baked::submap_by_map_id(int(door.get(L"maps")->arr.front().n));const auto* sr=sm?fixture::records.get(baked::name_of(*sm)):nullptr;if(sr&&sr->get(L"area_id")&&int(sr->get(L"area_id")->n)==area)receipts.push_back(int(door.get(L"receipt")->n));}
  const auto closed=reach_from(root,flags);auto union_one=closed;auto all_flags=flags;for(int receipt:receipts)all_flags.scenario[receipt]=1;const auto all=reach_from(root,all_flags);
  all_states.emplace(root,all_flags);all_current.emplace(root,all);
  map_state::Resolver before(flags,fixture::records),after(all_flags,fixture::records);
  for(const auto& door:facts.get(L"doors")->arr){const int map=int(door.get(L"maps")->arr.front().n);const auto* sm=baked::submap_by_map_id(map);if(!sm)continue;const auto* sr=fixture::records.get(baked::name_of(*sm));if(!sr||!sr->get(L"area_id")||int(sr->get(L"area_id")->n)!=area)continue;
   for(const auto& actor:baked::grids(*sm).objects)if(actor.type==baked::ObjType::Entity&&actor.entity_id()==int(door.get(L"id")->n)){
    const auto was=before.effective(*sm,actor),now=after.effective(*sm,actor);
    std::printf("Door state map%d id%d: closed active%d used%d at%d,%d; open active%d used%d at%d,%d\n",map,actor.entity_id(),before.active(*sm,actor),before.consumed(*sm,actor),was.tx,was.ty,after.active(*sm,actor),after.consumed(*sm,actor),now.tx,now.ty);
   }
  }
  if(!closed.valid||!all.valid||receipts.empty()){++failures;continue;}
  for(int receipt:receipts){
   auto one_flags=flags;one_flags.scenario[receipt]=1;const auto one=reach_from(root,one_flags);++doors;size_t newly=0;
   if(!one.valid){++failures;continue;}
   for(const auto& [map,bits]:one.cells){auto& combined=union_one.cells[map];if(combined.empty())combined.assign(bits.size(),0);const auto old=closed.cells.find(map);for(size_t i=0;i<bits.size();++i){if(bits[i]&&(old==closed.cells.end()||!old->second[i]))++newly;combined[i]|=bits[i];}}
   std::printf("Key topology root%d receipt%d: single-open %zu cells, +%zu, arrivals%d\n",root,receipt,count(one),newly,one.arrivals);
  }
  size_t uncovered=0;for(const auto& [map,bits]:all.cells){const auto found=union_one.cells.find(map);for(size_t i=0;i<bits.size();++i)if(bits[i]&&(found==union_one.cells.end()||!found->second[i]))++uncovered;}
  auto union_budget=closed;std::map<int,Coverage> all_except;
  for(int receipt:receipts){auto without=all_flags;without.scenario[receipt]=0;all_except.emplace(receipt,reach_from(root,without));}
  for(const auto& door:facts.get(L"doors")->arr){
   const int receipt=int(door.get(L"receipt")->n);if(std::find(receipts.begin(),receipts.end(),receipt)==receipts.end())continue;
   auto& parents=dependencies[receipt];for(int other:receipts)if(other!=receipt&&!can_approach(all_except.at(other),door))parents.push_back(other);
   auto required=flags;required.scenario[receipt]=1;for(int parent:parents)required.scenario[parent]=1;
   const auto minimal=reach_from(root,required);if(!minimal.valid||!can_approach(minimal,door)||parents.size()>1)++failures;
   for(const auto& [map,bits]:minimal.cells){auto& merged=union_budget.cells[map];if(merged.empty())merged.assign(bits.size(),0);for(size_t i=0;i<bits.size();++i)merged[i]|=bits[i];}
   std::printf("Native key prerequisites receipt%d:",receipt);for(int parent:parents)std::printf(" %d",parent);std::printf("; total keys%zu\n",parents.size()+1);
  }
  size_t budget_missing=0;for(const auto& [map,bits]:all.cells){const auto found=union_budget.cells.find(map);for(size_t i=0;i<bits.size();++i)if(bits[i]&&(found==union_budget.cells.end()||!found->second[i]))++budget_missing;}
  failures+=budget_missing!=0;total+=count(all);
  std::printf("Key topology root%d: closed%zu all-open%zu union-one%zu uncovered%zu, %zu doors\n",root,count(closed),count(all),count(union_one),uncovered,receipts.size());
  std::printf("Key prerequisite proof root%d: all budget-valid subsets cover%zu cells, missing%zu\n",root,count(union_budget),budget_missing);
 }
 // Offline ablation only: the former passage-enabled geometry treated every
 // cover cell as a fully open mask. Mutate our owned decoded fixture arrays,
 // use the actual flood/doors/stairs, then restore every byte before inspecting
 // the delta. No game/native memory exists here. This proves the count update
 // removes physical cover-walls rather than silently losing ordinary floor.
 struct Saved {baked::Grids* grid;int layer;size_t index;uint8_t value;};std::vector<Saved> saved;
 for(int n=0;n<baked::submap_count();++n){const auto* sm=baked::submap_at(n);if(sm->world)continue;
  auto& grid=const_cast<baked::Grids&>(baked::grids(*sm));
  for(int layer=0;layer<3;++layer)for(size_t i=0;i<grid.collision[layer].size();++i)
   if(i<grid.passage[layer].size()&&grid.passage[layer][i]&&grid.collision[layer][i]!=255){saved.push_back({&grid,layer,i,grid.collision[layer][i]});grid.collision[layer][i]=255;}
 }
 std::map<int,Coverage> legacy;size_t legacy_total=0;
 for(const auto& [root,flags]:all_states){auto coverage=reach_from(root,flags);failures+=!coverage.valid;legacy_total+=count(coverage);legacy.emplace(root,std::move(coverage));}
 for(const auto& cell:saved)cell.grid->collision[cell.layer][cell.index]=cell.value;
 struct Delta {int map,layer,x,y;};std::vector<Delta> delta;size_t current_only=0;
 for(const auto& [root,coverage]:legacy){const auto& current=all_current.at(root);
  for(const auto& [map,bits]:coverage.cells){const auto* sm=baked::submap_by_map_id(map);const auto& grid=baked::grids(*sm);const auto prior=current.cells.find(map);
   for(size_t i=0;i<bits.size();++i)if(bits[i]&&(prior==current.cells.end()||!prior->second[i])){
    const size_t cells=size_t(grid.width)*grid.height;const int layer=int(i/cells);const size_t at=i%cells;
    const bool wall=grid.collision[layer][at]==0&&at<grid.passage[layer].size()&&grid.passage[layer][at];failures+=!wall;
    delta.push_back({map,layer,int(at%grid.width),int(at/grid.width)});
    std::printf("Key cover delta map%d L%d (%d,%d): physical mask%d cover%d\n",map,layer,int(at%grid.width),int(at/grid.width),grid.collision[layer][at],at<grid.passage[layer].size()?grid.passage[layer][at]:0);
   }
  }
  for(const auto& [map,bits]:current.cells){const auto old=coverage.cells.find(map);for(size_t i=0;i<bits.size();++i)current_only+=bits[i]&&(old==coverage.cells.end()||!old->second[i]);}
  const auto restored=reach_from(root,all_states.at(root));failures+=!restored.valid||restored.cells!=current.cells;
 }
 failures+=legacy_total!=1861||total!=1853||delta.size()!=8||current_only!=0;
 std::printf("Key geometry delta: legacy%zu current%zu excluded%zu cover-walls current-only%zu; owned arrays restored\n",legacy_total,total,delta.size(),current_only);
 if(!failures&&doors==13){std::ofstream proof("build_check/ff3_key_dependencies.json");proof<<"{\"dependencies\":[";bool first=true;for(const auto& [receipt,parents]:dependencies){if(!first)proof<<',';first=false;proof<<"{\"receipt\":"<<receipt<<",\"parents\":[";for(size_t i=0;i<parents.size();++i){if(i)proof<<',';proof<<parents[i];}proof<<"]}";}proof<<"],\"max_keys\":2,\"all_open_cells\":"<<total<<",\"legacy_all_open_cells\":"<<legacy_total<<",\"cover_wall_delta\":[";for(size_t i=0;i<delta.size();++i){if(i)proof<<',';const auto& c=delta[i];proof<<"{\"map\":"<<c.map<<",\"layer\":"<<c.layer<<",\"x\":"<<c.x<<",\"y\":"<<c.y<<",\"collision\":0,\"cover\":1}";}proof<<"]}\n";}
 std::printf("FFIII consumable topology: %d doors/%zu all-open cells, failures%d owner%d\n",doors,total,failures,fixture::wrong_owner.load());return failures||doors!=13||fixture::wrong_owner?1:0;
}
}
#ifndef FFPR_KEY_TOPOLOGY_FIXTURE_ONLY
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff3/maps.bin");json::Value facts;if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records)||!json::parse(read("docs/re/ff3_magic_key_gates_20261101.json"),facts))return 2;fixture::game_id=game::Id::FF3;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=keys::run(facts);});worker.join();return result;}
#endif
