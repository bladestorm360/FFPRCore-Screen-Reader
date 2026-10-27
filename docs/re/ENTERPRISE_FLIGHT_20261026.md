# Enterprise unlocked flight routing, October 26

Status: complete, Closed; implemented, Sol-reviewed, offline verified and
installed/hash-verified all six. Combined delivery acceptance is recorded below.
Gameplay/native/frame/audible confirmations remain separate; public1.0 preserved.
All five scoped source checkpoints and local delivery are Closed.

## Scope ledger and finish line

For FFIII's upgraded Enterprise, selected routes and world-exit reachability must model
the actually unlocked ship/flight conversions and legal landings. Keep story state,
world wrapping, movement/collision rules, current native movement ownership and existing
settings. The planner observes vehicle identity; it never writes flags, scripts, ownership
or vehicle state. Existing safe autowalk may execute only the planned native actions.

Closed-system register: this Enterprise source follow-up is Closed. Previously Closed systems
remain untouched. The scope is one Enterprise movement/routing fix across existing
reachability, direction and phase consumers. No game launch/control/runtime probe,
deploy, install, tool installation, publication, reward-dialogue inspection or unrelated
refactor is included. The bounded coding/offline checkpoint was estimated at 45–60 minutes and completed
within that bound, including the demonstrated same-cell integration correction. The initial ship-5 hypothesis was corrected after locating the native upgraded identity;
the conversion contract is now settled. Initial research/first-replay estimate was
30–45 minutes; the completed baseline and retained receipts distinguish this from the
remaining coding checkpoint.

## Native contract and evidence

Native evidence is outside Git under `../tools/enterprise_flight_20261026/ff3`;
the factual contract is `build_check/enterprise_flight_20261026/native_contract.json`.
FFIII transport masters and reviewed native methods establish:

- `SystemCall.Current.RemodelEnterprise` (RVA `0x91F670`) changes original ship ID 5
to upgraded ship 13. Original ship 5 has no takeoff action and does not gain flight.
- `TransfarShip` (`0x920760`) converts ship 13 to airship 6; `TransfarAirShip`
(`0x920660`) converts airship 6 back to ship 13. A later observed equivalent pair is
ship 16 / airship 15. No other upgrade pairs are inferred.
- Ship 13/16 have type 2, movement group 5, landing group 7. Airship 6/15 have type 3,
movement group 6, landing group 5. Landing group 5 permits water attributes 3/16;
group 7 permits the authored shore types.
- Reviewed ship `UpScript` 851 and airship `EndLanding` 850 are the conversion scripts.
`MapUtility.BranchTransportationAction` (RVA `0xC53630`) selects the applicable native
action. `WorldActionCommand` (`0x297B40`) immediately invokes that sole action, so this
flow has no two-choice selector. `CanLandToWorldMap` (`0x278900`) owns duplicate,
terrain and collision validation. The planner uses existing native Confirm behavior,
then waits for the expected destination vehicle ID and ready movement controller; it
never invokes the script or writes a flag/vehicle field directly. No reward dialogue was
examined.

## Production data and route flow

`baked_route::Request.transportation_id` is the observed scalar identity. The pure
`enterprise_pair` / `enterprise_route` helper in `src/core/field/baked_router.h/.cpp`
accepts only IDs 6, 13, 15 and 16 and requires a world bake plus valid start. It chooses
only the corresponding observed pair. Its ordered states are ship-before-flight,
flight, ship-after-flight and foot. Neighbor expansion reuses the existing baked router
rules for wraps, directional edges, terminals, blockers, traps and hazards. Conversion
is a zero-tile state transition only where the native landing groups allow it; walking
off is a cardinal shore step. There is no foot-to-new-ship teleport. The helper searches
at most four states per world cell, checks cancellation during traversal, can return one
foot-reach flood for destination listings, or reconstruct mode segments for a selected
route. It is intentionally cohesive in the existing pure router (under the ~1,000-line
file budget) to reuse its native-accurate neighbor semantics.

`baked_nav.cpp::world_mode` copies the current observed transport ID and mode into the
request. `land_after_ride` routes an already-ridden supported Enterprise through the
helper. `parked_vehicle_reach` includes a parked supported pair in list reachability;
`boarding_route` constructs the selected phases for a parked or currently ridden pair.
The normal phase sequence represents walking to/boarding the craft, ConfirmBoard takeoff,
ConfirmBoard water landing, and cardinal WalkOff to shore as needed. Existing consumers
for destination listing, selected directions, beacon routes and autowalk consume these
ordinary structured phases; no new consumer or setting was added. Typed native vehicle
pads, arbitrary Spots, ship-scene goals and ordinary town foot-entry retain their
existing goal policies. Other identities and games retain prior behavior. `same_movement`
includes transportation ID so quick-grid work is invalidated when identity changes.

The real one-cell landing regression is covered separately: actual
`boarding_route` at water (39,13) to shore (39,12) constructs three phases;
legacy `remaining_from` at the unchanged water cell discards the required
Confirm-to-ship phase. `enterprise_conversion_plan` therefore excludes only
these FFIII same-cell conversion plans from legacy remaining-route reuse.
An actual new request replans from the captured vehicle identity; ordinary
routes and the immutable moving beacon corner cache retain their existing
behavior. No new per-frame invalidation, native read or background polling is
introduced. The pre-fix failure is retained in
`build_check/enterprise_flight_20261026/same_cell_before_fix.txt`; final guarded
replay passes: legacy action retained0, guard1, newly planned action1. The final
two-search case took2.142ms worker compute; repeated real-town request3.577ms.

No new hook, native scan, owner wait or pointer transfer was added. The implementation
uses the existing immutable baked snapshot, Navigation worker, cancellation and route
publication path. Existing count/work bound is four states times world cells. The user
blocking rule remains cumulative: mod-caused application lag above 25 ms across related
hooks, frames, the complete action/startup/loading/transition episode or sustained
backlog is a blocker. Asynchronous placement alone is not performance proof. Initial
own-world logs observed 1.2–1.4 ms aggregate native work across three slices, 32–34 ms
elapsed capture and 2–6 ms worker time; these do not establish a main-thread stall.
Full capture, worker/backlog, usable/audible response and game-frame measurements remain
unverified. Existing localized action keys “take off” and “land on water” are in the
12-language catalog.

## Offline evidence and limits

Evidence and recovery live under `build_check/enterprise_flight_20261026/`. The actual
production `land_after_ride` baseline used the real FFIII world bake with airship 6 at
(39,13) and legal ship-13 shore at (39,12): old foot reach was 0 and wrong-owner worker
accesses were 0. The first implementation replay reports shore reach 1 and wrong-owner
accesses 0. The final canonical `enterprise_flight_check` passes. Mounted6/13/15/16 route
to a real town in122 steps with expected conversion/WalkOff phases; selected
plan, browse, three filters and production follower agree. Nearby town18steps,
direct shore without takeoff, parked13 boarding, original5 locked, source/destination
identity, blockers, directional walls, traps, world wraps and cancellation pass.
No native APIs were invoked from worker tests.

Final real-world coverage7.539ms worker compute,23,250 expansions, peak queue242
over16,384 cells (65,536-state ceiling). Fifty rapid cancellations use5,050 token
checks and3.118ms cumulative worker time. Combined plan+browse+three filters+story
checks per identity use74.996–80.308ms total worker compute; earlier concurrent
compile load produced209–519ms. These aggregate multiple requests and do not
measure native callback cost, first audible output, or application unresponsiveness.
Source adds zero native capture reads. The canonical all-six build/checks pass:
8275 bound names/0 missing;141used/348defined/0undefined wiring. Final receipts
and source hashes: `build_check/enterprise_flight_20261026/checkpoint.json`.
The snapshot has0 active native ship scenes, so a final-ship scene fixture is
inapplicable there. Quick-query native capture and actual frame/audio latency
remain explicit gameplay-side gaps.

The initial canonical sandbox build stalled at globs; that record is preserved as
`sandbox_build_stall.txt`. The escalated canonical build compiled, then the existing
route-worker sequence stopped after FFII timing. Its unchanged direct retry exited 0
with 0 failures (`baseline_route_worker_retry.txt`); the original partial output remains
in `baseline_route_worker_build.txt`. These observations are retained without assigning
a cause. The final full canonical suite passed. Combined smoke, security scan and install checks
remain pending after the other human-requested scoped tasks.
No deployment or gameplay confirmation is claimed.

## Combined delivery acceptance

Installed and hash-verified all six games after full canonical8287/0 native names,
wiring142/349/0,47-export smoke,67 embedded resources and134 indexed registrations.
Exact22-file Defender scan: no threats/zero matching detections/unchanged hashes,
enabled/current definitions1.461.216.0. All132 installed payload hashes match;
144 protected settings/dependency entries are unchanged. DLL11032064 bytes,
SHA25673115338cf025dc45b46673fbaabcf69dca62730756c623fcd42530359154013. Recovery/source/test/scan/install receipts:
build_check/ff3_navigation_20261026/. No game launch/control/runtime probe.
All five scoped source checkpoints and combined local delivery are Closed;
actual gameplay/native/frame/audible confirmations remain separate. Published
release1.0 was preserved; this maintenance does not replace its archive/tag.
