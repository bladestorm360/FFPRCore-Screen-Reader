#pragma once
#include "baked_router.h"
#include <limits>

namespace field::baked_nav {
// Pure owned-snapshot evidence for a blocked destination. This names no story
// outcome: it offers the nearest safe, currently active step-on event, if any.
// One flood and at most one A*; never remove gates, NPCs, directional rules or traps.
inline std::vector<baked_route::Cell> reachable_event_route(baked_route::Request request) {
    using namespace baked_route;
    if(!request.submap || request.hazards.empty())return {};
    request.goals.clear();request.goals_open=false;request.allow_trap_goal=false;
    const auto& grid=baked::grids(*request.submap);
    const auto reach=flood(request);
    int best=std::numeric_limits<int>::max();Cell chosen;bool found=false;
    for(const auto& event:request.hazards)for(int layer=0;layer<3;++layer) {
        if(request.cancelled && request.cancelled())return {};
        if(event.layer>=0 && event.layer!=layer)continue;
        const Cell at{layer,event.x,event.y};
        if(!open(request,grid,at)||trap_cell(request,grid,at))continue;
        request.goals={at};
        const int distance=transition_distance(request,reach);
        if(distance>0 && distance<best){best=distance;chosen=at;found=true;}
    }
    if(!found)return {};
    request.goals={chosen};
    return find(request).cells;
}
}
