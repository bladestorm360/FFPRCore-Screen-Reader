#pragma once
#include "library_model.h"
#include "../../core/field/map_state.h"
namespace ff5::jachol {
struct Snapshot {
 int map=0;field::baked_route::Cell player;
 field::map_state::Flags flags;
 std::optional<bool> skull_hidden,chest_door_hidden;
 bool fresh=false,timers_frozen=false;
 std::wstring locale;std::function<bool()> cancelled;
};
bool supported(int map);
library::Plan plan(const Snapshot&,const json::Value&);
bool phase_fresh(int map,int phase);
}
