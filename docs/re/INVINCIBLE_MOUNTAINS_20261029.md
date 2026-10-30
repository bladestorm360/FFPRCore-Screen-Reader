# Invincible mountain crossing and Falgabard access

Later human-scoped follow-up: [mountain preference](INVINCIBLE_PREFERENCE_20261029.md)
prioritizes ordinary flight and minimizes required actions before physical
distance. Original85-tile witness/measurements below describe the prior objective;
the preferred valley path is now87 tiles. Native geometry/manual ownership remain.
The separately reported Manor rejection was correct: human confirmed Nautilus is
required through the winds. No Manor restriction was relaxed.

Scope: next open FFIII checkpoint in the human-requested full map-event and
reachability audit. The reported page/vehicle follow-ups and per-leg selector
implementation are installed in FFIII d3053479; their accepted design remains.
This feature adds the actual unlocked Invincible mountain action to navigation,
including category reachability before boarding, selected routes and manual
guidance. Other games/vehicles keep their existing terrain and input behavior.
Sol only. No game launch/control/probe, new dependency, agent or publication.

Status: source/offline/delivery **Closed**. FFIII ad0d80c1/11327488 bytes installed
and hash-verified. Serial canonical build,8406 native names/0 missing, wiring,
smoke,67 exact embedded resources,12 locales and22-file scan pass. Gameplay/audio
remain unconfirmed. Broader all-six audit remains open and is a separate system.
Estimated checkpoint60–90minutes plus serial release checks; report overruns or
new semantic dependencies. Do not declare Falgabard reachable until the real
owned provider/filter/boarding/flight/landing/door path passes offline.

## Pre-change evidence

Falgabard's3 ordinary scripted entrance cells on surface world4 are(36,101),
(36,102),(37,101), native entities888/889/890, script381/sc_e0354→map144/point1.
They already have names and Map Exits classification. A walking valley start is
reachable; aircraft10/11 still fail because the router treats mountains as
ordinary closed tiles. Nautilus8/9 must continue to fail this crossing.

Exact native owner Serial.FF3.Map.TransportationEvent:
OnAirshipSpecialEvent386B10, IsFrontMountain3861C0,
IsOverTheMountain386610. The latter follows the facing vector, wraps positions,
permits at most2 consecutive terrain19 cells, and requires the first far side's
attribute in the selected native transportation group's allowed list. Scripts
910/911 perform2/3-tile displacement;912 is the failure branch. Match exact
native/script order and action ownership before connecting any new route edge.
Native/data evidence remains outside Git at
../tools/map_scripted_entries_20261028/ff3_mountain_native/.

Original offline prototype tools/mountain_crossing_probe.py on the actual bake:
Saronia(83,62)→valley(36,100), Invincible10/11:85 displacement tiles/4 legal
crossings,10276 nodes; Nautilus8/9 cannot reach it. This proves missing geometry,
not live story state, landing, current capture or audible output by itself.

## Planned bounded implementation/acceptance

Use worker-owned actual transport10/11 only, cardinal direction, at most3 looked-
ahead tiles per direction and actual far-side/actor/terminal validity. Preserve
unit step_direction and all other vehicles. Route/flood cost must include2/3-
tile displacement; do not price a jump as one tile or fabricate traversable
mountain terrain. Emit a manual mountain-action boundary with actual facing,
keep native input ownership and stop autowalk where manual action is required.
Expose only the existing approved destination identification/native action.
Localize new guidance into all12 supported languages.

Check source-to-output through actual parked/current vehicle snapshots, literal
and reachable categories, full directions, legal landing then the exact entrance,
manual boundary/beacon/continued route and cancellation. Negative cases include
Nautilus, missing/unowned Invincible, three-thick mountains, closed far sides,
obstacles/events, world wrapping and other games. Preserve current routing,
NPC blockers, nearby walking, terminal/partial collision and speech contracts.
Record owned-work/cost bounds separately from unmeasured native/game/audio time.

## Implemented source-to-output checkpoint

`field/mountain_crossing.cpp` translates only the reviewed native geometry into
owned-data edges. Capability requires FFIII, a world map, actual model10/11,
type8/group6. Each cardinal candidate checks at most3 wrapped tiles. The first
non-mountain must be legal for that actual model. Nautilus8/9 has the same group
but never acquires this action. No bake terrain is changed or invented.

`baked_router.cpp` validates every skipped edge against existing directional,
actor, terminal, trap and hazard rules. An intermediate event/hazard cannot be
silently skipped even with hazard avoidance disabled. A* and reachability flood
price the actual2/3-tile displacement. Non-capable vehicles retain their unit
edges and unweighted flood. The work remains on the cancellable navigation
worker, with no new native hook, pointer capture, polling, queue or game input.

`baked_nav.cpp::prepare_mountain_phases` splits accepted raw edges at the exact
native action. Ordinary movement phases contain only unit steps; their manual
boundary says “fly over the mountains {direction}” and retains structured
facing. The following phase begins at the real far side. Boarding and the final
landing/door interaction keep their original ownership. `route_progress.h` and
`beacon_route_index.h` include manual displacement in remaining distance and
stop the corner at the action. Existing autowalk returns its manual Action state
with facing and no automated vehicle action. There is no new control or setting.
`tools/modtext_mountains.py` maintains the guidance in all12 supported locales.

`tools/invincible_route_check.cpp` exercises actual planner, parked-vehicle bulk
eligibility, category publication, retained route, beacon and autowalk consumers.
The real surface ownership witness is model10 at(83,63), player(83,62), Falgabard
entrance(36,101). Owned Invincible makes the existing destination reachable before
boarding; absent ownership remains unreachable. Literal Current map retains it
in both cases. It stays in Map Exits and is never duplicated under Events.
The3 real Falgabard entrance cells all pass; Nautilus8/9 still fail. A separate
test uses actual floating-continent model11/world1, rather than claiming that
the surface geometric family test represents native model11 ownership.

Evidence is in `build_check/invincible_20261029/`. Before production, the actual
Invincible selected route was NoPath; after implementation, real valley A* and
flood distances are85 physical tiles, with manual boundaries and landing before
the exact entrance. Initial direct-grid fixture failures also included an
incorrect foot request; those are documented fixture failures, not additional
production defects. Later intermediate-blocker fixtures initially used layer1
instead of the actual world capture's wildcard layer-1; category assertions
initially inspected names rather than source indices. Corrected pipeline,
boundary, navigation-follow-up and all3 world-entrance checks pass. Preserve
these earlier failed outputs; do not count them as unexplained regressions.
The first canonical run then caught one cancellation-fixture error: without an
observed terminal plane its conservative story-trigger fallback ended the flood
before the cancellation threshold. The cancellation-only fixture now supplies
an explicitly empty observed plane and requires more than50 token checks, so a
trivial non-search cannot pass. Production code did not change for this repair.
The initial build/report are retained as first_build_fixture_failure.txt and
first_full_fixture.txt; the corrected full canonical rerun passes, including
actual floating-model and cancellation/other-game abstention cases.

Bounds remain the existing finite world/state expansion limits, snapshot-owned
obstacle/selector planes, at most4 mountain candidates with3 geometry reads each,
and per-search/per-lookahead cancellation checks. Targeted route-worker tests
pass98 boarded cases and582 exits. Measured worst owned route36.665ms and filter
19.106ms are background compute measurements, not game-thread or audible latency
claims. No new hook/native cost is introduced; end-to-end game-frame/audio cost
and live story histories remain unmeasured. These offline timings do not certify
the user's cumulative25ms application-lag requirement.

Final source-to-output acceptance: final_build.txt/final_smoke.txt pass;
canonical_*_report.txt preserves the relevant current regressions (the entrance
survey is separately executed after the canonical build). final_resources.json
matches every embedded source, with no terrain/bake change. security_predeploy
records Defender's no-threat result for22 exact files, stable hashes,0 matching
detections, no signing certificate and OS-denied exclusion-status verification.
deploy_after.json/completion.json verifies22 installed hashes,144 protected files
unchanged and the other5 game installations untouched. Previous d3053479 DLL is
retained in recovery. No game was launched/controlled/probed and no dependency
installed or security setting changed. A receipt-assembly check initially found
the separately-run entrance report absent; it was executed successfully before
recording final acceptance. Do not confuse that missing evidence file with a
production route failure. This accepted design is Closed; continue the retained
all-six audit cases without automatically refactoring it.

## Retained power-lifetime audit dependency

Additional scoped research is retained in VEHICLE_APPLICABILITY_20261028.md:
Hide/Reset and native exit rearm triggers; setup/suspend and actual overlap own
unpowered states. FFVI dispatches EntityOverlap virtually (slot58), including
EventSwitchTriggerEntity's loop-sensitive override; lack of direct xrefs is not
absence of calls. No speculative global powered-trigger revival was added.
tools/re/auto_transport_selectors.py verifies every current world selector on
automatic terrain: all six have0 Player1/automatic-ID membership differences.
The exact stable overlap-origin/rearming question remains paired with the later
conditional-arrival/state audit; no current failure is asserted or hidden.
