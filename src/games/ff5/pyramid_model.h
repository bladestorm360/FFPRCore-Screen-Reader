#pragma once
#include "library_model.h"
#include "../../core/field/map_state.h"
namespace ff5::pyramid {
struct Snapshot {
 int map=0;field::baked_route::Cell player;
 field::map_state::Flags flags;
 std::optional<bool> last_sand_switch_hidden;
 std::array<std::optional<bool>,3> corridor_hidden;
 bool phase_fresh=false,timers_frozen=false;
 std::wstring locale;std::function<bool()> cancelled;
};
bool supported(int map);
library::Plan plan(const Snapshot& snapshot,const json::Value& puzzles);
// Native-owner scalar phase observation;no game pointer enters the model.
bool phase_fresh(int map,int phase);
}
