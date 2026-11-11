# Floor and world exits, October 13, 2026

**Ebot’s Rock source/focused/documentation Closed, November4:** twentieth guide
uses actual current-room coral/chests/layout, the native22 threshold, active
Strago first-feed check, correct keep/feed choices and full first/repeat Hidon
completion. Fresh random warps have12 real outcomes; first entry stays
deterministic. Normal routes label random attempts and retire on actual arrival
without consuming the pad. No terrain/native object/other-room change, new hook
or native getter call. Actual152 route/state assertions,27 pocket/layout cases,
442 guide identities/current controller, role readers17/18/37 and all-six route/
Magitek regressions pass. Five keys/all12 locales. Exact source/report/resource
identities: broader_audit_20261104/ebot_completion.json.1117logical/1149physical;
350V+186VI=536 pending. No Main DLL/install/game/probe/agent; combined release/
delivery and natural gameplay remain required.
[Topic](EBOTS_ROCK_GUIDANCE_20261104.md).

**Magitek Factory source/focused/documentation Closed, November4:**27 exact
Factory/Research identities reviewed. Eleven native ladder/conveyor links
(ten manual/one automatic) restore missing lower/upper routes on396; no absent
46/31 operation is granted on643. Exact22-object addition preserves all terrain,
old objects/other rooms/initializers/19 story/five static guides. Normal same-floor
manual directions now retain the planner's exact receipt beside the control;
actual start and exact landing own completion. No new hook/native read/write/
queue/search or VM design. Hidden passage397/PointIn11 already matches17,28.
Actual11 arcs/11 approaches/50 assertions, all-six routing105 manual/84 directional
and trap regressions pass. Stale long-warning fixture expectations corrected;
actual concise warning policy retained. Source/semantic/canonical/docs/evidence:
broader_audit_20261104/magitek_completion.json;
[topic](MAGITEK_PASSAGES_20261104.md).1114logical/1146physical reviewed;
350V+189VI=539 pending. No Main DLL/install/game/probe/agent. Combined release/
delivery and natural gameplay remain required.

November3 [Owzer audit correction](OWZERS_MANSION_20261103.md): enabled
instantiated object-collider0 is a full wall, unlike terrain-grid None0. Exact
GetCollisionRect/SetBoxCollision/Init/CanEnter evidence in all six binaries
supports the current/remote/transport-specific correction. Partial2–5 and
axis6–7 behavior, selectors and native grids are retained. Actual closed/open
door cases, Chocobo/Owen and all-six transport fixtures pass. The combined
native output and broader final audit checks remain pending.

October26 [locked-door approach](LOCKED_DOORS_20261026.md): an Objects/Door row
uses Interact even when a GotoMap shares its tile. The room's Transition remains
blocked by the native lock, while an accessible Door can be examined without a
key. Selection and story filtering share the policy; separate approach barriers
still gate. FFIII21/21 Door replay and all-six regressions pass; bakes unchanged.

October26 [remote return listing](REMOTE_RETURNS_20261026.md) omits only proven
reciprocal ordinary return doors from other floors. Native PointIn/footprints and
directed minimum depth establish the pair; one-way/uncertain and current-floor
doors remain. Complete bakes and routing graph are unchanged. All-six replay/
preservation tests and scanned deployment pass; new listening remains pending.

October23 [script-backed world exit correction](WORLD_EXIT_FOLLOWUP_20261023.md)
classifies direct native Event travel as MapExit from per-game script evidence.
Prepared flags/indexes preserve ordinary GotoMap behavior, category grouping and
physical terminal tiles, without false scene-trigger route warnings.

The user relayed missing South Figaro floor exits, especially houses, and requested
an FF5/FF6 bake audit plus reachable world transition tiles and nearest-exit grouping
in all six games. Sol handled discovery, implementation and verification alone.
No tester logs/saves were accessed and no game was launched or probed.

## Reproduced source faults

- Every raw GotoMap/ToLayer was present in the old baked object tables. Geometry
  nevertheless disconnected their approaches or failed the native dimension check.
- FF6 has 406 MapRange records differing from their shared tile sheets. The old
  bake ignored these rectangles. Grids and objects now use native room bounds and
  subtract the room origin. South Figaro Map_20071_11, for example, is 27 by 20 at
  asset origin (100,41), not 128 by 64. Six existing FF6 puzzle step coordinates
  receive the same shift; their original asset coordinates remain in `_about`.
- Raw collision IDs are not boolean walls. Native CollisionFilter preserves half
  tiles' cardinal direction masks. DirectCollision/DiagonalBinaryInfo joins open
  cells alongside triangular stairs. Schema 2 stores these recovered masks;
  the reader remains compatible with schema 1. Search, flood, directions and
  autowalk share the edges. Open rooms gain no unrestricted diagonals. Native
  movement still checks every executed edge.
- ToLayer acts from its own layer. Inventing a reverse switch at the same tile
  could send a route straight back downstairs. The reverse direction now uses
  its separately placed native trigger.
- An unresolved package alias overwrote complete geometry with null dimensions
  and empty grids: Map_20051 mentions FF5 Karnak's Map_20050; Map_20050 mentions
  FF4 Kaipo's Map_20030. Their owning bundles restore the grids. The extractor
  preserves earlier complete geometry; `tools/repair_bake_sources.py` performs
  bounded recovery. Original JSON is archived under
  `../tools/mapdump/exit_audit_2026_10_13/`.
- Blob offset zero also meant absent data. Reserving a byte preserves the first
  grid, restoring FF3's intro-world attribute grid. All six bakes were regenerated.

## World exits and grouping

World passability requires both the transportation grid and the native terrain
permission. Native CheckOkList rejects out-of-range attributes. FFV's reviewed
Walse terrain resolves through the native story-state conversions below. Transition
goals need an actual GotoMap or step-on warp footprint. Empty/unenterable footprints
stay empty, removing the former fallback to a decorative or blocked tile.
Ordinary entrances are entered after legal landing/walking rather than flying over
the town being treated as entry.

The worker measures each exit row's exact active GotoMap footprint against copied
current movement and obstacles. Sibling arcs cannot rescue an invalid row.
Unreachable world rows are pruned even with grouping off. Grouping keeps the
shortest reachable route per destination, independently of list sorting, then
prunes the other rows. An inaccessible closest tile cannot hide a reachable one.
Indoor groups retain one representative if all routes are blocked, preserving
the existing explanation. Remote exits use the cross-floor planner.

Legal shore floods retain ride costs rather than making distant shores free.
World wrapping remains. Browse, directions/autowalk and beacon requests obtain
the current representative after movement/vehicle changes, and stale publication
is rejected. Existing category event guards remain; no text suppression was added.

## Native evidence and ownership

`tools/re/exit_native_check.py` records installed binary/metadata and source/bake
SHA256 values in `build_check/exit_native_provenance.json`. All six installed
metadata files match these initializer SHA1 identities:
CollisionFilter `8C4FA5D2E9C81CD19F19DA7FE51DB94B4413C3C8`, DirectCollision
`313F7A37A0D731DF68F640F81946E99CBBA07B2B`.

| Function | Matching RVA | Evidence/consumer |
| --- | --- | --- |
| FF5 MapConstants..cctor | 0x2E8740 | Native table initialization plus metadata; baker |
| FF5 MapUtility.ParseCollisionMappingData | 0x2F6570 | Cardinal masks and conditional stair diagonal patches |
| FF5 TransportationInfo.CheckOkList | 0xA97BE0 | Bounds/permission at attribute minus one; world eligibility |
| FF6 MapUtility.ParseMapRange | 0xCDE000 | Object type 27; pixel rectangle divided by 16 |
| FF6 MapUtility.ParseMapObjectGroupData | 0xCDDB80 | Subtract rectangle origin; native local position uses rectangle dimensions |
| FF6 MapUtility.ParseCollisionMappingData | 0xCDC480 | Cropped grids and cropped diagonal indices; baker |

Private ignored exports: `docs/re/ff5_exit_current.c`, `ff5_exit_attribute.c`,
`ff5_exit_movement.c`, `ff6_exit_current.c`. The older FF5_Analysis project did not
cover two matching targets correctly. Bounded no-analysis current-build projects
under `build_check/` preserve the old archive. Current scripts, function signatures,
addresses, logs and bundle hashes remain in these local evidence records.
No game function was executed for discovery.

Capture reuses EntityCursor, NativeObstacle and the central CaptureBudget. Native
pointers stay in the owner closure. ExitSnapshot contains copied state, cleared
vehicle pointers and immutable bake references. Navigation owns masks, floods,
route distances, remote planning, grouping and category construction. The native
owner never waits for work; request epochs cancel obsolete computation. Ordinary
floods keep FIFO BFS; only nonzero shore seed costs need a priority queue.

## FFV Walse terrain November 10

The actual November 7 FFV log part 1 reports no goal cells for the Tower of Walse
at (196,88) and Walse Meteorite at (204,84). The party at (195,87) later entered
the tower manually. The game assets place GotoMap entity 64 to map 234 and the
active meteorite script or door entity 48 to map 244 at those exact cells.

The installed FFV misc bundle's MapFootAttribute record for map 1 resolves raw
terrain 42 through 47. MapAssetLoader.CreateMapFootInfo at RVA 0x2A3EA0 applies
the ranges via MapUtility.EnableRangeFlag at RVA 0x2F55E0. The meteor scene
sc_e_0038_3 sets ScenarioFlag4 bit 222 and the tower scene sc_e_0476 sets bit
223. Those bits are 990 and 991 in the copied scenario bank. The tower entrance
changes from terrain 20 to water 16. The meteorite changes from 6 to 20.

The owned route request resolves only the six Walse terrain IDs before the
shared eligibility, walking, boarding and landing checks. FFV's
TransportationAsyncTask at RVA 0x2AF800 replaces the collision cells for those
IDs from the converted terrain's permission in each transportation group.
The planner mirrors that rule for Walse and keeps all other transportation
cells, native transition footprints and group prerequisites.
The live edge guard retains the existing native collision check. The change
uses the existing request paths and adds no hook. Other high FFV terrain IDs
require separate source qualification. Gameplay and cumulative application
timing still need confirmation.

Asset SHA-256 values are `b8dfed01eb0d4c56b7af506671f1314cb33bf57d02e59dae0b37c0a0d56f197b`
for the FFV misc bundle and `28671174335cdc3cfd3ef35eb10f8be1deaf73394d5beee297fc052dc2c43151`
for Map_10010. The maintained FFV story-contact replay covers the logged
walking pose and Wind Drake landings, both active meteorite phases, the sunk
tower and unknown flag state.

## Straight-line bearing for story-gated targets

The current entity announcement adds the approximate geometric distance and
bearing to the existing Story gated note, including across a looping map's
seam. Entity browsing and K or P use the same formatter. A blocked explicit
directions request can report the same bearing. Other floors keep their floor
label because their local coordinate systems have separate origins. The
independent Hide story-gated destinations option hides those rows when enabled.
The bearing is informational and never becomes a route.

## Offline coverage and explicit limits

`tools/map_exit_check.cpp` uses the production reader/router. All 178 South Figaro
exits across both town variants have arrival-connected approaches; cropped room
bounds and recovered Karnak exits pass. World trigger geometry receives separate
terrain/approach checks. The actual worker/grouping fixture checks all 582 world
GotoMap candidates across 24 world records, prunes unusable rows from representative
starts, routes retained representatives and tests copied native gates. The existing
103 boarded-vehicle cases remain. Scanner/speech checks cover movement, current
publication, deferred planning and stale categories. Autowalk replays real routes
from all six regenerated bakes, including native stair edges.

This does not certify every script/puzzle state. Remaining FF5 static arrival
flags: Istory's upper world exit (Map_20090) and collision-less scripted Map_40011.
Remaining FF6 flags: three Lethe River forward exits (Map_30380), four Magitek
Factory exits (Map_30521_1/11), and Kefka's Tower Map_30871_4's north exit.
Lethe River requires native raft travel. Other disconnected cases remain exact
script/state investigations or gameplay checks; do not open walls or claim them
fixed from an arrival flood. They are distinct from the reproduced South Figaro
bake faults. Remaining zero-dimension FF5/FF6 master entries have no extracted
objects or incoming GotoMap/ChangeMap reference, rather than a demonstrated
navigable map. Preserve these distinctions.

Real native capture/frame impact, usable/audible response and special-state
behavior remain unmeasured; asynchronous placement alone is not responsiveness
evidence. Final offline results and deployment are recorded below.

## Prioritized gameplay checks

1. South Figaro: enter a house with stairs, browse Map exits, request directions to
   the next floor, then return. Expect listed exits and a stair route, without map
   data unavailable, no path on an ordinary stair, or a wall route. Repeat with
   autowalk if enabled; manual input must stop it.
2. A reachable world town: enable Group exits and browse Map exits. Expect one
   usable entry per destination. Follow directions onto it. After returning, move
   closer to another entrance to the same destination and request directions again.
   Expect the new nearest usable trigger. Grouping off restores individual usable
   triggers while decorative/unreachable tiles remain absent.
3. When available, repeat world entry aboard the FF2 ship: expect legal
   disembarkation and walking, retaining the previously confirmed ship behavior.
4. Progression-dependent Istory/Magitek Factory/Kefka's Tower cases: record the
   actual map/floor and whether the gate, lift or puzzle has been operated if the
   ordinary route remains unavailable. Own-session logs are useful only when the
   user requests inspection. No broad capture or early game launch is needed.

## Final validation and deployment

Canonical `build.bat` passed with all included offline regressions: 7,077 native
name checks, zero missing names and zero undefined wiring. The production
reader/router, actual filter worker, scanner/speech readiness, map state, vehicle
routes, input and autowalk checks passed. `smoketest.bat` passed in its isolated
host: all 47 exports resolved/forwarded, normal boot thread, no crash/fastfail.
Neither script launches a game. `git diff --check` passed.

Fresh `tools/bake_maps.py all --out build_check/exit_roundtrip --check` output
matched all six committed-data locations byte for byte, including collision
roundtrip checks. The native initializer check passed for all six installed
metadata files. Production autowalk replay passed 2,293 sampled routes (FF1 118,
FF2 338, FF3 345, FF4 307, FF5 591, FF6 594), including stairs and wrapping.
The world worker fixture evaluated 582 candidates from representative starts:
46 reachable rows and 536 pruned rows. These are start/state-dependent results,
not permanent deletion of 536 triggers. It routed the retained rows and checked
native gate vetoes; all 103 boarded-vehicle cases also passed.

### Comparable offline performance

`tools/re/exit_perf_baseline.py` staged the previous reader/router and bakes from
HEAD `1c8dfb689818b59eea491601ec39692c1ba36a70` into ignored
`build_check/exit_baseline/`, then ran the same current performance harness. The
current implementation ran after that baseline, with no concurrent build. Both
checked 24 world records and 16 far routes with zero failures. Fifteen start/goal
pairs were identical; FF5 Map_10010's far goal changed from (70,157) to (63,158)
because native terrain validation excludes the old goal.

| Offline workload | Previous worst | Current worst |
| --- | ---: | ---: |
| World flood | 6.521 ms | 10.904 ms |
| Far world route | 51.927 ms | 64.235 ms |

The stricter native terrain checks add worker computation. These single-run
measurements are not distributions or game-frame/audio latency claims; the route
maximum includes the changed FF5 goal. Per-case inputs/timings and comparison are
in `build_check/exit_baseline/world_baseline.txt`, `exit_world_current.txt` and
`exit_perf_comparison.json`. The actual route-worker regression's worst vehicle
route/filter were 37.910/18.728 ms; world exit/category preparation's worst was
3.658 ms. These are separate offline workloads, not a combined pipeline duration.

Native observation/publication uses the existing 0.5 ms elapsed CaptureBudget per
continuation, checked between operations; this is not a hard cap on one native
call or on aggregate frame work. The fake-native scanner fixture published 2,000
rows in four ticks and passed movement-refresh/stale-result checks. Warm category
selection measured about 0.023 microseconds there. Those fixtures cannot measure
real Unity read cost, input-to-usable/audible response, data age, cancellation tail
or aggregate native cost under gameplay load. Pose/vehicle changes reject stale
publication and recapture; continuous movement can delay publication until a
current snapshot is ready. Worker cancellation checks run between flood/route
expansions and exit rows, using the owner epoch. Special script states, worst-case
overload and real cancellation delays remain untested.

### Installed build and recovery

Before deployment, the six installed DLLs were copied and hash-verified under
`build_check/exit_recovery/ff1-hid.dll` through `ff6-hid.dll`; `manifest.json`
records their previous `E3F6C174CAE0FE65D969E3258B72936035EBA579BE051084B1BBB0845D0602BE`
identity. Canonical `deploy.bat` passed its closed-game/Prism checks and installed
the new DLL in all six games. All installed hashes match the 9,122,304-byte build:

`24C1EF06A7E7153CADB8249D0C8FEE06324FB6E46C751320C0CC97E037E692B5`

`build_check/exit_deploy.json` retains the six-game receipt. Prism was unchanged;
no game launch, runtime probe or dependency install occurred. Gameplay confirmation
of these new changes remains pending. Sol performed the entire task independently.
The public V0.9 archive/tag/release was not updated during this fix. The user
subsequently requested commit/push and release 0.9.1; its packaging/publication
record is maintained in [release.md](../release.md).


## All-game asset audit, October 14

The user requested every bake be checked, especially complex FF4–FF6 towns,
multi-floor dungeons and FF6 World of Balance/Ruin variants. Sol independently
re-extracted all six installed games with `../tools/dump_map_objects.py all` into
ignored `build_check/map_audit_2026_10_14/fresh/`. No game process/function ran.
All extraction runs reported zero errors. Retained and fresh submaps, grids,
objects and scripts match semantically; master data was copied from the existing
matching extraction into this private audit directory, not changed in game folders.

`tools/re/map_bake_audit.py` regenerated each bake, decoded its binary grids and
validated every native MapRange, collision/stair layer, hidden passage, terrain,
transportation grid and map-id mapping. Native collision initializers still match
all six installed metadata files (`exit_native_check.py`). Counts:

| Game | Submaps | Collision cells checked |
| --- | ---: | ---: |
| I | 129 | 413,369 |
| II | 349 | 263,355 |
| III | 411 | 287,449 |
| IV | 368 | 234,726 |
| V | 655 | 574,822 |
| VI | 638 | 657,695 |
| Total | 2,550 | 2,431,416 |

FF1–FF5 regenerate byte-identically. FF6 has one confirmed additional conversion
fault: Map_30641_2, native map id 449, has MapRange pixel y=-2. Native signed integer
division truncates to tile origin zero; the baker used floor(-2/16)=-1. Python
negative slicing discarded the rows, then padding turned both collision layers
into all-blocked masks. The baker now uses the signed native quotient for x/y/w/h.
The previous private bake decodes to 1,116 zero masks per layer (raw padding value 1). The corrected room is 36 by 31, with both Ground/Upper collision layers retained.
`map_exit_check.cpp` checks that these layers contain native blocked cells instead
of uniformly blocked padding. No object coordinates or unrelated puzzle routes change.
Only `data/ff6/maps.bin` changed: SHA256
`DD75D6EA83FFCB921ACCB3C1E6E0B3BEE71A6DFD6EC5041E525329693E612729`.
The previous binary is preserved in this audit directory.

The private report records all rooms, dimensions, map ids, layers and object
counts, including mapped scenes without native grids. Each variant retains its
own submap/mapping; the audit does not merge rooms by displayed town title.
Both South Figaro sets still pass all 178 floor exit approaches. The existing
production router/follower checks replay 2,294 real routes from all six bakes.
Fresh-source parity and every static grid pass; this is not confirmation of every
native script, puzzle or vehicle state.

The static exit audit also records two FF1 intro-map variants with no native
collision/terrain grid, two FF4 Baron event exits outside the 34 by 34 map (group
ev_e_0033, x=-2), and FF4 Map_30141_1's disconnected Bottom exit. FF5 Map_40011
also has no native collision grid. The Istory, Lethe River, Magitek Factory and
Kefka's Tower cases described above remain exact script/state or gameplay gaps;
no walls were opened to force them to pass. Original objects/scripts for all
flagged cases are preserved in `flagged_native_records.json`. Absent native
geometry is recorded explicitly rather than invented. No further conversion or
bundle-overwrite fault was found in the fresh extraction.


## Exact selected entrance correction, October16

The shared goal builder previously merged sibling entrances to the same floor/arrival.
Explicitly selected transitions now retain their own x/y, including multi-cell trigger
footprints. Eligibility distance, browse and explicit planning use that same point;
optional exit grouping selects its representative before planning. The FFIV exception
below uses a reachable entrance only when native arrivals are identical. Native terrain/layer, story state,
vehicles, interaction facing and cross-floor transition rules remain in effect.
[All-six evidence and Cornelia replay](BASELINE_FIXES_2026_10_16.md#exact-transition-goals).
This supersedes older nearby-arc goal-union descriptions; bakes are unchanged.

## November 10 FFIV directions while flying

An explicit spoken directions request to a selected target on the current
world map uses the live airship identity. The first route may finish after
the ship has moved. For an ordinary flight and landing route, the same
navigation worker can then update the flight leg from the new tile using
the owned terrain and gate rules. The selected landing and final foot
approach are retained. The adjustment is attempted once and limited to
8,192 expansions. It shares the original request's cancellation epoch.

If movement continues through that publication or there is no qualified
flight leg to update, the response is an explicitly straight-line bearing.
It names the planned landing when available and carries no movement route.
A map or vehicle change retires that response. The native owner reads only
the current pose and vehicle and formats the bearing. Both path searches
and full route formatting stay on the worker. This slice covers FFIV manual
directions on the current world map. See the [scope and replay evidence](MAP_AUDIT_FOLLOWUPS_20261028.md#november-10-ffiv-directions-during-airship-movement).

## November 10 FFIV world entrances and remote returns

For an inaccessible ordinary FFIV world entrance, the owned route worker
can finish at a reachable entrance to the same map and unique native
PointIn. The physical rows and existing world exit grouping still use
their original identities. Directions, browse and autowalk use the same
proved route endpoint. Routes into different arrival positions keep their
own terrain and cross-floor handoffs.

On foot, a proved ground route takes priority over boarding a parked FFIV
airship. A new boarding route needs actual flight before landing. A player
already aboard may land on the current tile where the native rules allow it.

An indoor remote exit that returns to the occupied floor is suppressed
when its source floor has just one known native landing on the occupied
floor. Compare layer and tile. Distinct or uncertain landings remain
selectable even with exit grouping on. The owned projection marks a redundant
scripted return and the scanner skips it before resolving names. Raw scripted
travel records and all native routing edges remain available.
[Scope and evidence](MAP_AUDIT_FOLLOWUPS_20261028.md#november-10-ffiv-world-entrance-directions-and-remote-returns).
