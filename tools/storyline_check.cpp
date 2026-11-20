// check-sources: field/storyline.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
#define main retained_exdeath_route_main
#include "ff5_exdeath_route_check.cpp"
#undef main
#include "../src/core/field/storyline.h"

namespace {
using namespace field;
using namespace field::storyline;
using exdeath_routes::check;

map_state::Flags progress_at(const CorneliaProgress& progress, int map)
{
    map_state::Flags flags;
    flags.scenario.fill(0);
    flags.treasure.fill(0);
    flags.global.fill(0);
    flags.area.fill(0);
    flags.local.fill(0);
    flags.current_map_id = map;
    flags.scenario[4] = progress.bridge;
    flags.scenario[9] = progress.ship;
    for (const auto& [index, value] : std::array<std::pair<int, std::optional<int>>, 7>{{
         {0, progress.opening}, {1, progress.audience}, {2, progress.garland},
         {3, progress.returned}, {5, progress.lute}, {6, progress.bridge}, {7, progress.crossed}}})
        flags.scenario[index] = value;
    if (map == 1) flags.current_area_id = 1;
    else if (map == 13) flags.current_area_id = 29;
    else if (map == 72) flags.current_area_id = 11;
    else flags.current_area_id = 16;
    return flags;
}

void replay(const CorneliaProgress& progress, ObjectiveId expected, int origin, baked_route::Cell at,
            int goal_map, std::vector<int> contacts_expected)
{
    const auto objective = cornelia_objective(progress);
    check(objective && objective->id == expected, "persistent receipts select the expected objective");
    if (!objective) return;
    const auto targets = contacts(*objective);
    std::vector<int> actual;
    for (const auto& target : targets) actual.push_back(target.goal.entity_id);
    check(actual == contacts_expected, "exact baked placements and authored group are retained");
    if (targets.empty()) return;
    const auto& target = targets.front();
    check(target.goal.map_id == goal_map && target.goal.allow_world_transit,
          "route goal is independent of current position and supports intervening world travel");
    const auto route = exdeath_routes::route(origin, at, progress_at(progress, origin), target.goal, target.kind);
    check(route.path.success && !route.path.required_trap, "the production worker plans a verified first movement leg");
    if (!route.path.success || route.path.required_trap) {
        std::printf("  map %d at %d/%d/%d, target %d at %d/%d/%d, %ls, %ls\n", origin, at.layer, at.x, at.y,
                    target.goal.map_id, target.goal.cell.layer, target.goal.cell.x, target.goal.cell.y,
                    route.path.error.c_str(), route.path.reason.c_str());
    }
}

int verify()
{
    const CorneliaProgress audience{1,0,0,0,0,0,0};
    const CorneliaProgress garland{1,1,0,0,0,0,0};
    const CorneliaProgress lute{1,1,1,1,0,0,0};
    const CorneliaProgress leave{1,1,1,1,1,0,0};
    const CorneliaProgress bridge{1,1,1,1,1,1,0};
    const CorneliaProgress ship{1,1,1,1,1,1,1,0};
    check(!cornelia_objective({0,0,0,0,0,0,0}), "opening must finish first");
    check(!cornelia_objective({1,1,1,0,0,0,0}), "rescue return transition withholds the next objective");
    check(!cornelia_objective({1,1,0,1,0,0,0}), "a missing earlier receipt is ambiguous");
    check(!cornelia_objective({1,1,1,1,1,1,{}}), "unavailable receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1}), "an unavailable ship receipt stays unknown");
    check(!cornelia_objective({1,1,1,1,1,1,1,1}), "an existing save with the ship skips the earlier stages");
    replay(audience, ObjectiveId::Audience, 3, {2,20,17}, 3, {14});
    replay(audience, ObjectiveId::Audience, 2, {2,26,41}, 3, {14});
    replay(audience, ObjectiveId::Audience, 1, {1,145,156}, 3, {14});
    replay(garland, ObjectiveId::Garland, 72, {0,38,31}, 72, {32});
    replay(garland, ObjectiveId::Garland, 1, {1,145,156}, 72, {32});
    replay(garland, ObjectiveId::Garland, 2, {2,26,41}, 72, {32});
    replay(lute, ObjectiveId::Lute, 3, {0,20,9}, 3, {7});
    replay(leave, ObjectiveId::LeaveCastle, 2, {2,26,41}, 2, {45,44,46});
    replay(bridge, ObjectiveId::CrossBridge, 1, {1,145,146}, 1, {89});
    replay(bridge, ObjectiveId::CrossBridge, 1, {1,145,156}, 1, {89});
    replay(ship, ObjectiveId::GetShip, 1, {1,145,145}, 13, {38});
    replay(ship, ObjectiveId::GetShip, 13, {1,27,37}, 13, {38});
    std::printf("Cornelia storyline: %d assertions, %d physical legs, %d failures\n",
                exdeath_routes::assertions, exdeath_routes::legs, exdeath_routes::failures);
    return exdeath_routes::failures || fixture::wrong_owner ? 3 : 0;
}
}

int main()
{
    const auto read = [](const char* path) {
        std::ifstream file(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), {});
    };
    const auto maps = read("data/ff1/maps.bin");
    if (!baked::load(maps.data(), maps.size()) || !json::parse(read("data/ff1/mapstate.json"), fixture::records)) return 2;
    fixture::game_id = game::Id::FF1;
    baked_nav::g_loaded = baked_nav::g_prepared = true;
    int result = 2;
    std::thread worker([&] { result = verify(); });
    worker.join();
    return result;
}
