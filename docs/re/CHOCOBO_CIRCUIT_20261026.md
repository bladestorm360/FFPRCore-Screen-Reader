# FFIII chocobo circuit guide, October 26

Status: complete, Closed; implemented, Sol-reviewed, offline verified and
installed/hash-verified all six. Combined delivery acceptance is recorded below.
Gameplay/native/frame/audible confirmations remain separate; public1.0 preserved.
All five scoped source checkpoints and local delivery are Closed.

Player task: on the correct world map while riding the chocobo, Shift+G guides
to the circuit's start and then through a complete circuit. Determine the actual
native/script completion conditions and any required step triggers first. Never
reveal the prize, read reward dialogue aloud or write game state. Preserve existing
guide controls, other games and manually selected navigation objectives.

Initial evidence: the prior world-event investigation identifies map1 event2368,
script380/sc_e_0286, as passive chocobo-only ScenarioFlag1/192 bookkeeping. It is
deliberately absent from ordinary navigation and audio. The dedicated guide is
newly authorized; no generic-event or routing redesign is implied. Existing
Shift+G controller is src/core/field/puzzle_guide.cpp, with game-specific dispatch
and owned asynchronous route previews. Exact circuit/quest contracts remain to
be established from matching scripts and native functions outside Git.

Finite finish: recover start/step/completion predicates; implement one bounded
context-owned guide with correct mounted movement, cancellation and localized
instructions; verify real-world data, native semantics and source-to-output
controller offline; document controls/ownership/evidence and close this source
checkpoint. Then complete combined build/smoke/resource/security/deployment checks.
Initial research20–35minutes; implementation/checks40–70minutes after the contract
is known. Report an overrun or unresolved required native fact before expanding.

No game launch/control/runtime probe, hidden-prize disclosure, native flag/script
execution, new polling, unbounded worker/native work or closed-system refactor.
Use copied data and existing owner/worker/cancellation services. Aggregate
mod-caused application lag above25ms remains a blocker; worker elapsed time alone
does not establish frame/audible latency. Raw scripts and fixtures remain outside
all Git trees; retain factual locators/hashes and original extraction tools only.


## Verified native contract

The exact world1/Map_10010 event2368/editor5426 is a one-cell step trigger at
baked(86,25), layer1; Tiled's bottom-anchor row26 is corrected to25. It has
native action2/script380/sc_e_0286, RidingVehicle enabled, and the exact target
transportation list12. Its only effect is setting ScenarioFlag1/192. Scanning
all839 matching scripts finds only this writer and one acceptance/reset consumer,
script247/sc_e_0301 in Village of the Ancients, map60. That consumer tests192==1
and resets it after its native exchange. No lap-distance, direction, ordered
markers, start flag or prerequisite conversation is checked. Exchange contents
were neither exported nor included in speech/docs. The ordinary event remains
route-only and absent from destination/audio lists.

PropertyEntity.CheckTargetTransportationIds RVA4603D0 uses native list membership.
DataStorage.GetFlag CB88E0 maps the category segment and index directly; native
Flags.Get CC06D0 validates index then reads word(index>>5), bit(index&31).
ScenarioFlag1/192 is absolute192, word6, bit0. Matching DLL/dump hashes, native
function locators and source-input hashes are in parent tools/chocobo_circuit_20261026/.
Original tools/re/chocobo_circuit.py verifies the exact event/script/hash/flag
contract without exporting exchange contents; map_location_native.py's
--chocobo-circuit exports eight matching functions outside Git. No game runs.

## Implementation and integration boundary

src/games/ff3/chocobo_circuit.cpp owns a game-specific guide with14 authored
waypoints from its shared header. The first is an approach near the village and
nearby woods, then the west/north coast, exact native checkpoint, east/south coast
and return to the start. Every leg is a Spot on world1 under transportation12,
chocobo type9/group2/landing2. The initial pure-grid prototype is484 cardinal
steps; production planner verifies the same484 steps and all13 legs, as recorded below.

Existing GameModule guide actions own Shift+G/Ctrl+G and mod-mode RB/LB. The sole
core guide integration dispatches its existing movement-input event to FFIII's
module when active. No new native hook/timer/polling/movement injection. Other
indoor guides and FFVI's story guide retain their existing controls. A single
active/step/epoch/last-cell record remains on the native owner. Unchanged-cell
input skips vehicle/flag reads and new routes; changed-cell arrivals advance
once. Explicit repeat rechecks an arrival missed by the last input callback.

Start/repeat/waypoint advance requests the existing asynchronous owned-snapshot
route preview, separately from the manual selected objective. The callback uses
the full next-leg description and rechecks epoch/step/map/vehicle plus cached
menu/native field/battle/transition readiness. Dismounting stops the active guide.
Map-changing/battle-entry/title callbacks retire scalar state/epochs without
engine reads or speech; user stop clears only the guide's preview. New code adds
no native pointer transfer, wait, flags/script write or scene scan.

The small map_state::scenario_flag accessor reuses existing storage references,
validates singleton/bank/1024 logical bits/32..128 backing words and reads exactly
one uint. It is an integration accessor, preserving existing full snapshots and
projection. Only fresh start and final return read192; no per-footstep flag scan.
True at start reports already recorded. Final true confirms recorded; false and
unknown have distinct truthful output. Arrival alone never certifies native
completion. Nine FFIII-only text keys cover all12 supported languages, following
the established Auto/manual language setting.

## Final source acceptance

Terra owns implementation/tests; Sol reviewed native contracts and source,
requested concrete repeat/menu/full-direction/test-data-sharing repairs and
retains final integration. Luna documented the controls/testing/threading entries
from verified facts; Sol retains this native feature record. Final canonical checks pass:8287 native names/0 missing, wiring142 used/
349 defined/0 undefined. Source hashes/recovery are in
build_check/chocobo_circuit_20261026/checkpoint.json. Actual production planning
passes13 legs/484 legal mounted steps/one checkpoint/no boarding or dismounting,
with0 native worker calls. Aggregate38.580ms and slowest leg5.979ms include
per-leg worker launches. Production requests one leg only on start/repeat/arrival,
spread over the player's actual trip; this total is not one native callback.
1,000 stationary controller inputs take0.366ms with no added vehicle reads/jobs;
1,000 explicit fake-capture repeats plus stop take1.688ms, stale callbacks silent.
These controller fixtures exclude actual capture/scheduling/native costs; existing
service serial cancellation is retained, not re-benchmarked as real game latency.
Flag reader bounds/unknowns, core FFIII dispatch/FFVI exclusion, menu/transition/
battle/title cancellation and positive/false/unknown completion pass. First canonical had a duplicate test-main macro failure, preserved and
corrected; one mistaken clear-bit fixture assertion was corrected to190. An
existing route_worker interrupted without an assertion; unchanged standalone and
final canonical pass. A sandbox file-glob stall was preserved before canonical
escalation. No cause or product game failure is inferred.
Actual native/frame/audible response and gameplay remain unmeasured. No measured
speedup or application-lag claim follows from offline worker/controller timings.

All five source checkpoints are Closed. Combined smoke/resource/scan/guarded
installation and commit remain pending; the public1.0 artifact is unchanged.

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
