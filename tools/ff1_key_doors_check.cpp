// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Every native Mystic Key/story door variant, source-selected layouts and all six
// affected rooms. Concrete paths must never clip a closed opening plane.
#define FFPR_KEY_DOOR_FIXTURE_ONLY
#include "key_door_route_check.cpp"
namespace every_door {
using namespace field;using namespace baked_nav;
int run(const json::Value& facts){
 int failures=0,routes=0,layouts=0,controls=0,chests=0,rooms=0;std::set<std::wstring> covered;
 const auto expect=[&](bool okay,const char* why,int map,int id){if(!okay){++failures;if(failures<25)std::printf("FAIL FF1 all-map %s map%d entity%d\n",why,map,id);}};
 for(const auto& [name,record]:fixture::records.obj){
  const auto* declarations=record.get(L"collision_layers");if(!declarations)continue;++rooms;
  const int map=int(record.get(L"map_ids")->arr.front().n);const auto* sm=baked::submap_by_map_id(map);if(!sm)return 2;
  std::vector<int> bits;for(const auto& row:facts.get(L"doors")->arr)if(row.get(L"submap")->s==name){for(const auto& bit:row.get(L"initializer_scenario_flags")->arr)if(int(bit.n)!=13)bits.push_back(int(bit.n));break;}
  std::sort(bits.begin(),bits.end());bits.erase(std::unique(bits.begin(),bits.end()),bits.end());if(bits.size()>8)return 2;
  std::set<std::wstring> seen;
  for(unsigned pattern=0;pattern<(1u<<bits.size());++pattern){
   map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=map;
   for(size_t i=0;i<bits.size();++i)flags.scenario[bits[i]]=(pattern>>i)&1;
   map_state::Resolver state(flags,fixture::records);const auto selected=map_state::project(record,flags);expect(selected.known,"native initializer known",map,0);
   std::wstring signature;for(const auto& group:selected.groups)signature+=group+L";";
   for(const auto& [id,e]:selected.entities)signature+=std::to_wstring(id)+L":"+std::to_wstring(e.visible.value_or(true))+L","+std::to_wstring(e.x.value_or(-1))+L","+std::to_wstring(e.y.value_or(-1))+L";";
   if(!seen.insert(signature).second)continue;++layouts;
   std::vector<const baked::Object*> doors;
   for(const auto& declaration:declarations->arr)for(const auto& o:baked::grids(*sm).objects)
    if(o.entity_id()==int(declaration.get(L"id")->n)&&baked::str(o.group)==declaration.get(L"group")->s&&state.active(*sm,o)){doors.push_back(&o);covered.insert(name+L":"+std::to_wstring(o.entity_id()));break;}
   for(const auto* door:doors)for(const auto& fact:facts.get(L"doors")->arr)if(fact.get(L"submap")->s==name&&int(fact.get(L"id")->n)==door->entity_id()&&fact.get(L"group")->s==baked::str(door->group)){
    for(const auto& opening:fact.get(L"open_controls")->arr)for(const auto& o:baked::grids(*sm).objects)
     if(o.type==baked::ObjType::OpenTrigger&&o.a==int(opening.get(L"link")->n)&&o.layer==int(opening.get(L"layer")->n)&&baked::str(o.group)==opening.get(L"group")->s)
      expect(!state.active(*sm,o),"closed lock already has an active native opening",map,door->entity_id());
   }
   const auto clear_path=[&](const PathInfo& path){if(!path.success)return true;for(const auto& phase:path.walk_phases)for(size_t i=1;i<phase.cells.size();++i){const auto at=phase.cells[i];for(const auto* door:doors)if((state.collision_layers(*sm,*door)&(1u<<at.layer))&&door->contains(at.x,at.y))return false;}return true;};
   const auto* arrival=point_in(*sm,1,state);if(!arrival)return 2;const Cell start{arrival->layer,arrival->tx,arrival->ty};
   for(const auto& target:baked::grids(*sm).objects)if(target.type==baked::ObjType::TreasureBox&&state.active(*sm,target)){
    auto s=keyed::capture(*sm,flags,start,target);const auto path=plan_snapshot(s).path;++routes;++chests;expect(clear_path(path),"chest route crosses a closed key door",map,target.entity_id());
   }
   for(const auto* target:doors){
    bool usable=false;for(const auto d:std::array<Cell,4>{{{0,0,1},{0,0,-1},{0,1,0},{0,-1,0}}}){
     const Cell adjacent{target->layer,target->tx+d.x,target->ty+d.y};baked_route::Request movement;movement.submap=sm;movement.start=adjacent;movement.passages=true;
     if(!baked_route::open(movement,baked::grids(*sm),adjacent))continue;
     auto s=keyed::capture(*sm,flags,adjacent,*target);const auto path=plan_snapshot(s).path;++routes;
     if(path.success&&path.step_count==0&&clear_path(path)){usable=true;break;}
    }
    expect(usable,"closed door itself has no legal adjacent interaction",map,target->entity_id());++controls;
   }
  }
  if(map==72){
   // Receipt1 opens the early shrine story seal, before the Garland/Sarah
   // episode. Prove that adding Bottom0 collision did not block that required
   // initial progression from the native entrance.
   map_state::Flags early;early.scenario.fill(0);early.treasure.fill(0);early.global.fill(0);early.area.fill(0);early.local.fill(0);early.current_map_id=map;early.scenario[1]=1;
   map_state::Resolver open(early,fixture::records);const auto* entrance=point_in(*sm,1,open);if(!entrance)return 2;
   bool boss=false;for(const auto& actor:baked::grids(*sm).objects)if(actor.entity_id()==32&&open.active(*sm,actor)){
    auto s=keyed::capture(*sm,early,{entrance->layer,entrance->tx,entrance->ty},actor);
    const auto path=plan_snapshot(s).path;++routes;expect(path.success,"early opened shrine must reach Garland",map,actor.entity_id());boss=true;
   }
   expect(boss,"early native Garland variant found",map,32);
  }
  // A real late key-owned state retires every native key-door variant. Other
  // protected story interactions retain their own normal route restrictions.
  map_state::Flags flags;flags.scenario.fill(1);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);flags.current_map_id=map;map_state::Resolver state(flags,fixture::records);
  for(const auto& declaration:declarations->arr)for(const auto& o:baked::grids(*sm).objects)if(o.entity_id()==int(declaration.get(L"id")->n)&&baked::str(o.group)==declaration.get(L"group")->s)expect(!state.active(*sm,o),"key-owned state retains a closed key door",map,o.entity_id());
  for(const auto& target:baked::grids(*sm).objects)if(target.type==baked::ObjType::TreasureBox&&state.active(*sm,target)){
   bool reached=false;for(const auto& arrival:baked::grids(*sm).objects)if(arrival.type==baked::ObjType::PointIn&&state.active(*sm,arrival)){
    auto s=keyed::capture(*sm,flags,{arrival.layer,arrival.tx,arrival.ty},target);const auto path=plan_snapshot(s).path;++routes;if(path.success){reached=true;break;}
   }
   expect(reached,"key-owned chest has no native entrance route",map,target.entity_id());++chests;
  }
 }
 expect(rooms==6&&covered.size()==24,"complete six-map/24-variant coverage",0,int(covered.size()));
 std::printf("FF1 all-map key doors: %d rooms/%zu variants/%d layouts/%d control approaches/%d chest cases/%d routes; failures%d owner%d\n",rooms,covered.size(),layouts,controls,chests,routes,failures,fixture::wrong_owner.load());return failures||fixture::wrong_owner?1:0;
}
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};const auto bytes=read("data/ff1/maps.bin");json::Value facts;if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff1/mapstate.json"),fixture::records)||!json::parse(read("docs/re/key_gate_layers_20261101.json"),facts))return 2;fixture::game_id=game::Id::FF1;field::baked_nav::g_loaded=field::baked_nav::g_prepared=true;int result=2;std::thread worker([&]{result=every_door::run(facts);});worker.join();return result;}
