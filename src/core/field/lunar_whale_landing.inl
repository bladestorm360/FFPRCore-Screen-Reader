// Included by baked_nav.cpp. Landing the ridden Whale opens its native cabin.
// The route ends at the landing command, which the player performs manually.
void plan_whale_landing(const RouteSnapshot& snapshot, const Cell& start, PathInfo& out)
{
    const auto& map = *snapshot.submap;
    const auto& grid = baked::grids(map);
    auto foot = walk_request(map, start);
    const auto live = live_blockers(map, grid, {});
    foot.blocked = live.hard(true, true, true);
    foot.live_hazards = foot.live_traps = true;
    foot.hazards = live.triggers;
    foot.traps = live.traps;
    auto flight = foot;
    world_mode(flight);
    const auto can_land = [&](const Cell& cell) {
        return baked::group_lands(flight.landing_group, baked_route::terrain_at(flight, grid, cell.x, cell.y)) &&
               baked_route::safe_landing(foot, grid, cell);
    };

    auto landing = start;
    if (!can_land(start)) {
        const auto reach = baked_route::flood(flight);
        int best = INT_MAX;
        for (int y = 0; y < grid.height && !planning_cancelled(); ++y) {
            for (int x = 0; x < grid.width; ++x) {
                const Cell cell{start.layer, x, y};
                const int distance = reach.distance(cell);
                if (distance < 0 || distance >= best || !can_land(cell)) continue;
                best = distance;
                landing = cell;
            }
        }
        if (best == INT_MAX || planning_cancelled()) { out.error = T(L"No path found"); return; }
    }
    flight.goals = {landing};
    auto route = baked_route::find(flight);
    if (!route.found) { out.error = T(L"No path found"); return; }
    Phase phase;
    phase.cells = std::move(route.cells);
    phase.transportation_id = phase.movement_transportation_id = snapshot.vehicle.id;
    phase.action_after = T(L"land");
    phase.manual_after = true;
    set_active(snapshot.map_id, start, snapshot.map_id, {phase}, 0);
    fill_result(map, {phase}, out);
}
