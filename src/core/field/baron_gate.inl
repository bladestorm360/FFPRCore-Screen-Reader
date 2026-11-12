// The courtyard's first drawbridge opens when the player enters either of
// its paired native contacts. The route ends at that action and resumes from
// freshly observed collision after the script. Floor changes keep the exit
// as the destination while the normal cross-floor planner leads to the gate.
void prepare_baron_gate_prefix(RouteSnapshot& snapshot, PathInfo& path)
{
    constexpr int courtyard = 4, west_hall = 9;
    constexpr int opened_flag = 280;
    if (path.success || !game::is(game::Id::FF4) || path.reason != T(L"Story gated") ||
        snapshot.kind != TargetKind::Transition || snapshot.goal.map_id != courtyard ||
        (snapshot.map_id != courtyard && snapshot.map_id != west_hall) ||
        snapshot.goal.entity_id < 1 || snapshot.goal.entity_id > 4 ||
        snapshot.goal.cell != Cell{0,17 + snapshot.goal.entity_id,34} ||
        snapshot.goal.avoid_other_events || !remote_foot_context(*snapshot.submap) ||
        snapshot.state.scenario_value(opened_flag) != 0) return;

    const auto* exterior = baked::submap_by_map_id(courtyard);
    if (!exterior) return;
    struct Contact { int id, script; Cell cell; };
    constexpr Contact contacts[] = {
        {142,1033,{0,19,31}}, {49,1001,{0,19,28}},
        {143,1033,{0,20,31}}, {50,1001,{0,20,28}},
    };
    std::optional<PathInfo> prefix;
    ActiveRoute active;
    Pending pending;
    const auto& objects = baked::grids(*exterior).objects;
    for (const auto& contact : contacts) {
        if (planning_cancelled()) return;
        const auto source = std::find_if(objects.begin(),objects.end(),[&](const auto& object) {
            return object.type == baked::ObjType::Event && object.entity_id() == contact.id &&
                object.a == contact.script && object.b == 2 && object.layer == contact.cell.layer &&
                object.tx == contact.cell.x && object.ty == contact.cell.y && object.tw == 1 &&
                object.th == 1 && baked::str(object.group) == L"entity_default";
        });
        if (source == objects.end() || !snapshot.state.active(*exterior,*source)) continue;
        const auto raw = baked::object_identifier(*source);
        if (snapshot.map_id == courtyard &&
            std::none_of(snapshot.obstacles.begin(),snapshot.obstacles.end(),[&](const auto& observed) {
                return observed.trigger && observed.enabled && observed.applies &&
                    observed.id == contact.id && observed.cell == contact.cell && observed.raw_name == raw;
            })) continue;

        auto approach = snapshot;
        approach.active = {};
        approach.previous_active.reset();
        approach.pending = {};
        approach.goal = {courtyard,contact.cell,false,{},0,contact.id,raw};
        approach.kind = TargetKind::Spot;
        PathInfo route;
        {
            SnapshotScope scope(approach);
            route_to_goal(nullptr,nullptr,approach.goal,approach.kind,approach.name,route);
        }
        if (!route.success || route.required_trap || route.walk_phases.empty()) continue;
        if (route.next_map_id > 0) {
            if (prefix || !approach.pending.incoming_transition) continue;
        } else {
            auto& phase = route.walk_phases.back();
            if (phase.cells.empty() || phase.cells.back() != contact.cell) continue;
            phase.manual_after = true;
            phase.action_after = TF(L"Step on {0}.",{T(L"Gate")});
            PathInfo local;
            fill_result(*snapshot.submap,route.walk_phases,local);
            local.reason = T(L"Event required");
            local.event_on_route = true;
            route = std::move(local);
            approach.active.phases = route.walk_phases;
        }
        prefix = std::move(route);
        active = std::move(approach.active);
        pending = std::move(approach.pending);
        if (!prefix->next_map_id) break;
    }
    if (!prefix) return;

    path = std::move(*prefix);
    active.target = snapshot.goal.cell;
    active.goal_map_id = snapshot.goal.map_id;
    active.key_approach = true;
    snapshot.active = std::move(active);
    remember_destination(snapshot.goal,snapshot.kind,snapshot.name);
    snapshot.pending.incoming_transition = std::move(pending.incoming_transition);
}
