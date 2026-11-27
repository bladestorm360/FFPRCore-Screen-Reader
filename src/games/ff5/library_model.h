#pragma once
#include <array>
#include <optional>
#include <span>
#include "../../core/field/baked_nav.h"
#include "../../core/json.h"

// Owned Library observations and original offline decisions. No native pointer,
// script execution, game write or layout search belongs to this worker model.
namespace ff5::library {
inline constexpr std::array<int,8> flags{48,49,50,163,371,395,396,397};
struct Control {int id=0;const wchar_t* raw=L"";};
std::span<const Control> controls(int map);
bool supported(int map);
struct Snapshot {
 int map=0;
 field::baked_route::Cell player;
 std::array<std::optional<int>,8> values;
 std::array<std::optional<bool>,8> hidden;
 std::wstring locale;
 std::function<bool()> cancelled;
};
struct Plan {
 int map=0;bool supported=false,finished=false,unknown=false;
 std::wstring note;
 std::vector<field::baked_nav::GuideTarget> targets;
 // Pyramid and the new Jachol leaf carry native phase freshness. Older
 // untimed leaves keep-1 and retain their accepted lifecycle.
 int floor_phase=-1;
 int phase_map=424; // Jachol's new timed leaf uses284;older leaves are unchanged.
 int finish_map=0; // New Jachol leaf:full native climb return,armed after a route.
 unsigned puzzle_types=0; // New rule leaf;older models retain their typed owner.
 std::wstring guide_id; // Rule-leaf identity permits only its authored transit maps.
 // Passage controls use exact standing/facing points. Arrival alone cannot
 // operate them;only the actual native script-ready event changes the layout.
 bool observe_waypoint_arrival=true;
 int karnak_treasure_flag=-1; // Native receipt for the chosen timed chest.
};
Plan plan(const Snapshot& snapshot,const json::Value& puzzles);
}
