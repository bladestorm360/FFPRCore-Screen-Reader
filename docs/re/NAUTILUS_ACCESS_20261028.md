# Nautilus dive guidance and underwater entrances

Status: dive/underwater/ordinary-landing source and delivery Closed. Full serial
build, offline checks, isolated smoke and exact-payload scan pass; FFIII is
installed/hash-verified. No game was launched,
controlled or probed. The startup candidate was held by the running-game guard;
the human then requested these corrections before installing it. The broader
all-unlocked-vehicle applicability audit remains open and separately recorded in
[the full audit ledger](MAP_AUDIT_FOLLOWUPS_20261028.md).

## Reported state and finite checkpoint

The user's latest FFIII session reaches underwater world3, then requests entrances
from cell113,118. All three grouped scene-labelled destinations fail; native
entrance tiles are44,138 and45,138 (Temple of Time/map242),95,77 (Saronia
Catacombs/map234),232,202 (Sunken Cave/map269). Actual logs are archived in ignored
build_check/nautilus_access_20261028/underwater_session.log. The bake already
contains the complete256x256 map, seven transport layers and all four script
arcs: missing terrain is not the cause. Production before-check reproduces12
failures across classification, full routes and list eligibility.

Checkpoint: correct native dive-versus-land feedback; classify these real
transfers under Map Exits; route underwater using the mounted Nautilus until
the script itself disembarks; verify the exact logged start, physical endpoint,
list and actual native detector together; deliver with the checked startup
update. Estimate45–75minutes including final serial build/scan. No synthetic
surface markers are required for this reported location: the corrected real
router reaches all four entry tiles from113,118 (eligibility distances101,102,
59,223 respectively). This is offline source/data evidence; audible gameplay,
all possible dive origins and actual native/frame timing remain unconfirmed.

## Native mechanics and source locators

FFIII transportation8 is the original Nautilus. Transportation9 is its actual
diving upgrade: release906, group6, landing group10, confirm landing terrain16,
end_landing script358/sc_e_0345. The earlier investigation summary calling358
a down_script was incorrect; that field is0. The complete script changes to
world3 with its arrival script applying submarine transportation14/group7.
The submarine's up_script360/sc_e_0346 returns to surface world4. Landing group10
adds terrain16 to ordinary group6 landing types4/14/15. Ownership is established
by the currently mounted model, never by a nearby parked vehicle or guessed
story progress.

Native FFIII locators: FieldController.UpdateStateSwitchLandable296900,
CanLandToWorldMap(int)278900, GetCellAttribute27E800. CanLandToWorldMap retains
native duplicate-vehicle/collider checks, wrapped position, attribute and the
current model's landing list; its actual visible-guide boolean remains the
authority for availability. Original extractor tools/re/map_location_native.py
--dive-guidance retains matching DLL/dump hashes and source outside Git at
../tools/dive_guidance_20261028/ff3/. Full source scripts stay outside Git in
../tools/mapdump/ff3_scripts.jsonl. GameAssembly SHA256 is
8db9ab31a893ca4b13e5e4d2e2b1b086ac6e3fb5699b9be1d8037dafaaa3ae31.

Coordinate ABI/work inspection additionally confirms the exact aggregate-return
wrapper ConvertWorldPositionToCellPosition27C7C0 (30 instructions) delegates to
WorldPositionToCellPosition2982A0 (92 instructions): finite arithmetic/rounding,
no backward branch in either body. Source-bearing coordinate_conversion.txt and
world_to_cell.txt remain beside the external native archive. The latter reads
the same current map range already used by the native landing decision. This
supports bounded coordinate observation, not a real-game latency measurement.

Underwater native events8/9 (entity4/5/script370),10 (entity6/script362) and11
(entity7/script366) run complete five-operation disembark/map-transfer scripts.
Their complete arrival scripts contain travel/UI/bookkeeping, with no narrative
dialogue or battle. tools/submarine_entrances.py pins every actor/native ID,
root and arrival body hash and actual target/point. Bake generation adds only
ordinary-transfer and submarine-entry flags to the four existing Warp records;
native objects, geometry, master tables and arc endpoints remain unchanged.
This is an explicit reviewed entry policy, not a general rule permitting a
submarine to enter ordinary surface town doors.

## Implementation and ownership

fieldnav/vehicle_landing.cpp retains the existing Show/Switch/Hide hooks and
native originals. Only ready FFIII model9 on world4, with a visible available
guide, reads its current foot terrain through the established FieldMap lifecycle
cache and two read-only native coordinate/attribute functions. No getter creates
a singleton, no scan/worker wait/bake decode occurs, and no native pointer leaves
its owner. Finite coordinates are normalized to the verified256x256 world before
the native attribute read, bounding native negative-coordinate wrapping. Missing
or failed observations remain silent. Other games/models add no terrain read.

The event guard now tracks the native available action alongside its boolean:
land to dive and dive to land are distinct edges even when true stays true.
Both native UI producers share that state. Loading/fades seed state silently;
control return cannot replay it. Map/battle resets preserve the established
context policy. Can dive is localized in all12 languages by tools/modtext_dive.py.
No text-content deduplication or new controls/settings/timer/hook is introduced.

field/baked_map.h defines the reviewed bit18 submarine-entry marker. The pure
baked router checks submarine terrain and that exact entry marker; goals and
boarding retain the mounted mode through arrival, with no fictional prior
landing. Discovery's existing verified-transfer index produces actual destination
names under Map Exits, without an Events duplicate. All planning/category work
uses owned snapshots on Navigation. No native script, vehicle, collider or
movement state is changed by these corrections.

## Verification and limits

tools/vehicle_landing_check.cpp executes the actual hook/helper ABI with copied
native-owner fixtures:5 before failures to0 after, land/dive true-to-true edges,
shared producer suppression, re-entry, loading/fade silence, failed reads and
other-game/unupgraded-model abstention. tools/nautilus_route_check.cpp runs the
real map3 bake and production planner/list distances from the logged origin;
its initial12 failures become0 after classification/entry correction. Detector,
wrong-vehicle, ordinary-aircraft landing and unfiltered-map checks are part of
the final checkpoint. Retained failed fixture compilation assumptions are not
counted as valid production before/after evidence.

Actual detector chains now publish all4 underwater tiles as named Map Exits.
Wrong-aircraft entry is rejected. Doga's Manor at128,184 and both northern
Saronia examples require ConfirmLand before walking into their ordinary doors;
the former weakened Saronia landing assertion has been restored. Shared
mounted-foot-target classification retains native mounted armor/Boko entry,
reviewed ship/submarine entries, special landing pads and real airborne scenes.
Current map carries an explicit unfiltered discovery policy into the worker:
unreachable world exits and route-story-gated rows remain, including with exit
grouping off; explicit sibling grouping may still choose one physical entrance.
The source-level detector/list/route/landing checks all pass with zero worker
native-owner violations. A failed wrong-aircraft fixture first left group0 and
therefore tested walking instead; corrected to actual14/group7 before its
negative aircraft-type assertion. The old soundscape fixture expected its
blocked castle exit to vanish even in Current map; it now proves both that exit
and independently typed Saronia Airspace survive. These fixture corrections
preserve the documented requested behavior rather than accepting bad routes.

The user's clarification that landing failure concerns Nautilus generally is
covered by an additional actual-planner survey:63 aircraft/world/model witnesses
across all six games each require a structured landing action before the exact
ordinary door tile, with0 failures and at least one witness per game. This
extends the test beyond Doga's Manor; it does not certify every story/collision
state. The current broader98 boarded-transport witnesses also pass (90 other
world/model combinations have no eligible fixture start/land destination). The final
dive-host sample observes1000 copied-native reads in0.440ms aggregate/0.001ms
peak, excluding actual native/game/frame/audio costs.

Normal guidance adds at most two bounded read-only native calls per ready
Nautilus9 available-guide observation, on the existing native event cadence.
No background backlog or per-frame job is added. Actual combined callback cost,
startup/loading episode cost and first audible response remain unmeasured; fake
hosts cannot certify the user's cumulative25ms mod-caused lag limit.

## Delivery checkpoint

Final DLL11103232 bytes, SHA256
b2cacd6fab1b71d4d7cab5ff52567c151d8c0e24b2e2ea1d549a49e71b1a257b.
Canonical build8325 names/0 missing,142 used/349 defined wiring/0 undefined;
47-export isolated smoke passes. All67 resources match source,136 feature
registrations are indexed and Can dive has all12 languages. All-six data compare
proves46 reviewed ordinary-transfer promotions (FFIII7/FFV39) plus4 submarine
entry flags; native geometry/objects/masters/old arc endpoints are preserved.
No new map arc is fabricated. FF1/2/4/6 bake bytes are unchanged.

Defender reports no threats across the exact22-file staged payload; hashes remain
stable through final canonical validation. Existing unsigned/no-certificate and
unavailable protected-exclusion-query limits remain; no protection setting changed.
FFIII installation via deploy.bat succeeds after the human closed the game.
All22 installed hashes match,144 protected files retain bytes/timestamps, and
the other five game payloads are unchanged against installbefore. Recovery,
before/after receipts, resources/source-to-bake proof, retained failed attempts,
full/targeted reports and security evidence are in ignored
build_check/nautilus_access_20261028/. Startup safety/diagnostics are included;
the intermittent hang's exact producer and gameplay repair remain unconfirmed.

The Current map ordinary-world-exit pruning correction is delivered, with
blocked exits retained. Full discovery still needs the authorized provider audit:
synthetic nearest aerial entries must also remain discoverable in unfiltered
mode when no immediate ride proves reachability. This is an open integration
case within the human's literal Current map requirement; do not claim all
providers were certified by the ordinary-row fixture. All-unlocked vehicle
capture/per-leg applicability and the broader six-game audit remain open.
