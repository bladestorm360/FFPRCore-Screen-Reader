// Original route facts are qualified against both native callback phases and
// both retained states of the other bridge. Only owned recipe/position/flags
// are read here, on the worker; no engine handle or clock guess enters a plan.
#include "sealed_gate_model.h"
#include "../../core/modtext.h"
#include <algorithm>
#include <cmath>
namespace ffpr::ff6::story {
namespace {
using BridgeCell=field::baked_route::Cell;
bool bridge_point(const json::Value& row,BridgeCell& at) {
    if(row.arr.size()!=2)return false;
    for(const auto& v:row.arr)if(v.type!=json::Value::Type::Number||v.n<0||v.n>26||std::floor(v.n)!=v.n)return false;
    at={1,static_cast<int>(row.arr[0].n),static_cast<int>(row.arr[1].n)};
    return at.y<20;
}
bool bridge_leg(const json::Value& row,int mode,Plan::BridgeCrossing& out) {
    const auto* phase=row.get(L"phase"),*cells=row.get(L"cells");
    if(!phase||phase->type!=json::Value::Type::Number||phase->n<-1||phase->n>1||std::floor(phase->n)!=phase->n||
        !cells||cells->arr.size()<2||cells->arr.size()>7)return false;
    out.mode=mode;out.phase=static_cast<int>(phase->n);
    for(const auto& point:cells->arr){BridgeCell at;if(!bridge_point(point,at))return false;out.cells.push_back(at);}
    for(size_t i=1;i<out.cells.size();++i)if(std::abs(out.cells[i].x-out.cells[i-1].x)+std::abs(out.cells[i].y-out.cells[i-1].y)!=1)return false;
    return true;
}
std::wstring bridge_directions(const std::vector<BridgeCell>& cells) {
    static constexpr const wchar_t* words[]{L"North",L"East",L"South",L"West"};
    std::wstring line;
    for(const auto leg:field::baked_route::legs(cells)){
        if(leg.dir<0||leg.dir>3)return {};
        if(!line.empty())line+=L", ";line+=std::to_wstring(leg.count)+L" "+T(words[leg.dir]);
    }
    return line;
}
void bridge_target(Plan& out,BridgeCell at,int entity=-1,bool exit=false) {
    field::baked_nav::GuideTarget target;target.goal.map_id=407;target.goal.cell=at;target.goal.entity_id=entity;
    target.goal.avoid_other_events=true;target.goal.allow_scripted_crossings=true;
    target.kind=exit?field::TargetKind::Transition:field::TargetKind::Spot;
    target.instruction=T(exit?L"Continue through the passage.":L"Stop in the next safe area, then request the guide again.");
    out.targets.push_back(std::move(target));
}
}
Plan sealed_gate_bridge_plan(const Snapshot& state,const json::Value& guide) {
    modtext::LocaleScope locale(state.locale);Plan out;out.map=state.map;out.supported=true;
    const auto waiting=[&]{out.unknown=true;out.note=T(L"Story guidance is waiting for current game state.");return out;};
    if(state.map!=407||state.player.layer!=1||!state.flags.local[1]||state.cancelled&&state.cancelled())return waiting();
    const int mode=*state.flags.local[1];if(mode<0||mode>2)return waiting();
    if(state.player.x>=17&&state.player.y>=13){bridge_target(out,{1,18,15},3,true);return out;}
    if(mode==0){bridge_target(out,{1,8,4},12);return out;}
    const auto* bridges=guide.get(L"bridges");const auto* bridge=bridges?bridges->get(std::to_wstring(mode)):nullptr;
    const auto* legs=bridge?bridge->get(L"legs"):nullptr;const auto* recoveries=bridge?bridge->get(L"recoveries"):nullptr;
    if(!legs||legs->arr.size()!=4||!recoveries||recoveries->arr.size()>128)return waiting();
    std::vector<Plan::BridgeCrossing> routes;
    for(const auto& row:legs->arr){Plan::BridgeCrossing leg;if(!bridge_leg(row,mode,leg))return waiting();routes.push_back(std::move(leg));}
    for(size_t i=1;i<routes.size();++i)if(!(routes[i-1].cells.back()==routes[i].cells.front()))return waiting();
    if(state.player==routes.back().cells.back()){
        if(mode==1)bridge_target(out,{1,16,5},9);else bridge_target(out,{1,18,15},3,true);
        return out;
    }
    const auto advise=[&](Plan::BridgeCrossing leg){
        const auto directions=bridge_directions(leg.cells);if(directions.empty())return waiting();
        if(leg.phase<0){out.note=directions+L". "+T(L"Stop in the next safe area, then request the guide again.");return out;}
        out.note=leg.immediate?directions+L". "+T(L"Reach this safe area immediately, then request the guide again."):
            TF(L"Next safe area: {0}. Request the guide again when ready, then wait for Go now.",{directions});
        out.bridge=std::move(leg);return out;
    };
    for(const auto& leg:routes)if(state.player==leg.cells.front())return advise(leg);
    for(const auto& row:recoveries->arr){
        Plan::BridgeCrossing leg;if(!bridge_leg(row,mode,leg))return waiting();
        if(state.player==leg.cells.front())return advise(std::move(leg));
    }
    // A repeated request during an already-started crossing must not ask the
    // player to stand still on a temporary tile. Retain only the short suffix
    // and its current native phase; publication rechecks that phase/position.
    const auto phase=state.flags.local[mode==1?2:3];
    if(phase)for(auto leg:routes){
        const auto at=std::find(leg.cells.begin(),leg.cells.end(),state.player);
        if(at==leg.cells.end()||at==leg.cells.begin()||at+1==leg.cells.end()||*phase!=leg.phase)continue;
        leg.cells.erase(leg.cells.begin(),at);leg.immediate=true;return advise(std::move(leg));
    }
    return waiting();
}
}
