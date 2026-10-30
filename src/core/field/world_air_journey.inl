// Actual FFIII continent scripts move the carried craft to a fixed native
// cell, unlike the position-preserving Nautilus water conversion. These first
// legs reuse the reviewed event families and prove the remaining journey in an
// owned future context. No off-map Unity object/parked position is invented.
NautilusJourney world_map_journey(const MapGoal& goal,int depth=0);
struct FutureWorldScope {
    RouteSnapshot snapshot;
    RouteSnapshot* previous;
    FutureWorldScope(int map,Cell at,int id):snapshot(*g_route_snapshot),previous(g_route_snapshot) {
        snapshot.map_id=map;snapshot.submap=baked::submap_by_map_id(map);snapshot.layer=1;
        snapshot.position=world_of_cell(*snapshot.submap,at);
        snapshot.vehicle={true,id,baked::transport(id)->type,{}};snapshot.vehicle.map_id=map;
        snapshot.vehicles.clear();snapshot.obstacles.clear();snapshot.previous_active.reset();snapshot.active={};snapshot.pending={};
        snapshot.live_transitions.reset();snapshot.transport_rules.reset();snapshot.transport_rules_prepared=false;
        snapshot.projected_origin=true;g_route_snapshot=&snapshot;
        // Off-map craft come from the native departure save, never a transform
        // from the wrong map. The carried craft supersedes its saved old form.
        for(const auto& saved:snapshot.saved_vehicles) {
            if(saved.map!=map||journey_vehicle_family(saved.id)==journey_vehicle_family(id))continue;
            const auto* row=baked::transport(saved.id);if(!row||row->type<=1||row->type==4||row->type==kTransportContent)continue;
            const Cell cell{1,static_cast<int>(saved.cell.x),static_cast<int>(saved.cell.y)};
            if(!baked::in_bounds(baked::grids(*snapshot.submap),cell.x,cell.y))continue;
            VehicleOnMap model;model.transportation_id=row->id;model.type=row->type;model.pos=world_of_cell(*snapshot.submap,cell);
            snapshot.vehicles.push_back(std::move(model));
        }
    }
    ~FutureWorldScope(){g_route_snapshot=previous;}
};
int future_world_distance(const MapGoal& goal,int depth) {
    if(planning_cancelled()||depth>3)return -1;
    if(goal.map_id!=planning_map_id()) {
        const auto route=world_map_journey(goal,depth);return route.found?route.total:-1;
    }
    const auto* map=g_route_snapshot->submap;const auto& grid=baked::grids(*map);
    auto request=walk_request(*map,cell_of(*map,snapshot_position(),1));world_mode(request);
    map_state::add_obstacles(g_route_snapshot->state,*map,request,true,false,false);
    request.goals=goals_for(*map,goal.cell,TargetKind::Transition).cells;
    if(!request.goals.empty()) {
        request.allow_trap_goal=true;
        std::vector<Phase> phases;
        if(vehicle_journey_route(*map,grid,request,request.goals,phases))return phase_steps(phases);
    }
    const auto via=world_map_journey(goal,depth);return via.found?via.total:-1;
}

NautilusJourney world_map_journey(const MapGoal& goal,int depth) {
    NautilusJourney best;
    if(depth>3||!g_route_snapshot||!g_route_snapshot->vehicle_shortcuts||!game::is(game::Id::FF3))return best;
    const auto* here=g_route_snapshot->submap;const auto* there=baked::submap_by_map_id(goal.map_id);
    if(!here||!here->world||!there||!there->world||planning_cancelled())return best;
    if((planning_map_id()==3||planning_map_id()==4)&&(goal.map_id==3||goal.map_id==4))
        return nautilus_world_journey(goal);
    const int map_id=planning_map_id();
    if(map_id==3&&goal.map_id==1) {
        // First prove surfacing with an airborne endpoint at the real return
        // trigger, then prove that carried Nautilus can reach the island goal.
        const auto* surface=baked::submap_by_map_id(4);if(!surface)return best;
        for(const auto& object:baked::grids(*surface).objects) {
            if(planning_cancelled())return {};
            const auto* rule=air_transitions::match(3,4,object);
            if(!rule||!air_transitions::accepts(*rule,9)||!g_route_snapshot->state.active(*surface,object))continue;
            MapGoal border;border.map_id=4;border.cell={1,object.tx,object.ty};
            auto water=nautilus_world_journey(border,true);if(!water.found)continue;
            int after=-1;
            {FutureWorldScope future(4,water.arrival,9);after=future_world_distance(goal,depth+1);}
            if(after<0)continue;
            water.total=phase_steps(water.phases)+after;return water;
        }
        return best;
    }
    if(map_id!=1&&map_id!=2&&map_id!=4)return best;
    if(goal.map_id==map_id)return best;
    const auto& grid=baked::grids(*here);const Cell start=cell_of(*here,snapshot_position(),1);
    auto request=walk_request(*here,start);world_mode(request);
    if(g_route_snapshot->projected_origin)map_state::add_obstacles(g_route_snapshot->state,*here,request,true,false,false);
    else {
        const auto live=live_blockers(*here,grid,{});request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
        request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;
    }
    request.goals_open=false;request.terminal_goal=true;request.allow_trap_goal=true;
    std::unordered_set<int> seen;
    for(const auto& object:grid.objects) {
        if(planning_cancelled())return {};
        const auto* rule=air_transitions::match(3,map_id,object);
        if(!rule||!seen.insert(rule->script).second)continue;
        int destination=rule->destination;
        if(!destination) {
            const auto restored=g_route_snapshot->state.scenario_value(66);if(!restored)continue;
            destination=*restored?4:2;
        }
        // No world detour can reverse to its caller. Three world identities
        // suffice for a continent/surface/underwater journey.
        if(map_id==4&&goal.map_id!=1)continue;
        if(map_id==2&&goal.map_id==2)continue;
        std::vector<Cell> targets;
        for(const auto& trigger:grid.objects)if(air_transitions::match(3,map_id,trigger)==rule&&
            g_route_snapshot->state.active(*here,trigger)&&!g_route_snapshot->state.consumed(*here,trigger)&&
            !(g_pending&&g_pending->valid&&g_pending->goal.map_id==g_route_snapshot->goal.map_id&&
              g_pending->goal.cell==g_route_snapshot->goal.cell&&
              std::any_of(g_pending->completed_transitions.begin(),g_pending->completed_transitions.end(),[&](const auto& used) {
                  return used.map_id==map_id&&used.destination==destination&&used.entity_id==trigger.entity_id()&&used.x==trigger.tx&&used.y==trigger.ty;
              })))
            targets.push_back({1,trigger.tx,trigger.ty});
        if(targets.empty())continue;
        std::vector<Phase> phases;
        if(!vehicle_journey_route(*here,grid,request,targets,phases,rule)||phases.empty()||phases.back().cells.empty())continue;
        const int boarded=phases.back().movement_transportation_id;
        int carried=0;
        if(destination==1)carried=boarded==10?11:(boarded==8||boarded==9)?8:6;
        else if(boarded==11)carried=10;
        else if(boarded==8||boarded==9) {
            const auto upgraded=g_route_snapshot->state.scenario_value(78);if(!upgraded)continue;
            carried=*upgraded?9:8;
        } else carried=destination==2?15:6;
        const Cell arrival=destination==1?Cell{1,2,97}:Cell{1,32,216};
        int after=-1;
        {FutureWorldScope future(destination,arrival,carried);after=future_world_distance(goal,depth+1);}
        if(after<0)continue;
        const int total=phase_steps(phases)+after;
        if(best.found&&total>=best.total)continue;
        auto& last=phases.back();last.action_after=TF(L"take the exit to {0}",{map_name_of(destination)});
        best.found=true;best.next_map=destination;best.total=total;best.crossing=last.cells.back();best.arrival=arrival;
        for(const auto& trigger:grid.objects)if(air_transitions::match(3,map_id,trigger)==rule&&trigger.contains(best.crossing.x,best.crossing.y)) {
            best.entity=trigger.entity_id();break;
        }
        best.phases=std::move(phases);
    }
    return best;
}
