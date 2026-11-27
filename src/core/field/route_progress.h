#pragma once
#include "baked_router.h"
#include <string>
#include <optional>
#include <vector>
#include <algorithm>
#include <cstdlib>

namespace field::route_progress {
struct PrecisionPoint {
    int layer=0,x2=0,y2=0;
    bool operator==(const PrecisionPoint& other) const {return layer==other.layer&&x2==other.x2&&y2==other.y2;}
};
enum class VehicleAction { None, WalkOn, WalkOff, ConfirmBoard, ConfirmLand, BackLand };
struct Phase {
    std::vector<baked_route::Cell> cells;
    std::wstring action_before;
    std::wstring action_after;
    // Interaction guidance belongs to arrival, even when the last movement faced it.
    std::wstring arrival_facing;
    int arrival_direction = -1; // structured cardinal heading; never parse localized speech
    bool manual_after = false;   // e.g. an examine warp; walking alone cannot activate it
    bool random_transfer = false; // next room is an attempt, never a promised arrival
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
    // A reviewed narrow passage retains its two coarse endpoints for route
    // ownership and exact half-tile movement points for every output consumer.
    // At most seven cardinal points, no game pointers, timers or worker native calls.
    std::vector<PrecisionPoint> precision;
    bool announce_arrival_facing = false; // a stand-on interaction needs its turn after the walking directions
};
inline int steps(const Phase& phase) {
    if(!phase.precision.empty()) {
        int halves=0;for(size_t i=1;i<phase.precision.size();++i)
            halves+=std::max(std::abs(phase.precision[i].x2-phase.precision[i-1].x2),std::abs(phase.precision[i].y2-phase.precision[i-1].y2));
        return (halves+1)/2;
    }
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

enum class ChunkProgress { Unchanged, Advanced, Completed, Deviated };
inline ChunkProgress advance_chunk(const std::vector<baked_route::Cell>& cells, size_t& cursor,
                                   const baked_route::Cell& at)
{
    if (cursor >= cells.size()) return ChunkProgress::Deviated;
    if (cells[cursor] == at) return ChunkProgress::Unchanged;
    // A dash or delayed owner frame can deliver a short run in one update.
    // Bound the lookahead and accept the first matching occurrence in order.
    constexpr size_t max_cell_advance = 4;
    const size_t end = std::min(cells.size(), cursor + 1 + max_cell_advance);
    for (size_t next = cursor + 1; next < end; ++next) {
        if (cells[next] != at) continue;
        cursor = next;
        return cursor + 1 == cells.size() ? ChunkProgress::Completed : ChunkProgress::Advanced;
    }
    return ChunkProgress::Deviated;
}
}
