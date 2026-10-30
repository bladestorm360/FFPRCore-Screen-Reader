// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real Istory NPCs, source sheep interaction and production owned planner.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
#include <tuple>
int main(){
 using namespace field;using namespace baked_nav;
 std::setvbuf(stdout,nullptr,_IONBF,0);int failures=0;
 auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 auto read=[](const char* path){std::ifstream file(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(file),{});};
 const auto bytes=read("data/ff5/maps.bin");if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/mapstate.json"),fixture::records))return 2;
 fixture::game_id=game::Id::FF5;g_loaded=g_prepared=true;
 const auto* town=baked::submap_by_map_id(118);if(!town)return 2;
 for(int target_id:{22,103}){
  map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
  flags.current_map_id=118;flags.current_area_id=29;
  RouteSnapshot snapshot{map_state::Resolver(flags,fixture::records)};snapshot.submap=town;snapshot.map_id=118;snapshot.layer=1;snapshot.locale=L"en";
  const baked::Object* entry=nullptr;const baked::Object* target=nullptr;
  std::set<std::tuple<int,int,int,int,std::wstring>> seen;
  for(const auto& original:baked::grids(*town).objects){
   if(original.type==baked::ObjType::PointIn&&original.a==1)entry=&original;
   if(!snapshot.state.active(*town,original))continue;const auto object=snapshot.state.effective(*town,original);
   const bool npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;
   if(npc&&object.entity_id()==target_id)target=&original;
   const bool chest=object.type==baked::ObjType::TreasureBox;
   const bool event=object.type==baked::ObjType::Event||(object.type==baked::ObjType::Warp&&(object.d&255)==1);
   const bool wall=(object.type==baked::ObjType::Entity||object.type==baked::ObjType::AnimEntity||object.type==baked::ObjType::EffectEntity)&&(object.d&1);
   if(!npc&&!chest&&!event&&!wall&&object.type!=baked::ObjType::CollisionEntity)continue;
   if(!seen.emplace(object.entity_id(),object.layer,object.tx,object.ty,baked::object_identifier(object)).second)continue;
   NativeObstacle actor;actor.cell={object.layer,object.tx,object.ty};actor.type=event?2:int(object.type);actor.id=object.entity_id();actor.raw_name=baked::object_identifier(object);
   actor.npc=npc;actor.chest=chest;actor.trigger=event;actor.movement_known=npc;actor.wanders=(object.d&16)!=0;
   actor.collider=object.type==baked::ObjType::CollisionEntity;if(actor.collider)actor.collision=object.a;
   snapshot.obstacles.push_back(actor);
  }
  if(!target||!entry)return 2;
  snapshot.position=world_of_cell(*town,{entry->layer,entry->tx,entry->ty});snapshot.kind=TargetKind::Interact;
  snapshot.goal.map_id=118;snapshot.goal.cell={target->layer,target->tx,target->ty};snapshot.goal.entity_id=target_id;snapshot.goal.raw_name=baked::object_identifier(*target);
  Planned before;std::thread worker([&]{before=plan_snapshot(snapshot);});worker.join();
  expect(before.path.success,"Istory destination routes through its native sheep prerequisite");
  if(before.path.success){
   expect(before.pending.valid&&before.pending.goal.entity_id==target_id,"sheep approach retains the final NPC identity");
   const auto& phase=before.path.walk_phases.back();
   expect(!phase.cells.empty()&&phase.cells.back()==Cell{1,35,19}&&phase.manual_after&&phase.arrival_direction==1,
     "first leg stops west of the sheep with manual interaction and East facing");
  }
  snapshot.position=world_of_cell(*town,{1,33,19});snapshot.pending={};snapshot.active={};snapshot.previous_active.reset();
  Planned after;std::thread landed([&]{after=plan_snapshot(snapshot);});landed.join();
  expect(after.path.success&&after.goal.entity_id==target_id,"native kick landing reaches the original NPC");
  std::thread identity([&]{
   const baked::Object* crossing=nullptr;int house=0;
   for(const auto& object:baked::grids(*town).objects){
    if(object.type==baked::ObjType::Warp&&object.entity_id()==21)crossing=&object;
    if(!house&&object.type==baked::ObjType::GotoMap&&object.a>0){const auto* floor=baked::submap_by_map_id(object.a);if(floor&&floor!=town&&!floor->world)house=object.a;}
   }
   expect(crossing!=nullptr,"exact reviewed sheep crossing is baked");
   if(crossing){baked_route::Request request;request.submap=town;
    const auto approach=baked_route::goals_for(request,{1,36,19},baked_route::GoalKind::Interact,crossing);
    expect(approach.cells.size()==1&&approach.cells.front()==Cell{1,35,19},"manual sheep graph has no wrong-side interaction fallback");
   }
   expect(house>0,"Istory has a real connected house for remote discovery");
   if(house){const auto objects=connected_objects_snapshot(house,snapshot.state);int sheep=0,duplicates=0;
    for(const auto& object:objects)if(object.map_id==118&&object.entity_id==21){if(object.type==baked::ObjType::NPC&&object.event_type.empty())++sheep;else ++duplicates;}
    expect(sheep==1&&duplicates==0,"graph companion preserves one remote Sheep NPC without Events/Object/Exit duplicates");
   }
  });identity.join();
  std::printf("Istory NPC%d: prerequisite%d native-landing%d, reason=%ls\n",target_id,before.path.success,after.path.success,before.path.reason.c_str());
 }
 expect(fixture::wrong_owner==0,"Istory planner never queries native objects on its worker");
 std::printf("Istory sheep prerequisite: %d failures\n",failures);return failures?1:0;
}
