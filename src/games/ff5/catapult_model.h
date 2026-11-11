#pragma once
#include "library_model.h"

namespace ff5::catapult {
// Both FFV leaves use the established owned guide-plan result. One native
// controller owns observation, cancellation, route publication and previews.
using Plan=library::Plan;
inline constexpr std::array<int,5> flags{309,310,311,312,313};
struct Snapshot {
 int map=0;
 field::baked_route::Cell player;
 std::array<std::optional<int>,5> values;
 std::array<std::optional<bool>,5> hidden;
 std::wstring locale;
 std::function<bool()> cancelled;
};
bool supported(int map);
std::span<const library::Control> controls(int map);
Plan plan(const Snapshot&,const json::Value& puzzles);
}
