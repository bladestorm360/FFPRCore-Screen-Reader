// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real optional FFIII ridge and actual route/publication consumers; native-free.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
 using namespace field;using namespace baked_nav;
 std::ifstream file("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(file),{}};
 if(!baked::load(bytes.data(),bytes.size()))return 2;
 std::ifstream metadata("data/ff3/mapstate.json",std::ios::binary);std::string text{std::istreambuf_iterator<char>(metadata),{}};
 if(!json::parse(text,fixture::records))return 2;
 fixture::game_id=game::Id::FF3;g_loaded=g_prepared=true;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 const auto* world=baked::submap_by_map_id(4);
 auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->treasure.fill(0);flags->global.fill(0);
 flags->scenario[73]=flags->scenario[905]=1;
 // A real two-cell ridge has a legal route around it. Even a much longer
 // ordinary flight must win over the optional three-tile manual action.
 {
  baked_route::Request move;move.submap=world;move.start={1,175,20};move.goals={{1,175,23}};
  const auto* row=baked::transport(10);move.vehicle_group=row->group;move.vehicle_type=row->type;move.transportation_id=10;
  move.live_hazards=move.live_traps=true;move.transitions=std::make_shared<baked::TransitionGrid>();move.goals_open=false;
  baked_route::Route path;baked_route::Reach reach;
  std::thread worker([&]{path=baked_route::find(move);reach=baked_route::flood(move);});worker.join();
  int jumps=0;for(size_t i=1;i<path.cells.size();++i)jumps+=baked_route::step_direction(path.cells[i-1],path.cells[i],world->width,world->height)<0;
  expect(path.found&&jumps==0&&path.steps()>3,"real optional ridge uses ordinary flight even when the detour is longer");
  expect(reach.crossings(move.goals.front())==0&&reach.distance(move.goals.front())>3,"landing/filter flood follows the same ordinary-flight priority and preserves physical steps");
  auto snapshot=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,fixture::records));
  snapshot->map_id=4;snapshot->submap=world;snapshot->layer=1;snapshot->locale=L"en";snapshot->vehicle={true,10,8,L""};
  snapshot->position=world_of_cell(*world,move.start);snapshot->kind=TargetKind::Spot;snapshot->goal.map_id=4;snapshot->goal.cell=move.goals.front();
  Planned plan;std::thread publisher([&]{plan=plan_snapshot(*snapshot);});publisher.join();
  expect(plan.path.success&&plan.path.step_count==5&&std::none_of(plan.path.walk_phases.begin(),plan.path.walk_phases.end(),[](const auto& p){return p.manual_displacement_steps>0;}),
    "actual published ordinary flight omits the optional manual mountain instruction");
  std::printf("Optional real ridge: route%d steps%d crossings%d flood%d/%d\n",path.found,path.steps(),jumps,reach.distance(move.goals.front()),reach.crossings(move.goals.front()));
 }
 expect(fixture::wrong_owner==0,"all route work remains native-free");
 std::printf("Invincible preference: %d failures\n",failures);return failures?1:0;
}
