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
