# World event availability and vehicle advice, October26

Status: implementation complete, offline verified and installed/hash-verified
all six games with the [combined status payload](ALLY_STATUS_20261026.md).
Gameplay confirmation remains pending. No game launch/control/probe.

## Exact entries and native requirements

The user's FFIII log identifies map1/Map_10010 event2368 at(86,25), the zero-step
scene trigger, and event2367 at(87,91), the Requires The Enterprise entry.
Tiled sprite anchors are one tile below the baked trigger coordinates.

| Object/editor ID | Native identity | Actual requirement/purpose | Correction |
| --- | --- | --- | --- |
| 2368/5426 | 世界一周, script380/sc_e_0286 | Chocobo transportation12 only; sets ScenarioFlag1/192 then exits, no scene/travel/interaction | Remove passive bookkeeping destination/sound |
| 2367/5422 | トーザスの抜け道, script379/sc_e_0342_1 | Foot transportation1; all occupied active party members Mini, then travel map168; otherwise exit | Native Tozus Tunnel name and exact Mini guard |

The first marker does nothing by standing there on foot and is not a meaningful
scene destination. The second is unrelated to Enterprise ownership. Its remote
route also received misleading hypothetical vehicle advice: ownership does not
prove usable boarding/landing. FFIII's installed StoryGateFilter=false is preserved:
direct known-inactive event filtering is independent of optional reachability
filtering, so no setting/default change is needed.

Current.CheckChild0x91B400 delegates CheckConditionCommon(401)0x91B450.
These write MapLocal0 and are never called by the mod. Recovered
External.Corps.CorpsController0x92ACC0 enumerates current corps, skips empty IDs,
resolves corresponding owned characters and tests their current conditions.
The predicate starts true and becomes false for any active member missing401;
inactive owned characters do not participate. Native get_Id0x668E80, parameter0x78,
current-condition list0x88 are factual build locators; production resolves names.

## Production data flow and controls

map_state.cpp::snapshot copies optional FFIII-only party_mini through read-only
getters. Limits: corps1..9, owned1..16, conditions0..128 per member, total condition
budget512. Missing/uninitialized/malformed data or no active member is unknown.
This adds bounded reads to the existing snapshot, no new polling loop or script call.

map_state.h::Flags carries the copied optional scalar. The existing immutable
snapshot/Navigation worker owns it; native lists/condition/character pointers
never cross to workers. map_state_program.cpp adds navigation category14/index401
and includes presence/value in state signatures. Form changes invalidate reused
plans; existing generation/cancellation rules remain.

tools/bake_map_state.py recognizes only the exact reviewed FFIII Mini SysCall,
copies its result to private projected map-local0, then continues the existing
branch/Exit analysis. It never executes or changes a game script. Only a proven
preceding Mini query allows that map-local predicate in an entry guard; unrelated
unsupported calls/locals stay conservative. FFIII has295 initializer records and
3 exact entry guards. Resolver.consumed removes Tozus only when known false;
true/unknown retain it. Live/remote filtering and routing use the same resolver.

tools/world_event_reviews.json pins both occurrences, native IDs, names and full
script hashes. event_label_identities.py validates these before generation.
Passive bookkeeping becomes @route-only; event_labels.cpp::event_route_only filters
it before live/baked navigation and sound publication, preserving native routing
geometry. Defensive formatting uses Event trigger if reached early; the marker is
never spoken. Tozus reuses the existing game-specific12-language name catalog,
Japanese native name and established English fallback. No new control/string.

baked_nav.cpp::vehicle_required is shared by all six games. It excludes owned
auto-vehicle, current mounted and parked owned-vehicle groups before hypothetical missing-vehicle
reachability. Already-owned boarding/landing failures cannot justify Requires The
Enterprise. An unowned hypothetical vehicle keeps existing generic wording;
genuine route search, transportation rules, native flags and movement remain.

## Offline checks, performance and confirmation

mapstate_native_check exercises the actual reader: all active Mini versus one
normal member, inactive owned members, missing data/count bounds, other-game
exclusion and changed signatures. mapstate_check projects the real Tozus guard
false/true/unknown. chocobo_owen_check exercises the actual diagnostic with every
native group owned and requires no missing-vehicle message. entity_scanner_check
verifies1525 removed entries:1524 routine borders plus the passive lap marker,
while retaining real scenes. Full all-six route/event/story/localization checks pass.

Comparable complete fake-native snapshot timings,100 requests: no party input
0.281ms aggregate/0.005ms peak; two active Mini members0.385ms/0.005ms.
These include existing full flag copying. Real native frame cost, route publication
tail/data age and audible latency remain unmeasured; bounded workers alone do not
establish those costs.

Source-bearing evidence remains outside Git: parent tools/mapdump and
tools/world_event_conditions_20261026/ff3/native.txt with DLL/dump hashes.
Earlier extraction is retained as native_before_corps/provenance_before_corps.
Original extractor mode --world-conditions is in tools/re/map_location_native.py.
No game source/script snippets enter this repository. Recovery starts at
build_check/world_events_20261026/; final combined checks/scan/install receipts
are in build_check/ally_status_20261026/. All132 staged-installed hashes and144
protected files verified; exact payload hash/security limits are in the combined
status record.

During normal FFIII progression confirm the passive scene disappears, Tozus is
absent with a normal-sized active member and available with the entire active
party Mini, and owned Enterprise no longer produces missing-ship advice. Unknown
gates and unrelated future scripts remain conservative. Extend only with exact
native semantics and real-data tests. [Gameplay steps](../TESTING.md) require no
user code/architecture review.


## October27 continent travel: scope ledger

User-requested scope: restore one nearest reachable unlocked airship transition in each direction for Map Exits, with an airborne terminal route and ordinary owned-vehicle boarding/takeoff when existing policy permits. Do not restore the full rough 1,535 border-trigger population or the passive Chocobo lap marker. The native producer omission was confirmed: live `goto_map_detector` skipped reviewed World map transition metadata, while ordinary world goals intentionally become foot/landing destinations. The correction adds a typed airborne travel goal, preserving ordinary town-exit behavior. Other games have no FFIII floating-continent mechanic.

### Exact native families and coordinate contract

The seven reviewed event families remain exact-data gated; native object/script evidence is outside Git at parent `tools/continent_transition_20261027/`, with game data under parent `tools/mapdump/ff3_objects.jsonl` and `ff3_scripts.jsonl`. Full hashes, IDs, script IDs, map IDs, counts and cells are recorded in `build_check/continent_transitions_20261027/native_contract.json`:

- Map1 Enterprise outward: script341, transportation ID6,508 routine-border copies; known ScenarioFlag1/66 false selects map2, true map4, unknown leaves destination unspecified.
- Map1 other outward families: script345, IDs8/9,508 copies; script349, ID11,508 copies; each targets map4.
- Map2 return: script356, ID15, one event at canonical baked cell(33,216), targets map1.
- Map4 return families: script343 ID6, script347 IDs8/9, script351 ID10; each has five-cell cross footprint at canonical center(33,217), target map1.

The native event properties authorize these precise transportation groups; candidate selection also checks active event/vehicle eligibility and reachability. Tiled gid cells anchor at the bottom-left: `bake_maps.tile_row` subtracts tile height and map-bounds origin. Tests therefore derive their endpoints from the matching baked Event, not raw Tiled `ty`. The initial `before_goal` and first typed endpoint report used raw map2(33,217) rather than canonical(33,216); those endpoint assertions are invalid and retained. They do not invalidate the separate native producer-omission finding. Canonical endpoint evidence is the later actual-bake test.

### Implementation and state flow

Shared `field/airship_transitions.h/.cpp` holds the native-free seven-family resolver, gated to FFIII factual data; core has no per-game engine classes. `TargetKind` appends the typed airship transition. `baked_nav::air_transition_target` runs on the existing Navigation worker, selects one eligible active baked Event and prefers the continued/selected exact physical identity before shortest reachable eligible target. It consumes copied `ExitSnapshot` state, family masks and owned paired-vehicle data; Enterprise pairs are13→6 and16→15. Known ScenarioFlag66 false/true resolves map2/map4; unknown returns no guessed destination. Parked aircraft remain allowed only through existing policy. No native pointer reaches the worker.

`entity_scanner` suppresses the16 exact native return actors and preserves the prior1,524 outward-border suppression. It publishes one pointer-free `local_baked` map/generation entity row through All, Exits and audio flows; native ID0 recreation/selection is covered. `world_exit` remains true even when a destination is unknown/zero. The existing four-state search accepts an optional `final_air` mode for this typed goal. Aerial-only goals keep `goals_open=false` so flight terrain stays eligible; ordinary town foot goals are unchanged. Existing landing design, native actions and route phase ownership remain intact. No script execution, flag/vehicle writes, input path, new hook or bake change was added. Labels “Surface World” and “Floating Continent” are present in all12 locales; no language policy changed.

### Source acceptance and limits

Root reviewed and accepted the implementation; source checkpoint is complete, Closed. Final canonical build `build_check/continent_transitions_20261027/final_build.txt` passes:8,287 native-name checks/0 missing,142 used/349 defined/0 undefined. `final_goal_report.txt` and `final_scanner_report.txt` each report0 failures. DLL is11,055,616 bytes, SHA256 `8c49dd6d57f11e31918c1385554a009cfbf2571416bcb69e330bcb38566cdb79`. The final actual-bake map2 test from(33,215) fails as ordinary Transition but succeeds as typed one-step goal to canonical(33,216), final vehicle15, with no landing. Map4 current aircraft6/8/9/10 and ship13 selected routes pass; map1 current6/8/9/11/13 worker target checks pass, and parked13 boards/takes off to final aircraft6. Wrong craft, cancellation, explicit FFII representative and FFIII game guard, and preservation of a distinct continued pin are covered. The full offline report and final build log are the evidence; no proprietary source excerpts belong here.

Earlier invalid evidence retained: raw map2 coordinate proof and first typed endpoint report; ninth fixture resolver-assignment compile failure, corrected to independent snapshots; an initial mounted/parked aerial-goal `goals_open` failure, corrected with the new eligibility evidence. The tenth verification attempt intermittently stopped in the existing `route_worker` fixture with stack overflow; it is retained separately. The eighth full pass and final full canonical pass both succeed, superseding neither the recorded failure history nor the current offline-only limits. The FFIII game-ID guard and explicit FFII exclusion check do not constitute a five-game runtime test. The failure history and receipts remain under `build_check/continent_transitions_20261027/`.

Offline aggregate worker measurements: ten evaluations total55.145ms on map1 and205.165ms on map4; the latter includes five selected plans. The worker evaluates one flood per eligible aircraft, not per candidate/NPC; each object uses a single four-neighbor pass, with cancellation tokens across vehicle/object/search work. No hard cap exists beyond captured game assets. These are offline computation figures only; native callback/frame cost, queue backlog/tail and audible/user response are unmeasured. No cumulative responsiveness claim is made; the >25ms mod-caused application-lag threshold remains applicable.

The October27 source candidate is not deployed or gameplay-confirmed. Other parked-aircraft combinations, real gameplay and map transfers, frame/audio response and final combined delivery remain unverified. No game was launched or probed. Preserve the October26 installed baseline/history above as a separate record; this source checkpoint does not replace it.

### Closed-system register and next authorized work

Continent transition source is Closed; no further refactor follows from review alone. Current active work moves to the separately authorized Temple of Water native boarding/vehicle-state investigation, followed by the Libra HP/weakness and Study displayed-information audit. The user’s later-reported multiple-NPC route oscillation is a separate queued checkpoint. Keep each scope independent and update this register only when its work is explicitly opened. Combined delivery and gameplay confirmation for this candidate remain pending.