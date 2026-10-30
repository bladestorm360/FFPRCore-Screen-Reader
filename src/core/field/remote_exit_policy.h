#pragma once
// Remote destination-list policy only. Never feed this mask into the routing
// graph, current-floor exits, transitions or bake generation. See the October26
// remote-return record for native identity evidence and conservative limits.
#include "baked_router.h"
#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace field::remote_exits {
struct Door {
    int source=-1,destination=-1,source_depth=-1,destination_depth=-1;
    baked::Object footprint{};
    std::optional<baked_route::Cell> arrival;
    int arrival_direction=0; // native PointIn facing: Up1,Right2,Down4,Left8
    bool known=false;
};

inline bool at_door(const baked_route::Cell& point,const baked::Object& door,int facing=0) {
    if(point.layer!=door.layer||door.tw==0||door.th==0)return false;
    // Native PointIn stands on or immediately beside the corresponding door's
    // footprint. Larger/ambiguous separations are not certified return pairs.
    const int dx=point.x<door.tx?door.tx-point.x:point.x>=door.tx+door.tw?point.x-(door.tx+door.tw-1):0;
    const int dy=point.y<door.ty?door.ty-point.y:point.y>=door.ty+door.th?point.y-(door.ty+door.th-1):0;
    if(dx+dy<=1)return true;
    // Native shops can place PointIn two cells inside, facing into the room.
    // Certify only that straight inward placement, not a larger radius around
    // arbitrary doors. Exact unique reciprocal IDs/arrivals still own the pair.
    if(dx+dy!=2||(!dx&&!dy)||(dx&&dy))return false;
    if(!dx)return point.y<door.ty?facing==1:facing==4;
    return point.x<door.tx?facing==8:facing==2;
}

inline std::vector<bool> return_mask(const std::vector<Door>& doors,const std::function<bool()>& cancelled={}) {
    std::vector<bool> hidden(doors.size(),false);
    std::map<std::pair<int,int>,std::vector<size_t>> directions;
    for(size_t i=0;i<doors.size();++i){
        if(cancelled&&cancelled())return {};
        const auto& door=doors[i];
        if(door.known&&door.arrival)directions[{door.source,door.destination}].push_back(i);
    }
    for(size_t i=0;i<doors.size();++i){
        if(cancelled&&cancelled())return {};
        const auto& door=doors[i];
        if(!door.known||!door.arrival||door.source_depth<=0||door.destination_depth<0||
            door.destination_depth>=door.source_depth||door.source==door.destination)continue;
        const auto back=directions.find({door.destination,door.source});
        if(back==directions.end())continue;
        int matches=0;
        for(const size_t index:back->second){
            if(cancelled&&cancelled())return {};
            const auto& forward=doors[index];
            if(at_door(*door.arrival,forward.footprint,door.arrival_direction)&&
               at_door(*forward.arrival,door.footprint,forward.arrival_direction))++matches;
        }
        // A unique reciprocal physical doorway is the only suppression proof.
        // Keep one-way drops, alternative entrances, loops and uncertain pairs.
        hidden[i]=matches==1;
    }
    return hidden;
}
}
