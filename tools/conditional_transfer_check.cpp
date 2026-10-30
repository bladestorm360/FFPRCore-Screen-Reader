// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual three FFV native Main branch destinations and owned graph consumers.
#define main retained_conditional_route_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 struct Case{const wchar_t* map;int actor,flag,first,point1,second,point2;};
 const Case cases[]={{L"Map_30031_1",1,25,198,2,211,1},{L"Map_30151_1",10,64,250,2,296,2},{L"Map_30161_1",4,125,1,26,19,31}};
 int failures=0,plans=0;auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 for(const auto& item:cases){
  const auto* source=baked::submap_by_name(item.map);if(!source)return 2;
  const baked::Object* native=nullptr;for(const auto& o:baked::grids(*source).objects)if(o.type==baked::ObjType::Event&&o.entity_id()==item.actor)native=&o;
  if(!native)return 2;
  for(int mode:{0,1,2}){
   map_state::Flags flags;flags.current_map_id=source->first_map_id;
   if(mode!=2){flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[item.flag]=mode;}
   std::thread worker([&]{
    map_state::Resolver state(flags,fixture::records);int allowed=0;const baked::Object* branch=nullptr;
    const int destination=mode==1?item.second:item.first,point=mode==1?item.point2:item.point1;
    for(const auto& o:baked::grids(*source).objects)if(o.type==baked::ObjType::Warp&&o.entity_id()==item.actor&&!state.consumed(*source,o)){
     ++allowed;if(o.a==destination&&o.b==point)branch=&o;
     if(mode!=2)expect(o.a==destination&&o.b==point,"only the actual story-selected transfer is usable");
    }
    expect(allowed==(mode==2?2:1),"known flag selects one real arc; unavailable flag preserves both possibilities");
    expect(!state.consumed(*source,*native),"destination branch guards never consume the native physical door");
    if(mode==2)return;
    expect(branch!=nullptr,"story-selected native transfer exists in the actual bake");if(!branch)return;
    const auto* target=baked::submap_by_map_id(destination);const auto* arrival=point_in(*target,point,state);
    expect(arrival!=nullptr,"selected branch has its native destination PointIn");if(!arrival)return;
    RouteSnapshot snapshot(state);snapshot.map_id=source->first_map_id;snapshot.submap=source;snapshot.locale=L"en";
    const auto* entry=point_in(*source,1,state);expect(entry!=nullptr,"actual source-floor native entrance exists");if(!entry)return;
    snapshot.layer=entry->layer;snapshot.position=world_of_cell(*source,{entry->layer,entry->tx,entry->ty});
    NativeObstacle powered;powered.type=2;powered.id=native->entity_id();powered.raw_name=baked::object_identifier(*native);
    powered.cell={native->layer,native->tx,native->ty};powered.trigger=true;snapshot.obstacles={powered};
    snapshot.kind=TargetKind::Spot;snapshot.goal.map_id=destination;snapshot.goal.cell={target->world?1:arrival->layer,arrival->tx,arrival->ty};
    const auto output=plan_snapshot(snapshot);
    {SnapshotScope scope(snapshot);
     const auto* selected=baked_by_entity_id(baked::grids(*source),item.actor,baked::ObjType::Warp,powered.cell,powered.raw_name);
     expect(selected&&selected->a==destination&&selected->b==point,"loaded native trigger matches the active companion, including the second branch");
     const auto mask=current_transitions(*source);const auto& cells=(*mask)[native->layer];const auto at=size_t(native->ty)*source->width+native->tx;
     expect(at<cells.size()&&(cells[at]&2),"fresh powered trigger retains its terminal boundary for either native branch");
    }
    expect(output.path.success&&output.path.next_map_id==destination,"actual owned planner follows the story-selected door branch");
    expect(output.pending.incoming_transition&&output.pending.incoming_transition->destination==destination&&output.pending.incoming_transition->entity_id==item.actor,
     "continuation receipt retains the chosen native actor/destination");
    plans+=output.path.success&&output.path.next_map_id==destination;
   });worker.join();
  }
 }
 expect(plans==6,"both actual native story branches are planned for all three doors");
 expect(fixture::wrong_owner==0,"branch decisions never query native state from a worker");
 std::printf("Conditional door transfers: %d/6 actual plans, %d failures, owner%d\n",plans,failures,fixture::wrong_owner.load());return failures?1:0;
}
