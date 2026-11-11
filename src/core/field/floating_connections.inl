// Included after edges_of by baked_nav.cpp. Original mod graph facts only.
// Data prepares a conservative union of every baked interior transfer; no flag,
// collision, NPC or route is discarded while building this over-approximation.
// It can prove a world-to-interior detour redundant, never prove a new route.
struct FloatingConnection {
    bool unknown=false;
    std::vector<Cell> returns;
};
struct FloatingConnections {
    const baked::Submap* world=nullptr;
    std::unordered_map<int,FloatingConnection> rooms;
};
const FloatingConnections* g_floating_connections=nullptr; // immutable after Data readiness

void prepare_floating_connections() try {
    g_floating_connections=nullptr;
    if(baked::game_number()!=3)return;
    const auto* world=baked::submap_by_map_id(1);
    const int count=baked::submap_count();
    if(!world||!world->world||count<=0||count>4096)return;
    auto prepared=std::make_unique<FloatingConnections>();prepared->world=world;
    for(const auto& entry:edges_of(baked::grids(*world),1)) {
        const auto* root=baked::submap_by_map_id(entry.dest_map_id);
        if(!root||root->world)continue;
        const int root_index=baked::submap_index(root);
        if(root_index<0||root_index>=count||prepared->rooms.contains(root_index))continue;
        FloatingConnection facts;
        std::vector<uint8_t> seen(static_cast<size_t>(count));seen[static_cast<size_t>(root_index)]=1;
        std::vector<int> pending{root_index};
        for(size_t next=0;next<pending.size()&&!facts.unknown;++next) {
            const auto* room=baked::submap_at(pending[next]);
            if(!room){facts.unknown=true;break;}
            for(const auto& edge:edges_of(baked::grids(*room),room->first_map_id)) {
                const auto* destination=baked::submap_by_map_id(edge.dest_map_id);
                if(!destination){facts.unknown=true;break;}
                if(destination->world) {
                    if(destination!=world){facts.unknown=true;break;}
                    bool found=false;
                    // Include every matching PointIn, even a dormant variant.
                    // This union is deliberately broader than point_in's
                    // current native-group selection. Unknown never prunes.
                    for(const auto& point:baked::grids(*destination).objects) {
                        if(point.type!=baked::ObjType::PointIn||
                           (edge.dest_point>0&&point.a!=edge.dest_point))continue;
                        found=true;const Cell cell{1,point.tx,point.ty};
                        if(std::find(facts.returns.begin(),facts.returns.end(),cell)==facts.returns.end())facts.returns.push_back(cell);
                    }
                    if(!found){facts.unknown=true;break;}
                    continue;
                }
                const int index=baked::submap_index(destination);
                if(index<0||index>=count){facts.unknown=true;break;}
                if(seen[static_cast<size_t>(index)])continue;
                // The supported game's complete interior graph is smaller
                // than the existing400-state route bound. Do not silently
                // truncate an unexpected larger closure and certify it.
                if(pending.size()>=400){facts.unknown=true;break;}
                seen[static_cast<size_t>(index)]=1;pending.push_back(index);
            }
        }
        prepared->rooms.emplace(root_index,std::move(facts));
    }
    g_floating_connections=prepared.release(); // process-lifetime prepared bake owner
} catch(...) {
    // This optional static proof must never hold navigation readiness hostage.
    // The established full graph remains the safe fallback on preparation fault.
    g_floating_connections=nullptr;
    LOGF("[BakedNav] fixed-world connection proof unavailable; full graph retained");
}

bool floating_connection_dominated(const baked::Submap& room,const baked_route::Reach& initial,int entered_cost) {
    if(!g_floating_connections||g_floating_connections->world!=initial.submap||entered_cost<0||entered_cost>=INT_MAX-1)return false;
    const auto found=g_floating_connections->rooms.find(baked::submap_index(&room));
    if(found==g_floating_connections->rooms.end()||found->second.unknown)return false;
    // Every return needs at least one further transition (+1), regardless of
    // the interior's gates, walking distance, trap choices or NPC policy.
    const int earliest_return=entered_cost+1;
    for(const auto& cell:found->second.returns) {
        const int prior=initial.distance(cell);
        if(prior<0||prior>earliest_return)return false;
    }
    return true;
}
