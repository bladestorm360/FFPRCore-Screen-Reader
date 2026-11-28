// The Cave 2F switch removes the native door collider for this visit.
// Route to the switch while preserving the player's selected destination.
void prepare_pirate_cave_prefix(RouteSnapshot& snapshot, PathInfo& path)
{
    constexpr int entrance = 191, switch_floor = 193;
    constexpr int switch_id = 11, collider_id = 29, entrance_id = 62;
    constexpr Cell switch_cell{1,22,6}, door_cell{1,19,5};
    constexpr Cell south_portal{1,165,115};
    if (path.success || !game::is(game::Id::FF5) || snapshot.goal.avoid_other_events ||
        snapshot.state.scenario_value(8) != 1 || planning_cancelled()) return;
    const auto& goal = snapshot.goal;
    const bool north_entrance = goal.map_id == 1 && goal.entity_id == 7 && goal.cell == Cell{1,166,106};
    const bool upper_stairs = goal.map_id == switch_floor && goal.entity_id == 5 && goal.cell == door_cell;
    if (snapshot.kind != TargetKind::Transition || (!north_entrance && !upper_stairs) ||
        (snapshot.map_id != 1 && (snapshot.map_id < entrance || snapshot.map_id > switch_floor))) return;

    const auto* room = baked::submap_by_map_id(switch_floor);
    if (!room) return;
    const auto& grid = baked::grids(*room);
    const auto* control = baked_by_entity_id(grid,switch_id,baked::ObjType::Entity,switch_cell,L"sc_e_0206:扉が開く");
    const auto* collider = baked_by_entity_id(grid,collider_id,baked::ObjType::Entity,door_cell,L"扉のコリジョン");
    if (!control || !collider || control->layer != switch_cell.layer || control->tx != switch_cell.x ||
        control->ty != switch_cell.y || collider->layer != door_cell.layer || collider->tx != door_cell.x ||
        collider->ty != door_cell.y || !snapshot.state.active(*room,*control)) return;
    if (snapshot.map_id == switch_floor && std::none_of(snapshot.obstacles.begin(),snapshot.obstacles.end(),
        [&](const auto& actor) { return actor.id == collider_id && actor.cell == door_cell && actor.enabled && actor.applies; })) return;

    const auto original_goal = goal;
    const auto original_kind = snapshot.kind;
    auto original_active = std::exchange(snapshot.active,{});
    auto original_pending = std::exchange(snapshot.pending,{});
    const auto restore = [&] {
        snapshot.active = std::move(original_active);
        snapshot.pending = std::move(original_pending);
    };
    snapshot.goal = {switch_floor,switch_cell,true,{},0,switch_id,baked::object_identifier(*control)};
    snapshot.goal.object_type = static_cast<int>(control->type);
    snapshot.kind = TargetKind::Interact;
    if (snapshot.map_id == 1) {
        const auto& world = baked::grids(*snapshot.submap);
        const auto* portal = baked_by_entity_id(world,entrance_id,baked::ObjType::Warp,south_portal);
        if (!portal || !baked::world_exit(*portal) || portal->a != entrance || portal->b != 1 ||
            portal->tx != south_portal.x || portal->ty != south_portal.y || !snapshot.state.active(*snapshot.submap,*portal)) {
            snapshot.goal = original_goal;
            snapshot.kind = original_kind;
            restore();
            return;
        }
        // Plan the actual landing and entry from the current vehicle, then
        // resume the held destination after the native Cave Entrance arrival.
        snapshot.goal = {1,south_portal,false,{},0,entrance_id,baked::object_identifier(*portal)};
        snapshot.kind = TargetKind::Transition;
    }
    PathInfo approach;
    route_to_goal(nullptr,nullptr,snapshot.goal,snapshot.kind,{},approach);
    snapshot.goal = original_goal;
    snapshot.kind = original_kind;
    if (!approach.success || approach.required_trap || approach.walk_phases.empty()) {
        restore();
        return;
    }
    if (snapshot.map_id == 1 && approach.next_map_id == 0) {
        const auto* cave = baked::submap_by_map_id(entrance);
        const auto* arrival = cave ? point_in(*cave,1,snapshot.state) : nullptr;
        auto phases = std::move(approach.walk_phases);
        if (!arrival || phases.back().cells.empty() || phases.back().cells.back() != south_portal) {
            restore();
            return;
        }
        phases.back().action_after = TF(L"take the exit to {0}",{map_name_of(entrance)});
        fill_result(*snapshot.submap,phases,approach);
        approach.next_map_id = entrance;
        snapshot.active.next_map_id = entrance;
        snapshot.active.phases = std::move(phases);
        snapshot.pending.incoming_transition = TrapVisit{1,entrance_id,entrance,south_portal.x,south_portal.y,
            south_portal,{arrival->layer,arrival->tx,arrival->ty}};
    } else if (snapshot.map_id == switch_floor && !approach.npc_approach) {
        auto phases = std::move(approach.walk_phases);
        auto& end = phases.back();
        end.manual_after = true;
        end.action_after = T(L"Examine the switch");
        end.arrival_interaction = route_progress::InteractionTarget{switch_cell,switch_id,static_cast<int>(control->type),true};
        fill_result(*snapshot.submap,phases,approach);
        snapshot.active.phases = std::move(phases);
    } else if (snapshot.map_id != switch_floor && !snapshot.pending.incoming_transition) {
        restore();
        return;
    }
    snapshot.active.target = original_goal.cell;
    snapshot.active.goal_map_id = original_goal.map_id;
    snapshot.active.key_approach = true;
    const auto transition = snapshot.pending.incoming_transition;
    remember_destination(original_goal,original_kind,snapshot.name);
    snapshot.pending.incoming_transition = transition;
    path = std::move(approach);
}
