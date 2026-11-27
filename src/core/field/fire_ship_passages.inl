// The authored passage policy is shared with the explicit puzzle guide. The
// directions planner uses one operation at a time, against actual collision.
void prepare_fire_ship_prefix(RouteSnapshot& snapshot, PathInfo& path)
{
    if (path.success || !game::is(game::Id::FF5) || planning_cancelled() ||
        snapshot.goal.avoid_other_events || snapshot.goal.transportation_id > 0) return;
    const int map = snapshot.map_id;
    const auto& destination = snapshot.goal;
    const bool side_door = destination.map_id == 256 && destination.entity_id == 6 &&
        destination.cell == Cell{1,8,8} && snapshot.kind == TargetKind::Transition;
    const bool side_room = destination.map_id == 259 || side_door;
    const bool passage_destination = destination.map_id == 270 && destination.entity_id >= 15 && destination.entity_id <= 26;
    const bool behind_passage = map == 270 && fire_ship_room(destination.map_id) &&
        destination.map_id != 270 && destination.map_id != 271;
    const bool back_from_boss = map == 271 && destination.map_id != 271 && fire_ship_room(destination.map_id);
    if (!side_room && !passage_destination && !behind_passage && !back_from_boss && destination.map_id != 271) return;
    if ((map == 271 && !back_from_boss) || (destination.map_id == 259 && map == 259) || !fire_ship_room(map)) return;

    int stage_map = 269;
    int entity = 6;
    int direction = -1;
    if (back_from_boss) {
        stage_map = 271;
        entity = 2;
    } else if (side_room && map != 270) {
        stage_map = 256;
        entity = 13;
        if (map == stage_map && std::none_of(snapshot.obstacles.begin(), snapshot.obstacles.end(), [](const auto& actor) {
                return actor.id == 15 && actor.enabled && actor.cell == Cell{1,8,8};
            })) return;
    } else if (map == 270) {
        stage_map = map;
        const auto start = cell_of(*snapshot.submap, snapshot.position, snapshot.layer);
        if (start.layer != 1 || start.x < 0 || start.y < 0 || start.x >= 25 || start.y >= 27) return;
        int mask = 0;
        for (int bit = 0; bit < 7; ++bit) {
            const auto value = snapshot.state.scenario_value(301 + bit);
            if (!value || (*value != 0 && *value != 1)) return;
            mask |= *value << bit;
        }
        const auto* guide = data::game_json("puzzles").get(L"fire_ship_guide");
        const auto* rows = guide ? guide->get(L"rows") : nullptr;
        if (guide && (behind_passage || passage_destination)) {
            const auto* targets = guide->get(L"target_rows");
            const int target = behind_passage ? 15 : destination.entity_id;
            if (target != 16) rows = targets ? targets->get(std::to_wstring(target)) : nullptr;
        }
        if (!rows || rows->arr.size() != 128) return;
        const auto& row = rows->arr[mask].s;
        if (!row.empty() && row.size() != 675) return;
        constexpr std::wstring_view codes = L".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh";
        const auto code = row.empty() ? 0 : codes.find(row[start.y * 25 + start.x]);
        if (code == codes.npos) return;
        if (code == 0) {
            if (!side_room) return;
            // A walking route can reach the lower exit without a platform
            // action. Prove the full onward approach to the side-room switch.
            stage_map = 256;
            entity = 13;
        } else if (code <= 32) {
            constexpr int controls[]{21,19,20,23,24,26,25,22};
            entity = controls[(code - 1) / 4];
            direction = static_cast<int>((code - 1) % 4);
        } else {
            entity = code == 33 ? 16 : 15;
            if (passage_destination && destination.entity_id != 16 && entity == 16) return;
        }
    }

    const auto* stage = baked::submap_by_map_id(stage_map);
    if (!stage) return;
    const auto& objects = baked::grids(*stage).objects;
    const bool unlock_door = side_room && stage_map == 256;
    const auto actor = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
        return object.entity_id() == entity &&
            object.type == (direction >= 0 || unlock_door ? baked::ObjType::Entity : baked::ObjType::GotoMap);
    });
    if (actor == objects.end()) return;

    // Route to the verified interaction or entrance using normal live state.
    // Keep the selected target so the next request can plan from the new pose.
    const MapGoal original_goal = snapshot.goal;
    const auto original_kind = snapshot.kind;
    auto original_active = std::move(snapshot.active);
    auto original_pending = std::move(snapshot.pending);
    snapshot.active = {};
    snapshot.pending = {};
    snapshot.goal = {};
    snapshot.goal.map_id = stage_map;
    snapshot.goal.cell = {actor->layer, actor->tx, actor->ty};
    snapshot.goal.entity_id = entity;
    snapshot.goal.raw_name = baked::object_identifier(*actor);
    snapshot.goal.examine = unlock_door;
    snapshot.kind = unlock_door ? TargetKind::Interact : TargetKind::Transition;
    if (direction >= 0) {
        constexpr int dx[]{0,1,0,-1}, dy[]{-1,0,1,0};
        snapshot.goal.cell.x -= dx[direction];
        snapshot.goal.cell.y -= dy[direction];
        snapshot.goal.entity_id = -1;
        snapshot.goal.raw_name.clear();
        snapshot.kind = TargetKind::Spot;
    }
    if (map == stage_map) {
        snapshot.goal.avoid_other_events = true;
        snapshot.goal.allow_scripted_crossings = true;
    }
    PathInfo approach;
    route_to_goal(nullptr, nullptr, snapshot.goal, snapshot.kind, {}, approach);
    bool requires_boss = false;
    if (!approach.success && back_from_boss && !planning_cancelled() &&
        snapshot.state.scenario_value(45) == 0) {
        // From the far side of the boiler room, the only return corridor
        // crosses Liquid Flame's contact. Stop at the encounter and keep the
        // original destination for the next request after the native battle.
        const auto boss = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
            return object.type == baked::ObjType::Event && object.entity_id() == 1 &&
                object.layer == 1 && object.tx == 15 && object.ty == 24 &&
                snapshot.state.active(*stage, object) && !snapshot.state.consumed(*stage, object);
        });
        if (boss != objects.end()) {
            snapshot.active = {};
            snapshot.pending = {};
            snapshot.goal.cell = {boss->layer, boss->tx, boss->ty};
            snapshot.goal.entity_id = boss->entity_id();
            snapshot.goal.raw_name = baked::object_identifier(*boss);
            snapshot.kind = TargetKind::Infer;
            approach = {};
            route_to_goal(nullptr, nullptr, snapshot.goal, snapshot.kind, {}, approach);
            requires_boss = approach.success && approach.next_map_id == 0;
        }
    }
    snapshot.goal = original_goal;
    snapshot.kind = original_kind;
    if (!approach.success || approach.walk_phases.empty()) {
        snapshot.active = std::move(original_active);
        snapshot.pending = std::move(original_pending);
        return;
    }
    if (requires_boss) {
        auto phases = std::move(approach.walk_phases);
        phases.back().action_after = T(L"Defeat Liquid Flame");
        phases.back().manual_after = true;
        fill_result(*snapshot.submap, phases, approach);
        approach.event_on_route = true;
        snapshot.active.phases = std::move(phases);
    } else if (direction >= 0 && approach.next_map_id == 0 && !approach.npc_approach) {
        auto phases = std::move(approach.walk_phases);
        phases.back().action_after = TF(L"Face {0}. Operate the switch.", {T(direction_key(direction))});
        phases.back().manual_after = true;
        fill_result(*snapshot.submap, phases, approach);
        snapshot.active.phases = std::move(phases);
    } else if (unlock_door && map == stage_map && approach.next_map_id == 0 && !approach.npc_approach) {
        auto phases = std::move(approach.walk_phases);
        phases.back().action_after = T(L"Examine the switch");
        phases.back().manual_after = true;
        fill_result(*snapshot.submap, phases, approach);
        snapshot.active.phases = std::move(phases);
    } else if (direction < 0 && map == stage_map && approach.next_map_id == 0 && !approach.npc_approach) {
        approach.next_map_id = actor->a;
        snapshot.active.next_map_id = actor->a;
        const auto* next_map = baked::submap_by_map_id(actor->a);
        const auto* arrival = next_map ? point_in(*next_map, actor->b, snapshot.state) : nullptr;
        const auto& cells = approach.walk_phases.back().cells;
        if (arrival && !cells.empty())
            snapshot.pending.incoming_transition = TrapVisit{map, entity, actor->a, actor->tx, actor->ty,
                cells.back(), {arrival->layer, arrival->tx, arrival->ty}};
    }
    snapshot.active.target = original_goal.cell;
    snapshot.active.goal_map_id = original_goal.map_id;
    // Recheck the native switch layout even if the next request starts on the
    // same standing tile. A cached approach must never replay an old action.
    snapshot.active.key_approach = true;
    const auto transition = snapshot.pending.incoming_transition;
    remember_destination(original_goal, original_kind, snapshot.name);
    if (transition) snapshot.pending.incoming_transition = transition;
    path = std::move(approach);
}
