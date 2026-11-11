#pragma once
#include "../json.h"
#include "puzzle_policy.h"
#include <array>
#include <optional>

namespace field::puzzle_guide {
// Read immutable recipe metadata, not routes, actors, party conditions or native
// script bodies. Called once after a real map entry reaches field readiness.
// Bounds are shared recipe limits; at most64 distinct single-bit observations
// are permitted. Missing selection state never advertises an unavailable guide.
template<class ReadFlag>
bool entry_guide_available(const json::Value& root,int map,bool module_available,ReadFlag read_flag,
                           puzzle_policy::Policy policy={}) {
    if(!policy.any())return false;
    std::array<int,64> indices{};std::array<std::optional<int>,64> values{};size_t used=0;
    const auto flag=[&](int index)->std::optional<int> {
        if(index<0||index>=1024)return {};
        for(size_t i=0;i<used;++i)if(indices[i]==index)return values[i];
        if(used==indices.size())return {};
        indices[used]=index;values[used]=read_flag(index);return values[used++];
    };
    const auto conditions=[&](const json::Value* list) {
        if(!list)return true;if(list->arr.size()>32)return false;
        for(const auto& row:list->arr) {
            if(row.arr.size()!=4||row.arr[0].s!=L"scenario")return false;
            const auto actual=flag(static_cast<int>(row.arr[1].n));if(!actual)return false;
            const int wanted=static_cast<int>(row.arr[3].n);const auto& op=row.arr[2].s;
            if(!(op==L"eq"?*actual==wanted:op==L"ne"?*actual!=wanted:
                 op==L"ge"?*actual>=wanted:op==L"lt"?*actual<wanted:false))return false;
        }
        return true;
    };
    const auto here=[&](const json::Value& row,bool steps) {
        if(const auto* maps=row.get(L"maps")) {
            if(maps->arr.size()>128)return false;
            for(const auto& id:maps->arr)if(id.n==map)return true;
        }
        if(steps)if(const auto* list=row.get(L"steps")) {
            if(list->arr.size()>32)return false;
            for(const auto& step:list->arr)if(const auto* id=step.get(L"map");id&&id->n==map)return true;
        }
        return false;
    };
    if(const auto* rows=root.get(L"puzzles");rows&&rows->arr.size()<=64)
        for(const auto& row:rows->arr) {
            if(!here(row,true))continue;
            const auto* id=row.get(L"id");
            if(!id||!puzzle_policy::any_enabled(root,id->s,policy))continue;
            if(const auto* end=row.get(L"done_scenario");end&&flag(static_cast<int>(end->n))==1)continue;
            if(const auto* start=row.get(L"requires_scenario");start&&flag(static_cast<int>(start->n))!=1)continue;
            return true;
        }
    bool story_here=false;
    if(const auto* rows=root.get(L"story_guides");rows&&rows->arr.size()<=48)
        for(const auto& row:rows->arr) {
            if(!here(row,false))continue;story_here=true;
            if(!conditions(row.get(L"select_when"))||!conditions(row.get(L"when")))continue;
            const auto* id=row.get(L"id");
            if(!id||!puzzle_policy::any_enabled(root,id->s,policy))continue;
            if(const auto* end=row.get(L"complete_scenario");end&&flag(static_cast<int>(end->n))==1)continue;
            return true;
        }
    // FFV's new rule leaf also has a map-only native availability query.
    // Once a story row owns this map,that broad fallback must not override
    // its completed/unknown/selection receipts or disabled puzzle type.
    if(story_here)return false;
    // FFIII forms and FFV typed policies already expose a bounded map-only
    // query. FFVI's broad registered-map query is intentionally not used: its
    // optional episodes share ordinary world/town maps and need selection bits.
    if(const auto* pyramid=root.get(L"pyramid_guide");pyramid&&here(*pyramid,false)) {
        const auto* end=pyramid->get(L"done_scenario");
        if(!end||flag(static_cast<int>(end->n))!=0)return false;
    }
    return module_available&&puzzle_policy::map_has_enabled_kind(root,map,policy);
}
}
