#pragma once
#include "../field/baked_nav.h"

namespace ffpr::fieldnav::autowalk {
bool active();
bool following_guide();
/// All input devices use this start/stop contract. A second request stops the walk.
bool stop_requested();
void request(field::baked_nav::MapGoal goal, field::TargetKind kind, std::wstring name,
             bool keep_destination = true);
void start(const field::PathInfo& path, const field::baked_nav::MapGoal* goal = nullptr,
           field::TargetKind kind = field::TargetKind::Spot, const std::wstring& name = L"",
           bool keep_destination = true);
void cancel(bool announce = false);
/// Called after keyboard/SDL polling, before dispatch, even with no keyboard key pressed.
void input_updated();
/// Native field-state boundary for a verified walk-open door.
bool door_phase_started(bool scripted);
void door_phase_returned(int map_id);
/// Native input checks see a one-frame confirm/back press for a planned vehicle action.
bool virtual_key(int action, bool up = false);
bool virtual_input_active();
}
