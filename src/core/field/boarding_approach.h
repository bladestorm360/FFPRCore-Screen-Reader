#pragma once
#include "baked_router.h"

// Value-only physical approach policy. Native SeamlessHitCheck accepts a
// walk-on symbol boat only when its landing table accepts the CURRENT shore
// attribute. Walking/canoe passability alone is insufficient. Aircraft retain
// their separate confirm/deck actions. Apply before choosing the shortest side.
namespace field::boarding_approach {
inline bool allowed(const baked_route::Request& foot,const baked::TransportRow& vehicle,
                    const baked_route::Cell& shore,const baked_route::Cell& aboard) {
    if(!foot.submap||!foot.submap->world)return false;
    const auto& grid=baked::grids(*foot.submap);
    const int direction=baked_route::step_direction(shore,aboard,grid.width,grid.height);
    if(direction<0||direction>3||!baked_route::open(foot,grid,shore))return false;
    const bool occupied=shore.x==foot.start.x&&shore.y==foot.start.y;
    if(!occupied&&(baked_route::terminal_transition(foot,grid,shore)||baked_route::trap_cell(foot,grid,shore)))return false;
    const bool walk_on=vehicle.type==2||vehicle.type==6;
    if(!walk_on)return true;
    return baked_route::direction_allowed(foot,shore,aboard)&&vehicle.landing_group>0&&
        baked::group_lands(vehicle.landing_group,baked_route::terrain_at(foot,grid,shore.x,shore.y));
}
}
