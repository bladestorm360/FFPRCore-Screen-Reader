// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define FFPR_ROUTE_FIXTURE_ONLY
#include "route_worker_check.cpp"
#include "../src/games/ff5/mechanics_model.cpp"

namespace exdeath_routes {
using namespace field;
using namespace baked_nav;
int assertions = 0, failures = 0, legs = 0;

void check(bool condition, const char* message) {
    ++assertions;
    if (!condition) {
        ++failures;
        std::printf("FAIL %s\n", message);
    }
}

map_state::Flags flags(int map, int layout, int bridge) {
    map_state::Flags state;
    state.scenario.fill(0);
    state.treasure.fill(0);
    state.global.fill(0);
    state.area.fill(0);
    state.local.fill(0);
    state.current_map_id = map;
    state.current_area_id = 40;
    state.scenario[359] = 1; // The illusion has already been removed before these floors.
    state.area[0] = layout;
    state.area[1] = bridge;
    return state;
}

Planned route(int map, Cell start, const map_state::Flags& state,
              MapGoal goal, TargetKind kind, bool spinning = false, int vehicle = 1,
              std::shared_ptr<const ActiveRoute> previous = {}, std::optional<Cell> observed_goal = {},
              std::optional<Cell> parked_goal = {}) {
    ++legs;
    RouteSnapshot snapshot(map_state::Resolver(state, fixture::records));
    snapshot.map_id = map;
    snapshot.submap = baked::submap_by_map_id(map);
    snapshot.layer = start.layer;
    snapshot.position = world_of_cell(*snapshot.submap, start);
    snapshot.vehicle = {true, vehicle, baked::transport(vehicle)->type, {}};
    snapshot.vehicle_shortcuts = goal.storyline;
    snapshot.locale = L"en";
    snapshot.kind = kind;
    snapshot.goal = std::move(goal);
    if (parked_goal && snapshot.goal.transportation_id > 0) {
        const auto* model = baked::transport(snapshot.goal.transportation_id);
        snapshot.vehicles.push_back({nullptr, model->type, model->id, {}, world_of_cell(*snapshot.submap, *parked_goal)});
    }
    snapshot.previous_active = std::move(previous);
    for (const auto& original : baked::grids(*snapshot.submap).objects) {
        if (original.type == baked::ObjType::PointIn || original.type == baked::ObjType::GotoMap ||
            original.type == baked::ObjType::Warp) continue;
        bool active = snapshot.state.active(*snapshot.submap, original, true);
        if (map == 397 && spinning) {
            // The initializer does not run the center scene. Compare the
            // actual original scene's bounded Show and Hide effects instead.
            const int id = original.entity_id();
            if ((id >= 12 && id <= 23) || (id >= 26 && id <= 33) || (id >= 35 && id <= 40)) active = true;
            if (id == 25) active = false;
        }
        if (!active) continue;
        const auto object = snapshot.state.effective(*snapshot.submap, original);
        NativeObstacle obstacle;
        obstacle.type = int(object.type);
        obstacle.id = object.entity_id();
        obstacle.raw_name = baked::object_identifier(original);
        obstacle.cell = {object.layer, object.tx, object.ty};
        if (observed_goal && snapshot.goal.map_id == map && obstacle.id == snapshot.goal.entity_id &&
            obstacle.raw_name == snapshot.goal.raw_name)
            obstacle.cell = *observed_goal;
        obstacle.npc = object.type == baked::ObjType::NPC || object.type == baked::ObjType::ShopNPC;
        obstacle.chest = object.type == baked::ObjType::TreasureBox;
        obstacle.trigger = baked_route::step_event(object);
        obstacle.collider = object.type == baked::ObjType::CollisionEntity;
        obstacle.collision = object.a;
        obstacle.transportation_ids = snapshot.state.transportation_mask(*snapshot.submap, original);
        obstacle.transportation_restricted = obstacle.transportation_ids != ~uint64_t{0};
        obstacle.applies = (obstacle.transportation_ids & baked_route::transportation_bit(vehicle)) != 0;
        snapshot.obstacles.push_back(std::move(obstacle));
    }
    return plan_snapshot(snapshot);
}

ff5::library::Plan advice(int map, Cell player, const map_state::Flags& state,
                         std::optional<bool> stop = {}, puzzle_policy::Policy policy = {true,true}) {
    ff5::mechanics::Snapshot snapshot;
    snapshot.map = map;
    snapshot.player = player;
    snapshot.flags = state;
    snapshot.exdeath_stop_visible = stop;
    snapshot.locale = L"en";
    snapshot.policy = policy;
    return ff5::mechanics::plan(snapshot, fixture::guide_recipes);
}

void guided_leg(int map, Cell start, int layout, int bridge, int expected,
                const char* message, std::optional<bool> stop = {}) {
    const auto state = flags(map, layout, bridge);
    const auto plan = advice(map, start, state, stop);
    const auto selected = std::find_if(plan.targets.begin(), plan.targets.end(), [expected](const auto& target) {
        return target.goal.entity_id == expected;
    });
    const bool correct = !plan.unknown && selected != plan.targets.end() && plan.targets.size() <= 2;
    check(correct, message);
    if (!correct) return;
    const auto& target = *selected;
    const auto result = route(map, start, state, target.goal, target.kind, stop == true);
    check(result.path.success && !result.path.required_trap, "selected production target has a trap-free route");
    if (!result.path.success || result.path.required_trap)
        std::printf("  map %d at %d,%d,%d layout %d bridge %d: %ls / %ls\n", map, start.layer, start.x, start.y,
                    layout, bridge, result.path.error.c_str(), result.path.reason.c_str());
}

int verify() {
    const Cell stops[] = {{2,5,9},{2,6,9},{2,7,9},{2,7,10},{2,7,11},{2,6,11},{2,5,11},{2,5,10}};
    guided_leg(397, {2,6,17}, 0, 0, 24, "south entry starts the moving floor at its actual center");
    guided_leg(397, {2,6,10}, 0, 0, 27, "native visible stop contact is one cardinal step away", true);
    const auto reload = advice(397, {2,6,10}, flags(397,0,0), false);
    check(reload.targets.empty() && reload.note.find(L"South once") != std::wstring::npos,
          "reload at the center restarts the scene before requesting a hidden stop");
    check(advice(397, {2,6,10}, flags(397,0,0)).unknown, "unknown stop visibility cannot assert a target");
    guided_leg(397, {2,6,3}, 0, 0, 4, "north entry is already beyond the puzzle");
    guided_leg(397, {2,5,6}, 2, 0, 25, "north corridor's west edge uses its actual native reset trigger");
    guided_leg(397, {2,7,6}, 8, 0, 25, "north corridor's east edge uses its actual native reset trigger");
    guided_leg(397, {2,6,6}, 5, 0, 4, "the native north reset permits the next stair route");
    guided_leg(397, {2,6,6}, 2, 0, 4, "a save already on the reset tile can depart north");
    for (int layout = 1; layout <= 9; ++layout) {
        const int goal = layout >= 4 && layout <= 6 ? 4 : 24;
        for (const Cell stop : stops) guided_leg(397, stop, layout, 0, goal,
            "all nine native outcomes progress or return to the center without timing assumptions");
    }
    guided_leg(397, {2,9,6}, 2, 0, 24, "east optional chest branch can retry the floor");
    guided_leg(397, {2,3,6}, 8, 0, 24, "west optional chest branch can retry the floor");
    for (int map : {398,399,400}) {
        const auto* room = baked::submap_by_map_id(map);
        const auto& objects = baked::grids(*room).objects;
        const auto entry = std::find_if(objects.begin(), objects.end(), [](const auto& object) {
            return object.type == baked::ObjType::PointIn && object.a == 1;
        });
        check(entry != objects.end(), "intermediate stair arrival has native PointIn 1");
        if (entry != objects.end())
            guided_leg(map, {entry->layer,entry->tx,entry->ty}, 5, 0, 14,
                "intermediate floors link the moving floor to the actual bridge trigger");
    }
    guided_leg(401, {1,14,9}, 5, 0, 14, "west entrance reaches the actual collapse trigger");
    guided_leg(401, {1,18,9}, 5, 1, 1, "collapse routes around unsafe tiles to restore the bridge");
    guided_leg(401, {1,22,13}, 5, 0, 13, "restored bridge reaches the final exit without re-triggering collapse");
    guided_leg(401, {1,18,14}, 5, 1, 13, "already across the collapsed bridge can leave directly");
    guided_leg(401, {1,26,6}, 5, 0, 10, "isolated east landing returns to the lower floor");
    guided_leg(401, {1,6,9}, 5, 0, 8, "first isolated west landing retains its actual lower stair");
    guided_leg(401, {1,8,11}, 5, 1, 9, "second isolated west landing retains its actual lower stair");
    guided_leg(401, {1,11,7}, 5, 0, 12, "isolated middle landing returns to the lower floor");
    for (Cell landing : {Cell{1,21,6},Cell{1,22,6},Cell{1,20,8},Cell{1,22,8},Cell{1,24,9},Cell{1,23,10}})
        guided_leg(400, landing, 5, 1, 1, "each exact lower-floor fall landing can recover to the restore contact");
    for (const auto& point : baked::grids(*baked::submap_by_map_id(400)).objects) {
        if (point.type != baked::ObjType::PointIn || point.a < 2 || point.a > 6) continue;
        const Cell at{point.layer,point.tx,point.ty};
        guided_leg(400, at, 5, 0, 14, "native return staircase can reach the bridge before collapse");
        guided_leg(400, at, 5, 1, 1, "native return staircase can reach the restore contact after collapse");
    }
    MapGoal ordinary;
    ordinary.map_id = 401;
    ordinary.cell = {1,14,9};
    const auto recovered = route(400, {1,11,8}, flags(400,5,0), ordinary, TargetKind::Spot);
    check(recovered.path.success && !recovered.path.required_trap,
          "ordinary routing also uses the restored same-floor passage");
    const auto& passage = recovered.pending.incoming_transition;
    check(passage && passage->destination == 400 && passage->arrival == Cell{1,15,4},
          "passage ends at the controlled player position, not the cinematic start");
    const auto last = advice(401, {1,22,13}, flags(401,5,0));
    check(last.finish_map == 402, "only the actual destination after the final stair completes the crossing");
    check(advice(397, {2,6,17}, flags(397,0,0), {}, {false,true}).targets.empty(),
          "Navigation toggle disables these routes independently");
    check(!advice(397, {2,6,17}, flags(397,0,0), {}, {true,false}).targets.empty(),
          "Other toggle preserves the navigation solution");
    auto wrong = flags(397,0,0);
    wrong.current_area_id = 39;
    check(advice(397, {2,6,17}, wrong).unknown, "area-local state cannot leak between maps or areas");
    std::printf("Exdeath production: %d assertions, %d physical legs, %d failures\n", assertions, legs, failures);
    return failures || fixture::wrong_owner ? 3 : 0;
}
}

int main() {
    using namespace exdeath_routes;
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    const auto read = [](const char* path) {
        std::ifstream stream(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream), {});
    };
    const auto bytes = read("data/ff5/maps.bin");
    if (!baked::load(bytes.data(), bytes.size()) ||
        !json::parse(read("data/ff5/mapstate.json"), fixture::records) ||
        !json::parse(read("data/ff5/puzzles.json"), fixture::guide_recipes)) return 2;
    fixture::game_id = game::Id::FF5;
    g_loaded = g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = verify(); });
    worker.join();
    return result;
}
