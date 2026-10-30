# Animated interactive objects across floors

Human-priority follow-up, October29: Noah's Lute appeared only on its own Temple
of Time floor; Cave of Shadows Objects was empty until reaching the Fang floor.
Source/offline/FFIII delivery Closed: installed63a24c76/11341824 bytes. After the
running-game guard blocked, the human saved/closed and deployment verified22
installed files,144 protected files and the other five installations unchanged.
Recovery75de3594 retained. Full receipt: build_check/remote_objects_20261029/completion.json.
The broader NPC/map audit now resumes with its pending evidence. Sol only;
no game launch/control/probe or other-game deployment.

## Cause and exact evidence

The shared connected_objects_snapshot collector admits Entity5 but omitted
AnimEntity10. Both reported items use the latter and were already correctly
baked. Current-floor native interaction discovery has a different producer,
which explains why reaching their actual floor restored them. The newest own
FFIII log confirms Fang in Objects on map290, at448114.875 and448117.578. Earlier
entries report No Objects found. Its current route already reaches the Fang
approach(11,24,L0); no route/grid change is needed.

| Object | Native source | Master map | Native ID | Owned cell | Availability |
| --- | --- | --- | --- | --- | --- |
| Lute | Map_30171_11, ev_e_0080, editor6, AnimEntity10, script134/sc_e_0080 | 252 | 5 | L1(8,7) | scenario78 set,79 clear |
| Fang | Map_30231_11, ev_e_0092, editor8, AnimEntity10, script143/sc_e_0084 | 290 | 5 | L0(11,24) | scenario84 clear |

Both have native action2, real scripts, and baked flags13 (collision, script,
action). Source artifacts and the own-log excerpt/provenance stay outside Git
under ../tools/remote_objects_20261029/. Only the six own rolling FFIII logs were
searched; the retained Lute mention is inventory text, not a surviving Temple
object-list trace. Code/real-data reproduction, not that inventory entry,
establishes the Lute cause. No tester logs were read.

## Correction and ownership

One additional enum case admits actionable AnimEntity through the existing
Interactive Object path. The identical script/action guard excludes decoration.
State.active/consumed, hidden/collected items, real connected floors, bounds,
current-floor ownership, return-exit policy and existing category filters retain
their established behavior. No gate, map bake, route algorithm or item name is
changed. The fix is shared by all six games.

The existing Navigation worker owns collection over immutable bakes and copied
state. RemoteBuilder captures floor/localization inputs on the native owner in
the existing0.5ms slices; build_remote uses owned values and the established
safe_event_label service on its worker. Published entities retain source type,
native ID, raw identity, floor, exact layer/cell and Objects category. Existing
selection/rebinding and object numbering remain in charge. There is no new hook,
native query, polling, input, wait, queue, cache, deduplication or mod string.
Existing Lute/Fang keys already have the12-language catalog coverage.

The source inventory contains2301 animated actors across all six games;88 have
an action/script (I16,II10,III10,IV15,V12,VI25). The other2213 do not satisfy the
unchanged action guard. This is an applicability inventory, not a claim that all
88 are simultaneously visible or all require remote publication.

## Acceptance

Actual-source remote_exit_check reproduces20 failures before the correction:
both items absent from each of their ten earlier connected floors. After:0.
It checks exact identity/layer/cell/category, collected flags, the Lute's native
prerequisite and no duplicate on the native-owned item floor. All-six existing
largest-graph, reciprocal-door and state-projection regressions remain passing.

Actual RemoteBuilder→build_remote checks use both real baked objects and the
real label reader/catalog. They retain Lute/Fang, Objects, remote map/ID/raw
identity and exact cell. The existing2000-row capture/publication, selection,
numbering, native-floor rebinding and cancellation checks pass with no native
worker calls. Evidence: build_check/remote_objects_20261029/before.txt,
after.txt and scanner.txt. Full canonical/offline/smoke/security/hash checks and
gameplay confirmation remain separate. Delivery is verified; listening/gameplay
confirmation remains user-side.

Canonical second run120.2s passes, as do isolated smoke,8458 native names,
142 used/349 defined/0 undefined text keys,67 exact resources,12 locales and
Defender's22-file exact payload scan. Hashes remain unchanged; exclusion status
was unavailable and no signing certificate exists. No game was launched/probed.
The first full serial rebuild took655.6s and exposed an old test that explicitly
prohibited all animated objectives. Its1213 failures represented the newly
admitted objects across repeated state/floor projections. The corrected test
requires every active actionable animation, excludes decorative animations and
checks selection/continuation in Objects. It passes2454 master roots and2523
unique projected event identities. This was a test-policy correction, not a
runtime failure. Evidence retains first_canonical_build.txt and the old report.

The candidate also preserves the separately checked bounded instruction-patrol
correction from NPC_GEOMETRY_CONTEXT_20261029.md. That audit remains open; this
human report authorizes the minimal object-inclusion repair, not a new navigation
refactor or a claim that every remaining map uncertainty is resolved.
