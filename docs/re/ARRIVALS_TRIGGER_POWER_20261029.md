# Native arrival selection and trigger power

Scope and closure belong to the [single audit checkpoint record](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
This topic records the existing producers, exact evidence and minimal integration
correction. No game was launched or probed. Gameplay remains separate.

## Arrival coordinates

All eight originally ambiguous door sets have valid native PointIns. The actual
III/V FieldController.MapSetup passes MapModel.GetEntitysDefault to
MapManager.SearchPointInEntity. Its nested predicates select ObjectType0 and
the requested ObjectId, without an active-group predicate. The terminal helper
returns the first match; duplicate markers do not imply a conditional arrival.
Native rectangle-center conversion and cropped bake origins agree.

| Door source | Native default arrival | Reason competing marker is not selected |
| --- | --- | --- |
| III Map_20220 actors1/2/3/21 to world4, PointIn11 | Entity27 at74,69,world layer1 | Default entities30/33 occur later; no Main player track changes the landing. |
| III Map_30231_10 actor3 to290, PointIn1 | Entity2 at35,8,Bottom | Entity0 at11,25 is the later duplicate. |
| V Map_20050 actor60 to81, PointIn1 | Entity4 at11,12,Ground | Story-group marker is outside the default lookup list. |
| V Map_20120 actor12 to133, PointIn1 | Entity1 at14,13,Ground | Entity9 is at the same cell. |
| V Map_30130 actor2 to world1, PointIn12 | Entity47 at204,85,world layer1 | Conditional duplicate is outside the default list; world layers normalize. |

The existing bake and PointIn resolver are correct for these cases. No arrival,
initializer, terrain, destination or map master was changed. Real source doors
have no transfer script. Main-segment CFG review of each destination finds no
player-position track or SysCall overriding the selected landing.

`tools/re/arrival_contexts.py` retains exact original objects/initializers outside
Git. `tools/arrival_choice_check.cpp` exercises the production PointIn selector
and a fresh owned arrival through plan_snapshot: eight door identities, all-zero,
all-one and unknown flags,24 exact choices and24 zero-step continuations pass.
This proves arrival selection, not complete story traversal from every state.

Native evidence is outside Git under `../tools/conditional_arrivals_20261029/`:
contexts.json and ff3/ff5 native/provenance/caller archives. SearchPointInEntity
RVAs are III2B4260/V2A95A0; terminal helpers III9BCF60/V95CCF0 use their exact
x64 runtime-function spans. Exporter: `tools/re/map_location_native.py
N --arrival-selection`. Generic helpers are not bounded by an unrelated next
dump symbol. Original game code never belongs in this repository.

## Power is current native state

Across I–VI EventTriggerEntity, EntityOverlap disables power; SetProperty and
Suspend clear power/overlap. Reset and OnTriggerExit restore power and clear
overlap. Hide calls the parent Hide and dispatches virtual Reset, so hiding a
vehicle-inapplicable actor does not permanently consume its trigger. Native
power offsets are128 in I/II/IV,130 in III/V and148 in VI (hex). Production uses
resolved fields/classes, not these hardcoded offsets.

FFVI EventSwitchTriggerEntity inherits EventTriggerEntity. Its EntityOverlap
and Suspend override virtual slots58/59. Pressed/leave-pending flags and a
LeaveScriptId distinguish press and release; OnTriggerExit rearms power and
sets leave state, and LeaveTriggerFinished clears that state. Do not replace
this lifecycle with a persistent mod-side consumed flag or globally revive
unpowered actors. The native actor owns timing, including party members holding
pressure switches. Current snapshot capture observes ready state only.

Existing native archives: `../tools/map_transport_audit_20261028/
ffN_trigger_lifetime/`. Additional exact VI press/leave/property bodies and
provenance: `../tools/conditional_arrivals_20261029/ff6_switch_power/`, exported
by `tools/re/map_location_native.py 6 --switch-power`. Dump/native inheritance
and named field ownership are checked; no new hook or native invocation added.

## Actual SwitchEvent omission and correction

There are17 real VI SwitchEvent30 records, in Map_30781_1 (9), Map_30871_23 (3),
Map_30871_28 (3), Map_30871_7 (2). All have press/release scripts and unrestricted
transportation lists; I–V have none. They were already baked and admitted by the
native EventTriggerDetector and remote collector. But capture_obstacle accepted
only Event2/RandomEvent28, and baked_route::step_event omitted SwitchEvent30.
Thus live and future-floor routes could cross these physical scripts without
the existing event-boundary treatment.

The minimal correction admits30 in those two existing predicates and matches
live source identity using its actual captured type. It preserves native power,
the exact source ID/name/cell, existing hazard/terminal semantics and route bounds.
No entity is synthesized, no script/state/bake changed, and no movement or
press/release action is executed by this patch. Destination detector/category
and accepted routing design are unchanged.

`tools/transport_guard_check.cpp` uses every real switch bake record, native-like
properties and named-field reads, then the actual live_blockers and remote
map_state::add_obstacles consumers. Before:0 powered captures/0 remote hazards,
86 failed assertions. After:17 switches/34 powered-or-rearmed captures/17 remote
hazards, unpowered controls absent,0 failures. Native-owner violations0. Each
fresh observation carries its own power; worker code receives only owned values.
Existing all-six selector, malformed-list and future-vehicle controls pass.

Retained plans already require the current vehicle identity and validate remaining
edges against fresh rules. still_current rejects publication after a map, start
cell or vehicle change. This patch adds no vehicle invalidation hook and no
path-dependent power simulation. Story consumption remains initializer/script
evidence, distinct from temporary native power.

## Bounds, verification and extension

The capture already visits these actors. Added work is the existing class/scenery/
power observation for at most the17 actual VI switch records; existing entity,
slice, selector and cancellation bounds remain authoritative. There is no new
loop, retained native pointer, game-thread search/wait or worker-native access.
Owned test timing includes file/preparation/thread-host overhead; it does not
certify cumulative game-frame lag or audible response. The user’s cumulative
25ms application-lag rule still applies.

Reports: `build_check/trigger_power_20261029/{before,after,arrival}.txt`;
canonical build now runs the source-context, arrival-choice and transportation
guard checks. Final shared regression/canonical/security/delivery status belongs
to the single checkpoint record. Installed FFIII b61153fc remains unchanged.
Future extensions must preserve exact native subclass/ID ownership and distinguish
temporary power from consumption; do not widen event types by guessing names.
