#pragma once
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <memory>
#include <array>
#include "field_api.h"
#include "baked_router.h"
#include "route_progress.h"
namespace field::map_state { class Resolver; struct Flags; }

// The game-side glue of the baked-map router (baked_router.h): the live map, the player's
// cell and layer, the current vehicle, the owned auto-vehicles, the live blockers and the
// counter rule, turned into a request; the answer turned into the spoken route. Called by
// field::find_path_to_quick (navigation.cpp) and the asynchronous request_route service.
// Missing map data is a bake bug; there is no native route-search fallback.
//
// Modes (FF5's PathSearchMode, kept): Full = an explicit player action: live blockers, the
// hidden-passage retry, and on a world map boarding a parked ship or airship when walking
// alone cannot reach the target ("board the Ship", "land"). Quick = per entity / per tick:
// one reachability flood per player cell, no blockers, no boarding.
namespace field::baked_nav {

/// Starts data-only preparation on the native worker. Does not wait for or call the game.
void preload();
bool prepared();

/// True when the baked data knows the current map (and its size agrees with the live map).
bool available(void* map_handle);

/// The route for the current map, or false when required baked map data is unavailable.
/// `kind` says where the route ends (field_api.h TargetKind). Full mode keeps the planned
/// route: asked again for the same target from a cell on it, it speaks the rest from there;
/// a target cut off on this floor is reached through another floor ("take the exit to ...");
/// and when nothing reaches it `out.reason` says why when the router knows ("Story gated",
/// "Blocked by an NPC", "Requires Ship"). Quick mode with `with_vehicles` counts a world-map
/// target as reachable when a vehicle the party has here carries the player to it.
bool find(unity::Vec3 from, unity::Vec3 to, void* map_handle, void* player, bool full, TargetKind kind,
          bool with_vehicles, PathInfo& out);

/// The audio beacon follows the planned route (the FFXII mod's model; user, 2026-10-11): the
/// next corner of the route to `target` from the player's cell, planning the route when none
/// is active or the player has left it (a failed plan is retried after two seconds). False when
/// the baked data does not know the map or no route exists: the beacon then falls back to the
/// target itself.
struct Corner {
    bool found = false;
    bool arrived = false;          // the player stands on the route's last cell
    unity::Vec3 point{};           // the corner to steer at (map local space)
    int steps_to_corner = 0;
    int steps_left = 0;            // to the end of the route
    bool pending = false;          // owned work underway; distinct from a proved blocked route
};
bool follow(void* map_handle, void* player, unity::Vec3 target, TargetKind kind, Corner& out, int entity_id = -1, int transportation_id = 0);

/// The player's cell and layer on the current map's baked grid (false off the field).
bool player_cell(void* map_handle, void* player, baked_route::Cell& out);
/// A world position as a baked cell on the current map (the layer is left at 1).
bool cell_at(void* map_handle, unity::Vec3 world, baked_route::Cell& out);
/// The centre of a baked cell in the map's local space (the inverse of cell_at).
unity::Vec3 world_of(void* map_handle, const baked_route::Cell& cell);

/// The cells a route may end on for a target at `target` (baked_router.h goals_for, kind
/// inferred from what is baked there), under the player's current movement rules.
std::vector<baked_route::Cell> goal_cells(void* map_handle, unity::Vec3 target);

/// Movement instructions, with interaction facing only at the arrival cell.
using Phase = route_progress::Phase;
std::wstring describe(const std::vector<Phase>& phases, bool full = false);
/// Recheck a single autowalk edge against current NPCs, story gates and movement rules.
bool walk_edge(void* map_handle, void* player, const baked_route::Cell& from, const baked_route::Cell& to, bool goal_edge,
               route_progress::VehicleAction vehicle_action = route_progress::VehicleAction::None, int transportation_id = 0);

/// Forget the cached reachability flood (a scan or a map change).
void invalidate();
void reset_transition_history(); // native same-floor entity-group change, not a floor load

// --- across maps -----------------------------------------------------------------------------
/// A destination on any sub-map of this game, by map master id and baked cell.
struct MapGoal {
    int map_id = 0;
    baked_route::Cell cell;
    bool examine = false;   // the target is examined from a neighbouring cell (a switch, a clock)
    std::vector<baked_route::Cell> cells;   // explicit goal cells (baked_router goals_for), else from cell/examine
    int transportation_id = 0;     // a selected vehicle uses its captured native model position
    int entity_id = -1;            // -1 for coordinate-only goals; zero is a valid native identity
    std::wstring raw_name;         // developer identity only, never spoken; separates story variants
};

/// Capture bounded native state on its owner, then plan/format on an owned worker.
/// Publication returns to the native owner only for the current map/request.
/// keep_destination=false keeps a separate latest guide/follower trap receipt;
/// it does not replace the manually selected destination.
void request_route(MapGoal goal, TargetKind kind, std::wstring name,
                   std::function<void(PathInfo)> publish, bool keep_destination = true);
// One native capture for a bounded ordered set of scenario objectives. The
// Navigation worker picks the first currently usable target, preserving real
// gates and encounter positions. The guide owns a preview, never the player's
// manually selected entity/waypoint. At most 32 candidates are accepted.
struct GuideTarget { MapGoal goal; TargetKind kind = TargetKind::Infer; std::wstring instruction; };
struct DefenseContext {
    MapGoal protected_actor;
    std::vector<MapGoal> enemies;
    std::array<std::optional<baked_route::Cell>,3> parties;
    int current_party=0;
    std::wstring warning;
};
struct DefenseAdvice {
    std::optional<MapGoal> intercept;
    int closest_party=-1;
    std::wstring warning;
};
void request_guidance(std::vector<GuideTarget> candidates, std::function<void(PathInfo)> publish,
                      const map_state::Flags* anticipated_release = nullptr, const DefenseContext* defense = nullptr);
// anticipated_release, when present, is copied synchronously. Only static gates
// are projected for departure; live NPCs retain captured positions. No game write.
void cancel_requests();
void interrupt_requests(); // epochs only; retain owner callbacks until normal cleanup
bool request_pending();
/// Read-only browsing evidence, on its own cancellable capture/worker lane owner.
/// Never publishes an active route or changes the manually tracked destination.
struct BrowseRoute { bool direct = false; int via_map_id = 0; std::wstring reason; };
void request_browse_route(MapGoal goal, TargetKind kind, std::function<void(BrowseRoute)> publish);
void cancel_browse_route();
void request_reachability(bool with_vehicles, std::function<void()> ready);
// Same owned native obstacle snapshot and movement rules as explicit routes.
// Capture functions run on the native owner; distances run only on Navigation.
struct ExitSnapshot;
DefenseAdvice defense_advice(ExitSnapshot& snapshot, const DefenseContext& context, std::function<bool()> cancelled);
struct ExitTarget {
    unity::Vec3 position; int map_id = 0, destination = 0; baked_route::Cell cell;
    TargetKind kind = TargetKind::Transition;
};
std::shared_ptr<ExitSnapshot> begin_exit_capture(bool with_vehicles, bool indoor, bool story_filter = false);
void capture_exit_obstacle(ExitSnapshot& snapshot, void* entity);
void finish_exit_capture(ExitSnapshot& snapshot);
/// Worker-only teleport clearance using the same copied native obstacles and movement rules.
bool teleport_landing_safe(ExitSnapshot& snapshot, unity::Vec3 target);
bool exit_snapshot_world(const ExitSnapshot& snapshot);
/// Owned worker fact: current FFIII world1 snapshot has no live NPC/party actor.
bool exit_snapshot_stationary_inventory(const ExitSnapshot& snapshot);
int exit_snapshot_vehicle(const ExitSnapshot& snapshot);
std::vector<int> exit_distances(ExitSnapshot& snapshot, const std::vector<ExitTarget>& targets,
                              std::function<bool()> cancelled);
// Worker-only batch classification, independent of translated failure text.
// True requires a complete gated search and a usable route with only gates
// removed. Unknown/cancelled/bounded-out routes stay visible. No active route write.
// One selected local baked trigger; contains owned values only, never native pointers.
struct AirTransitionTarget { MapGoal goal; unity::Vec3 position; std::wstring name; int distance=-1, destination_map=0; };
// Same owned marker payload for manual Nautilus surfacing. No native trigger
// exists here; source-to-output verification pins the real paired basin tile.
std::optional<AirTransitionTarget> nautilus_surface_target(ExitSnapshot& snapshot,
    std::function<bool()> cancelled,bool require_reachable=true);
// require_reachable=false is unfiltered discovery: retain one real nearest
// trigger even without a current ride, with distance=-1 rather than invented access.
std::optional<AirTransitionTarget> air_transition_target(ExitSnapshot& snapshot,
    const std::optional<MapGoal>& continued,const std::optional<MapGoal>& selected,std::function<bool()> cancelled,bool story_entry=false,bool require_reachable=true);
struct StoryTarget { MapGoal goal; TargetKind kind = TargetKind::Infer; };
std::vector<bool> story_gate_mask(ExitSnapshot& snapshot, const std::vector<StoryTarget>& targets,
                                 std::function<bool()> cancelled);

/// The first leg of the shortest way from the player to `goal`, possibly on another map: the
/// route across the current map to the exit or warp to take (Dijkstra over the sub-map graph of
/// GotoMap exits and script warps, each arriving at the destination's PointIn), or the route to
/// the goal itself when it is on this map. `description` says what to do at the end of the leg
/// ("take the exit to Castle Cornelia 2F", "step on the warp"). False when unreachable.
struct CrossLeg {
    bool found = false;
    bool requires_trap = false;
    bool trap_leg = false;
    int trap_entity = 0, trap_x = 0, trap_y = 0;
    baked_route::Cell trap_arrival;
    bool trap_repeat_blocked = false;
    bool transition_repeat_blocked = false;
    bool event_on_route = false, incomplete = false;
    int visited_maps = 0;          // owned search-work evidence for bounded regression checks
    bool on_this_map = false;        // the goal itself is the end of this leg
    int next_map_id = 0;             // the map the exit / warp leads to (when !on_this_map)
    std::vector<baked_route::Cell> cells;
    std::vector<Phase> phases;       // the leg as phases (the action word in action_after)
    std::wstring description;        // the spoken directions, action word included
    int steps_here = 0;              // steps of this leg
    int steps_total = 0;             // estimated steps to the goal across every map
};
/// `with_gates`: the current map's live story gates (a sealed door, a locked gate) close the
/// cells and exits they stand on; false ignores them, which tells whether a gate alone is
/// what cuts the goal off.
// A failed complete search may retry later-floor NPCs as interaction stops. Its
// current leg always retains with_npcs; native recapture owns each floor handoff.
// Optional worker-only world target batch shares this exact bounded search for
// list eligibility; pointers refer only to the immutable bake, never game objects.
CrossLeg find_across(void* map_handle, void* player, const MapGoal& goal, bool with_gates = true, bool allow_traps = false, bool with_npcs = true, bool with_remote_npcs = true,
    const std::vector<const baked::Object*>* world_targets = nullptr, std::vector<int>* distances = nullptr);
/// Retain a bounded completed-trap history for the current destination, preventing repeated legs.
bool note_trap_start(int map_id, const baked_route::Cell& at);
void record_map_arrival(int map_id, const baked_route::Cell& at);
/// Retire only the guide/follower receipt matching this goal, preserving manual navigation.
void clear_preview_destination(const MapGoal& goal);

/// The map master name of a baked map id, through the game ("Citadel of Trials 2F"), or empty.
std::wstring map_name_of(int map_id);

// --- other floors (user, 2026-10-11: "destinations from other floors need to be visible ...
// current map should include all dungeon floors, town floors etc") ---------------------------
/// A baked object on another floor: a talk target, chest, save point, eligible event/interaction,
/// teleport/vehicle pad or outward exit. `raw_name` is the developer's name (PropertyEntity.Name,
/// to be translated like a live entity's).
struct RemoteObject {
    bool pickup=false;
    int map_id = 0;                 // the floor's map master id (the id its exit was reached by)
    baked_route::Cell cell;
    baked::ObjType type = baked::ObjType::Event;
    std::wstring raw_name;
    std::wstring group;             // the object's Tiled group ("entity_default" or an "ev_e_N" variant)
    int content_id = 0;             // TreasureBox: its contents
    std::optional<bool> opened;     // TreasureBox: copied native save flag; absent when unknown
    bool world_exit = false;       // verified script-backed travel; native identity/type retained
    int dest_map_id = 0;            // GotoMap: its real destination, including connected floors
    bool shop = false;              // ShopNPC
    bool scripted = false;          // NPC/ShopNPC: an interaction script, not a name-only placeholder
    int entity_id = -1;
    std::wstring event_type;        // nonempty only for eligible event/interactive targets
};
/// The objects of every sub-map connected to the current one through exits and warps without
/// crossing a world map (a dungeon's floors, a town's houses and castle), in breadth-first
/// order from here; the current floor's own objects are not included. Empty on a world map,
/// off the field, or on a map the baked data does not know.
std::vector<RemoteObject> connected_objects(void* map_handle);
/// Pure immutable bake/state projection, safe on the data worker after prepared().
std::vector<RemoteObject> connected_objects_snapshot(int map_id, map_state::Resolver& state,
                                                  std::function<bool()> cancelled = {});
// Fourth filter: objects physically on connected world bakes, including town/
// dungeon entrance tiles there; never the interiors behind those entrances.
std::vector<RemoteObject> world_objects_snapshot(int map_id,map_state::Resolver& state,
                                               std::function<bool()> cancelled = {});

/// The route to `goal` wherever it is: on this floor the ordinary kept route (find); on another
/// floor the first leg ("..., take the exit to X") and `out.next_map_id`. A route that goes on
/// past this map is remembered as the pending destination under `name`, so that after the
/// transition `continue_pending` speaks the next leg; a route ending at the goal clears it.
/// False only when the baked data cannot route here at all.
bool route_to_goal(void* map_handle, void* player, const MapGoal& goal, TargetKind kind, const std::wstring& name,
                   PathInfo& out);
/// The pending destination (a spoken route that continues on another floor), or false.
bool pending_goal(MapGoal& goal, TargetKind& kind, std::wstring& name);
void remember_destination(const MapGoal& goal, TargetKind kind, const std::wstring& name);
void clear_pending(bool include_preview = false);
/// After a map change (entity_nav subscribes, 0.8 s after "Entering X"): speaks "<name>: <the
/// next leg>" to the pending destination from where the player arrived. Cleared when that leg
/// ends at the destination or nothing reaches it any more.
void continue_pending();
/// The beacon's next corner towards a goal on this map or another floor: the plan is kept and
/// re-made silently when the player leaves it, like `follow`.
bool follow_goal(void* map_handle, void* player, const MapGoal& goal, TargetKind kind, Corner& out);

} // namespace field::baked_nav
