# Vehicle goals, reasons and mounted filtering, October 22

Later October29 human follow-up: [ordinary-flight preference](INVINCIBLE_PREFERENCE_20261029.md)
minimizes required Invincible mountain actions before distance, including landing
selection. The human confirmed Doga's Manor needs Nautilus through the winds;
that restriction is correct and unchanged. Installed FFIII0a511191 passed canonical/smoke/package/scan checks.

Current October29: [Invincible mountain crossing](INVINCIBLE_MOUNTAINS_20261029.md)
source/delivery Closed in FFIII ad0d80c1. Worker geometry matches native model10/11
cardinal1/2-mountain action, physical2/3-tile costs and real far-side legality.
Owned parked-provider/category/full-route checks publish Falgabard in Map Exits,
manual crossing/facing boundaries stop autowalk, then legal landing and entrance.
No new native capture/hook/input or bake terrain. Nautilus and other games retain
their capability boundary; original page/surface/vehicle-name/Duster fixes remain.

Current audit source after b2cacd6f delivery: [vehicle applicability ledger](MAP_AUDIT_FOLLOWUPS_20261028.md)
records a valid native Player1 skipping parked aircraft and unfiltered synthetic
entries depending on flight reachability. Actual provider/category host3→0;
existing Saronia/continent checks pass. New require_reachable policy preserves
strict vehicle-aware proof and one nearest physical unfiltered representative
with unknown distance. This later source is not yet deployed. Per-leg target-ID
and vehicle-hidden actor capture remain open, including automatic canoe order.

October28 explicit human follow-up: [Nautilus dive/underwater](NAUTILUS_ACCESS_20261028.md)
adds real native Can dive guidance and reviewed submarine-entry Map Exits. Ordinary
world doors now use the existing legal landing/final walking planner for every
manual vehicle, including Nautilus8/9; only verified mounted entry/pad/scene rules
retain aboard arrival. Doga's Manor and northern Saronia landing assertions pass.
Current map disables accessibility/story-gate pruning while preserving real
object presence and explicit physical grouping; selected directions still report
actual blockers. All-unlocked-vehicle capture/per-leg applicability remains an
open audit checkpoint, not declared fixed by these entry/filter corrections.

October28 human follow-up: [nearby destinations](LOCAL_GUIDANCE_20261028.md)
with a legal ground route retain walking within15-tile proximity; required
crossings and already-boarded movement keep their existing behavior.

[October25 chocobo/Owen follow-up](CHOCOBO_OWEN_20261025.md) preserves native
partial collision faces, restores Owen's adjacent on-foot entrance/filter, and
keeps chocobo riding to the closest equally short legal dismount approach.
Examined entrances sharing native exit tiles use their real interaction/facing
positions. Supported Boko/Magitek entry remains mounted; NPC/object interaction
and actual landing/Toad conditions remain native. Final combined build passes
all checks and is installed/hash-verified in all six games.

[October25 terminal-entry correction](TRANSITION_ROUTES_20261025.md) prevents
walking through a town entrance while approaching a parked vehicle. Selected
exits and mounted flight retain their native rules. Shared checks pass and the
exact scanned combined build is installed/hash-verified in all six games.

[October25 loading-context correction](LANDING_LOAD_20261025.md) keeps native
Show/Switch guidance, but construction/fade observations only seed the current
state. Shared readiness owns speech; initialization cannot replay at control return.

October25 [landing guidance producer follow-up](FF3_FEEDBACK_20261025.md)
observes native ShowLandingGuide as well as SwitchLandable in all six games.
A visible native guide owns Can land; stale movement-speech state cannot veto
it. This changes guidance only, preserving native terrain and route rules.

October24 [Leviathan sea-scene entry](LEVIATHAN_20261024.md) permits its reviewed
Ship entry without a fictional landing; ordinary native town rules remain.

## October23 live Ship follow-up

Later October23 deployment: the user lifted the FFII hold and requested deployment.
The [Castle Fynn/NPC build](CASTLE_FYNN_20261023.md) includes this correction and is
now hash-verified in all six games, including FFII. The earlier hold below is a
historical deployment record. Gameplay retest remains pending.

Implementation complete; canonical offline and isolated smoke checks pass.
FF1/III/IV/V/VI are installed and hash-verified. FFII deployment is explicitly
held by the user; its existing DLL/SDL remain unchanged. Gameplay is unconfirmed.

The latest FFII log (session1103039828, V0.9.5) records Canoe travel on
world map1, then Ship list failures at1103299.312 and1103300.921 and a
No path direction result at1103301.671. The last successful captured origin is
(83,90). The failed job takes0.021ms and has no search-result diagnostic.
The log does not record the rejected Ship coordinates, so its exact live target
cannot be replayed from this log. The old cell_of retained out-of-range local
transform cells even though the world graph already wraps. A copied live Ship
position can therefore produce empty goal cells before search and the reported
generic browse/direction failure. The new actual-source regression reproduces
that rejection, then verifies physically identical world copies reach the same
Ship using the legal Canoe/boarding route. Gameplay confirmation remains pending.

### Implementation and native evidence

field/baked_nav.cpp cell_of retains the native float tile conversion, validates
finite/integer-safe observations, then wraps world x/y by that bake's width and
height. This shared boundary serves cell_at, player origin, native obstacle and
parked-vehicle snapshots, route reuse and beacon tracking. Indoor out-of-bounds
coordinates remain out of bounds; oversized/NaN/infinite observations produce an
invalid cell instead of an unsafe cast or plausible destination. No native actor,
map grid, story flag or game-owned route state is changed.

route_to_goal now logs the copied target/map/kind/size/current transport when
goal discovery rejects before search. It runs on the existing Navigation worker,
including bounded asynchronous diagnostics, and adds no speech. The existing
boarding_route still obtains the exact Ship cell and a legal adjacent approach
using owned auto-use transport groups, then the WalkOn phase. Browse and explicit
directions consume the same plan; this is not an unconditional reachable label.
Live barriers, directional entry, hazards, traps, native landing and actual
ownership checks remain in force. There are no new controls, settings, output
keys or localization changes.

Matching FFII GameAssembly.dll static evidence confirms MapUtility's
CorrectionMapAreaRangeWidth RVA AA0B00 and CorrectionMapAreaRangeHeight AA0760
correct a position outside the map rectangle by its width/height. Native transport
visual offsets use FieldCharaEntity.SetOffsetUnit; these offsets do not justify
inventing a new route or vehicle location. Source-bearing disassembly and its
DLL/dump SHA256 provenance stay outside Git at
../tools/ship_route_20261023/native.txt and provenance.json. The bounded original
extractor/receipts are in ignored build_check/ship_route_20261023/.

Capture cadence, native-owner pointer checks and worker queue/cancellation remain
unchanged. Only copied coordinates cross the existing RouteSnapshot boundary.
Conversion adds a constant finite/range test and two world-only modulus operations;
it introduces no scan, loop, cache or native callback wait. Existing actual-source
worker affinity and cancellation tests remain mandatory. The placement in the
existing large planner file retains its shared coordinate boundary; splitting the
surrounding pipeline is unnecessary for this small correction.

### Verification and remaining player check

The scoped fixture uses real FFII terrain, a Canoe-only bank, a Ship-only water
tile and current native Canoe state. It checks five positive/negative world-copy
offsets, identical full directions and browse action, exact boat/WalkOn endpoint,
the old unwrapped-goal rejection, unchanged indoor bounds and invalid observations.
All-six parked-vehicle cases also check both world axes. Existing 103 boarded
world/transport cases, 582 exit candidates, gate/filter/trap/cancellation and
worker-affinity coverage run through the canonical build. These are owned offline
fixtures, not a reproduction of the missing live Ship coordinate or audible
latency. The initial expanded fixture exceeded the host stack (C00000FD); copied
state now lives on the heap in a separate fixture function. No stack limit was
increased. Final canonical build/smoke and deployment receipts follow here.

Canonical build passes all regressions, 7,182 native names with zero missing,
125 used/333 defined wiring names with zero undefined and 47 HID exports.
The smoke host forwards all 47 exports and exits normally. Five world-copy
Canoe-to-Ship cases, all-six parked world-copy checks and the retained 103 boarded
transport/582 exit cases pass. This run's broader fixture maximum route/filter
compute is 30.515/14.452ms; six-category preparation maximum is 2.881ms.
These are offline computation observations, not a comparable latency baseline,
native callback measurement or request-to-audible certification. The existing
request/capture/queue/compute/submit and data-age diagnostics remain available.

The attempted all-six deployment correctly refused the running FFII process;
the user then said “hold deploy to ffii for now. other games are fine”. The
other five DLLs match the tested build; FFII DLL/SDL bytes and timestamps still
match their pre-deployment recovery. All 144 protected Prism/OpenAL/settings/
waypoint files retain their bytes/timestamps across all six games. Recovery,
before/after manifests, reports and smoke log are in
build_check/ship_route_20261023/. The built DLL is 10,557,952 bytes, SHA256
ce462c5f92002e182661a9e899b7836c80856ebbc21eb252592d7cda12a708a0.

After the user lifts the FFII deployment hold and the fix is installed, when
convenient at the reported location, select Ship and request directions.
Expect usable directions and the boarding action; if already beside it, expect
the boarding direction. No replay or additional capture is required in advance.
No game was launched, controlled or probed. The published V0.9.5 asset is preserved.

## October22 correction and previous verification

Implementation complete; actual-source offline route checks pass. Gameplay retest
is pending, including the user's exact location, which was not supplied. Shared
behavior applies across all six games; native ownership, terrain and story gates
remain authoritative. [Current combined verification](../FIXES_2026_10_22.md).

## Ship/Canoe report and correction

The user saw Ship (Requires vehicle) while explicit directions said Requires Canoe,
even with the Canoe owned. Two independent defects were found in `field/baked_nav.cpp`:

- Browse RouteSnapshot copied ownership/groups and parked vehicles but omitted
  localized vehicle_names. Both request_browse_route and request_route now call
  native-owner capture_vehicle_names. Workers receive the same owned strings and
  vehicle records with game pointers removed. Genuine requirements use the same
  specific name in both descriptions and directions.
- The general transfer planner tried to ride and land after reaching a selected
  parked boat. Its water cell has no walking area for that last leg, falsely
  rejecting a valid approach with the owned Canoe. boarding_route now recognizes
  that exact selected Ship/submarine cell. It walks to a legal adjacent approach
  with owned auto-use groups, checks the native directional edge and blockers,
  then ends at the boat via the existing WalkOn boarding phase. It does not invent
  a walkable water tile or require a fictional disembarkation afterward.

A usable indirect route reports its specific action, such as board the Ship, rather
than claiming a missing vehicle. Trap warnings retain priority. Genuinely blocked
routes retain specific missing requirements; possession alone does not defeat map,
access, landing or scenario gates.

## Current map with vehicles hides mounted destinations, October 22

User report: while riding a Chocobo, Current map with vehicles hid destinations
that Current map showed; an earlier similar Ship issue was recalled. The exact
game/map was not separately specified.

Filter queries use TargetKind::Infer. On a riding grid, native transition inference
could yield an empty goal footprint even though the mount could cross the terrain;
explicit Transition already used the walking footprint. goals_for now retries an
empty inferred world goal using that footprint, with vehicle/type/landing values
cleared, then the existing mounted/landing flood decides actual reachability.
This changes goal discovery, not collision/vehicle ownership or transition rules.

## Bounds, lifetime and evidence

EntityCursor capture stays on the native owner under existing CaptureBudget slices.
Owned immutable snapshots go to the Navigation lane; no Unity/IL2CPP pointer is
usable by the worker. Route/browse owners and cancellation epochs retire obsolete
requests; publication validates the current target/map/pose/vehicle. Existing grid-
bounded flood arrays, iteration cancellation and overload rules are unchanged.
See [threading](THREADING.md), [autowalk](AUTOWALK.md) and [map state](MAP_STATE.md).

route_worker_check now constructs a real FF2 Canoe-only river approach to a Ship-only
water destination, verifies successful WalkOn at the exact boat cell and a matching
specific browse action. All-six mounted scenarios compare inferred and explicit
transition footprints. The broader retained fixture covers103 boarded world/transport
cases (85 have no eligible representative start/landing target),582 world exits and
parked-vehicle cancellation; these counts do not certify all gameplay locations.
Final timing observations and player retest are in the fix register/TESTING. No game
was launched, controlled or probed. Extend these paths through the same snapshot and
planner for both output consumers; do not add separate string-only requirement rules.

## October23 second live-identity correction

The earlier coordinate-only fix was insufficient: the latest log's selected Ship
had NPC collision layer-1, and a sprite origin can differ from its transportation
model. The [current combined follow-up](WORLD_EXIT_FOLLOWUP_20261023.md) binds
vehicle targets to that same captured model used by distant boarding routes. It
preserves transport identity in browse, directions, autowalk and beacon requests.
Known parked water tiles may supply a goal, with actual shore/boarding proof still
required. Missing models never become arbitrary coordinates. Current build is
deployed and hash-verified in all six games; gameplay confirmation remains pending.


## October24 shorter world-object routes

[Current vehicle comparison contract](OBJECT_VEHICLE_SHORTCUTS_20261024.md)
supersedes the old rule of trying parked vehicles only after walking fails.
Vehicle/map filter modes compare legal candidates for Objects and all ordinary
world destinations; current floor retains walking when available. Current mounts
land before ground interactions. Native vehicle-action pads retain their separate
contract. Exact target/model identity, native landing, cancellation and manual
special-flight controls remain unchanged.


## October27 movement wording: scope ledger

Status: source checkpoint complete, Closed; final combined delivery pending.

User-scoped wording correction at `field/baked_nav.cpp::plan_browse_snapshot`: the pre-action movement phrase now comes from the captured current `RouteSnapshot.vehicle`, not the destination craft. Airships type3/8 say Fly; ships/submarines type2/6 say Sail; mounts and foot say Walk. Native type7 is game-specific: FFIV Hovercraft is a watercraft/Sail; FFV Wind Drake is a mount/Walk. Auto Content type5 uses Sail for I-III Canoe ID2; FFII Snowcraft ID5 remains Walk. Parked-vehicle boarding approaches remain Walk because the snapshot begins on foot. No geometry, timing, native observation, route phase or action changed.

Two mod-owned phrase keys, `Fly {0}, then {1}` and `Sail {0}, then {1}`, have all12 supported locale values with each placeholder once. Auto/manual language policy is unchanged; `tools/modtext_vehicle_wording.py` maintains the entries. Actual production browse assertions pass for airborne Enterprise before water landing and FFII ridden-Canoe distant-Ship browse before boarding. An earlier Walk expectation failed as expected and was updated. Full navigation and landing follow-ups pass. Canonical evidence:8287 names/0 missing;142 used/349 defined/0 undefined; `route_worker`0 failures; 47-export smoke pass. Earlier intermittent standalone `route_worker` 0xC00000FD is retained as a known fixture issue, not a universal stability claim. DLL11,037,696 bytes, SHA256 `9a4a87e3082b709a78a407035abc0f9b9e2d5f4c82d78e94fe03d5140bd6e8ab`. Localization, navigation, Enterprise and worker reports are in `build_check/vehicle_wording_20261027/`.

Fixed formatting is on the existing worker path; no additional hook, capture, native read or queue was added. Reported route-worker samples are offline computation only; native/frame/audio/request-to-audible cost remains unmeasured against the cumulative25ms application-lag blocker. October26 installed baseline/history above remains distinct. October27 candidate is uninstalled and gameplay-unconfirmed; no game launch/control/probe occurred. Wording source is Closed. Continue the explicitly authorized outward and return floating-continent transition work, then Temple of Water investigation; Libra/Study display audit remains separately queued. See [WORLD_EVENTS](WORLD_EVENTS_20261026.md) for the current scope ledger.
## November 13 FFIV hovercraft pickup and lowering

The FFIV log gives the attached hovercraft a bearing of 90.5 steps west
and routes to (128,128) from (216,117). After the native lowering sequence
it routes to the newly parked (214,118). The matching FFIV binary parents
the hovercraft to the Enterprise on pickup and restores its map parent
on lowering. The cargo transform's local origin was mistaken for a map
position. While attached, the entity row follows the carrier and says
“Carried by the Enterprise.” The route worker and beacon receive no
boardable target for the attached craft. After lowering, the real model
becomes a target at its live map-local position. FFIV's normal return to
field control requests a new snapshot on the relevant maps after Gimmick.

Native `Serial.FF4.Map.TransportationEvent.IsLandingEvent` at RVA 3274A0
has one matching body. For Enterprise-with-hook ID 10 carrying hovercraft
ID 2, it writes the result of the native hovercraft lowering check to the
boolean output. This is also the check the native confirmation path uses.
The mod forwards the original once, observes that result and announces
“Can lower hovercraft” on a new available edge during ready field control.
Ordinary airship landing keeps its own guide. The carry bit comes from the
ready SaveMapManager/SaveMapData instance, with no singleton construction,
native delegate invocation, or save writes by the mod. The two strings are
available in all 12 game languages.

Native trace and assembly provenance are in the external
`tools/ff4_underworld_20261112` archive. Offline checks cover native result
forwarding, the distinct cue, fade suppression, live attached/parked
positions and readiness-gated same-map refresh. In-game confirmation is
pending. With the hovercraft attached, move the Enterprise until the
lowering cue sounds. Lower it using the normal game control, then select
Hovercraft and request directions to its new position.

## November 15 FFIV Falcon and hovercraft routes

**Offline verified and installed.** **Current map with vehicles** includes
destinations reachable by relocating the hovercraft. Browse and directions
share the reachability rules and retain the selected destination through the
handoffs. The installed game's carry action belongs to the Enterprise with the hook.

1. Fly the Enterprise to the hovercraft and pick it up if it is parked elsewhere.
2. Carry it to a usable release tile and lower it onto the map. The party
   remains aboard the Enterprise after lowering.
3. Fly to a legal Enterprise landing with a walkable approach to the released
   hovercraft. Land and park the Enterprise separately, then walk to and board
   the hovercraft from the ground.
4. Use the hovercraft to reach the selected destination.

Later verification should cover an already carried hovercraft, a parked
hovercraft that needs relocation, and a release site with no usable Falcon
landing and walking approach. Routes must respect the carry unlock, native
release, landing and boarding rules, hovercraft terrain and any story gate.

The current FFIV Steam log records Falcon travel and filter selection at
262240.390–262260.375. The owner supplied the lowering and separate
boarding sequence. The November 13 Enterprise pickup and lowering evidence
above covers the related Enterprise behavior.

The installed transportation master binds the two Falcon variants to IDs 8
and 9 and the hooked Enterprise to ID 10. FFIV `IsCarryingHoverShip` at
RVA 327450, `IsLandingEvent` at 3274A0, `OnMoveFinished` at 327BF0, and
`OnStartLanding` at 327D50 all require ID 10 for cargo behavior. Their
native checks refer to the owned hovercraft model, ID 2. The lowering
branch uses `CanLandToWorldMap(int)` at 275C40, which checks other vehicle
occupancy, the appropriate collision rule, and the cargo's own landing
attribute table. The planner still needs to prove both that release and a
separate parking position with a legal foot approach. The trace is in the
external November 13 hovercraft archive cited above.

The Surface World planner uses the copied native cargo receipt and actual
vehicle poses. It checks the carrier's approach to the parked hovercraft,
hovercraft release terrain, a distinct carrier parking cell, the ground
approach from that cell, and the hovercraft's ride and final landing. The
parking search also supports release sites that require walking from a
landable area. A previously released hovercraft is preferred when it can
complete the journey. The reachable list evaluates those ground approaches
once per owned snapshot and the directions worker builds the exact legs.

Directions stop at a pickup or lowering action and retain the selected goal.
After the native action, a fresh snapshot supplies the actual flight mode and
cargo pose. A directions request then gives the next leg. The full plan
includes the separate Enterprise landing and the subsequent ground boarding
action. The game supplies the pickup and release controls. Both action
instructions are localized in all 12 languages. Autowalk uses the published
leg and stops at the manual action.

The production replay uses the surface entrance at (224,123). It verifies
the with-vehicles listing, complete attached and parked-cargo journeys,
starting on foot, retained destination through manual cargo actions,
separate parking, and walking to board the craft after release. It also
checks unavailable parking and the Falcon's independent vehicle identity.
The attached journey took 32 ms in the resource-capped offline fixture on
the navigation worker. Gameplay and end-to-end audible timing remain unmeasured.
[Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-ffiv-hovercraft-journeys).

## November 15 FFIV airborne vehicle interaction loses selection

While flying the Falcon on the Overworld, the owner reported
that the selected Vehicle interaction event sometimes loses focus during
repeated directions requests. Using the destination select keys restores
the event. The owner noticed this around the “Planned landing is … in a
straight line” fallback. That association and behavior on other airships
still need verification.

The FFIV Steam log `ffpr_screen_reader.1.log` gives directions for the
selected Vehicle interaction event, followed by “30.5 steps Northwest in
a straight line” at 262792.968. At 262793.953 and 262794.656 it says “No
entity selected.” The next destination selection at 262795.328 restores the
event, and directions resume at 262795.687. The specific “Planned landing”
wording appears in earlier Castle Baron directions at 262215.906 and
262217.390. That route resumes detailed directions at 262218.359 without
a new selection.

Later acceptance is to retain the same vehicle interaction through repeated
directions requests while flying and through any landing fallback, so the
player can continue without reselecting it. Preserve explicit selection
changes and the handling of genuinely unavailable targets. See the existing
[target retention contract](TARGET_RETENTION_20261024.md#selection-ownership).

The surface map has distinct Falcon descent tiles at (112,198) and (112,200).
Native IDs 1072 and 1075 share script 357 and the same pad name. Before the
fix, regrouping by that name kept the first row in the new distance order.
That could drop the still-available selected tile before selection was
restored. The group now prioritizes a continued objective or exact selection,
including a recreated native row with the same identity and anchor. Unselected
groups keep their normal order. An unavailable tile remains unselected.

The focused filter replay exercises both logged coordinates. It verifies
selection through movement and native row recreation, and verifies that an
unavailable objective does not transfer selection. Installation is verified
in all six games. Gameplay confirmation remains pending.
[Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-vehicle-destination-focus-and-ffiv-markers).
