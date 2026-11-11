#pragma once
#include "library_model.h"
namespace ff5::book {
using Plan=library::Plan;
inline constexpr std::array<int,6> flags{521,522,523,524,525,526};
struct Snapshot {int map=0;std::array<std::optional<int>,6> values;std::array<std::optional<bool>,3> hidden;std::optional<bool> gate_hidden;std::wstring locale;std::function<bool()> cancelled;};
bool supported(int map);
std::span<const library::Control> pickups();
Plan plan(const Snapshot&,const json::Value& puzzles);
}
