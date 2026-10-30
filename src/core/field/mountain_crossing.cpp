// Read-only owned-data translation of FFIII TransportationEvent's reviewed
// Invincible mountain rule. No native calls or synthetic open mountain terrain.
#include "baked_router.h"

namespace field::baked_route {
bool mountain_vehicle(const Request& request) {
    return request.submap&&request.submap->world&&request.vehicle_group==6&&request.vehicle_type==8&&
        (request.transportation_id==10||request.transportation_id==11)&&baked::game_number()==3;
}
std::optional<MountainEdge> mountain_edge(const Request& request,const baked::Grids& grid,const Cell& from,int direction) {
    if(!mountain_vehicle(request)||direction<0||direction>3||grid.width<4||grid.height<4||
       !baked::in_bounds(grid,from.x,from.y))return {};
    static constexpr int directions[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
    const auto& delta=directions[direction];
    for(int distance=1;distance<=3;++distance) {
        if(request.cancelled&&request.cancelled())return {};
        const Cell cell{from.layer,(from.x+delta[0]*distance+grid.width)%grid.width,
                                   (from.y+delta[1]*distance+grid.height)%grid.height};
        const int attribute=baked::attribute(grid,cell.x,cell.y);
        if(attribute==19)continue;
        // The native action must start at a mountain and ends at the first
        // non-mountain. It cannot skip water/closed far sides to a later tile.
        if(distance==1||!baked::group_crosses(request.vehicle_group,attribute)||
           !baked::vehicle_open(grid,request.vehicle_group,cell.x,cell.y))return {};
        return MountainEdge{cell,distance,direction};
    }
    return {}; // three consecutive mountain tiles are never a legal crossing
}
}
