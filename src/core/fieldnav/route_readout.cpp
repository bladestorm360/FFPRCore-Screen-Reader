#include "route_readout.h"
#include "../core.h"
#include "../field/baked_nav.h"
#include "../frame.h"
#include "../system/system.h"
#include "../ui/preferences.h"
#include <cmath>
#include <optional>
#include <utility>

namespace ffpr::fieldnav::route_readout {
namespace {
using ::field::baked_route::Cell;
struct IssuedChunk {
    int map;
    std::uint64_t token;
    std::vector<Cell> cells;
    size_t cursor = 0;
    std::function<void()> next;
};
std::optional<IssuedChunk> issued;
struct FieldAction { int map; Cell stance; };
std::optional<FieldAction> field_action;
struct RouteEnd { int map; Cell stance; unity::Vec3 position; };
std::optional<RouteEnd> route_end;
std::function<void()> refresh_route;
std::uint64_t generation = 0;
std::uint64_t refresh_token = 0;
bool sampling = false;
bool action_started = false;
bool battle_interrupted = false;
bool refresh_queued = false;
bool obstacle_tracking = false;
constexpr int settle_frames = 24;
constexpr int return_ready_frames = 1800;

void invalidate_observation()
{
    ++generation;
    sampling = false;
    action_started = false;
    refresh_queued = false;
}
}

void cancel()
{
    invalidate_observation();
    issued.reset();
    field_action.reset();
    route_end.reset();
    refresh_route = {};
    battle_interrupted = false;
    obstacle_tracking = false;
}

std::uint64_t begin(std::function<void()> refresh)
{
    cancel();
    if (ui::prefs::auto_route_readout()) {
        refresh_token = generation;
        refresh_route = std::move(refresh);
    }
    return generation;
}

bool current(std::uint64_t token) { return token == generation; }

int leg_limit()
{
    return ui::prefs::full_route_readout() ? 0 : ui::prefs::route_legs_per_readout();
}

void follow(::field::PathInfo& path, std::uint64_t token, std::function<void()> next,
            std::function<void()> refresh)
{
    if (token != generation || !ui::prefs::auto_route_readout()) return;
    if (!path.success || path.walk_phases.empty()) { refresh_route = {}; return; }
    const int map = ::field::current_map_id();
    const auto& last = path.walk_phases.back();
    // Keep the spoken interaction when walking to its stance did not finish
    // the route. The entry observation precedes any switch carry or hide.
    if (!path.readout_limited && !path.next_map_id && last.manual_after &&
        !last.action_after.empty() && !last.cells.empty())
        field_action = FieldAction{map, last.cells.back()};
    if (!path.readout_limited && !path.next_map_id && !last.manual_after &&
        !path.npc_approach && !path.event_on_route && !path.required_trap && !last.cells.empty()) {
        const auto& at = last.cells.back();
        route_end = RouteEnd{map, at, ::field::baked_nav::world_of(::field::map_handle(), at)};
    }
    obstacle_tracking = path.step_count > 0 || field_action.has_value();
    if (path.step_count == 0 && !path.next_map_id && !field_action) refresh_route = {};
    else if (refresh) {
        refresh_token = token;
        refresh_route = std::move(refresh);
    }
    if (path.readout_cells.size() >= 2 && next)
        issued.emplace(IssuedChunk{map, token, std::move(path.readout_cells), 0, std::move(next)});
}

bool expects_field_action()
{
    return field_action && refresh_token == generation && refresh_route;
}

void field_action_started(int map, const Cell& at)
{
    if (expects_field_action() && field_action->map == map && field_action->stance == at)
        action_started = true;
}

bool battle_refresh_pending() { return battle_interrupted; }

namespace {
void queue_refresh()
{
    invalidate_observation();
    refresh_queued = true;
    frame::every_frame([token = generation, budget = return_ready_frames]() mutable {
        if (token != generation) return false;
        if (!ui::prefs::auto_route_readout() || --budget == 0) { cancel(); return false; }
        if (ffpr::system::input_transition_blocked() || !ffpr::system::is_field_navigation_ready() ||
            menu_state::any_active() || queries::is("ModMenu.IsOpen")) return true;
        Cell at;
        if (!::field::baked_nav::player_cell(::field::map_handle(), ::field::player(), at)) return true;
        // Consume before requesting a new route, so duplicate native ready
        // callbacks cannot prompt twice or reuse the previous walking cells.
        auto refresh = std::move(refresh_route);
        cancel();
        refresh();
        return false;
    }, "RouteReadout.FieldReturn", return_ready_frames);
}
}

void field_returned(bool same_map_action)
{
    if (!refresh_queued && refresh_route && (battle_interrupted || (same_map_action && action_started)))
        queue_refresh();
}

std::uint64_t obstacle_token()
{
    return obstacle_tracking && refresh_token == generation && refresh_route ? generation : 0;
}

void obstacle_hit(int map, const Cell& at)
{
    if (!obstacle_token()) return;
    if (route_end && route_end->map == map && route_end->stance == at) { cancel(); return; }
    queue_refresh();
}

void movement_settled(std::uint64_t token, const unity::Vec3& position)
{
    if (!token || token != generation || !route_end ||
        std::abs(position.x - route_end->position.x) >= 8 ||
        std::abs(position.y - route_end->position.y) >= 8) return;
    // The movement observer already owns the position. Only confirm the
    // layer and tile when it reaches the vicinity of the completed route.
    if (ffpr::system::input_transition_blocked() || !ffpr::system::is_field_navigation_ready() ||
        route_end->map != ::field::current_map_id()) return;
    Cell at;
    if (::field::baked_nav::player_cell(::field::map_handle(), ::field::player(), at) && at == route_end->stance)
        cancel();
}

void motion_input()
{
    if (!issued || issued->token != generation || sampling) return;
    sampling = true;
    // Native movement input can return before the destination cell is settled.
    // Watch only a bounded tail of this input burst, while a chunk is armed.
    frame::every_frame([token = generation, budget = settle_frames]() mutable {
        if (token != generation) return false;
        if (!issued || !ffpr::system::is_field_navigation_ready() ||
            menu_state::any_active() || queries::is("ModMenu.IsOpen") ||
            issued->map != ::field::current_map_id()) {
            cancel();
            return false;
        }
        auto& chunk = *issued;
        Cell at;
        if (::field::baked_nav::player_cell(::field::map_handle(), ::field::player(), at)) {
            using ::field::route_progress::ChunkProgress;
            const auto progress = ::field::route_progress::advance_chunk(chunk.cells, chunk.cursor, at);
            if (progress == ChunkProgress::Deviated) {
                // Keep the destination for a later confirmed obstacle, but
                // never speak solely because the player left the spoken path.
                issued.reset();
                sampling = false;
                return false;
            }
            if (progress == ChunkProgress::Completed) {
                auto next = std::move(chunk.next);
                cancel();
                next();
                return false;
            }
        }
        if (--budget > 0) return true;
        sampling = false;
        return false;
    }, "RouteReadout.Movement", settle_frames);
}

namespace {
int install()
{
    for (auto event : {events::Kind::MapChanging, events::Kind::PartyChanging}) {
        // Native transition prefixes invalidate the token only. Release the
        // owned cells after the gate has closed, off the native prefix.
        events::on(event, &invalidate_observation);
    }
    events::on(events::Kind::BattleEntering, [] {
        battle_interrupted = battle_interrupted || refresh_queued || (refresh_route && refresh_token == generation);
        invalidate_observation();
    });
    events::on(events::Kind::BattleStart, [] { issued.reset(); field_action.reset(); });
    events::on(events::Kind::MapChanged, [] { if (!battle_interrupted) cancel(); });
    for (auto event : {events::Kind::MenuOpened, events::Kind::TitleScreen})
        events::on(event, [] { cancel(); });
    for (const char* event : {"Settings.Changed.FullRouteReadout", "Settings.Changed.RouteLegsPerReadout",
                              "Settings.Changed.AutoRouteReadout"})
        actions::define(event, [] { cancel(); });
    return 0;
}
core::Install install_readout{"RouteReadout", &install, 135};
}
}
