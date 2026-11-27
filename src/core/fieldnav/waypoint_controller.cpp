// Ports FF1 Core/WaypointController.cs (cycling, pathfinding, add / rename / remove / clear)
// and Core/Handlers/WaypointHandler.cs (the "WaypointHandler.*" actions). Also loads the
// waypoint file at start, as the C# WaypointManager constructor did in OnInitializeMelon.
//
// The naming and confirmation windows belong to the mod-UI area (src/core/ui/mod_ui.h).
#include <functional>
#include <string>
#include "audio_loops.h"
#include "autowalk.h"
#include "fieldnav_util.h"
#include "nav_target.h"
#include "route_readout.h"
#include "waypoints.h"
#include "../core.h"
#include "../modtext.h"
#include "../say.h"
#include "../settings.h"
#include "../field/field_api.h"
#include "../ui/mod_ui.h"
#include "../ui/preferences.h"

namespace ffpr::fieldnav::waypoints::controller {
namespace {

std::wstring map_id_string()
{
    return std::to_wstring(::field::current_map_id());
}

void cycle_next()
{
    if (!ensure_field_context()) return;

    std::wstring map_id = map_id_string();
    nav::refresh_list(map_id);

    if (nav::count() == 0) {
        ffpr::say(T(L"No waypoints on this map"));
        return;
    }

    nav::cycle_next();
    route_readout::cancel();
    nav_target::mark_waypoint();
    ffpr::say(nav::format_current_waypoint());
}

void cycle_previous()
{
    if (!ensure_field_context()) return;

    std::wstring map_id = map_id_string();
    nav::refresh_list(map_id);

    if (nav::count() == 0) {
        ffpr::say(T(L"No waypoints on this map"));
        return;
    }

    nav::cycle_previous();
    route_readout::cancel();
    nav_target::mark_waypoint();
    ffpr::say(nav::format_current_waypoint());
}

void cycle_next_category()
{
    if (!ensure_field_context()) return;

    std::wstring map_id = map_id_string();
    nav::cycle_next_category(map_id);
    route_readout::cancel();
    if (nav::count() > 0)
        nav_target::mark_waypoint();
    ffpr::say(nav::category_announcement());
}

void cycle_previous_category()
{
    if (!ensure_field_context()) return;

    std::wstring map_id = map_id_string();
    nav::cycle_previous_category(map_id);
    route_readout::cancel();
    if (nav::count() > 0)
        nav_target::mark_waypoint();
    ffpr::say(nav::category_announcement());
}

void announce_waypoint_route(::field::baked_nav::MapGoal goal, EntityPtr waypoint, bool automatic = false)
{
    const auto token = route_readout::begin();
    goal.spoken_leg_limit = route_readout::leg_limit();
    ::field::baked_nav::request_route(goal, ::field::TargetKind::Spot, waypoint->name,
        [goal, waypoint, token, automatic](::field::PathInfo path) {
            if (!route_readout::current(token) || !ensure_field_context() || nav::selected() != waypoint) return;
            auto line = path.success && !path.description.empty() ? path.description :
                (!path.reason.empty() ? path.reason : T(L"No path"));
            if (ui::prefs::mod_warnings_enabled() && !path.warning.empty())
                ffpr::say_sequence({path.warning, std::move(line)}, !automatic);
            else ffpr::say(line, !automatic);
            route_readout::follow(path, token, [goal, waypoint] {
                if (nav::selected() == waypoint && ensure_field_context())
                    announce_waypoint_route(goal, waypoint, true);
            });
        });
}

void pathfind_to_current_waypoint()
{
    if (!ensure_field_context()) return;
    if (autowalk::stop_requested()) return;

    EntityPtr waypoint = nav::selected();
    if (!waypoint) {
        ffpr::say(T(L"No waypoint selected"));
        return;
    }

    nav_target::mark_waypoint();

    // Beacon mode: restart the beacon to re-ping toward this waypoint.
    if (settings::get_bool("AudioBeacons", false)) {
        audio_loops::restart_beacon();
        if (!ui::prefs::autowalk_enabled() && !ui::prefs::beacon_with_directions_enabled()
            && settings::get_bool("AnnounceOnBeaconRestart", false))
            ffpr::say(nav::format_current_waypoint());
        if (!ui::prefs::autowalk_enabled() && !ui::prefs::beacon_with_directions_enabled()) return;
    }

    // Turn-by-turn mode: A* path + descriptive route (as entity pathfinding).
    unity::Vec3 player_pos{};
    if (!player_local_position(player_pos)) {
        ffpr::say(T(L"Cannot determine directions"));
        return;
    }

    ::field::baked_nav::MapGoal goal;
    goal.map_id = ::field::current_map_id();
    if (!::field::baked_nav::cell_at(::field::map_handle(), waypoint->position, goal.cell)) {
        ffpr::say(T(L"Navigation map data unavailable")); return;
    }
    if (ui::prefs::autowalk_enabled()) {
        route_readout::cancel();
        autowalk::request(goal, ::field::TargetKind::Spot, waypoint->name);
    } else announce_waypoint_route(goal, waypoint);
}

void add_new_waypoint_with_naming()
{
    if (!ensure_field_context()) return;

    if (nav::current_category() == All) {
        ffpr::say(T(L"Please select a category"));
        return;
    }

    unity::Vec3 player_pos{};
    if (!player_local_position(player_pos)) {
        ffpr::say(T(L"Cannot get player position"));
        return;
    }

    std::wstring map_id = map_id_string();
    int category = nav::current_category();

    ui::text_input::open(T(L"Enter waypoint name"), L"",
        [player_pos, map_id, category](const std::wstring& name) {
            store::add(name, player_pos, map_id, category);
            nav::refresh_list(map_id);
            ffpr::say(TF(L"Waypoint added: {0}", {name}));
        });
}

void rename_current_waypoint()
{
    if (!ensure_field_context()) return;

    EntityPtr waypoint = nav::selected();
    if (!waypoint) {
        ffpr::say(T(L"No waypoint selected"));
        return;
    }

    std::wstring map_id = map_id_string();
    std::wstring waypoint_id = waypoint->id;

    ui::text_input::open(T(L"Enter new waypoint name"), waypoint->name,
        [waypoint_id, map_id](const std::wstring& new_name) {
            if (store::rename(waypoint_id, new_name)) {
                nav::refresh_list(map_id);
                ffpr::say(TF(L"Waypoint renamed to: {0}", {new_name}));
            } else {
                ffpr::say(T(L"Failed to rename waypoint"));
            }
        });
}

void remove_current_waypoint()
{
    if (!ensure_field_context()) return;

    EntityPtr waypoint = nav::selected();
    if (!waypoint) {
        ffpr::say(T(L"No waypoint selected"));
        return;
    }

    std::wstring map_id = map_id_string();
    std::wstring waypoint_name = waypoint->name;
    std::wstring waypoint_id = waypoint->id;

    ui::confirmation::open(TF(L"Delete waypoint {0}?", {waypoint_name}),
        [waypoint_id, waypoint_name, map_id] {
            if (store::remove(waypoint_id)) {
                nav::refresh_list(map_id);
                nav::clear_selection();
                ffpr::say(TF(L"Waypoint deleted: {0}", {waypoint_name}));
            } else {
                ffpr::say(T(L"Failed to delete waypoint"));
            }
        },
        [] {});
}

void clear_all_waypoints_for_map()
{
    if (!ensure_field_context()) return;

    std::wstring map_id = map_id_string();
    int count = store::count_for_map(map_id);

    if (count == 0) {
        ffpr::say(T(L"No waypoints to clear on this map"));
        return;
    }

    std::wstring plural = count == 1 ? T(L"waypoint") : T(L"waypoints");

    ui::confirmation::open(TF(L"Clear all {0} {1} from this map?", {std::to_wstring(count), plural}),
        [map_id, plural] {
            int cleared = store::clear_map(map_id);
            nav::refresh_list(map_id);
            nav::clear_selection();
            ffpr::say(TF(L"Cleared {0} {1}", {std::to_wstring(cleared), plural}));
        },
        [] {});
}

int install()
{
    store::load();

    actions::define("WaypointHandler.CyclePrevious", [] { cycle_previous(); });
    actions::define("WaypointHandler.CycleNext", [] { cycle_next(); });
    actions::define("WaypointHandler.CyclePreviousCategory", [] { cycle_previous_category(); });
    actions::define("WaypointHandler.CycleNextCategory", [] { cycle_next_category(); });
    actions::define("WaypointHandler.PathfindToCurrentWaypoint", [] { pathfind_to_current_waypoint(); });
    actions::define("WaypointHandler.AddNewWaypointWithNaming", [] { add_new_waypoint_with_naming(); });
    actions::define("WaypointHandler.RenameCurrentWaypoint", [] { rename_current_waypoint(); });
    actions::define("WaypointHandler.RemoveCurrentWaypoint", [] { remove_current_waypoint(); });
    actions::define("WaypointHandler.ClearAllWaypointsForMap", [] { clear_all_waypoints_for_map(); });
    return 0;
}
core::Install s_install{"WaypointController", &install, 110};

} // namespace
} // namespace ffpr::fieldnav::waypoints::controller
