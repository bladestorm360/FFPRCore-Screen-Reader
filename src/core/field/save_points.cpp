#include "save_points.h"
#include "entity_filter.h"
#include <cmath>
#include <map>
#include <tuple>

namespace field::save_points {
void consolidate(filter_worker::FilterInput& input) {
    using Key=std::tuple<int,int,int,int>;
    std::map<Key,size_t> points;
    const auto key=[&](const filter_worker::FilterRow& row,Key& out) {
        const int id=row.remote_map?row.remote_map:input.current_map;
        const auto* sm=baked::submap_by_map_id(id);if(!sm)return false;
        int x=row.cell_x,y=row.cell_y;
        if(!row.remote_map) {
            if(!std::isfinite(row.position.x)||!std::isfinite(row.position.y)||std::abs(row.position.x)>1.e6||std::abs(row.position.y)>1.e6)return false;
            x=static_cast<int>(std::floor(sm->width*.5+row.position.x/16));
            y=static_cast<int>(std::floor(sm->height*.5-row.position.y/16));
        }
        if(row.cell_layer<0 || !baked::in_bounds(baked::grids(*sm),x,y))return false;
        out={baked::submap_index(sm),row.cell_layer,x,y};return true;
    };
    for(size_t i=0;i<input.rows.size();++i) {
        if(input.cancelled&&input.cancelled())return;
        auto& row=input.rows[i];if(!row.available||row.kind!=Kind::SavePoint)continue;
        Key at;if(!key(row,at))continue;
        // Stable source order; a selection never creates a second destination.
        if(!points.emplace(at,i).second)row.available=false;
    }
    if(baked::game_number()!=6)return;
    for(auto& row:input.rows) {
        if(input.cancelled&&input.cancelled())return;
        if(!row.available||row.kind!=Kind::Event||!tutorial(row.raw_name))continue;
        Key at;if(!key(row,at))continue;
        const auto point=points.find(at);if(point==points.end())continue;
        const auto* sm=baked::submap_at(std::get<0>(at));
        const auto& canonical=input.rows[point->second];
        // Exact native identity + script/action + shared physical tile. The
        // separate "to save-point room" scene (script 619) must stay an event.
        for(const auto& object:baked::grids(*sm).objects) {
            if(object.type!=baked::ObjType::Event || object.entity_id()!=row.entity_id ||
               baked::object_identifier(object)!=row.raw_name || object.a!=20 || object.b!=2 ||
               object.layer!=row.cell_layer || !object.contains(std::get<2>(at),std::get<3>(at)))continue;
            for(const auto& native:baked::grids(*sm).objects)
                if(native.type==baked::ObjType::SavePoint && native.entity_id()==canonical.entity_id &&
                   native.group==object.group && native.layer==object.layer && native.contains(object.tx,object.ty)) {
                    row.available=false;break;
                }
            if(!row.available)break;
        }
    }
}
}
