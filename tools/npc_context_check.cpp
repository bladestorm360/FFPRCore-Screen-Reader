// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Finite22 exact-source geometry diagnostics; counterfactuals are never routes.
#include "field/map_state.h"
#include <cstdio>
#include <fstream>
#include <iterator>
int main(){
 namespace b=field::baked;namespace r=field::baked_route;
 auto read=[](const std::string& path){std::ifstream file(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(file),{});};
 json::Value inventory;if(!json::parse(read("../tools/npc_geometry_context_20261029/contexts.json"),inventory))return 2;
 const auto* cases=inventory.get(L"cases");if(!cases||cases->arr.size()!=22)return 2;int inspected=0;
 for(int n:{2,5,6}){
  const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");if(!b::load(bytes.data(),bytes.size()))return 2;
  json::Value records;if(!json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),records))return 2;
  for(const auto& row:cases->arr){
   if(int(row.get(L"game")->n)!=n)continue;
   const auto* sm=b::submap_by_name(row.get(L"submap")->s);if(!sm)return 2;
   const auto& grid=b::grids(*sm);const b::Object* npc=nullptr;
   for(const auto& o:grid.objects)if((o.type==b::ObjType::NPC||o.type==b::ObjType::ShopNPC)&&o.entity_id()==int(row.get(L"id")->n)&&
       b::str(o.group)==row.get(L"group")->s&&b::str(o.name)==row.get(L"raw")->s)npc=&o;
   if(!npc)return 2;++inspected;
   r::Request request;request.submap=sm;request.passages=true;request.ignore_hazards=true;request.live_relays=true;
   std::vector<r::Cell> seeds,all_layers;
   for(const auto& o:grid.objects){
    if(o.type==b::ObjType::RelayInteraction){const auto group=b::str(o.group);if(group.empty()||group==L"entity_default"||group==L"default"||o.group==npc->group)request.relays.push_back(o);}
    if(o.type==b::ObjType::PointIn){seeds.push_back({o.layer,o.tx,o.ty});for(int layer=0;layer<3;++layer)all_layers.push_back({layer,o.tx,o.ty});}
   }
   const auto goals=r::goals_for(request,{npc->layer,npc->tx,npc->ty},r::GoalKind::Interact,npc);
   const auto ordinary=r::flood(request,seeds),counterfactual=r::flood(request,all_layers);
   int reached=0,other_layer=0,physical=0;for(const auto& goal:goals.cells){reached+=ordinary.distance(goal)>=0;other_layer+=counterfactual.distance(goal)>=0;}
   auto any_layer=*npc;any_layer.d|=2;const auto any=r::goals_for(request,{npc->layer,npc->tx,npc->ty},r::GoalKind::Interact,&any_layer);
   for(const auto& goal:any.cells)physical+=ordinary.distance(goal)>=0;
   std::printf("FF%d %ls/%d L%d (%d,%d) flags%d: goals%zu reachable%d; counterfactual seed-layer%d/interaction-layer%d; entries",
       n,b::name_of(*sm).c_str(),npc->entity_id(),npc->layer,npc->tx,npc->ty,npc->d,goals.cells.size(),reached,other_layer,physical);
   for(const auto& seed:seeds)std::printf(" L%d(%d,%d)",seed.layer,seed.x,seed.y);std::printf("\n");
   // Separate source geometry from possible NPC congestion. These are copied
   // initializer witnesses, not an assertion that every flag bank is reachable.
   for(int bit:{0,1}){
    field::map_state::Flags flags;flags.scenario.fill(bit);flags.treasure.fill(bit);flags.global.fill(bit);
    const auto& alias=row.get(L"map_aliases")->arr.front();flags.current_map_id=std::stoi(alias.get(L"id")->s);flags.current_area_id=std::stoi(alias.get(L"area_id")->s);
    field::map_state::Resolver resolver(flags,records);auto projected=request;field::map_state::add_obstacles(resolver,*sm,projected,true,false,false);
    const auto visited=r::flood(projected,seeds);const auto effective=resolver.effective(*sm,*npc);
    const auto final=r::goals_for(projected,{effective.layer,effective.tx,effective.ty},r::GoalKind::Interact,&effective);
    int home=0,region=0;for(const auto& goal:final.cells)home+=visited.distance(goal)>=0;
    if(const auto* candidates=row.get(L"region_candidates"))for(const auto& cell:candidates->arr){auto moved=effective;moved.layer=uint8_t(cell.arr[0].n);moved.tx=int16_t(cell.arr[1].n);moved.ty=int16_t(cell.arr[2].n);moved.a=0;
     const auto around=r::goals_for(projected,{moved.layer,moved.tx,moved.ty},r::GoalKind::Interact,&moved);bool yes=false;for(const auto& goal:around.cells)yes|=visited.distance(goal)>=0;region+=yes;
    }
    std::printf("  Initializer witness%d, without NPC congestion: known%d active%d home%d region%d\n",bit,resolver.known(*sm),resolver.active(*sm,*npc),home,region);
   }
   if(const auto* candidates=row.get(L"region_candidates");candidates&&!candidates->arr.empty()){
    int possible=0;for(const auto& cell:candidates->arr){auto moving=*npc;moving.layer=uint8_t(cell.arr[0].n);moving.tx=int16_t(cell.arr[1].n);moving.ty=int16_t(cell.arr[2].n);moving.a=0;
     const auto around=r::goals_for(request,{moving.layer,moving.tx,moving.ty},r::GoalKind::Interact,&moving);
     bool reachable=false;for(const auto& goal:around.cells)reachable|=ordinary.distance(goal)>=0;possible+=reachable;
    }
    std::printf("  Authored region candidates: %d/%zu entry-connected approaches (not native movement/timing proof)\n",possible,candidates->arr.size());
   }
   if(const auto* entry=row.get(L"manual_entry")){
    const auto& cell=entry->get(L"cell")->arr;const auto after=r::flood(request,{{int(cell[0].n),int(cell[1].n),int(cell[2].n)}});
    int possible=0;for(const auto& goal:goals.cells)possible+=after.distance(goal)>=0;
    std::printf("  Actual manual-script entry L%d(%d,%d): %d reachable home approaches\n",int(cell[0].n),int(cell[1].n),int(cell[2].n),possible);
   }
   if(const auto* controls=row.get(L"scene_controls")){
    int found=0,reachable=0;
    for(const auto& source:controls->arr)for(const auto& event:grid.objects)if((event.type==b::ObjType::Event||event.type==b::ObjType::Warp)&&
       event.entity_id()==int(source.get(L"props")->get(L"entity_id")->n)){
      ++found;bool yes=false;for(int y=event.ty;y<event.ty+event.th;++y)for(int x=event.tx;x<event.tx+event.tw;++x)yes|=ordinary.distance({event.layer,x,y})>=0;reachable+=yes;
    }
    std::printf("  Native scene controls: %d captured, %d directly entry-connected\n",found,reachable);
   }
  }
 }
 std::printf("22-context source geometry inspected: %d; no story-state or route change inferred from counterfactuals\n",inspected);
 return inspected==22?0:1;
}
