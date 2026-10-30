// Included after boarding_route by baked_nav.cpp. This is the FFIII owned
// vehicle-change adapter, not another native pathfinder or capture pipeline.
// Every edge reuses the existing qualified single-craft/foot leg, preserving
// its landing, shore, deck, hazard and manual-action contracts.
int journey_vehicle_family(int id) {
    if(id==6||id==13)return 6;
    if(id==15||id==16)return 15;
    if(id==8||id==9||id==14)return 8;
    if(id==10||id==11)return 10;
    return id;
}
std::vector<VehicleOnMap> journey_vehicles() {
    std::vector<VehicleOnMap> result;
    if(!g_route_snapshot||g_route_snapshot->vehicles.size()>32)return result;
    const auto current=planning_vehicle();
    for(const auto& vehicle:g_route_snapshot->vehicles) {
        if(!std::isfinite(vehicle.pos.x)||!std::isfinite(vehicle.pos.y))continue;
        const auto* row=baked::transport(vehicle.transportation_id);
        if(!row||row->type==0||row->type==1||row->type==4||row->type==kTransportContent)continue;
        if(current.valid&&journey_vehicle_family(current.id)==journey_vehicle_family(row->id))continue;
        result.push_back(vehicle);
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b) {
        const auto fa=journey_vehicle_family(a.transportation_id),fb=journey_vehicle_family(b.transportation_id);
        if(fa!=fb)return fa<fb;
        return a.transportation_id>b.transportation_id;
    });
    result.erase(std::unique(result.begin(),result.end(),[](const auto& a,const auto& b) {
        return journey_vehicle_family(a.transportation_id)==journey_vehicle_family(b.transportation_id);
    }),result.end());
    return result;
}

// A worker-only virtual leg changes only value-owned planning observations.
// The original goal, current craft and captured parked models are restored
// before returning. No native pointer, API, input or persistent state is used.
struct JourneyModeScope {
    RouteSnapshot& snapshot;
    VehicleState vehicle;
    Vec3 position;
    std::vector<VehicleOnMap> models;
    int target;
    JourneyModeScope(RouteSnapshot& owner,const VehicleState& mode,const Cell& at,
                     const VehicleOnMap* candidate,bool intermediate)
        :snapshot(owner),vehicle(owner.vehicle),position(owner.position),
         models(std::move(owner.vehicles)),target(owner.goal.transportation_id) {
        owner.vehicle=mode;owner.position=world_of_cell(*owner.submap,at);
        owner.vehicles.clear();if(candidate)owner.vehicles.push_back(*candidate);
        if(intermediate)owner.goal.transportation_id=0;
    }
    ~JourneyModeScope() {
        snapshot.vehicle=vehicle;snapshot.position=position;
        snapshot.vehicles=std::move(models);snapshot.goal.transportation_id=target;
    }
};

bool vehicle_journey_route(const baked::Submap& sm,const baked::Grids& grid,
                           const baked_route::Request& movement,const std::vector<Cell>& goals,
                           std::vector<Phase>& phases,const air_transitions::Rule* aerial=nullptr) {
    if(boarding_route(sm,grid,movement,goals,phases,aerial))return true;
    if(!game::is(game::Id::FF3)||!g_route_snapshot||!g_route_snapshot->vehicle_shortcuts||!sm.world)return false;
    const auto candidates=journey_vehicles();if(candidates.empty())return false;
    struct Node {
        VehicleOnMap craft;
        Cell approach;
        bool root=false,done=false;
        int steps=INT_MAX,crossings=INT_MAX,depth=INT_MAX;
        std::vector<Phase> prefix;
    };
    std::vector<Node> nodes(candidates.size()+1);
    nodes[0].root=true;nodes[0].approach=movement.start;
    nodes[0].steps=nodes[0].crossings=nodes[0].depth=0;
    for(size_t i=0;i<candidates.size();++i)nodes[i+1].craft=candidates[i];
    auto foot=movement;foot.vehicle_group=foot.vehicle_type=foot.landing_group=foot.transportation_id=0;
    const auto crossings=[&](const std::vector<Phase>& route) {
        int total=0;
        for(const auto& phase:route) {
            total+=phase.manual_displacement_steps>0;
            for(size_t i=1;i<phase.cells.size();++i)
                total+=baked_route::step_direction(phase.cells[i-1],phase.cells[i],grid.width,grid.height)<0;
        }
        return total;
    };
    const auto better=[](int jump,int distance,int depth,int old_jump,int old_distance,int old_depth) {
        return jump<old_jump||(jump==old_jump&&(distance<old_distance||(distance==old_distance&&depth<old_depth)));
    };
    int best_steps=INT_MAX,best_crossings=INT_MAX,best_depth=INT_MAX;
    std::vector<Phase> best;
    for(size_t expanded=0;expanded<nodes.size();++expanded) {
        if(planning_cancelled())return false;
        int chosen=-1;
        for(size_t i=0;i<nodes.size();++i)if(!nodes[i].done&&nodes[i].steps!=INT_MAX&&
            (chosen<0||better(nodes[i].crossings,nodes[i].steps,nodes[i].depth,
                              nodes[chosen].crossings,nodes[chosen].steps,nodes[chosen].depth)))chosen=static_cast<int>(i);
        if(chosen<0)break;
        auto& node=nodes[static_cast<size_t>(chosen)];node.done=true;
        // A root represents the currently ridden craft or walking. Later nodes
        // are on foot at the proven contact tile immediately before boarding.
        const auto leg=[&](const std::vector<Cell>& target,std::vector<Phase>& out,bool terminal) {
            if(planning_cancelled())return false;
            auto request=node.root?movement:foot;request.start=node.approach;request.goals=target;
            if(node.root&&movement.vehicle_group==0) {
                // Ordinary walking was already evaluated by the caller. Do
                // not use a goal-open override to walk into an airborne/sea
                // event which actually requires its qualified vehicle.
                if(terminal)return false;
                request.goals_open=terminal?movement.goals_open:false;
                const auto walked=baked_route::find(request);if(!walked.found)return false;
                Phase phase;phase.cells=walked.cells;phase.movement_transportation_id=0;out={std::move(phase)};return true;
            }
            const VehicleState mode=node.root?planning_vehicle():VehicleState{};
            JourneyModeScope scope(*g_route_snapshot,mode,node.approach,node.root?nullptr:&node.craft,!terminal);
            return boarding_route(sm,grid,request,target,out,terminal?aerial:nullptr);
        };
        std::vector<Phase> tail;
        // The unchanged single-craft goal attempt at entry already failed.
        // Repeating it for the root would duplicate a full flood/search.
        if(!node.root&&leg(goals,tail,true)) {
            const int distance=node.steps+phase_steps(tail),jump=node.crossings+crossings(tail);
            if(better(jump,distance,node.depth,best_crossings,best_steps,best_depth)) {
                best=node.prefix;best.insert(best.end(),tail.begin(),tail.end());
                best_steps=distance;best_crossings=jump;best_depth=node.depth;
            }
        }
        for(size_t index=1;index<nodes.size();++index) {
            if(planning_cancelled())return false;
            auto& next=nodes[index];if(next.done)continue;
            const auto* row=baked::transport(next.craft.transportation_id);if(!row)continue;
            const auto at=cell_of(sm,next.craft.pos,movement.start.layer);
            if(!baked::in_bounds(grid,at.x,at.y))continue;
            std::vector<Cell> contact;
            constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
            for(const auto& side:sides) {
                const Cell beside{at.layer,(at.x+side[0]+grid.width)%grid.width,(at.y+side[1]+grid.height)%grid.height};
                if(!baked_route::open(foot,grid,beside)||baked_route::terminal_transition(foot,grid,beside))continue;
                if((row->type==kTransportShip||row->type==kTransportSubmarine)&&!baked_route::direction_allowed(foot,beside,at))continue;
                contact.push_back(beside);
            }
            if(contact.empty())continue;
            std::vector<Phase> prefix;if(!leg(contact,prefix,false)||prefix.empty()||prefix.back().cells.empty())continue;
            const int distance=node.steps+phase_steps(prefix),jump=node.crossings+crossings(prefix);
            if(!better(jump,distance,node.depth+1,next.crossings,next.steps,next.depth))continue;
            next.steps=distance;next.crossings=jump;next.depth=node.depth+1;
            next.approach=prefix.back().cells.back();next.prefix=node.prefix;
            next.prefix.insert(next.prefix.end(),prefix.begin(),prefix.end());
        }
    }
    if(best.empty())return false;
    phases=std::move(best);return true;
}
