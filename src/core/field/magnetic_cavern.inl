// The Black Chocobo leaves from its Chocobo Village NPC interaction and returns there
// when dismounted. It is not a parked aircraft that a surface snapshot can
// offer to the normal vehicle planner. Keep the cavern selected across the
// forest entrance and the mount's scripted return to the world.
void prepare_magnetic_cavern_prefix(RouteSnapshot& snapshot, PathInfo& path)
{
    constexpr int forest_id = 160, black_chocobo_transport = 7;
    if (path.success || snapshot.goal.avoid_other_events ||
        !magnetic_cavern_target(snapshot.goal, snapshot.kind) ||
        (snapshot.vehicle.valid && snapshot.vehicle.id == black_chocobo_transport)) return;

    const bool entering = snapshot.map_id == 1;
    const auto& map = *snapshot.submap;
    const auto& objects = baked::grids(map).objects;
    PathInfo best;
    std::optional<TrapVisit> arrival;
    const auto talk = TF(L"Talk to {0}.", {T(L"Black Chocobo")});
    for (const auto& object : objects) {
        if (planning_cancelled()) return;
        const int id = object.entity_id();
        if (entering ? (object.type != baked::ObjType::GotoMap || id != 82 || object.a != forest_id)
                     : (object.type != baked::ObjType::NPC || (id != 12 && id != 14))) continue;
        if (!snapshot.state.active(map, object)) continue;

        auto approach = snapshot;
        approach.active = {};
        approach.previous_active.reset();
        approach.pending = {};
        const Cell target{object.layer, object.tx, object.ty};
        const auto raw = baked::object_identifier(object);
        approach.goal = {snapshot.map_id, target, !entering, {}, 0, entering ? -1 : id, raw};
        approach.kind = entering ? TargetKind::Transition : TargetKind::Interact;
        PathInfo route;
        {
            SnapshotScope scope(approach);
            if (!entering) {
                const auto* actor = selected_live_npc(approach);
                if (!actor) continue;
                approach.goal.cell = actor->cell;
            }
            route_to_goal(nullptr, nullptr, approach.goal, approach.kind, approach.name, route);
        }
        if (!route.success || route.next_map_id || route.required_trap || route.npc_approach || route.walk_phases.empty() ||
            (best.success && best.step_count <= route.step_count)) continue;

        auto& phase = route.walk_phases.back();
        if (phase.cells.empty()) continue;
        if (entering) {
            const auto end = phase.cells.back();
            if (!object.contains(end.x, end.y)) continue;
            const auto* forest = baked::submap_by_map_id(forest_id);
            const auto* point = forest ? point_in(*forest, object.b, snapshot.state) : nullptr;
            if (!point) continue;
            arrival = TrapVisit{snapshot.map_id, id, forest_id, object.tx, object.ty, end,
                {point->layer, point->tx, point->ty}};
        }
        if (phase.action_after.empty()) phase.action_after = talk;
        phase.manual_after = true;
        best = std::move(route);
    }
    if (!best.success) return;

    path = {};
    fill_result(map, best.walk_phases, path);
    if (best.walk_phases.back().action_after == talk) {
        auto brief_phases = best.walk_phases;
        brief_phases.back().action_after.clear();
        path.description = path.step_count ? describe_phases(brief_phases, false, &map) + L", " + talk : talk;
    }
    path.event_on_route = best.event_on_route;
    path.verify_interaction_on_arrival = !entering;
    path.next_map_id = entering ? forest_id : 1;
    set_active(snapshot.map_id, snapshot.goal.cell, snapshot.goal.map_id, best.walk_phases, path.next_map_id);
    snapshot.active.key_approach = true;
    remember_destination(snapshot.goal, snapshot.kind, snapshot.name);
    snapshot.pending.incoming_trap.reset();
    snapshot.pending.incoming_transition = std::move(arrival);
}
