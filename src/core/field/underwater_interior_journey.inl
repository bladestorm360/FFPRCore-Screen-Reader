// Final indoor destinations must retain the same native world journey as
// their underwater entrance marker. Prove the indoor continuation separately,
// then publish only the real first world transition, retaining the final goal.
// Four underwater arcs and the two reviewed Doga doors bound this adapter;
// ordinary indoor graph limits,
// cancellation, story gates and transition history remain authoritative.
NautilusJourney underwater_interior_journey(const MapGoal& goal,TargetKind kind) {
    NautilusJourney best;
    if(!game::is(game::Id::FF3)||!g_route_snapshot||!g_route_snapshot->vehicle_shortcuts||
       goal.avoid_other_events||planning_cancelled())return best;
    const auto* water=baked::submap_by_map_id(3);
    const auto* surface=baked::submap_by_map_id(4);
    const auto* target=baked::submap_by_map_id(goal.map_id);
    if(!water||!surface||!target||target->world)return best;
    MapGoal indoor=goal;
    if(indoor.cells.empty())indoor.cells=goals_for(*target,indoor.cell,kind).cells;
    if(indoor.cells.empty())return best;
    std::vector<std::pair<const baked::Submap*,const baked::Object*>> entries;
    for(const auto& door:baked::grids(*water).objects)
        if(baked::world_exit(door)&&baked::submarine_entry(door))entries.push_back({water,&door});
    for(const auto& door:baked::grids(*surface).objects)
        if(door.type==baked::ObjType::GotoMap&&((door.tx==128&&door.ty==184)||(door.tx==150&&door.ty==203)))
            entries.push_back({surface,&door});
    if(entries.size()>6)return {}; // a changed bake needs explicit qualification
    for(const auto& entry:entries) {
        if(planning_cancelled())return {};
        const auto& door=*entry.second;const auto& world=*entry.first;
        // Current underwater interiors already have an ordinary riding leg.
        // Current Surface Manor likewise needs no manual world detour. Village
        // is the reviewed same-world case which really requires the tunnel.
        if(planning_map_id()==world.first_map_id&&
           (world.first_map_id==3||Cell{1,door.tx,door.ty}!=nautilus_surface::village_cell))continue;
        const int carrier=world.first_map_id==3?14:9;
        auto& state=g_route_snapshot->state;
        if(!state.active(world,door)||state.consumed(world,door)||
           !(state.transportation_mask(world,door)&baked_route::transportation_bit(carrier)))continue;
        const auto* room=baked::submap_by_map_id(door.a);
        if(!room||room->world)continue;
        if(room!=target) {
            const auto floors=connected_floors(room,door.a,state,&planning_cancelled);
            if(std::none_of(floors.begin(),floors.end(),[&](const Floor& floor){return floor.submap==target;}))continue;
        }
        const auto* arrival=point_in(*room,door.b,state);if(!arrival)continue;
        if(world.first_map_id==planning_map_id()) {
            // The tunnel is needed only until the craft reaches Village's
            // actual landing pocket. After surfacing, prefer the ordinary
            // legal entry; a fresh forced dive would loop the final leg.
            const auto& grid=baked::grids(world);
            auto request=walk_request(world,cell_of(world,snapshot_position(),1));world_mode(request);
            const auto live=live_blockers(world,grid,{});
            request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
            request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;
            request.goals=goals_for(world,{1,door.tx,door.ty},TargetKind::Transition).cells;
            request.allow_trap_goal=true;
            std::vector<Phase> direct;
            if((!request.vehicle_group&&baked_route::find(request).found)||
               boarding_route(world,grid,request,request.goals,direct))continue;
        }
        CrossLeg after;
        {
            // The ordinary graph carries the final Nautilus identity into
            // this native PointIn too. Preserve that selector contract;
            // do not invent a Player reset or an unloaded actor observation.
            FutureWorldScope future(door.a,{arrival->layer,arrival->tx,arrival->ty},carrier);
            after=find_across(nullptr,nullptr,indoor,true);
            if(!after.found&&!after.incomplete)after=find_across(nullptr,nullptr,indoor,true,true);
        }
        if(!after.found)continue;
        MapGoal entrance;entrance.map_id=world.first_map_id;entrance.cell={1,door.tx,door.ty};
        auto journey=world_map_journey(entrance);
        if(!journey.found)continue;
        journey.total+=1+after.steps_total;
        if(!best.found||journey.total<best.total)best=std::move(journey);
    }
    return best;
}
