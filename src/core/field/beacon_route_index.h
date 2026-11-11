#pragma once
// Worker-built lookup for every cell of one immutable published route. Moving
// within that route must not queue another corner job or require an idle frame.
#include "baked_nav.h"
#include "route_progress.h"
#include <algorithm>
#include <cmath>
#include <tuple>

namespace field::baked_nav {
struct BeaconCornerEntry {
    baked_route::Cell cell;
    Corner corner;
    size_t occurrence=0; // reverse traversal: first occurrence is native remaining()'s choice
};
inline auto beacon_cell_key(const baked_route::Cell& c) { return std::tuple{c.layer,c.x,c.y}; }
struct BeaconCornerIndex {
    std::vector<BeaconCornerEntry> entries;
    struct PrecisionEntry {route_progress::PrecisionPoint from,to;Corner corner;};
    std::vector<PrecisionEntry> precision; // bounded authored passage, seven points
    const Corner* find(const baked_route::Cell& cell) const {
        const auto key=beacon_cell_key(cell);
        const auto it=std::lower_bound(entries.begin(),entries.end(),key,
            [](const BeaconCornerEntry& entry,const auto& wanted){return beacon_cell_key(entry.cell)<wanted;});
        return it!=entries.end() && it->cell==cell ? &it->corner : nullptr;
    }
    const Corner* find(const baked_route::Cell& cell,double x2,double y2) const {
        // Different cardinal corners can share a rounded cell. Use the same
        // actual copied position as the follower, never skip a required turn.
        for(const auto& p:precision)if(p.from.layer==cell.layer&&std::abs(x2-p.from.x2)<0.03&&std::abs(y2-p.from.y2)<0.03)return &p.corner;
        for(const auto& p:precision)if(p.from.layer==cell.layer){
            const double dx=p.to.x2-p.from.x2,dy=p.to.y2-p.from.y2;
            const double t=((x2-p.from.x2)*dx+(y2-p.from.y2)*dy)/(dx*dx+dy*dy);
            if(t>0&&t<1&&std::abs(x2-p.from.x2-t*dx)<0.03&&std::abs(y2-p.from.y2-t*dy)<0.03)return &p.corner;
        }
        return find(cell);
    }
};
template<class Position> BeaconCornerIndex build_beacon_corner_index(const baked::Submap& sm,
    const std::vector<route_progress::Phase>& phases,bool partial,Position world_of_cell,
    const std::function<bool()>& cancelled={}) {
    BeaconCornerIndex out;
    size_t count=0;for(const auto& phase:phases)count+=phase.cells.size();
    out.entries.reserve(count); // one contiguous allocation, never one allocation per tile
    int later_steps=0;
    std::optional<baked_route::Cell> next_phase;
    for(size_t pi=phases.size();pi-->0;) {
        const auto& phase=phases[pi];int first_dir=-1,run=0;
        if(!phase.precision.empty()) {
            int half_left=0;const auto zero=world_of_cell(sm,baked_route::Cell{phase.precision.front().layer,0,0});
            for(size_t i=phase.precision.size();i-->0;) {
                const auto point=phase.precision[i];Corner corner;corner.found=true;
                corner.steps_left=later_steps+(half_left+1)/2;corner.arrived=!partial&&corner.steps_left==0;
                auto next=point;
                if(i+1<phase.precision.size()) {next=phase.precision[i+1];corner.steps_to_corner=1;}
                else if(next_phase){next={next_phase->layer,next_phase->x*2,next_phase->y*2};corner.steps_to_corner=1;}
                corner.point={zero.x+next.x2*8.f,zero.y-next.y2*8.f,zero.z};
                const baked_route::Cell cell{point.layer,(point.x2+1)/2,(point.y2+1)/2};
                out.entries.push_back({cell,corner,out.entries.size()});
                if(i+1<phase.precision.size())out.precision.push_back({point,next,corner});
                if(i)half_left+=std::max(std::abs(point.x2-phase.precision[i-1].x2),std::abs(point.y2-phase.precision[i-1].y2));
            }
            next_phase=phase.cells.front();later_steps+=route_progress::steps(phase);continue;
        }
        for(size_t i=phase.cells.size();i-->0;) {
            if(cancelled && cancelled())return {};
            const auto at=phase.cells[i];Corner corner;corner.found=true;
            corner.steps_left=later_steps+int(phase.cells.size()-1-i)+phase.manual_displacement_steps;
            corner.arrived=corner.steps_left==0&&!partial;
            corner.point=world_of_cell(sm,at);
            if(i+1<phase.cells.size()) {
                const int dir=baked_route::step_direction(at,phase.cells[i+1],baked::wrap_width(sm),baked::wrap_height(sm));
                // Match legs(): non-movement entries do not introduce a direction.
                if(dir>=0){run=dir==first_dir?run+1:1;first_dir=dir;}
                corner.steps_to_corner=run?run:int(phase.cells.size()-1-i);
                corner.point=world_of_cell(sm,phase.cells[i+size_t(corner.steps_to_corner)]);
                if((baked::wrap_width(sm)||baked::wrap_height(sm))&&run) {
                    corner.point=world_of_cell(sm,at);const float length=run*16.f;
                    if(first_dir==0||first_dir==4||first_dir==7)corner.point.y+=length;
                    if(first_dir==2||first_dir==5||first_dir==6)corner.point.y-=length;
                    if(first_dir==1||first_dir==4||first_dir==5)corner.point.x+=length;
                    if(first_dir==3||first_dir==6||first_dir==7)corner.point.x-=length;
                }
            } else if(next_phase&&!phase.manual_after) {
                corner.point=world_of_cell(sm,*next_phase);corner.steps_to_corner=1;
            }
            // A patrol endpoint is a temporary stop, never completed arrival.
            if(!phases.empty()&&phases.back().wait_for_npc&&!phases.back().cells.empty()&&at==phases.back().cells.back())corner.found=false;
            out.entries.push_back({at,corner,out.entries.size()});
        }
        if(!phase.cells.empty()) {
            next_phase=phase.cells.front();later_steps+=route_progress::steps(phase);
        }
    }
    // remaining() picks the final matching occurrence, including revisited cells
    // across vehicle/layer phases. Preserve that policy instead of inventing progress.
    std::sort(out.entries.begin(),out.entries.end(),[](const auto& a,const auto& b){
        const auto ka=beacon_cell_key(a.cell),kb=beacon_cell_key(b.cell);
        return ka==kb?a.occurrence<b.occurrence:ka<kb;
    });
    if(cancelled&&cancelled())return {};
    out.entries.erase(std::unique(out.entries.begin(),out.entries.end(),
        [](const auto& a,const auto& b){return a.cell==b.cell;}),out.entries.end());
    return out;
}
}
