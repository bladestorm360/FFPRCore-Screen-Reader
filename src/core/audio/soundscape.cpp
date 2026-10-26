#include "soundscape.h"
#include "../field/event_semantics.h"
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <unordered_set>
#include <map>
#include <tuple>
#include <numeric>
namespace sound::scape {
namespace {
struct Cell { int x,y,layer; bool operator==(const Cell&) const=default; };
struct Hash { size_t operator()(const Cell& c) const { return (size_t(uint32_t(c.x))*73856093u) ^ (size_t(uint32_t(c.y))*19349663u) ^ size_t(c.layer); } };
Cell cell(const Row& r) { return {int(std::floor(r.x)),int(std::floor(r.north)),r.layer}; }
bool coordinate(float value) { return std::isfinite(value) && std::abs(value)<=65536; }
}
float wrapped(float d,float width) { return width>0 ? d-std::round(d/width)*width : d; }
bool trap_hint(std::wstring hint) {
    return field::event_semantics::trap_identifier(hint);
}
std::vector<Emitter> classify(const Snapshot& s,const Options& o) {
    std::vector<Emitter> out;
    if(s.rows.size()>max_rows || !o.navigation || !o.soundscape || !coordinate(s.player_x) || !coordinate(s.player_north)
       || !coordinate(s.wrap_width) || !coordinate(s.wrap_height) || s.wrap_width<0 || s.wrap_height<0) return out;
    std::unordered_set<Cell,Hash> doors;
    for(const auto& row:s.rows) {
        if(!coordinate(row.x)||!coordinate(row.north)) continue;
        if(row.door && !trap_hint(row.hint)) doors.insert(cell(row));
    }
    auto adjacent=[&](const auto& cells,const Row& row) {
        const auto c=cell(row);
        for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x)
            if(cells.contains(Cell{c.x+x,c.y+y,c.layer})) return true;
        return false;
    };
    struct Candidate {size_t row;Emitter emitter;};std::vector<Candidate> candidates;
    for(size_t index=0;index<s.rows.size();++index) {
        const auto& r=s.rows[index];
        if(!r.id || !coordinate(r.x)||!coordinate(r.north) || r.event_type==L"ToLayer") continue;
        const float x=wrapped(r.x-s.player_x,s.wrap_width),n=wrapped(r.north-s.player_north,s.wrap_height);
        const float distance=std::hypot(x,n);
        std::optional<cues::Id> cue;
        switch(r.kind) {
        case Kind::Chest: if(!r.opened) cue=cues::Id::Chest; break;
        case Kind::NPC: cue=cues::Id::NPC; break;
        case Kind::Vehicle: cue=cues::Id::Vehicle; break;
        case Kind::SavePoint: cue=cues::Id::SavePoint; break;
        case Kind::Exit:
            if(trap_hint(r.hint)) cue=cues::Id::Trap;
            else if(r.verified_exit || r.destination>0) cue=(r.door || adjacent(doors,r))?cues::Id::Door:cues::Id::Exit;
            // A door guessed by the old scanner is not proof of a map exit.
            break;
        case Kind::Event: case Kind::Script:
            if(trap_hint(r.hint)) cue=cues::Id::Trap;
            else if(!r.door) cue=cues::Id::Event; // an adjacent exit owns the knock
            break;
        }
        if(!cue) continue;
        const size_t k=size_t(*cue)-cues::first_entity;
        if(!o.enabled[k]) continue;
        candidates.push_back({index,{r.id,*cue,x,n,distance,1,0}});
    }
    // Physical sound groups are independent from the pathfinder's list filters.
    // Components contain only adjacent tiles of the same transition/automatic
    // script identity. Manual objects/NPCs/chests retain individual identities.
    std::vector<size_t> roots(candidates.size());std::iota(roots.begin(),roots.end(),0);
    const auto root=[&](size_t n){while(roots[n]!=n){roots[n]=roots[roots[n]];n=roots[n];}return n;};
    using Identity=std::tuple<int,int,int,int,std::wstring>;
    using Tile=std::tuple<Identity,int,int>;
    std::map<Tile,size_t> tiles;std::map<Identity,size_t> compass;
    for(size_t i=0;i<candidates.size();++i) {
        const auto& c=candidates[i];const auto& row=s.rows[c.row];
        std::optional<Identity> identity;
        if(row.kind==Kind::Exit&&row.destination>0) {
            int direction=-1;
            if(row.world_exit && s.wrap_width==0 && s.map_width>0 && s.map_height>0) {
                const float edges[]={s.map_height*.5f-row.north,s.map_width*.5f-row.x,
                                     s.map_height*.5f+row.north,s.map_width*.5f+row.x};
                direction=int(std::min_element(std::begin(edges),std::end(edges))-std::begin(edges));
            }
            identity=Identity{int(c.emitter.cue),row.destination,row.layer,direction,L""};
            if(direction>=0) {
                const auto [at,inserted]=compass.emplace(*identity,i);
                if(!inserted)roots[root(i)]=root(at->second);
                continue; // one world-exit source per fixed town compass side
            }
        } else if((row.kind==Kind::Script||(row.kind==Kind::Event&&row.event_type==L"Event"))&&row.script>0)
            identity=Identity{int(c.emitter.cue),-row.script,row.layer,row.action,L""};
        if(!identity)continue;
        auto at=cell(row);
        auto normalize=[](int n,float span){const int w=int(span);return w>0?(n%w+w)%w:n;};
        at.x=normalize(at.x,s.wrap_width);at.y=normalize(at.y,s.wrap_height);
        for(const auto offset:{std::pair{0,0},std::pair{-1,0},std::pair{1,0},std::pair{0,-1},std::pair{0,1}}) {
            const auto found=tiles.find(Tile{*identity,normalize(at.x+offset.first,s.wrap_width),normalize(at.y+offset.second,s.wrap_height)});
            if(found!=tiles.end())roots[root(i)]=root(found->second);
        }
        tiles.try_emplace(Tile{*identity,at.x,at.y},i);
    }
    struct Group {size_t nearest;uint64_t id;};std::map<size_t,Group> groups;
    for(size_t i=0;i<candidates.size();++i) {
        const auto& e=candidates[i].emitter;auto [at,inserted]=groups.emplace(root(i),Group{i,e.id});
        if(!inserted) {
            at->second.id=std::min(at->second.id,e.id);
            if(e.distance<candidates[at->second.nearest].emitter.distance)at->second.nearest=i;
        }
    }
    std::array<size_t,cues::entity_count> ranks{};
    for(const auto& [key,group]:groups) {
        auto e=candidates[group.nearest].emitter;
        if(e.distance>std::clamp(o.range,5,25))continue;
        e.id=group.id;const size_t k=size_t(e.cue)-cues::first_entity;
        const size_t rank=ranks[k]++;
        const int slot=int(rank%13);
        const float detune=slot==0?1.f:1.f+(slot%2?1.f:-1.f)*float((slot+1)/2)*.05f;
        e.rank=rank;e.detune=detune;out.push_back(e);
    }
    return out;
}
double cadence(const Emitter& e,double clip_seconds,float pitch) {
    const bool near=e.cue==cues::Id::Trap && e.distance<=2;
    return std::max(near?.60:1.0,clip_seconds/pitch+(near?.20:.35))+double(e.rank%13)*.10;
}
}
