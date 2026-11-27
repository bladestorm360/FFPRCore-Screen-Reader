#pragma once
#include "library_model.h"
#include "mechanics_maps.h"
#include "../../core/field/map_state.h"
#include "../../core/field/puzzle_policy.h"
// New FFV rule leaf. Existing specialized models retain their designs. Only
// owned native-ready flags/settings/cells enter this bounded worker selector.
namespace ff5::mechanics {
struct Snapshot {
 int map=0;field::baked_route::Cell player;field::map_state::Flags flags;
 field::puzzle_policy::Policy policy;
 std::wstring locale;std::function<bool()> cancelled;
 std::wstring guide_id;
 // Read only at the moving floor's center. A reload can retain layout zero
 // without replaying the scene that made the stop contacts visible.
 std::optional<bool> exdeath_stop_visible;
 std::optional<int> karnak_seconds;
};
inline bool supported(int map){return types(map)!=0;}
library::Plan plan(const Snapshot&,const json::Value&);
}
