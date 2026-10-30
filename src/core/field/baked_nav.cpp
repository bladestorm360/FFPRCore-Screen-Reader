// The game-side glue of the baked-map router: see baked_nav.h.
//
// Cells: the game's MapRouteSearcher cell space, cx = floor(w/2 + x/16), cy = floor(h/2 - y/16)
// (FF1 FindPathTo; w, h = the sub-map's size in tiles), the player's layer = its game object's
// Unity layer - 9 (Bottom 0, Ground 1, Upper 2). A world map has no collision grid: the baked
// sub-map's size is used (the live IMapAccessor reports no collision size there).
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "baked_nav.h"
#include "airship_transitions.h"
#include "nautilus_surface.h"
#include "party_entities.h"
#include "baked_map.h"
#include "field_internal.h"
#include "map_names.h"
#include "map_state.h"
#include "entity_translator.h"
#include "../game.h"
#include "../hooks.h"
#include "../log.h"
#include "../frame.h"
#include "../background.h"
#include "../worker_job.h"
#include "capture_budget.h"
#include "npc_route.h"
#include "remote_exit_policy.h"
#include "beacon_route_index.h"
#include "../system/system.h"
#include "../modtext.h"
#include "../say.h"
#include "../text.h"
#include "../fieldnav/move_state.h"
#include "../ui/preferences.h"

namespace field::baked_nav {
namespace {

using baked_route::Cell;
using unity::Vec3;

struct ActiveRoute {
    bool valid = false;
    int map_id = 0;               // the map the route was planned on (the player's)
    Cell target;                  // the target cell the route was planned to (x, y)
    int goal_map_id = 0;          // the target's map: map_id, or another floor (route_to_goal)
    int next_map_id = 0;          // the route goes on there after its last leg (0 = ends at the target)
    int transportation_id = 0;
    std::vector<Phase> phases;
    bool requires_trap = false;
    int blocking_party = -1; // partial approaches are replanned, never kept as completed goals
    bool npc_approach = false;
    int target_vehicle=0;
    bool vehicle_shortcuts=false; // captured filter policy; invalidates a kept route on change
    Cell observed_target; // native sprite tile; vehicle geometry uses its model tile
};

struct TrapVisit {
    int map_id = 0, entity_id = 0, destination = 0, x = 0, y = 0;
    Cell entry, arrival;
    bool observed = false;
};

struct Pending {
    bool valid = false;
    MapGoal goal;
    TargetKind kind = TargetKind::Infer;
    std::wstring name;
    std::vector<TrapVisit> completed_traps;
    std::optional<TrapVisit> incoming_trap;
    std::vector<TrapVisit> completed_transitions;
    std::optional<TrapVisit> incoming_transition;
    uint64_t state_signature = 0;
};
thread_local Pending* g_pending = nullptr;
Pending* g_preview_pending = nullptr; // native-owner receipt for the latest guide/follower request
std::shared_ptr<const ActiveRoute> g_published_route; // native-owner handle to immutable worker data

struct RouteSnapshot;
thread_local RouteSnapshot* g_route_snapshot = nullptr;
const baked::Submap* snapshot_submap();
int planning_map_id();
int snapshot_layer();
Vec3 snapshot_position();
VehicleState planning_vehicle();
std::vector<int> planning_auto_groups();
std::vector<VehicleOnMap> planning_vehicles();
int journey_vehicle_family(int);
std::vector<VehicleOnMap> journey_vehicles();
int phase_steps(const std::vector<Phase>&);
std::wstring planning_vehicle_name(int id, int type, const std::wstring& message);
map_state::Resolver planning_state();
bool planning_object_active(const baked::Submap&,const baked::Object&);
void cancel_route_work();
void current_relays(const baked::Submap&, baked_route::Request&);
std::shared_ptr<const baked::TransitionGrid> current_transitions(const baked::Submap&);
void cancel_follow_work();
void cancel_quick_work();
bool planning_cancelled();
void prepare_mountain_phases(const baked::Submap&,std::vector<Phase>&);
struct ShipGoal { uint64_t ids=0; const baked::Object* source=nullptr; bool transition=false; };
std::optional<ShipGoal> ship_goal_at(const baked::Submap&,const Cell&);
bool quick_query(unity::Vec3 from, unity::Vec3 to, void* map, void* player, TargetKind kind, bool with_vehicles, PathInfo& out);
bool live_interaction(const baked::Submap& sm, const Cell& target, TargetKind kind, baked::Object& out);


constexpr float kTile = 16.0f;
constexpr int kLongRouteSpeechThreshold = 31; // speech brevity only; searches have no distance cap
constexpr size_t kMaxSpokenLegs = 6;
constexpr int kTransportContent = 5;   // auto-ridden vehicles (canoe, snowcraft)
constexpr int kTransportShip = 2;
ref::Class kCollider{"Last.Entity.Field", "FieldColliderEntity"};
ref::Field fColliderEnable{kCollider, "enable"};
ref::Field fColliderId{kCollider, "collisionId"};
ref::Class kTrigger{"Last.Entity.Field", "EventTriggerEntity"};
ref::Field fTriggerPower{kTrigger, "<TriggerPower>k__BackingField"};
// FieldCharaEntity.CanEnter reads this collider and permits passage when it is
// absent or a trigger. Capture the same facts on the native owner, never workers.
ref::Class kCharaCollision{"Last.Entity.Field", "FieldCharaEntity"};
ref::Field fCharaBox{kCharaCollision, "boxCollider2D"};
ref::Class kBoxCollision{"UnityEngine", "Collider2D"};
ref::Method mBoxTrigger{kBoxCollision, "get_isTrigger", 0};
ref::Class kNpcMovement{"Last.Map","PropertyNpc"};
ref::Field fNpcMoveType{kNpcMovement,"<MoveType>k__BackingField"};
ref::Field fNpcMoveStep{kNpcMovement,"<MoveStep>k__BackingField"};
// Only these versions publish RoutePuppet102. That marker survives a completed
// one-shot program, so inspect its current owned controller before saying Wait.
ref::Class kRoutineController{"Last.Map","FieldController",game::kFF3|game::kFF5|game::kFF6};
ref::Field fRoutineManager{kRoutineController,"nonPlayerControllerManager"};
ref::Class kRoutineManager{"Last.Map","FieldNonPlayerControllerManager",game::kFF3|game::kFF5|game::kFF6};
ref::Field fRoutineControllers{kRoutineManager,"nonPlayerControllerList"};
ref::Class kRoutineBase{"Last.Map","FieldNonPlayerController",game::kFF3|game::kFF5|game::kFF6};
ref::Field fRoutineActor{kRoutineBase,"nonPlayerEntity"};
ref::Class kRoutine{"Last.Map","FieldNonPlayerRouteMoveInstructionController",game::kFF3|game::kFF5|game::kFF6};
ref::Field fRoutineCurrent{kRoutine,"currentInstruction"};
ref::Field fRoutineList{kRoutine,"instructionList"};
ref::Field fRoutineIndex{kRoutine,"instructionIndex"};
ref::Field fRoutineLoop{kRoutine,"loop"};
ref::Class kRoutineMove{"Last.Map","MoveToInstruction",game::kFF3|game::kFF5|game::kFF6};
ref::Class kRoutineLerp{"Last.Map","MoveLerpInstruction",game::kFF3|game::kFF5|game::kFF6};
void invalidate_active();
constexpr int kTransportSubmarine = 6;

std::atomic<bool> g_prepared{false};
bool g_load_started = false; // boot/main-thread request only; all decoding belongs to worker
bool g_loaded = false;
std::string* g_blob = nullptr;   // the embedded resource bytes: the reader keeps the pointer
std::unordered_map<int, bool>* g_size_warned = nullptr;

bool ensure_loaded()
{
    return g_prepared.load(std::memory_order_acquire) && g_loaded;
}

const baked::Submap* current_submap(void* map_handle)
{
    if (g_route_snapshot) return snapshot_submap();
    if (!ensure_loaded()) return nullptr;
    const int map_id = planning_map_id();
    if (map_id <= 0) return nullptr;
    const baked::Submap* sm = baked::submap_by_map_id(map_id);
    if (!sm) return nullptr;
    // The live map's collision size must agree (a stale map handle, or the wrong sub-map).
    if (map_handle && !sm->world) {
        const int w = call_virtual_int(r::mGetCollisionLayerWidth, map_handle);
        const int h = call_virtual_int(r::mGetCollisionLayerHeight, map_handle);
        if (w > 0 && h > 0 && (w != sm->width || h != sm->height)) {
            if (!g_size_warned) g_size_warned = new std::unordered_map<int, bool>();
            if (!(*g_size_warned)[map_id]) {
                (*g_size_warned)[map_id] = true;
            LOGF("[BakedMap] map %d: live size %dx%d, baked %ls %dx%d: navigation data mismatch", map_id, w, h,
                     baked::name_of(*sm).c_str(), sm->width, sm->height);
            }
            return nullptr;
        }
    }
    return sm;
}

Cell cell_of(const baked::Submap& sm, Vec3 world, int layer)
{
    // Live world entities can occupy another visual copy of the looping map.
    // Normalize at the shared conversion boundary so goals, parked vehicles,
    // origins, blockers and beacon tracking all identify the same physical tile.
    // Indoor maps retain their bounds; invalid observations never wrap into a
    // plausible destination or reach an undefined floating-to-integer cast.
    const double x=std::floor(sm.width*0.5f+world.x/kTile);
    const double y=std::floor(sm.height*0.5f-world.y/kTile);
    if(!std::isfinite(x)||!std::isfinite(y)||x<INT_MIN||x>INT_MAX||y<INT_MIN||y>INT_MAX)
        return Cell{layer,-1,-1};
    Cell c{layer,static_cast<int>(x),static_cast<int>(y)};
    if(sm.world&&sm.width>0&&sm.height>0) {
        c.x=(c.x%sm.width+sm.width)%sm.width;
        c.y=(c.y%sm.height+sm.height)%sm.height;
    }
    return c;
}

Vec3 world_of_cell(const baked::Submap& sm, const Cell& c)
{
    return Vec3{(c.x - sm.width * 0.5f) * kTile + kTile * 0.5f, (sm.height * 0.5f - c.y) * kTile - kTile * 0.5f, 0};
}

int player_layer(void* player)
{
    if (g_route_snapshot) return snapshot_layer();
    const int layer = game_object_layer(player) - 9;
    return layer < 0 || layer > 2 ? 1 : layer;
}

const wchar_t* direction_key(int dir)
{
    switch (dir) {
    case 0: return L"North";
    case 1: return L"East";
    case 2: return L"South";
    case 3: return L"West";
    case 4: return L"Northeast";
    case 5: return L"Southeast";
    case 6: return L"Southwest";
    default: return L"Northwest";
    }
}

void world_mode(baked_route::Request& req);

baked_route::GoalKind goal_kind(TargetKind kind)
{
    switch (kind) {
    case TargetKind::Interact: return baked_route::GoalKind::Interact;
    case TargetKind::Transition: return baked_route::GoalKind::Transition;
    case TargetKind::Spot: return baked_route::GoalKind::Spot;
    default: return baked_route::GoalKind::Infer;
    }
}

/// The goals for `target` under the player's movement rules now (baked_router.h goals_for).
const baked::Object* vehicle_pad_at(const baked::Submap& sm,const Cell& target) {
    if(!sm.world)return nullptr;
    const auto& grid=baked::grids(sm);
    const auto found=grid.vehicle_pads.find(target.y*grid.width+target.x);
    if(found==grid.vehicle_pads.end())return nullptr;
    for(size_t index:found->second) {
        const auto& object=grid.objects[index];
        if(planning_object_active(sm,object))return &object;
    }
    return nullptr;
}
void vehicle_pad_terminal(Phase& phase,const baked::Object* pad) {
    if(!pad)return;
    phase.manual_after=true; // native vehicle action/landing owns activation
    if(game::is(game::Id::FF2)&&pad->a==134)phase.action_after=T(L"land");
}
baked_route::Goals goals_for(const baked::Submap& sm, const Cell& target, TargetKind kind, const baked_route::Request* copied = nullptr)
{
    baked_route::Request movement;
    if (copied) { movement.live_relays=copied->live_relays; movement.relays=copied->relays; movement.transitions=copied->transitions; }
    else current_relays(sm,movement);
    movement.submap = &sm;
    movement.passages = game::flags().has_hidden_passages;
    world_mode(movement);
    if(kind==TargetKind::AirshipTransition) { baked_route::Goals goals;goals.cells.push_back(target);return goals; }
    if(kind==TargetKind::Infer)if(const auto* pad=vehicle_pad_at(sm,target)) {
        // A transportation action is reached aboard its allowed vehicle. Its
        // tile can be closed to walking/ordinary landing (Palamecia mountains).
        // Define the actual mounted footprint; full planning still proves
        // ownership, boarding, terrain and every intervening edge.
        baked_route::Goals goals;
        for(const auto& row:baked::transports())if(row.type!=1&&row.type!=kTransportContent&&baked::vehicle_pad_accepts(*pad,row.id)) {
            auto ride=movement;ride.vehicle_group=row.group;ride.vehicle_type=row.type;ride.landing_group=row.landing_group;
            if(baked_route::open(ride,baked::grids(sm),target)) {goals.cells.push_back(target);break;}
        }
        return goals;
    }
    // A reviewed sea scene has a Ship footprint, just as a town has a walking
    // footprint. Goal discovery does not assert ownership/reachability: the
    // normal mounted/boarding planner must still prove every legal route leg.
    const auto ship_goal=(kind==TargetKind::Infer||kind==TargetKind::Transition)?ship_goal_at(sm,target):std::nullopt;
    const bool ship_entry=ship_goal.has_value()||(sm.world&&std::any_of(baked::grids(sm).objects.begin(),baked::grids(sm).objects.end(),
        [&](const baked::Object& object){return baked::ship_entry(object)&&object.contains(target.x,target.y);}));
    const bool submarine_entry=sm.world&&std::any_of(baked::grids(sm).objects.begin(),baked::grids(sm).objects.end(),
        [&](const baked::Object& object){return baked::submarine_entry(object)&&object.contains(target.x,target.y);});
    if(ship_goal&&!ship_goal->ids)return {};
    if(ship_entry)for(const auto& row:baked::transports())if(row.type==kTransportShip&&
        (!ship_goal||(ship_goal->ids&baked_route::transportation_bit(row.id)))) {
        movement.vehicle_group=row.group;movement.vehicle_type=row.type;movement.landing_group=row.landing_group;break;
    }
    if(submarine_entry)for(const auto& row:baked::transports())if(row.type==kTransportSubmarine) {
        movement.vehicle_group=row.group;movement.vehicle_type=row.type;movement.landing_group=row.landing_group;break;
    }
    // Land destinations are reached after legally leaving the current vehicle.
    // Determine their actual walking footprint rather than a ship's fallback tile.
    if (!ship_entry && !submarine_entry && sm.world && movement.vehicle_group > 0 && !baked_route::native_ground_entry(movement.vehicle_type) &&
        (kind == TargetKind::Transition || !baked::group_crosses(movement.vehicle_group,baked::attribute(baked::grids(sm),target.x,target.y)))) {
        movement.vehicle_group = 0;
    }
    baked::Object actor{};
    const auto* interaction = live_interaction(sm, target, kind, actor) ? &actor : nullptr;
    auto goals = baked_route::goals_for(movement, target, goal_kind(kind), interaction);
    if(sm.world && movement.vehicle_group>0 && goals.face) {
        // Ordinary examined objects/NPCs need their ground interaction approach,
        // not an arbitrary adjacent airspace cell. Vehicle-action pads returned
        // above retain their separate mounted contract. All-six world talk
        // placements have no vehicle restriction or explicitly require foot/1.
        movement.vehicle_group=movement.vehicle_type=movement.landing_group=0;
        goals=baked_route::goals_for(movement,target,goal_kind(kind),interaction);
    }
    // Entity filtering asks for Infer, which recognizes GotoMap only inside
    // goals_for. A mount may cross the town's terrain while native transitions
    // still require leaving it. Retry that empty footprint as walking, just as
    // an explicit Transition request does, then use the legal landing flood.
    if (goals.cells.empty() && sm.world && movement.vehicle_group > 0 && kind == TargetKind::Infer) {
        movement.vehicle_group = 0;
        movement.vehicle_type = movement.landing_group = 0;
        goals = baked_route::goals_for(movement, target, goal_kind(kind), interaction);
    }
    if(goals.cells.empty()&&sm.world&&kind==TargetKind::Spot) {
        // A parked ship's water is closed to walking. The owned vehicle snapshot
        // proves this exact goal; the normal boarding planner still must find
        // a real legal shore and native boarding edge. Never relax arbitrary spots.
        for(const auto& vehicle:planning_vehicles()) {
            const auto* row=baked::transport(vehicle.transportation_id);
            const Cell parked=cell_of(sm,vehicle.pos,1);
            if(row&&row->type!=kTransportContent&&row->type!=1&&parked.x==target.x&&parked.y==target.y&&
               baked::vehicle_open(baked::grids(sm),row->group,parked.x,parked.y)&&
               baked::group_crosses(row->group,baked::attribute(baked::grids(sm),parked.x,parked.y))) {
                goals.cells.push_back(parked);break;
            }
        }
    }
    return goals;
}

/// Direction to the interaction from the route's arrival cell.
std::wstring facing_text(const std::vector<Cell>& cells, const baked_route::Goals& goals)
{
    const int dir = cells.empty() ? -1 : baked_route::facing_direction(cells.back(), goals);
    if (dir < 0) return L"";
    return TF(L"face {0}", {T(direction_key(dir))});
}

void annotate_facing(Phase& phase, const baked_route::Goals& goals)
{
    phase.arrival_direction = phase.cells.empty() ? -1 : baked_route::facing_direction(phase.cells.back(), goals);
    phase.arrival_facing = facing_text(phase.cells, goals);
}

/// Live obstacles for an explicit route, by what they are (user, 2026-10-11: say why a route
/// is blocked): a chest is a wall; a collision entity or a collision object that is active now
/// is a story gate (FF1's bridge before it is built, a locked door, "no entry until after the
/// event"); an NPC that stands still is a wall while it stands there; a wandering NPC is a
/// passing obstacle. Cells the route may end on are never obstacles. World maps: gates only.
struct Blockers {
    std::vector<Cell> chests, gates, fixed_npcs, moving_npcs, triggers, traps;
    struct Party {Cell cell;int slot;};
    std::vector<Party> parties;
    std::vector<npc_route::Actor> actors;
    std::vector<baked_route::Request::Directional> directional;
    std::shared_ptr<const baked_route::TransportRules> transport_rules;
    void apply_transport(baked_route::Request& request,bool gates=true) const {
        request.transport_rules=transport_rules;request.transport_gates=gates;
    }
    bool has_gates() const {
        if(!gates.empty()||!directional.empty())return true;
        if(!transport_rules)return false;
        if(!transport_rules->directional.empty())return true;
        for(const auto& plane:transport_rules->cells[baked_route::TransportRules::Gate])if(!plane.empty())return true;
        return false;
    }
    std::vector<Cell> hard(bool with_gates = true,bool with_parties = true,bool with_npcs = true) const
    {
        std::vector<Cell> out = chests;
        if (with_gates) out.insert(out.end(), gates.begin(), gates.end());
        if(with_npcs) {
            out.insert(out.end(), fixed_npcs.begin(), fixed_npcs.end());
            out.insert(out.end(), moving_npcs.begin(), moving_npcs.end());
        }
        if(with_parties)for(const auto& party:parties)out.push_back(party.cell);
        return out;
    }
};
struct NativeObstacle {
    Cell cell;
    int type = 0, id = 0, collision = -1; // unknown stays solid; native None(0) is actually empty
    bool collider = false, applies = true, enabled = true, trigger = false;
    uint64_t transportation_ids=~uint64_t(0); // null list allows all; empty allows none
    bool transportation_restricted=false,transportation_complete=true;
    bool chest = false, npc = false;
    bool solidity_known = false, solid = true; // native collider observation, conservative when unavailable
    bool movement_known = false, wanders = false; // authored routine, never instantaneous motion
    bool party = false;
    int party_slot = -1;
    std::wstring raw_name;
};
struct RouteSnapshot {
    double requested_at = 0, captured_at = 0;
    double native_ms = 0;
    unsigned capture_slices = 0;
    unsigned routine_work = 0; // total extra owner-side reads across this capture
    unsigned routine_actors = 0; // invalid/empty registries must also consume a budget
    int map_id = 0, layer = 1;
    const baked::Submap* submap = nullptr; // immutable prepared bake, process lifetime
    Vec3 position;
    VehicleState vehicle;
    std::vector<int> auto_groups;
    std::vector<VehicleOnMap> vehicles; // every .game is null before handoff
    std::vector<SavedWorldVehicle> saved_vehicles; // ready native save values, not engine objects
    std::unordered_map<int, std::wstring> vehicle_names;
    std::vector<NativeObstacle> obstacles;
    std::shared_ptr<const baked::TransitionGrid> live_transitions; // per worker scope, rebuilt from captured triggers
    std::shared_ptr<const baked_route::TransportRules> transport_rules;
    bool transport_rules_prepared=false;
    map_state::Resolver state;
    std::shared_ptr<const map_state::Flags> anticipated_release;
    std::wstring locale;
    MapGoal goal;
    Cell observed_goal;
    std::vector<GuideTarget> candidates;
    std::optional<DefenseContext> defense;
    TargetKind kind = TargetKind::Infer;
    std::wstring name;
    ActiveRoute active;
    std::shared_ptr<const ActiveRoute> previous_active;
    Pending pending;
    bool keep_destination = true;
    bool vehicle_shortcuts = false;
    bool projected_origin = false; // worker-only future world; no live actor inventory exists there
    std::shared_ptr<std::atomic<uint64_t>> epoch;
    uint64_t generation = 0;
    std::function<bool()> cancelled;
    explicit RouteSnapshot(map_state::Resolver copied) : state(std::move(copied)) {}
};
// A moving unit's identity survives displacement; its baked home does not.
// This runs only on owned snapshots. Keep the native capture authoritative on
// the loaded floor, and retain immutable bake metadata for masks/identity.
const NativeObstacle* selected_live_npc(const RouteSnapshot& snapshot) {
    // A parked vehicle can be a native NPC. Its Spot target is not a talk
    // interaction and must not inherit an all-layer (-1) collision footprint.
    if(snapshot.kind==TargetKind::Spot||snapshot.kind==TargetKind::Transition)return nullptr;
    if (snapshot.goal.map_id != snapshot.map_id || snapshot.goal.entity_id < 0) return nullptr;
    const NativeObstacle* found = nullptr;
    for (const auto& actor : snapshot.obstacles) {
        if (!actor.npc || actor.party || actor.id != snapshot.goal.entity_id ||
            (!snapshot.goal.raw_name.empty() && actor.raw_name != snapshot.goal.raw_name)) continue;
        if (found) return nullptr; // ambiguous identities must not redirect a route
        found = &actor;
    }
    return found;
}
bool live_interaction(const baked::Submap& sm, const Cell& target, TargetKind kind, baked::Object& out) {
    if (!g_route_snapshot || g_route_snapshot->submap != &sm || kind != TargetKind::Interact) return false;
    const auto* actor = selected_live_npc(*g_route_snapshot);
    if (!actor || actor->cell.x != target.x || actor->cell.y != target.y) return false;
    out.type = baked::ObjType::NPC; out.b = 15; out.tw = out.th = 1;
    for (const auto& object : baked::grids(sm).objects)
        if ((object.type == baked::ObjType::NPC || object.type == baked::ObjType::ShopNPC) &&
            object.entity_id() == actor->id && baked::object_identifier(object) == actor->raw_name) {
            out = object;
            // A route/polyline link is not a counter; a real counter approach
            // remains valid only while a stationary NPC is still at its home.
            if ((object.d & 16) || object.tx != actor->cell.x || object.ty != actor->cell.y) out.a = 0;
            break;
        }
    out.layer = static_cast<uint8_t>(actor->cell.layer < 0 ? target.layer : actor->cell.layer);
    out.tx = static_cast<int16_t>(actor->cell.x); out.ty = static_cast<int16_t>(actor->cell.y);
    return true;
}
const baked::Submap* snapshot_submap() { return g_route_snapshot->submap; }
bool planning_object_active(const baked::Submap& sm,const baked::Object& object) {
    // Quick owner-thread queries address a live observed entity. Avoid reading
    // the native flag store there; worker snapshots own their projected state.
    if(!g_route_snapshot)return true;
    if(air_transitions::saronia_entry(static_cast<int>(game::current()),g_route_snapshot->map_id,object)) {
        // The native world initializer activates this exact scene solely while
        // scenario73 is clear. A later additive-group observation is not a
        // complete group inventory and must not erase that known prerequisite.
        // Read the already copied bit here; no worker/native read or gate write.
        if(const auto complete=g_route_snapshot->state.scenario_value(73))return *complete==0;
    }
    return g_route_snapshot->state.active(sm,object);
}
int planning_map_id() { return g_route_snapshot ? g_route_snapshot->map_id : field::current_map_id(); }
int snapshot_layer() { return g_route_snapshot->layer; }
Vec3 snapshot_position() { return g_route_snapshot->position; }
VehicleState planning_vehicle() { return g_route_snapshot ? g_route_snapshot->vehicle : field::current_vehicle(); }
std::vector<int> planning_auto_groups() { return g_route_snapshot ? g_route_snapshot->auto_groups : field::owned_auto_vehicle_groups(); }
std::vector<VehicleOnMap> planning_vehicles() { return g_route_snapshot ? g_route_snapshot->vehicles : field::vehicles_on_map(); }
std::wstring planning_vehicle_name(int id, int type, const std::wstring& message) {
    if (!g_route_snapshot) return field::vehicle_display_name(id, type, message);
    const auto name = g_route_snapshot->vehicle_names.find(id);
    return name == g_route_snapshot->vehicle_names.end() ? std::wstring() : name->second;
}
// Native owner only, shared by list descriptions and explicit direction jobs.
// Workers receive identical owned names and never invoke game localization.
void capture_vehicle_names(RouteSnapshot& snapshot) {
    snapshot.auto_groups = field::owned_auto_vehicle_groups();
    snapshot.vehicles = field::vehicles_on_map();
    const auto* destination=baked::submap_by_map_id(snapshot.goal.map_id);
    const bool world_scope=settings::get_int(ui::prefs::kPathfindingFilterMode,-1)==ui::prefs::kFilterWorldMap||
        (snapshot.submap->world&&destination&&destination->world&&snapshot.goal.map_id!=snapshot.map_id);
    if(world_scope&&snapshot.vehicle_shortcuts&&game::is(game::Id::FF3))snapshot.saved_vehicles=field::saved_world_vehicles();
    for (auto& vehicle : snapshot.vehicles) {
        vehicle.game = nullptr;
        snapshot.vehicle_names.emplace(vehicle.transportation_id,
            field::vehicle_display_name(vehicle.transportation_id, vehicle.type, vehicle.message_id));
    }
    for (const auto& row : baked::transports())
        if (std::find(snapshot.auto_groups.begin(), snapshot.auto_groups.end(), row.group) != snapshot.auto_groups.end())
            snapshot.vehicle_names.emplace(row.id, field::vehicle_display_name(row.id, row.type, baked::str(row.message)));
    for(const auto& saved:snapshot.saved_vehicles)if(!snapshot.vehicle_names.contains(saved.id))
        if(const auto* row=baked::transport(saved.id))snapshot.vehicle_names.emplace(saved.id,
            field::vehicle_display_name(row->id,row->type,baked::str(row->message)));
}
map_state::Resolver planning_state() { return g_route_snapshot ? g_route_snapshot->state : map_state::Resolver(); }
void current_relays(const baked::Submap& sm, baked_route::Request& request) {
    if (g_route_snapshot) {
        map_state::add_relays(g_route_snapshot->state,sm,request);
        request.transitions=current_transitions(sm);
        request.transport_rules=g_route_snapshot->submap==&sm?g_route_snapshot->transport_rules:nullptr;
    }
}
bool planning_cancelled() {
    return g_route_snapshot && ((g_route_snapshot->epoch &&
        g_route_snapshot->epoch->load() != g_route_snapshot->generation) ||
        (g_route_snapshot->cancelled && g_route_snapshot->cancelled()));
}

baked_route::Request phase_movement(const baked_route::Request& root,const Phase& phase) {
    auto movement=root;
    if(phase.movement_transportation_id<0)return movement;
    movement.transportation_id=phase.movement_transportation_id;
    movement.vehicle_group=movement.vehicle_type=movement.landing_group=0;
    if(const auto* row=baked::transport(phase.movement_transportation_id)) {
        movement.vehicle_group=row->type==5||row->type==1?0:row->group;
        movement.vehicle_type=row->type;movement.landing_group=row->landing_group;
    }
    return movement;
}

bool event_on_route(const std::vector<Cell>& cells, const std::vector<Cell>& triggers, const std::vector<Cell>& selected_event = {},
                    const baked_route::Request* movement=nullptr) {
    std::vector<Cell> other;
    for (const auto& trigger : triggers) {
        const bool selected = std::any_of(selected_event.begin(), selected_event.end(), [&](const auto& cell) {
            return cell.x == trigger.x && cell.y == trigger.y && (trigger.layer < 0 || cell.layer == trigger.layer);
        });
        if (!selected) other.push_back(trigger);
    }
    if(baked_route::crosses_event(cells,other))return true;
    if(movement&&movement->transport_rules&&movement->submap)for(size_t i=1;i<cells.size();++i) {
        const auto& cell=cells[i];
        if(std::any_of(selected_event.begin(),selected_event.end(),[&](const auto& selected){
            return cell.x==selected.x&&cell.y==selected.y&&(movement->submap->world||cell.layer==selected.layer);
        }))continue;
        if(baked_route::transport_rule_at(*movement,baked::grids(*movement->submap),cell,baked_route::TransportRules::Hazard))return true;
    }
    return false;
}
std::wstring route_warning(const PathInfo& path, const MapGoal& goal) {
    if (path.required_trap) {
        const auto* map = baked::submap_by_map_id(goal.map_id);
        bool chest = false;
        if (map) for (const auto& object : baked::grids(*map).objects)
            chest |= object.type == baked::ObjType::TreasureBox && object.contains(goal.cell.x, goal.cell.y);
        return T(chest ? L"Warning: a trap is required to reach this chest." : L"Warning: a trap is required on this route.");
    }
    return path.event_on_route ? T(L"Event trigger on the route") : std::wstring();
}
constexpr int kObjCollisionEntity = 19, kObjEntity = 5, kObjAnimEntity = 10;   // MapConstants.ObjectType
constexpr int kFlagCollision = 1, kFlagWanders = 16;                           // bake field d

const baked::Object* baked_by_entity_id(const baked::Grids& g, int entity_id, baked::ObjType type, const Cell& live,
                                      const std::wstring& raw_name = {})
{
    if (entity_id < 0) return nullptr;
    const baked::Object* best = nullptr;
    int nearest = INT_MAX;
    const auto consider=[&](const baked::Object& o) {
        if (o.type == type && o.entity_id() == entity_id &&
            (raw_name.empty() || baked::object_identifier(o) == raw_name) &&
            (entity_id != 0 || !raw_name.empty() || o.contains(live.x,live.y))) {
            // Conditional companions share the native actor ID/cell. Loaded
            // terminal masks must use its currently selected branch, rather
            // than match the consumed first companion and lose the boundary.
            if(type==baked::ObjType::Warp&&g_route_snapshot&&g_route_snapshot->submap&&
               &baked::grids(*g_route_snapshot->submap)==&g&&g_route_snapshot->state.consumed(*g_route_snapshot->submap,o))return;
            const int d = std::abs(o.tx - live.x) + std::abs(o.ty - live.y);
            if (d < nearest) { best = &o; nearest = d; }
        }
    };
    if(g.entity_index_prepared) {
        const auto found=g.objects_by_entity.find(entity_id);
        if(found!=g.objects_by_entity.end())for(size_t index:found->second)consider(g.objects[index]);
    } else for(const auto& object:g.objects)consider(object); // original/synthetic grid compatibility
    return best;
}

// Worker-owned goal semantics, not a new native query. Native Ship-only triggers
// are reached aboard their allowed model; town landing policy cannot rank them.
// An empty mask is a known ship goal with no legal/verified current model.
std::optional<ShipGoal> ship_goal_at(const baked::Submap& sm,const Cell& target) {
    if(!sm.world||!g_route_snapshot||g_route_snapshot->submap!=&sm)return {};
    const auto& grid=baked::grids(sm);if(!baked::in_bounds(grid,target.x,target.y))return {};
    uint64_t ships=0,legal=0;const int attribute=baked::attribute(grid,target.x,target.y);
    for(const auto& row:baked::transports())if(row.type==kTransportShip) {
        const auto bit=baked_route::transportation_bit(row.id);ships|=bit;
        if(baked::group_crosses(row.group,attribute)&&baked::vehicle_open(grid,row.group,target.x,target.y))legal|=bit;
    }
    if(!ships)return {};
    auto& snapshot=*g_route_snapshot;
    for(const auto& object:grid.objects)if(baked::ship_entry(object)&&object.contains(target.x,target.y)) {
        uint64_t ids=snapshot.state.transportation_mask(sm,object);
        const auto name=baked::object_identifier(object);
        for(const auto& raw:snapshot.obstacles)if((raw.type==2||raw.type==28)&&raw.id==object.entity_id()&&raw.raw_name==name) {
            ids=!raw.transportation_complete?0:raw.transportation_restricted?raw.transportation_ids:~uint64_t(0);break;
        }
        return ShipGoal{ids&legal,&object,true};
    }
    std::optional<ShipGoal> found;
    for(const auto& raw:snapshot.obstacles) {
        if(planning_cancelled())return {};
        if(!raw.trigger||(raw.type!=2&&raw.type!=28)||raw.cell.x!=target.x||raw.cell.y!=target.y||
           !raw.transportation_restricted||!raw.transportation_complete||!raw.transportation_ids||
           (raw.transportation_ids&~ships))continue;
        const auto* source=baked_by_entity_id(grid,raw.id,raw.type==2?baked::ObjType::Event:baked::ObjType::RandomEvent,raw.cell,raw.raw_name);
        if(!source||source->a<=0||!baked_route::step_event(*source)||baked::is_trap(*source)||snapshot.state.consumed(sm,*source)||
           baked::world_exit_at(grid,source->entity_id(),baked::object_identifier(*source),source->tx,source->ty))continue;
        // Conflicting current actors at one point are not evidence that the
        // selected script accepts the union of their different vehicle lists.
        if(found&&found->source!=source)return ShipGoal{};
        found=ShipGoal{raw.transportation_ids&legal,source,false};
    }
    return found;
}

std::shared_ptr<const baked::TransitionGrid> current_transitions(const baked::Submap& sm)
{
    if(!g_route_snapshot)return {};
    auto& snapshot=*g_route_snapshot;
    const auto projected=snapshot.state.transitions(sm);
    if(snapshot.submap!=&sm)return projected;
    if(snapshot.live_transitions)return snapshot.live_transitions;
    // The loaded floor already has authoritative powered-trigger observations.
    // Unknown future variants are conservative only on remote, unloaded floors.
    auto mask=std::make_shared<baked::TransitionGrid>(*projected);
    for(auto& layer:*mask)for(auto& bits:layer)bits&=1;
    const auto& grid=baked::grids(sm);
    for(const auto& raw:snapshot.obstacles)if(raw.trigger) {
        if(raw.transportation_restricted)continue; // evaluated by each leg's owned selector mask
        const auto* warp=baked_by_entity_id(grid,raw.id,baked::ObjType::Warp,raw.cell,raw.raw_name);
        if(!warp||!(baked::transition_bits(*warp)&2)||snapshot.state.consumed(sm,*warp))continue;
        const int layer=sm.world?0:raw.cell.layer;
        if(layer<0||layer>2)continue;
        auto& cells=(*mask)[layer];if(cells.empty())cells.assign(static_cast<size_t>(grid.width)*grid.height,0);
        const int x0=warp->tw==1&&warp->th==1?raw.cell.x:warp->tx;
        const int y0=warp->tw==1&&warp->th==1?raw.cell.y:warp->ty;
        for(int y=std::max(0,y0);y<std::min(grid.height,y0+warp->th);++y)
            for(int x=std::max(0,x0);x<std::min(grid.width,x0+warp->tw);++x)
                cells[static_cast<size_t>(y)*grid.width+x]|=2;
    }
    snapshot.live_transitions=mask;return mask;
}

Blockers live_blockers(const baked::Submap& sm, const baked::Grids& g, const std::vector<Cell>& goals)
{
    Blockers out;
    auto is_goal = [&](int x, int y) {
        for (const Cell& c : goals)
            if (c.x == x && c.y == y) return true;
        return false;
    };
    auto add_footprint = [&](std::vector<Cell>& into, const baked::Object* o, const Cell& live, bool exclude_goals = true) {
        if (!o) {
            if (!exclude_goals || !is_goal(live.x, live.y)) into.push_back(live);
            return;
        }
        for (int y = o->ty; y < o->ty + o->th; ++y)
            for (int x = o->tx; x < o->tx + o->tw; ++x)
                if (!exclude_goals || !is_goal(x, y)) into.push_back(Cell{live.layer, x, y});
    };
    if (g_route_snapshot) {
        // Classification, bake matching and footprints are pure worker computation.
        using Rules=baked_route::TransportRules;
        auto& snapshot=*g_route_snapshot;
        std::shared_ptr<Rules> prepared;
        for (const auto& raw : g_route_snapshot->obstacles) {
            if(g_route_snapshot->anticipated_release &&
               (raw.type==kObjEntity||raw.type==kObjAnimEntity||raw.type==11||raw.type==kObjCollisionEntity))continue;
            Cell c = raw.cell;
            const auto mark=[&](Rules::Kind kind,const Cell& cell) {
                if(snapshot.transport_rules_prepared||!raw.transportation_ids)return;
                if(!prepared)prepared=std::make_shared<Rules>();
                baked_route::add_transport_cell(*prepared,kind,sm,cell,raw.transportation_ids);
            };
            const auto append=[&](std::vector<Cell>& cells,const Cell& cell,Rules::Kind kind) {
                if(raw.transportation_restricted)mark(kind,cell);else cells.push_back(cell);
            };
            const auto footprint=[&](std::vector<Cell>& cells,const baked::Object* object,Rules::Kind kind) {
                std::vector<Cell> owned;add_footprint(owned,object,c,false);
                for(const auto& cell:owned)append(cells,cell,kind);
            };
            if(raw.trigger&&raw.transportation_restricted) {
                const auto* warp=baked_by_entity_id(g,raw.id,baked::ObjType::Warp,c,raw.raw_name);
                if(warp&&baked::transition_bits(*warp)&&!snapshot.state.consumed(sm,*warp)) {
                    const auto kind=(baked::transition_bits(*warp)&2)?Rules::Terminal:Rules::GroundTerminal;
                    if(warp->tw==1&&warp->th==1)mark(kind,c);
                    else {std::vector<Cell> cells;add_footprint(cells,warp,c,false);for(const auto& cell:cells)mark(kind,cell);}
                }
            }
            if(raw.party) {
                if(!sm.world)out.parties.push_back({c,raw.party_slot});
                continue; // synthetic party properties cannot inherit a baked gate/NPC identity
            }
            if (raw.type == kObjCollisionEntity) {
                if (!raw.enabled) continue;
                // Collider names in the bake hold transportation IDs, rather
                // than the native developer name; position disambiguates ID 0.
                const auto* o = baked_by_entity_id(g, raw.id, baked::ObjType::CollisionEntity, c);
                const int collision = raw.collider ? raw.collision : (o ? o->a : -1);
                if(collision==0)continue;
                std::vector<Cell> cells; add_footprint(cells, o, c, false);
                for (const Cell& at : cells) {
                    if(raw.transportation_restricted) {
                        if(!snapshot.transport_rules_prepared&&raw.transportation_ids) {
                            if(!prepared)prepared=std::make_shared<Rules>();
                            baked_route::add_transport_collision(*prepared,sm,at,collision,raw.transportation_ids);
                        }
                    } else if(raw.applies)baked_route::add_collision(out.gates,out.directional,at,collision);
                }
            } else if (raw.trigger) {
                // Native transition helpers share an exit's cell but are not
                // independent scene hazards, even in a fresh live snapshot.
                if(baked::transition_helper_at(g,raw.id,raw.raw_name,c.x,c.y))continue;
                if(baked::world_exit_at(g,raw.id,raw.raw_name,c.x,c.y)){append(out.traps,c,Rules::Trap);continue;}
                const auto* o = baked_by_entity_id(g, raw.id, static_cast<baked::ObjType>(raw.type), c, raw.raw_name);
                if (!o) o = baked_by_entity_id(g, raw.id, baked::ObjType::Warp, c, raw.raw_name);
                if (o&&g_route_snapshot->state.consumed(sm,*o)) continue;
                // A completed scene may move its still-powered trigger away
                // instead of destroying it (FF6 Ymir: native SetPos after the
                // battle). A one-tile trigger owns its captured current cell;
                // resurrecting the baked home produces a phantom route warning.
                const auto add_trigger=[&](std::vector<Cell>& cells,Rules::Kind kind) {
                    if(o&&o->tw==1&&o->th==1)append(cells,c,kind);
                    else footprint(cells,o,kind);
                };
                add_trigger(out.triggers,Rules::Hazard);
                if (o && baked::is_trap(*o)) add_trigger(out.traps,Rules::Trap);
            } else if (raw.type == kObjEntity || raw.type == kObjAnimEntity || raw.type == 11) {
                // Type11 uses the native effect's collision role. Capture has
                // already observed its ordinary Show/Hide lifecycle; it remains
                // excluded from the display-only destination detector chain.
                const auto* o = baked_by_entity_id(g, raw.id, static_cast<baked::ObjType>(raw.type), c, raw.raw_name);
                if (o && (o->d & kFlagCollision)) {
                    // Scripted single-tile walls/rocks can move without a map
                    // reload. Their current native tile, including ID zero,
                    // owns collision; the baked home must not remain closed.
                    if (o->tw == 1 && o->th == 1)append(out.gates,c,Rules::Gate);
                    else footprint(out.gates,o,Rules::Gate);
                }
            } else if (!sm.world && raw.chest)append(out.chests,c,Rules::Solid);
            else if (!sm.world && raw.npc) {
                const auto* o = baked_by_entity_id(g, raw.id, baked::ObjType::NPC, c, raw.raw_name);
                if (!o) o = baked_by_entity_id(g, raw.id, baked::ObjType::ShopNPC, c, raw.raw_name);
                if(raw.solidity_known ? !raw.solid : (o && !(o->d & kFlagCollision)))continue;
                if (o && (o->d & 2)) c.layer = -1;
                const bool wanders=raw.movement_known?raw.wanders:(o&&(o->d&kFlagWanders)!=0);
                append((!o || wanders)?out.moving_npcs:out.fixed_npcs,c,Rules::Solid);
                baked::Object interaction{};
                if(o)interaction=*o;
                else {interaction.type=baked::ObjType::NPC;interaction.b=15;interaction.tw=interaction.th=1;interaction.c=raw.id;}
                // A displaced patrol cannot inherit its home's counter relay.
                if(!o || wanders || (o->d&kFlagWanders) || o->tx!=c.x || o->ty!=c.y)interaction.a=0;
                interaction.tx=static_cast<int16_t>(c.x);interaction.ty=static_cast<int16_t>(c.y);
                interaction.layer=static_cast<uint8_t>(c.layer<0?g_route_snapshot->layer:c.layer);
                if(raw.applies)out.actors.push_back({c,interaction,raw.raw_name,wanders});
            }
        }
        if(!snapshot.transport_rules_prepared) {
            snapshot.transport_rules=std::move(prepared);snapshot.transport_rules_prepared=true;
        }
        out.transport_rules=snapshot.transport_rules;
        if(g_route_snapshot->anticipated_release) {
            // Leaving a pressure plate changes native geometry before the next
            // action. Project only those gates; keep captured NPCs at their real
            // positions. Hidden gates which the release shows must reappear too.
            baked_route::Request predicted;predicted.submap=&sm;
            map_state::add_obstacles(g_route_snapshot->state,sm,predicted,true,true);
            out.gates.insert(out.gates.end(),predicted.blocked.begin(),predicted.blocked.end());
            out.directional.insert(out.directional.end(),predicted.directional.begin(),predicted.directional.end());
            if(predicted.transport_rules) {
                auto combined=out.transport_rules?std::make_shared<Rules>(*out.transport_rules):std::make_shared<Rules>();
                baked_route::merge_transport_rules(*combined,*predicted.transport_rules);
                out.transport_rules=std::move(combined);
            }
        }
        return out;
    }
    // Live capture belongs to request_route; worker callers always have a snapshot.

    return out;
}

// --- reachability cache (Quick mode) --------------------------------------------------------
struct ReachCache {
    const baked::Submap* submap = nullptr;
    Cell start;
    bool passages = false;
    int vehicle_group = 0;
    int transportation_id=0;
    bool transport_gates=true;
    std::shared_ptr<const baked_route::TransportRules> transport_rules;
    std::vector<int> auto_groups;
    baked_route::Reach reach;
};
thread_local ReachCache* g_cache = nullptr;

const baked_route::Reach& cached_flood(const baked_route::Request& req)
{
    if (!g_cache) g_cache = new ReachCache();
    if (g_cache->submap != req.submap || !(g_cache->start == req.start) || g_cache->passages != req.passages ||
        g_cache->vehicle_group != req.vehicle_group || g_cache->auto_groups != req.auto_groups||
        g_cache->transportation_id!=req.transportation_id||g_cache->transport_rules!=req.transport_rules||g_cache->transport_gates!=req.transport_gates) {
        g_cache->submap = req.submap;
        g_cache->start = req.start;
        g_cache->passages = req.passages;
        g_cache->vehicle_group = req.vehicle_group;
        g_cache->transportation_id=req.transportation_id;g_cache->transport_rules=req.transport_rules;g_cache->transport_gates=req.transport_gates;
        g_cache->auto_groups = req.auto_groups;
        g_cache->reach = baked_route::flood(req);
    }
    return g_cache->reach;
}

/// A goal counts as reached when the flood reaches it or a cell beside it: a goal cell closed
/// on the grid (a town tile, an exit on a wall) is entered from its neighbour, as the router
/// enters it, and the cached flood was made for another target's goals.
bool any_reachable(const baked_route::Reach& reach, const std::vector<Cell>& goals)
{
    static const int kSide[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (const Cell& goal : goals) {
        if (reach.reachable(goal)) return true;
        for (const auto& s : kSide)
            if (reach.reachable(Cell{goal.layer, goal.x + s[0], goal.y + s[1]})) return true;
    }
    return false;
}

/// The player's movement rules on a world map now: riding a vehicle (its group) or walking
/// with the owned auto-vehicles.
void world_mode(baked_route::Request& req)
{
    req.vehicle_group = 0;
    req.vehicle_type = req.landing_group = 0;
    req.transportation_id = 0;
    req.auto_groups.clear();
    if (!req.submap || !req.submap->world) {
        req.transportation_id=g_route_snapshot&&g_route_snapshot->vehicle.valid?g_route_snapshot->vehicle.id:1;
        return;
    }
    req.auto_groups = planning_auto_groups();
    VehicleState v = planning_vehicle();
    if (v.valid && v.type != 1 && v.type != 0) {
        if (const baked::TransportRow* row = baked::transport(v.id)) {
            if (row->type != kTransportContent) {
                req.vehicle_group = row->group;
                req.transportation_id = row->id;
                req.vehicle_type = row->type; req.landing_group = row->landing_group;
                return;
            }
        }
    }
}

// Only ordinary entry and interaction targets require a foot approach. Vehicle
// pads, reviewed ship scenes, arbitrary spots and step events keep their own rules.
bool mounted_foot_target(const baked_route::Request& req, const Cell& target, TargetKind kind, bool face) {
    if(!req.submap||!req.submap->world||req.vehicle_group<=0||baked_route::native_ground_entry(req.vehicle_type))return false;
    if(face||kind==TargetKind::Interact)return true;
    if(kind==TargetKind::Spot)return false;
    const auto& g=baked::grids(*req.submap);
    return std::any_of(g.objects.begin(),g.objects.end(),[&](const baked::Object& o){
        return !baked::ship_entry(o)&&!baked::submarine_entry(o)&&(o.type==baked::ObjType::GotoMap||baked::world_exit(o))&&o.contains(target.x,target.y);
    });
}

/// Test hypothetical terrain access from an actually missing vehicle group,
/// cheapest first. Owned vehicles cannot explain a missing prerequisite; their
/// boarding/landing failures have separate constraints. Unknown future vehicles
/// retain the existing generic wording.
std::wstring vehicle_required(const baked::Grids& g, const baked_route::Request& req)
{
    std::vector<int> owned = planning_auto_groups();
    // A mounted vehicle can have no separate live parked entity. Its current
    // movement group is also positive ownership evidence from the snapshot.
    if (req.vehicle_group > 1) owned.push_back(req.vehicle_group);
    for (const VehicleOnMap& v : planning_vehicles())
        if (const baked::TransportRow* row = baked::transport(v.transportation_id)) owned.push_back(row->group);
    auto rank = [](int type) {
        switch (type) {
        case kTransportContent: return 0;
        case kTransportShip: return 1;
        case kTransportSubmarine: return 2;
        default: return 3;
        }
    };
    std::vector<const baked::TransportRow*> rows;
    for (const baked::TransportRow& row : baked::transports())
        if (row.type != 0 && row.type != 1 && row.group > 1 && row.group <= static_cast<int>(g.transport.size()))
            rows.push_back(&row);
    std::stable_sort(rows.begin(), rows.end(), [&](const baked::TransportRow* a, const baked::TransportRow* b) {
        return rank(a->type) < rank(b->type);
    });
    std::vector<int> tried;
    for (const baked::TransportRow* row : rows) {
        if (std::find(tried.begin(), tried.end(), row->group) != tried.end()) continue;
        tried.push_back(row->group);
        if (std::find(owned.begin(),owned.end(),row->group)!=owned.end()) continue;
        baked_route::Request with = req;
        with.blocked.clear();with.transport_gates=false;
        with.auto_groups.push_back(row->group);
        if (any_reachable(baked_route::flood(with), req.goals)) {
            return T(L"vehicle"); // unknown future identity; never blame an owned ship
        }
    }
    return L"";
}

/// Why no route was found, when the router knows: the story gates alone in the way, else the
/// NPCs that stand still, else (a world map) the vehicle that would reach the target.
std::optional<Phase> party_approach(const baked_route::Request& req,const Blockers& live,int& slot) {
    if(live.parties.empty())return {};
    auto without=req;without.blocked=live.hard(true,false);
    // A marker whose slot changed during capture is still solid. Never let a
    // named later blocker create a prefix which walks through that unknown one.
    for(const auto& party:live.parties)if(party.slot<0)without.blocked.push_back(party.cell);
    without.ignore_hazards=true;
    if(game::flags().has_hidden_passages)without.passages=true;
    const auto clear=baked_route::find(without);if(!clear.found)return {};
    for(size_t i=1;i<clear.cells.size();++i)for(const auto& party:live.parties) {
        const auto& cell=clear.cells[i];const auto& before=clear.cells[i-1];
        if(party.slot<0||cell.x!=party.cell.x||cell.y!=party.cell.y||(party.cell.layer>=0&&cell.layer!=party.cell.layer))continue;
        // A party advisory belongs to an actual next walking step, never a
        // distant/scripted landing or a general unreachable-target message.
        if(cell.layer!=before.layer||std::abs(cell.x-before.x)+std::abs(cell.y-before.y)!=1)return {};
        Phase prefix;prefix.cells.assign(clear.cells.begin(),clear.cells.begin()+static_cast<ptrdiff_t>(i));
        prefix.action_after=TF(L"Blocked by Party {0}.",{std::to_wstring(party.slot+1)});
        prefix.manual_after=true;slot=party.slot;return prefix;
    }
    return {};
}
// Build a wait/interaction prefix only from a demonstrated obstruction. Both
// ordinary directions and cross-floor routes share this owned-snapshot adapter.
std::optional<Phase> npc_approach(baked_route::Request request,const Blockers& live,
                                const std::vector<Cell>& proof) {
    request.blocked=live.hard();request.directional=live.directional;live.apply_transport(request);
    request.ignore_hazards=true;
    if(game::flags().has_hidden_passages)request.passages=true;
    auto approach=npc_route::approach(request,live.actors,proof);
    if(!approach)return {};
    auto label=npc_display_label_snapshot(translate_entity_name_snapshot(live.actors[approach->actor].raw_name,planning_map_id())).base;
    if(label.empty())label=T(L"NPC");
    approach->phase.action_after=TF(approach->phase.wait_for_npc?L"Wait for {0} to move.":L"Interact: {0}",{label});
    const auto& actor=live.actors[approach->actor];
    LOGF("[BakedNav] NPC %s: id %d at (%d,%d) L%d; %zu legal prefix cells",
        approach->phase.wait_for_npc?"wait":"approach",actor.interaction.entity_id(),
        actor.cell.x,actor.cell.y,actor.cell.layer,approach->phase.cells.size());
    if(approach->phase.arrival_direction>=0)
        approach->phase.arrival_facing=TF(L"face {0}",{T(direction_key(approach->phase.arrival_direction))});
    return std::move(approach->phase);
}
std::wstring no_route_reason(const baked::Submap& sm, const baked::Grids& g, const baked_route::Request& req, const Blockers& live)
{
    if (live.has_gates() || !live.fixed_npcs.empty() || !live.moving_npcs.empty()) {
        baked_route::Request without = req;
        without.blocked = live.hard(false);
        live.apply_transport(without,false);
        without.directional.clear();
        if (live.has_gates() && baked_route::find(without).found) return T(L"Story gated");
    }
    if (sm.world && req.vehicle_group == 0) {
        const std::wstring vehicle = vehicle_required(g, req);
        if (!vehicle.empty()) return TF(L"Requires {0}", {vehicle});
    }
    return L"";
}

std::vector<Vec3> world_path_of(const baked::Submap& sm, const std::vector<Cell>& cells)
{
    std::vector<Vec3> out;
    out.reserve(cells.size());
    for (const Cell& c : cells) out.push_back(world_of_cell(sm, c));
    return out;
}

// Pure worker reachability after legally leaving the already-ridden vehicle.
// Seed all reachable shores once; never flood the whole map separately per shore.
baked_route::Reach land_after_ride(const baked_route::Request& riding, const baked_route::Reach& ride)
{
    if (!riding.submap || !riding.submap->world || riding.vehicle_group <= 0 || !ride.submap) return {};
    if (game::is(game::Id::FF3) && baked_route::enterprise_pair(riding.transportation_id))
        return baked_route::enterprise_route(riding,false).foot;
    const auto& g = baked::grids(*riding.submap);
    auto foot = riding;
    foot.vehicle_group = 0; foot.vehicle_type = foot.landing_group = 0;
    foot.goals.clear();
    const bool sails = riding.vehicle_type == kTransportShip || riding.vehicle_type == kTransportSubmarine;
    std::vector<Cell> starts;
    std::vector<int> costs;
    constexpr int sides[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
    for (int y=0; y<g.height; ++y) {
        if (planning_cancelled() || (riding.cancelled && riding.cancelled())) return {};
        for (int x=0; x<g.width; ++x) {
            const Cell shore{riding.start.layer,x,y};
            if (!baked_route::open(foot,g,shore) ||
                (riding.landing_group > 0 && !baked::group_lands(riding.landing_group,baked::attribute(g,x,y)))) continue;
            if (std::any_of(foot.blocked.begin(),foot.blocked.end(),[&](const auto& cell) {
                return cell.x==x&&cell.y==y&&(cell.layer<0||cell.layer==shore.layer);
            }) || baked_route::trap_cell(foot,g,shore)) continue;
            if (!foot.ignore_hazards) {
                if (foot.live_hazards) {
                    if (std::any_of(foot.hazards.begin(),foot.hazards.end(),[&](const auto& cell) {
                        return cell.x==x&&cell.y==y&&(cell.layer<0||cell.layer==shore.layer);
                    })) continue;
                } else if (!g.hazard.empty()&&g.hazard[static_cast<size_t>(y)*g.width+x]) continue;
            }
            if(baked_route::terminal_transition(foot,g,shore))continue;
            int cost = !sails ? ride.distance(shore) : -1;
            if (sails) for (const auto& side : sides) {
                const Cell water{riding.start.layer,(x+side[0]+g.width)%g.width,(y+side[1]+g.height)%g.height};
                const int distance = ride.distance(water);
                if (distance>=0 && baked_route::direction_allowed(foot,water,shore) && (cost<0 || distance+1<cost)) cost=distance+1;
            }
            if (cost>=0) { starts.push_back(shore); costs.push_back(cost); }
        }
    }
    return baked_route::flood(foot,starts,costs);
}

// List preparation needs distances, not a complete route for every exit. Flood
// each reachable parked vehicle and its legal shores once, then keep the best
// owned distance per cell. Memory is bounded by the current baked world grid;
// no live pointers or cross-request cache/invalidation assumptions are involved.
baked_route::Reach parked_vehicle_reach(const baked_route::Request& walking, const baked_route::Reach& walk) {
    if (!walking.submap || !walking.submap->world || walking.vehicle_group || !walk.submap) return {};
    const auto& sm=*walking.submap;
    const auto& grid=baked::grids(sm);
    const bool chained=game::is(game::Id::FF3)&&g_route_snapshot;
    const auto vehicles=chained?journey_vehicles():planning_vehicles();
    if(chained&&vehicles.size()>32)return {};
    baked_route::Reach best,combined=walk;
    std::vector<bool> done(vehicles.size(),false);
    constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
    for(size_t expanded=0;expanded<vehicles.size();++expanded) {
        if(planning_cancelled())return {};
        int chosen=-1,approach=INT_MAX;
        for(size_t index=0;index<vehicles.size();++index) {
            if(done[index])continue;
            const auto* row=baked::transport(vehicles[index].transportation_id);
            if(!row||row->type==kTransportContent||row->type==1||row->group<1||row->group>static_cast<int>(grid.transport.size()))continue;
            if(game::is(game::Id::FF3)&&(row->id==6||row->id==15))continue;
            const auto at=cell_of(sm,vehicles[index].pos,walking.start.layer);
            if(!baked::in_bounds(grid,at.x,at.y))continue;
            for(const auto& side:sides) {
                const Cell beside{at.layer,(at.x+side[0]+grid.width)%grid.width,(at.y+side[1]+grid.height)%grid.height};
                if(!baked_route::open(walking,grid,beside)||(chained&&baked_route::terminal_transition(walking,grid,beside)))continue;
                if(game::is(game::Id::FF3)&&baked_route::enterprise_pair(row->id)&&!baked_route::direction_allowed(walking,beside,at))continue;
                const int distance=(chained?combined:walk).distance(beside);
                if(distance>=0&&distance<approach){chosen=static_cast<int>(index);approach=distance;}
            }
        }
        if(chosen<0)break;
        done[static_cast<size_t>(chosen)]=true;
        const auto& vehicle=vehicles[static_cast<size_t>(chosen)];const auto* row=baked::transport(vehicle.transportation_id);
        auto riding=walking;riding.start=cell_of(sm,vehicle.pos,walking.start.layer);riding.goals.clear();riding.goals_open=false;
        riding.vehicle_group=row->group;riding.vehicle_type=row->type;riding.landing_group=row->landing_group;riding.transportation_id=row->id;
        const auto sea=baked_route::flood(riding);const auto land=land_after_ride(riding,sea);
        if(planning_cancelled())return {};
        if(!land.submap)continue;
        if(!best.submap) {
            best.submap=&sm;best.width=grid.width;best.height=grid.height;
            best.dist[0].assign(static_cast<size_t>(grid.width)*grid.height,0xffff);
        }
        for(size_t i=0;i<land.dist[0].size();++i) {
            if(planning_cancelled())return {};
            if(land.dist[0][i]==0xffff)continue;
            const auto cost=static_cast<uint16_t>(std::min(0xfffe,approach+1+land.dist[0][i]));
            best.dist[0][i]=std::min(best.dist[0][i],cost);
            if(chained&&i<combined.dist[0].size())combined.dist[0][i]=std::min(combined.dist[0][i],cost);
        }
    }
    return best;
}

/// Full mode vehicle candidate, for an unreachable target or a shorter legal
/// alternative to walking: board a parked ship, submarine or airship. Walk beside it, ride
/// to a cell from which the target's walkable area is entered (a ship: a water cell beside a
/// land cell of that area the ship may stop at; an airship: a land cell of that area it may
/// land on), then walk. The shortest total over the vehicles the party has on this map.
bool boarding_route(const baked::Submap& sm, const baked::Grids& g, const baked_route::Request& walk,
                    const std::vector<Cell>& goals, std::vector<Phase>& phases,
                    const air_transitions::Rule* aerial=nullptr)
{
    const bool riding = walk.vehicle_group > 0;
    auto foot = walk;
    foot.vehicle_group = 0; foot.vehicle_type = foot.landing_group = 0;
    foot.transportation_id=0;
    foot.goals_open = true;
    std::vector<VehicleOnMap> vehicles;
    if (riding) {
        const auto current = planning_vehicle();
        VehicleOnMap vehicle;
        vehicle.transportation_id = current.id; vehicle.type = current.type;
        vehicle.message_id = current.message_id; vehicle.pos = snapshot_position();
        if (current.valid) vehicles.push_back(std::move(vehicle));
    } else vehicles = planning_vehicles();
    if (vehicles.empty()) return false;

    const bool ship_scene=!goals.empty()&&std::all_of(goals.begin(),goals.end(),[&](const Cell& cell) {
        return ship_goal_at(sm,cell).has_value()||std::any_of(g.objects.begin(),g.objects.end(),[&](const baked::Object& object) {
            return baked::ship_entry(object)&&object.contains(cell.x,cell.y);
        });
    });
    const bool submarine_scene=!goals.empty()&&std::all_of(goals.begin(),goals.end(),[&](const Cell& cell) {
        return std::any_of(g.objects.begin(),g.objects.end(),[&](const baked::Object& object) {
            return baked::submarine_entry(object)&&object.contains(cell.x,cell.y);
        });
    });
    const auto* action_pad=goals.empty()?nullptr:vehicle_pad_at(sm,goals.front());
    // A sea scene ends aboard; ordinary destinations still need a landing and
    // final walking area. Do not flood a fictitious walking area from the sea.
    baked_route::Request from_target = foot;
    from_target.goals.clear();
    from_target.start = goals.front();
    baked_route::Reach land;
    if(!ship_scene&&!submarine_scene&&!action_pad&&!aerial)land=baked_route::flood(from_target,goals);

    int best_total = 1 << 30;
    int best_actions=INT_MAX;
    const auto improves=[&](int actions,int total){return actions<best_actions||(actions==best_actions&&total<best_total);};
    std::vector<Phase> best;
    for (const VehicleOnMap& v : vehicles) {
        const baked::TransportRow* row = baked::transport(v.transportation_id);
        if (!row || row->type == kTransportContent || row->type == 1) continue;
        const int eligible_air=row->id==13?6:row->id==16?15:row->id;
        if(aerial&&!air_transitions::accepts(*aerial,eligible_air))continue;
        if(ship_scene&&row->type!=kTransportShip&&!(game::is(game::Id::FF3)&&baked_route::enterprise_pair(row->id)))continue;
        if(ship_scene&&std::any_of(goals.begin(),goals.end(),[&](const Cell& cell){
            const auto goal=ship_goal_at(sm,cell);
            const int id=game::is(game::Id::FF3)&&baked_route::enterprise_pair(row->id)?
                (row->id==15||row->id==16?16:13):row->id;
            return goal&&!(goal->ids&baked_route::transportation_bit(id));
        }))continue;
        if(submarine_scene&&row->type!=kTransportSubmarine)continue;
        if(action_pad&&!baked::vehicle_pad_accepts(*action_pad,row->id))continue;
        // Native NotPossibleVoluntaryGetOffTable contains Boko/Magitek. Their
        // map-entry trigger tables permit ordinary doors while still mounted;
        // do not fabricate a generic landing action for a failed direct route.
        if(riding&&(row->type==11||row->type==12))continue;
        const int group = row->group;
        if (group < 1 || group > static_cast<int>(g.transport.size())) continue;
        const bool sails = row->type == kTransportShip || row->type == kTransportSubmarine;
        Cell vcell = cell_of(sm, v.pos, walk.start.layer);
        if (!baked::in_bounds(g, vcell.x, vcell.y)) continue;

        // 1. Walk to a cell beside the vehicle.
        baked_route::Request to_vehicle = foot;
        to_vehicle.goals.clear();
        // A boarding approach is an ordinary standable tile. An adjacent town
        // entrance changes maps and cannot be made legal by goal substitution.
        to_vehicle.goals_open=false;
        static const int kSide[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
        for (const auto& d : kSide) {
            Cell c{walk.start.layer, vcell.x + d[0], vcell.y + d[1]};
            c.x = (c.x + g.width) % g.width; c.y = (c.y + g.height) % g.height;
            if (baked_route::open(foot, g, c) &&
                !(game::is(game::Id::FF3)&&baked_route::enterprise_pair(row->id)&&sails&&!baked_route::direction_allowed(foot,c,vcell)))
                to_vehicle.goals.push_back(c);
        }
        baked_route::Route leg1;
        if (!riding) {
            if (to_vehicle.goals.empty()) continue;
            leg1 = baked_route::find(to_vehicle);
            if (!leg1.found) continue;
        }

        // The selected parked boat can itself be the destination. The general
        // transfer planner below requires a walkable area after landing; applying
        // that requirement to the boat's water tile falsely reports a missing
        // Canoe even when its owned auto-use group reaches the boarding shore.
        // Use the same native WalkOn edge as an ordinary transfer, then stop at
        // this exact boat position without inventing a ride/landing round trip.
        if (!aerial && !riding && sails && std::find(goals.begin(),goals.end(),vcell)!=goals.end() &&
            std::find(walk.blocked.begin(),walk.blocked.end(),vcell)==walk.blocked.end()) {
            const auto approach=leg1.cells.back();
            if (!baked_route::direction_allowed(foot,approach,vcell)) continue;
            const int total=leg1.steps()+1;
            if(!improves(0,total))continue;
            auto name=planning_vehicle_name(v.transportation_id,v.type,v.message_id);
            if(name.empty())name=T(L"Vehicle");
            Phase before;before.cells=leg1.cells;
            Phase board;board.cells={approach,vcell};board.action_cell=approach;
            before.movement_transportation_id=0;board.movement_transportation_id=v.transportation_id;
            board.transportation_id=v.transportation_id;
            board.vehicle_before=route_progress::VehicleAction::WalkOn;
            board.vehicle_direction=baked_route::step_direction(approach,vcell,g.width,g.height);
            board.action_before=TF(L"board the {0}",{name})+L" "+T(direction_key(board.vehicle_direction));
            best_actions=0;best_total=total;best={std::move(before),std::move(board)};
            continue;
        }

        // The observed upgraded Enterprise identity alone authorizes its ordered
        // flight conversions. Parked flight decks retain the existing manual path.
        if(game::is(game::Id::FF3)&&baked_route::enterprise_pair(row->id)&&!action_pad&&
           (riding||sails)) {
            auto request=walk; request.start=vcell;request.goals=goals;request.transportation_id=row->id;
            // The final foot or ship-scene goal has its own entry policy; the
            // original air movement deliberately keeps town goals closed.
            // Airborne triggers use actual flight terrain; the ordinary
            // goal-open override applies ground/ship entry rules instead.
            request.goals_open=!aerial;
            auto route=baked_route::enterprise_route(request,true,ship_scene,aerial!=nullptr);
            if(route.steps<0||route.segments.empty())continue;
            const int total=leg1.steps()+route.steps+(riding?0:1);
            if(!improves(0,total))continue;
            std::vector<Phase> candidate;
            if(!riding) { Phase before;before.cells=leg1.cells;before.movement_transportation_id=0;candidate.push_back(std::move(before)); }
            const int ship=(row->id==15||row->id==16)?16:13, air=ship==13?6:15;
            for(size_t i=0;i<route.segments.size();++i) {
                const auto& segment=route.segments[i]; Phase p;p.cells=segment.cells;
                p.transportation_id=segment.mode==1?air:ship;
                p.movement_transportation_id=segment.mode==3?0:segment.mode==1?air:ship;
                if(i==0&&!riding) {
                    const auto approach=leg1.cells.back();
                    if(!baked_route::direction_allowed(foot,approach,vcell)) { candidate.clear();break; }
                    p.cells.insert(p.cells.begin(),approach);p.action_cell=approach;
                    p.vehicle_before=route_progress::VehicleAction::WalkOn;
                    p.vehicle_direction=baked_route::step_direction(approach,vcell,g.width,g.height);
                    auto name=planning_vehicle_name(v.transportation_id,v.type,v.message_id);if(name.empty())name=T(L"Vehicle");
                    p.action_before=TF(L"board the {0}",{name})+L" "+T(direction_key(p.vehicle_direction));
                } else if(i>0) {
                    const Cell prior=route.segments[i-1].cells.back(); p.action_cell=prior;
                    if(segment.mode==1||segment.mode==2) {
                        p.vehicle_before=route_progress::VehicleAction::ConfirmBoard;
                        p.action_before=T(segment.mode==1?L"take off":L"land on water");
                    } else if(segment.mode==3) {
                        p.cells.insert(p.cells.begin(),prior);
                        p.vehicle_before=route_progress::VehicleAction::WalkOff;
                        p.vehicle_direction=baked_route::step_direction(prior,segment.cells.front(),g.width,g.height);
                        p.action_before=T(L"land")+L" "+T(direction_key(p.vehicle_direction));
                    }
                }
                candidate.push_back(std::move(p));
            }
            if(candidate.empty())continue;
            best_actions=0;best_total=total;best=std::move(candidate);continue;
        }

        // 2. Ride to where the target's area is entered.
        baked_route::Request ride;
        ride.cancelled = walk.cancelled;
        ride.submap = &sm;
        ride.transitions=walk.transitions;
        ride.transport_rules=walk.transport_rules;ride.transport_gates=walk.transport_gates;
        ride.vehicle_group = group;
        ride.vehicle_type=row->type;ride.landing_group=row->landing_group;
        ride.transportation_id=row->id;
        ride.start = vcell;
        ride.goals_open = false;
        ride.blocked = walk.blocked; ride.directional = walk.directional;
        ride.live_hazards = walk.live_hazards; ride.hazards = walk.hazards;
        ride.live_traps = walk.live_traps; ride.traps = walk.traps;
        ride.allow_trap_goal = walk.allow_trap_goal;
        ride.terminal_goal = aerial!=nullptr; // keep the exact airborne event endpoint through boarding
        ride.ignore_hazards = walk.ignore_hazards;
        if(ship_scene||submarine_scene||action_pad||aerial) {
            // Reuse the normal native WalkOn boarding edge, then sail all the
            // way to the reviewed trigger. Its script owns the later dismount.
            ride.vehicle_type=row->type;ride.landing_group=row->landing_group;
            ride.goals=goals;ride.goals_open=ship_scene||submarine_scene;
            auto sailing=baked_route::find(ride);
            if(!sailing.found)continue;
            const int total=leg1.steps()+sailing.steps()+(riding?0:1);
            if(!improves(sailing.mountain_actions,total))continue;
            Phase aboard;aboard.cells=std::move(sailing.cells);aboard.transportation_id=v.transportation_id;
            aboard.movement_transportation_id=v.transportation_id;
            vehicle_pad_terminal(aboard,action_pad);
            std::vector<Phase> candidate;
            if(!riding) {
                const auto approach=leg1.cells.back();
                if(sails&&!baked_route::direction_allowed(foot,approach,vcell))continue;
                auto name=planning_vehicle_name(v.transportation_id,v.type,v.message_id);if(name.empty())name=T(L"Vehicle");
                Phase before;before.cells=leg1.cells;before.movement_transportation_id=0;candidate.push_back(std::move(before));
                if(sails)aboard.cells.insert(aboard.cells.begin(),approach);aboard.action_cell=approach;
                const bool deck=game::in(game::kFF3|game::kFF6)&&(row->type==3||row->type==7||row->type==8);
                aboard.vehicle_before=deck?route_progress::VehicleAction::None:sails?route_progress::VehicleAction::WalkOn:route_progress::VehicleAction::ConfirmBoard;
                aboard.vehicle_direction=baked_route::step_direction(approach,vcell,g.width,g.height);
                aboard.action_before=TF(L"board the {0}",{name})+L" "+T(direction_key(aboard.vehicle_direction));
            }
            candidate.push_back(std::move(aboard));best_actions=sailing.mountain_actions;best_total=total;best=std::move(candidate);
            continue;
        }
        const auto reachable_ride = baked_route::flood(ride);
        int best_landing = INT_MAX,best_walking=INT_MAX,best_crossings=INT_MAX;
        Cell land_start;
        for (int y = 0; y < g.height; ++y) {
            if (walk.cancelled && walk.cancelled()) return false;
            for (int x = 0; x < g.width; ++x) {
                Cell here{walk.start.layer, x, y};
                if (!land.reachable(here) || !baked_route::open(foot, g, here)) continue;
                if(baked_route::terminal_transition(foot,g,here))continue;
                const int attr = baked::attribute(g, x, y);
                if (sails) {
                    // A water cell beside this land cell, where the ship may stop.
                    for (const auto& d : kSide) {
                        Cell w{walk.start.layer, x + d[0], y + d[1]};
                        w.x = (w.x + g.width) % g.width; w.y = (w.y + g.height) % g.height;
                        const int distance = reachable_ride.distance(w);
                        if (distance < 0 || !baked::vehicle_open(g, group, w.x, w.y)) continue;
                        if (row->landing_group > 0 && !baked::group_lands(row->landing_group, attr)) continue;
                        if (!baked_route::direction_allowed(foot, w, here)) continue;
                        const int cost = distance + 1 + land.distance(here);
                        if (cost < best_landing) { best_landing = cost; ride.goals = {w}; land_start = here; }
                    }
                } else {
                    const int distance = reachable_ride.distance(here);
                    if (distance < 0 || !baked::vehicle_open(g, group, x, y)) continue;
                    if (row->landing_group > 0 && !baked::group_lands(row->landing_group, attr)) continue;
                    const int cost = distance + land.distance(here);
                    const int walking=land.distance(here);
                    const int crossings=reachable_ride.crossings(here);
                    // Choose an ordinary-flight landing before a mountain
                    // shortcut. For equal crossing/tile costs prefer the
                    // shortest final walk for aircraft too: scanning west-to-
                    // east must not choose an early landing and a long walk.
                    if(crossings<best_crossings||(crossings==best_crossings&&
                       (cost<best_landing||(cost==best_landing&&walking<best_walking)))) {
                        best_crossings=crossings;
                        best_landing=cost;best_walking=walking;ride.goals={here};land_start=here;
                    }
                }
            }
        }
        if (ride.goals.empty()) continue;
        baked_route::Route leg2 = baked_route::find(ride);
        if (!leg2.found) continue;

        // 3. Walk from the landing to the target. A ship's stop is a water cell: the first step
        //    of this leg walks off it onto the land cell beside it (a start cell is never
        //    tested for openness, only the cells entered).
        baked_route::Request last = foot;
        const Cell stop = leg2.cells.back();
        last.start = land_start;
        baked_route::Route leg3 = baked_route::find(last);
        if (!leg3.found) continue;
        if (sails) leg3.cells.insert(leg3.cells.begin(), stop);

        const int total = leg1.steps() + leg2.steps() + leg3.steps();
        if(!improves(leg2.mountain_actions,total))continue;
        best_actions=leg2.mountain_actions;
        best_total = total;
        best.clear();
        std::wstring name = planning_vehicle_name(v.transportation_id, v.type, v.message_id);
        if (name.empty()) name = T(L"Vehicle");
        Phase p1;
        p1.movement_transportation_id=0;
        p1.cells = leg1.cells;
        Phase p2;
        p2.cells = leg2.cells;
        p2.transportation_id=v.transportation_id;
        p2.movement_transportation_id=v.transportation_id;
        if (!riding) {
            p2.action_before = TF(L"board the {0}", {name});
            p2.transportation_id = v.transportation_id;
            // These games board onto an airship deck; reaching the flight controls is manual.
            const bool deck = game::in(game::kFF3 | game::kFF6) && (row->type == 3 || row->type == 7 || row->type == 8);
            p2.vehicle_before = deck ? route_progress::VehicleAction::None :
                sails ? route_progress::VehicleAction::WalkOn : route_progress::VehicleAction::ConfirmBoard;
            const auto approach = leg1.cells.back();
            p2.action_cell = approach;
            int dx = vcell.x - approach.x, dy = vcell.y - approach.y;
            if (std::abs(dx) > 1) dx = dx > 0 ? -1 : 1;
            if (std::abs(dy) > 1) dy = dy > 0 ? -1 : 1;
            p2.vehicle_direction = dy < 0 ? 0 : dx > 0 ? 1 : dy > 0 ? 2 : 3;
            p2.action_before += L" " + T(direction_key(p2.vehicle_direction));
            if (sails) p2.cells.insert(p2.cells.begin(), approach);
        }
        Phase p3;
        p3.cells = leg3.cells;
        p3.action_before = T(row->type==9||row->type==10?L"dismount":L"land");
        p3.transportation_id = v.transportation_id;
        p3.movement_transportation_id=0;
        p3.action_cell = stop;
        p3.vehicle_before = sails ? route_progress::VehicleAction::WalkOff :
            game::is(game::Id::FF6) && (row->type == 3 || row->type == 8) ?
                route_progress::VehicleAction::BackLand : route_progress::VehicleAction::ConfirmLand;
        if (sails) {
            int dx = land_start.x - stop.x, dy = land_start.y - stop.y;
            if (std::abs(dx) > 1) dx = dx > 0 ? -1 : 1;
            if (std::abs(dy) > 1) dy = dy > 0 ? -1 : 1;
            p3.vehicle_direction = dy < 0 ? 0 : dx > 0 ? 1 : dy > 0 ? 2 : 3;
            p3.action_before += L" " + T(direction_key(p3.vehicle_direction));
        }
        if (!riding) best.push_back(p1);
        best.push_back(p2);
        best.push_back(p3);
    }
    if (best.empty()) return false;
    phases = std::move(best);
    prepare_mountain_phases(sm,phases);
    return true;
}

#include "world_vehicle_journey.inl"

// A native mountain action skips closed cells. Keep it as a manual boundary,
// never a fictitious sequence of ordinary movement inputs. The following phase
// starts at the real far side; facing, costs, beacon and continuation share it.
void prepare_mountain_phases(const baked::Submap& sm,std::vector<Phase>& phases) {
    if(!sm.world||baked::game_number()!=3)return;
    if(std::none_of(phases.begin(),phases.end(),[](const Phase& phase){
        const int id=phase.movement_transportation_id<0?phase.transportation_id:phase.movement_transportation_id;
        return (id==10||id==11)&&phase.cells.size()>1;
    }))return; // other vehicles/foot routes do not allocate or copy phase data
    const auto& grid=baked::grids(sm);
    std::vector<Phase> prepared;bool changed=false;
    for(const auto& original:phases) {
        const int id=original.movement_transportation_id<0?original.transportation_id:original.movement_transportation_id;
        if((id!=10&&id!=11)||original.cells.size()<2){prepared.push_back(original);continue;}
        const auto* row=baked::transport(id);if(!row){prepared.push_back(original);continue;}
        baked_route::Request movement;movement.submap=&sm;movement.transportation_id=id;
        movement.vehicle_group=row->group;movement.vehicle_type=row->type;
        Phase segment=original;segment.cells={original.cells.front()};
        for(size_t i=1;i<original.cells.size();++i) {
            if(planning_cancelled())return;
            const auto from=original.cells[i-1],to=original.cells[i];
            if(baked_route::step_direction(from,to,sm.width,sm.height)>=0||from==to){segment.cells.push_back(to);continue;}
            const int direction=baked_route::direction_towards(from,to,sm.width,sm.height);
            const auto jump=baked_route::mountain_edge(movement,grid,from,direction);
            if(!jump||!(jump->to==to)){segment.cells.push_back(to);continue;}
            segment.action_after=TF(L"fly over the mountains {0}",{T(direction_key(direction))});
            segment.manual_after=true;segment.wait_for_npc=false;segment.arrival_direction=direction;
            segment.arrival_facing=TF(L"Face {0}",{T(direction_key(direction))});
            segment.manual_displacement_steps=jump->distance;
            prepared.push_back(std::move(segment));changed=true;
            segment=Phase{};segment.cells={to};segment.transportation_id=original.transportation_id;
            segment.movement_transportation_id=id;
        }
        // Only the final segment owns the original terminal interaction/facing.
        segment.action_after=original.action_after;segment.manual_after=original.manual_after;
        segment.arrival_facing=original.arrival_facing;segment.arrival_direction=original.arrival_direction;
        segment.manual_displacement_steps=original.manual_displacement_steps;
        segment.wait_for_npc=original.wait_for_npc;
        prepared.push_back(std::move(segment));
    }
    if(changed)phases=std::move(prepared);
}

int phase_steps(const std::vector<Phase>& phases) {
    int steps=0;
    for(const auto& phase:phases)steps+=route_progress::steps(phase);
    return steps;
}
bool may_shorten_with_vehicle(const baked_route::Request& walking,int steps) {
    if(!g_route_snapshot || !g_route_snapshot->vehicle_shortcuts ||
       !walking.submap->world || walking.vehicle_group>0 || steps<15 ||
       g_route_snapshot->goal.transportation_id>0)return false;
    // Human policy: a nearby destination stays on foot when walking is legal.
    // This guard applies only after walking succeeds; required crossings and
    // guidance for a vehicle already being ridden use their existing planners.
    const auto& sm=*walking.submap;
    auto distance=[&](const Cell& a,const Cell& b) {
        const int dx=std::abs(a.x-b.x),dy=std::abs(a.y-b.y);
        return std::min(dx,sm.width-dx)+std::min(dy,sm.height-dy);
    };
    for(const auto& goal:walking.goals)if(distance(walking.start,goal)<15)return false;
    for(const auto& vehicle:g_route_snapshot->vehicles) {
        if(planning_cancelled())return false;
        const auto* row=baked::transport(vehicle.transportation_id);
        if(!row||row->type==1||row->type==kTransportContent)continue;
        const auto at=cell_of(sm,vehicle.pos,walking.start.layer);
        if(!baked::in_bounds(baked::grids(sm),at.x,at.y))continue;
        // Admissible geometry only: boarding from beside the model can be
        // instantaneous, and all intervening terrain is optimistically open.
        // This prunes provably unhelpful comparisons, never distant routes.
        const int approach=std::max(0,distance(walking.start,at)-1);
        for(const auto& goal:walking.goals)if(approach+distance(at,goal)<steps)return true;
    }
    return false;
}

} // namespace

void preload()
{
    if (g_load_started) return;
    g_load_started = true;
    if (!background::post([] {
        const double started = frame::now();
        data::prepare_game_json();
        field::prepare_entity_translations();
        g_blob = new std::string(data::game_file("maps"));
        g_loaded = !g_blob->empty() && baked::load(g_blob->data(), g_blob->size());
        if (g_loaded) {
            // Decode once here. All readers, including worker projections, then use
            // immutable tables; map transitions never allocate/decode whole grids.
            for (int i = 0; i < baked::submap_count(); ++i) (void)baked::grids(*baked::submap_at(i));
        }
        LOGF("[BakedMap] %s prepared on data worker: %d maps, %.1f ms, loaded=%d",
             game::key(), g_loaded ? baked::submap_count() : 0, (frame::now() - started) * 1000, g_loaded);
        g_prepared.store(true, std::memory_order_release);
    })) {
        LOGF("[BakedMap] cannot start data worker; navigation preparation unavailable");
        g_load_started = false;
    }
}
bool prepared() { return g_prepared.load(std::memory_order_acquire); }
bool available(void* map_handle) { return current_submap(map_handle) != nullptr; }

bool player_cell(void* map_handle, void* player, Cell& out)
{
    const baked::Submap* sm = current_submap(map_handle);
    if (g_route_snapshot) { out = cell_of(*sm, snapshot_position(), snapshot_layer()); return true; }
    if (!sm || !player) return false;
    void* tr = unity::transform(player);
    if (!tr) return false;
    out = cell_of(*sm, unity::local_position(tr), sm->world ? 1 : player_layer(player));
    return true;
}

bool cell_at(void* map_handle, Vec3 world, Cell& out)
{
    if(!std::isfinite(world.x)||!std::isfinite(world.y))return false;
    const baked::Submap* sm = current_submap(map_handle);
    if (!sm) return false;
    out = cell_of(*sm, world, 1);
    return out.x>=0 && out.y>=0 && out.x<sm->width && out.y<sm->height;
}

Vec3 world_of(void* map_handle, const Cell& cell)
{
    const baked::Submap* sm = current_submap(map_handle);
    return sm ? world_of_cell(*sm, cell) : Vec3{};
}

std::vector<Cell> goal_cells(void* map_handle, Vec3 target)
{
    const baked::Submap* sm = current_submap(map_handle);
    if (!sm) return {};
    return goals_for(*sm, cell_of(*sm, target, 1), TargetKind::Infer).cells;
}

std::wstring describe(const std::vector<Phase>& phases, bool full)
{
    const auto* map = snapshot_submap();
    const int width = map && map->world ? map->width : 0;
    const int height = map && map->world ? map->height : 0;
    std::vector<std::wstring> parts;
    std::vector<std::vector<baked_route::Leg>> movement;
    size_t total_legs = 0;
    int total_steps = 0;
    for (const Phase& p : phases) {
        if(planning_cancelled())return {};
        auto legs=baked_route::legs(p.cells,width,height,&planning_cancelled);
        // The spoken board/land action already performs this one shore step.
        // Keep physical phase cells intact for native movement and route reuse.
        if (!p.action_before.empty() && p.action_cell && !p.cells.empty() &&
            *p.action_cell==p.cells.front() && !legs.empty() &&
            (p.vehicle_before==route_progress::VehicleAction::WalkOn || p.vehicle_before==route_progress::VehicleAction::WalkOff)) {
            if (--legs.front().count==0) legs.erase(legs.begin());
        }
        total_legs += legs.size()+(p.manual_displacement_steps>0?1:0);
        movement.push_back(std::move(legs));
        total_steps += route_progress::steps(p);
    }
    // Only the short spoken summary is abbreviated after six direction runs.
    // Geometry, full directions and destination reachability remain complete.
    const size_t cap = !full && total_steps > kLongRouteSpeechThreshold ? kMaxSpokenLegs : total_legs;
    size_t spoken = 0;
    for (size_t pi = 0; pi < phases.size(); ++pi) {
        const Phase& p = phases[pi];
        if (!p.action_before.empty()) parts.push_back(p.action_before);
        bool whole = true;
        for (const baked_route::Leg& leg : movement[pi]) {
            if (spoken >= cap) {
                whole = false;
                break;
            }
            parts.push_back(text::num(leg.count) + L" " + T(direction_key(leg.dir)));
            ++spoken;
        }
        if (!whole) break;
        if(p.manual_displacement_steps>0) {
            if(spoken>=cap)break;
            ++spoken; // a manual mountain crossing is one concise route action
        }
        if (!p.action_after.empty()) parts.push_back(p.action_after);
        if (route_progress::at_interaction(phases, pi) && !p.arrival_facing.empty()) parts.push_back(p.arrival_facing);
    }
    if (parts.empty()) return T(L"No movement needed");
    std::wstring out = text::join(parts, L", ");
    if (total_legs > spoken) out += L", " + TF(L"and {0} more", {text::num(static_cast<long long>(total_legs - spoken))});
    return out;
}

void invalidate()
{
    if (g_cache) g_cache->submap = nullptr;
    invalidate_active();
    cancel_route_work();
    cancel_follow_work();
    cancel_quick_work();
    cancel_browse_route();
}

// --- across maps -----------------------------------------------------------------------------
namespace {

/// An exit or warp of a sub-map, as the cross-map graph sees it.
struct Edge {
    const baked::Object* object;
    int dest_map_id;
    int dest_point;
    bool warp;        // a script warp (step-on or examined), else a GotoMap exit
};

std::vector<Edge> edges_of(const baked::Grids& g, int map_id)
{
    std::vector<Edge> out;
    for (const baked::Object& o : g.objects) {
        if (o.type == baked::ObjType::GotoMap && o.a > 0) out.push_back(Edge{&o, o.a, o.b, false});
        else if (o.type == baked::ObjType::Warp && (o.a > 0 || o.a == -1)) out.push_back(Edge{&o, o.a == -1 ? map_id : o.a, o.b, true});
    }
    return out;
}

/// The PointIn of a sub-map with this point id (the arrival cell of an exit), or null.
const baked::Object* point_in(const baked::Submap& sm, int point, map_state::Resolver& state)
{
    const auto& g = baked::grids(sm);
    const baked::Object* first = nullptr;
    for (const baked::Object& o : g.objects) {
        if (o.type != baked::ObjType::PointIn) continue;
        if (!state.active(sm, o)) continue;
        if (o.a == point) return &o;
        if (!first) first = &o;
    }
    return point <= 0 ? first : nullptr;
}

/// Distance from a flood to the nearest cell from which `o` is entered: the object's own
/// cells, or (a closed exit cell, a town tile) a neighbouring open cell plus one.
int distance_to_object(const baked_route::Request& request, const baked_route::Reach& reach, const baked::Object& o, int layer, bool world)
{
    int best = -1;
    if (!request.submap) return best;
    // An examined travel object is solid at its own cell. Its graph edge starts
    // at a legal interaction approach, including a reviewed facing restriction.
    if(o.type==baked::ObjType::Warp && (o.d&255)!=1) {
        const auto approach=baked_route::goals_for(request,{o.layer,o.tx,o.ty},baked_route::GoalKind::Interact,&o);
        for(const auto& cell:approach.cells) {
            const int distance=reach.distance(cell);
            if(distance>=0&&(best<0||distance<best))best=distance;
        }
        return best;
    }
    const auto& grid=baked::grids(*request.submap);
    for (int y = o.ty; y < o.ty + o.th; ++y) {
        for (int x = o.tx; x < o.tx + o.tw; ++x) {
            if (!baked::in_bounds(grid,x,y)) continue;
            const int l = world ? layer : o.layer;
            bool blocked = false;
            for (const Cell& c : request.blocked)
                if (c.x == x && c.y == y && (c.layer < 0 || c.layer == l)) { blocked = true; break; }
            if (blocked) continue; // A closed exit is enterable; a solid NPC/gate over it is not.
            int d = reach.distance(Cell{l, x, y});
            if (d < 0) {
                static const int kSide[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
                for (const auto& s : kSide) {
                    if (!baked_route::direction_allowed(request, Cell{l, x+s[0], y+s[1]}, Cell{l,x,y})) continue;
                    const int dn = reach.distance(Cell{l, x + s[0], y + s[1]});
                    if (dn >= 0 && (d < 0 || dn + 1 < d)) d = dn + 1;
                }
                if(!world) {
                    // Native stair entries may have only a diagonal approach
                    // (South Figaro Upper: masks159 ->31, with all four
                    // cardinal neighbors closed). A* already accepts this real
                    // edge; the flood's terminal exit is not itself expanded.
                    // Validate the same source/destination direction bits with
                    // the exact exit as goal. Never invent a corner across zero
                    // collision or add diagonals to world movement.
                    static const int diagonal[4][2]={{1,-1},{1,1},{-1,1},{-1,-1}};
                    std::optional<baked_route::Request> terminal;
                    const Cell end{l,x,y};
                    for(const auto& side:diagonal) {
                        const Cell from{l,x+side[0],y+side[1]};const int dn=reach.distance(from);
                        if(dn<0||baked_route::layer_after(grid,l,x,y)!=l)continue;
                        if(!terminal){terminal=request;terminal->goals={end};}
                        if(baked_route::edge_allowed(*terminal,grid,from,end)&&(d<0||dn+1<d))d=dn+1;
                    }
                }
            }
            if (d >= 0 && (best < 0 || d < best)) best = d;
        }
    }
    return best;
}

baked_route::Request walk_request(const baked::Submap& sm, const Cell& start)
{
    baked_route::Request req;
    req.cancelled = &planning_cancelled;
    req.submap = &sm;
    if(g_route_snapshot)req.transitions=current_transitions(sm);
    if(g_route_snapshot&&g_route_snapshot->submap==&sm)req.transport_rules=g_route_snapshot->transport_rules;
    if(!sm.world)req.transportation_id=g_route_snapshot&&g_route_snapshot->vehicle.valid?g_route_snapshot->vehicle.id:1;
    req.start = start;
    req.passages = game::flags().has_hidden_passages;
    if (sm.world) req.auto_groups = planning_auto_groups();
    return req;
}

constexpr int kMaxVisitedMaps = 400;

} // namespace

std::wstring map_name_of(int map_id) {
    // Native map-name resolution is a separate owner continuation. The token is
    // replaced by worker formatting before any route is published or spoken.
    if (g_route_snapshot) return L"\uE000" + std::to_wstring(map_id) + L"\uE001";
    return resolve_map_name_by_id(map_id);
}

CrossLeg find_across(void* map_handle, void* player, const MapGoal& goal, bool with_gates, bool allow_traps, bool with_npcs, bool with_remote_npcs,
    const std::vector<const baked::Object*>* world_targets, std::vector<int>* distances)
{
    CrossLeg out;
    const baked::Submap* here = current_submap(map_handle);
    const baked::Submap* there = baked::submap_by_map_id(goal.map_id);
    if (!here || !there || (!player && !g_route_snapshot)) return out;
    const bool batch=world_targets&&distances;
    if(batch&&(!here->world||here!=there||world_targets->size()>512))return out;
    std::vector<int> target_traps;
    if(batch){distances->assign(world_targets->size(),-1);target_traps.assign(world_targets->size(),INT_MAX);}
    return hooks::guarded_r("BakedNav.FindAcross", out, [&] {
        Cell start;
        if (!player_cell(map_handle, player, start)) return out;
        const baked::Grids& gh = baked::grids(*here);
        if (!baked::in_bounds(gh, start.x, start.y)) return out;
        // The live obstacles of this map (only this map has live state): a story gate closes
        // the cells it stands on, and an exit it stands on is sealed (Kashuan Keep's door).
        const Blockers live = live_blockers(*here, gh, {});
        map_state::Resolver state = planning_state();
        auto sealed = [&](const baked::Object& o) {
            if (with_gates) for (const Cell& c : live.gates)
                if (o.contains(c.x, c.y)) return true;
            return false;
        };

        // The goal's cells on its own map.
        std::vector<Cell> goal_cells = goal.cells;
        if (goal_cells.empty()) {
            const baked::Grids& gt = baked::grids(*there);
            Cell gc = goal.cell;
            if (goal.examine) {
                static const int kSide[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
                for (const auto& s : kSide) {
                    Cell c{gc.layer, gc.x + s[0], gc.y + s[1]};
                    if (baked::in_bounds(gt, c.x, c.y)) goal_cells.push_back(c);
                }
            }
            if (goal_cells.empty()) goal_cells.push_back(gc);
            if (there->world)
                for (Cell& c : goal_cells) c.layer = 1;
        }

        // Dijkstra over (sub-map, arrival cell): the whole map is flooded from the arrival once.
        struct State { int submap; Cell at; int cost; int parent; const baked::Object* via; int via_dest; int traps; };
        std::vector<State> states;
        std::vector<int> order;   // indices sorted by cost on each pop (small graphs)
        states.push_back(State{baked::submap_index(here), start, 0, -1, nullptr, 0, 0});
        std::vector<bool> done;
        int goal_state = -1;
        int goal_steps = 0;
        int visited = 0;
        while (visited < kMaxVisitedMaps) {
            if (planning_cancelled()) return out;
            // pop the cheapest unfinished state
            int best = -1;
            for (size_t i = 0; i < states.size(); ++i) {
                if (i < done.size() && done[i]) continue;
                if (best < 0 || states[i].traps < states[static_cast<size_t>(best)].traps ||
                    (states[i].traps == states[static_cast<size_t>(best)].traps && states[i].cost < states[static_cast<size_t>(best)].cost)) best = static_cast<int>(i);
            }
            if (best < 0) break;
            if (done.size() < states.size()) done.resize(states.size(), false);
            done[static_cast<size_t>(best)] = true;
            ++visited;out.visited_maps=visited;
            const State cur = states[static_cast<size_t>(best)];
            const baked::Submap* sm = baked::submap_at(cur.submap);
            if (!sm) continue;
            const baked::Grids& g = baked::grids(*sm);
            baked_route::Request req = walk_request(*sm, cur.at);
            if (sm == here) {
                req.blocked = live.hard(with_gates,true,with_npcs);
                live.apply_transport(req,with_gates);
                if (with_gates) req.directional = live.directional;
                req.live_hazards = true;
                req.hazards = live.triggers;
                req.live_traps = true; req.traps = live.traps;
            } else map_state::add_obstacles(state, *sm, req, with_gates, false, with_remote_npcs);
            // Keep destinations behind a visible event available; warn on the actual leg.
            req.ignore_hazards = true;
            baked_route::Reach reach = baked_route::flood(req);
            if(best==0&&sm->world) {
                // A world destination can require entering a dungeon and
                // returning to another land pocket (Crystal Tower beyond the
                // Ancients' Maze). The first graph edge needs the same legal
                // owned aircraft landing/boarding coverage as a local exit.
                const auto vehicle=planning_vehicle();const auto* model=vehicle.valid?baked::transport(vehicle.id):nullptr;
                const bool leave_current=model&&(model->type==3||model->type==7||model->type==8||
                    (game::is(game::Id::FF3)&&(model->type==kTransportShip||model->type==9||model->type==10)));
                if(leave_current) {
                    auto ride=req;world_mode(ride);live.apply_transport(ride,with_gates);
                    reach=land_after_ride(ride,baked_route::flood(ride));
                    if(game::is(game::Id::FF3)&&g_route_snapshot&&g_route_snapshot->vehicle_shortcuts&&reach.submap) {
                        auto foot=req;foot.transportation_id=0;
                        const auto parked=parked_vehicle_reach(foot,reach);
                        if(parked.submap)for(size_t i=0;i<reach.dist[0].size()&&i<parked.dist[0].size();++i) {
                            if(planning_cancelled())return out;
                            reach.dist[0][i]=std::min(reach.dist[0][i],parked.dist[0][i]);
                        }
                    }
                } else if(!vehicle.valid&&g_route_snapshot&&g_route_snapshot->vehicle_shortcuts) {
                    const auto parked=parked_vehicle_reach(req,reach);
                    // Assistance adds legal landfalls; it must never discard
                    // ordinary walking when no parked craft is usable.
                    if(parked.submap&&reach.submap)for(size_t i=0;i<reach.dist[0].size()&&i<parked.dist[0].size();++i) {
                        if(planning_cancelled())return out;
                        reach.dist[0][i]=std::min(reach.dist[0][i],parked.dist[0][i]);
                    }
                }
            }

            if(batch&&sm==here)for(size_t index=0;index<world_targets->size();++index) {
                if(planning_cancelled())return out;
                const auto* object=(*world_targets)[index];if(!object||sealed(*object)||!state.active(*sm,*object))continue;
                const int d=distance_to_object(req,reach,*object,cur.at.layer,true);if(d<0)continue;
                if(cur.traps<target_traps[index]||(cur.traps==target_traps[index]&&
                    ((*distances)[index]<0||cur.cost+d<(*distances)[index]))) {
                    target_traps[index]=cur.traps;(*distances)[index]=cur.cost+d;
                }
            }
            if (!batch && sm == there) {
                int d = -1;
                for (const Cell& c : goal_cells) {
                    int dc = reach.distance(c);
                    if (dc < 0 && !goal.examine) {
                        // a closed goal cell (a chest, a switch): from beside it
                        static const int kSide[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
                        for (const auto& s : kSide) {
                            const int dn = reach.distance(Cell{c.layer, c.x + s[0], c.y + s[1]});
                            if (dn >= 0 && (dc < 0 || dn + 1 < dc)) dc = dn + 1;
                        }
                    }
                    if (dc >= 0 && (d < 0 || dc < d)) d = dc;
                }
                if (d >= 0) {
                    goal_state = best;
                    goal_steps = cur.cost + d;
                    break;
                }
            }
            for (const Edge& e : edges_of(g, sm == here ? planning_map_id() : sm->first_map_id)) {
                if (!batch && g_pending && g_pending->goal.map_id == goal.map_id &&
                    g_pending->goal.cell == goal.cell && g_pending->goal.entity_id == goal.entity_id &&g_pending->goal.raw_name==goal.raw_name) {
                    bool repeated = g_pending->completed_transitions.size() >= static_cast<size_t>(kMaxVisitedMaps);
                    for (const auto& used : g_pending->completed_transitions)
                        repeated |= used.map_id == sm->first_map_id && used.entity_id == e.object->entity_id() &&
                            used.x == e.object->tx && used.y == e.object->ty &&
                            baked::submap_by_map_id(used.destination) == baked::submap_by_map_id(e.dest_map_id);
                    if (repeated) { out.transition_repeat_blocked = true; continue; }
                }
                const bool trap = baked::is_trap(*e.object);
                if (trap && !allow_traps) continue;
                if (trap) {
                    bool repeated = false;
                    for (int parent = best; parent >= 0; parent = states[static_cast<size_t>(parent)].parent)
                        repeated |= states[static_cast<size_t>(parent)].via == e.object;
                    if (!batch && g_pending && g_pending->goal.map_id == goal.map_id &&
                        g_pending->goal.cell == goal.cell) {
                        repeated |= g_pending->completed_traps.size() >= static_cast<size_t>(kMaxVisitedMaps);
                        for (const auto& used : g_pending->completed_traps)
                            repeated |= used.map_id == sm->first_map_id && used.entity_id == e.object->entity_id() &&
                                used.x == e.object->tx && used.y == e.object->ty;
                    }
                    if (repeated) { out.trap_repeat_blocked = true; continue; }
                }
                if (!state.active(*sm, *e.object)||state.consumed(*sm,*e.object)) continue;
                const baked::Submap* dest = baked::submap_by_map_id(e.dest_map_id);
                if (!dest) continue;
                // Indoor floor navigation never walks an entire world as a
                // speculative detour. That was both inconsistent with remote
                // discovery and hundreds of redundant floods for a local gate.
                if(!here->world&&!there->world&&dest->world)continue;
                if (sm == here && sealed(*e.object)) continue;
                const int d = distance_to_object(req, reach, *e.object, cur.at.layer, sm->world);
                if (d < 0) continue;
                const baked::Object* arrival = point_in(*dest, e.dest_point, state);
                if (!arrival) continue;
                Cell at{arrival->layer, arrival->tx, arrival->ty};
                if (dest->world) at.layer = 1;
                const int cost = cur.cost + d + 1;
                const int traps = cur.traps + (trap ? 1 : 0);
                // keep the cheapest arrival per (map, cell)
                bool known = false;
                for (size_t i = 0; i < states.size(); ++i) {
                    if (states[i].submap == baked::submap_index(dest) && states[i].at == at) {
                        known = true;
                        if ((traps < states[i].traps || (traps == states[i].traps && cost < states[i].cost)) && !(i < done.size() && done[i])) {
                            states[i].cost = cost;
                            states[i].parent = best;
                            states[i].via = e.object;
                            states[i].via_dest = e.dest_map_id;
                            states[i].traps = traps;
                        }
                        break;
                    }
                }
                if (!known) states.push_back(State{baked::submap_index(dest), at, cost, best, e.object, e.dest_map_id, traps});
            }
        }
        if(batch) {
            out.incomplete=visited>=kMaxVisitedMaps&&(done.size()<states.size()||std::any_of(done.begin(),done.end(),[](bool value){return !value;}));
            // Match selected routing's one conservative later-floor NPC retry,
            // once for the whole batch rather than one full graph per UI row.
            if(!out.incomplete&&with_remote_npcs&&std::find(distances->begin(),distances->end(),-1)!=distances->end()) {
                std::vector<int> retry;
                const auto second=find_across(map_handle,player,goal,with_gates,allow_traps,with_npcs,false,world_targets,&retry);
                for(size_t i=0;i<retry.size();++i)if((*distances)[i]<0)(*distances)[i]=retry[i];
                out.incomplete=second.incomplete;
            }
            out.found=std::any_of(distances->begin(),distances->end(),[](int value){return value>=0;});return out;
        }
        if (goal_state < 0) {
            out.incomplete = visited >= kMaxVisitedMaps &&
                (done.size() < states.size() || std::any_of(done.begin(), done.end(), [](bool finished) { return !finished; }));
            // A later-floor NPC is a possible interaction stop, not proof that
            // its whole floor is unreachable. Issue only this floor's legal
            // leg; entry recaptures native actors and the existing NPC approach
            // stops at the real blocker. Keep current NPCs, gates, terrain,
            // trap policy and transition history. One bounded retry only.
            if(!out.incomplete&&with_remote_npcs)
                return find_across(map_handle,player,goal,with_gates,allow_traps,with_npcs,false);
            return out;
        }
        out.requires_trap = states[static_cast<size_t>(goal_state)].traps > 0;

        // Walk back to the first hop taken from the start state.
        int first = goal_state;
        while (first >= 0 && states[static_cast<size_t>(first)].parent > 0) first = states[static_cast<size_t>(first)].parent;
        std::vector<Phase> phases;
        baked_route::Request req = walk_request(*here, start);
        if (goal_state == 0) {
            // On this map: route to the goal itself.
            req.goals = goal_cells;
            if (!goal.examine && goal.cells.empty()) {
                std::vector<Cell> around = goals_for(*here, goal.cell, TargetKind::Infer).cells;
                req.goals.insert(req.goals.end(), around.begin(), around.end());
            }
            req.blocked = live.hard(with_gates,true,with_npcs);
            live.apply_transport(req,with_gates);
            if (with_gates) req.directional = live.directional;
            req.live_hazards = true;
            req.hazards = live.triggers;
            req.live_traps = true; req.traps = live.traps;
            req.allow_trap_goal = std::any_of(gh.objects.begin(), gh.objects.end(), [&](const auto& object) {
                return baked::is_trap(object) && object.contains(goal.cell.x, goal.cell.y);
            });
            if(here->world) {
                auto movement=req;world_mode(movement);live.apply_transport(movement,with_gates);
                if(vehicle_journey_route(*here,gh,movement,req.goals,phases)) {
                    out.on_this_map=true;out.phases=phases;out.description=describe(phases);
                    for(const auto& phase:phases)out.cells.insert(out.cells.end(),phase.cells.begin(),phase.cells.end());
                    out.steps_here=phase_steps(phases);out.steps_total=goal_steps;out.found=true;return out;
                }
            }
            baked_route::Route route = baked_route::find(req);
            if (!route.found && !req.hazards.empty()) { req.ignore_hazards = true; route = baked_route::find(req); }
            if (!route.found) return out;
            Phase p;
            p.cells = route.cells;
            annotate_facing(p, goals_for(*here, goal.cell, TargetKind::Infer));
            phases.push_back(p);
            out.on_this_map = true;
            out.phases = phases;
            out.description = describe(phases);
            out.event_on_route = event_on_route(route.cells, req.hazards, req.allow_trap_goal ? req.goals : std::vector<Cell>{},&req);
            out.cells = route.cells;
            out.steps_here = route.steps();
            out.steps_total = goal_steps;
            out.found = true;
            return out;
        }
        const State& hop = states[static_cast<size_t>(first)];
        if (!hop.via) return out;
        const baked::Object& o = *hop.via;
        out.trap_leg = baked::is_trap(o);
        out.trap_arrival = hop.at;
        out.trap_entity = o.entity_id(); out.trap_x = o.tx; out.trap_y = o.ty;
        const bool examine_warp = o.type == baked::ObjType::Warp && (o.d&255) != 1;
        const auto warp_goals = baked_route::goals_for(req,Cell{o.layer,o.tx,o.ty},baked_route::GoalKind::Interact,&o);
        if (examine_warp) req.goals = warp_goals.cells;
        else for (int y = o.ty; y < o.ty + o.th; ++y)
            for (int x = o.tx; x < o.tx + o.tw; ++x)
                req.goals.push_back(Cell{here->world ? start.layer : o.layer, x, y});
        req.blocked = live.hard(with_gates,true,with_npcs);
        live.apply_transport(req,with_gates);
        if (with_gates) req.directional = live.directional;
        req.live_hazards = true;
        req.hazards = live.triggers;
        req.live_traps = true; req.traps = live.traps;
        req.allow_trap_goal = out.trap_leg||baked::world_exit(o);
        baked_route::Route route = baked_route::find(req);
        if (!route.found && !req.hazards.empty()) { req.ignore_hazards = true; route = baked_route::find(req); }
        if(here->world) {
            auto movement=req;world_mode(movement);live.apply_transport(movement,with_gates);
            const auto vehicle=planning_vehicle();
            if(vehicle.valid||!route.found) {
                if(!vehicle_journey_route(*here,gh,movement,req.goals,phases))return out;
            }
        }
        if(phases.empty()) {
            if (!route.found) return out;
            Phase p;p.cells=route.cells;phases.push_back(std::move(p));
        }
        auto& p=phases.back();
        std::wstring dest_name = map_name_of(hop.via_dest);
        if (o.type == baked::ObjType::Warp) {
            p.action_after = out.trap_leg ? T(L"step on the trap") : (o.d&255) == 1 ? T(L"step on the warp") : (o.d&255) == 2 ? T(L"interact to cross") : T(L"examine the warp");
            p.manual_after = (o.d&255) != 1;
            if (examine_warp) annotate_facing(p, warp_goals);
        } else {
            p.action_after = dest_name.empty() ? T(L"take the exit") : TF(L"take the exit to {0}", {dest_name});
        }
        out.found = true;
        out.on_this_map = false;
        out.next_map_id = hop.via_dest;
        for(const auto& phase:phases)out.cells.insert(out.cells.end(),phase.cells.begin(),phase.cells.end());
        out.phases = phases;
        out.description = describe(phases);
        for(const auto& phase:phases){const auto movement=phase_movement(req,phase);
            out.event_on_route|=event_on_route(phase.cells,req.hazards,req.goals,&movement);}
        out.steps_here = phase_steps(phases);
        out.steps_total = goal_steps;
        return out;
    });
}

// --- the active route (user, 2026-10-11) -----------------------------------------------------
// A route is planned once and followed to its end. Asking again for the same target while
// standing on the planned route speaks the rest of that route from the player's cell; the
// search runs again only for another target, another map, or a player who has left the route.
// Before this every query re-planned from the current cell, and a villager standing in a
// street at planning time (a live blocker) flipped the plan between the two ways round a
// block mid-walk: "West 4, South 10, East 4 ..." and four steps later "East 4, North 1, East 2".
namespace {


thread_local const ActiveRoute* g_active = nullptr;
void invalidate_active() {
    if (g_route_snapshot) g_route_snapshot->active.valid = false;
    else { g_active = nullptr; g_published_route.reset(); }
}

bool same_target(const ActiveRoute& r, int map_id, const Cell& target, int goal_map_id)
{
    return r.valid && r.map_id == map_id && r.goal_map_id == goal_map_id && r.target.x == target.x && r.target.y == target.y;
}

void set_active(int map_id, const Cell& target, int goal_map_id, const std::vector<Phase>& phases, int next_map_id)
{
    if (!g_route_snapshot) return; // only owned worker planning may construct routes
    auto& active = g_route_snapshot->active;
    active.valid = true; active.map_id = map_id; active.target = target;
    active.goal_map_id = goal_map_id; active.next_map_id = next_map_id; active.phases = phases;
    active.transportation_id = g_route_snapshot->vehicle.valid ? g_route_snapshot->vehicle.id : 0;
    active.requires_trap = false;
    active.blocking_party = -1;
    active.npc_approach = false;
    active.target_vehicle=g_route_snapshot->goal.transportation_id;
    active.vehicle_shortcuts=g_route_snapshot->vehicle_shortcuts;
    active.observed_target=g_route_snapshot->observed_goal;
}

/// The rest of the active route from the player's cell, when the cell lies on it (the last
/// occurrence, nearest the goal). A phase's action word is kept only at the phase's first cell.
bool remaining_from(const ActiveRoute& r, const Cell& at, std::vector<Phase>& out)
{
    if(!route_progress::remaining(r.phases, at, out, &planning_cancelled)||out.empty())return false;
    const auto& first=out.front();
    if(first.movement_transportation_id<0)return true; // unchanged single-mode leg
    const auto current=planning_vehicle();
    const int movement=current.valid&&current.type!=1&&current.type!=kTransportContent?current.id:0;
    if(first.movement_transportation_id==movement)return true;
    // The same tile can lie on the walk to an aircraft AND on its later flight
    // (Duster -> Ancient Ruins). Last-coordinate matching must not silently
    // skip boarding. Retire that incompatible suffix and let the normal planner
    // rebuild the complete legal route from the captured current vehicle mode.
    // A still-pending native action at this exact position is a valid boundary:
    // preserve boarding, dismount/landing, including FFIII/VI manual deck entry.
    if(first.action_before.empty()||!first.action_cell||!(*first.action_cell==at))return false;
    using Action=route_progress::VehicleAction;
    switch(first.vehicle_before) {
    case Action::WalkOff:case Action::ConfirmLand:case Action::BackLand:
        return current.valid&&first.transportation_id==current.id&&first.movement_transportation_id==0;
    case Action::WalkOn:case Action::ConfirmBoard:case Action::None:
        return movement==0&&first.transportation_id>0&&first.movement_transportation_id==first.transportation_id;
    }
    return false;
}

// Same-cell Enterprise conversions make a cell occur in consecutive phases.
// The legacy last-occurrence shortcut can skip the pending Confirm action, so
// recompute these uncommon plans from the newly captured vehicle identity.
bool enterprise_conversion_plan(const ActiveRoute& route) {
    if(!game::is(game::Id::FF3))return false;
    return std::any_of(route.phases.begin(),route.phases.end(),[](const Phase& phase) {
        return phase.vehicle_before==route_progress::VehicleAction::ConfirmBoard &&
            baked_route::enterprise_pair(phase.transportation_id) && phase.action_cell &&
            !phase.cells.empty() && *phase.action_cell==phase.cells.front();
    });
}

void fill_result(const baked::Submap& sm, const std::vector<Phase>& phases, PathInfo& out)
{
    out.success = true;
    out.error.clear();
    out.world_path.clear();
    out.step_count = 0;
    for (const Phase& p : phases) {
        std::vector<Vec3> w = world_path_of(sm, p.cells);
        out.world_path.insert(out.world_path.end(), w.begin(), w.end());
        out.step_count += route_progress::steps(p);
    }
    out.description = describe(phases);
    out.full_description = describe(phases, true);
    if((out.blocking_party>=0||out.npc_approach)&&!phases.empty()) {
        // Keep the terminal advisory after a brief route's "and n more" cap.
        // This is structural formatting, not suppression of equal speech text.
        auto spoken=phases;spoken.back().action_after.clear();
        const auto& boundary=phases.back().action_after;
        out.description=out.step_count?describe(spoken)+L", "+boundary:describe(phases);
        out.full_description=out.step_count?describe(spoken,true)+L", "+boundary:describe(phases,true);
    }
    out.walk_phases = phases;
    out.map_width = sm.width;
    out.map_height = sm.height;
    out.wraps = sm.world;
}

} // namespace

bool find(Vec3 from, Vec3 to, void* map_handle, void* player, bool full, TargetKind kind, bool with_vehicles, PathInfo& out)
{
    const baked::Submap* sm = current_submap(map_handle);
    if (!sm) return false;
    if (!g_route_snapshot) {
        if (!full) return quick_query(from, to, map_handle, player, kind, with_vehicles, out);
        out.reason = out.error = T(L"Navigation requires an asynchronous request");
        return true;
    }
    return hooks::guarded_r("BakedNav.Find", false, [&] {
        const baked::Grids& g = baked::grids(*sm);
        if (g.width <= 0 || g.height <= 0) return false;

        baked_route::Request req;
        req.cancelled = &planning_cancelled;
        req.submap = sm;
        req.transitions=current_transitions(*sm);
        req.start = cell_of(*sm, from, sm->world ? 1 : (g_route_snapshot ? snapshot_layer() : (!player ? 1 : player_layer(player))));
        const Cell target = cell_of(*sm, to, 1);
        const baked_route::Goals goals = goals_for(*sm, target, kind);
        req.goals = goals.cells;
        world_mode(req);
        const bool surface_marker=nautilus_surface::matches(static_cast<int>(game::current()),planning_map_id(),target,
            g_route_snapshot->goal.entity_id,g_route_snapshot->goal.raw_name);
        if(surface_marker && (kind!=TargetKind::Spot || !planning_vehicle().valid ||
            planning_vehicle().id!=14 || planning_vehicle().type!=kTransportSubmarine)) {
            out.error=L"No path found";return true;
        }
        const auto* action_pad=kind==TargetKind::Infer?vehicle_pad_at(*sm,target):nullptr;
        const air_transitions::Rule* aerial=nullptr;
        if(game::is(game::Id::FF3)&&planning_map_id()==4&&(kind==TargetKind::Infer||kind==TargetKind::Transition)) {
            for(const auto& object:g.objects)
                if(object.contains(target.x,target.y)&&planning_object_active(*sm,object))
                    if(const auto* entry=air_transitions::saronia_entry(static_cast<int>(game::current()),planning_map_id(),object)) {
                        aerial=entry;req.goals_open=false;req.terminal_goal=true;break;
                    }
        }
        if(kind==TargetKind::AirshipTransition) {
            req.terminal_goal=true;
            for(const auto& object:g.objects) {
                const auto* rule=air_transitions::match(static_cast<int>(game::current()),planning_map_id(),object);
                if(!rule)rule=air_transitions::saronia_entry(static_cast<int>(game::current()),planning_map_id(),object);
                if(rule&&object.contains(target.x,target.y)&&planning_object_active(*sm,object)&&
                   (g_route_snapshot->goal.entity_id<0||g_route_snapshot->goal.entity_id==object.entity_id())&&
                   (g_route_snapshot->goal.raw_name.empty()||g_route_snapshot->goal.raw_name==baked::object_identifier(object))) { aerial=rule;break; }
            }
            if(!aerial){out.error=L"No path found";return true;}
        }
        if(action_pad)req.goals_open=false;
        if (req.vehicle_group > 0) req.goals_open = baked_route::native_ground_entry(req.vehicle_type);
        if(req.vehicle_type==kTransportSubmarine&&std::any_of(g.objects.begin(),g.objects.end(),[&](const auto& object){
            return baked::submarine_entry(object)&&object.contains(target.x,target.y);
        }))req.goals_open=true;
        if (!baked::in_bounds(g, req.start.x, req.start.y) || req.goals.empty()) {
            out.error = L"No path found";
            return true;   // handled: the game's searcher would not do better off the grid
        }

        if (!full) {
            // Events are actionable obstacles, not reasons to hide every destination beyond
            // them. Explicit routing evaluates active triggers and gives the warning.
            req.ignore_hazards = true;
            // "Current map with vehicles": the vehicles the party has on this map carry it too
            // (their groups join walking, as the auto-ridden canoe does).
            if (with_vehicles && sm->world && req.vehicle_group == 0) {
                for (const VehicleOnMap& v : planning_vehicles())
                    if (const baked::TransportRow* row = baked::transport(v.transportation_id))
                        if (row->type != kTransportContent && row->type != 1 && row->group > 1) req.auto_groups.push_back(row->group);
            }
            // Quick: one flood per player cell; passages as a second flood only when needed.
            bool ok = any_reachable(cached_flood(req), req.goals);
            if (!ok && game::flags().has_hidden_passages && !sm->world) {
                baked_route::Request with = req;
                with.passages = true;
                ok = any_reachable(baked_route::flood(with), req.goals);
            }
            out.success = ok;
            if (!ok) out.error = L"No path found";
            return true;
        }

        const Blockers live = live_blockers(*sm, g, req.goals);
        req.blocked = live.hard();
        live.apply_transport(req);
        req.directional = live.directional;
        req.live_hazards = true;
        req.hazards = live.triggers;
        req.live_traps = true; req.traps = live.traps;
        const bool selected_event = (aerial || kind == TargetKind::Infer || kind == TargetKind::AirshipTransition) && std::any_of(g.objects.begin(), g.objects.end(), [&](const auto& object) {
            return baked_route::step_event(object) && object.contains(target.x, target.y);
        });
        const bool selected_exit=std::any_of(g.objects.begin(),g.objects.end(),[&](const auto& object){return baked::world_exit(object)&&object.contains(target.x,target.y);});
        req.allow_trap_goal = selected_event||selected_exit;
        auto annotate_events = [&](const std::vector<Phase>& phases) {
            for (const Phase& p : phases) {
                const auto movement=phase_movement(req,p);
                if (event_on_route(p.cells, live.triggers, selected_event ? req.goals : std::vector<Cell>{},&movement)) {
                    out.event_on_route = true;
                    return;
                }
            }
        };
        // The route already planned to this target, while the player is still on it.
        const int map_id = planning_map_id();
        if (g_active && !enterprise_conversion_plan(*g_active) && g_active->blocking_party<0 && !g_active->npc_approach && g_active->vehicle_shortcuts==g_route_snapshot->vehicle_shortcuts && g_active->target_vehicle==g_route_snapshot->goal.transportation_id && g_active->transportation_id == (g_route_snapshot->vehicle.valid ? g_route_snapshot->vehicle.id : 0) &&
            same_target(*g_active, map_id, target, map_id)) {
            std::vector<Phase> rest;
            if (remaining_from(*g_active, req.start, rest)) {
                bool blocked = false;
                for(const auto& phase:rest)if(phase.transportation_id>0 &&
                    (!g_route_snapshot->vehicle.valid||g_route_snapshot->vehicle.id!=phase.transportation_id) &&
                    std::none_of(g_route_snapshot->vehicles.begin(),g_route_snapshot->vehicles.end(),[&](const auto& model){return model.transportation_id==phase.transportation_id;}))blocked=true;
                for (const Phase& phase : rest) {
                  const auto movement=phase_movement(req,phase);
                  for (size_t i = 1; i < phase.cells.size(); ++i) {
                    if (planning_cancelled()) return true;
                    if (!baked_route::direction_allowed(movement, phase.cells[i-1], phase.cells[i])) blocked = true;
                    if(baked_route::transport_rule_at(movement,g,phase.cells[i],baked_route::TransportRules::Gate)||
                        baked_route::transport_rule_at(movement,g,phase.cells[i],baked_route::TransportRules::Solid))blocked=true;
                    const bool planned_exit=g_active->next_map_id>0&&i+1==phase.cells.size()&&
                        std::any_of(g.objects.begin(),g.objects.end(),[&](const auto& object){
                            return baked::transition_bits(object)&&object.contains(phase.cells[i].x,phase.cells[i].y)&&
                                (object.a==g_active->next_map_id||(object.a==-1&&g_active->next_map_id==map_id));
                        });
                    if(baked_route::terminal_transition(movement,g,phase.cells[i])&&!planned_exit&&
                       std::none_of(req.goals.begin(),req.goals.end(),[&](const auto& goal){return goal.x==phase.cells[i].x&&goal.y==phase.cells[i].y&&(sm->world||goal.layer==phase.cells[i].layer);}))blocked=true;
                    const bool planned_drop = g_active->requires_trap && g_active->next_map_id > 0 && i+1 == phase.cells.size() &&
                        std::any_of(g.objects.begin(),g.objects.end(),[&](const auto& object) {
                            return baked::is_trap(object) && object.type==baked::ObjType::Warp &&
                                (object.a==g_active->next_map_id || (object.a==-1 && g_active->next_map_id==map_id)) && object.contains(phase.cells[i].x,phase.cells[i].y);
                        });
                    if (baked_route::trap_cell(movement, g, phase.cells[i]) && !planned_drop &&
                        !(req.allow_trap_goal && std::find(req.goals.begin(), req.goals.end(), phase.cells[i]) != req.goals.end())) blocked = true;
                    for (const Cell& obstacle : req.blocked) {
                        const Cell& c = phase.cells[i];
                        if (c.x == obstacle.x && c.y == obstacle.y && (obstacle.layer < 0 || c.layer == obstacle.layer)) blocked = true;
                    }
                  }
                }
                const bool walking_only=std::none_of(rest.begin(),rest.end(),[](const auto& phase){return phase.transportation_id>0;});
                if(walking_only&&may_shorten_with_vehicle(req,phase_steps(rest)))blocked=true;
                if(!walking_only&&req.vehicle_group==0&&g_route_snapshot->goal.transportation_id<=0) {
                    // A previously useful flight must also retire once walking
                    // is short. Geometry only gates this check; actual terrain,
                    // gates, NPCs and terminal cells decide the walking proof.
                    const bool near_target=std::any_of(req.goals.begin(),req.goals.end(),[&](const auto& goal){
                        const int dx=std::abs(req.start.x-goal.x),dy=std::abs(req.start.y-goal.y);
                        return std::min(dx,sm->width-dx)+std::min(dy,sm->height-dy)<15;
                    });
                    if(near_target){const auto walk=baked_route::find(req);if(walk.found)blocked=true;}
                }
                if (blocked) invalidate_active();
                else {
                fill_result(*sm, rest, out);
                out.required_trap = g_active->requires_trap;
                annotate_events(rest);
                out.next_map_id = g_active->next_map_id;
                LOGF("[BakedNav] route kept: at (%d,%d) L%d, %d steps left to (%d,%d)", req.start.x, req.start.y,
                     req.start.layer, out.step_count, target.x, target.y);
                return true;
                }
            }
        }

        // Every solid live NPC is a wall at its current position. Native Stamp movement
        // changes facing without leaving the tile (Dreadnought captains); even a travelling
        // NPC may not be crossed just because the real detour is longer than six cells.
        baked_route::Route route;
        const bool riding_goal = std::any_of(req.goals.begin(),req.goals.end(),[&](const Cell& goal) {
            return baked_route::open(req,g,goal) || (req.goals_open&&baked_route::transition_enterable(req,g,goal.x,goal.y));
        });
        // A land goal cannot terminate ship-only A*. Go straight to the owned
        // ride/landing/walk planner instead of exhausting the whole sea first.
        const auto current_transport=planning_vehicle();
        const auto ship_goal=(kind==TargetKind::Infer||kind==TargetKind::Transition)?ship_goal_at(*sm,target):std::nullopt;
        const bool ship_ready=!ship_goal||(current_transport.valid&&(ship_goal->ids&baked_route::transportation_bit(current_transport.id)));
        const bool aerial_ready=!aerial||(current_transport.valid&&air_transitions::accepts(*aerial,current_transport.id));
        const bool pad_ready=ship_ready && aerial_ready && (!action_pad || (req.vehicle_group>0 && current_transport.valid && baked::vehicle_pad_accepts(*action_pad,current_transport.id))) &&
            (action_pad || aerial || !mounted_foot_target(req,target,kind,goals.face)) &&
            !(sm->world&&req.vehicle_group>0&&goals.face);
        if (pad_ready && (req.vehicle_group == 0 || riding_goal)) route = baked_route::find(req);
        if (!route.found && game::flags().has_hidden_passages && !sm->world) {
            baked_route::Request with = req;
            with.passages = true;
            route = baked_route::find(with);
        }
        if (pad_ready && !route.found && !live.triggers.empty() && (req.vehicle_group == 0 || riding_goal)) {
            auto through_events = req;
            through_events.ignore_hazards = true;
            if (game::flags().has_hidden_passages && !sm->world) through_events.passages = true;
            route = baked_route::find(through_events);
        }
        std::vector<Phase> phases;
        if (route.found) {
            Phase p;
            p.cells = route.cells;
            if(aerial||req.vehicle_group>0)p.transportation_id=current_transport.id;
            vehicle_pad_terminal(p,action_pad);
            phases.push_back(p);
            prepare_mountain_phases(*sm,phases);
            // Finding a walking path does not settle the route when vehicle
            // assistance is enabled. Objects, NPCs, chests and exits share the
            // same legal boarding/riding/landing/final-approach planner.
            if(may_shorten_with_vehicle(req,route.steps())) {
                std::vector<Phase> assisted;
                if(vehicle_journey_route(*sm,g,req,req.goals,assisted,aerial)&&phase_steps(assisted)<route.steps()&&
                   std::none_of(assisted.begin(),assisted.end(),[](const auto& phase){return phase.manual_displacement_steps>0;}))
                    phases=std::move(assisted);
            }
        } else if (sm->world && (!aerial || current_transport.valid || g_route_snapshot->vehicle_shortcuts)) {
            if (!vehicle_journey_route(*sm, g, req, req.goals, phases,aerial) && !live.triggers.empty()) {
                // The existing explicit-route policy may cross a live event with
                // a warning. Apply it to both vehicle/walking legs, without
                // repeating a ship-only search for an unreachable land goal.
                auto through_events = req;
                through_events.ignore_hazards = true;
                vehicle_journey_route(*sm, g, through_events, req.goals, phases,aerial);
            }
        }
        std::wstring reason;
        int next_map = 0;
        bool alternative_incomplete=false;
        const bool world_dungeon_return=sm->world&&kind==TargetKind::Transition&&std::any_of(g.objects.begin(),g.objects.end(),
            [&](const auto& object){return object.type==baked::ObjType::GotoMap&&object.contains(target.x,target.y);});
        if (phases.empty() && (!sm->world||world_dungeon_return)) {
            // Cut off on this floor: through another part of the map and back (user,
            // 2026-10-11: Kashuan Keep's 2F stairs are reached through the other 1F part and
            // back in at its upper door). The leg ends at the exit to take; the player asks
            // again on the other side. When only a story gate (the sealed door) stands in the
            // way of that, say so.
            MapGoal mg;
            mg.map_id = map_id;
            mg.cell = target;
            mg.cells = req.goals;
            CrossLeg leg = find_across(map_handle, player, mg, true);
            if (!leg.found && !leg.incomplete) leg = find_across(map_handle, player, mg, true, true);
            out.trap_repeat_blocked = leg.trap_repeat_blocked;
            out.transition_repeat_blocked = leg.transition_repeat_blocked;
            alternative_incomplete=leg.incomplete;
            if (leg.found && !leg.on_this_map) {
                phases = leg.phases;
                next_map = leg.next_map_id;
                out.required_trap = leg.requires_trap; out.event_on_route = leg.event_on_route;
            } else if (!leg.incomplete && find_across(map_handle, player, mg, false).found) {
                reason = T(L"Story gated");
            }
            if (!leg.found && leg.trap_repeat_blocked) reason = T(L"Trap route would repeat. Choose another route.");
            if (!leg.found && leg.transition_repeat_blocked) reason = T(L"Route would repeat. Choose another route.");
            if (!leg.found && leg.incomplete) reason = T(L"No safe route found");
        }
        if(phases.empty()&&!sm->world)if(auto approach=party_approach(req,live,out.blocking_party)) {
            out.reason=approach->action_after;phases.push_back(std::move(*approach));
        }
        if(phases.empty()&&!alternative_incomplete&&!sm->world&&!live.actors.empty()) {
            auto without=req;without.blocked=live.hard(true,true,false);without.ignore_hazards=true;
            if(game::flags().has_hidden_passages)without.passages=true;
            auto proof=baked_route::find(without);
            if(!proof.found) {
                MapGoal proof_goal;proof_goal.map_id=map_id;proof_goal.cell=target;proof_goal.cells=req.goals;
                auto leg=find_across(map_handle,player,proof_goal,true,false,false);
                if(!leg.found&&!leg.incomplete)leg=find_across(map_handle,player,proof_goal,true,true,false);
                if(leg.found)proof.cells=std::move(leg.cells);
            }
            if(auto approach=npc_approach(req,live,proof.cells)) {
                out.npc_approach=true;out.reason=approach->action_after;phases.push_back(std::move(*approach));
            }
        }
        if (phases.empty()) {
            out.error = L"No path found";
            out.reason = reason.empty() ? no_route_reason(*sm, g, req, live) : reason;
            if (g_active && same_target(*g_active, map_id, target, map_id)) invalidate_active();
            LOGF("[BakedNav] no route: (%d,%d) L%d -> (%d,%d) on map %d, %zu goal cells; %zu chests, %zu gates, %zu fixed and %zu moving NPCs; %ls",
                 req.start.x, req.start.y, req.start.layer, target.x, target.y, map_id, req.goals.size(),
                 live.chests.size(), live.gates.size(), live.fixed_npcs.size(), live.moving_npcs.size(),
                 out.reason.empty() ? L"no reason known" : out.reason.c_str());
            return true;
        }
        if(surface_marker) {
            // Native confirmation owns surfacing. End the movement/beacon at
            // the real basin tile and leave this action manual, including at0 steps.
            phases.back().action_after=T(L"surface here");phases.back().manual_after=true;
        }
        if (phases.back().action_after.empty()) annotate_facing(phases.back(), goals);
        set_active(map_id, target, map_id, phases, next_map);
        if(g_route_snapshot) {g_route_snapshot->active.blocking_party=out.blocking_party;g_route_snapshot->active.npc_approach=out.npc_approach;}
        if (g_route_snapshot) g_route_snapshot->active.requires_trap = out.required_trap;
        fill_result(*sm, phases, out);
        annotate_events(phases);
        out.next_map_id = next_map;
        LOGF("[BakedNav] route planned: (%d,%d) L%d -> (%d,%d) on map %d: %d steps, %zu phase(s); %zu chests, %zu gates, %zu fixed and %zu moving NPCs; %zu parked parties",
             req.start.x, req.start.y, req.start.layer, target.x, target.y, map_id, out.step_count, phases.size(),
             live.chests.size(), live.gates.size(), live.fixed_npcs.size(), live.moving_npcs.size(),live.parties.size());
        return true;
    });
}

namespace {
double g_follow_failed_at = 0;
constexpr double kFollowRetrySeconds = 2.0;

/// The beacon's next corner on `rest` from `at`: the end of the first leg; a phase of one cell
/// (standing beside the vehicle to board) steers at the next phase's first cell.
void corner_of(const baked::Submap& sm, const std::vector<Phase>& rest, const Cell& at, Corner& out,
               const std::function<bool()>& cancelled = {})
{
    int steps_left = 0;
    for (const Phase& p : rest) steps_left += route_progress::steps(p);
    Cell corner = at;
    int to_corner = 0;
    Vec3 projected_corner{};
    bool project = false;
    for (size_t pi = 0; pi < rest.size(); ++pi) {
        if (cancelled && cancelled()) return;
        const Phase& p = rest[pi];
        if (p.cells.size() >= 2) {
            const std::vector<baked_route::Leg> legs = baked_route::legs(p.cells,sm.world ? sm.width : 0,sm.world ? sm.height : 0,cancelled);
            if (cancelled && cancelled()) return;
            to_corner = legs.empty() ? static_cast<int>(p.cells.size()) - 1 : legs.front().count;
            corner = p.cells[static_cast<size_t>(to_corner)];
            if (sm.world && !legs.empty()) {
                // The beacon follows this leg's direction across an edge, not
                // the long Cartesian vector to the opposite side of the map.
                projected_corner = world_of_cell(sm,p.cells.front());
                const float length = to_corner * kTile;
                if (legs.front().dir == 0) projected_corner.y += length;
                else if (legs.front().dir == 1) projected_corner.x += length;
                else if (legs.front().dir == 2) projected_corner.y -= length;
                else projected_corner.x -= length;
                project = true;
            }
            break;
        }
        if(p.manual_after) {corner=p.cells.back();to_corner=0;break;}
        if (pi + 1 < rest.size() && !rest[pi + 1].cells.empty()) {
            corner = rest[pi + 1].cells.front();
            to_corner = 1;
            break;
        }
    }
    out.found = true;
    out.arrived = steps_left == 0;
    out.point = project ? projected_corner : world_of_cell(sm, corner);
    out.steps_to_corner = to_corner;
    out.steps_left = steps_left;
}
enum class CornerState { Pending, Ready, Missing };
struct CornerSnapshot {
    std::shared_ptr<const ActiveRoute> route;
    const baked::Submap* submap = nullptr;
    Cell at, target;
    int map_id = 0, goal_map = 0;
    std::function<bool()> cancelled;
};
struct CornerResult { CornerSnapshot input; BeaconCornerIndex index; };
worker_job::Owner g_follow_owner;
std::optional<CornerResult> g_corner;
std::optional<CornerSnapshot> g_corner_pending;
bool same_corner(const CornerSnapshot& a, const CornerSnapshot& b) {
    return a.route == b.route && a.submap == b.submap && a.target == b.target &&
        a.map_id == b.map_id && a.goal_map == b.goal_map;
}
void cancel_follow_work() { g_follow_owner.cancel(); g_corner.reset(); g_corner_pending.reset(); }
CornerResult build_corner(CornerSnapshot& input) {
    CornerResult result; result.input = input;
    if(input.route&&input.submap&&same_target(*input.route,input.map_id,input.target,input.goal_map))
        result.index=build_beacon_corner_index(*input.submap,input.route->phases,
            input.route->blocking_party>=0||input.route->npc_approach,&world_of_cell,input.cancelled);
    return result;
}
CornerState cached_corner(const baked::Submap* sm, int map_id, const Cell& at,
                         const Cell& target, int goal_map, Corner& out) {
    if (!g_published_route || !same_target(*g_published_route, map_id, target, goal_map)) return CornerState::Missing;
    CornerSnapshot input{g_published_route, sm, at, target, map_id, goal_map};
    if (g_corner && same_corner(g_corner->input, input)) {
        const auto* corner=g_corner->index.find(at);
        if(!corner)return CornerState::Missing;
        out=*corner;
        return out.found?CornerState::Ready:CornerState::Missing;
    }
    if (g_corner_pending && same_corner(*g_corner_pending, input)) return CornerState::Pending;
    g_follow_owner.cancel();
    const auto epoch = g_follow_owner.epoch(); const auto generation = epoch->load();
    input.cancelled = [epoch,generation] { return epoch->load() != generation; };
    g_corner_pending = input;
    worker_job::start(g_follow_owner, input, &build_corner, [](CornerResult result) {
        if (!g_corner_pending || !same_corner(*g_corner_pending, result.input)) return;
        g_corner = std::move(result); g_corner_pending.reset();
    }, background::Lane::Navigation);
    return CornerState::Pending;
}
} // namespace

bool follow(void* map_handle, void* player, Vec3 target, TargetKind kind, Corner& out, int entity_id, int transportation_id)
{
    out = Corner{};
    if (!ffpr::system::is_field_navigation_ready()) return false;
    const baked::Submap* sm = current_submap(map_handle);
    if (!sm || !player) return false;
    return hooks::guarded_r("BakedNav.Follow", false, [&] {
        void* tr = unity::transform(player);
        if (!tr) return false;
        const Vec3 from = unity::local_position(tr);
        const Cell at = cell_of(*sm, from, sm->world ? 1 : player_layer(player));
        Cell target_cell = cell_of(*sm, target, 1);
        if(transportation_id>0&&g_published_route&&g_published_route->target_vehicle==transportation_id&&
           g_published_route->observed_target==target_cell)target_cell=g_published_route->target;
        const int map_id = planning_map_id();
        const auto state = cached_corner(sm, map_id, at, target_cell, map_id, out);
        if (state == CornerState::Pending) {out.pending=true;return false;}
        if (state == CornerState::Missing) {
            // No route yet, or the player has left it: plan one silently. A target without a
            // route is retried every two seconds, not on every ping.
            const double now = frame::now();
            if (g_follow_failed_at > 0 && now - g_follow_failed_at < kFollowRetrySeconds) return false;
            if (!request_pending()) {
                MapGoal goal; goal.map_id = map_id; goal.cell = target_cell;
                goal.entity_id = entity_id;goal.transportation_id=transportation_id;
                request_route(goal, kind, L"", [](PathInfo info) {
                    g_follow_failed_at = info.success && (info.walk_phases.empty()||!info.walk_phases.back().wait_for_npc) ? 0 : frame::now();
                }, false);
            }
            out.pending=request_pending();return false;
        }
        return true;
    });
}

bool walk_edge(void* map_handle, void* player, const Cell& from, const Cell& to, bool goal_edge,
               route_progress::VehicleAction vehicle_action, int transportation_id)
{
    const baked::Submap* sm = current_submap(map_handle);
    if (!sm) return false; // missing bake is a data bug; do not approve an unknown edge
    const auto& g = baked::grids(*sm);
    baked_route::Request req;
    req.submap = sm;
    req.start = from;
    req.goals = {to};
    req.passages = game::flags().has_hidden_passages;
    world_mode(req);
    // Reuse the native movement collision query for this single edge. The game
    // owns moving NPCs, gates and directional collisions; never scan a map here.
    void* transform = unity::transform(player);
    if (!transform) return false;
    auto position = unity::local_position(transform);
    int dx = to.x - from.x, dy = to.y - from.y;
    if (sm->world && std::abs(dx) > 1) dx = dx > 0 ? -1 : 1;
    if (sm->world && std::abs(dy) > 1) dy = dy > 0 ? -1 : 1;
    position.x += dx * kTile; position.y -= dy * kTile;
    if (!native_position_walkable(player, position)) return false;
    // Stairs may be open on either the incoming or outgoing layer, just like the planner.
    const Cell incoming{from.layer, to.x, to.y};
    const bool open = baked_route::open(req, g, to) ||
        (from.layer != to.layer && baked_route::open(req, g, incoming));
    bool boundary = false;
    const auto* transport = baked::transport(transportation_id);
    if (sm->world && transport && std::abs(dx) + std::abs(dy) == 1) {
        auto foot = req; foot.vehicle_group = foot.vehicle_type = foot.landing_group = 0;
        if (vehicle_action == route_progress::VehicleAction::WalkOff && field::current_vehicle().id == transportation_id) {
            boundary = baked::vehicle_open(g, transport->group, from.x, from.y) && baked_route::open(foot, g, to) &&
                (transport->landing_group <= 0 || baked::group_lands(transport->landing_group, baked::attribute(g, to.x, to.y)));
        } else if (vehicle_action == route_progress::VehicleAction::WalkOn) {
            // The planned parked-vehicle edge was captured by the route worker.
            // Native collision above still owns boarding availability; do not
            // enumerate every live vehicle again on each input-axis callback.
            boundary = baked_route::open(foot, g, from) && baked::vehicle_open(g, transport->group, to.x, to.y);
        }
    }
    // Native exit/town cells can be closed in the static grid and explicitly entered as a
    // route goal. Keep that planner rule only for the final edge; live gates still veto it.
    return baked_route::edge_allowed(req, g, from, to) && (open || goal_edge || boundary);
}

// --- other floors (user, 2026-10-11) ---------------------------------------------------------
// "Current map" lists what is on the other floors too, and a route to one of them (or to a
// target cut off on this floor) goes on after the transition: the destination is remembered and
// the next leg is spoken from where the player arrives, until a leg ends at it. Before this the
// player was told which exit to take and then lost the destination, which was no longer in the
// list on the other side.
namespace {

struct Floor { const baked::Submap* submap; int map_id; int depth=0; };

/// The sub-maps reachable from `here` through exits and warps without crossing a world map,
/// each with the map id it was reached by (the exit's, so the floor's own title), in
/// breadth-first order; `here` itself is not included.
std::vector<Floor> connected_floors(const baked::Submap* here, int here_map_id, map_state::Resolver& state,
                                  const std::function<bool()>& cancelled = {})
{
    std::vector<Floor> out;
    if (!here || here->world) return out;
    std::vector<int> seen{baked::submap_index(here)};
    std::vector<Floor> queue{Floor{here, here_map_id}};
    for (size_t qi = 0; qi < queue.size() && queue.size() < static_cast<size_t>(kMaxVisitedMaps); ++qi) {
        if (cancelled && cancelled()) return {};
        const Floor f = queue[qi];
        for (const Edge& e : edges_of(baked::grids(*f.submap), f.map_id)) {
            if (!state.active(*f.submap, *e.object)||state.consumed(*f.submap,*e.object)) continue;
            const baked::Submap* dest = baked::submap_by_map_id(e.dest_map_id);
            if (!dest || dest->world) continue;
            const int idx = baked::submap_index(dest);
            if (std::find(seen.begin(), seen.end(), idx) != seen.end()) continue;
            seen.push_back(idx);
            queue.push_back(Floor{dest, e.dest_map_id,f.depth+1});
            out.push_back(Floor{dest, e.dest_map_id,f.depth+1});
        }
    }
    return out;
}

// Only the remote listing consumes this set. The full directed graph and native
// door/PointIn objects remain unchanged for every route and current-floor row.
std::unordered_set<const baked::Object*> remote_return_doors(const baked::Submap* here,int map_id,
    const std::vector<Floor>& floors,map_state::Resolver& state,const std::function<bool()>& cancelled) {
    std::unordered_set<const baked::Object*> hidden;
    if(floors.empty())return hidden;
    std::vector<Floor> all{{here,map_id,0}};all.insert(all.end(),floors.begin(),floors.end());
    std::map<int,int> depth;
    for(const auto& floor:all)depth[baked::submap_index(floor.submap)]=floor.depth;
    struct Arrival {baked_route::Cell cell;int count=0,direction=0;};
    std::map<std::pair<int,int>,Arrival> points;
    std::vector<remote_exits::Door> doors;
    std::vector<const baked::Object*> originals;
    for(const auto& floor:all){
        if(cancelled&&cancelled())return {};
        const int source=baked::submap_index(floor.submap);
        const auto& grid=baked::grids(*floor.submap);
        for(const auto& original:grid.objects){
            if(cancelled&&cancelled())return {};
            if(original.type!=baked::ObjType::PointIn&&original.type!=baked::ObjType::GotoMap)continue;
            if(!state.active(*floor.submap,original)||state.consumed(*floor.submap,original))continue;
            const auto object=state.effective(*floor.submap,original);
            if(object.tx<0||object.ty<0||object.tx>=grid.width||object.ty>=grid.height)continue;
            if(object.type==baked::ObjType::PointIn){
                auto& point=points[{source,object.a}];point.cell={object.layer,object.tx,object.ty};point.direction=object.b;++point.count;
                continue;
            }
            const auto* destination=baked::submap_by_map_id(object.a);
            const int target=destination?baked::submap_index(destination):-1;
            const auto level=depth.find(target);
            remote_exits::Door door;door.source=source;door.destination=target;door.source_depth=floor.depth;
            door.destination_depth=level==depth.end()?-1:level->second;door.footprint=object;
            door.known=destination&&!destination->world&&state.known(*floor.submap)&&state.known(*destination);
            doors.push_back(door);originals.push_back(&original);
        }
    }
    for(auto& door:doors){
        const auto at=points.find({door.destination,door.footprint.b});
        // Unlike route fallback, suppression requires exactly one explicit
        // active PointIn with this ID; missing/duplicate defaults stay visible.
        if(at!=points.end()&&at->second.count==1){door.arrival=at->second.cell;door.arrival_direction=at->second.direction;}
    }
    const auto mask=remote_exits::return_mask(doors,cancelled);
    for(size_t i=0;i<mask.size();++i)if(mask[i])hidden.insert(originals[i]);
    return hidden;
}


} // namespace

std::vector<RemoteObject> connected_objects(void* map_handle)
{
    const baked::Submap* here = current_submap(map_handle);
    if (!here || here->world) return {};
    map_state::Resolver state;
    return connected_objects_snapshot(planning_map_id(), state);
}
std::vector<RemoteObject> connected_objects_snapshot(int map_id, map_state::Resolver& state, std::function<bool()> cancelled)
{
    std::vector<RemoteObject> out;
    if (!prepared()) return out;
    const baked::Submap* here = baked::submap_by_map_id(map_id);
    if (!here || here->world) return out;
    return hooks::guarded_r("BakedNav.ConnectedObjects", out, [&] {
        const std::vector<Floor> floors = connected_floors(here, map_id, state, cancelled);
        const auto returns=remote_return_doors(here,map_id,floors,state,cancelled);
        for (const Floor& f : floors) {
            const baked::Grids& g = baked::grids(*f.submap);
            for (const baked::Object& original : g.objects) {
                if (cancelled && cancelled()) return std::vector<RemoteObject>{};
                if (!state.active(*f.submap, original)||state.consumed(*f.submap,original)) continue;
                const auto o=state.effective(*f.submap,original);
                if (o.tx>=g.width || o.ty>=g.height || o.tx+o.tw<=0 || o.ty+o.th<=0) continue;
                RemoteObject r;
                r.map_id = f.map_id;
                r.cell = Cell{o.layer, o.tx, o.ty};
                r.type = o.type;
                r.raw_name = baked::object_identifier(o);
                r.group = baked::str(o.group);
                r.entity_id = o.entity_id();
                switch (o.type) {
                case baked::ObjType::NPC:
                case baked::ObjType::ShopNPC:
                    if (!(o.d & (4 | 8))) continue;   // neither script nor action: scenery (the live rule)
                    r.shop = o.type == baked::ObjType::ShopNPC;
                    r.scripted = (o.d & 4) != 0;
                    break;
                case baked::ObjType::TreasureBox:
                    r.opened = state.treasure_opened(*f.submap, o);
                    r.pickup=baked::pickup_treasure(o);
                    if(r.pickup && r.opened.value_or(false))continue;
                    r.content_id = o.chest_content_id();
                    break;
                case baked::ObjType::SavePoint:
                    break;
                case baked::ObjType::Entity:
                // Animated manual objects (including III's Lute/Fang) have the
                // same action/script contract; their animation is not scenery.
                case baked::ObjType::AnimEntity:
                    if (!(o.d & (4 | 8))) continue; // scripted/actionable; never empty decoration
                    r.event_type = L"Interactive Object";
                    break;
                case baked::ObjType::Event:
                case baked::ObjType::RandomEvent:
                case baked::ObjType::SwitchEvent:
                    if (!o.a && !o.b) continue;
                    if(const auto* exit=baked::world_exit_at(g,o.entity_id(),r.raw_name,o.tx,o.ty)) {
                        r.world_exit=true;r.dest_map_id=exit->a;
                    } else r.event_type = L"Event";
                    break;
                case baked::ObjType::TelepoPoint:
                    // Native MapManager.CreateEntity returns null for type20
                    // in all six games. These are Teleport return-cache metadata,
                    // paired with a real GotoMap/PointIn, not playable destinations.
                    continue;
                case baked::ObjType::TransportationEventAction:
                    if (!o.a && !o.b) continue;
                    // The bake prefixes this type's name with transport IDs for
                    // movement rules. Labels and existing pad grouping use the name.
                    r.event_type = L"Event";
                    break;
                case baked::ObjType::Warp:
                    // The original trigger and its routing arc describe one event.
                    if (std::any_of(g.objects.begin(), g.objects.end(), [&](const auto& original) {
                        return original.type != baked::ObjType::Warp && original.entity_id() == o.entity_id() &&
                            original.group == o.group && original.tx == o.tx && original.ty == o.ty;
                    })) continue;
                    r.event_type = (o.d&255) == 1 ? L"Event" : L"Interactive Object";
                    break;
                case baked::ObjType::GotoMap:
                    // Retain genuine room entrances and all routing edges. Only
                    // a proven ordinary reciprocal return is omitted remotely.
                    if(o.a<=0||returns.contains(&original))continue;
                    r.dest_map_id = o.a;
                    break;
                default:
                    continue;
                }
                out.push_back(r);
            }
        }
        LOGF("[BakedNav] other floors of map %d: %zu floors, %zu objects", map_id, floors.size(), out.size());
        return out;
    });
}

#include "world_map_inventory.inl"

namespace {
// The native Player row and a remembered craft on another map both describe
// walking here. An actual or unknown-map local vehicle remains out of scope.
bool remote_foot_context(const baked::Submap& here) {
    if(!g_route_snapshot)return false;
    const auto& vehicle=g_route_snapshot->vehicle;
    return !vehicle.valid||vehicle.type<=1||(vehicle.map_id>0&&vehicle.map_id!=planning_map_id());
}
// Recognize the prior ordinary door leg independently of its pending receipt:
// a missing/changed receipt must invalidate that leg, while Warp and vehicle
// plans stay on their established planner and lifecycle paths.
bool remote_door_candidate(const baked::Submap& here) {
    if(!g_route_snapshot||!g_active||!g_active->valid||here.world||!remote_foot_context(here)||
       g_active->requires_trap||g_active->blocking_party>=0||g_active->npc_approach||
       g_active->next_map_id<=0||g_active->map_id!=planning_map_id()||g_active->goal_map_id==planning_map_id()||
       (g_pending&&g_pending->incoming_trap)||enterprise_conversion_plan(*g_active)||g_active->phases.empty()||
       g_active->phases.back().cells.empty())return false;
    const auto* destination=baked::submap_by_map_id(g_active->goal_map_id);
    if(!destination||destination->world)return false;
    for(const auto& phase:g_active->phases)if(phase.transportation_id!=0||phase.vehicle_before!=route_progress::VehicleAction::None)return false;
    const Cell end=g_active->phases.back().cells.back();bool door=false;
    for(const auto& object:baked::grids(here).objects) {
        if(planning_cancelled()){invalidate_active();return false;}
        if(!object.contains(end.x,end.y)||object.a!=g_active->next_map_id)continue;
        if(object.type==baked::ObjType::Warp)return false;
        if(object.type==baked::ObjType::GotoMap)door=true;
    }
    return door;
}
// Preserve a walked indoor remote leg only while the same owned destination,
// exact first-hop receipt and every remaining native edge are still valid.
// No native objects leave the captured snapshot and no worker is waited on.
bool keep_remote_foot_leg(const baked::Submap& here,const MapGoal& goal,TargetKind kind,PathInfo& out) {
    if(!g_route_snapshot||!g_active||!g_pending||here.world||g_active->map_id!=planning_map_id()||
       g_active->goal_map_id!=goal.map_id||!(g_active->target==goal.cell)||!g_active->valid||
       g_active->next_map_id<=0||g_active->requires_trap||g_active->blocking_party>=0||g_active->npc_approach||enterprise_conversion_plan(*g_active)||
       g_active->transportation_id!=(g_route_snapshot->vehicle.valid?g_route_snapshot->vehicle.id:0)||
       !remote_foot_context(here)||g_active->vehicle_shortcuts!=g_route_snapshot->vehicle_shortcuts||
       g_active->target_vehicle!=goal.transportation_id||!g_pending->valid||g_pending->kind!=kind||
       g_pending->goal.map_id!=goal.map_id||!(g_pending->goal.cell==goal.cell)||
       g_pending->goal.entity_id!=goal.entity_id||g_pending->goal.raw_name!=goal.raw_name||
       g_pending->goal.transportation_id!=goal.transportation_id)return false;
    for(const auto& phase:g_active->phases)if(phase.transportation_id!=0||phase.vehicle_before!=route_progress::VehicleAction::None)return false;
    const auto* destination=baked::submap_by_map_id(goal.map_id);
    if(!destination||destination->world||g_pending->incoming_trap||!g_pending->incoming_transition)return false;
    const auto& receipt=g_pending->incoming_transition;
    if(!receipt||receipt->destination!=g_active->next_map_id||baked::submap_by_map_id(receipt->map_id)!=&here)return false;
    const auto& g=baked::grids(here);auto state=planning_state();const baked::Object* hop=nullptr;
    for(const auto& o:g.objects) {
        if(planning_cancelled())return false;
        if(o.type!=baked::ObjType::GotoMap||o.entity_id()!=receipt->entity_id||
           o.tx!=receipt->x||o.ty!=receipt->y||(o.a!=receipt->destination&&!(o.a==-1&&receipt->destination==planning_map_id()))||
           !state.active(here,o)||state.consumed(here,o))continue;
        const auto* dest=baked::submap_by_map_id(receipt->destination);const auto* arrival=dest?point_in(*dest,o.b,state):nullptr;
        if(arrival&&Cell{arrival->layer,arrival->tx,arrival->ty}==receipt->arrival){hop=&o;break;}
    }
    if(!hop)return false;
    const Cell start=cell_of(here,snapshot_position(),snapshot_layer());std::vector<Phase> rest;
    if(!remaining_from(*g_active,start,rest)||rest.empty()||rest.back().cells.empty()||!(rest.back().cells.back()==receipt->entry))return false;
    auto req=walk_request(here,start);const auto live=live_blockers(here,g,{});
    req.blocked=live.hard();req.directional=live.directional;live.apply_transport(req);req.live_hazards=true;req.hazards=live.triggers;
    req.live_traps=true;req.traps=live.traps;req.ignore_hazards=true;req.goals={receipt->entry};req.goals_open=true;
    if(receipt->entry.layer!=hop->layer||!hop->contains(receipt->entry.x,receipt->entry.y))return false;
    auto blocked=[&](const Cell& c){return std::any_of(req.blocked.begin(),req.blocked.end(),[&](const Cell& b){return b.x==c.x&&b.y==c.y&&(b.layer<0||b.layer==c.layer);});};
    Cell prior=start;bool first=true;
    for(const auto& phase:rest) {
        if(phase.cells.empty()||!(phase.cells.front()==prior))return false;
        for(const auto& c:phase.cells) {
            if(planning_cancelled()||!baked::in_bounds(g,c.x,c.y)||blocked(c))return false;
            const bool final=c==receipt->entry;
            if(baked_route::trap_cell(req,g,c))return false;
            if(first){first=false;if(baked_route::terminal_transition(req,g,c)&&!final)return false;continue;}
            if(c==prior)continue;
            if(baked_route::layer_after(g,prior.layer,c.x,c.y)!=c.layer||!baked_route::edge_allowed(req,g,prior,c)||
               (!baked_route::open(req,g,c)&&!(c.layer!=prior.layer&&baked_route::open(req,g,{prior.layer,c.x,c.y}))&&!final))return false;
            prior=c;
        }
    }
    fill_result(here,rest,out);out.required_trap=g_active->requires_trap;out.next_map_id=g_active->next_map_id;
    for(const auto& phase:rest) {
        const auto movement=phase_movement(req,phase);
        if(event_on_route(phase.cells,live.triggers,{receipt->entry},&movement)){out.event_on_route=true;break;}
    }
    return true;
}
}

namespace {
#include "world_map_journey.inl"
#include "world_air_journey.inl"
}

bool route_to_goal(void* map_handle, void* player, const MapGoal& goal_in, TargetKind kind, const std::wstring& name,
                   PathInfo& out)
{
    out = PathInfo{};
    const baked::Submap* here = current_submap(map_handle);
    const baked::Submap* there = baked::submap_by_map_id(goal_in.map_id);
    if (!here || !there || (!player && !g_route_snapshot)) {
        out.error = L"No path found";
        return false;
    }
    return hooks::guarded_r("BakedNav.RouteToGoal", false, [&] {
        const int map_id = planning_map_id();
        MapGoal goal = goal_in;
        std::vector<Cell> remote_npc_cells;
        if (goal.entity_id >= 0 && goal.transportation_id<=0) {
            auto state = planning_state();
            bool active = false,matched=false,all_consumed=true, npc_identity=false;
            for (const auto& object : baked::grids(*there).objects)
                if ((object.type == baked::ObjType::NPC || object.type == baked::ObjType::ShopNPC) &&
                    object.entity_id() == goal.entity_id &&
                    (goal.raw_name.empty() || baked::object_identifier(object) == goal.raw_name)) npc_identity=true;
            for (const auto& object : baked::grids(*there).objects)
                if (object.type != baked::ObjType::TelepoPoint && object.type != baked::ObjType::PointIn &&
                    object.type != baked::ObjType::AnimEntity && object.type != baked::ObjType::Warp &&
                    object.entity_id() == goal.entity_id && object.contains(goal.cell.x, goal.cell.y) &&
                    (goal.raw_name.empty()||baked::object_identifier(object)==goal.raw_name)) {
                    matched=true;
                    const bool used=state.consumed(*there,object);
                    all_consumed &= used;
                    const bool current_scene=there==here&&air_transitions::saronia_entry(
                        static_cast<int>(game::current()),goal.map_id,object);
                    active |= (current_scene?planning_object_active(*there,object):state.active(*there,object))&&!used;
                }
            // A ready current native instance is authoritative if a remote initializer
            // cannot be projected. Other-floor targets must have known active state.
            if (!active && (!matched||!all_consumed) && there == here && g_route_snapshot)
                for (const auto& observed : g_route_snapshot->obstacles)
                    active |= observed.id == goal.entity_id && observed.cell.x == goal.cell.x && observed.cell.y == goal.cell.y &&
                        (goal.raw_name.empty()||observed.raw_name==goal.raw_name);
            // A current-floor NPC must still exist in the authoritative live
            // snapshot. The bake is a candidate, not proof an enemy survived.
            if (npc_identity && there == here && g_route_snapshot)
                active = selected_live_npc(*g_route_snapshot) != nullptr;
            if (!active) {
                out.reason = T(L"Destination is no longer available");
                if(goal.map_id!=map_id&&remote_door_candidate(*here))invalidate_active();
                clear_pending(); return true;
            }
            if(npc_identity&&there!=here&&kind==TargetKind::Interact) {
                // A future-floor actor can start in a scenery-only staging cell
                // and move into the playable room on entry. Keep its immutable
                // identity/home as the destination; only the first-leg proof may
                // use exact native route positions selected by this initializer.
                // Once that floor loads, selected_live_npc owns position/arrival.
                const baked::Object* source=nullptr;
                for(const auto& object:baked::grids(*there).objects)
                    if((object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC)&&
                        object.entity_id()==goal.entity_id&&object.contains(goal.cell.x,goal.cell.y)&&
                        (goal.raw_name.empty()||baked::object_identifier(object)==goal.raw_name)&&state.active(*there,object)) {
                        if(source){source=nullptr;break;}source=&object;
                    }
                if(source) {
                    const auto positions=state.route_positions(*there,*source);
                    if(!positions.empty()) {
                        auto movement=walk_request(*there,goal.cell);
                        map_state::add_relays(state,*there,movement);
                        std::vector<Cell> approaches;
                        for(const auto& pose:positions) {
                            if(planning_cancelled())break;
                            auto actor=*source;actor.layer=static_cast<uint8_t>(pose.layer);
                            actor.tx=static_cast<int16_t>(pose.x);actor.ty=static_cast<int16_t>(pose.y);
                            const auto native=baked_route::goals_for(movement,pose,baked_route::GoalKind::Interact,&actor);
                            for(const auto& cell:native.cells)if(std::find(approaches.begin(),approaches.end(),cell)==approaches.end())
                                approaches.push_back(cell);
                        }
                        // 32 poses, at most three native interaction layers and
                        // bounded map-sized relay rays; no native work or search
                        // per pose. The existing cancellable graph runs once.
                        if(!planning_cancelled()&&!approaches.empty())remote_npc_cells=std::move(approaches);
                    }
                }
            }
        }
        const auto world_route=[&]() {
            if(!here->world||!there->world||kind!=TargetKind::Transition)return false;
            const auto journey=world_map_journey(goal);if(!journey.found)return false;
            out=PathInfo{};
            set_active(map_id,goal.cell,goal.map_id,journey.phases,journey.next_map);
            fill_result(*here,journey.phases,out);out.next_map_id=journey.next_map;
            remember_destination(goal,kind,name);
            if(g_pending) {
                g_pending->incoming_trap.reset();
                g_pending->incoming_transition=TrapVisit{map_id,journey.entity,journey.next_map,journey.crossing.x,journey.crossing.y,
                    journey.crossing,journey.arrival};
            }
            return true;
        };
        if(goal.map_id!=map_id&&world_route())return true;
        if (goal.cells.empty()) goal.cells = goals_for(*there, goal.cell, kind).cells;
        if (goal.cells.empty()) {
            out.error = L"No path found";
            // This rejection precedes find() and used to leave no destination
            // evidence in the log. Keep the owned values on the worker so a
            // bad live observation can be distinguished from a blocked route.
            LOGF("[BakedNav] no goal cells: map %d -> %d, target (%d,%d) L%d, kind %d, size %dx%d, vehicle %d/%d",
                map_id,goal.map_id,goal.cell.x,goal.cell.y,goal.cell.layer,static_cast<int>(kind),
                there->width,there->height,g_route_snapshot?g_route_snapshot->vehicle.id:0,
                g_route_snapshot?g_route_snapshot->vehicle.type:0);
            if(goal.map_id!=map_id&&remote_door_candidate(*here))invalidate_active();
            clear_pending();
            return true;
        }
        if (goal.map_id == map_id) {
            // This floor: the ordinary route (kept while it is walked, the facing at its end);
            // it goes through another floor and back only when this floor is cut (find).
            void* tr = g_route_snapshot ? nullptr : unity::transform(player);
            if (!tr && !g_route_snapshot) return false;
            if (!find(g_route_snapshot ? snapshot_position() : unity::local_position(tr), world_of_cell(*here, goal.cell), map_handle, player, true, kind, false, out))
                return false;
            if(!out.success&&world_route())return true;
            if (out.success && out.next_map_id > 0) {
                remember_destination(goal, kind, name);
                if (g_pending) {
                    g_pending->incoming_trap.reset();
                    g_pending->incoming_transition.reset();
                    if (!out.walk_phases.empty() && !out.walk_phases.front().cells.empty()) {
                        const auto cell = out.walk_phases.front().cells.back();
                        for (const auto& object : baked::grids(*here).objects)
                            if ((object.type == baked::ObjType::Warp || object.type == baked::ObjType::GotoMap) &&
                                (object.a == out.next_map_id || (object.a == -1 && out.next_map_id == map_id)) && object.contains(cell.x, cell.y)) {
                                auto state = planning_state();
                                const auto* dest = baked::submap_by_map_id(out.next_map_id);
                                const auto* arrival = dest ? point_in(*dest,object.b,state) : nullptr;
                                if (arrival) {
                                    g_pending->incoming_transition = TrapVisit{here->first_map_id, object.entity_id(), out.next_map_id, object.tx, object.ty,
                                        cell, {arrival->layer,arrival->tx,arrival->ty}};
                                    if (baked::is_trap(object)) {
                                        g_pending->incoming_trap = g_pending->incoming_transition;
                                        g_pending->incoming_transition.reset();
                                    }
                                }
                            }
                    }
                }
            } else if ((out.trap_repeat_blocked || out.transition_repeat_blocked) && g_pending) {
                g_pending->valid = false; g_pending->incoming_trap.reset(); g_pending->incoming_transition.reset();
            }
            else if(out.npc_approach)remember_destination(goal,kind,name);
            else clear_pending();
            return true;
        }
        if(remote_door_candidate(*here)) {
            if(keep_remote_foot_leg(*here,goal,kind,out))return true;
            invalidate_active(); // a rejected ordinary door leg cannot survive a failed fresh plan
        }
        // Derived patrol geometry belongs to this state/snapshot only. Never
        // retain it in Pending: a changed assignment must prove its route again.
        MapGoal across_goal=goal;
        if(!remote_npc_cells.empty())across_goal.cells=std::move(remote_npc_cells);
        CrossLeg leg = find_across(map_handle, player, across_goal, true);
        if (!leg.found && !leg.incomplete) leg = find_across(map_handle, player, across_goal, true, true);
        if(!leg.found&&!leg.incomplete) {
            const auto live=live_blockers(*here,baked::grids(*here),{});
            if(!live.actors.empty()) {
                auto proof=find_across(map_handle,player,across_goal,true,false,false);
                if(!proof.found&&!proof.incomplete)proof=find_across(map_handle,player,across_goal,true,true,false);
                Cell start;
                if(proof.found&&player_cell(map_handle,player,start)) {
                    auto request=walk_request(*here,start);
                    request.live_hazards=true;request.hazards=live.triggers;
                    request.live_traps=true;request.traps=live.traps;
                    if(auto approach=npc_approach(request,live,proof.cells)) {
                        out.npc_approach=true;out.reason=approach->action_after;
                        set_active(map_id,goal.cell,goal.map_id,{*approach},0);
                        g_route_snapshot->active.npc_approach=true;
                        fill_result(*here,{*approach},out);
                        out.event_on_route=event_on_route(approach->cells,live.triggers,{},&request);
                        remember_destination(goal,kind,name);
                        if(g_pending){g_pending->incoming_trap.reset();g_pending->incoming_transition.reset();}
                        return true;
                    }
                }
            }
        }
        if (!leg.found) {
            out.error = L"No path found";
            out.trap_repeat_blocked = leg.trap_repeat_blocked;
            out.transition_repeat_blocked = leg.transition_repeat_blocked;
            const Blockers live = live_blockers(*here, baked::grids(*here), {});
            // A gate on a later floor matters just as much as a gate beside
            // the player. The proof planner removes only gates throughout the
            // same graph; NPCs, treasure, terrain and trap rules stay intact.
            if (find_across(map_handle, player, across_goal, false).found) out.reason = T(L"Story gated");
            if (leg.trap_repeat_blocked) out.reason = T(L"Trap route would repeat. Choose another route.");
            if (leg.transition_repeat_blocked) out.reason = T(L"Route would repeat. Choose another route.");
            if (leg.incomplete) out.reason = T(L"No safe route found");
            if ((leg.trap_repeat_blocked || leg.transition_repeat_blocked) && g_pending) {
                g_pending->valid = false; g_pending->incoming_trap.reset(); g_pending->incoming_transition.reset();
            }
            else clear_pending();
            LOGF("[BakedNav] no route to map %d (%d,%d) from map %d; %ls", goal.map_id, goal.cell.x, goal.cell.y, map_id,
                 out.reason.empty() ? L"no reason known" : out.reason.c_str());
            return true;
        }
        const int next = leg.on_this_map ? 0 : leg.next_map_id;
        set_active(map_id, goal.cell, goal.map_id, leg.phases, next);
        if (g_route_snapshot) g_route_snapshot->active.requires_trap = leg.requires_trap;
        fill_result(*here, leg.phases, out);
        out.description = leg.description;
        out.required_trap = leg.requires_trap; out.event_on_route = leg.event_on_route;
        out.next_map_id = next;
        if (next > 0) {
            remember_destination(goal, kind, name);
            if (g_pending) g_pending->incoming_trap.reset();
            if (g_pending && !leg.trap_leg) g_pending->incoming_transition = TrapVisit{here->first_map_id,leg.trap_entity,next,leg.trap_x,leg.trap_y,
                leg.cells.back(),leg.trap_arrival};
            if (leg.trap_leg && g_pending)
                g_pending->incoming_trap = TrapVisit{here->first_map_id, leg.trap_entity, next, leg.trap_x, leg.trap_y,
                    leg.cells.back(),leg.trap_arrival};
        }
        else clear_pending();
        LOGF("[BakedNav] route to map %d (%d,%d) from map %d: %d steps here, %d in all, %ls", goal.map_id, goal.cell.x,
             goal.cell.y, map_id, leg.steps_here, leg.steps_total, next > 0 ? L"goes on" : L"ends there");
        return true;
    });
}

bool pending_goal(MapGoal& goal, TargetKind& kind, std::wstring& name)
{
    if (!g_pending || !g_pending->valid) return false;
    goal = g_pending->goal;
    kind = g_pending->kind;
    name = g_pending->name;
    return true;
}

void remember_destination(const MapGoal& goal, TargetKind kind, const std::wstring& name)
{
    if (!g_pending) g_pending = new Pending();
    const bool same = g_pending->goal.map_id == goal.map_id &&
        g_pending->goal.cell == goal.cell && g_pending->goal.entity_id == goal.entity_id &&g_pending->goal.raw_name==goal.raw_name &&
        g_pending->goal.transportation_id==goal.transportation_id;
    if (!same) {
        g_pending->completed_traps.clear(); g_pending->incoming_trap.reset();
        g_pending->completed_transitions.clear(); g_pending->incoming_transition.reset();
    }
    g_pending->valid = true;
    g_pending->goal = goal;
    g_pending->kind = kind;
    g_pending->name = name;
}

bool note_trap_start(int map_id, const Cell& at)
{
    auto observe = [&](Pending* pending) {
        if (!pending || !pending->valid) return false;
        bool same_map = false;
        for (auto* planned : {&pending->incoming_trap, &pending->incoming_transition}) {
            if (!*planned) continue;
            auto& hop = **planned;
            if (baked::submap_by_map_id(hop.map_id) != baked::submap_by_map_id(map_id) ||
                at.layer != hop.entry.layer || std::abs(at.x-hop.entry.x)+std::abs(at.y-hop.entry.y)>1) continue;
            hop.observed = true;
            same_map |= baked::submap_by_map_id(hop.destination) == baked::submap_by_map_id(map_id);
        }
        return same_map;
    };
    bool same = observe(g_pending);
    same |= observe(g_preview_pending);
    return same;
}
void record_map_arrival(int map_id, const Cell& at)
{
    auto record = [&](Pending* pending) {
        if (!pending || !pending->valid) return;
        const auto finish = [&](auto& planned, auto& completed) {
            if (!planned) return;
            const auto hop = *planned;
            const bool same_map = baked::submap_by_map_id(hop.map_id) == baked::submap_by_map_id(hop.destination);
            if (hop.observed && baked::submap_by_map_id(hop.destination) == baked::submap_by_map_id(map_id) &&
                (!same_map || at == hop.arrival) && completed.size() < static_cast<size_t>(kMaxVisitedMaps)) {
                completed.push_back(hop); planned.reset();
            }
        };
        finish(pending->incoming_trap,pending->completed_traps);
        finish(pending->incoming_transition,pending->completed_transitions);
    };
    record(g_pending); record(g_preview_pending);
}
void clear_preview_destination(const MapGoal& goal)
{
    if (g_preview_pending && g_preview_pending->goal.map_id==goal.map_id &&
        ((goal.entity_id>=0 && g_preview_pending->goal.entity_id==goal.entity_id &&
          g_preview_pending->goal.raw_name==goal.raw_name) || g_preview_pending->goal.cell==goal.cell))
        *g_preview_pending = Pending{};
}

void clear_pending(bool include_preview)
{
    if (g_pending) {
        g_pending->valid = false; g_pending->incoming_trap.reset(); g_pending->completed_traps.clear();
        g_pending->incoming_transition.reset(); g_pending->completed_transitions.clear();
    }
    if (include_preview && g_preview_pending) *g_preview_pending = Pending{};
}

void reset_transition_history()
{
    for (auto* pending : {g_pending,g_preview_pending}) if (pending) {
        pending->completed_transitions.clear();pending->incoming_transition.reset();
    }
}

void continue_pending()
{
    if (!g_pending || !g_pending->valid) return;
    void* map = field::map_handle();
    void* pl = field::player();
    if (!map || !pl) return;
    hooks::guarded("BakedNav.ContinuePending", [&] {
        const Pending p = *g_pending;   // route_to_goal re-remembers or clears it
        request_route(p.goal, p.kind, p.name, [name = p.name](PathInfo info) {
            std::wstring line = name.empty() ? std::wstring() : name + L": ";
            if (info.success && !info.description.empty()) line += info.description;
            else line += !info.reason.empty() ? info.reason : T(L"No path");
            if (!info.warning.empty() && ui::prefs::mod_warnings_enabled()) ffpr::say_sequence({info.warning, line}, false);
            else ffpr::say(line, false);
        });
    });
}

bool follow_goal(void* map_handle, void* player, const MapGoal& goal_in, TargetKind kind, Corner& out)
{
    out = Corner{};
    if (!ffpr::system::is_field_navigation_ready()) return false;
    const baked::Submap* sm = current_submap(map_handle);
    const baked::Submap* there = baked::submap_by_map_id(goal_in.map_id);
    if (!sm || !there || !player) return false;
    const int map_id = planning_map_id();
    if (goal_in.map_id == map_id) return follow(map_handle, player, world_of_cell(*sm, goal_in.cell), kind, out, goal_in.entity_id, goal_in.transportation_id);
    return hooks::guarded_r("BakedNav.FollowGoal", false, [&] {
        void* tr = unity::transform(player);
        if (!tr) return false;
        const Cell at = cell_of(*sm, unity::local_position(tr), sm->world ? 1 : player_layer(player));
        const auto state = cached_corner(sm, map_id, at, goal_in.cell, goal_in.map_id, out);
        if (state == CornerState::Pending) {out.pending=true;return false;}
        if (state == CornerState::Missing) {
            const double now = frame::now();
            if (g_follow_failed_at > 0 && now - g_follow_failed_at < kFollowRetrySeconds) return false;
            if (!request_pending()) request_route(goal_in, kind, L"", [](PathInfo info) {
                g_follow_failed_at = info.success && (info.walk_phases.empty()||!info.walk_phases.back().wait_for_npc) ? 0 : frame::now();
            }, false);
            out.pending=request_pending();return false;
        }
        return true;
    });
}

namespace {
worker_job::Owner g_route_owner;
worker_job::Owner g_quick_owner;
worker_job::Owner g_browse_owner;
struct QuickGrid {
    baked_route::Request movement;
    baked_route::Reach reach, passages, landed;
    int map_id = 0;
};
std::shared_ptr<const QuickGrid> g_quick_grid;
baked_route::Request g_quick_wanted;
bool g_quick_pending = false;
std::vector<std::function<void()>> g_quick_waiters;
void cancel_quick_work() {
    g_quick_owner.cancel(); g_quick_grid.reset(); g_quick_pending = false; g_quick_waiters.clear();
}
bool same_movement(const baked_route::Request& a, const baked_route::Request& b) {
    return a.submap == b.submap && a.start == b.start && a.vehicle_group == b.vehicle_group &&
        a.vehicle_type == b.vehicle_type && a.landing_group == b.landing_group && a.transportation_id == b.transportation_id && a.auto_groups == b.auto_groups;
}
baked_route::Request quick_movement(Vec3 from, void* map, void* player, bool with_vehicles) {
    baked_route::Request req;
    req.submap = current_submap(map);
    if (!req.submap) return req;
    req.start = cell_of(*req.submap, from, req.submap->world ? 1 : player_layer(player));
    req.ignore_hazards = true;
    world_mode(req);
    if (with_vehicles && req.submap->world && req.vehicle_group == 0)
        for (const auto& v : field::vehicles_on_map())
            if (const auto* row = baked::transport(v.transportation_id))
                if (row->type != kTransportContent && row->type != 1 && row->group > 1) req.auto_groups.push_back(row->group);
    return req;
}
void ensure_quick(baked_route::Request request, std::function<void()> ready = {}) {
    if (!request.submap) { if (ready) ready(); return; }
    if (g_quick_grid && same_movement(request, g_quick_grid->movement)) { if (ready) ready(); return; }
    if (ready) g_quick_waiters.push_back(std::move(ready));
    if (g_quick_pending && same_movement(request, g_quick_wanted)) return;
    g_quick_owner.cancel();
    g_quick_wanted = request; g_quick_pending = true;
    const int map_id = field::current_map_id();
    auto epoch = g_quick_owner.epoch(); const auto generation = epoch->load();
    request.cancelled = [epoch, generation] { return epoch->load() != generation; };
    // Copy native flags once for a new flood, never once per entity. Group
    // projection and relay collection run on Navigation with owned state.
    struct Input { baked_route::Request request; int map_id; bool passages; map_state::Resolver state; };
    worker_job::start(g_quick_owner, Input{std::move(request), map_id, game::flags().has_hidden_passages, map_state::Resolver()},
        [](Input& input) {
            map_state::add_relays(input.state,*input.request.submap,input.request);
            input.request.transitions=input.state.transitions(*input.request.submap);
            auto grid = std::make_shared<QuickGrid>();
            grid->map_id = input.map_id; grid->movement = input.request;
            grid->reach = baked_route::flood(input.request);
            grid->landed = land_after_ride(input.request,grid->reach);
            if (input.passages && !input.request.submap->world) {
                input.request.passages = true; grid->passages = baked_route::flood(input.request);
            }
            return grid;
        }, [](std::shared_ptr<QuickGrid> grid) {
            g_quick_pending = false;
            if (!grid || grid->map_id != field::current_map_id() || !ffpr::system::is_field_navigation_ready()) {
                g_quick_waiters.clear(); return;
            }
            g_quick_grid = std::move(grid);
            auto waiters = std::move(g_quick_waiters); g_quick_waiters.clear();
            for (auto& ready : waiters) ready();
        }, background::Lane::Navigation);
}
bool quick_query(Vec3 from, Vec3 to, void* map, void* player, TargetKind kind, bool with_vehicles, PathInfo& out) {
    auto req = quick_movement(from, map, player, with_vehicles);
    if (!req.submap) return false;
    ensure_quick(req);
    if (!g_quick_grid || !same_movement(req, g_quick_grid->movement)) { out.pending = true; return true; }
    const auto goals = goals_for(*req.submap, cell_of(*req.submap, to, 1), kind, &g_quick_grid->movement);
    const Cell target=cell_of(*req.submap,to,1);
    if(mounted_foot_target(req,target,kind,goals.face)) {
        auto foot=req;foot.vehicle_group=foot.vehicle_type=foot.landing_group=0;foot.goals=goals.cells;foot.goals_open=true;
        out.success=baked_route::transition_distance(foot,g_quick_grid->landed)>=0;
        if(!out.success)out.error=T(L"No path found");
        return true;
    }
    out.success = any_reachable(g_quick_grid->reach, goals.cells) || any_reachable(g_quick_grid->passages, goals.cells) ||
        any_reachable(g_quick_grid->landed, goals.cells);
    if (!out.success) out.error = T(L"No path found");
    return true;
}
bool g_request_pending = false;
uint64_t g_request_serial = 0;
void cancel_route_work() {
    if (g_route_snapshot) return;
    ++g_request_serial; g_request_pending = false;
    g_route_owner.cancel(); frame::cancel_label("BakedNav.Capture");
}
struct SnapshotScope {
    RouteSnapshot* old;
    const ActiveRoute* active;
    Pending* pending;
    explicit SnapshotScope(RouteSnapshot& snapshot)
        : old(g_route_snapshot), active(g_active), pending(g_pending) {
        g_route_snapshot = &snapshot; g_active = &snapshot.active; g_pending = &snapshot.pending;
        snapshot.live_transitions.reset(); // changed owned obstacle fixtures/captures cannot retain old trigger masks
        snapshot.transport_rules.reset();snapshot.transport_rules_prepared=false;
    }
    ~SnapshotScope() { g_route_snapshot = old; g_active = active; g_pending = pending; }
};
struct Planned {
    std::optional<DefenseContext> defense;
    double requested_at = 0, capture_ms = 0, compute_ms = 0, native_ms = 0, queue_ms = 0;
    unsigned capture_slices = 0;
    VehicleState vehicle;
    PathInfo path;
    std::shared_ptr<ActiveRoute> active;
    Pending pending;
    int map_id = 0;
    Cell start;
    std::wstring locale;
    MapGoal goal;
    std::vector<GuideTarget> candidates;
    std::shared_ptr<const map_state::Flags> anticipated_release;
    TargetKind kind = TargetKind::Infer;
    std::wstring name;
    bool keep_destination = true;
};
Planned plan_single_snapshot(RouteSnapshot& snapshot) {
    const double compute_started = frame::now();
    const auto state_signature = snapshot.state.signature();
    if (snapshot.pending.state_signature != state_signature) {
        snapshot.pending.completed_transitions.clear(); snapshot.pending.incoming_transition.reset();
        snapshot.pending.state_signature = state_signature;
    }
    if (snapshot.previous_active) snapshot.active = *snapshot.previous_active;
    modtext::LocaleScope locale(snapshot.locale);
    SnapshotScope scope(snapshot);
    snapshot.observed_goal=snapshot.goal.cell;
    bool vehicle_missing=snapshot.goal.transportation_id>0;
    if(snapshot.goal.transportation_id>0&&snapshot.goal.map_id==snapshot.map_id) {
        const auto model=std::find_if(snapshot.vehicles.begin(),snapshot.vehicles.end(),[&](const auto& vehicle){
            return vehicle.transportation_id==snapshot.goal.transportation_id;
        });
        if(model!=snapshot.vehicles.end()) {vehicle_missing=false;snapshot.goal.cell=cell_of(*snapshot.submap,model->pos,1);}
    }
    // Capture can finish after the player requested a moving encounter. Rebind
    // the owned goal to the captured actor rather than route to its old tile.
    if (const auto* actor = selected_live_npc(snapshot)) {
        snapshot.goal.cell=actor->cell;
        if(snapshot.submap->world)snapshot.goal.cell.layer=1;
    }
    Planned result;
    result.requested_at = snapshot.requested_at;
    result.vehicle = snapshot.vehicle;
    result.capture_ms = (snapshot.captured_at - snapshot.requested_at) * 1000;
    result.native_ms = snapshot.native_ms;
    result.capture_slices = snapshot.capture_slices;
    result.queue_ms = (compute_started - snapshot.captured_at) * 1000;
    result.map_id = snapshot.map_id;
    result.start = cell_of(*snapshot.submap, snapshot.position, snapshot.layer);
    result.locale = snapshot.locale;
    result.goal = snapshot.goal; result.kind = snapshot.kind;
    result.candidates = snapshot.candidates;
    result.defense=snapshot.defense;
    result.anticipated_release=snapshot.anticipated_release;
    result.name = snapshot.name; result.keep_destination = snapshot.keep_destination;
    if(vehicle_missing)result.path.reason=T(L"Destination is no longer available");
    else if (!route_to_goal(nullptr, nullptr, snapshot.goal, snapshot.kind, snapshot.name, result.path)) {
        result.path.reason = result.path.error = T(L"Navigation map data unavailable");
    }
    // A nearby event is not evidence that it unlocks this destination. Keep
    // the confirmed gate reason; authored story guidance owns causal next steps.
    result.active = std::make_shared<ActiveRoute>(std::move(snapshot.active)); result.pending = std::move(snapshot.pending);
    result.path.warning = route_warning(result.path, snapshot.goal);
    result.compute_ms = (frame::now() - compute_started) * 1000;
    return result;
}
DefenseAdvice defense_advice_snapshot(RouteSnapshot& snapshot,const DefenseContext& context) {
    DefenseAdvice out;
    if(context.enemies.size()>16)return out;
    modtext::LocaleScope locale(snapshot.locale);
    const auto old_goal=snapshot.goal;const auto old_position=snapshot.position;
    const auto old_kind=snapshot.kind;
    snapshot.goal=context.protected_actor;
    const auto* protected_actor=selected_live_npc(snapshot);
    if(!protected_actor){snapshot.goal=old_goal;return out;}
    const auto protected_cell=protected_actor->cell;
    int danger=4;
    for(const auto& enemy:context.enemies) {
        if(planning_cancelled())break;
        snapshot.goal=enemy;const auto* actor=selected_live_npc(snapshot);if(!actor)continue;
        if(actor->cell.layer>=0&&protected_cell.layer>=0&&actor->cell.layer!=protected_cell.layer)continue;
        const int distance=std::max(std::abs(actor->cell.x-protected_cell.x),std::abs(actor->cell.y-protected_cell.y));
        if(distance<danger){danger=distance;out.intercept=enemy;out.intercept->cell=actor->cell;}
    }
    if(out.intercept) {
        out.warning=context.warning;
        const auto original_obstacles=snapshot.obstacles;
        bool complete=true;int best=INT_MAX;
        for(int offset=0;offset<3;++offset) {
            const int slot=(context.current_party+offset)%3;
            if(!context.parties[static_cast<size_t>(slot)]){complete=false;continue;}
            // Compare the world after switching: release this corps' parked
            // marker and park the formerly active player. Other parties stay
            // solid, including narrow encounter approaches.
            snapshot.obstacles=original_obstacles;
            snapshot.obstacles.erase(std::remove_if(snapshot.obstacles.begin(),snapshot.obstacles.end(),[&](const auto& actor){return actor.party&&actor.party_slot==slot;}),snapshot.obstacles.end());
            if(slot!=context.current_party&&context.parties[static_cast<size_t>(context.current_party)]) {
                NativeObstacle active;active.party=true;active.party_slot=context.current_party;
                active.cell=*context.parties[static_cast<size_t>(context.current_party)];snapshot.obstacles.push_back(active);
            }
            snapshot.position=world_of_cell(*snapshot.submap,*context.parties[static_cast<size_t>(slot)]);
            snapshot.goal=*out.intercept;snapshot.kind=TargetKind::Interact;
            snapshot.previous_active.reset();snapshot.active={};snapshot.pending={};
            const auto plan=plan_single_snapshot(snapshot);
            if(plan.path.success&&plan.path.blocking_party<0&&!plan.path.npc_approach&&plan.path.step_count<best){best=plan.path.step_count;out.closest_party=slot;}
        }
        snapshot.obstacles=original_obstacles;
        // Do not claim a closest party while another party's pose is unknown.
        if(!complete)out.closest_party=-1;
        if(out.closest_party>=0)out.warning+=L" "+TF(L"Party {0} closest.",{std::to_wstring(out.closest_party+1)});
    }
    snapshot.goal=old_goal;snapshot.position=old_position;snapshot.kind=old_kind;
    return out;
}
Planned plan_snapshot(RouteSnapshot& snapshot) {
    if (snapshot.candidates.empty()) return plan_single_snapshot(snapshot);
    const auto started=frame::now();
    Planned first;
    const size_t count=std::min<size_t>(snapshot.candidates.size(),32);
    DefenseAdvice advice;
    if(snapshot.defense){SnapshotScope scope(snapshot);advice=defense_advice_snapshot(snapshot,*snapshot.defense);}
    std::vector<size_t> order;
    if(advice.intercept)for(size_t i=0;i<count;++i)
        if(snapshot.candidates[i].goal.entity_id==advice.intercept->entity_id)order.push_back(i);
    for(size_t i=0;i<count;++i)if(order.empty()||i!=order.front())order.push_back(i);
    for(size_t step=0;step<count;++step) {
        const auto i=order[step];
        if (snapshot.epoch && snapshot.epoch->load()!=snapshot.generation) break;
        if (snapshot.cancelled && snapshot.cancelled()) break;
        const auto& candidate=snapshot.candidates[i];
        snapshot.goal=candidate.goal; snapshot.kind=candidate.kind;
        snapshot.previous_active.reset(); snapshot.active={}; snapshot.pending={};
        auto result=plan_single_snapshot(snapshot);
        if(result.path.blocking_party<0&&!result.path.npc_approach)result.path.guidance=candidate.instruction;
        result.path.guidance_index=static_cast<int>(i);
        result.path.defense_warning=advice.warning;
        if(step==0) first=result;
        if(result.path.success) {result.compute_ms=(frame::now()-started)*1000;return result;}
    }
    first.compute_ms=(frame::now()-started)*1000;
    return first;
}
void resolve_plan_name(Planned& result, const std::wstring& name) {
    modtext::LocaleScope locale(result.locale);
    if (result.path.next_map_id <= 0) return;
    const auto token = L"\uE000" + std::to_wstring(result.path.next_map_id) + L"\uE001";
    const auto old_phrase = TF(L"take the exit to {0}", {token});
    const auto phrase = name.empty() ? T(L"take the exit") : TF(L"take the exit to {0}", {name});
    auto replace = [&](std::wstring& value) {
        size_t at = 0;
        while ((at = value.find(old_phrase, at)) != std::wstring::npos) {
            value.replace(at, old_phrase.size(), phrase); at += phrase.size();
        }
    };
    replace(result.path.description); replace(result.path.full_description);
    for (auto& phase : result.path.walk_phases) replace(phase.action_after);
    if (result.active) for (auto& phase : result.active->phases) replace(phase.action_after);
}
template<class Result> bool still_current(const Result& result) {
    if (!ffpr::system::is_field_navigation_ready() || field::current_map_id() != result.map_id) return false;
    Cell now;
    if (!player_cell(field::map_handle(), field::player(), now) || !(now == result.start)) return false;
    const auto vehicle = field::current_vehicle();
    return vehicle.valid == result.vehicle.valid && (!vehicle.valid ||
        (vehicle.id == result.vehicle.id && vehicle.type == result.vehicle.type));
}
void commit_plan(Planned& result) {
    g_request_pending = false;
    auto& pending = result.keep_destination ? g_pending : g_preview_pending;
    if (!pending) pending = new Pending();
    g_published_route = std::move(result.active); g_active = g_published_route.get();
    *pending = std::move(result.pending);
    LOGF("[BakedNav] ready map %d: capture %.3f ms (%u slices, native %.3f ms), queue %.3f ms, compute %.3f ms, total %.3f ms",
        result.map_id, result.capture_ms, result.capture_slices, result.native_ms, result.queue_ms,
        result.compute_ms, (frame::now() - result.requested_at) * 1000);
}
// Native owner only; no controller method invocation, allocation, hook or retained
// pointer. The map's controller registry is read only for a positively observed
//102 actor. A single4096-entry budget covers the ENTIRE snapshot, including all
// such actors and instruction scans; malformed/overload/unknown stays manual.
// Completed one-shot programs keep102 but clear currentInstruction. A live
// animation-only program is also stationary. Looping delays retain a later
// movement instruction; one-shot programs inspect only their remaining suffix.
bool instruction_will_move(void* fe,RouteSnapshot& snapshot) {
    constexpr unsigned limit=4096;
    if(!game::in(game::kFF3|game::kFF5|game::kFF6)||snapshot.routine_work>=limit||snapshot.routine_actors>=64)return false;
    ++snapshot.routine_actors;
    void* handle=field::map_handle();
    if(!kRoutineController.is(handle)||fRoutineManager.get()<0||fRoutineControllers.get()<0||
       fRoutineActor.get()<0||fRoutineCurrent.get()<0||fRoutineList.get()<0||
       fRoutineIndex.get()<0||fRoutineLoop.get()<0)return false;
    void* manager=ref::rd<void*>(handle,fRoutineManager);
    void* controllers=manager?ref::rd<void*>(manager,fRoutineControllers):nullptr;
    const int count=unity::list_count(controllers);
    if(count<=0||count>1024)return false;
    for(int i=0;i<count&&snapshot.routine_work<limit;++i) {
        ++snapshot.routine_work;void* controller=unity::list_at(controllers,i);
        if(!kRoutine.is(controller)||ref::rd<void*>(controller,fRoutineActor)!=fe)continue;
        if(!ref::rd<void*>(controller,fRoutineCurrent))return false;
        void* instructions=ref::rd<void*>(controller,fRoutineList);
        const int size=unity::list_count(instructions),index=ref::rd<int32_t>(controller,fRoutineIndex);
        if(size<=0||size>256||index<0||index>=size)return false;
        const int start=ref::rd<bool>(controller,fRoutineLoop)?0:index;
        for(int j=start;j<size&&snapshot.routine_work<limit;++j) {
            ++snapshot.routine_work;void* instruction=unity::list_at(instructions,j);
            if(kRoutineMove.is(instruction)||kRoutineLerp.is(instruction))return true;
        }
        return false;
    }
    return false;
}
NativeObstacle capture_obstacle(void* fe, RouteSnapshot& snapshot) {
    NativeObstacle out;
    void* tr = unity::transform(fe);
    void* property = entity_property(fe);
    out.raw_name=(property?ref::rd_str(property,r::fPropName):std::wstring()).substr(0,256);
    out.type = object_type_of(property);
    out.id = property && r::fPropEntityId.get() >= 0 ? ref::rd<int32_t>(property, r::fPropEntityId) : 0;
    out.cell = cell_of(*snapshot.submap, unity::local_position(tr), game_object_layer(fe) - 9);
    if (out.cell.layer < 0 || out.cell.layer > 2 || snapshot.submap->world) out.cell.layer = -1;
    if(property&&r::fPropTargetTransportationIdList.get()>=0) {
        void* ids=ref::rd<void*>(property,r::fPropTargetTransportationIdList);
        if(ids) {
            out.transportation_restricted=true;out.transportation_ids=0;
            const int count=unity::list_count(ids);
            // Actual all-six masters use at most4 entries/ID19. Observe at most
            //64 values; malformed or unsupported selectors remain conservative.
            out.transportation_complete=count>=0&&count<=64;
            if(out.transportation_complete)for(int i=0;i<count;++i) {
                const int id=unity::list_value<int32_t>(ids,i);
                if(id<=0||id>=64){out.transportation_complete=false;break;}
                out.transportation_ids|=uint64_t(1)<<id;
            }
            if(!out.transportation_complete)out.transportation_ids=~uint64_t(0);
        }
    }
    const int current_id=snapshot.vehicle.valid?snapshot.vehicle.id:1;
    out.applies=!out.transportation_restricted||(current_id>0&&current_id<64&&
        (out.transportation_ids&(uint64_t(1)<<current_id))!=0);
    if (out.type == kObjCollisionEntity) {
        out.collider = kCollider.is(fe);
        out.enabled = !out.collider || ref::rd<bool>(fe, fColliderEnable);
        out.collision = out.collider ? ref::rd<int32_t>(fe, fColliderId) : 0;
    }
    // SwitchEvent30 is EventSwitchTriggerEntity, a native EventTriggerEntity
    // subclass. Its current power can be rearmed by Reset/Hide/OnTriggerExit;
    // copy it for this snapshot rather than infer permanent consumption.
    out.trigger = (out.type == 2 || out.type == 28 || out.type == 30) && kTrigger.is(fe)
        && !entity_is_scenery(fe) && ref::rd<bool>(fe, fTriggerPower);
    out.chest = r::kFieldTresureBox.is(fe); out.npc = r::kFieldNonPlayer.is(fe);
    if(out.npc&&kNpcMovement.is(property)&&fNpcMoveType.get()>=0&&fNpcMoveStep.get()>=0) {
        // Synthetic/id-zero actors and moving actors near another NPC's baked
        // home must use their own routine.102 alone includes completed programs;
        // only a live controller with future movement can authorize waiting.
        const int type=ref::rd<int32_t>(property,fNpcMoveType),steps=ref::rd<int32_t>(property,fNpcMoveStep);
        const bool instruction_patrol=type==102&&instruction_will_move(fe,snapshot);
        out.movement_known=true;out.wanders=instruction_patrol||((type==2||type==3)&&steps>0);
    }
    if(out.npc && fCharaBox.get()>=0 && mBoxTrigger.ptr()) {
        void* box=ref::rd<void*>(fe,fCharaBox);
        if(!box || unity::alive(box)) {
            out.solidity_known=true;
            out.solid=box && !ref::call<bool>(mBoxTrigger,box);
        }
    }
    // Companion CanEnter/player movement explicitly swaps or passes this
    // follower. Its collider is a movement callback, not an impassable NPC.
    if(out.npc && party_entities::is_companion(fe)) {out.solidity_known=true;out.solid=false;}
    out.party=party_entities::is_native(fe);
    if(out.party)out.party_slot=party_entities::native_slot(fe);
    return out;
}
}

void cancel_requests() { cancel_route_work(); cancel_follow_work(); }
void interrupt_requests()
{
    ++g_request_serial;
    g_request_pending = false;
    g_quick_pending = false;
    g_route_owner.interrupt();
    g_follow_owner.interrupt();
    g_quick_owner.interrupt();
    g_browse_owner.interrupt();
}
bool request_pending() { return g_request_pending; }

void cancel_browse_route() {
    g_browse_owner.cancel();
    frame::cancel_label("BakedNav.BrowseCapture");
}

namespace {
struct BrowsePlanned {
    BrowseRoute route;
    double requested_at=0,capture_ms=0,native_ms=0,queue_ms=0,compute_ms=0;
    unsigned capture_slices=0;
    int map_id = 0;
    Cell start;
    VehicleState vehicle;
    MapGoal goal;
    TargetKind kind = TargetKind::Infer;
};
BrowsePlanned plan_browse_snapshot(RouteSnapshot& snapshot) {
    const double compute_started=frame::now();
    // Browsing always evaluates the current snapshot from scratch. Kept walking
    // plans and trap receipts belong to explicit navigation, not list labels.
    snapshot.previous_active.reset(); snapshot.active = {}; snapshot.pending = {};
    auto planned = plan_snapshot(snapshot);
    BrowsePlanned result;
    result.requested_at=planned.requested_at;result.capture_ms=planned.capture_ms;
    result.native_ms=planned.native_ms;result.capture_slices=planned.capture_slices;result.queue_ms=planned.queue_ms;
    result.map_id = planned.map_id; result.start = planned.start; result.vehicle = planned.vehicle;
    result.goal = std::move(planned.goal); result.kind = planned.kind;
    const auto& path = planned.path;
    result.route.direct = path.success && !path.next_map_id && !path.required_trap && !path.event_on_route;
    SnapshotScope scope(snapshot);
    const auto live = live_blockers(*snapshot.submap, baked::grids(*snapshot.submap), {});
    const bool selected_trap = snapshot.goal.map_id == snapshot.map_id &&
        std::any_of(baked::grids(*snapshot.submap).objects.begin(),baked::grids(*snapshot.submap).objects.end(),
            [&](const auto& object) { return baked::is_trap(object) && object.contains(snapshot.goal.cell.x,snapshot.goal.cell.y); });
    if(selected_trap)result.route.direct=false;
    // The selected step-on event is a usable destination. Do not count its own
    // final cell as an intervening event a second time after find() excluded it.
    std::vector<Cell> selected;
    if (!selected_trap && (snapshot.kind == TargetKind::Infer || snapshot.kind == TargetKind::AirshipTransition) && snapshot.goal.map_id == snapshot.map_id &&
        std::any_of(baked::grids(*snapshot.submap).objects.begin(), baked::grids(*snapshot.submap).objects.end(),
            [&](const auto& object) { return baked_route::step_event(object) && !baked::is_trap(object) && object.contains(snapshot.goal.cell.x,snapshot.goal.cell.y); }))
        selected = goals_for(*snapshot.submap,snapshot.goal.cell,snapshot.kind).cells;
    auto root_movement=walk_request(*snapshot.submap,cell_of(*snapshot.submap,snapshot.position,snapshot.layer));
    world_mode(root_movement);live.apply_transport(root_movement);
    const bool surface_endpoint=path.success&&!path.npc_approach&&path.blocking_party<0&&
        snapshot.goal.map_id==snapshot.map_id&&nautilus_surface::matches(static_cast<int>(game::current()),snapshot.map_id,
            snapshot.goal.cell,snapshot.goal.entity_id,snapshot.goal.raw_name);
    for (const auto& phase : path.walk_phases) {
        const auto movement=phase_movement(root_movement,phase);
        // Surfacing is this exact marker's requested destination action. It
        // does not make the route to the marker indirect. Keep intervening
        // actions/events and partial blocker approaches under the normal rule.
        const bool own_surface=surface_endpoint&&phase.manual_after&& &phase==&path.walk_phases.back()&&
            !phase.cells.empty()&&phase.cells.back()==snapshot.goal.cell;
        if (!phase.action_before.empty() || (!phase.action_after.empty()&&!own_surface) || event_on_route(phase.cells, live.triggers, selected,&movement))
            result.route.direct = false;
    }
    if (path.success && path.next_map_id != planned.map_id) result.route.via_map_id = path.next_map_id;
    result.route.reason = path.reason;
    if(selected_trap && result.route.reason.empty())result.route.reason=T(L"Warning: a trap is required on this route.");
    if (!result.route.direct && result.route.reason.empty() && path.success)
        for(size_t i=0;i<path.walk_phases.size();++i) {
            const auto& phase=path.walk_phases[i];
            const bool before=phase.transportation_id>0&&!phase.action_before.empty();
            const bool after=phase.manual_after&&!phase.action_after.empty();
            if(before||after) {
                // A usable owned-vehicle route is not a missing vehicle. Match
                // the same specific action/name spoken by explicit directions,
                // but don't present a distant boarding heading as the next step.
                int approach=0;
                for(size_t j=0;j<i;++j) if(!path.walk_phases[j].cells.empty())
                    approach+=static_cast<int>(path.walk_phases[j].cells.size())-1;
                if(!before)approach+=route_progress::steps(phase);
                const auto& action=before?phase.action_before:phase.action_after;
                // Before the first vehicle action the captured current mode
                // still owns movement. Parked-vehicle approaches are on foot;
                // the destination vehicle does not decide this verb.
                const auto& moving=snapshot.vehicle;
                const wchar_t* approach_key=L"Walk {0}, then {1}";
                if(moving.valid) {
                    if(moving.type==3||moving.type==8)approach_key=L"Fly {0}, then {1}";
                    else if(moving.type==2||moving.type==6||
                        (moving.type==7&&game::is(game::Id::FF4))|| // Hovercraft, not FFV's Wind Drake
                        (moving.type==5&&moving.id==2&&game::in(game::kFF1|game::kFF2|game::kFF3)))
                        approach_key=L"Sail {0}, then {1}";
                }
                result.route.reason=approach>0
                    ? TF(approach_key, {field::format_steps(approach*kTile),action})
                    : action;
                break;
            }
        }
    if (!result.route.direct && result.route.reason.empty() && path.success)
        result.route.reason = route_warning(path,snapshot.goal);
    result.compute_ms=(frame::now()-compute_started)*1000;
    // Route geometry, phase text and temporary plans are destroyed here on the
    // worker. Only small owned evidence goes back to the native owner.
    return result;
}
}

bool capture_vehicle_shortcuts_policy() {
    int mode=settings::get_int(ui::prefs::kPathfindingFilterMode,-1);
    if(mode<0)mode=ui::prefs::pathfinding_filter_enabled()?ui::prefs::kFilterCurrentFloor:ui::prefs::kFilterCurrentMap;
    return mode!=ui::prefs::kFilterCurrentFloor;
}
void request_browse_route(MapGoal goal, TargetKind kind, std::function<void(BrowseRoute)> publish) {
    const double requested_at=frame::now();
    cancel_browse_route();
    if (!ffpr::system::is_field_navigation_ready()) return;
    void* pl = field::player();
    const auto* submap = current_submap(field::map_handle());
    if (!submap || !pl || !unity::transform(pl)) { publish({}); return; }
    auto snapshot = std::make_shared<RouteSnapshot>(map_state::Resolver());
    snapshot->vehicle_shortcuts=capture_vehicle_shortcuts_policy();
    snapshot->requested_at = requested_at;
    snapshot->map_id = field::current_map_id(); snapshot->submap = submap;
    snapshot->position = unity::local_position(unity::transform(pl));
    snapshot->layer = submap->world ? 1 : player_layer(pl);
    snapshot->vehicle = field::current_vehicle(); snapshot->locale = modtext::lang();
    snapshot->goal = std::move(goal); snapshot->kind = kind;
    snapshot->epoch = g_browse_owner.epoch(); snapshot->generation = snapshot->epoch->load();
    auto entities = std::make_shared<EntityCursor>(); // native-owner closure only
    entities->cache_frame_roots=game::is(game::Id::FF3)&&snapshot->map_id==1;
    snapshot->native_ms=(frame::now()-requested_at)*1000;
    frame::every_frame([snapshot, entities, publish = std::move(publish)]() mutable {
        const double native_started=frame::now();
        if (snapshot->epoch->load() != snapshot->generation) return false;
        if (!ffpr::system::is_field_navigation_ready() || field::current_map_id() != snapshot->map_id) {
            cancel_browse_route(); return false;
        }
        CaptureBudget budget;
        bool more = true;
        while (budget.available()) {
            void* entity = nullptr;
            if (!(more = entities->next(entity))) break;
            if (live_entity(entity) && entity_is_present(entity) && unity::transform(entity))
                snapshot->obstacles.push_back(capture_obstacle(entity, *snapshot));
        }
        ++snapshot->capture_slices;
        if (more) {snapshot->native_ms+=(frame::now()-native_started)*1000;return true;}
        capture_vehicle_names(*snapshot);
        snapshot->captured_at = frame::now();
        snapshot->native_ms+=(snapshot->captured_at-native_started)*1000;
        worker_job::start(g_browse_owner, std::move(*snapshot), &plan_browse_snapshot,
            [publish = std::move(publish)](BrowsePlanned result) mutable {
                if (!still_current(result)) {
                    if (result.map_id > 0 && result.map_id == field::current_map_id() && ffpr::system::is_field_navigation_ready())
                        request_browse_route(std::move(result.goal), result.kind, std::move(publish));
                    return;
                }
                // In particular, do not commit_plan: browsing must not redirect a
                // beacon/autowalk, remember a destination or consume a trap receipt.
                if(result.map_id==1&&game::is(game::Id::FF3))
                    LOGF("[BakedNav] browse ready map %d: capture %.3f ms (%u slices, native %.3f ms), queue %.3f ms, compute %.3f ms, total %.3f ms",
                         result.map_id,result.capture_ms,result.capture_slices,result.native_ms,result.queue_ms,result.compute_ms,(frame::now()-result.requested_at)*1000);
                publish(std::move(result.route));
            }, background::Lane::Navigation);
        return false;
    }, "BakedNav.BrowseCapture", INT_MAX);
}

struct ExitSnapshot {
    RouteSnapshot route;
    bool with_vehicles;
    bool story_filter = false;
    explicit ExitSnapshot(bool vehicles, map_state::Resolver state = map_state::Resolver())
        : route(std::move(state)), with_vehicles(vehicles) {route.vehicle_shortcuts=vehicles&&game::is(game::Id::FF3);}
};
DefenseAdvice defense_advice(ExitSnapshot& capture,const DefenseContext& context,std::function<bool()> cancelled) {
    capture.route.cancelled=std::move(cancelled);
    SnapshotScope scope(capture.route);
    return defense_advice_snapshot(capture.route,context);
}
std::shared_ptr<ExitSnapshot> begin_exit_capture(bool with_vehicles, bool indoor, bool story_filter) {
    if (!ffpr::system::is_field_navigation_ready()) return {};
    void* pl = field::player();
    const auto* sm = current_submap(field::map_handle());
    if (!sm || !pl || !unity::transform(pl)) return {};
    if (!sm->world && !indoor) return {};
    auto capture = std::make_shared<ExitSnapshot>(with_vehicles);
    capture->story_filter = story_filter;
    auto& snapshot = capture->route;
    snapshot.map_id = field::current_map_id(); snapshot.submap = sm;
    snapshot.position = unity::local_position(unity::transform(pl));
    snapshot.layer = sm->world ? 1 : player_layer(pl);
    snapshot.vehicle = field::current_vehicle();
    snapshot.locale=modtext::lang();
    return capture;
}
void capture_exit_obstacle(ExitSnapshot& capture, void* entity) {
    if (live_entity(entity) && entity_is_present(entity) && unity::transform(entity))
        capture.route.obstacles.push_back(capture_obstacle(entity,capture.route));
}
void finish_exit_capture(ExitSnapshot& capture) {
    capture.route.auto_groups = field::owned_auto_vehicle_groups();
    if (capture.with_vehicles || capture.story_filter) {
        capture.route.vehicles = field::vehicles_on_map();
        for (auto& vehicle : capture.route.vehicles) vehicle.game = nullptr;
        if(game::is(game::Id::FF3)&&settings::get_int(ui::prefs::kPathfindingFilterMode,-1)==ui::prefs::kFilterWorldMap)
            capture.route.saved_vehicles=field::saved_world_vehicles();
    }
}
bool exit_snapshot_world(const ExitSnapshot& capture) { return capture.route.submap->world; }
bool exit_snapshot_stationary_inventory(const ExitSnapshot& capture) {
    return game::is(game::Id::FF3)&&capture.route.map_id==1&&capture.route.submap&&capture.route.submap->world&&!capture.route.obstacles.empty()&&
        std::none_of(capture.route.obstacles.begin(),capture.route.obstacles.end(),[](const auto& actor){return actor.npc||actor.party;});
}
bool teleport_landing_safe(ExitSnapshot& capture, Vec3 target) {
    auto& snapshot = capture.route;
    if (!snapshot.submap || !std::isfinite(target.x) || !std::isfinite(target.y) || !std::isfinite(target.z)) return false;
    SnapshotScope scope(snapshot);
    const auto& sm = *snapshot.submap;
    // Validate coordinates before conversion; float-to-int overflow is undefined.
    const double x = sm.width * .5 + target.x / kTile, y = sm.height * .5 - target.y / kTile;
    if (x < 0 || y < 0 || x >= sm.width || y >= sm.height) return false;
    const auto& g = baked::grids(sm);
    auto req = walk_request(sm, {snapshot.layer, static_cast<int>(x), static_cast<int>(y)});
    world_mode(req);
    req.passages = game::flags().has_hidden_passages;
    const auto live = live_blockers(sm, g, {});
    req.blocked = live.hard(); req.directional = live.directional;live.apply_transport(req);
    // World-map NPCs/chests are also solid teleport landings, even though world routing
    // normally only collects story gates there.
    if (sm.world) for (const auto& raw : snapshot.obstacles)
        if (raw.chest || (raw.npc && (!raw.solidity_known || raw.solid))) req.blocked.push_back(raw.cell);
    req.live_hazards = true; req.hazards = live.triggers;
    req.live_traps = true; req.traps = live.traps;
    return baked_route::safe_landing(req, g, req.start);
}
int exit_snapshot_vehicle(const ExitSnapshot& capture) { return capture.route.vehicle.valid ? capture.route.vehicle.id : 0; }
std::optional<AirTransitionTarget> nautilus_surface_target(ExitSnapshot& capture,std::function<bool()> cancelled,bool require_reachable) {
    auto& snapshot=capture.route;snapshot.cancelled=cancelled;
    modtext::LocaleScope locale(snapshot.locale);SnapshotScope scope(snapshot);
    const auto* sm=snapshot.submap;
    if(!game::is(game::Id::FF3)||snapshot.map_id!=nautilus_surface::map||!sm||!sm->world||planning_cancelled())return {};
    const auto* surface=baked::submap_by_map_id(nautilus_surface::destination);
    const auto* submarine=baked::transport(14);const auto& grid=baked::grids(*sm);
    const auto at=nautilus_surface::cell;
    // Pin the actual native surfacing terrain and corresponding surface water,
    // rather than turning an arbitrary underwater tile into a transition.
    if(!surface||!surface->world||!submarine||
       !baked::group_lands(submarine->landing_group,baked::attribute(grid,at.x,at.y))||
       baked::attribute(baked::grids(*surface),at.x,at.y)!=16)return {};
    int distance=-1;
    if(snapshot.vehicle.valid&&snapshot.vehicle.id==14&&snapshot.vehicle.type==kTransportSubmarine) {
        auto req=walk_request(*sm,cell_of(*sm,snapshot.position,snapshot.layer));world_mode(req);
        req.goals={at};req.goals_open=false;
        const auto live=live_blockers(*sm,grid,{});req.blocked=live.hard();req.directional=live.directional;live.apply_transport(req);
        req.live_hazards=true;req.hazards=live.triggers;req.live_traps=true;req.traps=live.traps;
        const auto path=baked_route::find(req);if(path.found)distance=static_cast<int>(path.cells.size())-1;
    }
    if(planning_cancelled()||(require_reachable&&distance<0))return {};
    AirTransitionTarget result;result.goal.map_id=nautilus_surface::map;result.goal.cell=at;
    result.goal.entity_id=nautilus_surface::entity;result.goal.raw_name=nautilus_surface::identity;
    result.position=world_of_cell(*sm,at);result.distance=distance;result.destination_map=nautilus_surface::destination;
    result.name=T(L"Surface near Doga's Village");return result;
}
std::optional<AirTransitionTarget> air_transition_target(ExitSnapshot& capture,const std::optional<MapGoal>& continued,
    const std::optional<MapGoal>& selected,std::function<bool()> cancelled,bool story_entry,bool require_reachable) {
    auto& snapshot=capture.route; snapshot.cancelled=cancelled; modtext::LocaleScope locale(snapshot.locale); SnapshotScope scope(snapshot);
    const auto* sm=snapshot.submap;
    if(!sm||!sm->world||!game::is(game::Id::FF3)||(snapshot.map_id!=1&&snapshot.map_id!=2&&snapshot.map_id!=4))return {};
    if(story_entry&&snapshot.map_id!=4)return {};
    if(story_entry&&std::none_of(baked::grids(*sm).objects.begin(),baked::grids(*sm).objects.end(),[&](const auto& o){
        return air_transitions::saronia_entry(3,4,o)&&planning_object_active(*sm,o)&&!snapshot.state.consumed(*sm,o);
    }))return {};
    const auto& g=baked::grids(*sm);auto req=walk_request(*sm,cell_of(*sm,snapshot.position,snapshot.layer));world_mode(req);
    if(!baked::in_bounds(g,req.start.x,req.start.y))return {};
    req.cancelled=cancelled;const auto live=live_blockers(*sm,g,{});req.blocked=live.hard();req.directional=live.directional;live.apply_transport(req);
    req.live_hazards=true;req.hazards=live.triggers;req.live_traps=true;req.traps=live.traps;
    struct Flight { int id,approach; baked_route::Request request; baked_route::Reach reach; };
    std::vector<Flight> flights;
    auto add=[&](int id,Cell at,int approach) {
        const int air=id==13?6:id==16?15:id;const auto* row=baked::transport(air);
        if(!row)return;
        if(story_entry ? (air!=6&&air!=15) : !air_transitions::possible_aircraft(static_cast<int>(game::current()),snapshot.map_id,air))return;
        auto movement=req;movement.start=at;movement.vehicle_group=row->group;movement.vehicle_type=row->type;movement.landing_group=row->landing_group;
        movement.transportation_id=air;movement.goals_open=false;
        baked_route::Reach reach;
        if(id==13||id==16) {auto enterprise=req;enterprise.start=at;enterprise.transportation_id=id;reach=baked_route::enterprise_route(enterprise,false,false,true).air;}
        else reach=baked_route::flood(movement);
        flights.push_back({air,approach,std::move(movement),std::move(reach)});
    };
    const int current_air=snapshot.vehicle.id==13?6:snapshot.vehicle.id==16?15:snapshot.vehicle.id;
    const bool mounted_air=snapshot.vehicle.valid&&(story_entry?(current_air==6||current_air==15):
        air_transitions::possible_aircraft(static_cast<int>(game::current()),snapshot.map_id,current_air));
    // Player1 is a valid native transportation. It is not a mounted flight,
    // and must not consume this branch before parked aircraft are considered.
    if(mounted_air)add(snapshot.vehicle.id,req.start,0);
    else if(capture.with_vehicles&&req.vehicle_group==0) {
        auto foot=req;foot.vehicle_group=foot.vehicle_type=foot.landing_group=0;const auto reach=baked_route::flood(foot);
        for(const auto& v:snapshot.vehicles) {
            if(cancelled&&cancelled())return {};
            const auto* row=baked::transport(v.transportation_id);if(!row)continue;
            const auto at=cell_of(*sm,v.pos,snapshot.layer);int best=-1;
            for(const auto& d:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}) {
                Cell beside{at.layer,(at.x+d.x+g.width)%g.width,(at.y+d.y+g.height)%g.height};
                if(!baked_route::open(foot,g,beside))continue;
                if((row->type==kTransportShip||row->type==kTransportSubmarine)&&!baked_route::direction_allowed(foot,beside,at))continue;
                const int n=reach.distance(beside);if(n>=0&&(best<0||n<best))best=n;
            }
            if(best>=0)add(v.transportation_id,at,best+1);
        }
    }
    std::optional<AirTransitionTarget> best;int best_priority=-1,best_metric=INT_MAX;
    auto identity=[&](const MapGoal& goal,const baked::Object& o,const std::wstring& raw) {
        return goal.map_id==snapshot.map_id&&goal.cell.x==o.tx&&goal.cell.y==o.ty&&
            (goal.entity_id<0||goal.entity_id==o.entity_id())&&(goal.raw_name.empty()||goal.raw_name==raw);
    };
    for(const auto& object:g.objects) {
        if(cancelled&&cancelled())return {};
        const auto* rule=story_entry?air_transitions::saronia_entry(3,snapshot.map_id,object):
            air_transitions::match(static_cast<int>(game::current()),snapshot.map_id,object);
        if(!rule||!planning_object_active(*sm,object)||(story_entry&&snapshot.state.consumed(*sm,object)))continue;
        Cell at{snapshot.layer,object.tx,object.ty};const auto raw=baked::object_identifier(object);
        int distance=-1;
        for(auto& flight:flights) {
            if(!air_transitions::accepts(*rule,flight.id)||!baked_route::open(flight.request,g,at))continue;
            if(std::any_of(flight.request.blocked.begin(),flight.request.blocked.end(),[&](const Cell& c){return c.x==at.x&&c.y==at.y&&(c.layer<0||c.layer==at.layer);}))continue;
            int d=flight.reach.distance(at);
            // The event may itself be terminal/hazard: enter only as the final edge.
            flight.request.goals={at};flight.request.terminal_goal=true;
            for(const auto& side:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}) {
                Cell from{at.layer,(at.x+side.x+g.width)%g.width,(at.y+side.y+g.height)%g.height};
                const int n=flight.reach.distance(from);if(n>=0&&baked_route::edge_allowed(flight.request,g,from,at)&&(d<0||n+1<d))d=n+1;
            }
            if(d>=0&&(distance<0||d+flight.approach<distance))distance=d+flight.approach;
        }
        if(distance<0) {
            if(require_reachable)continue;
            // Current map discovers the same physical provider independently
            // of an immediate ride. Keep one nearest genuine native trigger,
            // and leave distance unknown; selected routing still proves access.
            bool physical=false;
            for(const auto& row:baked::transports())if(air_transitions::accepts(*rule,row.id)) {
                auto flight=req;flight.vehicle_group=row.group;flight.vehicle_type=row.type;
                if(baked_route::open(flight,g,at)){physical=true;break;}
            }
            if(!physical)continue;
        }
        const int priority=continued&&identity(*continued,object,raw)?2:selected&&identity(*selected,object,raw)?1:0;
        const int dx=std::abs(at.x-req.start.x),dy=std::abs(at.y-req.start.y);
        const int metric=distance>=0?distance:std::min(dx,g.width-dx)+std::min(dy,g.height-dy);
        if(best) {
            if(priority<best_priority)continue;
            if(priority==best_priority) {
                if(best->distance>=0&&distance<0)continue;
                if((best->distance>=0)==(distance>=0)&&metric>=best_metric)continue;
            }
        }
        AirTransitionTarget value;value.goal.map_id=snapshot.map_id;value.goal.cell=at;value.goal.entity_id=object.entity_id();value.goal.raw_name=raw;
        value.position=world_of_cell(*sm,at);value.distance=distance;value.destination_map=rule->destination;
        if(rule->map==1&&rule->script==341) {
            const auto flag=snapshot.state.scenario_value(66);
            if(flag)value.destination_map=*flag?4:2;
        }
        const wchar_t* destination=rule->destination==1?L"Floating Continent":L"Surface World";
        value.name=story_entry?T(L"Saronia Airspace"):TF(L"Exit to {0}",{T(destination)});
        best=std::move(value);best_priority=priority;best_metric=metric;
    }
    return best;
}
std::vector<bool> story_gate_mask(ExitSnapshot& capture, const std::vector<StoryTarget>& targets,
                                 std::function<bool()> cancelled) {
    auto& snapshot = capture.route;
    snapshot.cancelled = std::move(cancelled);
    SnapshotScope scope(snapshot);
    std::vector<bool> hidden(targets.size(),false);
    const auto* here = snapshot.submap;
    if (!here || targets.empty() || planning_cancelled()) return hidden;
    const auto live = live_blockers(*here,baked::grids(*here),{});
    // One pair of graph traversals for the entire list, rather than a complete
    // route search per entity. Each arrival is flooded once per traversal. The
    // graph uses the planner's exact edge, gate, pressure-state and PointIn rules.
    std::unordered_map<int,std::vector<size_t>> rows;
    for (size_t i=0;i<targets.size();++i) {
        const auto& goal=targets[i].goal;
        const auto* sm=baked::submap_by_map_id(goal.map_id);
        if (!sm || goal.cell.layer<0 || goal.cell.layer>2 || !baked::in_bounds(baked::grids(*sm),goal.cell.x,goal.cell.y)) continue;
        rows[baked::submap_index(sm)].push_back(i);
    }
    struct Coverage { std::vector<bool> reached; bool complete=true; };
    const auto traverse = [&](bool gates) {
        Coverage out{std::vector<bool>(targets.size(),false),true};
        size_t remaining=0;
        for(const auto& [map,indices]:rows)remaining+=indices.size();
        if(!remaining)return out;
        struct Arrival { const baked::Submap* map; Cell at; };
        std::vector<Arrival> queue{{here,cell_of(*here,snapshot.position,snapshot.layer)}};
        for (size_t next=0;next<queue.size();++next) {
            if (planning_cancelled()) { out.complete=false; return out; }
            const auto [sm,at]=queue[next];
            const auto& grid=baked::grids(*sm);
            auto movement=walk_request(*sm,at);
            if (sm==here) {
                world_mode(movement);
                movement.blocked=live.hard(gates);
                live.apply_transport(movement,gates);
                if(gates)movement.directional=live.directional;
                movement.live_hazards=movement.live_traps=true;
                movement.hazards=live.triggers;movement.traps=live.traps;
            } else map_state::add_obstacles(snapshot.state,*sm,movement,gates);
            // Ordinary scene triggers must remain available as progression
            // actions, including destinations reached after their scene plays.
            movement.ignore_hazards=true;
            const auto reach=baked_route::flood(movement);
            baked_route::Reach landed,parked;
            auto foot=movement;foot.vehicle_group=foot.vehicle_type=foot.landing_group=0;
            if(sm->world) {
                if(movement.vehicle_group>0) {
                    landed=land_after_ride(movement,reach);
                    if(game::is(game::Id::FF3)&&snapshot.vehicle_shortcuts) {
                        auto walking=foot;walking.transportation_id=0;
                        parked=parked_vehicle_reach(walking,landed);
                    }
                }
                else parked=parked_vehicle_reach(movement,reach);
            }
            const auto found=rows.find(baked::submap_index(sm));
            if(found!=rows.end())for(size_t index:found->second) {
                if(planning_cancelled()) { out.complete=false;return out; }
                if(out.reached[index])continue;
                snapshot.goal=targets[index].goal;
                Cell target=snapshot.goal.cell;
                if(const auto* actor=selected_live_npc(snapshot))target=actor->cell;
                baked::Object actor;
                const auto kind=targets[index].kind;
                const auto* interaction=live_interaction(*sm,target,kind,actor)?&actor:nullptr;
                const auto test=[&](baked_route::Request request,const baked_route::Reach& flood) {
                    if(!flood.submap)return false;
                    request.goals=baked_route::goals_for(request,target,goal_kind(kind),interaction).cells;
                    request.allow_trap_goal=true;
                    return baked_route::transition_distance(request,flood)>=0;
                };
                out.reached[index]=test(movement,reach)||test(foot,landed)||test(foot,parked);
                if(out.reached[index])--remaining;
            }
            if(!remaining)return out;
            // Remote browsing never crosses a world map. A world-map list is
            // checked using walking, riding and legal owned-vehicle landings.
            if(sm->world)continue;
            for(const auto& edge:edges_of(grid,sm==here?snapshot.map_id:sm->first_map_id)) {
                if(planning_cancelled()) { out.complete=false;return out; }
                const auto& object=*edge.object;
                if(!snapshot.state.active(*sm,object)||snapshot.state.consumed(*sm,object))continue;
                const auto* dest=baked::submap_by_map_id(edge.dest_map_id);
                if(!dest||dest->world)continue;
                if(distance_to_object(movement,reach,object,at.layer,false)<0)continue;
                const auto* entry=point_in(*dest,edge.dest_point,snapshot.state);
                if(!entry)continue;
                const Cell arrival{entry->layer,entry->tx,entry->ty};
                if(std::any_of(queue.begin(),queue.end(),[&](const Arrival& old){return old.map==dest&&old.at==arrival;}))continue;
                if(queue.size()>=static_cast<size_t>(kMaxVisitedMaps)) { out.complete=false;continue; }
                queue.push_back({dest,arrival});
            }
        }
        // World-local entrances can lie beyond an actual dungeon return. Use
        // the same shared batch as eligibility, including the gate-disabled
        // proof pass; local terrain alone cannot establish their story gate.
        if(here->world&&!planning_cancelled()) {
            std::vector<const baked::Object*> doors;std::vector<size_t> indices;
            for(size_t i=0;i<targets.size();++i)if(!out.reached[i]&&targets[i].kind==TargetKind::Transition&&targets[i].goal.map_id==snapshot.map_id) {
                const auto& goal=targets[i].goal;
                for(const auto& object:baked::grids(*here).objects)if(object.type==baked::ObjType::GotoMap&&object.contains(goal.cell.x,goal.cell.y)&&
                    (goal.entity_id<0||goal.entity_id==object.entity_id())&&(goal.raw_name.empty()||goal.raw_name==baked::object_identifier(object))) {
                    doors.push_back(&object);indices.push_back(i);
                }
            }
            if(!doors.empty()) {
                MapGoal goal;goal.map_id=snapshot.map_id;std::vector<int> distances;
                const auto graph=find_across(nullptr,nullptr,goal,gates,true,true,true,&doors,&distances);
                for(size_t i=0;i<distances.size();++i)if(distances[i]>=0)out.reached[indices[i]]=true;
                if(graph.incomplete&&std::any_of(indices.begin(),indices.end(),[&](size_t i){return !out.reached[i];}))out.complete=false;
            }
        }
        return out;
    };
    const auto closed=traverse(true);
    if(!closed.complete || planning_cancelled())return hidden;
    bool remaining=false;
    for(const auto& [map,indices]:rows)for(size_t index:indices)remaining|=!closed.reached[index];
    if(!remaining)return hidden;
    const auto open=traverse(false);
    if(planning_cancelled())return hidden;
    for(size_t i=0;i<targets.size();++i)hidden[i]=!closed.reached[i]&&open.reached[i];
    return hidden;
}
std::vector<int> exit_distances(ExitSnapshot& capture, const std::vector<ExitTarget>& targets,
                              std::function<bool()> cancelled) {
    if(targets.empty())return {};
    auto& snapshot = capture.route;
    snapshot.cancelled = cancelled;
    SnapshotScope scope(snapshot);
    const auto* sm = snapshot.submap;
    const auto& g = baked::grids(*sm);
    auto req = walk_request(*sm,cell_of(*sm,snapshot.position,snapshot.layer));
    world_mode(req);
    req.cancelled = cancelled;
    const auto live = live_blockers(*sm,g,{});
    req.blocked = live.hard(); req.directional = live.directional;live.apply_transport(req);
    req.live_hazards = true; req.hazards = live.triggers;
    req.live_traps = true; req.traps = live.traps;
    const auto reach = baked_route::flood(req);
    const auto landed = land_after_ride(req,reach);
    baked_route::Reach parked;
    bool parked_ready=false;
    struct MountedReach {int id=0,approach=0; baked_route::Reach reach;};
    std::vector<MountedReach> mounted;
    bool mounted_ready=false;
    baked_route::Reach story_air;
    baked_route::Request story_flight;
    bool story_air_ready=false;
    auto foot = req; foot.vehicle_group = foot.vehicle_type = foot.landing_group = 0;
    const auto prepare_parked=[&] {
        if(parked_ready)return;
        parked_ready=true;
        if(!capture.with_vehicles)return;
        if(req.vehicle_group>0) {
            if(!game::is(game::Id::FF3))return;
            auto walking=foot;walking.transportation_id=0;
            parked=parked_vehicle_reach(walking,landed);
        } else parked=parked_vehicle_reach(req,reach);
    };
    std::vector<int> result(targets.size(),-1);
    // A sea destination cannot use a flood of positions reached after landing.
    // Share the actual owned boarding/sailing planner for entries and events.
    const auto ship_distance=[&](const Cell& at,const ShipGoal& goal,TargetKind kind) {
        if(!goal.source||!goal.ids||(cancelled&&cancelled()))return -1;
        if(goal.transition&&!snapshot.state.active(*sm,*goal.source))return -1;
        auto assisted=std::make_unique<RouteSnapshot>(snapshot);
        assisted->goal={};assisted->goal.map_id=snapshot.map_id;assisted->goal.cell=at;
        assisted->goal.entity_id=goal.source->entity_id();assisted->goal.raw_name=baked::object_identifier(*goal.source);
        assisted->kind=kind;if(cancelled)assisted->cancelled=cancelled;
        const auto plan=plan_snapshot(*assisted);
        return plan.path.success?plan.path.step_count:-1;
    };
    for (size_t i=0; i<targets.size(); ++i) {
        if (cancelled && cancelled()) return result;
        const auto& target = targets[i];
        if (target.map_id > 0 && target.map_id != snapshot.map_id) {
            MapGoal goal; goal.map_id = target.map_id; goal.cell = target.cell;
            if(sm->world&&baked::submap_by_map_id(target.map_id)&&baked::submap_by_map_id(target.map_id)->world&&game::is(game::Id::FF3)) {
                const auto journey=world_map_journey(goal);
                if(journey.found)result[i]=journey.total;
                continue;
            }
            auto leg = find_across(nullptr,nullptr,goal);
            if (!leg.found && !leg.incomplete) leg = find_across(nullptr,nullptr,goal,true,true);
            if (leg.found) result[i] = leg.steps_total;
            continue;
        }
        const Cell at = cell_of(*sm,target.position,snapshot.layer);
        const bool story_entry=game::is(game::Id::FF3)&&snapshot.map_id==4&&std::any_of(g.objects.begin(),g.objects.end(),[&](const auto& object){
            return object.contains(at.x,at.y)&&planning_object_active(*sm,object)&&
                air_transitions::saronia_entry(static_cast<int>(game::current()),snapshot.map_id,object);
        });
        if(story_entry) {
            // A scene entered in flight must not use town transition_distance:
            // that helper deliberately requires dismounting. One copied flight
            // flood serves the whole native trigger cluster, never one per row.
            if(!story_air_ready) {
                story_air_ready=true;
                if(snapshot.vehicle.valid&&baked_route::enterprise_pair(snapshot.vehicle.id)) {
                    story_flight=req;story_flight.vehicle_group=6;story_flight.vehicle_type=3;
                    story_flight.transportation_id=(snapshot.vehicle.id==15||snapshot.vehicle.id==16)?15:6;story_flight.landing_group=5;story_flight.goals_open=false;
                    if(baked_route::open(story_flight,g,story_flight.start))story_air=baked_route::flood(story_flight);
                }
            }
            if(story_air.submap&&baked_route::open(story_flight,g,at)&&
               std::none_of(story_flight.blocked.begin(),story_flight.blocked.end(),[&](const Cell& c){return c.x==at.x&&c.y==at.y&&(c.layer<0||c.layer==at.layer);})) {
                int distance=story_air.distance(at);
                story_flight.goals={at};story_flight.terminal_goal=true;
                for(const auto& side:std::array<Cell,4>{{{1,0,-1},{1,1,0},{1,0,1},{1,-1,0}}}) {
                    Cell from{at.layer,(at.x+side.x+g.width)%g.width,(at.y+side.y+g.height)%g.height};
                    const int d=story_air.distance(from);
                    if(d>=0&&baked_route::edge_allowed(story_flight,g,from,at)&&(distance<0||d+1<distance))distance=d+1;
                }
                result[i]=distance;
            }
            continue;
        }
        if(target.kind==TargetKind::Infer) {
            // The same flood can rank step-on event targets: the goal edge may
            // enter that event, but may not cross a different active trigger.
            auto movement=req;
            movement.goals=goals_for(*sm,at,TargetKind::Infer).cells;
            movement.allow_trap_goal=true;
            movement.terminal_goal=true; // only this selected native trigger may end the leg
            const auto ship_goal=ship_goal_at(*sm,at);
            if(!ship_goal||(snapshot.vehicle.valid&&(ship_goal->ids&baked_route::transportation_bit(snapshot.vehicle.id))))
                result[i]=baked_route::transition_distance(movement,reach,
                    ship_goal&&!ship_goal->transition?baked_route::GoalKind::Infer:baked_route::GoalKind::Transition);
            const auto* pad=vehicle_pad_at(*sm,at);
            if(pad) {
                // Pad activation belongs to a real mounted transport, never a
                // fictitious walking/landing goal on an impassable mountain.
                result[i]=snapshot.vehicle.valid&&req.vehicle_group>0&&baked::vehicle_pad_accepts(*pad,snapshot.vehicle.id)?reach.distance(at):-1;
                if(result[i]<0&&capture.with_vehicles&&req.vehicle_group==0) {
                    if(!mounted_ready) {
                        mounted_ready=true;
                        for(const auto& vehicle:snapshot.vehicles) {
                            if(cancelled&&cancelled())return result;
                            const auto* row=baked::transport(vehicle.transportation_id);
                            if(!row||row->type==1||row->type==kTransportContent)continue;
                            const Cell parked_at=cell_of(*sm,vehicle.pos,snapshot.layer);
                            int approach=-1;
                            static constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
                            for(const auto& side:sides) {
                                Cell beside{parked_at.layer,(parked_at.x+side[0]+g.width)%g.width,(parked_at.y+side[1]+g.height)%g.height};
                                if(!baked_route::open(foot,g,beside))continue;
                                if((row->type==kTransportShip||row->type==kTransportSubmarine)&&!baked_route::direction_allowed(foot,beside,parked_at))continue;
                                const int distance=reach.distance(beside);
                                if(distance>=0&&(approach<0||distance<approach))approach=distance;
                            }
                            if(approach<0)continue;
                            auto ride=req;ride.start=parked_at;ride.goals.clear();ride.goals_open=false;
                            ride.vehicle_group=row->group;ride.vehicle_type=row->type;ride.landing_group=row->landing_group;
                            mounted.push_back({row->id,approach+1,baked_route::flood(ride)});
                        }
                    }
                    for(const auto& candidate:mounted)if(baked::vehicle_pad_accepts(*pad,candidate.id)) {
                        const int distance=candidate.reach.distance(at);
                        if(distance>=0&&(result[i]<0||distance+candidate.approach<result[i]))result[i]=distance+candidate.approach;
                    }
                }
                continue;
            }
            // Ordinary events can also require reaching another land area by
            // vehicle, just like map exits. Share each prepared landing flood.
            if(result[i]<0&&sm->world&&req.vehicle_group>0) {
                auto transfer=foot;transfer.goals=movement.goals;
                result[i]=baked_route::transition_distance(transfer,landed);
            }
            if(result[i]<0&&sm->world&&capture.with_vehicles&&(req.vehicle_group==0||game::is(game::Id::FF3))) {
                prepare_parked();auto transfer=foot;transfer.goals=movement.goals;
                result[i]=baked_route::transition_distance(transfer,parked);
            }
            if(result[i]<0&&sm->world&&capture.with_vehicles&&req.vehicle_group==0) {
                if(ship_goal)result[i]=ship_distance(at,*ship_goal,TargetKind::Infer);
            }
            continue;
        }
        const auto ship_goal=ship_goal_at(*sm,at);
        for (const auto& object : g.objects) {
            if ((object.type != baked::ObjType::GotoMap&&!baked::world_exit(object)) || object.a != target.destination || !object.contains(at.x,at.y)) continue;
            if (!snapshot.state.active(*sm,object)) continue;
            // The displayed point, eligibility distance and explicit route must
            // use the same tile, including within a multi-cell trigger footprint.
            auto movement = req;movement.allow_trap_goal=true;
            movement.goals = baked_route::goals_for(movement,at,baked_route::GoalKind::Transition).cells;
            int distance = !ship_goal||(snapshot.vehicle.valid&&(ship_goal->ids&baked_route::transportation_bit(snapshot.vehicle.id)))?
                baked_route::transition_distance(movement,reach):-1;
            if (sm->world && req.vehicle_group > 0) {
                foot.goals = baked_route::goals_for(foot,at,baked_route::GoalKind::Transition).cells;
                const int land = baked_route::transition_distance(foot,landed);
                if (land >= 0 && (distance < 0 || land < distance)) distance = land;
            }
            if (distance < 0 && sm->world && capture.with_vehicles && (req.vehicle_group == 0||game::is(game::Id::FF3)) && !movement.goals.empty()) {
                prepare_parked();
                auto transfer=foot;transfer.goals=movement.goals;
                distance=baked_route::transition_distance(transfer,parked);
            }
            if (distance >= 0 && (result[i]<0 || distance<result[i])) result[i] = distance;
        }
        if(result[i]<0&&sm->world&&capture.with_vehicles&&req.vehicle_group==0&&ship_goal)
            result[i]=ship_distance(at,*ship_goal,TargetKind::Transition);
    }
    if(game::is(game::Id::FF3)&&capture.with_vehicles&&sm->world&&
       (snapshot.map_id==3||snapshot.map_id==4)&&!planning_cancelled()) {
        for(size_t i=0;i<targets.size();++i) {
            if(planning_cancelled())return result;
            if(result[i]>=0||targets[i].kind!=TargetKind::Transition||
               (targets[i].map_id>0&&targets[i].map_id!=snapshot.map_id))continue;
            const auto at=cell_of(*sm,targets[i].position,snapshot.layer);
            // The sole surface tunnel basin is the already qualified Village
            // entrance. Underwater has only four actual script entrances;
            // each may require surfacing/flying/diving from another basin.
            if(snapshot.map_id==4&&!(at==nautilus_surface::village_cell))continue;
            MapGoal goal;goal.map_id=snapshot.map_id;goal.cell=at;
            const auto journey=nautilus_world_journey(goal);
            if(journey.found)result[i]=journey.total;
        }
    }
    if(sm->world&&!planning_cancelled()) {
        std::vector<const baked::Object*> doors;std::vector<size_t> indices;
        for(size_t i=0;i<targets.size();++i)if(result[i]<0&&targets[i].kind==TargetKind::Transition&&
            (targets[i].map_id<=0||targets[i].map_id==snapshot.map_id)) {
            const auto at=cell_of(*sm,targets[i].position,snapshot.layer);
            for(const auto& object:g.objects)if(object.type==baked::ObjType::GotoMap&&object.a==targets[i].destination&&object.contains(at.x,at.y)) {
                doors.push_back(&object);indices.push_back(i);break;
            }
        }
        if(!doors.empty()) {
            MapGoal goal;goal.map_id=snapshot.map_id;std::vector<int> distances;
            find_across(nullptr,nullptr,goal,true,true,true,true,&doors,&distances);
            if(!planning_cancelled())for(size_t i=0;i<distances.size();++i)if(distances[i]>=0)result[indices[i]]=distances[i];
        }
    }
    return result;
}
void request_reachability(bool with_vehicles, std::function<void()> ready) {
    if (!ffpr::system::is_field_navigation_ready()) return;
    ensure_quick(quick_movement(field::player_position(), field::map_handle(), field::player(), with_vehicles),
        [with_vehicles, ready = std::move(ready)]() mutable {
            auto now = quick_movement(field::player_position(), field::map_handle(), field::player(), with_vehicles);
            if (now.submap && (!g_quick_grid || !same_movement(now, g_quick_grid->movement)))
                request_reachability(with_vehicles, std::move(ready));
            else ready();
        });
}
static void request_route_impl(MapGoal goal, TargetKind kind, std::wstring name,
                   std::function<void(PathInfo)> publish, bool keep_destination, std::vector<GuideTarget> candidates,
                   std::shared_ptr<const map_state::Flags> anticipated_release = {}, std::optional<DefenseContext> defense = {}) {
    const double requested_at = frame::now();
    cancel_route_work();
    const uint64_t serial = g_request_serial;
    if (!ffpr::system::is_field_navigation_ready()) return;
    void* map = field::map_handle(); void* pl = field::player();
    const auto* submap = current_submap(map);
    if (!submap || !pl || !unity::transform(pl)) {
        PathInfo missing;
        missing.reason = missing.error = T(L"Navigation map data unavailable");
        publish(std::move(missing)); return;
    }
    auto snapshot = std::make_shared<RouteSnapshot>(anticipated_release ?
        map_state::Resolver(*anticipated_release,data::game_json("mapstate")) : map_state::Resolver());
    snapshot->anticipated_release=std::move(anticipated_release);
    snapshot->vehicle_shortcuts=capture_vehicle_shortcuts_policy();
    snapshot->requested_at = requested_at;
    snapshot->map_id = field::current_map_id(); snapshot->submap = submap;
    snapshot->position = unity::local_position(unity::transform(pl));
    snapshot->layer = submap->world ? 1 : player_layer(pl);
    record_map_arrival(snapshot->map_id,cell_of(*submap,snapshot->position,snapshot->layer));
    snapshot->vehicle = field::current_vehicle();
    snapshot->locale = modtext::lang();
    snapshot->epoch = g_route_owner.epoch(); snapshot->generation = snapshot->epoch->load();
    snapshot->goal = std::move(goal); snapshot->kind = kind; snapshot->name = std::move(name);
    snapshot->candidates = std::move(candidates);
    snapshot->defense=std::move(defense);
    snapshot->keep_destination = keep_destination;
    snapshot->previous_active = g_published_route;
    const auto* pending = keep_destination ? g_pending : g_preview_pending;
    if (pending) snapshot->pending = *pending;
    // Native pointers stay in this owner-thread capture closure, never in Snapshot/Work.
    auto entities = std::make_shared<EntityCursor>();
    entities->cache_frame_roots=game::is(game::Id::FF3)&&snapshot->map_id==1;
    g_request_pending = true;
    snapshot->native_ms = (frame::now() - requested_at) * 1000;
    auto capture = [snapshot, entities, serial, publish = std::move(publish)]() mutable {
        const double started = frame::now();
        if (serial != g_request_serial) return false;
        if (!ffpr::system::is_field_navigation_ready() || field::current_map_id() != snapshot->map_id) {
            cancel_route_work(); return false;
        }
        bool more = true;
        CaptureBudget budget;
        for (; budget.available();) {
            void* fe = nullptr;
            if (!(more = entities->next(fe))) break;
            if (!live_entity(fe) || !entity_is_present(fe) || !unity::transform(fe)) continue;
            snapshot->obstacles.push_back(capture_obstacle(fe, *snapshot));
        }
        ++snapshot->capture_slices;
        if (more) { snapshot->native_ms += (frame::now() - started) * 1000; return true; }
        capture_vehicle_names(*snapshot);
        snapshot->captured_at = frame::now();
        snapshot->native_ms += (snapshot->captured_at - started) * 1000;
        worker_job::start(g_route_owner, std::move(*snapshot), &plan_snapshot,
            [serial, publish = std::move(publish)](Planned result) mutable {
                if (serial != g_request_serial) return;
                if (!still_current(result)) {
                    g_request_pending = false;
                    if (result.map_id > 0 && result.map_id == field::current_map_id() && ffpr::system::is_field_navigation_ready())
                        request_route_impl(std::move(result.goal), result.kind, std::move(result.name), std::move(publish), result.keep_destination, std::move(result.candidates),std::move(result.anticipated_release),std::move(result.defense));
                    else if (result.map_id == 0) {
                        PathInfo failure; failure.reason = failure.error = T(L"Navigation planner unavailable");
                        publish(std::move(failure));
                    }
                    return;
                }
                const auto name = result.path.next_map_id > 0 ? resolve_map_name_by_id(result.path.next_map_id) : std::wstring();
                if (result.path.next_map_id <= 0) {
                    // Same-floor routes already have their complete worker text.
                    // Do not add another queue/frame round trip to every request.
                    commit_plan(result);
                    publish(std::move(result.path));
                    return;
                }
                struct Finish { Planned result; std::wstring name; };
                worker_job::start(g_route_owner, Finish{std::move(result), name},
                    [](Finish& finish) { resolve_plan_name(finish.result, finish.name); return std::move(finish.result); },
                    [serial, publish = std::move(publish)](Planned finished) mutable {
                        if (serial != g_request_serial) return;
                        g_request_pending = false;
                        if (!still_current(finished)) {
                            if (finished.map_id == field::current_map_id() && ffpr::system::is_field_navigation_ready())
                                request_route_impl(std::move(finished.goal), finished.kind, std::move(finished.name), std::move(publish), finished.keep_destination, std::move(finished.candidates),std::move(finished.anticipated_release),std::move(finished.defense));
                            return;
                        }
                        commit_plan(finished);
                        publish(std::move(finished.path));
                    }, background::Lane::Navigation);
            }, background::Lane::Navigation);
        return false;
    };
    // Take the first bounded slice on this input. Small/warm captures need no
    // unconditional extra game frame before their owned worker job can start.
    if (capture()) frame::every_frame(std::move(capture), "BakedNav.Capture", INT_MAX);
}
void request_route(MapGoal goal, TargetKind kind, std::wstring name,
                   std::function<void(PathInfo)> publish, bool keep_destination) {
    request_route_impl(std::move(goal),kind,std::move(name),std::move(publish),keep_destination,{});
}
void request_guidance(std::vector<GuideTarget> candidates,std::function<void(PathInfo)> publish,const map_state::Flags* anticipated_release,const DefenseContext* defense) {
    if(candidates.empty()||candidates.size()>32) {
        PathInfo failure;failure.reason=T(L"No path to the next step");publish(std::move(failure));return;
    }
    const auto first=candidates.front();
    request_route_impl(first.goal,first.kind,L"",std::move(publish),false,std::move(candidates),
        anticipated_release?std::make_shared<const map_state::Flags>(*anticipated_release):nullptr,
        defense?std::optional<DefenseContext>(*defense):std::nullopt);
}

} // namespace field::baked_nav
