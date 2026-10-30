// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Exact native default-list arrival choices. Original map objects/initializers
// and native bodies remain outside Git; no game/runtime probe is performed.
#define main retained_arrival_route_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 json::Value inventory;if(!json::parse(read("../tools/conditional_arrivals_20261029/contexts.json"),inventory))return 2;
 int failures=0,cases=0,choices=0,continuations=0;
 const auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 for(int n:{3,5}){
  const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");
  if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff"+std::to_string(n)+"/mapstate.json"),fixture::records))return 2;
  fixture::game_id=static_cast<game::Id>(n);g_loaded=g_prepared=true;
  for(const auto& item:inventory.arr){
   if(int(item.get(L"game")->n)!=n)continue;++cases;
   const auto* source=item.get(L"source");const int dest_id=int(source->get(L"destination")->n),point=int(source->get(L"point")->n);
   const auto* root=baked::submap_by_name(source->get(L"submap")->s);const auto* destination=baked::submap_by_map_id(dest_id);
   if(!root||!destination)return 2;
   const baked::Object* door=nullptr;for(const auto& object:baked::grids(*root).objects)
    if(object.type==baked::ObjType::GotoMap&&object.entity_id()==int(source->get(L"id")->n)&&object.a==dest_id&&object.b==point)door=&object;
   expect(door!=nullptr,"actual native door retains its exact destination and PointIn number");
   const auto& expected=item.get(L"expected_cell")->arr;const Cell target{int(expected[0].n),int(expected[1].n),int(expected[2].n)};
   const int native_id=int(item.get(L"default_point")->get(L"props")->get(L"entity_id")->n);
   for(int mode:{0,1,2}){
    auto flags=std::make_unique<map_state::Flags>();if(mode!=2){flags->scenario.fill(mode);flags->treasure.fill(mode);flags->global.fill(mode);}
    flags->current_map_id=root->first_map_id;
    if(const auto* record=fixture::records.get(baked::name_of(*root)))if(const auto* area=record->get(L"area_id"))flags->current_area_id=int(area->n);
    std::thread worker([&]{
     map_state::Resolver state(*flags,fixture::records);const auto* selected=point_in(*destination,point,state);
     expect(selected!=nullptr,"real graph selector returns the native arrival");if(!selected)return;
     const Cell actual{destination->world?1:selected->layer,selected->tx,selected->ty};
     expect(actual==target&&selected->entity_id()==native_id,"native default-list first match agrees with the actual baked graph receiver");
     ++choices;
     auto arrived_flags=*flags;arrived_flags.current_map_id=dest_id;
     if(const auto* record=fixture::records.get(baked::name_of(*destination)))if(const auto* area=record->get(L"area_id"))arrived_flags.current_area_id=int(area->n);
     auto arrived=std::make_unique<RouteSnapshot>(map_state::Resolver(arrived_flags,fixture::records));
     arrived->submap=destination;arrived->map_id=dest_id;arrived->position=world_of_cell(*destination,actual);arrived->layer=actual.layer;
     arrived->locale=L"en";arrived->kind=TargetKind::Spot;arrived->goal.map_id=dest_id;arrived->goal.cell=actual;
     {SnapshotScope scope(*arrived);Cell observed;expect(player_cell(nullptr,nullptr,observed)&&observed==actual,
      "fresh owned arrival capture uses the exact native cell, including world layer normalization");}
     const auto output=plan_snapshot(*arrived);
     expect(output.path.success&&output.path.step_count==0,"fresh arrival continuation does not invent movement away from its native position");
     continuations+=output.path.success&&output.path.step_count==0;
    });worker.join();
   }
   std::printf("FF%d %ls/%d ->%d point%d native entity%d at%d,%d,L%d: all known/unknown state choices checked\n",n,
    source->get(L"submap")->s.c_str(),int(source->get(L"id")->n),dest_id,point,native_id,target.x,target.y,target.layer);
  }
 }
 expect(cases==8&&choices==24&&continuations==24,"all eight flagged sets have all three real selector/arrival-state checks");
 expect(fixture::wrong_owner==0,"native arrival evidence never causes a worker-native query");
 std::printf("Native arrival choice: %d cases/%d choices/%d owned continuations, %d failures; no game movement or complete story-trajectory claim\n",
  cases,choices,continuations,failures);return failures?1:0;
}
