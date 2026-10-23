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
// Owned worker rows only. Collapse native points at one floor/cell and remove
// a proven co-located FF6 tutorial companion; never group translated labels.
void consolidate(filter_worker::FilterInput& input);
}
