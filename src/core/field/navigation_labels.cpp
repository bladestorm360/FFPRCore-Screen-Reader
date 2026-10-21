#include "navigation_labels.h"
#include "entity_filter.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <tuple>

namespace field::filter_worker {
namespace {
bool cancelled(const FilterInput& input) { return input.cancelled && input.cancelled(); }
std::wstring letters(size_t index) {
    std::wstring result;
    do { result.insert(result.begin(),static_cast<wchar_t>(L'a'+index%26)); index=index/26; if(!index)break; --index; } while(true);
    return result;
}
struct Floor {
    const baked::Submap* map = nullptr;
    std::map<int,std::vector<const baked::Object*>> events;
    std::vector<const baked::Object*> exits;
    std::vector<size_t> roots;
    std::map<size_t,std::wstring> suffixes;
};
bool prepare_floor(Floor& floor, FilterInput& input) {
    const auto& map=*floor.map;
    for(const auto& object:baked::grids(map).objects) {
        if(cancelled(input))return false;
        if ((object.type==baked::ObjType::Event || object.type==baked::ObjType::RandomEvent) && (object.a || object.b))
            floor.events[object.entity_id()].push_back(&object);
        if (!input.entrance_suffixes || object.type!=baked::ObjType::GotoMap || object.a<=0)continue;
        if (input.state && !input.state->active(map,object))continue;
        floor.exits.push_back(&object);
    }
    auto& roots=floor.roots;roots.resize(floor.exits.size());std::iota(roots.begin(),roots.end(),0);
    const auto root=[&](size_t n){while(roots[n]!=n){roots[n]=roots[roots[n]];n=roots[n];}return n;};
    // One physical doorway may consist of several adjacent GotoMap tiles. Join
    // only tiles with the same destination PointIn, on the same source layer.
    // Separate doors to that floor keep different letters, even if both arrive
    // at the same PointIn. All active baked doors participate before list filters.
    using Tile=std::tuple<int,int,int,int,int>; // layer, destination, point, x, y
    std::map<Tile,size_t> tiles;
    for(size_t i=0;i<floor.exits.size();++i) {
        const auto& o=*floor.exits[i];
        for(int y=std::max(0,int(o.ty));y<std::min(int(map.height),o.ty+o.th);++y)
            for(int x=std::max(0,int(o.tx));x<std::min(int(map.width),o.tx+o.tw);++x) {
                if(cancelled(input))return false;
                const auto key=[&](int a,int b){return Tile{o.layer,o.a,o.b,a,b};};
                for(const auto& offset:{std::pair{0,0},std::pair{-1,0},std::pair{1,0},std::pair{0,-1},std::pair{0,1}}) {
                    const auto at=tiles.find(key(x+offset.first,y+offset.second));
                    if(at!=tiles.end())roots[root(i)]=root(at->second);
                }
                tiles.emplace(key(x,y),i);
            }
    }
    using Anchor=std::tuple<int,int,int,int>; // layer, y, x, destination point
    std::map<int,std::map<size_t,Anchor>> destinations;
    for(size_t i=0;i<floor.exits.size();++i) {
        if(cancelled(input))return false;
        const auto& o=*floor.exits[i];roots[i]=root(i);
        auto& group=destinations[o.a];const Anchor anchor{o.layer,o.ty,o.tx,o.b};
        const auto at=group.find(roots[i]);
        if(at==group.end())group.emplace(roots[i],anchor);else at->second=std::min(at->second,anchor);
    }
    for(const auto& [destination,groups]:destinations) {
        if(groups.size()<2)continue;
        std::vector<std::pair<Anchor,size_t>> ordered;
        for(const auto& [id,anchor]:groups)ordered.emplace_back(anchor,id);
        std::sort(ordered.begin(),ordered.end());
        for(size_t i=0;i<ordered.size();++i)floor.suffixes.emplace(ordered[i].second,L" ("+letters(i)+L")");
    }
    return true;
}
}
bool prepare_navigation_labels(FilterInput& input) {
    if(!input.events && !input.entrance_suffixes)return true;
    std::map<int,Floor> floors;
    for(auto& row:input.rows) {
        if(cancelled(input))return false;
        row.event_group.clear();
        if(!row.available || (row.kind!=Kind::Event && row.kind!=Kind::MapExit))continue;
        const int id=row.remote_map?row.remote_map:input.current_map;
        auto found=floors.find(id);
        if(found==floors.end()) {
            Floor floor;floor.map=baked::submap_by_map_id(id);
            if(floor.map && !prepare_floor(floor,input))return false;
            found=floors.emplace(id,std::move(floor)).first;
        }
        const auto& floor=found->second;if(!floor.map)continue;
        int x=row.cell_x,y=row.cell_y;
        if(!row.remote_map) {
            if(!std::isfinite(row.position.x)||!std::isfinite(row.position.y)||std::abs(row.position.x)>1.e6||std::abs(row.position.y)>1.e6)continue;
            x=static_cast<int>(std::floor(floor.map->width*.5+row.position.x/16));
            y=static_cast<int>(std::floor(floor.map->height*.5-row.position.y/16));
        }
        if(input.events && row.kind==Kind::Event && row.event_type==L"Event" && row.entity_id>=0 && !row.raw_name.empty()) {
            const auto at=floor.events.find(row.entity_id);
            if(at==floor.events.end())continue;
            bool ambiguous=false;
            for(const auto* o:at->second) {
                if(!o->contains(x,y)||(row.cell_layer>=0 && o->layer!=row.cell_layer)||baked::object_identifier(*o)!=row.raw_name)continue;
                // Live rows prove their own inclusion. Exact bake identity is
                // required to group; unknown or ambiguous variants stay separate.
                const auto key=std::to_wstring(id)+L":"+std::to_wstring(o->layer)+L":"+baked::str(o->group)+L":"+
                    std::to_wstring(static_cast<int>(o->type))+L":"+std::to_wstring(o->a)+L":"+std::to_wstring(o->b)+L":"+row.raw_name;
                if(!row.event_group.empty() && row.event_group!=key)ambiguous=true;
                row.event_group=key;
            }
            if(ambiguous)row.event_group.clear();
        }
        if(input.entrance_suffixes && row.kind==Kind::MapExit) {
            std::wstring suffix;bool ambiguous=false,matched=false;
            for(size_t i=0;i<floor.exits.size();++i) {
                const auto& o=*floor.exits[i];if(o.a!=row.destination||!o.contains(x,y)||(row.cell_layer>=0 && o.layer!=row.cell_layer))continue;
                const auto at=floor.suffixes.find(floor.roots[i]);
                const auto value=at==floor.suffixes.end()?std::wstring():at->second;
                if(matched&&suffix!=value)ambiguous=true;
                suffix=value;matched=true;
            }
            if(!ambiguous)row.name+=suffix;
        }
    }
    return true;
}
}
