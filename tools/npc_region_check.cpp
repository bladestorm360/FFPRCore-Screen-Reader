// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Three real South Figaro area/line actors, through the production collector
// and selected-route producer. Original asset/script bodies stay outside Git.
#define main retained_region_route_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;namespace r=baked_route;
 std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0,witnesses=0;
 const auto expect=[&](bool yes,const char* reason){if(!yes){++failures;std::printf("FAIL %s\n",reason);}};
 const auto read=[](const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 const auto bytes=read("data/ff6/maps.bin");json::Value inventory;
 if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||
  !json::parse(read("../tools/npc_geometry_context_20261029/contexts.json"),inventory))return 2;
 fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
 for(const auto& item:inventory.get(L"cases")->arr){
  if(int(item.get(L"game")->n)!=6||item.get(L"submap")->s!=L"Map_20051_3")continue;
  const int id=int(item.get(L"id")->n);const auto* target=baked::submap_by_name(item.get(L"submap")->s);
  if(!target)return 2;const baked::Object* source=nullptr;
  for(const auto& object:baked::grids(*target).objects)if(object.entity_id()==id&&object.type==baked::ObjType::NPC&&
   baked::str(object.name)==item.get(L"raw")->s&&baked::str(object.group)==item.get(L"group")->s)source=&object;
  if(!source)return 2;const Cell home{source->layer,source->tx,source->ty};
  // Native PropertyPolyline expands each16-unit vertical segment. The reviewed
  // area and both lines share four exact cell centers at y12..15 on Upper.
  // Actor and player movement are proved separately; an authored rectangle
  // alone is never accepted as proof an NPC can reach its frontier.
  const std::vector<Cell> positions{{2,home.x,12},{2,home.x,13},{2,home.x,14},{2,home.x,15}};
  bool witnessed=false;int roots=0,entries=0,listed_count=0,movable_count=0,graph_count=0,entry_count=0,local_count=0,visited=0;
  for(int index=0;index<baked::submap_count()&&!witnessed;++index){
   const auto* root=baked::submap_at(index);if(!root||root==target||root->world||root->first_map_id<=0)continue;
   bool adjacent=false;for(const auto& door:baked::grids(*root).objects)if((door.type==baked::ObjType::GotoMap||door.type==baked::ObjType::Warp)&&
    door.a>0&&baked::submap_by_map_id(door.a)==target)adjacent=true;
   if(!adjacent)continue;++roots;
   for(const auto& entry:baked::grids(*root).objects){
    if(witnessed)break;if(entry.type!=baked::ObjType::PointIn)continue;++entries;
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
    flags->current_map_id=root->first_map_id;flags->current_area_id=int(fixture::records.get(baked::name_of(*root))->get(L"area_id")->n);
    auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));snapshot->map_id=root->first_map_id;snapshot->submap=root;
    snapshot->position=world_of_cell(*root,{entry.layer,entry.tx,entry.ty});snapshot->layer=entry.layer;snapshot->locale=L"en";
    Planned output;bool listed=false,proof=false,actor_reached=false;Cell pose;
    std::thread worker([&]{
     const auto remote=connected_objects_snapshot(snapshot->map_id,snapshot->state);
     for(const auto& row:remote)if(row.entity_id==id&&row.raw_name==item.get(L"raw")->s&&baked::submap_by_map_id(row.map_id)==target){
      listed=true;snapshot->goal.map_id=row.map_id;snapshot->goal.cell=row.cell;snapshot->goal.entity_id=id;
      snapshot->goal.raw_name=row.raw_name;snapshot->kind=TargetKind::Interact;output=plan_snapshot(*snapshot);break;
     }
     if(!listed)return;++listed_count;
     expect(snapshot->state.route_positions(*target,*source).size()==4,"exact active native area/line publishes four verified positions");
     auto stationary=*source;stationary.d&=~16;
     expect(snapshot->state.route_positions(*target,stationary).empty(),"stationary actor cannot borrow a default movement region");
     auto unknown=*flags;unknown.scenario[15].reset();map_state::Resolver uncertain(unknown,fixture::records);
     expect(uncertain.route_positions(*target,*source).empty(),"unknown initializer cannot certify a default movement region");
     if(id==15){auto hidden=*flags;hidden.scenario[15]=1;hidden.scenario[203]=1;map_state::Resolver absent(hidden,fixture::records);
      expect(absent.route_positions(*target,*source).empty(),"native hidden actor never inherits a region destination");}
     for(const auto& door:baked::grids(*root).objects)if(door.type==baked::ObjType::GotoMap&&door.a>0&&baked::submap_by_map_id(door.a)==target){
      auto control=std::make_unique<RouteSnapshot>(*snapshot);control->active={};control->pending={};control->goal={};
      control->goal.map_id=root->first_map_id;control->goal.cell={door.layer,door.tx,door.ty};control->goal.entity_id=door.entity_id();
      control->kind=TargetKind::Transition;entry_count+=plan_snapshot(*control).path.success;
     }
     auto movement=walk_request(*target,home);map_state::add_obstacles(snapshot->state,*target,movement,true,false,false);
     auto player_movement=movement;player_movement.start={2,11,7};const auto local_reach=r::flood(player_movement);
     for(const auto& at:positions){
      auto actor=movement;actor.goals={at};actor.goals_open=false;
      if(!r::find(actor).found)continue;actor_reached=true;++movable_count;
      auto moved=*source;moved.layer=static_cast<uint8_t>(at.layer);moved.tx=static_cast<int16_t>(at.x);moved.ty=static_cast<int16_t>(at.y);
      auto exact=snapshot->goal;exact.cells=r::goals_for(movement,at,r::GoalKind::Interact,&moved).cells;exact.examine=true;
      for(const auto& cell:exact.cells)local_count+=local_reach.distance(cell)>=0;
      SnapshotScope scope(*snapshot);const auto leg=find_across(nullptr,nullptr,exact,true,true);visited=std::max(visited,leg.visited_maps);
      if(leg.found){++graph_count;proof=true;pose=at;break;}
     }
    });worker.join();
    if(!listed||!proof)continue;witnessed=true;++witnesses;
    std::printf("FF6 South Figaro/%d: listed%d actor-can-move%d native-frontier%d selected-route%d next%d pose%d,%d,L%d\n",
     id,listed,actor_reached,proof,output.path.success,output.path.next_map_id,pose.x,pose.y,pose.layer);
    expect(output.path.success,"native area/line frontier must retain a cross-floor first leg");
    if(output.path.success){
     expect(output.pending.valid&&output.pending.goal.cell==home&&output.pending.goal.entity_id==id,"region first leg retains real actor identity/home");
     std::thread continuation([&]{
      auto local_flags=*flags;local_flags.current_map_id=snapshot->goal.map_id;
      local_flags.current_area_id=int(fixture::records.get(baked::name_of(*target))->get(L"area_id")->n);
      auto live=std::make_unique<RouteSnapshot>(map_state::Resolver(local_flags,fixture::records));live->submap=target;live->map_id=snapshot->goal.map_id;
      live->locale=L"en";live->kind=TargetKind::Interact;live->goal=output.pending.goal;live->pending=output.pending;
      if(!output.pending.incoming_transition){expect(false,"real entry receipt exists");return;}
      const auto arrival=output.pending.incoming_transition->arrival;live->position=world_of_cell(*target,arrival);live->layer=arrival.layer;
      NativeObstacle npc;npc.npc=true;npc.id=id;npc.raw_name=live->goal.raw_name;npc.cell=pose;live->obstacles.push_back(npc);
      const auto result=plan_snapshot(*live);expect(result.path.success&&result.goal.cell==pose,"actual loaded native region actor owns final interaction");
      live->obstacles.clear();expect(!plan_snapshot(*live).path.success,"missing current region actor cannot be synthesized from region metadata");
     });continuation.join();
    }
   }
  }
  std::printf("Region%d controls: %d neighboring roots/%d native entries/%d collected/%d actor-route/%d graph/%d reachable doors/%d target-local approaches\n",id,roots,entries,listed_count,movable_count,graph_count,entry_count,local_count);
  std::printf("Region%d graph max visited%d, production passages%d\n",id,visited,game::flags().has_hidden_passages);
  expect(witnessed,"each region actor has a real movement and first-leg geometry witness");
 }
 expect(witnesses==3,"all three native area/line actors are checked");expect(fixture::wrong_owner==0,"no worker native query");
 // Native Upper stair: direct A* and graph reachability must agree on its
 // directional diagonal. A blocker or a disallowed approach still closes it.
 const auto* stairs=baked::submap_by_name(L"Map_20051_1");if(!stairs)return 2;
 const auto& stair_grid=baked::grids(*stairs);const baked::Object* door=nullptr;
 for(const auto& object:stair_grid.objects)if(object.type==baked::ObjType::GotoMap&&object.a==24)door=&object;
 if(!door)return 2;
 r::Request stair;stair.submap=stairs;stair.start={2,10,13};stair.passages=true;stair.live_hazards=stair.live_traps=true;
 const auto reach=r::flood(stair);auto direct=stair;direct.goals={{2,11,14}};
 expect(r::find(direct).found&&distance_to_object(stair,reach,*door,2,false)==1,"native diagonal stair has the same one-step local and graph distance");
 auto blocked=stair;blocked.blocked.push_back({2,11,14});
 expect(distance_to_object(blocked,reach,*door,2,false)<0,"solid blocker still seals the diagonal stair");
 auto wrong_side=stair;wrong_side.directional.push_back({{2,11,14},0,0});
 expect(distance_to_object(wrong_side,reach,*door,2,false)<0,"native direction restriction still seals the diagonal stair");
 std::printf("Region NPC source-to-output: %d witnesses, %d failures\n",witnesses,failures);return failures?1:0;
}
