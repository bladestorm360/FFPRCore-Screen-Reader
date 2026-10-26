# Locked-door interaction reachability, October26

Status: implementation complete, offline verified and installed/hash-verified
all six. The user requires a locked door to remain reachable for examination
without a key or unlock ability. A separate obstacle preventing approach may
still justify Story gated. New gameplay confirmation remains pending.

## Exact cause and native evidence

FFIII Castle Argus map67/Map_20130 contains manual key-door entities30 and29,
editor identities70 and71, at baked cells(7,7) and(31,7). Raw map tiles have
y8 before the established bake coordinate conversion. Both are native type5,
action2, solid colliders with scripts550 and551. Each shares its tile with an
ordinary GotoMap, leading to room maps73 and74 respectively. The native object
identifies a Magic Key door; the door is present and interactable while locked.

entity_nav.cpp::target_kind_of and entity_filter.cpp::categories_impl previously
used Infer for manual Event rows. baked_router.cpp::goals_for gives a collocated
transition priority under Infer. Selecting the Door therefore asked to enter the
blocked transition cell instead of reaching an adjacent examination position.
The story filter repeated that mistaken target, hiding an accessible Door when
Hide story-gated destinations was enabled. The Argus replay reproduces both old
failures as Story gated, then succeeds with the corrected interaction target.

The reviewed FFIII inventory contains21 actionable physical Door entities,
including the Argus pair and other key/story-locked doors. Native map/object/
script provenance, source hashes and factual entity records are retained in
build_check/locked_doors_20261026/native_door_facts.json. Source-bearing masters,
objects and scripts remain in parent tools/mapdump outside Git. No game source
snippet is reproduced here or copied into a repository.

## Implementation and safe extension

field/target_kind.h owns one small pure classification shared by selected route
requests and the batch story filter. Category Objects explicitly means Interact.
NPCs/chests retain Interact; Map Exits retain Transition; vehicles retain Spot.
Save Points and automatic Events retain Infer, preserving step-on warps, tutorial
overlaps and vehicle activation pads. The two consumers must use this helper
together so selection and optional filtering agree about a row's meaning.

An interaction goal uses the existing bounded route planner's legal adjacent
approaches and final facing. The door collider stays solid. The native Confirm
interaction still decides whether a key, ability or story condition permits
opening. Inventory, abilities and script state are never modified or bypassed.
The room beyond can remain blocked while the Door object is reachable. Separate
gates on the approach still prevent reachability and still qualify for filtering.

No binary bake, map-state record, transition, connected-floor edge or door identity
was changed. All six maps.bin hashes match the pre-change baseline. The existing
remote-return listing policy remains independent: it only changes which proven
reciprocal transition rows are presented remotely. This correction preserves the
meaning of a manual Object once presented, including on a connected floor.

Source: src/core/field/target_kind.h, entity_nav.cpp and entity_filter.cpp.
The helper keeps this policy cohesive without adding code to the large shared
router or refactoring unrelated responsibilities. No new setting, control or
user-facing string; existing localized names, guidance and Story gated text apply.

## Ownership, work bounds and verification

Classification is a constant-size enum branch over existing copied metadata.
No hook, timer, capture, native lookup, game-thread wait or pointer transfer was
added. Route and category work continue on their existing workers, using immutable
bakes, captured colliders, copied map state and existing cancellation/publication
guards. No text deduplication or native input behavior changed. The regression's
native-owner assertions pass. This is static/harness evidence, not a measurement
of engine capture, frame cost, audible response or tail latency during gameplay.

tools/locked_door_check.cpp runs in build.bat against actual shared planner and
filter code and all-six bakes. With each physical door's captured collider, the
fixture starts at a legal cardinal approach and requires a successful zero-step
interaction goal with facing, plus retention by the real story filter. The Argus
fixture also surrounds the approach with separate captured Collider19 obstacles
and requires Story gated, preventing a blanket door exemption.

| Game | Actionable Door/Iron gate candidates | Verified active approaches | Inactive/no cardinal approach |
| --- | ---: | ---: | ---: |
| I | 24 | 13 | 11 |
| II | 5 | 3 | 2 |
| III | 21 | 21 | 0 |
| IV | 29 | 19 | 10 |
| V | 23 | 20 | 3 |
| VI | 13 | 3 | 10 |

All21 FFIII candidates pass in the default closed-state projection; no key or
ability inventory is supplied to the planner. Shared checks cover79 active
approaches among115 candidates. The other36 are retained as explicit native-state
confirmation limits, not claimed tested in every plot state. Existing automatic
events/save-point/transition/gate/remote-floor and all other canonical tests pass.

The first fixture used the wrong filter API name; compilation failure is retained.
Its first separate-obstacle fixture used invented entity IDs that production
correctly ignored. The fixture was corrected to captured Collider19 semantics;
no production bypass was added. Both failure reports are preserved and no failing
intermediate DLL was installed.

## Delivery and remaining user check

Canonical8275 native-name checks/0 missing, wiring141 used/348 defined/0 undefined,
full offline checks and47-export smoke pass. All49 embedded data and18 sounds match
source, and all133 registrations remain documented. Exact22-file Defender scan
reports no threats/zero matching detections with unchanged hashes. Antivirus and
real-time protection enabled, current signatures1.461.211.0. Exclusion visibility
remains denied and no signing certificate is available; package/reputation and
other-product acceptance remain untested. See the [security record](../SECURITY_AND_DISTRIBUTION.md).

Installed all six using the guarded deploy script after the user's close
confirmation. All132 scanned payload hashes match and144 protected files retain
bytes/timestamps. DLL10983424 bytes, SHA256
574da7fc17ed8c8a0b2dcc7697566b0833370263f8b31f1bf61cd322da77aa2b.
Includes the earlier Warehouse, remote-return, status, shop and world-event fixes.
No game launch/control/probe, dependency change, delegation, upload or release.

When progression permits, select Objects/Door at Castle Argus without unlock
means: expect an adjacent facing route and the native locked-door response to
Confirm. Repeat with Hide story-gated destinations enabled; the reachable Door
must remain. An inaccessible approach should still gate. The room exit beyond
may remain blocked until the native lock opens. [Test procedure](../TESTING.md).

Evidence/recovery: build_check/locked_doors_20261026/, including native facts,
prior installed DLLs, initial failures, final reports, smoke, exact scan, source/
bake/resource hashes and deployment receipts. Gameplay/listening pending.
