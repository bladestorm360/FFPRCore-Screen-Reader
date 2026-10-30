#pragma once
#include "baked_router.h"
#include <string>
#include <optional>
#include <vector>

namespace field::route_progress {
enum class VehicleAction { None, WalkOn, WalkOff, ConfirmBoard, ConfirmLand, BackLand };
struct Phase {
    std::vector<baked_route::Cell> cells;
    std::wstring action_before;
    std::wstring action_after;
    // Interaction guidance belongs to arrival, even when the last movement faced it.
    std::wstring arrival_facing;
    int arrival_direction = -1; // structured cardinal heading; never parse localized speech
    bool manual_after = false;   // e.g. an examine warp; walking alone cannot activate it
    bool wait_for_npc = false;   // temporary patrol occupancy; replan, never interact/arrive
    VehicleAction vehicle_before = VehicleAction::None;
    int transportation_id = 0;
    int vehicle_direction = -1;
    std::optional<baked_route::Cell> action_cell;
    // Movement after this phase's native vehicle action. The action's vehicle
    // can be the craft being left; it must not govern the final walking leg.
    int movement_transportation_id=-1; // -1 current mode, 0 foot/owned auto-use
    // Native manual movement between this phase's last cell and the next phase
    // (Invincible mountain crossing). These tiles are not ordinary walk inputs.
    int manual_displacement_steps=0;
};
inline int steps(const Phase& phase) {
    return (phase.cells.empty()?0:static_cast<int>(phase.cells.size())-1)+phase.manual_displacement_steps;
}

inline bool remaining(const std::vector<Phase>& phases, const baked_route::Cell& at,
                      std::vector<Phase>& out, const std::function<bool()>& cancelled = {})
{
    for (size_t pi = phases.size(); pi-- > 0;) {
        if (cancelled && cancelled()) return false;
        const auto& p = phases[pi];
        for (size_t i = p.cells.size(); i-- > 0;) {
            if (cancelled && cancelled()) return false;
            if (!(p.cells[i] == at)) continue;
            out.assign(phases.begin() + static_cast<ptrdiff_t>(pi), phases.end());
            out.front().cells.erase(out.front().cells.begin(), out.front().cells.begin() + static_cast<ptrdiff_t>(i));
            if (i != 0) { out.front().action_before.clear(); out.front().vehicle_before = VehicleAction::None; }
            return true;
        }
    }
    return false;
}

inline bool at_interaction(const std::vector<Phase>& phases, size_t index)
{
    return index + 1 == phases.size() && phases[index].cells.size() == 1;
}
}
