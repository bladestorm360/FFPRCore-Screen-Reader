// FFIII's keys/Thief, FFIV's important items and FFVI's Locke/Rust Rid establish that a journey is possible, not that its still
// closed native collision may be crossed. This worker-only adapter stops the
// current-floor leg at the first actual lock. Every affordable lock is solid
// during the small approach search; no key is spent and no script is invoked.
void prepare_manual_key_prefix(RouteSnapshot& snapshot,PathInfo& path) {
    if(!path.success||!snapshot.submap||snapshot.submap->world||
       (baked::game_number()!=3&&baked::game_number()!=4&&baked::game_number()!=6))return;
    const auto& sm=*snapshot.submap;const auto& grid=baked::grids(sm);
    struct Door {const baked::Object* source;Cell at;};std::vector<Door> doors;
    for(const auto& raw:snapshot.obstacles) {
        if(raw.type!=kObjEntity||!raw.enabled||!raw.applies)continue;
        const auto* object=baked_by_entity_id(grid,raw.id,baked::ObjType::Entity,raw.cell,raw.raw_name);
        if(object&&snapshot.state.can_unlock(sm,*object))doors.push_back({object,raw.cell});
    }
    if(doors.empty())return;
    const Door* first=nullptr;
    for(const auto& phase:path.walk_phases) {
        for(size_t i=1;i<phase.cells.size()&&!first;++i)for(const auto& door:doors)
            if(phase.cells[i]==door.at){first=&door;break;}
        // An earlier native action still owns the immediate instruction.
        if(first||phase.manual_after)break;
    }
    if(!first)return;
    const auto live=live_blockers(sm,grid,{});
    auto movement=walk_request(sm,cell_of(sm,snapshot.position,snapshot.layer));
    movement.blocked=live.hard();movement.directional=live.directional;live.apply_transport(movement);
    for(const auto& door:doors)movement.blocked.push_back(door.at);
    movement.live_hazards=movement.live_traps=true;movement.hazards=live.triggers;movement.traps=live.traps;
    auto actor=*first->source;actor.tx=static_cast<int16_t>(first->at.x);actor.ty=static_cast<int16_t>(first->at.y);
    actor.layer=static_cast<uint8_t>(first->at.layer);
    const auto goals=baked_route::goals_for(movement,first->at,baked_route::GoalKind::Interact,&actor);
    movement.goals=goals.cells;movement.goals_open=false;
    auto route=baked_route::find(movement);
    if(!route.found&&!snapshot.goal.avoid_other_events) {movement.ignore_hazards=true;route=baked_route::find(movement);}
    if(!route.found) {
        // Never publish the virtual through-door path if the physical approach
        // changed or has no legal interaction side in the captured world.
        path=PathInfo{};path.reason=path.error=T(L"No safe route found");snapshot.active={};return;
    }
    Phase prefix;prefix.cells=std::move(route.cells);annotate_facing(prefix,goals);
    prefix.action_after=baked::game_number()!=3?T(L"Interact to unlock the door."):
        TF(L"Interact: {0}",{translate_entity_name_snapshot(baked::object_identifier(actor),snapshot.map_id)});
    prefix.manual_after=true;
    path=PathInfo{};fill_result(sm,{prefix},path);path.reason=prefix.action_after;
    path.event_on_route=event_on_route(prefix.cells,live.triggers,{},&movement);
    set_active(snapshot.map_id,snapshot.goal.cell,snapshot.goal.map_id,{prefix},0);
    snapshot.active.key_approach=true;
    remember_destination(snapshot.goal,snapshot.kind,snapshot.name);
    snapshot.pending.incoming_transition.reset();snapshot.pending.incoming_trap.reset();
}
