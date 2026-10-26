// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// All-six real-bake survey: separate geometry from a wandering NPC's unknown
// future position. Unreachable/gated candidates are reported, not guessed open.
#include "field/map_state.h"
#include <fstream>
#include <iterator>
#include <cstdio>
#include <thread>
#include <memory>
#include <chrono>
std::string read_npc_file(const std::string& name){std::ifstream f(name,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 using namespace field;int failures=0;
 for(int game=1;game<=6;++game){
  const auto root="data/ff"+std::to_string(game)+"/";const auto bytes=read_npc_file(root+"maps.bin");json::Value records;
  if(!baked::load(bytes.data(),bytes.size())||!json::parse(read_npc_file(root+"mapstate.json"),records))return 1;
  for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));
  for(int profile=0;profile<(game==3?3:2);++profile){
   auto flags=std::make_unique<map_state::Flags>();for(auto& v:flags->scenario)v=profile==1;for(auto&v:flags->treasure)v=0;for(auto&v:flags->global)v=0;if(profile==2)flags->scenario[16]=1;
   int eligible=0,reachable=0,no_goals=0,no_entry=0,staged=0,uncertain=0,pickups=0;const auto began=std::chrono::steady_clock::now();
   std::thread worker([&]{
    map_state::Resolver state(*flags,records);
    for(int i=0;i<baked::submap_count();++i){
     const auto& sm=*baked::submap_at(i);if(sm.world)continue;const auto& grid=baked::grids(sm);
     if(!state.known(sm))++uncertain;
     baked_route::Request req;req.submap=&sm;req.passages=true;req.ignore_hazards=true;
     map_state::add_relays(state,sm,req);map_state::add_obstacles(state,sm,req,true,true);
     std::vector<baked_route::Cell> entries;
     for(const auto& o:grid.objects)if(o.type==baked::ObjType::PointIn&&state.active(sm,o))entries.push_back({o.layer,o.tx,o.ty});
     const auto flood=baked_route::flood(req,entries);
     for(const auto& original:grid.objects){
      if(baked::pickup_treasure(original))++pickups;
      if((original.type!=baked::ObjType::NPC&&original.type!=baked::ObjType::ShopNPC)||!(original.d&(4|8))||!state.active(sm,original)||state.consumed(sm,original))continue;
      const auto o=state.effective(sm,original);if(!baked::in_bounds(grid,o.tx,o.ty)){++staged;continue;}++eligible;
      const auto goals=baked_route::goals_for(req,{o.layer,o.tx,o.ty},baked_route::GoalKind::Interact,&o);
      if(goals.cells.empty())++no_goals;
      bool found=false;for(const auto& c:goals.cells)found|=flood.distance(c)>=0;
      if(found)++reachable;else if(entries.empty())++no_entry;
      for(const auto& c:goals.cells)if(!(o.d&2)&&c.layer!=o.layer){++failures;std::printf("FAIL FF%d map%d NPC%d wrong interaction layer\n",game,sm.first_map_id,o.entity_id());}
     }
    }
   });worker.join();
   std::printf("FF%d profile%d: %d active in-bounds NPCs, %d reachable ignoring transient NPC placement, %d no legal approaches, %d without entry metadata, %d off-map staged; %d unresolved initializers; %d pickup records; %.3f ms offline\n",game,profile,eligible,reachable,no_goals,no_entry,staged,uncertain,pickups,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
   // This is an audit, not a promise every NPC is reachable in every story
   // state. Geometry/gates and unknown initializers remain visible in counts.
  }
 }
 std::printf("All-six NPC geometry/layer audit: %d failures\n",failures);return failures?1:0;
}
