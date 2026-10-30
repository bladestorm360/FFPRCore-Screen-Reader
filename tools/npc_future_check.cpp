// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual collector and first-leg planner for the11 source-assigned NPC routes.
// Complete original scripts/route assets stay outside all Git trees.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <set>
int main(int argc,char** argv){
 using namespace field;using namespace baked_nav;
 std::setvbuf(stdout,nullptr,_IONBF,0);
 const auto read=[](const std::string&p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
 json::Value inventory;if(!json::parse(read("../tools/npc_geometry_context_20261029/contexts.json"),inventory))return 2;
 int cases=0,listed=0,failures=0,continued=0;double total_ms=0,peak_ms=0;
 const auto expect=[&](bool yes,const char* why){if(!yes){++failures;std::printf("FAIL %s\n",why);}};
 for(int n:{5,6}){
  const bool before=argc>1&&std::strcmp(argv[1],"--before")==0;
  const auto bytes=read("data/ff"+std::to_string(n)+"/maps.bin");if(!baked::load(bytes.data(),bytes.size())||
   !json::parse(read(before?"build_check/npc_future_20261029/before_ff"+std::to_string(n)+"_state.json":"data/ff"+std::to_string(n)+"/mapstate.json"),fixture::records))return 2;
  fixture::game_id=static_cast<game::Id>(n);
  g_loaded=g_prepared=true; // immutable bake preparation is complete in this host
  json::Value assets;if(!json::parse(read("../tools/npc_geometry_context_20261029/ff"+std::to_string(n)+"_route_assets.json"),assets))return 2;
  for(const auto& item:inventory.get(L"cases")->arr){
   if(int(item.get(L"game")->n)!=n)continue;
   const int id=int(item.get(L"id")->n);bool assigned=false;std::set<int> route_ids;
   for(const auto& script:item.get(L"local_motion")->arr)for(const auto& op:script.get(L"instructions")->arr)
    if(op.get(L"mnemonic")->s==L"AssignMoveRoute"){
     const auto& values=op.get(L"operands")->get(L"iValues")->arr;if(values.size()>1&&int(values[0].n)==id){assigned=true;route_ids.insert(int(values[1].n));}
    }
   if(!assigned)continue;++cases;const auto* target=baked::submap_by_name(item.get(L"submap")->s);if(!target)return 2;
   const baked::Object* authored=nullptr;for(const auto& o:baked::grids(*target).objects)if((o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC)&&
    o.entity_id()==id&&baked::str(o.group)==item.get(L"group")->s&&baked::str(o.name)==item.get(L"raw")->s)authored=&o;
   if(!authored)return 2;
   std::vector<baked_route::Cell> poses;const auto& bounds=item.get(L"bounds")->arr;
   for(int route_id:route_ids){
    const std::wstring tail=L"/"+baked::name_of(*target)+L"/route_instruction_"+std::to_wstring(route_id)+L".json";
    for(const auto& asset:assets.get(L"assets")->obj)if(asset.first.size()>=tail.size()&&asset.first.compare(asset.first.size()-tail.size(),tail.size(),tail)==0)
     for(const auto& instruction:asset.second.get(L"body")->get(L"instruction_list")->arr){
      const int type=int(instruction.get(L"instruction_type")->n);if(type!=1&&type!=7)continue;
      poses.push_back({int(instruction.get(L"layer")->n),int(instruction.get(L"cellx")->n)-int(bounds[0].n),int(instruction.get(L"celly")->n)-int(bounds[1].n)});
     }
   }
   bool witnessed=false;int roots=0;
   for(int index=0;index<baked::submap_count()&&!witnessed&&roots<4;++index){
    const auto* root=baked::submap_at(index);if(!root||root==target||root->world||root->first_map_id<=0)continue;
    const auto& grid=baked::grids(*root);
    bool adjacent=false;for(const auto& o:grid.objects)if((o.type==baked::ObjType::GotoMap||o.type==baked::ObjType::Warp)&&
     o.a>0&&baked::submap_by_map_id(o.a)==target)adjacent=true;
    if(!adjacent)continue;++roots;
    for(const auto& point:grid.objects){
    if(witnessed)break;
    if(point.type!=baked::ObjType::PointIn)continue;
    const auto* entry=&point;
    for(int bit:{0,1}){
     // Exact initializer ownership: the turtle patrol starts only after bank2
     // bit76; the dragon needs bit107 set while bit108 remains clear. All-zero
     // or all-one banks alone are not evidence that these routes are assigned.
     if(n==6&&baked::name_of(*target)==L"Map_30641_1"&&!bit)continue;
     auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(bit);flags->treasure.fill(bit);flags->global.fill(bit);
     if(n==6&&baked::name_of(*target)==L"Map_30671_1"){
      flags->scenario[256+107]=1;flags->scenario[256+108]=0;
     }
     flags->current_map_id=root->first_map_id;
     if(const auto* record=fixture::records.get(baked::name_of(*root)))if(const auto* area=record->get(L"area_id"))flags->current_area_id=int(area->n);
     auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));snapshot->submap=root;snapshot->map_id=root->first_map_id;
     snapshot->position=world_of_cell(*root,{entry->layer,entry->tx,entry->ty});snapshot->layer=entry->layer;snapshot->locale=L"en";
     std::vector<RemoteObject> remote;Planned result;bool have=false,scripted=false,entry_reachable=false,pose_reachable=false;
     std::thread worker([&]{
      const auto started=std::chrono::steady_clock::now();
      remote=connected_objects_snapshot(snapshot->map_id,snapshot->state);
      for(const auto& o:remote)if(o.entity_id==id&&baked::submap_by_map_id(o.map_id)==target&&o.group==item.get(L"group")->s){
       have=true;scripted=o.scripted;snapshot->goal.map_id=o.map_id;snapshot->goal.cell=o.cell;snapshot->goal.entity_id=o.entity_id;
       snapshot->goal.raw_name=o.raw_name;snapshot->kind=TargetKind::Interact;result=plan_snapshot(*snapshot);break;
      }
      if(!have)return;
      const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
      if(result.path.success){total_ms+=elapsed;peak_ms=std::max(peak_ms,elapsed);}
      for(const auto& door:grid.objects)if((door.type==baked::ObjType::GotoMap||door.type==baked::ObjType::Warp)&&door.a>0&&baked::submap_by_map_id(door.a)==target){
       auto control=std::make_unique<RouteSnapshot>(*snapshot);control->pending={};control->active={};control->previous_active.reset();control->goal={};
       control->goal.map_id=root->first_map_id;control->goal.cell={door.layer,door.tx,door.ty};control->goal.entity_id=door.entity_id();
       control->goal.raw_name=baked::object_identifier(door);control->kind=TargetKind::Transition;
       entry_reachable|=plan_snapshot(*control).path.success;
      }
      if(entry_reachable&&!result.path.success)for(const auto& pose:poses){
       auto control=std::make_unique<RouteSnapshot>(*snapshot);control->pending={};control->active={};control->previous_active.reset();control->goal.cell=pose;
       auto actual=*authored;actual.layer=static_cast<uint8_t>(pose.layer);actual.tx=static_cast<int16_t>(pose.x);actual.ty=static_cast<int16_t>(pose.y);
       auto movement=walk_request(*target,pose);const auto goals=baked_route::goals_for(movement,pose,baked_route::GoalKind::Interact,&actual);
       control->goal.cells=goals.cells;control->goal.examine=true;
       // Geometry control deliberately bypasses only the remote authored-cell
       // availability check, not collision, native talk sides or the real graph.
       // It is evidence of a possible native pose, not production authorization
       // to move the NPC or claim a complete story trajectory.
       SnapshotScope scope(*control);if(find_across(nullptr,nullptr,control->goal).found){pose_reachable=true;break;}
      }
     });worker.join();
     if(!have)continue;
     if(entry_reachable)std::printf("FF%d %ls/%d from%ls flag%d: listed1 scripted%d route%d entry%d native-pose%d next%d goal(%d,%d,L%d) reason=%ls\n",n,baked::name_of(*target).c_str(),id,
      baked::name_of(*root).c_str(),bit,scripted,result.path.success,entry_reachable,pose_reachable,result.path.next_map_id,snapshot->goal.cell.x,snapshot->goal.cell.y,snapshot->goal.cell.layer,result.path.reason.c_str());
     if(!entry_reachable||(!result.path.success&&!pose_reachable))continue;
     ++listed;witnessed=true;
     if(!result.path.success){++failures;std::puts("FAIL a real reachable native patrol pose must retain a cross-floor first leg despite its authored home");}
     if(result.path.success) {
      expect(result.pending.valid&&result.pending.goal.entity_id==id&&result.pending.goal.raw_name==snapshot->goal.raw_name&&
       result.pending.goal.cell==snapshot->goal.cell,"first leg retains authored identity/home instead of committing a future pose");
      bool live_arrived=false;
      std::thread continuation([&]{
       // A newly usable native stair may choose a shorter *multi-floor* path.
       // Replay each actual first-hop receipt before using a target-floor pose;
       // never treat an intermediate floor's arrival as the destination entry.
       auto terminal=result;auto travel_flags=*flags;int hops=0;
       while(terminal.path.next_map_id!=snapshot->goal.map_id&&terminal.path.next_map_id>0&&hops++<16){
        const auto* next=baked::submap_by_map_id(terminal.path.next_map_id);
        if(!next||!terminal.pending.incoming_transition){expect(false,"intermediate native floor has a real entry receipt");return;}
        travel_flags.current_map_id=terminal.path.next_map_id;
        if(const auto* record=fixture::records.get(baked::name_of(*next)))if(const auto* area=record->get(L"area_id"))travel_flags.current_area_id=int(area->n);
        auto leg=std::make_unique<RouteSnapshot>(map_state::Resolver(travel_flags,fixture::records));
        leg->submap=next;leg->map_id=terminal.path.next_map_id;leg->locale=L"en";leg->kind=TargetKind::Interact;
        leg->goal=terminal.pending.goal;leg->pending=terminal.pending;
        const auto arrival=terminal.pending.incoming_transition->arrival;
        leg->position=world_of_cell(*next,arrival);leg->layer=arrival.layer;
        terminal=plan_snapshot(*leg);
        if(!terminal.path.success){expect(false,"each intermediate native floor retains the destination and a usable next leg");return;}
       }
       expect(hops<16&&terminal.path.next_map_id==snapshot->goal.map_id,"finite actual floor sequence reaches the NPC floor");
       auto local_flags=*flags;local_flags.current_map_id=snapshot->goal.map_id;
       if(const auto* record=fixture::records.get(baked::name_of(*target)))if(const auto* area=record->get(L"area_id"))local_flags.current_area_id=int(area->n);
       auto live=std::make_unique<RouteSnapshot>(map_state::Resolver(local_flags,fixture::records));
       live->submap=target;live->map_id=snapshot->goal.map_id;live->locale=L"en";live->kind=TargetKind::Interact;
       live->goal=terminal.pending.goal;live->pending=terminal.pending;
       if(!terminal.pending.incoming_transition)return;
       const auto arrival=terminal.pending.incoming_transition->arrival;
       live->position=world_of_cell(*target,arrival);live->layer=arrival.layer;
       const auto native_poses=live->state.route_positions(*target,*authored);
       expect(!native_poses.empty(),"real active assignment produces reviewed native route positions");
       auto wrong=*authored;++wrong.tx;
       expect(live->state.route_positions(*target,wrong).empty(),"wrong authored home cannot borrow another actor's route");
       wrong=*authored;++wrong.c; // Object::entity_id is c; a is the NPC movement payload
       expect(live->state.route_positions(*target,wrong).empty(),"wrong actor ID cannot borrow a route");
       for(const auto& pose:native_poses) {
        auto attempt=std::make_unique<RouteSnapshot>(*live);NativeObstacle actor;
        actor.npc=true;actor.id=id;actor.raw_name=live->goal.raw_name;actor.cell=pose;
        attempt->obstacles.push_back(actor);const auto output=plan_snapshot(*attempt);
        if(output.path.success){live_arrived=true;expect(output.goal.cell==pose,"loaded floor rebinds target to actual copied native position/layer");break;}
       }
       auto absent=std::make_unique<RouteSnapshot>(*live);absent->obstacles.clear();
       expect(!plan_snapshot(*absent).path.success,"future patrol facts never replace a missing current-floor native actor");
       auto cancelled=std::make_unique<RouteSnapshot>(*snapshot);cancelled->cancelled=[]{return true;};
       expect(!plan_snapshot(*cancelled).path.success,"cancelled remote patrol route cannot publish success");
       if(n==6&&(baked::name_of(*target)==L"Map_30641_1"||baked::name_of(*target)==L"Map_30671_1")) {
        auto off=*flags;off.scenario[256+76]=0;off.scenario[256+107]=0;
        map_state::Resolver inactive(off,fixture::records);
        expect(inactive.route_positions(*target,*authored).empty(),"inactive turtle/dragon branch grants no future position");
        auto changed=std::make_unique<RouteSnapshot>(inactive);changed->map_id=snapshot->map_id;changed->submap=root;
        changed->position=snapshot->position;changed->layer=snapshot->layer;changed->kind=TargetKind::Interact;
        changed->goal=result.pending.goal;changed->pending=result.pending;changed->previous_active=result.active;
        expect(!plan_snapshot(*changed).path.success,"retained destination cannot keep a predicted pose after its native assignment becomes inactive");
        off.scenario[256+76].reset();off.scenario[256+107].reset();
        map_state::Resolver unknown(off,fixture::records);
        expect(unknown.route_positions(*target,*authored).empty(),"unknown native assignment grants no future position");
       }
       auto malformed=fixture::records;auto& cells=malformed.obj[baked::name_of(*target)].obj[L"patrols"].arr.front().obj[L"cells"];
       cells.arr.resize(33,cells.arr.front());map_state::Resolver oversized(*flags,malformed);
       expect(oversized.route_positions(*target,*authored).empty()||id!=int(malformed.obj[baked::name_of(*target)].obj[L"patrols"].arr.front().get(L"id")->n),
        "oversized selected patrol metadata grants no future position");
      });continuation.join();
      expect(live_arrived,"actual first-leg arrival continues to at least one real native patrol pose");
      continued+=live_arrived;
     }
     break;
    }
    }
   }
   if(!witnessed)std::printf("FF%d %ls/%d: no active collected neighbor witness; retain source-state uncertainty\n",n,baked::name_of(*target).c_str(),id);
  }
 }
 if(fixture::wrong_owner){++failures;std::puts("FAIL native work occurred on the route worker");}
 expect(cases==11&&listed==11&&continued==11,"all eleven source actors have active collected first-leg and native continuation witnesses");
 std::printf("Collector plus first-leg worker compute: total%.3fms peak%.3fms for eleven successful requests; no native/frame/audio timing claim\n",total_ms,peak_ms);
 std::printf("Future-floor native motion: %d source actors, %d collected witnesses, %d failures; sampled flags are not complete story trajectories\n",cases,listed,failures);
 return failures?1:0;
}
