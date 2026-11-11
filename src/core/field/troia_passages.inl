// Exact Troia controls hide their own trigger and passage collision. Use the
// native action as a manual prefix when that collision alone blocks the goal.
// The speculative copy only establishes relevance. Published cells always use
// the actual snapshot, including every still-closed passage and live NPC.
void prepare_troia_passage_prefix(RouteSnapshot& snapshot, PathInfo& path)
{
    if (path.success || !game::is(game::Id::FF4) || snapshot.goal.avoid_other_events ||
        !snapshot.submap || snapshot.submap->world) return;
    struct Control { int map, id; std::array<int,3> hidden; bool examined; };
    static constexpr Control controls[] = {
        {91,26,{23,26,0},false}, {91,22,{23,26,0},true},
        {93,20,{22,20,0},false},
        {94,16,{10,11,16},false}, {94,10,{10,11,16},true},
        {95,20,{5,20,24},false}, {95,24,{5,20,24},false},
    };
    const auto& room = *snapshot.submap;
    const auto& grid = baked::grids(room);
    const auto live = live_blockers(room,grid,{});
    auto movement = walk_request(room,cell_of(room,snapshot.position,snapshot.layer));
    movement.blocked = live.hard();
    movement.directional = live.directional;
    live.apply_transport(movement);
    movement.live_hazards = movement.live_traps = true;
    movement.hazards = live.triggers;
    movement.traps = live.traps;
    std::optional<Phase> best;
    bool best_npc = false;
    for (const auto& control : controls) {
        if (control.map != snapshot.map_id || planning_cancelled()) continue;
        const auto seen = std::find_if(snapshot.obstacles.begin(),snapshot.obstacles.end(),
            [&](const auto& actor) { return actor.id == control.id && (control.examined || actor.trigger); });
        if (seen == snapshot.obstacles.end()) continue;
        const auto* source = baked_by_entity_id(grid,seen->id,static_cast<baked::ObjType>(seen->type),
                                                seen->cell,seen->raw_name);
        if (!source) continue;
        // A selected control that has already retired cannot be reached by
        // first activating another instance of that same one-shot action.
        if (snapshot.goal.map_id == snapshot.map_id &&
            std::find(control.hidden.begin(),control.hidden.end(),snapshot.goal.entity_id) != control.hidden.end()) continue;
        auto possible = snapshot;
        possible.active = {};
        possible.previous_active.reset();
        possible.pending = {};
        std::erase_if(possible.obstacles,[&](const auto& actor) {
            return std::find(control.hidden.begin(),control.hidden.end(),actor.id) != control.hidden.end();
        });
        PathInfo onward;
        {
            SnapshotScope scope(possible);
            route_to_goal(nullptr,nullptr,possible.goal,possible.kind,possible.name,onward);
        }
        if (!onward.success) continue;
        auto approach = movement;
        auto actor = *source;
        actor.tx = static_cast<int16_t>(seen->cell.x);
        actor.ty = static_cast<int16_t>(seen->cell.y);
        actor.layer = static_cast<uint8_t>(seen->cell.layer);
        const auto goals = baked_route::goals_for(approach,seen->cell,
            control.examined ? baked_route::GoalKind::Interact : baked_route::GoalKind::Spot,&actor);
        approach.goals = goals.cells;
        approach.goals_open = false;
        auto route = baked_route::find(approach);
        Phase phase;
        bool npc = false;
        if (!route.found) {
            auto without_npcs = approach;
            without_npcs.blocked = live.hard(true,true,false);
            const auto proof = baked_route::find(without_npcs);
            const auto stop = npc_approach(approach,live,proof.cells);
            if (!stop) continue;
            phase = *stop;
            npc = true;
        } else phase.cells = std::move(route.cells);
        if (!npc && control.examined) {
            annotate_facing(phase,goals);
            phase.action_after = TF(L"Interact: {0}",{
                translate_entity_name_snapshot(baked::object_identifier(actor),snapshot.map_id)});
        } else if (!npc) phase.action_after = TF(L"Step on {0}.",{T(L"Passage switch")});
        phase.manual_after = true;
        if (!best || route_progress::steps(phase) < route_progress::steps(*best)) {
            best = std::move(phase);
            best_npc = npc;
        }
    }
    if (!best) return;
    path = {};
    fill_result(room,{*best},path);
    path.reason = best_npc ? best->action_after : T(L"Event required");
    path.event_on_route = !best_npc;
    path.npc_approach = best_npc;
    set_active(snapshot.map_id,snapshot.goal.cell,snapshot.goal.map_id,{*best},0);
    snapshot.active.key_approach = true;
    remember_destination(snapshot.goal,snapshot.kind,snapshot.name);
    snapshot.pending.incoming_transition.reset();
    snapshot.pending.incoming_trap.reset();
}
