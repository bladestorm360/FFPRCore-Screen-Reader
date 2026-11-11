// Speech-only grouping. Physical route cells/native movement remain unchanged.
#pragma once
#include "baked_router.h"
namespace field::current_directions {
inline constexpr int ride=-2;
inline std::vector<baked_route::Leg> legs(const std::vector<baked_route::Cell>& cells,
    const baked::Grids* grid,int width,int height,bool (*cancelled)()) {
    if(!grid||(grid->currents[0].empty()&&grid->currents[1].empty()&&grid->currents[2].empty()))
        return baked_route::legs(cells,width,height,cancelled);
    std::vector<baked_route::Leg> result;
    size_t at=0;
    while(at+1<cells.size()) {
        if(cancelled&&cancelled())return {};
        const auto& c=cells[at];
        if(baked::current_bits(*grid,c.layer,c.x,c.y)&15) {
            int count=0;
            do{++count;++at;}while(at+1<cells.size()&&(baked::current_bits(*grid,cells[at].layer,cells[at].x,cells[at].y)&15));
            result.push_back({ride,count});
        } else {
            const auto begin=at++;
            while(at+1<cells.size()&&!(baked::current_bits(*grid,cells[at].layer,cells[at].x,cells[at].y)&15))++at;
            const std::vector<baked_route::Cell> run(cells.begin()+begin,cells.begin()+at+1);
            auto normal=baked_route::legs(run,width,height,cancelled);result.insert(result.end(),normal.begin(),normal.end());
        }
    }
    return result;
}
}
