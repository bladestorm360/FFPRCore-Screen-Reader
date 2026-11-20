#pragma once
#include "baked_map.h"
#include <algorithm>
#include <string>

namespace field::filter_worker { struct FilterInput; }
namespace field::save_points {
// FF6's asset-256 decorations have these two exact developer identities. They
// have no action/script; neither is the native ObjectType 9 save destination.
inline bool animation(const std::wstring& raw) {
    return raw == L"savepoint" || raw == L"セーブポイント_エフェクト";
}
inline bool tutorial(const std::wstring& raw) {
    return raw == L"セーブポイントについての説明を受けるかを問われる";
}
// A capture hint only. Worker consolidation separately proves the exact baked
// script/action, identity, group and co-located native ObjectType9 point.
inline bool companion(int game,const std::wstring& raw) {
    if(game==4)return raw==L"セーブポイント";
    if(game==5)return raw==L"セーブポイント（初）"||raw==L"セーブポイント(初)";
    return game==6&&tutorial(raw);
}
inline bool tutorial_script(const baked::Object& event, int game) {
    if (event.type != baked::ObjType::Event || event.b != 2) return false;
    const bool script_matches = (game == 4 && (event.a == 853 || event.a == 855)) ||
                                (game == 5 && event.a == 2006) || (game == 6 && event.a == 20);
    return script_matches && companion(game, baked::object_identifier(event));
}

inline bool passable_tutorial(const baked::Grids& grid, const baked::Object& event, int game) {
    if (!tutorial_script(event, game)) return false;
    // A first-use message can pause native control, but stepping across this
    // save circle is ordinary travel. The action gate still owns the pause.
    return std::any_of(grid.objects.begin(), grid.objects.end(), [&](const auto& point) {
        return point.type == baked::ObjType::SavePoint && point.layer == event.layer &&
               point.contains(event.tx, event.ty);
    });
}
// Owned worker rows only. Collapse native points at one floor/cell and remove
// proven co-located IV/V/VI companions; never group translated labels.
void consolidate(filter_worker::FilterInput& input);
}
