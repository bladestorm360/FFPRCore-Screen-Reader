// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Focused connected-floor regressions; keep large legacy fixture main off the stack.
#define main retained_route_fixture_main
#include "route_worker_check.cpp"
#undef main
__declspec(noinline) int check_kazus_routes(const field::map_state::Flags& chest_flags,const json::Value& chest_records) {
    using namespace field;using namespace field::baked_nav;using namespace field::baked_route;
    int failures=0;auto expect=[&](bool ok,const char*why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
            // User's exact Kazus report: both directions through the inn's
            // stairs, plus Mines B1 -> upstairs bed. No renderer/game process.
            for(const auto& endpoints:std::vector<std::pair<int,int>>{{13,15},{15,13},{159,15}}) {
                auto flags=std::make_unique<map_state::Flags>(chest_flags);flags->scenario[16]=1;
                auto owned=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,chest_records));auto& check=*owned;
                check.map_id=endpoints.first;check.submap=baked::submap_by_map_id(check.map_id);check.layer=1;check.locale=L"en";
                const Cell start=check.map_id==13?Cell{1,22,18}:check.map_id==15?Cell{1,6,14}:Cell{1,10,20};
                check.position=world_of_cell(*check.submap,start);check.goal.map_id=endpoints.second;
                check.goal.cell=endpoints.second==15?Cell{1,10,7}:Cell{1,22,18};
                check.kind=endpoints.second==15?TargetKind::Interact:TargetKind::Spot;
                if(endpoints.second==15){check.goal.entity_id=20;check.goal.raw_name=L"HP/MP回復(右)";}
                NativeObstacle follower;follower.type=6;follower.npc=true;follower.id=0;follower.raw_name=L"FieldCompanionNpc";follower.cell=check.map_id==15?Cell{1,6,15}:start;follower.solidity_known=true;follower.solid=false;check.obstacles.push_back(follower);
                Planned plan;std::thread worker([&]{plan=plan_snapshot(check);});worker.join();
                std::printf("Kazus %d -> %d: success%d next%d steps%d\n",endpoints.first,endpoints.second,plan.path.success,plan.path.next_map_id,plan.path.step_count);
                expect(plan.path.success&&!plan.path.npc_approach,"Kazus inn second floor and Mines retain both-direction cross-floor paths without a false follower warning");
                if(check.map_id==15){check.obstacles.back().solid=true;Planned blocked;std::thread old([&]{blocked=plan_snapshot(check);});old.join();expect(!blocked.path.success||blocked.path.npc_approach,"a misclassified solid follower reproduces the Kazus stair blocker/wait prefix");}
            }
    auto flags=std::make_unique<map_state::Flags>(chest_flags);flags->scenario[16]=1;
    auto woman=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,chest_records));woman->submap=baked::submap_by_map_id(14);woman->map_id=14;woman->layer=1;woman->locale=L"en";woman->position=world_of_cell(*woman->submap,{1,18,11});woman->goal.map_id=14;woman->goal.cell={1,11,7};woman->goal.entity_id=9;woman->goal.raw_name=L"4:女性4";woman->kind=TargetKind::Interact;
    NativeObstacle actor;actor.type=6;actor.npc=true;actor.id=9;actor.cell={1,11,7};actor.raw_name=woman->goal.raw_name;woman->obstacles.push_back(actor);
    Planned approach;std::thread walk([&]{approach=plan_snapshot(*woman);});walk.join();expect(approach.path.success,"Kazus Woman uses her logged moving position and legal approach, not her baked home as an obstacle");
    fixture::capture_ready=true;fixture::capture_companion=true;fixture::current_position=woman->position;
    const auto captured=capture_obstacle(reinterpret_cast<void*>(1),*woman);expect(captured.npc&&captured.solidity_known&&!captured.solid,"actual native capture overrides a follower's collider with its swappable companion contract");
    fixture::capture_ready=fixture::capture_companion=false;
    return failures;
}

__declspec(noinline) int check_sasune_and_treasure(const field::map_state::Flags& initial,const json::Value& records) {
 using namespace field;using namespace field::baked_nav;using namespace field::baked_route;
 int failures=0;auto expect=[&](bool b,const char* why){if(!b){++failures;std::printf("FAIL %s\n",why);}};
 auto flags=std::make_unique<map_state::Flags>(initial);flags->scenario[16]=1;
 for(int from:{23,24,25,26,27,28,29,30,31,32,33,34,35}) {
  map_state::Resolver state(*flags,records);std::vector<RemoteObject> other;
  const auto begin=std::chrono::steady_clock::now();std::thread worker([&]{other=connected_objects_snapshot(from,state);});worker.join();
  bool sara=false,spring=false;for(const auto& r:other){sara|=r.map_id==35&&r.type==baked::ObjType::NPC&&r.entity_id==5;spring|=r.type==baked::ObjType::GotoMap&&r.dest_map_id==27;}
  if(from!=35)expect(sara,"Sasune other-floor NPC survives visual fog/unknown follower state from every connected floor");
  if(from!=23&&from!=27)expect(spring,"spring entrance remains selectable from other remote floors");
  if(from==27)expect(!spring,"the spring's own remote reciprocal return is omitted while the current spring remains live-owned");
  std::printf("Sasune floor%d remote%zu worker+join %.3f ms\n",from,other.size(),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
 }
 const auto* town=baked::submap_by_map_id(36);const auto& grid=baked::grids(*town);
 // The logged Tozus chest uses saved bit0. Exercise complete cross-floor
 // row projection independently of the loose-pickup identity bit.
 for(const auto bit:std::vector<std::optional<int>>{0,1,std::nullopt}) {
  flags->treasure[0]=bit;map_state::Resolver state(*flags,records);
  const auto rows=connected_objects_snapshot(48,state);
  const auto chest=std::find_if(rows.begin(),rows.end(),[](const auto& r){return r.map_id==47&&r.entity_id==6&&r.type==baked::ObjType::TreasureBox;});
  expect(chest!=rows.end(),"flag-zero Tozus chest remains present on another floor");
  if(chest!=rows.end())expect(!chest->pickup&&chest->opened==(bit?std::optional<bool>(*bit!=0):std::nullopt),"remote Tozus chest preserves opened/unopened/unknown state independently of pickup identity");
 }
 flags->treasure[0]=0;
 const auto at=std::find_if(grid.objects.begin(),grid.objects.end(),[](const auto&o){return o.type==baked::ObjType::TreasureBox&&o.entity_id()==39;});
 expect(at!=grid.objects.end()&&baked::pickup_treasure(*at),"Caanan's actual disappearing treasure retains pickup metadata");
 if(at!=grid.objects.end())for(const auto& start:std::vector<Cell>{{1,29,18},{0,33,30},{0,31,33},{0,33,33},{1,32,31}}) {
  auto owned=std::make_unique<RouteSnapshot>(map_state::Resolver(*flags,records));auto& check=*owned;check.submap=town;check.map_id=36;check.layer=start.layer;check.position=world_of_cell(*town,start);check.locale=L"en";
  check.goal.map_id=36;check.goal.cell={at->layer,at->tx,at->ty};check.goal.entity_id=39;check.goal.raw_name=baked::object_identifier(*at);check.kind=TargetKind::Interact;
  NativeObstacle chest;chest.type=7;chest.chest=true;chest.id=39;chest.cell=check.goal.cell;chest.raw_name=check.goal.raw_name;check.obstacles.push_back(chest);
  Planned plan;std::thread worker([&]{plan=plan_snapshot(check);});worker.join();
  expect(plan.path.success&&!plan.path.walk_phases.empty(),"Caanan route reaches a real Ground-layer interaction from each logged approach");
  if(plan.path.success&&!plan.path.walk_phases.empty()) {
   const auto& last=plan.path.walk_phases.back();expect(last.cells.back()==Cell{1,32,31}&&last.arrival_direction==1,"hidden treasure route ends west of the item and faces east, never on the river layer");
  }
 }
 return failures;
}

int main(){
 using namespace field;using namespace field::baked_nav;
 fixture::game_id=game::Id::FF3;
 std::setvbuf(stdout,nullptr,_IONBF,0);
 std::ifstream f("data/ff3/maps.bin",std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(f)),{});
 if(!baked::load(bytes.data(),bytes.size()))return 1;
 for(int i=0;i<baked::submap_count();++i)baked::grids(*baked::submap_at(i));g_loaded=g_prepared=true;
 std::ifstream j("data/ff3/mapstate.json");const std::string text((std::istreambuf_iterator<char>(j)),{});json::Value records;if(!json::parse(text,records))return 1;
 auto flags=std::make_unique<map_state::Flags>();for(auto& x:flags->scenario)x=0;for(auto&x:flags->treasure)x=0;for(auto&x:flags->global)x=0;
 const int failures=check_kazus_routes(*flags,records)+check_sasune_and_treasure(*flags,records);std::printf("Connected floor regression: %d failures\n",failures);return failures?1:0;
}
