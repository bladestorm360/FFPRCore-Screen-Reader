// Owned FFIV Surface World route. The active Enterprise model and the native
// cargo receipt are the pickup authority. Cargo actions are player-operated.
struct HovercraftRoute {
    struct Release {
        Cell cargo, park, approach;
        int cost=0;
        bool lower=false;
    };
    baked_route::Request foot, flight, hover;
    baked_route::Reach riding, landed;
    std::vector<Release> releases;
    std::vector<Phase> before;
    std::vector<Cell> pickup;
    std::optional<Cell> carrier_approach;
    Cell carrier;
};

std::array<Cell,4> hover_neighbors(const baked::Grids& grid, Cell cell) {
    return {{{cell.layer,cell.x,(cell.y+grid.height-1)%grid.height},
             {cell.layer,(cell.x+1)%grid.width,cell.y},
             {cell.layer,cell.x,(cell.y+1)%grid.height},
             {cell.layer,(cell.x+grid.width-1)%grid.width,cell.y}}};
}

bool hover_landing(const baked_route::Request& movement,const baked::Grids& grid,Cell at) {
    return baked_route::open(movement,grid,at) &&
        (!movement.landing_group || baked::group_lands(movement.landing_group,baked_route::terrain_at(movement,grid,at.x,at.y))) &&
        !baked_route::terminal_transition(movement,grid,at) && !baked_route::trap_cell(movement,grid,at) &&
        !baked_route::transport_rule_at(movement,grid,at,baked_route::TransportRules::Gate) &&
        !baked_route::transport_rule_at(movement,grid,at,baked_route::TransportRules::Solid) &&
        !baked_route::transport_rule_at(movement,grid,at,baked_route::TransportRules::Hazard) &&
        std::none_of(movement.blocked.begin(),movement.blocked.end(),[&](const Cell& block) {
            return block.x==at.x && block.y==at.y && (block.layer<0 || block.layer==at.layer);
        }) && std::none_of(movement.hazards.begin(),movement.hazards.end(),[&](const Cell& hazard) {
            return hazard.x==at.x && hazard.y==at.y && (hazard.layer<0 || hazard.layer==at.layer);
        });
}

std::optional<HovercraftRoute> prepare_hovercraft_route(const baked_route::Request& source) {
    if (!g_route_snapshot || !game::is(game::Id::FF4) || g_route_snapshot->map_id!=1 ||
        !g_route_snapshot->vehicle_shortcuts || !g_route_snapshot->hovercraft_carried ||
        !source.submap || !source.submap->world) return {};
    const auto& sm=*source.submap;
    const auto& grid=baked::grids(sm);
    const auto* enterprise=baked::transport(10);
    const auto* hovercraft=baked::transport(2);
    if (!enterprise || !hovercraft) return {};
    const auto current=planning_vehicle();
    const auto& snapshot=*g_route_snapshot;
    auto model_at=[&](int id) -> std::optional<Cell> {
        for (const auto& model:snapshot.vehicles) if (model.transportation_id==id) {
            const auto at=cell_of(sm,model.pos,source.start.layer);
            if (baked::in_bounds(grid,at.x,at.y)) return at;
        }
        return {};
    };
    const bool aboard=current.valid && current.id==enterprise->id;
    const auto carrier=aboard?std::optional<Cell>(source.start):model_at(enterprise->id);
    const auto cargo=model_at(hovercraft->id);
    const bool attached=*snapshot.hovercraft_carried;
    if (!carrier || (!attached && !cargo) || current.id==hovercraft->id) return {};

    HovercraftRoute result;
    result.carrier=*carrier;
    result.foot=source;
    result.foot.vehicle_group=result.foot.vehicle_type=result.foot.landing_group=result.foot.transportation_id=0;
    result.foot.auto_groups.clear();
    result.foot.goals.clear();
    result.foot.goals_open=result.foot.terminal_goal=result.foot.allow_trap_goal=false;
    auto with_vehicle=[&](const baked::TransportRow& model) {
        auto request=result.foot;
        request.vehicle_group=model.group;
        request.vehicle_type=model.type;
        request.landing_group=model.landing_group;
        request.transportation_id=model.id;
        return request;
    };
    result.flight=with_vehicle(*enterprise);
    result.hover=with_vehicle(*hovercraft);
    int boarding_cost=0;
    if (!aboard) {
        auto approach=result.foot;
        auto from=source;
        for (const Cell& side:hover_neighbors(grid,*carrier))
            if (boarding_approach::allowed(approach,*enterprise,side,*carrier)) approach.goals.push_back(side);
        if (approach.goals.empty()) return {};
        approach.blocked.push_back(*carrier);
        if (source.vehicle_group>0) {
            from.blocked.push_back(*carrier);
            if (!boarding_route(sm,grid,from,approach.goals,result.before) || result.before.empty()) return {};
        } else {
            auto walk=baked_route::find(approach);
            if (!walk.found) return {};
            Phase phase;
            phase.cells=std::move(walk.cells);
            phase.movement_transportation_id=0;
            result.before.push_back(std::move(phase));
        }
        result.carrier_approach=result.before.back().cells.back();
        boarding_cost=phase_steps(result.before)+1;
    }
    // The old poses of the player and movable craft are no longer occupied
    // during the projected release and ground approach. Keep other vehicles.
    for (const auto& model:snapshot.vehicles) {
        if (model.transportation_id==2 || model.transportation_id==10 || model.transportation_id==current.id) continue;
        const auto at=cell_of(sm,model.pos,source.start.layer);
        if (!baked::in_bounds(grid,at.x,at.y)) continue;
        result.foot.blocked.push_back(at);
        result.hover.blocked.push_back(at);
    }
    result.flight.start=*carrier;
    auto first_flight=baked_route::flood(result.flight);
    if (!first_flight.submap || (cargo && first_flight.distance(*cargo)<0)) return {};
    if (!attached) {
        auto pickup_request=result.flight;
        pickup_request.goals={*cargo};
        auto pickup=baked_route::find(pickup_request);
        if (!pickup.found) return {};
        result.pickup=std::move(pickup.cells);
    }
    const Cell loaded_origin=attached?*carrier:*cargo;
    result.flight.start=loaded_origin;
    const auto loaded=attached?std::move(first_flight):baked_route::flood(result.flight);
    const auto offset=[&](Cell at) {return size_t(at.y)*grid.width+at.x;};
    const size_t cells=size_t(grid.width)*grid.height;
    std::vector<unsigned char> walkable(cells),parkable(cells);
    std::vector<Cell> parks;
    for (int y=0; y<grid.height; ++y) {
        if (planning_cancelled()) return {};
        for (int x=0; x<grid.width; ++x) {
            const Cell at{source.start.layer,x,y};
            if (!hover_landing(result.foot,grid,at)) continue;
            walkable[offset(at)]=1;
            if (loaded.distance(at)>=0 && hover_landing(result.flight,grid,at)) {
                parkable[offset(at)]=1;
                parks.push_back(at);
            }
        }
    }
    const auto parking_reach=baked_route::flood(result.foot,parks);
    if (!parking_reach.submap) return {};
    struct Walk { Cell at, approach; int cost; };
    std::vector<Walk> queue;
    std::vector<unsigned> seen(cells);
    unsigned search=0;
    auto find_parking=[&](Cell at) -> std::optional<Walk> {
        ++search;
        queue.clear();
        for (const auto& side:hover_neighbors(grid,at))
            if (walkable[offset(side)] && parking_reach.reachable(side)) {
                queue.push_back({side,side,0});
                seen[offset(side)]=search;
            }
        for (size_t i=0;i<queue.size();++i) {
            if (planning_cancelled()) return {};
            const auto entry=queue[i];
            // Native pickup takes precedence over landing within the centered
            // 3-by-3 overlap. Walk back through that area after parking outside it.
            if (parkable[offset(entry.at)]) {
                const int dx=std::abs(at.x-entry.at.x), dy=std::abs(at.y-entry.at.y);
                if (std::min(dx,grid.width-dx)>1 || std::min(dy,grid.height-dy)>1) return entry;
            }
            for (const auto& next:hover_neighbors(grid,entry.at)) {
                const auto index=offset(next);
                if (next==at || seen[index]==search || !walkable[index] ||
                    !baked_route::edge_allowed(result.foot,grid,next,entry.at)) continue;
                seen[index]=search;
                queue.push_back({next,entry.approach,entry.cost+1});
            }
        }
        return {};
    };
    std::vector<Cell> starts;
    std::vector<int> costs;
    for (int y=0;y<grid.height;++y) {
        if (planning_cancelled()) return {};
        for (int x=0;x<grid.width;++x) {
            const Cell at{source.start.layer,x,y};
            // A release leaves the hovercraft parked. It cannot perform the
            // step that enters a grotto or any other ground map transition.
            if (baked_route::terminal_transition(result.foot,grid,at)) continue;
            const bool already_parked=cargo && at==*cargo && !attached;
            const int carry=loaded.distance(at);
            if (!already_parked && (carry<0 || !hover_landing(result.hover,grid,at))) continue;
            const auto park=find_parking(at);
            if (!park) continue;
            const auto fly=park->at;
            const int parking_steps=std::min(std::abs(x-fly.x),grid.width-std::abs(x-fly.x))+
                std::min(std::abs(y-fly.y),grid.height-std::abs(y-fly.y));
            const int cost=boarding_cost+(already_parked?first_flight.distance(fly):
                (attached?0:int(result.pickup.size()))+carry+1+parking_steps)+park->cost+2;
            starts.push_back(at);
            costs.push_back(cost);
            result.releases.push_back({at,fly,park->approach,cost,!already_parked});
        }
    }
    if (starts.empty()) return {};
    result.riding=baked_route::flood(result.hover,starts,costs);
    result.landed=land_after_ride(result.hover,result.riding);
    if (!result.landed.submap) return {};
    return result;
}

template<class IsSource>
std::optional<Cell> hover_route_source(const baked_route::Request& request,const baked_route::Reach& reach,
                                      Cell at,IsSource is_source) {
    const auto& grid=baked::grids(*request.submap);
    for (int distance=reach.distance(at);distance>=0;--distance) {
        if (planning_cancelled()) return {};
        if (is_source(at,distance)) return at;
        bool found=false;
        for (const Cell& from:hover_neighbors(grid,at)) {
            if (distance>0 && reach.distance(from)==distance-1 && baked_route::edge_allowed(request,grid,from,at)) {
                at=from;
                found=true;
                break;
            }
        }
        if (!found) return {};
    }
    return {};
}

std::vector<Phase> hovercraft_directions(const baked_route::Request& source) {
    auto prepared=prepare_hovercraft_route(source);
    if (!prepared) return {};
    auto& plan=*prepared;
    const auto& map=*source.submap;
    const auto& grid=baked::grids(map);
    auto foot=plan.foot;
    foot.goals=source.goals;
    // A landed party may enter the native floor contact even when an aircraft
    // is forbidden to enter that same terminal tile while mounted.
    foot.goals_open=true;
    foot.terminal_goal=source.terminal_goal;
    const auto deposited=std::find_if(plan.releases.begin(),plan.releases.end(),[](const auto& release){return !release.lower;});
    if (deposited!=plan.releases.end()) {
        auto ride=baked_route::flood(plan.hover,{deposited->cargo},{deposited->cost});
        auto land=land_after_ride(plan.hover,ride);
        if (baked_route::transition_distance(foot,land)>=0) {
            plan.riding=std::move(ride);
            plan.landed=std::move(land);
            const auto carried=*deposited;
            plan.releases={carried};
        }
    }
    std::optional<Cell> finish;
    int best=INT_MAX;
    for (const auto& goal:source.goals) {
        if (!baked::in_bounds(grid,goal.x,goal.y)) continue;
        const int distance=plan.landed.distance(goal);
        if (distance>=0 && distance<best) {best=distance;finish=goal;}
        for (const auto& side:hover_neighbors(grid,goal)) {
            const int steps=plan.landed.distance(side);
            if (steps>=0 && steps+1<best && baked_route::edge_allowed(foot,grid,side,goal)) {
                best=steps+1;
                finish=side;
            }
        }
    }
    if (!finish) return {};
    const auto landing=hover_route_source(plan.foot,plan.landed,*finish,[&](Cell at,int distance) {
        return plan.riding.distance(at)==distance && hover_landing(plan.hover,grid,at);
    });
    if (!landing) return {};
    auto release_at=[&](Cell at) {
        return std::lower_bound(plan.releases.begin(),plan.releases.end(),at,[](const auto& release,const Cell& cell){
            return std::tie(release.cargo.y,release.cargo.x)<std::tie(cell.y,cell.x);
        });
    };
    const auto release_cell=hover_route_source(plan.hover,plan.riding,*landing,[&](Cell at,int distance) {
        const auto release=release_at(at);
        return release!=plan.releases.end() && release->cargo==at && release->cost==distance;
    });
    if (!release_cell) return {};
    const auto release=release_at(*release_cell);

    std::vector<Phase> phases=std::move(plan.before);
    auto fly=plan.flight;
    const auto name=planning_vehicle_name(10,8,L"");
    auto flight_phase=[&](std::vector<Cell> path) -> Phase& {
        Phase phase;
        phase.cells=std::move(path);
        phase.movement_transportation_id=phase.transportation_id=10;
        if (plan.carrier_approach) {
            phase.vehicle_before=route_progress::VehicleAction::ConfirmBoard;
            phase.action_cell=plan.carrier_approach;
            phase.vehicle_direction=baked_route::step_direction(*plan.carrier_approach,plan.carrier,grid.width,grid.height);
            phase.action_before=TF(L"board the {0}",{name.empty()?T(L"Airship"):name})+L" "+T(direction_key(phase.vehicle_direction));
            plan.carrier_approach.reset();
        }
        phases.push_back(std::move(phase));
        return phases.back();
    };
    if (release->lower) {
        if (!plan.pickup.empty()) {
            auto& pickup=flight_phase(std::move(plan.pickup));
            pickup.action_after=T(L"Pick up the hovercraft.");
            pickup.manual_after=true;
        }
        fly.goals={release->cargo};
        auto carried=baked_route::find(fly);
        if (!carried.found) return {};
        auto& lower=flight_phase(std::move(carried.cells));
        lower.action_after=T(L"Lower the hovercraft.");
        lower.manual_after=true;
    }
    fly.start=release->lower?release->cargo:plan.carrier;
    fly.goals={release->park};
    auto parking=baked_route::find(fly);
    if (!parking.found) return {};
    flight_phase(std::move(parking.cells));

    auto approach=plan.foot;
    approach.start=release->park;
    approach.goals={release->approach};
    approach.blocked.push_back(release->cargo);
    auto ground=baked_route::find(approach);
    if (!ground.found) return {};
    Phase on_foot;
    on_foot.cells=std::move(ground.cells);
    on_foot.movement_transportation_id=0;
    on_foot.transportation_id=10;
    on_foot.vehicle_before=route_progress::VehicleAction::ConfirmLand;
    on_foot.action_cell=release->park;
    on_foot.action_before=T(L"land");
    phases.push_back(std::move(on_foot));

    auto hover=plan.hover;
    hover.start=release->cargo;
    hover.goals=source.goals;
    hover.blocked.push_back(release->park);
    std::vector<Phase> last;
    {
        JourneyModeScope mode(*g_route_snapshot,{true,2,7,{}},release->cargo,nullptr,true);
        if (!boarding_route(map,grid,hover,source.goals,last) || last.empty()) return {};
    }
    auto& board=last.front();
    board.vehicle_before=route_progress::VehicleAction::ConfirmBoard;
    board.action_cell=release->approach;
    board.vehicle_direction=baked_route::step_direction(release->approach,release->cargo,grid.width,grid.height);
    board.action_before=TF(L"board the {0}",{T(L"Hovercraft")})+L" "+T(direction_key(board.vehicle_direction));
    for (auto& phase:last) phases.push_back(std::move(phase));
    return phases;
}
