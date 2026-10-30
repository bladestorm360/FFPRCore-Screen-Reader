// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Actual owned provider/category pipeline, including a valid native Player1.
#define main retained_route_worker_main
#include "route_worker_check.cpp"
#undef main
int main(){
    using namespace field;using namespace baked_nav;using namespace filter_worker;
    int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    std::ifstream f("data/ff3/maps.bin",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(f),{}};
    if(!baked::load(bytes.data(),bytes.size()))return 2;
    std::ifstream j("data/ff3/mapstate.json",std::ios::binary);std::string records{std::istreambuf_iterator<char>(j),{}};
    if(!json::parse(records,fixture::records))return 3;
    fixture::game_id=game::Id::FF3;const auto* world=baked::submap_by_map_id(4);
    auto flags=std::make_unique<map_state::Flags>();flags->scenario.fill(0);flags->scenario[66]=1;flags->current_map_id=4;
    auto capture=std::make_shared<ExitSnapshot>(false,map_state::Resolver(*flags,fixture::records));
    auto& snapshot=capture->route;snapshot.map_id=4;snapshot.submap=world;snapshot.layer=1;snapshot.locale=L"en";
    snapshot.position=world_of_cell(*world,{1,83,62});snapshot.vehicle={true,1,1,L""};snapshot.auto_groups={3};
    FilterInput input;input.current_map=4;input.world=true;input.navigation=capture;input.reachable_only=false;
    input.player=snapshot.position;input.sort=true;input.state=std::make_shared<map_state::Resolver>(*flags,fixture::records);
    Categories all;std::thread discover([&]{all=build_categories(input);});discover.join();
    expect(all.air_target.has_value()&&all.views[static_cast<size_t>(Category::MapExits)].order.size()==1,
        "unfiltered Current map publishes one nearest physical aerial exit even without an immediate flight");
    expect(all.story_target.has_value()&&all.views[static_cast<size_t>(Category::Events)].order.size()==1,
        "unfiltered Current map publishes active Saronia Airspace while on foot without a flight proof");
    input.rows.clear();input.reachable_only=true;
    std::thread strict([&]{all=build_categories(input);});strict.join();
    expect(!all.air_target&&!all.story_target,"reachable modes cannot claim a usable aerial entry without a real vehicle");
    // The parked Nautilus witness belongs after Saronia and its release; do
    // not combine its ownership with the earlier active-airspace plot state.
    flags->scenario[73]=1;flags->scenario[905]=1;
    auto later_capture=std::make_shared<ExitSnapshot>(true,map_state::Resolver(*flags,fixture::records));
    auto& later=later_capture->route;later.map_id=snapshot.map_id;later.submap=world;later.layer=snapshot.layer;
    later.locale=snapshot.locale;later.position=snapshot.position;later.vehicle=snapshot.vehicle;later.auto_groups=snapshot.auto_groups;
    input.navigation=later_capture;
    input.state=std::make_shared<map_state::Resolver>(*flags,fixture::records);
    VehicleOnMap parked;parked.transportation_id=8;parked.type=3;parked.pos=world_of_cell(*world,{1,83,63});later.vehicles={parked};
    input.rows.clear();std::thread boardable([&]{all=build_categories(input);});boardable.join();
    expect(all.air_target&&all.air_target->distance>=0,"valid native Player1 still considers a reachable parked Nautilus before boarding");
    expect(!all.story_target,"the later Nautilus witness cannot restore the consumed Saronia scene");
    if(all.air_target){
        auto route=std::make_unique<RouteSnapshot>(later.state);route->map_id=4;route->submap=world;route->layer=1;route->locale=L"en";
        route->position=later.position;route->vehicle=later.vehicle;route->vehicles=later.vehicles;route->auto_groups=later.auto_groups;
        route->goal=all.air_target->goal;route->kind=TargetKind::AirshipTransition;route->vehicle_shortcuts=true;
        Planned plan;std::thread selected([&]{plan=plan_snapshot(*route);});selected.join();
        expect(plan.path.success&&!plan.path.walk_phases.empty()&&plan.path.walk_phases.back().cells.back()==route->goal.cell,
            "the published parked-vehicle exit retains a legal selected boarding/flight route");
    }
    expect(fixture::wrong_owner==0,"provider discovery/category/planning never reads native objects on a worker");
    std::printf("Unfiltered/parked aerial provider: %d failures\n",failures);return failures?1:0;
}
