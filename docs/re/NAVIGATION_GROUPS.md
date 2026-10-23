# Scene triggers and entrance labels

October23 follow-up: [current navigation contracts](NAVIGATION_FOLLOWUPS_2026_10_23.md)
add independent default-Off StoryGateFilter (Ctrl+Shift+Backslash/F8 Navigation),
Save Points/category6 and native-position handling for moved one-tile triggers.
Event grouping remains independent; category iteration uses kCategoryCount.

October22 extension: `field/navigation_labels.cpp` now groups physical exits by
copied localized destination name, across destination IDs and floor/non-floor cases.
Native exits missing a baked GotoMap get a naming-only physical footprint from their
captured map/layer/tile/native identity. This never edits route geometry. Adjacent
tiles join only when they represent the same doorway/arrival PointIn. Stable layer,
y,x anchors assign uppercase(A)..(Z),(AA).. before distance/category filters. Both
live and remote rows use the shared preparation and selected names feed directions.
Existing EntranceSuffixes remains default Off; saved values are preserved.
All-six actual-bake grouping checks cover same-name distinct IDs, unknown native
exits and reversed row order. Exact six-Coliseum gameplay confirmation is pending.
[Combined verification](../FIXES_2026_10_22.md). The requirement below is the original report.


October 21: implementation and offline verification complete. The user reports
substantial improvement in a brief FF6 intro test; later states and other games
remain unconfirmed. Applies to the shared I–VI core.

## Open requirement: letters for all ambiguous same-name exits, October 22

**Status: not started; audit and implementation deferred.** User reports six
distinct navigation entries with the same destination name:

> Exit to Coliseum (1.0 step North) - Map Exit (1 of 6)
>
> Exit to Coliseum (6.7 steps Southwest) - Map Exit (2 of 6)
>
> Exit to Coliseum (7.3 steps South) - Map Exit (3 of 6)
>
> Exit to Coliseum (8.1 steps Northeast) - Map Exit (4 of 6)
>
> Exit to Coliseum (8.5 steps West) - Map Exit (5 of 6)
>
> Exit to Coliseum (14.3 steps West) - Map Exit (6 of 6)

The user requests destination letters such as **Coliseum (A)**, **Coliseum (B)**,
through **Coliseum (F)** so each physical exit is identifiable. Preserve the game's
localized destination name and ordinary exit wording. These examples do not assign
permanent letters according to the current distance-sorted list.

This is a general requirement for **all similar ambiguous same-name exits across
FF1–FF6**, not a Coliseum-only override and not limited to multiple floor entrances.
The existing entrance-suffix feature described below must be audited for gaps in
eligibility, grouping and application through shared and game-specific paths.
The October21 completed implementation/offline checks do not verify this new scope.

Future work should extend the existing lettering mechanism/settings as appropriate,
assign stable letters to distinct physical exits before distance/sort/filter changes,
and retain a single identity for the tiles of one doorway. Distinguish exits even
when their displayed destination names match; do not merge their actual coordinates,
route goals or destination identities. Keep letters consistent wherever those exit
names are presented, including navigation listings and direction requests. Preserve
existing preferences and route validity; no setting/default change is requested here.

Future checks: reproduce the six-entry case, cover other same-name destinations in
every applicable game, verify stable letters after movement/reordering/filter changes,
and verify that selecting each letter still routes to its own physical exit. Include
multi-tile doorways, floor and non-floor transitions, remote labels and relevant
grouping settings. Record genuinely inapplicable cases and missing evidence.

Documentation only at the user's request; no code audit, implementation, build,
deployment or runtime probe performed. Exact map/exit identities and root cause are
not independently established; offline and gameplay validation remain pending.

## Evidence and behavior

The user's FF6 game-root log records Narshe map3 at cell38,49, layer1.
The planner succeeds at reaching scene triggers (including38,50 in one step),
but list browsing then reports Not directly reachable. `plan_browse_snapshot`
rechecks live events without excluding the selected destination, unlike `find`.
The correction keeps the selected event usable while retaining warnings for
earlier events, traps, vehicle actions and required floor changes.

Narshe's Map_20010 bake contains118 Magitek collision objects in ev_e_0003,
plus the default rock-wall collider. FF6 `FieldColliderEntity.CanEnter`,
RVA FE2B90, was inspected offline in the installed native image. Enabled
collision modes6/7 block one movement axis; other modes block entry. Existing
classification matches. There is no basis to delete the intro's real barriers.
Raw disassembly is outside Git at ../tools/narshe_20261021/.

If a requested destination really is story gated, the worker offers directions
to the nearest reachable active scene trigger when one exists. This uses the
same live gates, NPCs, directional edges and trap exclusions. It identifies a
possible action and does not claim to know the event's story outcome.

## Controls and ownership

- Navigation → Event group filter; Control+Shift+P on the field. Persistent
  EventGroupFilter, default off, registered in the existing remapping catalog.
- Speech → Entrance suffixes. Persistent EntranceSuffixes, default off.

`entity_scanner::apply_filter` copies owned list rows in bounded owner slices.
When event grouping is enabled, the existing obstacle snapshot is also captured
indoors. Native pointers remain in capture closures; the Navigation worker sees
owned positions/identifiers, copied state and immutable bakes only.

`navigation_labels` derives event identities from exact map/entity/raw-name/cell
matches. A group requires the same source floor/layer, native object type,
entity group, script, action and raw identifier. Unknown/ambiguous identities and
interactive targets remain separate. Display labels are never identity keys.
The shared flood ranks usable targets; one representative remains when all are
blocked so the explanation remains available. Sorting and grouping are independent.
An explicitly remembered objective is retained alongside the nearest representative
when necessary, rather than rebound to a different trigger during continuation.

Entrance letters are assigned before distance/category/exit filters from active
baked GotoMap footprints. Adjacent tiles with the same source layer, destination
floor and destination PointIn form one physical doorway. Disconnected doors
remain distinct, including doors that share a PointIn. Components are ordered
by layer, north-to-south, west-to-east and PointIn, independent of player position.
Only destinations with multiple doorways get (a), (b), …, (z), (aa), ….
The source floor scopes letters; remote entries retain their floor context.
Reapplication starts from converted_name so toggling never stacks suffixes.
Native identity, saved goals, beacon geometry and destination IDs are unchanged.

The label helper is a separate worker responsibility. The existing large
baked_nav.cpp remains the cohesive native-snapshot/route integration boundary;
this change does not split unrelated planner state or introduce another owner.

## Verification and safe extension

`tools/navigation_groups_check.cpp` runs the actual browse planner and list worker
against all six shipped bakes. Results from the final canonical build:

| Game | Direct selected-event fixtures | Eligible event objects inspected | Entrance rows with letters |
| --- | ---: | ---: | ---: |
| I | 42 | 42 | 417 |
| II | 112 | 118 | 438 |
| III | 94 | 1,699 | 118 |
| IV | 146 | 149 | 171 |
| V | 864 | 937 | 330 |
| VI | 1,175 | 1,188 | 936 |

All 638 FF6 bakes were inspected. In total 2,433 real event routes agree between
full planning and browsing, with zero native-worker calls. The remaining inspected
objects did not supply a direct local walking fixture; this does not certify them
or every live story state. Fixtures do not replace gameplay confirmation.

An independent FF6 rebake from the extraction records, with grid verification,
reproduced the shipped 834,011-byte bake exactly: 13,078 objects, 666 warp arcs,
22 round-trip cutscenes retained as ordinary events, and 657,695 collision cells.
SHA256: 3C8866D03398F4D6A310841E7499F562E4502B8FF20D4288E96B29AA49F957AC.
The rebake is outside Git at ../tools/narshe_20261021/rebake/ff6/maps.bin.
No production bake was changed.

The Narshe fixture retains all 119 gates. It verifies the one-step scene trigger,
the school exit's truthful gate plus `1 South` trigger hint, and no invented hint
when no trigger exists. The production grouping flood accepts the selected event
edge and rejects a target behind another event. All 29 native script1180 border
triggers collapse to the usable representative; distinct scripts stay separate.
An explicitly remembered native target survives regrouping.

All-six entrance projections remain identical after reversing list order and moving
the player. The real Deist floors supply 22 labelled entrance rows. Source layers
remain distinct. Existing trap, cross-floor detour, exact exit and cancellation
checks pass; selecting a trap still warns instead of claiming event-free distance.
`modal_input_check` verifies both submenu placements, independent saved/default-off
toggles and the existing first Game speech row. `transition_input_check` dispatches
Control+Shift+P in both layouts, preserving the other P chords.

The full canonical build/all checks pass: 6,864 native names, zero missing;
119 used actions/326 defined, zero undefined; isolated smoke resolves all 47 exports.
Local DLL: 9,767,936 bytes, SHA256
E9A7392A4324F288B31397433F61E437BD3D49FD974AB22BA8C792306F981BDA.
Receipts: build_check/navigation_groups_2026_10_21/. After the user requested
deployment on October 21, deploy.bat installed this tested build in all six closed
games. Every installed DLL hash/size matches; SDL matches the bundled payload.
Settings, waypoint files and all private dependencies (including Prism/OpenAL)
retain their prior hashes and timestamps. Previous hid/SDL binaries are retained
in the receipt folder's timestamped predeploy directory; deployment_verified.json
records all six targets. No game launch/control/probe, commit/push or publication
was performed. This deployment also includes the earlier results/logging diagnostics.

Luna (gpt-6-luna, medium) supplied the eight menu/help/hint entries in 12 languages
in a bounded catalog-only task and follow-up. Sol owned design, research, code,
review and testing; independently verified exact English keys, locale sets,
placeholder retention and unchanged pre-existing catalog entries. No outstanding
delegated work or catalog validation issues remain. Linguistic gameplay review
remains distinct from structural catalog validation.

Extend event grouping only from proven native identity. Preserve unknown rows,
consumed-state guards, cancellation, exact exit coordinates and real gates.
The progress hint is one flood plus at most one A* per blocked request, bounded
by the copied trigger count and map grid, with existing request cancellation.
Labels use map/floor caches per preparation and a union of bounded map footprints;
no additional engine traversal or wait was added to a native callback. Indoor
grouping uses the existing count/time-limited obstacle capture. Remote target
ranking reuses the existing cancellable cross-floor planner.

Offline paired full/browse computation peaked at 7.765 ms in the final event
corpus. Existing 2,000-row capture/publication completed in three fixture ticks;
that fixture keeps the new toggles off and is a baseline, not a measurement of
new-feature native cost. The real-engine capture cost, large multi-floor grouping
tail/queue delays, audible response and snapshot age remain unmeasured. Do not
claim responsiveness from worker placement or this compute-only result.
Actual native event enablement, later story states, manual movement and audible
delivery remain the prioritized gameplay checks in docs/TESTING.md.
