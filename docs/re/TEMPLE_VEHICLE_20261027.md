# Temple of Water vehicle state

## Scope ledger

User-initiated October27 investigation; source complete, Closed. Determine whether
Temple of Water is entered aboard Enterprise or on foot, then correct movement
state and guidance only where native evidence requires it. Read the user's own
FFIII log and exact extracted native functions; no game launch/control/probe.
Keep native landing/entry/transportation ownership, route mechanics and controls.
Continent transition source is complete, Closed; other accepted subsystems stay
Closed. Libra/Study completeness and multiple-NPC pub-to-inn route oscillation are
separate queued source checkpoints.

Finite finish line: native entry/controller evidence, a before/after actual-source
regression if code needs correction, all relevant state/readiness/voice checks,
canonical build/smoke, current documentation and final combined delivery.
Research/implementation estimate30-45minutes; report unresolved contracts or an
overrun rather than silently broadening the feature.

Own log: build_check/warehouse_command_20261027/user_ff3_water_temple_session.log.
Map95 is Temple of Water; entering at251994.687, V says Enterprise at251995.515
and251996.328. Interior route evidence uses ordinary tile movement; this alone
does not prove the native transportation controller's semantics.

## Native behavior and information ownership

The Temple entrance script sc_map_20191_1 declares RidingVehicle=0. Maps95/96
are ordinary interior submaps, not a water world. Four Temple scripts explicitly
set that flag to0; the fifth leaves it unspecified. Exact scripts remain outside
Git in parent tools/mapdump/ff3_scripts.jsonl. The native FieldPlayer is constructed
for the map; native movement mode is independent of the remembered world craft.
Existing GetOn/GetOff hooks do not guarantee an edge when a new player is created.

Do not identify walking from the movement enum alone. The FFIII constructor at
RVA F188F0 builds the transportation table: Content(type5, automatic Canoe) and
Submarine(type6) also use native Walk(mode0). Ship uses3; ordinary airship uses2.
TransportationController.ChangeTransportation at466EC0 assigns its selected
currentTransport; get_CurrentTransportation at26C170 returns that owner. Its
TransportationInfo.StayMapId identifies the owning map. Native table/controller/
player functions and hashes are archived outside Git in parent
tools/temple_vehicle_20261027/{ff3,ff3-init,ff3-table}/. Original extractor mode:
tools/re/map_location_native.py --temple-vehicle [--temple-init|--temple-table].

tools/re/temple_vehicle.py verifies all-six FieldPlayer.moveState metadata and
Walk0/Dush1 values, plus the native transportation StayMapId getter; factual receipt
build_check/temple_vehicle_20261027/native_facts.json. No game source excerpts
are stored in this repository. Automatic canoe travel is real; landing Enterprise
on water leaves a ship, and later automatic canoe use is distinguished from
walking by actual current-map transport ownership. No native transport is changed.

## Implementation, lifetime and checks

navigation.cpp::player_movement_state reads only the current native-owned
FieldPlayer on its owner thread, using the name-resolved field. Missing metadata,
invalid owner, unreadable pointer or invalid enum stays unknown; failed reads
cannot turn into enum0. current_vehicle additionally copies native StayMapId into
VehicleState. No pointer crosses to a worker and no new native hook is added.

move_state.cpp::native_foot requires actual Walk/Dush and either no native
transport, the native Player row, or a retained transport owned by another map.
A current-map Canoe/Submarine keeps its proper vehicle name even in Walk mode.
V checks this ready state before using the boarding cache. Unknown/unready state
retains the existing fallback. On MapChanged a single next-frame callback reads
the constructed player after the native state-change stack unwinds. It queues
On foot once when replacing a prior riding state; a subsequent walking floor
cannot repeat it. A native riding state seeds its proper name without repeating
GetOn. MapChanging/BattleEntering generations retire obsolete work. Existing
localized Walking/Dashing/On foot strings cover all12 languages.

The initial candidate incorrectly treated every Walk mode as foot. Inspection of
the native transportation table, prompted by the user's canoe observation, caught
this before acceptance/deployment. It was replaced with the map-ownership rule;
the first build remains build_check/temple_vehicle_build.txt, not final proof.

The final actual-source vehicle_state_check covers cached Enterprise with absent
or other-map transport, walking/dashing, current sailing, same-map automatic
Canoe and Submarine, one deferred dismount, later walking floors, loading,
unknown owner, battle/transition cancellation and riding cache seeding. Final
canonical build passes8287 names/0 missing,142/349/0 wiring, all route checks
including route_worker0. Smoke47 passes. Candidate11059200bytes SHA256
ee3ee9f2eb1fde7c2eae010e774ba9af2388967c689441e1665cc66527b5557b.
Receipts: build_check/temple_vehicle_20261027/final_build.txt,
final_vehicle_report.txt and final_smoke.txt. No launch/deploy/gameplay confirmation.

Added work is one fixed field read per ready V/map-entry reconciliation, one
constant native StayMapId getter per existing current_vehicle observation, and at
most one pending observation for each native map-change event. No polling,
worker wait, route scan or queue growth per frame is added. Native/frame/audio
cost and actual map-entry listening remain unmeasured; no responsiveness claim
follows from these bounds. The cumulative25ms application-lag rule still applies.

Other source checkpoints stay Closed. Next active source work is the separately
requested Libra/Study display audit; multiple-NPC oscillation and the intermittent
EXP phase silence follow. Combined scan/install/gameplay remain pending.

Final static audit caught an incorrectly named ownership getter in the intermediate candidate. The exact native member is get_StayMapId, verified in all six dumps; the corrected final build and smoke pass. The intermediate pre_getter_audit_build/smoke receipts and their E5BA candidate are retained and are not accepted final native-contract proof. The new namespace-qualified refs require this direct all-six metadata audit because the general name parser does not count those declarations. No intermediate candidate was installed.
