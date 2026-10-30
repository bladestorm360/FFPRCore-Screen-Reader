// Native transportation selectors projected onto owned navigation geometry.
// No native pointers, SDK access, callbacks or queues are retained here.
#include "baked_router.h"

namespace field::baked_route {
uint64_t transportation_bit(int id) {
    // Unknown mode is conservative: it may activate any listed obstruction.
    return id>0&&id<64?uint64_t(1)<<id:~uint64_t(0);
}

int transportation_at(const Request& request,const baked::Grids& grid,const Cell& cell) {
    if(!request.submap||!request.submap->world||request.vehicle_group>0)
        return request.transportation_id>0?request.transportation_id:(request.vehicle_group>0?0:1);
    // The supported native automatic groups have disjoint terrain masks in all
    // six masters. Once their native transition finishes, terrain fixes the
    // stable foot/canoe/snowcraft identity; manual parked craft are not auto-use.
    const int attribute=baked::attribute(grid,cell.x,cell.y);
    if(baked::group_crosses(1,attribute))return 1;
    for(const int group:request.auto_groups)
        if(baked::group_crosses(group,attribute))
            for(const auto& row:baked::transports())
                if(row.type==5&&row.group==group)return row.id;
    return 1;
}

void add_transport_cell(TransportRules& rules,TransportRules::Kind kind,
                        const baked::Submap& map,const Cell& cell,uint64_t ids) {
    if(!ids||cell.x<0||cell.y<0||cell.x>=map.width||cell.y>=map.height)return;
    if(rules.submap&&rules.submap!=&map)return;
    rules.submap=&map;
    const size_t count=static_cast<size_t>(map.width)*map.height;
    for(int layer=0;layer<(map.world?1:3);++layer) {
        if(!map.world&&cell.layer>=0&&layer!=cell.layer)continue;
        auto& plane=rules.cells[kind][layer];
        if(plane.empty())plane.resize(count);
        plane[static_cast<size_t>(cell.y)*map.width+cell.x]|=ids;
    }
}

void add_transport_collision(TransportRules& rules,const baked::Submap& map,
                             const Cell& cell,int collision,uint64_t ids) {
    if(!ids||collision==0||(rules.submap&&rules.submap!=&map))return;
    rules.submap=&map;
    // Reuse the native collision decoder, including partial and unknown IDs.
    std::vector<Cell> hard;std::vector<Request::Directional> edges;
    add_collision(hard,edges,cell,collision);
    for(const auto& at:hard)add_transport_cell(rules,TransportRules::Gate,map,at,ids);
    for(const auto& edge:edges)rules.directional.push_back({edge.cell,edge.axis,edge.mask,ids});
}

void merge_transport_rules(TransportRules& into,const TransportRules& source) {
    if(!source.submap||(into.submap&&into.submap!=source.submap))return;
    into.submap=source.submap;
    for(size_t kind=0;kind<TransportRules::Count;++kind)for(size_t layer=0;layer<3;++layer) {
        const auto& input=source.cells[kind][layer];auto& output=into.cells[kind][layer];
        if(input.empty())continue;
        if(output.empty())output=input;
        else if(output.size()==input.size())for(size_t i=0;i<input.size();++i)output[i]|=input[i];
    }
    into.directional.insert(into.directional.end(),source.directional.begin(),source.directional.end());
}

bool transport_rule_at(const Request& request,const baked::Grids& grid,
                       const Cell& cell,TransportRules::Kind kind) {
    if(!request.transport_rules||request.transport_rules->submap!=request.submap||
        (kind==TransportRules::Gate&&!request.transport_gates)||
        !baked::in_bounds(grid,cell.x,cell.y))return false;
    const int layer=request.submap&&request.submap->world?0:cell.layer;
    if(layer<0||layer>=3)return false;
    const auto& plane=request.transport_rules->cells[kind][layer];
    const size_t index=static_cast<size_t>(cell.y)*grid.width+cell.x;
    const uint64_t ids=index<plane.size()?plane[index]:0;
    return ids&&(ids&transportation_bit(transportation_at(request,grid,cell)))!=0;
}
}
