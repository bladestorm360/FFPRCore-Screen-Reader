# Native NPC movement and cross-floor interaction

Current scope, ordered milestones, effort and Closed boundaries are maintained
in the [single audit record](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
This topic explains the implemented producer/consumer contracts and evidence.
Implementation/offline acceptance does not mean gameplay or audible confirmation.

## Player behavior and the22 audited contexts

Current-floor movement uses the captured actual NPC position and native identity.
Moving blockers can require waiting; a completed or animation-only instruction
controller is a manual interaction stop. A future-floor destination can use its
source-proven native patrol positions to choose the first exit. Entering that
floor requires a fresh native actor; metadata cannot synthesize a missing actor.
Controls, labels, speech identity, ordinary route retention and settings are unchanged.

| Source context | Count | Actual approach and evidence |
| --- | ---: | --- |
| II Cyclone7F Emperor4 | 1 | Ground stage, Bottom player. Native flags74=1/75=0 enable group ev_e_0081. Three real sc_e_0082 triggers5/6/7 have actual production routes; the script aligns the player and starts encounter469. Do not manufacture a cross-layer talk approach. |
| V Walse Dungeon5 | 1 | Initializer assigns route1 on every Main branch; settled movement endpoint16,10 replaces an inaccessible staging home only in future-floor proof. |
| V Istory22/103 | 2 | Actual sheep-kick player track ends33,19. Existing reviewed prerequisite edge reaches both objects; earlier35,19 alignment is not the kick landing. See [Istory sheep entry](ISTORY_SHEEP_ENTRY_20261029.md). |
| V Map_30381_7 actors2/16 | 2 | sc_e_0436 ends on shore12,32 after trigger12,35. Four actual planner before failures pass after the reviewed within-map landing. See [shore landing](SCRIPTED_SHORE_LANDING_20261029.md). |
| VI South Figaro Map_20051_3 actors12/14/15 | 3 | One Area2 and two Route3 natives, exact four positions each on Upper. Real stair masks and native region frontiers now retain cross-floor routes. |
| VI Map_20051_4 actor12 | 1 | Projected actual actors produce15-step named-blocker approach, without a made-up counter or layer. |
| VI Map_20090 actor24 | 1 | Chocobo-mounted item-shop actor; unconditional route95. Last movement14,10 is followed by a cue and flag write. This is an interactive actor. |
| VI Imperial Camp21 | 1 | Four exact native mask SysCalls affect rendering only. Projecting them as visual no-ops retains the guaranteed Hide21 prerequisite; see [mask projection](NPC_MASK_PROJECTION_20261029.md). |
| VI Zozo Map_30511_7 actors7–13 | 7 | Seven native looping routes305111–305117 cross real Bottom/Ground layers. A valid route can first visit floor389 before390; replay each real entrance receipt. |
| VI South Figaro Cave16 | 1 | Route306411 is assigned only with ScenarioFlag2/76 nonzero; do not use its moving pose before that condition. |
| VI Mt. Zozo2 | 1 | Route306711 requires ScenarioFlag2/107=1 and108=0. All-zero/all-one samples miss this legitimate active state. |
| VI Darill Map_40005 actor2 | 1 | Actual same-cell interaction succeeds in zero steps. |

The22-case survey was a list of uncertainties, not22 established game blockers.
Current geometry, native assignment/default movement, real prerequisite landings,
state selection, rendering-only effects and scene ownership now explain them.
Samples do not certify a complete reachable global story trajectory.

## Live actor capture and lifetime

Source: baked_nav.cpp instruction_will_move, capture_obstacle, selected_live_npc,
live_interaction; npc_route.h preserves the existing blocker policy. Worker
snapshots own cells, flags, IDs, raw identity and booleans, never native pointers.
Native actor/controller fields are copied only on their owner through the ready
map/controller registry. No GetTargetController, script/controller invocation,
whole-scene search, hook, timer, native wait or pointer transfer was added.

III/V/VI instruction-controller constructors save the original MoveType and
write102. Destruction restores it. I/II/IV constructors do not write102. Their
IsMovingType getter still returns true; the earlier contrary inference was wrong.
Current complete source inventories have AssignMoveRoute in V/VI and none in
I/II/III/IV; III separately supports the actual newer native102 controller signal.

| Game | Constructor | Getter | Destroy | NextInstruction / Update |
| --- | --- | --- | --- | --- |
| I | 961000 | 2680C0 | 960B50 | Older controller contract |
| II | 94A6D0 | 2680C0 | 94A220 | Older controller contract |
| III | 719390 | 26C0C0 | 7190A0 | 718FC0 /7191A0 |
| IV | 8CC4D0 | 26A0C0 | 8CC020 | Older controller contract |
| V | 7C3E30 | 2EAD10 | 7C3B40 | 7C3A60 /7C3C40 |
| VI | 5F2E80 | 32BCB0 | 5F2AB0 | 5F29D0 /5F2BB0 |

Completion clears currentInstruction while retaining102. Therefore that marker
alone cannot mean the NPC will move. For ready102 actors, copy whether remaining
MoveTo/MoveLerp instructions exist; inspect a full sequence only when it loops.
Completed, animation-only, unavailable, malformed and overloaded cases stay
manual. Ordinary Area2/Route3 with positive MoveStep keep their native policy.

One4096-entry budget covers every controller/instruction across the complete
capture; per-list caps are1024 controllers/256 instructions. A64-actor budget
also bounds empty/malformed registry lookups. Known largest unique script-assigned
actor sets are III0/V3/VI11. The ready registry is reached through manager10;
controller fields are actor18, instructionList78, index84, loop8C and current90
in the newer matching natives, resolved by name at boot. No controller method
runs on capture. Existing0.5ms capture slices/native_ms accounting remain.
Capture host tests completion, remaining moves, animation loops, restoration,
wrong indices, oversized inputs and whole-episode overload.1000 eight-actor
4096-entry episodes peaked1.117ms; the final64-actor host run peaked0.668ms.
These are offline fake-native observations, not game-frame/audio measurements.

## Future-floor native positions

Source: tools/re/npc_remote_routes.py, tools/npc_route_positions.json,
tools/bake_map_state.py, map_state.h and map_state_program.cpp route_positions.
The manifest contains hashes, native actor fingerprints and coordinates only.
Original scripts, route JSONs and native bodies remain outside every Git tree
at ../tools/npc_geometry_context_20261029/. Revalidate those originals when adding
facts; do not copy game source/fixtures into this repository.

Positive route IDs come from exact reviewed AssignMoveRoute instructions. One
opcode12 per native PC preserves all existing branch/call offsets. Assignments
common to completed branches live separately from visibility/group certainty;
uncertain patrols do not hide an otherwise known actor. Unknown assignments grant
no new position. Replacement assignments clear unsupported old route facts.

The eleven assigned actors contribute67 per-actor positions. One-shot routes use
only their final settled movement endpoint. Looping routes retain their exact
MoveTo/MoveLerp waypoints and real layers. Native V/VI AssignMoveRoute A3CB60/
B64D60 passes entity, route and loop through IMapAccessor26; FieldController
28B870/316550 loads the actual route asset and creates that actor's controller.
MoveTo327970/3A6330 uses range-cell conversion; MoveLerp326E10/3A57B0 changes the
real layer through326C40/3A55E0 and layer+9.

Route0 metadata represents only the three source-pinned VI default Area2/Route3
actors. It requires a known initializer, active exact actor and existing wandering
flag; a positive assigned route takes precedence. Native PropertyMoveAreaB31EF0
uses MapObjectProperty.posD4/original dimensionsE0/E4; WithinRange5E9B00 checks
its rectangle. ParseMapObjectGroupDataCDDB80 centers it, yielding exactly four
cell centers. PolylineB32C10 adds flipped local Y offsets to its linked line
entity transform; manager5EC7D0 binds the line by link/object ID. Intermediate
16-unit positions are native data, not invented geometry. These three actors
add12 positions, for14 actors/79 per-actor positions altogether.

Runtime lookup requires ID, raw name, group and authored home, with at most64
metadata rows and32 positions per actor. Coordinates/layers are bounded before
use. The existing8192-instruction/64-branch initializer bounds remain. Unknown,
hidden, stationary, mismatched or malformed defaults grant no future placement.

route_to_goal uses those positions only for remote Interact graph goals, preserving
native talk masks and active relay geometry. No per-position graph search runs.
The existing400-state graph and one later-floor-NPC retry remain cancellable.
Derived approach cells stay request-local; Pending retains the original identity/
home and normal authored footprint. Changed state must prove its assignment again.
On the destination floor selected_live_npc rebinds to the fresh actual position;
a missing native actor fails, regardless of metadata. Neither layer nor collision
is relaxed. VI relay IsInRangeE08E40 compares exact Z; no all-layer counter bypass
was added. Wide-counter behavior remains at its accepted boundary.

## Native diagonal stair exits

Map_20051_1 Upper10,13 mask159 enters11,14 mask31 Southeast. Its four cardinal
neighbors are0. Local A* accepted that step but distance_to_object checked only
cardinal predecessors, so the cross-floor graph never entered the room.

The minimal terminal-distance correction tests four extra indoor predecessors
through existing edge_allowed with the exact exit goal. Source and destination
native direction bits, layer changes, solid blockers and restricted directions
still apply. Zero collision does not create a diagonal; world movement retains
four directions. A request copy is made only for a reachable diagonal predecessor;
there is no new search/native call/hook or grid write. The temporary offline
trace used for diagnosis was removed. Tests compare the real one-step local/
graph route and reject solid-blocked or direction-restricted variants.

## Verification, cost and extension limits

Source and exact deltas are in build_check/npc_future_20261029/. Eleven assigned
actors reproduce11 failures with retained original state data, and pass all11
first legs, actual intermediate-floor replays and native-floor continuations
with the correction. Three area/line actors reproduce3 selected-route failures
after the stair is usable, then pass3/3 with their exact default facts. Hidden,
unknown, stationary, wrong identity/home, missing-current-actor, malformed,
cancellation and changed-state retention controls pass.

Current-pose tests cover all11 route assets/78 original native waypoints, with51
entry-connected approaches. Bedroom/Darill actual planner and the three active
mixed-state FFII scene routes are included. The inherited route host incorrectly
left hidden passages disabled for every game; it now uses immutable real III–VI
feature rows. Original before/after proofs were rerun with production flags.

Five state records change13 previous no-op PCs and add11 assigned metadata rows;
one further VI record adds three defaults without changing any prior field or
program. All existing program offsets and all six bakes are preserved. These
are mod-owned derived facts, not original source. Canonical checks run the future
and region consumer hosts plus the original-asset manifest check. All-six broad
planner/initializer checks and final release checks remain in the audit record.

After valid native stairs expose additional routes, collector plus first-leg
worker compute totals43.953ms across eleven requests, peak6.250ms, including
normal extra graph work. Earlier smaller runs used fewer eligible graph edges
and are not an efficiency claim. Native capture is unchanged. Actual combined
frame cost, backlog and audible latency remain unmeasured; the cumulative25ms
application-lag requirement is not certified by worker placement or these numbers.
No game was launched, controlled or probed.

Safe extension: pin the complete initializer and actual actor/movement owner,
verify native coordinate/layer/loop semantics, retain physical approaches, and
exercise collector→first leg→every real entrance→current native actor. A rectangle,
all-zero/all-one flags or an unreachable authored home alone is not sufficient
proof. New work must remain within the active scope recorded by the audit ledger.
