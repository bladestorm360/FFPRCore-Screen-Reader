#pragma once
#include "../field/route_progress.h"
#include <cmath>
#include <utility>

// Data-only follower. It advances only after a completed native tile move, never by time
// or interpolated transform position. The caller owns native movement and context checks.
namespace ffpr::fieldnav::autowalk {
using Cell = field::baked_route::Cell;
using Phase = field::route_progress::Phase;
using VehicleAction = field::route_progress::VehicleAction;
enum class Result { Move, Arrived, Action, Wait, OffRoute };
struct Step {
    Result result = Result::OffRoute;
    Cell next;
    int x = 0, y = 0; // native axis: positive y is north
    std::wstring instruction;
    bool goal_edge = false;
    int face_direction = -1;
    VehicleAction vehicle_action = VehicleAction::None;
    int transportation_id = 0;
    bool precision = false;
    float axis_x=0,axis_y=0;
    field::route_progress::PrecisionPoint precision_from,precision_to;
};
struct PrecisionPosition {double x2=0,y2=0;};
class Route {
    std::vector<Phase> phases_;
    size_t phase_ = 0, cell_ = 0;
    int width_ = 0, height_ = 0;
    bool cardinal_only_ = false;
    bool action_done_ = false;
    static int delta(int d, int size) {
        if (size > 0) {
            if (d > size / 2) d -= size;
            if (d < -size / 2) d += size;
        }
        return d;
    }
public:
    void set(std::vector<Phase> phases, int width, int height, bool world, int loop_axes = -1) {
        phases_ = std::move(phases); phase_ = cell_ = 0; action_done_ = false;
        if(loop_axes<0||(!loop_axes&&world))loop_axes=world?3:0;
        width_ = (loop_axes&1)?width:0; height_ = (loop_axes&2)?height:0; cardinal_only_ = world;
    }
    Step next(const Cell& at,std::optional<PrecisionPosition> position={}) {
        while (phase_ < phases_.size()) {
            const Phase& p = phases_[phase_];
            if (p.cells.empty()) return {};
            if(!p.precision.empty()) {
                if(!position||p.precision.size()>7)return {};
                size_t index=p.precision.size();
                for(size_t i=0;i<p.precision.size();++i)if(p.precision[i].layer==at.layer&&
                    std::abs(position->x2-p.precision[i].x2)<0.03&&std::abs(position->y2-p.precision[i].y2)<0.03)index=i;
                if(index==p.precision.size())return {};
                if(index+1==p.precision.size()){++phase_;cell_=0;action_done_=false;continue;}
                const auto from=p.precision[index],to=p.precision[index+1];
                if((from.x2!=to.x2&&from.y2!=to.y2)||from.layer!=to.layer)return {};
                Step result;result.result=Result::Move;result.next={to.layer,(to.x2+1)/2,(to.y2+1)/2};
                result.precision=true;result.axis_x=(to.x2-from.x2)*.5f;result.axis_y=(from.y2-to.y2)*.5f;
                result.precision_from=from;result.precision_to=to;return result;
            }
            // Boarding/landing can change position to the next phase's first cell. Give the
            // action before requiring that displacement; it belongs to the player.
            const bool automatic = p.vehicle_before == VehicleAction::WalkOn || p.vehicle_before == VehicleAction::WalkOff;
            if (cell_ == 0 && !p.action_before.empty() && !automatic && !action_done_) {
                if (p.vehicle_before != VehicleAction::None && (!p.action_cell || !(at == *p.action_cell))) return {};
                return {Result::Action, p.cells.front(), 0, 0, p.action_before, false, p.vehicle_direction,
                    p.vehicle_before, p.transportation_id};
            }
            while (cell_ + 1 < p.cells.size() && at == p.cells[cell_ + 1]) ++cell_;
            if (!(at == p.cells[cell_])) return {};
            if (cell_ + 1 < p.cells.size()) {
                const Cell& to = p.cells[cell_ + 1];
                int dx = delta(to.x - at.x, width_);
                int dy = delta(to.y - at.y, height_);
                if (std::max(std::abs(dx),std::abs(dy)) != 1 || (cardinal_only_ && dx && dy)) return {};
                // Ordinary diagonals retain native baked stair edges. Reviewed
                // sub-tile passages use the separate bounded branch above.
                return {Result::Move, to, dx, -dy, {}, cell_ + 2 == p.cells.size(), -1,
                    automatic && cell_ == 0 ? p.vehicle_before : VehicleAction::None, p.transportation_id};
            }
            if (p.wait_for_npc) return {Result::Wait, {}, 0, 0, p.action_after};
            if (p.manual_after) return {Result::Action, {}, 0, 0, p.action_after, false, p.arrival_direction};
            if (phase_ + 1 == phases_.size())
                return {Result::Arrived, {}, 0, 0, p.arrival_facing, false, p.arrival_direction};
            ++phase_; cell_ = 0; action_done_ = false;
        }
        return {};
    }
    void complete_vehicle_action() { action_done_ = true; }
};
}
