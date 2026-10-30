// FFIII's two Nautilus maps use different terrain and transportation masters.
// Native dive/surface scripts preserve the craft's exact position. This owned
// graph uses those real paired tiles, not fabricated GotoMap objects or foot
// landings in water. See WORLD_MAP_JOURNEYS_20261030.md for source locators.
// Included after the ordinary planner, before route_to_goal's publication owner.
struct NautilusJourney {
    bool found=false;
    int next_map=0,total=0,entity=-300020;
    Cell crossing,arrival;
    std::vector<Phase> phases;
};

NautilusJourney nautilus_world_journey(const MapGoal& goal,bool airborne_goal=false) {
    NautilusJourney result;
    if(!game::is(game::Id::FF3)||!g_route_snapshot||!g_route_snapshot->vehicle_shortcuts||
       (goal.map_id!=3&&goal.map_id!=4)||(planning_map_id()!=3&&planning_map_id()!=4))return result;
    const auto* surface=baked::submap_by_map_id(4);
    const auto* water=baked::submap_by_map_id(3);
    if(!surface||!water||!surface->world||!water->world||surface->width!=256||surface->height!=256||
       water->width!=surface->width||water->height!=surface->height)return result;
    const auto current=planning_vehicle();
    const int origin_map=planning_map_id();
    Cell origin=cell_of(*g_route_snapshot->submap,snapshot_position(),1);
    std::vector<Phase> prefix;
    if(origin_map==3) {
        if(!current.valid||current.id!=14)return result;
    } else if(!current.valid||current.id!=9) {
        // A real captured upgraded Nautilus must be reachable first, possibly
        // through another captured craft. Never infer its unlock or position.
        const auto candidates=journey_vehicles();
        const auto ship=std::find_if(candidates.begin(),candidates.end(),[](const auto& v){return v.transportation_id==9;});
        if(ship==candidates.end())return result;
        const auto at=cell_of(*surface,ship->pos,1);
        auto request=walk_request(*surface,origin);world_mode(request);
        const auto live=live_blockers(*surface,baked::grids(*surface),{});
        request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
        request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;
        request.goals_open=false;request.terminal_goal=true;
        const air_transitions::Rule upgraded{4,0,0,L"",1u<<9};
        if(!vehicle_journey_route(*surface,baked::grids(*surface),request,{at},prefix,&upgraded))return result;
        origin=at;
    }
    if(!baked::in_bounds(baked::grids(*surface),origin.x,origin.y))return result;
    std::array<baked_route::Request,2> modes;
    const baked::Submap* maps[2]={surface,water};
    for(int mode=0;mode<2;++mode) {
        auto& request=modes[mode];request=walk_request(*maps[mode],origin);
        const auto* row=baked::transport(mode==0?9:14);if(!row)return result;
        request.vehicle_group=row->group;request.vehicle_type=row->type;request.landing_group=row->landing_group;
        request.transportation_id=row->id;request.goals_open=false;
        if(maps[mode]==g_route_snapshot->submap&&!g_route_snapshot->projected_origin) {
            const auto live=live_blockers(*maps[mode],baked::grids(*maps[mode]),{});
            request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
            request.live_hazards=request.live_traps=true;request.hazards=live.triggers;request.traps=live.traps;
        } else map_state::add_obstacles(g_route_snapshot->state,*maps[mode],request,true,false,false);
    }
    const int target_mode=goal.map_id==4?0:1,initial_mode=origin_map==4?0:1;
    auto target_cells=goal.cells;
    if(target_cells.empty())target_cells={goal.cell};
    if(target_cells.empty())return result;
    // Surface town doors need a legal ordinary landing and walking approach.
    // Underwater script entrances explicitly permit submarine entry instead.
    auto foot=modes[0];foot.vehicle_group=foot.vehicle_type=foot.landing_group=foot.transportation_id=0;
    foot.auto_groups=planning_auto_groups();foot.goals=target_cells;foot.goals_open=true;
    baked_route::Reach final_walk;
    if(target_mode==0&&!airborne_goal) {
        // Reverse seeds must be able to leave their native town terminal.
        // Match boarding_route: these seeds are not forward request goals.
        auto reverse=foot;reverse.goals.clear();
        final_walk=baked_route::flood(reverse,target_cells);
    }
    if(target_mode==1) {modes[1].goals=target_cells;modes[1].goals_open=true;modes[1].allow_trap_goal=true;}
    if(target_mode==0&&airborne_goal) {modes[0].goals=target_cells;modes[0].terminal_goal=true;modes[0].allow_trap_goal=true;}
    constexpr int area=256*256;
    const int stages=initial_mode==target_mode?3:2;
    // At most two manual map changes: fly/dive/enter, or dive/tunnel/surface.
    // A user can still select a dungeon directly underwater for its slower
    // ordinary route. Prefer fewer underwater tiles, then fewer actions and
    // physical steps, without pretending these are measured animation times.
    struct Cost {int water=INT_MAX,actions=INT_MAX,steps=INT_MAX;};
    const auto less=[](const Cost& a,const Cost& b) {
        return a.water<b.water||(a.water==b.water&&(a.actions<b.actions||
            (a.actions==b.actions&&a.steps<b.steps)));
    };
    struct Node {Cost cost;int parent=-1;};
    struct Item {Cost cost;int index;};
    const auto later=[&](const Item& a,const Item& b){return less(b.cost,a.cost);};
    std::priority_queue<Item,std::vector<Item>,decltype(later)> queue(later);
    std::vector<Node> nodes(area*stages);
    const auto mode_of=[&](int stage){return (initial_mode+stage)%2;};
    std::array<std::unordered_set<int>,2> completed;
    if(g_pending&&g_pending->valid&&g_pending->goal.map_id==g_route_snapshot->goal.map_id&&
       g_pending->goal.cell==g_route_snapshot->goal.cell&&g_pending->goal.entity_id==g_route_snapshot->goal.entity_id&&
       g_pending->goal.raw_name==g_route_snapshot->goal.raw_name)
        for(const auto& used:g_pending->completed_transitions) {
            if(used.entity_id!=-300020||used.x<0||used.x>=256||used.y<0||used.y>=256)continue;
            if(used.map_id==4&&used.destination==3)completed[0].insert(used.y*256+used.x);
            if(used.map_id==3&&used.destination==4)completed[1].insert(used.y*256+used.x);
        }
    const auto cell_of_index=[](int index){return Cell{1,index%256,(index%area)/256};};
    const int start=origin.y*256+origin.x;
    nodes[start].cost={0,0,0};queue.push({{0,0,0},start});
    int finish=-1;
    Cost best;
    const auto blocked=[](const baked_route::Request& request,const Cell& at) {
        const auto& grid=baked::grids(*request.submap);
        return baked_route::transport_rule_at(request,grid,at,baked_route::TransportRules::Gate)||
            baked_route::transport_rule_at(request,grid,at,baked_route::TransportRules::Solid)||
            std::any_of(request.blocked.begin(),request.blocked.end(),[&](const Cell& c) {
            return c.x==at.x&&c.y==at.y&&(c.layer<0||c.layer==at.layer);
        });
    };
    while(!queue.empty()) {
        if(planning_cancelled())return {};
        const auto item=queue.top();queue.pop();
        if(less(nodes[item.index].cost,item.cost))continue;
        if(finish>=0&&!less(item.cost,best))break;
        const int stage=item.index/area,mode=mode_of(stage);
        const Cell at=cell_of_index(item.index);
        const auto& request=modes[mode];const auto& grid=baked::grids(*maps[mode]);
        int walk=-1;
        if(mode==target_mode) {
            if(mode==1&&std::find(target_cells.begin(),target_cells.end(),at)!=target_cells.end())walk=0;
            if(mode==0&&airborne_goal&&std::find(target_cells.begin(),target_cells.end(),at)!=target_cells.end())walk=0;
            if(mode==0&&!airborne_goal&&baked::attribute(grid,at.x,at.y)!=16&&
               baked::group_lands(request.landing_group,baked::attribute(grid,at.x,at.y))&&
               baked_route::open(foot,grid,at)&&!blocked(foot,at)&&!baked_route::terminal_transition(foot,grid,at))
                walk=final_walk.distance(at);
        }
        if(walk>=0&&stage>0) {
            auto cost=item.cost;cost.steps+=walk;
            if(less(cost,best)){best=cost;finish=item.index;}
        }
        const auto add=[&](int index,Cost cost) {
            if(!less(cost,nodes[index].cost))return;
            nodes[index]={cost,item.index};queue.push({cost,index});
        };
        std::array<Cell,8> next;
        const auto count=baked_route::successors(request,at,next);
        for(size_t i=0;i<count;++i) {
            auto cost=item.cost;++cost.steps;if(mode==1)++cost.water;
            add(stage*area+next[i].y*256+next[i].x,cost);
        }
        if(stage+1<stages&&!completed[mode].contains(at.y*256+at.x)&&
           baked::attribute(baked::grids(*surface),at.x,at.y)==16&&
           baked::group_lands(modes[1].landing_group,baked::attribute(baked::grids(*water),at.x,at.y))&&
           baked_route::open(modes[1-mode],baked::grids(*maps[1-mode]),at)&&
           !blocked(modes[0],at)&&!blocked(modes[1],at)&&
           !baked_route::terminal_transition(modes[0],baked::grids(*surface),at)&&
           !baked_route::terminal_transition(modes[1],baked::grids(*water),at)) {
            auto cost=item.cost;++cost.actions;
            add((stage+1)*area+at.y*256+at.x,cost);
        }
    }
    if(finish<0||planning_cancelled())return result;
    std::vector<int> path;
    for(int index=finish;index>=0;index=nodes[index].parent)path.push_back(index);
    std::reverse(path.begin(),path.end());
    Phase phase;phase.transportation_id=phase.movement_transportation_id=initial_mode==0?9:14;
    for(const int index:path) {
        if(index/area!=0)break;
        phase.cells.push_back(cell_of_index(index));
    }
    if(phase.cells.empty())return result;
    phase.action_after=T(initial_mode==0?L"dive here":L"surface here");phase.manual_after=true;
    result.found=true;result.next_map=initial_mode==0?3:4;result.crossing=phase.cells.back();
    result.arrival=result.crossing;
    result.total=phase_steps(prefix)+best.steps;result.phases=std::move(prefix);result.phases.push_back(std::move(phase));
    return result;
}
