// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main

namespace story_contacts {
using namespace field;
using namespace baked_nav;
using exdeath_routes::check;
using exdeath_routes::route;

const baked::Object& object(int map, int entity) {
    const auto& objects = baked::grids(*baked::submap_by_map_id(map)).objects;
    const auto actor = std::find_if(objects.begin(),objects.end(),[entity](const auto& row) {
        return row.entity_id() == entity;
    });
    if (actor == objects.end()) std::abort();
    return *actor;
}

bool active(const map_state::Flags& state, int entity) {
    map_state::Resolver resolver(state,fixture::records);
    const auto& room = *baked::submap_by_map_id(state.current_map_id);
    return resolver.active(room,object(state.current_map_id,entity));
}

bool reach(const map_state::Flags& state, Cell from, int entity, TargetKind kind, int vehicle = 1) {
    const auto& actor = object(state.current_map_id,entity);
    MapGoal goal;
    goal.map_id = state.current_map_id;
    goal.cell = {actor.layer,actor.tx,actor.ty};
    goal.entity_id = entity;
    goal.raw_name = baked::object_identifier(actor);
    const auto result = route(goal.map_id,from,state,goal,kind,false,vehicle);
    if (!result.path.success) std::printf("Contact %d/%d: %ls / %ls\n",goal.map_id,entity,
                                          result.path.error.c_str(),result.path.reason.c_str());
    return result.path.success && !result.path.required_trap;
}

int verify() {
    auto walse = exdeath_routes::flags(1,0,0);
    walse.current_area_id = 1;
    MapGoal tower;
    tower.map_id = 1;
    tower.cell = {1,196,88};
    const auto approach_tower = route(1,{1,195,87},walse,tower,TargetKind::Transition);
    check(approach_tower.path.success && !approach_tower.path.walk_phases.empty() &&
          approach_tower.path.walk_phases.back().cells.back() == tower.cell,
          "the logged walking pose reaches the exact Walse Tower entrance before the sinking");
    const auto flight_tower = route(1,{1,214,116},walse,tower,TargetKind::Transition,false,4);
    check(flight_tower.path.success && !flight_tower.path.walk_phases.empty() &&
          flight_tower.path.walk_phases.back().cells.back() == tower.cell,
          "the Wind Drake can fly from the Walse settlement and land for the tower approach");
    MapGoal meteor;
    meteor.map_id = 1;
    meteor.cell = {1,204,84};
    walse.scenario[37] = walse.scenario[991] = 1;
    const auto approach_meteor = route(1,{1,205,84},walse,meteor,TargetKind::Transition);
    check(approach_meteor.path.success && !approach_meteor.path.walk_phases.empty() &&
          approach_meteor.path.walk_phases.back().cells.back() == meteor.cell,
          "the Walse Meteorite uses its exact scripted entrance after the tower sinks");
    const auto flight_meteor = route(1,{1,214,116},walse,meteor,TargetKind::Transition,false,4);
    check(flight_meteor.path.success && !flight_meteor.path.walk_phases.empty() &&
          flight_meteor.path.walk_phases.back().cells.back() == meteor.cell,
          "the Wind Drake can land on the surviving eastern shore for the meteorite");
    check(!route(1,{1,195,87},walse,tower,TargetKind::Transition).path.success,
          "the same tower entrance cannot be reached once its native foot ID becomes water");
    walse.scenario[990] = 1;
    check(route(1,{1,205,84},walse,meteor,TargetKind::Transition).path.success,
          "the post-meteor terrain conversion also permits its active entrance");
    walse.scenario[991].reset();
    check(!route(1,{1,195,87},walse,tower,TargetKind::Transition).path.success,
          "unknown sinking state does not invent a walkable tower entrance");

    auto valley = exdeath_routes::flags(577,0,0);
    valley.current_area_id = 4;
    valley.scenario[5] = valley.scenario[417] = 1;
    for (int entity : {13,17,18}) {
        const auto& room = *baked::submap_by_map_id(577);
        map_state::Resolver resolver(valley,fixture::records);
        check(active(valley,entity) && resolver.transportation_mask(room,object(577,entity)) == (1ull<<3),
              "the real Boko contact is enabled for the scene's transportation 3");
        check(reach(valley,{1,13,57},entity,TargetKind::Spot,3),"each equivalent jump contact has a native approach");
    }
    check(!active(valley,12),"the first goblin contact is retired once the battle receipt is present");
    valley.scenario[418] = 1;
    const auto projection = map_state::project(*fixture::records.get(L"Map_40001"),valley);
    check(projection.known && projection.entities.contains(8) && projection.entities.contains(9),
          "the copied native initializer resolves the two earthquake regions after the jumps");
    check(active(valley,14),"the northern goblin contact remains available after the jump");
    valley.scenario[419] = 1;
    check(!active(valley,14),"the northern contact is retired by its own battle receipt");

    auto forest = exdeath_routes::flags(129,0,0);
    forest.current_area_id = 32;
    check(!active(forest,96),"recruitment does not activate before the native forest prerequisite");
    forest.scenario[55] = 1;
    check(active(forest,96) && reach(forest,{1,17,28},96,TargetKind::Spot),
          "the forest entrance reaches the automatic recruitment scene");
    forest.scenario[415] = 1;
    check(active(forest,111) && reach(forest,{1,17,24},111,TargetKind::Interact),
          "after the scene the black chocobo can be approached for the native conversation");

    auto village = exdeath_routes::flags(136,0,0);
    village.current_area_id = 44;
    village.scenario[93] = 1;
    check(active(village,29) && reach(village,{0,17,26},29,TargetKind::Interact),
          "the moogle conversation is reachable during the native scene phase");
    MapGoal house;
    house.map_id = 139;
    house.cell = {1,7,11};
    check(!route(136,{0,34,16},village,house,TargetKind::Spot).path.success,
          "the third house remains gated before the conversation receipt");
    village.scenario[94] = 1;
    const auto opened = route(136,{0,34,16},village,house,TargetKind::Spot);
    check(!active(village,29) && opened.path.success && opened.pending.incoming_transition &&
          opened.pending.incoming_transition->destination == 139,
          "the completed conversation opens the correct house and retires the old moogle");
    village.current_map_id = 139;
    MapGoal outside;
    outside.map_id = 136;
    outside.cell = {0,34,15};
    const auto back = route(139,{1,7,11},village,outside,TargetKind::Spot);
    check(back.path.success && back.pending.incoming_transition && back.pending.incoming_transition->arrival == outside.cell,
          "the third house returns to its own entrance in the village");

    auto pirates = exdeath_routes::flags(191,0,0);
    pirates.current_area_id = 81;
    MapGoal cave;
    cave.map_id = 192;
    cave.cell = {1,16,19};
    const auto entrance = route(191,{1,30,47},pirates,cave,TargetKind::Spot);
    check(entrance.path.success && entrance.path.next_map_id == 192 && !entrance.path.required_trap &&
          route_warning(entrance.path,cave) == L"Event required",
          "first Pirate's Hideout entrance uses the playable scene transfer and warns of the event");
    check(entrance.path.description.find(L"warp") == std::wstring::npos,
          "the cave conversation's counted directions never describe a warp");

    pirates.current_map_id = 1;
    pirates.current_area_id = 1;
    for (const int flag : {7,8,9,12}) pirates.scenario[flag] = 1;
    MapGoal south_entrance;
    south_entrance.map_id = 1;
    south_entrance.cell = {1,165,115};
    const auto cave_return = route(1,{1,166,107},pirates,south_entrance,TargetKind::Transition);
    if (!cave_return.path.success)
        std::printf("Cave 3F return: %ls / %ls\n",cave_return.path.error.c_str(),cave_return.path.reason.c_str());
    check(cave_return.path.success && cave_return.path.next_map_id == 194 && !cave_return.path.required_trap,
          "the logged world position uses the Cave 3F connector to the scripted Cave Entrance without a vehicle");

    auto tule = exdeath_routes::flags(30,0,0);
    tule.current_area_id = 9;
    tule.scenario[21] = 1;
    MapGoal world_exit;
    world_exit.map_id = 30;
    world_exit.entity_id = 135;
    world_exit.cell = {1,32,51};
    const Cell scene_cell{1,32,49};
    auto check_tule_scene = [&](Cell from, const char* message,
                                std::shared_ptr<const ActiveRoute> previous = {}) {
        const auto result = route(30,from,tule,world_exit,TargetKind::Transition,false,1,std::move(previous));
        if (!result.path.success)
            std::printf("Tule: %ls / %ls\n",result.path.error.c_str(),result.path.reason.c_str());
        check(result.path.success && !result.path.walk_phases.empty() &&
              result.path.walk_phases.back().cells.back() == scene_cell &&
              result.path.warning == L"Event required" && !result.path.required_trap &&
              result.path.description.find(L"warp") == std::wstring::npos, message);
        return result;
    };
    check(active(tule,58), "native Tule initializer activates the pirate farewell");
    const auto first = check_tule_scene({1,21,38}, "the reported Tule route ends at the farewell that exits to the World Map");
    if (first.path.success && first.path.walk_phases.back().cells.size() > 1) {
        const auto next = first.path.walk_phases.back().cells[1];
        check_tule_scene(next, "a repeated request keeps the active scene endpoint and warning", first.active);
    }
    check_tule_scene({1,32,48}, "one step before the farewell the route still announces the required event");
    tule.scenario[22] = 1;
    const auto exit_after = route(30,{1,21,38},tule,world_exit,TargetKind::Transition,false,1,first.active);
    check(!active(tule,58) && exit_after.path.success && !exit_after.path.walk_phases.empty() &&
          exit_after.path.walk_phases.back().cells.back() == world_exit.cell && exit_after.path.warning.empty(),
          "the completed farewell restores the ordinary World Map exit without a warning");
    tule.scenario[21] = tule.scenario[22] = 0;
    tule.scenario[273] = 1;
    check(active(tule,100), "native Tule initializer activates the alternate Faris scene");
    check_tule_scene({1,21,38}, "the alternate Faris scene also fulfills the selected exit");
    std::printf("FFV story contacts: %d assertions, %d route legs, %d failures\n",
                exdeath_routes::assertions,exdeath_routes::legs,exdeath_routes::failures);
    return fixture::wrong_owner || exdeath_routes::failures ? 3 : 0;
}
}

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    const auto read = [](const char* path) {
        std::ifstream stream(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream),{});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!field::baked::load(bytes.data(),bytes.size()) || !json::parse(read("data/ff5/mapstate.json"),fixture::records)) return 2;
    fixture::game_id = game::Id::FF5;
    field::baked_nav::g_loaded = field::baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = story_contacts::verify(); });
    worker.join();
    return result;
}
