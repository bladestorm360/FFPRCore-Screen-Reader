// Ports FF1 Patches/MovementSoundPatches.cs: wall bumps and footsteps.
//
// Wall bump: a prefix on FieldPlayerKeyController.OnTouchPadCallback(Vector2) (the field
// movement input; one body per game, not folded in any dump.cs) notes the player's position,
// and 0.08 s later (the C# coroutine's WaitForSeconds) a check plays the thud if the player
// did not move - only after two hits at the same spot, and at most every 0.3 s.
//
// Footsteps: PollFootsteps, the per-frame tile-crossing check the C# InputManager.Update ran
// right after ControllerRouter.Update. It is the action "MovementSoundPatches.PollFootsteps";
// the input area runs it each frame (an allowed audio loop).
//
// Reads other areas' state by name: "ControllerRouter.IsFieldActive" (input/router) and
// "MoveStateHelper.IsOnFoot" (vehicle state); the player is field::player().
#include <cmath>
#include <optional>
#include <utility>
#include "sound.h"
#include "../core.h"
#include "../fieldnav/route_readout.h"
#include "../frame.h"
#include "../hooks.h"
#include "../log.h"
#include "../refs.h"
#include "../settings.h"
#include "../system/system.h"
#include "../unity.h"
#include "../field/baked_nav.h"
#include "../field/field_api.h"

namespace ffpr::audio {
namespace {

ref::Class kFieldPlayerController{"Last.Map", "FieldPlayerController"};
ref::Class kFieldPlayerKeyController{"Last.Map", "FieldPlayerKeyController"};
ref::Field fFieldPlayer{kFieldPlayerController, "fieldPlayer"};
ref::Method mOnTouchPadCallback{kFieldPlayerKeyController, "OnTouchPadCallback", 1};

// Cooldown so holding a direction against a wall does not spam the bump.
constexpr double kBumpCooldown = 0.3;          // seconds between bump sounds
// Consecutive failed moves at one spot, to tell real walls from false positives.
constexpr int kRequiredConsecutiveHits = 2;
constexpr float kPositionTolerance = 1.0f;     // positions within 1 unit are "the same"
constexpr float kTileSize = 16.0f;
constexpr double kWallCheckDelay = 0.08;       // movement takes ~0.067 s per tile

double g_last_bump_time = -1.0e9;   // C# 0 against Time.time: no cooldown after a reset
bool g_logged_active = false;

unity::Vec3 g_last_collision_pos{};
int g_same_position_count = 0;

// Only one wall check at a time (no stacking while a direction is held).
bool g_wall_check_pending = false;
uint64_t g_movement_epoch = 0, g_direction_epoch = 0;
int g_obstacle_direction = -1;
bool g_tested_obstacle_edge = false;
unity::Vec3 g_tested_edge_origin{};

struct RouteAttempt {
    void* player;
    unity::Vec3 origin;
    uint64_t generation, direction_epoch;
    int direction;
};
std::optional<RouteAttempt> g_route_attempt;

// Set by a wall check that saw no movement, so the footstep poll skips the coincident step.
bool g_collision_detected_this_frame = false;

// Tile tracking for the per-frame footstep poll.
int g_last_tile_x = 0;
int g_last_tile_y = 0;
bool g_tile_tracking_initialized = false;

float distance(const unity::Vec3& a, const unity::Vec3& b)
{
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

constexpr float kInputThreshold = 0.1f;
bool has_movement_input(unity::Vec2 axis)
{
    return std::fabs(axis.x) > kInputThreshold || std::fabs(axis.y) > kInputThreshold;
}

int cardinal_direction(unity::Vec2 axis)
{
    constexpr int north = 0, east = 1, south = 2, west = 3;
    const bool horizontal = std::fabs(axis.x) > kInputThreshold;
    const bool vertical = std::fabs(axis.y) > kInputThreshold;
    if (horizontal == vertical) return -1;
    if (horizontal) return axis.x > 0 ? east : west;
    return axis.y > 0 ? north : south;
}

void observe_direction(int direction)
{
    if (g_obstacle_direction == direction) return;
    g_obstacle_direction = direction;
    // A tap can release before the move settles. Its pending observation is
    // still valid until a new direction attempt replaces it.
    if (direction >= 0) {
        ++g_direction_epoch;
        g_tested_obstacle_edge = false;
    }
}

void retire_movement_check()
{
    ++g_movement_epoch;
    g_wall_check_pending = false;
    g_route_attempt.reset();
    observe_direction(-1);
}

void confirm_route_obstacle(void* observed_player, const unity::Vec3& origin, int direction,
                            uint64_t direction_epoch, uint64_t route_token)
{
    namespace readout = fieldnav::route_readout;
    if (!route_token || !readout::current(route_token) || direction_epoch != g_direction_epoch ||
        direction < 0 || g_tested_obstacle_edge) return;
    if (system::input_transition_blocked() || !system::is_field_navigation_ready() ||
        menu_state::any_active() || queries::is("ModMenu.IsOpen") || !queries::is("MoveStateHelper.IsOnFoot")) return;
    auto* player = ::field::player();
    if (!player || player != observed_player) return;
    ::field::baked_route::Cell at;
    if (!::field::baked_nav::player_cell(::field::map_handle(), player, at)) return;
    g_tested_obstacle_edge = true;
    g_tested_edge_origin = origin;
    constexpr float dx[]{0, kTileSize, 0, -kTileSize}, dy[]{kTileSize, 0, -kTileSize, 0};
    const unity::Vec3 destination{origin.x + dx[direction], origin.y + dy[direction], origin.z};
    if (::field::confirm_blocked_move(player, destination)) {
        const int map = ::field::current_map_id();
        LOGF("[RouteReadout] blocked edge on map %d at (%d,%d) L%d direction %d", map, at.x, at.y, at.layer, direction);
        readout::obstacle_hit(map, at);
    }
}

void check_route_attempt(uint64_t movement, RouteAttempt scheduled, bool trailing)
{
    namespace readout = fieldnav::route_readout;
    if (movement != g_movement_epoch) return;
    if (!g_route_attempt || !readout::current(g_route_attempt->generation) ||
        system::input_transition_blocked() || !system::is_field_navigation_ready() ||
        menu_state::any_active() || queries::is("ModMenu.IsOpen")) {
        g_route_attempt.reset();
        return;
    }
    const auto latest = *g_route_attempt;
    if (latest.direction_epoch != scheduled.direction_epoch || latest.generation != scheduled.generation) {
        // A turn can arrive while the old step is settling. Give the actual
        // newer attempt its own observation window without stacking tasks.
        frame::after_seconds(kWallCheckDelay, [movement, latest] { check_route_attempt(movement, latest, false); },
                             "RouteReadout.Obstacle");
        return;
    }
    g_route_attempt.reset();
    auto* player = ::field::player();
    if (!player || player != latest.player || !unity::alive(player) || !unity::transform(player)) return;
    const auto current = unity::local_position(player);
    if (distance(latest.origin,current) < 0.1f) {
        confirm_route_obstacle(player, latest.origin, latest.direction, latest.direction_epoch, latest.generation);
        return;
    }
    g_tested_obstacle_edge = false;
    readout::movement_settled(latest.generation,current);
    if (trailing || !readout::current(latest.generation) || g_obstacle_direction != latest.direction ||
        g_direction_epoch != latest.direction_epoch) return;
    // A held step can reach the wall after moving during the first sample.
    // Confirm one trailing attempt from the new tile and then wait for input.
    auto tail = latest;
    tail.origin = current;
    g_route_attempt = tail;
    frame::after_seconds(kWallCheckDelay, [movement, tail] { check_route_attempt(movement, tail, true); },
                         "RouteReadout.Obstacle");
}

void observe_route_attempt(void* player, uint64_t token)
{
    if (!token || g_obstacle_direction < 0 || g_tested_obstacle_edge ||
        (g_route_attempt && g_route_attempt->direction_epoch == g_direction_epoch && g_route_attempt->generation == token)) return;
    const RouteAttempt attempt{player,unity::local_position(player),token,g_direction_epoch,g_obstacle_direction};
    const bool scheduled = g_route_attempt.has_value();
    g_route_attempt = attempt;
    if (scheduled) return;
    frame::after_seconds(kWallCheckDelay,[movement = g_movement_epoch, attempt] {
        check_route_attempt(movement,attempt,false);
    },"RouteReadout.Obstacle");
}

void play_bump_sound()
{
    hooks::guarded("MovementSoundPatches.PlayBumpSound", [] { sound::play_wall_bump(); });
}

/// CheckForWallBumpAfterFrame, after its 0.08 s wait.
void check_for_wall_bump(void* player, unity::Vec3 position_before)
{
    hooks::guarded("MovementSoundPatches.CheckForWallBumpAfterFrame", [&] {
        // The player may be gone (map change); a destroyed object reads as not alive.
        if (!unity::alive(player) || !unity::transform(player)) return;

        unity::Vec3 position_after = unity::local_position(player);
        float distance_moved = distance(position_before, position_after);
        if (g_tested_obstacle_edge && distance(g_tested_edge_origin,position_after) >= 0.1f)
            g_tested_obstacle_edge = false;

        if (distance_moved < 0.1f) {
            // Mark the collision so the per-frame footstep poll skips this step.
            g_collision_detected_this_frame = true;

            float dist_from_last = distance(position_before, g_last_collision_pos);
            if (dist_from_last < kPositionTolerance) {
                g_same_position_count++;
            } else {
                // New position: restart the count.
                g_same_position_count = 1;
                g_last_collision_pos = position_before;
            }

            // Only after confirmed consecutive hits.
            if (g_same_position_count < kRequiredConsecutiveHits) return;

            // Cooldown checked after the wait, so several checks never all play at once.
            double current_time = frame::now();
            if (current_time - g_last_bump_time < kBumpCooldown) return;

            play_bump_sound();
            g_last_bump_time = current_time;
        } else {
            // The player moved: reset the collision counter.
            g_same_position_count = 0;
        }

        // Reset the collision flag at the end of the check.
        g_collision_detected_this_frame = false;
    });
    // Always clear the pending flag so the next check can start (the C# finally).
    g_wall_check_pending = false;
}

/// OnTouchPadCallback prefix: capture the position, check it after the move.
void on_touch_pad_prefix(void* self, unity::Vec2 axis)
{
    if (!g_logged_active) {
        g_logged_active = true;
        LOGF("MovementSoundPatches: OnTouchPadCallback hook active");
    }

    // No audio when menus or battle are active (same gate as controller routing).
    if (!queries::is("ControllerRouter.IsFieldActive")) { observe_direction(-1); return; }
    const auto route_token = fieldnav::route_readout::obstacle_token();
    if (route_token || g_obstacle_direction >= 0) observe_direction(cardinal_direction(axis));

    // Only real movement input.
    if (!has_movement_input(axis)) return;

    void* player = ref::rd<void*>(self, fFieldPlayer);
    if (!player || !unity::transform(player)) return;
    observe_route_attempt(player, route_token);

    // One pending check at a time: holding a direction must not stack them.
    if (!g_wall_check_pending) {
        g_wall_check_pending = true;
        unity::Vec3 position_before = unity::local_position(player);
        frame::after_seconds(kWallCheckDelay, [player, position_before, movement = g_movement_epoch] {
            if (movement == g_movement_epoch) check_for_wall_bump(player, position_before);
        }, "MovementSoundPatches.WallCheck");
    }
    // Wall tones and beacons are the audio loops' job, not this hook's.
}

using OnTouchPadCallback_t = void (*)(void* self, unity::Vec2 axis, const void* mi);
OnTouchPadCallback_t o_OnTouchPadCallback = nullptr;

void d_OnTouchPadCallback(void* self, unity::Vec2 axis, const void* mi)
{
    hooks::guarded("MovementSoundPatches.OnTouchPadCallback", [&] { on_touch_pad_prefix(self, axis); });
    o_OnTouchPadCallback(self, axis, mi);
    if (has_movement_input(axis))
        hooks::guarded("RouteReadout.Movement", [] { ffpr::fieldnav::route_readout::motion_input(); });
    // The puzzle guide checks the player's cell against its current step on each movement
    // input (an input event, not a poll): see src/core/field/puzzle_guide.cpp.
    hooks::guarded("PuzzleGuide.OnMove", [] {
        if (actions::defined("PuzzleGuide.OnMove")) actions::run("PuzzleGuide.OnMove");
    });
}

/// PollFootsteps: a footstep each time the player's tile changes, so the cadence follows the
/// real speed (walk vs dash). Silent in vehicles, in menus and battle, and on the step that
/// coincides with a wall bump.
void poll_footsteps()
{
    hooks::guarded("MovementSoundPatches.PollFootsteps", [] {
        if (!queries::is("ControllerRouter.IsFieldActive")) return;
        if (!settings::get_bool("Footsteps", false)) return;
        if (!queries::is("MoveStateHelper.IsOnFoot")) return;

        void* player = field::player();
        if (!player || !unity::transform(player)) return;

        unity::Vec3 pos = unity::local_position(player);
        if (std::isnan(pos.x) || std::fabs(pos.x) > 10000.0f) return;

        int tile_x = static_cast<int>(std::floor(pos.x / kTileSize));
        int tile_y = static_cast<int>(std::floor(pos.y / kTileSize));

        if (!g_tile_tracking_initialized) {
            g_last_tile_x = tile_x;
            g_last_tile_y = tile_y;
            g_tile_tracking_initialized = true;
            return;
        }

        if (tile_x == g_last_tile_x && tile_y == g_last_tile_y) return;
        g_last_tile_x = tile_x;
        g_last_tile_y = tile_y;

        // A wall check set this for the frame it fired: skip the coincident step.
        if (g_collision_detected_this_frame) return;

        sound::play_footstep();
    });
}

/// ResetState: on scene loads, so stale collision and footstep data never cross maps.
void reset_state()
{
    g_last_bump_time = -1.0e9;
    g_last_collision_pos = unity::Vec3{};
    g_same_position_count = 0;
    retire_movement_check();
    g_collision_detected_this_frame = false;
    g_last_tile_x = 0;
    g_last_tile_y = 0;
    g_tile_tracking_initialized = false;
}

int install()
{
    actions::define("MovementSoundPatches.PollFootsteps", [] { poll_footsteps(); });
    actions::define("MovementSoundPatches.ResetState", [] { reset_state(); });
    events::on(events::Kind::SceneLoaded, [] { reset_state(); });
    for (auto event : {events::Kind::MapChanging, events::Kind::BattleEntering, events::Kind::PartyChanging})
        events::on(event, &retire_movement_check);
    return hooks::attach(mOnTouchPadCallback, &d_OnTouchPadCallback, &o_OnTouchPadCallback) ? 1 : 0;
}

core::Install s_install{"MovementSoundPatches", &install, 100};

} // namespace
} // namespace ffpr::audio
