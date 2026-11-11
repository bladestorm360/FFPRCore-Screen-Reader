#pragma once
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
// Owned worker rows only. Collapse native points at one floor/cell and remove
// proven co-located IV/V/VI companions; never group translated labels.
void consolidate(filter_worker::FilterInput& input);
}
