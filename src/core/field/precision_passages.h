#pragma once
#include "route_progress.h"
#include <array>
#include <cmath>

// Exact, immutable half-tile passage geometry authored into the bake. Ordinary
// collision/trap grids keep their meaning; only these reviewed segments can use
// finer movement. No native calls, global cache or request-time graph expansion.
namespace field::precision_passages {
using Point=route_progress::PrecisionPoint;
using Cell=baked_route::Cell;
using Passage=std::array<Point,7>;
inline std::optional<Passage> read(const baked::Submap& map) {
    if(baked::game_number()!=1||baked::name_of(map)!=L"Map_30061_4")return {};
    Passage result{};unsigned seen=0;
    for(const auto& object:baked::grids(map).objects)if(object.type==baked::ObjType::PrecisionPoint) {
        if(object.c!=-100001||object.d<0||object.d>6||(seen&(1u<<object.d))||object.layer!=0)return {};
        result[size_t(object.d)]={object.layer,object.a,object.b};seen|=1u<<object.d;
    }
    // East first avoids the pit immediately north of the lower entrance.
    // Every edge is a separate cardinal half step, including the east run.
    const Passage expected{{{0,36,36},{0,37,36},{0,37,35},{0,38,35},{0,39,35},{0,40,35},{0,40,34}}};
    return seen==127&&result==expected?std::optional<Passage>(result):std::nullopt;
}
inline int steps(const Passage& path){int halves=0;for(size_t i=1;i<path.size();++i)halves+=std::abs(path[i].x2-path[i-1].x2)+std::abs(path[i].y2-path[i-1].y2);return(halves+1)/2;}
inline Cell cell(const Point& point){return {point.layer,point.x2/2,point.y2/2};}
inline bool segment(const Point& from,const Point& to,const Passage& path) {
    for(size_t i=1;i<path.size();++i)if((from==path[i-1]&&to==path[i])||(from==path[i]&&to==path[i-1]))return true;
    return false;
}
inline bool clear(const baked_route::Request& movement,const Passage& path) {
    if(!movement.submap||movement.submap->world||movement.vehicle_group)return false;
    const auto& grid=baked::grids(*movement.submap);
    // The whole passage's six-tile bounding box is ordinary full-square floor.
    // Real pit triggers are not solids; all copied gates/NPCs/chests/colliders
    // and partial directional barriers retain their veto, including all-layer.
    for(int y=17;y<=18;++y)for(int x=18;x<=20;++x) {
        if(!baked::in_bounds(grid,x,y)||grid.collision[0].empty()||grid.collision[0][size_t(y)*grid.width+x]!=15)return false;
        const Cell at{0,x,y};
        for(const auto& block:movement.blocked)if(block.x==x&&block.y==y&&(block.layer<0||block.layer==0))return false;
        for(const auto& gate:movement.directional)if(gate.cell.x==x&&gate.cell.y==y&&(gate.cell.layer<0||gate.cell.layer==0))return false;
        for(const auto& object:grid.objects)if(object.type==baked::ObjType::GotoMap&&object.layer==0&&object.contains(x,y))return false;
    }
    // Native ordinary FootEvent dispatch requires matching player and event
    // positions, not merely a shared coarse tile. Check every segment against
    // every captured active center; never grant a general trap-grid override.
    const auto crosses=[&](const Cell& event) {
        if(event.layer>0)return false;
        for(size_t i=1;i<path.size();++i) {
            const double ax=path[i-1].x2,ay=path[i-1].y2,dx=path[i].x2-ax,dy=path[i].y2-ay;
            const double t=std::clamp(((event.x*2-ax)*dx+(event.y*2-ay)*dy)/(dx*dx+dy*dy),0.,1.);
            if(std::abs(ax+t*dx-event.x*2)<0.001&&std::abs(ay+t*dy-event.y*2)<0.001)return true;
        }return false;
    };
    for(const auto& event:movement.traps)if(crosses(event))return false;
    for(const auto& event:movement.hazards)if(crosses(event))return false;
    return true;
}
}
