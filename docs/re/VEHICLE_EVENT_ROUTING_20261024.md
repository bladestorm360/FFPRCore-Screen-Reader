# Mounted vehicle-action event routing

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation/offline verification complete. Player reports
FFII Palamecia's landing event does not use the Airship although map-exit routes
do. No game probe is run. Existing source/data evidence is offline/read-only.

## Native map contract

FFII world Map_10010, group ev_e_0086 contains nine native type21
TransportationEventAction pads with script134/sc_e_0084, IDs49 and800..807,
raw editor rectangle(232..234,176..178), baked anchors one row above. The script disembarks and changes to map276/point1
with continuation sc_e_0084_1. The native initializer selects this group before
scenario flag79 completes. The event must be reached aboard a vehicle; ordinary
walking/landing is closed on its mountain terrain. The prior planner attempted
an ordinary ride/landing/walking area, so it rejected this event's real footprint.

Native Property.TargetTransportationIdList controls allowed model IDs. The bake
already preserves its comma-separated IDs before the raw label. Empty permits
any native vehicle; actual terrain still determines which can reach the pad.
All-six raw census: no type21 pads in I/III; II uses empty restrictions; IV uses
3/9/10; V uses4,6/11/12,7/19; VI uses4/5. Shared parsing retains those restrictions
rather than assuming every event accepts an Airship. Only observed current or
owned parked models can provide a route; route discovery never unlocks vehicles.

## Implementation and ownership

baked_nav.cpp defines mounted goals for active vehicle-action pads, instead of
requiring a walking cell. Full routes can board an owned parked allowed vehicle
or continue aboard the current one, then stop at the exact pad cell. They retain
native terrain/direction masks, hazards, story state, model identity, cancellation
and world wrap. Native landing/activation remains manual; the reviewed FFII134
pad adds the existing localized land instruction. No scripted dismount is executed
by the mod and no forbidden ordinary landing is invented.

Event-list eligibility shares mounted floods for reachable parked models and
applies allowed IDs per pad. Ordinary events also use the prepared riding-to-land
or parked-vehicle landing floods, as map exits already do. The reviewed Leviathan
Ship-entry route retains its special sailing footprint and native boarding edge.
No search range/distance cap is added. Worker snapshots own all computation;
native captures retain existing budgets and no worker receives native pointers.
baked_map.cpp prepares the sparse world-cell pad index on Data (clipped to map
bounds). Goal lookup uses that index without a callback map scan or native flag
read; active-state checks use the existing worker snapshot. Mounted reachability
floods are computed once per observed parked model per list generation and reused
across its pad rows, with cancellation between models/cells.

navigation_followup_check uses the actual FFII pad/master/terrain and initializer:
already-mounted and parked Airship routes, exact pad cell/manual landing output,
Current map with vehicles eligibility and missing-model rejection. Existing
Leviathan, world-exit, long-distance island and vehicle-model checks remain.
These execute production planners with owned real bakes and fake native owners;
live landing/script activation, audible response and game-frame cost are pending.
No new control, setting or language key. Evidence/recovery:
build_check/stat_effects_20261024/. Game source stays outside Git in
../tools/mapdump/ and native analysis folders.
