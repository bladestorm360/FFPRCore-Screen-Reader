// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual Saronia data and production preview/direction planner. A later-floor
// Soldier remains a real obstacle; the current leg must reach its floor first.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
namespace {
using namespace field;using namespace baked_nav;
std::unique_ptr<RouteSnapshot> snapshot(int map,Cell start,map_state::Flags flags){
 flags.current_map_id=map;
 auto out=std::make_unique<RouteSnapshot>(map_state::Resolver(flags,fixture::records));
 out->map_id=map;out->submap=baked::submap_by_map_id(map);out->layer=start.layer;out->locale=L"en";
 out->position=world_of_cell(*out->submap,start);
 for(const auto& original:baked::grids(*out->submap).objects){
  if(!out->state.active(*out->submap,original))continue;const auto o=out->state.effective(*out->submap,original);
  NativeObstacle actor;actor.cell={o.layer,o.tx,o.ty};actor.id=o.entity_id();actor.type=static_cast<int>(o.type);actor.raw_name=baked::object_identifier(o);
  actor.npc=o.type==baked::ObjType::NPC||o.type==baked::ObjType::ShopNPC;actor.chest=o.type==baked::ObjType::TreasureBox;
  actor.trigger=(o.type==baked::ObjType::Event||o.type==baked::ObjType::RandomEvent)&&(o.a||o.b);
  actor.collider=o.type==baked::ObjType::CollisionEntity;actor.collision=o.a;
  if(actor.npc||actor.chest||actor.trigger||actor.collider||((o.type==baked::ObjType::Entity||o.type==baked::ObjType::AnimEntity)&&(o.d&1)))out->obstacles.push_back(std::move(actor));
 }
 return out;
}
}
int main(){
 int failures=0;auto expect=[&](bool okay,const char* why){if(!okay){++failures;std::printf("FAIL %s\n",why);}};
 std::ifstream maps("data/ff3/maps.bin",std::ios::binary);std::string blob{std::istreambuf_iterator<char>(maps),{}};
 std::ifstream ir("data/ff3/mapstate.json",std::ios::binary);std::string text{std::istreambuf_iterator<char>(ir),{}};
 if(!baked::load(blob.data(),blob.size())||!json::parse(text,fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.scenario[73]=flags.scenario[74]=1;
 const auto* target=baked::submap_by_map_id(106);const baked::Object* chest=nullptr;
 for(const auto& o:baked::grids(*target).objects)if(o.type==baked::ObjType::TreasureBox&&o.tx==12&&o.ty==33)chest=&o;
 if(!chest)return 3;
 auto remote=snapshot(105,{0,22,8},flags);
 remote->goal.map_id=106;remote->goal.cell={chest->layer,chest->tx,chest->ty};remote->goal.entity_id=chest->entity_id();remote->goal.raw_name=baked::object_identifier(*chest);remote->kind=TargetKind::Interact;
 Planned plan;BrowsePlanned preview;
 std::thread worker([&]{plan=plan_snapshot(*remote);preview=plan_browse_snapshot(*remote);});worker.join();
 std::printf("remote success=%d next=%d npc=%d preview direct=%d reason chars=%zu\n",plan.path.success,plan.path.next_map_id,plan.path.npc_approach,preview.route.direct,preview.route.reason.size());
 expect(plan.path.success&&plan.path.next_map_id==104,"Saronia chest on another floor routes to the Soldier's floor rather than No path");
 expect(preview.route.via_map_id==104,"remote browse identifies the required Soldier floor rather than Not directly reachable");
 auto local=snapshot(104,{0,17,31},flags);local->goal=remote->goal;local->kind=remote->kind;
 std::thread entered([&]{plan=plan_snapshot(*local);});entered.join();
 std::printf("entered success=%d next=%d npc=%d phases=%zu\n",plan.path.success,plan.path.next_map_id,plan.path.npc_approach,plan.path.walk_phases.size());
 expect(plan.path.success&&plan.path.npc_approach,"after entering the floor the actual Soldier interaction prefix remains available");
 expect(plan.path.walk_phases.size()==1&&plan.path.walk_phases.front().cells.size()==2&&
     plan.path.walk_phases.front().cells.back()==Cell{0,17,30},"reported one-step North ends beside the Soldier, never on his cell");
 NativeObstacle soldier;bool found=false;
 for(const auto& actor:local->obstacles)if(actor.npc&&actor.id==24){soldier=actor;found=true;break;}
 expect(found,"reported Soldier metadata is available for the displaced live-NPC witness");
 local->obstacles.erase(std::remove_if(local->obstacles.begin(),local->obstacles.end(),[](const auto& actor){return actor.npc&&actor.id==24;}),local->obstacles.end());
 std::thread cleared([&]{plan=plan_snapshot(*local);});cleared.join();
 expect(plan.path.success&&plan.path.next_map_id==106&&!plan.path.npc_approach,"when the native Soldier is gone the original chest continuation resumes");
 const Cell doorway=remote->active.phases.empty()?Cell{0,22,9}:remote->active.phases.front().cells.back();
 auto blocked=snapshot(105,{0,22,8},flags);blocked->goal=remote->goal;blocked->kind=remote->kind;
 if(found){soldier.cell=doorway;soldier.solidity_known=soldier.solid=true;soldier.movement_known=true;soldier.wanders=false;blocked->obstacles.push_back(soldier);
  std::thread current([&]{plan=plan_snapshot(*blocked);});current.join();
  expect(plan.path.success,"a current-floor NPC permits a legal detour or interaction prefix");
  if(plan.path.npc_approach)expect(plan.path.next_map_id==0,"an interaction prefix stops before a floor transition");
  expect(plan.pending.valid&&plan.pending.goal.map_id==106&&plan.pending.goal.cell==remote->goal.cell,"NPC approach retains the original remote chest objective");
  for(const auto& phase:plan.path.walk_phases)for(const auto& cell:phase.cells)expect(!(cell==doorway),"current leg cannot walk through its blocking NPC");
 }
 auto gated=snapshot(105,{0,22,8},flags);gated->goal=remote->goal;gated->kind=remote->kind;
 NativeObstacle gate;gate.type=19;gate.id=0;gate.collider=true;gate.collision=1;gate.cell=doorway;gated->obstacles.push_back(gate);
 std::thread closed([&]{plan=plan_snapshot(*gated);});closed.join();
 if(plan.path.success){
  for(const auto& phase:plan.path.walk_phases)for(const auto& cell:phase.cells)expect(!(cell==doorway),"a valid alternate exit must still avoid the current native solid gate");
 }else expect(plan.path.reason==L"Story gated","a current solid gate retains its blocked-route explanation");
 expect(!fixture::wrong_owner,"cross-floor and entered-floor planners read owned data only");
 std::printf("Cross-floor NPC regression: %d failures; no game run\n",failures);
 return failures?1:0;
}
