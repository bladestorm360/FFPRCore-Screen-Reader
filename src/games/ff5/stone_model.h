#pragma once
#include "library_model.h"
namespace ff5::stone {
using Plan=library::Plan;
inline constexpr std::array<int,7> flags{174,175,176,177,178,179,294};
struct Snapshot {int map=0;std::array<std::optional<int>,7> values;std::optional<bool> stairs_hidden;std::wstring locale;std::function<bool()> cancelled;};
bool supported(int map);
Plan plan(const Snapshot&,const json::Value& puzzles);
}
