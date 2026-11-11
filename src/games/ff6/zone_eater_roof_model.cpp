// Exact safe areas and primary-data route are authored, not inferred from names.
// The worker uses the ordinary baked router for recovery from an unexpected
// position. There are34 safe cells, nine fixed forward stations and three
// physical chests; no native call or pointer is used here.
#include "zone_eater_roof.h"
#include "../../core/modtext.h"
#include <algorithm>

namespace ffpr::ff6::story {
namespace {
bool cells(const json::Value* rows,std::vector<field::baked_route::Cell>& out,size_t count,int width,int height) {
    if(!rows||rows->arr.size()!=count)return false;
    for(const auto& row:rows->arr) {
        if(row.arr.size()!=3)return false;
        for(const auto& v:row.arr)if(v.type!=json::Value::Type::Number||!(v.n>=0&&v.n<=128)||v.n!=static_cast<int>(v.n))return false;
        if(row.arr[0].n!=1||row.arr[1].n>=width||row.arr[2].n>=height)return false;
        field::baked_route::Cell c{1,static_cast<int>(row.arr[1].n),static_cast<int>(row.arr[2].n)};
        if(std::find(out.begin(),out.end(),c)!=out.end())return false;
        out.push_back(c);
    }
    return true;
}
}
Plan roof_plan(const Snapshot& s,const json::Value& guide) {
    using namespace field;using baked_route::Cell;
    Plan out;out.map=s.map;out.supported=true;out.roof_start=s.player;
    const auto waiting=[&] {out.unknown=true;out.note=T(L"Story guidance is waiting for current game state.");return out;};
    if(s.map!=504||s.player.layer!=1)return waiting();
    const auto* map=baked::submap_by_map_id(504);if(!map||map->width<=0||map->height<=0||map->width>128||map->height>128)return waiting();
    std::vector<Cell> safe,stations;
    if(!cells(guide.get(L"safe_cells"),safe,34,map->width,map->height)||!cells(guide.get(L"stations"),stations,9,map->width,map->height))return waiting();
    for(const auto& station:stations)if(std::find(safe.begin(),safe.end(),station)==safe.end())return waiting();
    if(s.cancelled&&s.cancelled())return waiting();
    const bool protected_cell=std::find(safe.begin(),safe.end(),s.player)!=safe.end();
    Cell destination;
    const auto station=std::find(stations.begin(),stations.end(),s.player);
    if(station!=stations.end()) {
        const size_t index=static_cast<size_t>(station-stations.begin());
        if(index+1==stations.size()) {
            destination={1,9,20};out.finish_on_exit=505;
        } else {destination=stations[index+1];out.roof_timed=true;}
    } else {
        // A user can resume at any other safe marker or ask for recovery while
        // between them. One multi-goal search finds a real reachable destination;
        // Manhattan proximity cannot route through a chest or a rock column.
        baked_route::Request request;request.submap=map;request.start=s.player;
        request.goals=protected_cell?stations:safe;request.goals_open=false;
        request.blocked={{1,29,13},{1,24,9},{1,10,10},{1,37,19},{1,9,20}};
        request.live_hazards=true;request.live_traps=true;request.cancelled=s.cancelled;
        const auto route=baked_route::find(request);
        if(!route.found||route.cells.empty())return waiting();
        destination=route.cells.back();out.roof_timed=protected_cell;
    }
    baked_nav::GuideTarget next;next.goal.map_id=504;next.goal.cell=destination;
    next.kind=TargetKind::Spot;
    next.instruction=out.finish_on_exit?T(L"Walk through the exit."):
        protected_cell?T(L"Stop in the next safe area, then request the guide again."):
        T(L"Reach this safe area immediately, then request the guide again.");
    out.targets.push_back(std::move(next));
    return out;
}
}
