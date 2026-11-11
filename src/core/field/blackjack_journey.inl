// Exact native BJoutboard fly branch. The ordinary cross-floor graph must
// never use World PointIn1 (Narshe) for OnTheVehicle4. Prove the continuation
// from an owned saved pose, then publish only the reachable manual control.
// One source, one future context and one ordinary control route; no native
// access, vehicle creation, input, timer or unbounded recursive expansion.
bool try_blackjack_journey(const MapGoal& goal,TargetKind kind,const std::wstring& name,PathInfo& out) {
    if(!game::is(game::Id::FF6)||!g_route_snapshot||g_route_snapshot->map_id!=119||
       goal.map_id==119||!g_route_snapshot->vehicle_shortcuts||goal.avoid_other_events)return false;
    auto& snapshot=*g_route_snapshot;
    const auto* there=baked::submap_by_map_id(goal.map_id);
    if(!there||(!there->world&&!goal.allow_world_transit))return false;
    out={};out.error=T(L"No path found");
    if(planning_cancelled())return true;
    const baked::Object* flight=nullptr;
    for(const auto& object:baked::grids(*snapshot.submap).objects)
        if(object.type==baked::ObjType::Warp&&(object.d&baked::kBlackjackSavedFlight)&&
           snapshot.state.active(*snapshot.submap,object)&&!snapshot.state.consumed(*snapshot.submap,object))flight=&object;
    if(!flight)return true;
    const auto owned=snapshot.state.treasure_value(903),continent=snapshot.state.scenario_value(284);
    if(!owned)return true;
    const bool guest=*owned==0;
    Cell arrival{1,22,93};
    if(!guest) {
        if(!continent)return true;
        const auto saved=std::find_if(snapshot.saved_vehicles.begin(),snapshot.saved_vehicles.end(),
            [](const auto& value){return value.id==4&&value.map==1;});
        if(saved==snapshot.saved_vehicles.end())return true;
        arrival={1,static_cast<int>(saved->cell.x),static_cast<int>(saved->cell.y)};
        if(!baked::in_bounds(baked::grids(*baked::submap_by_map_id(1)),arrival.x,arrival.y))return true;
    }
    PathInfo after;
    {
        FutureWorldScope future(1,arrival,4);
        SnapshotScope scope(future.snapshot);
        if(!route_to_goal(nullptr,nullptr,goal,kind,name,after)||!after.success) {
            out.reason=after.reason;return true;
        }
    }
    if(planning_cancelled())return true;
    MapGoal control;control.map_id=119;control.entity_id=flight->entity_id();
    control.cell={flight->layer,flight->tx,flight->ty};control.raw_name=baked::object_identifier(*flight);
    const auto original=snapshot.goal;snapshot.goal=control;
    const bool planned=route_to_goal(nullptr,nullptr,control,TargetKind::Interact,name,out);
    snapshot.goal=original;
    if(!planned||!out.success||snapshot.active.phases.empty())return true;
    auto phases=snapshot.active.phases;
    auto& last=phases.back();last.manual_after=true;
    last.action_after=T(L"Interact to fly the airship.");
    if(!guest)last.action_after+=L" "+T(*continent?L"Choose the second option.":L"Choose the first option.");
    set_active(119,goal.cell,goal.map_id,phases,1);fill_result(*snapshot.submap,phases,out);out.next_map_id=1;
    remember_destination(goal,kind,name);
    if(g_pending) {
        g_pending->incoming_trap.reset();
        g_pending->incoming_transition=TrapVisit{119,flight->entity_id(),1,flight->tx,flight->ty,
            last.cells.back(),arrival};
    }
    return true;
}
