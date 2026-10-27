# Grouped exit continuation, October 26

Status: complete, Closed; implemented, Sol-reviewed, offline verified and
installed/hash-verified all six. Combined delivery acceptance is recorded below.
Gameplay/native/frame/audible confirmations remain separate; public1.0 preserved.
All five scoped source checkpoints and local delivery are Closed.

User reports Map Exit Filter grouping loses a selected exit across floors when
multiple physical entries share its destination (e.g. World Map). Source path is
entity_filter.cpp::build_impl -> restore_selection -> entity_scanner.cpp filter
publication. Grouping first chooses nearest usable row per destination ID, then
unconditionally removes its siblings before exact pending-goal restoration.
Event grouping already protects continued physical identity; exit grouping does not.
The accepted fix pins the exact available remembered exit as group representative,
keeping one entry per destination. Once the route is cleared or manually changed,
normal shortest-usable representative selection resumes. Do not retarget to another
physical exit merely because it is nearer after a floor change.

Match the existing continued-goal contract: source map (live/current or remote),
verified cell, native entity ID when present including0, and raw identity when
present. No translated-name matching, new global deduplication, story/terrain
bypass, extra native observer/capture or route-state redesign. Preserve availability
and story filtering, event/pad grouping, audio's ungrouped physical list, cancellation,
all categories, sorting independence, settings/defaults and manual cancellation.
If the physical objective is absent, retain conservative no-match behavior; do not
bind its replacement story variant or invent a route.

The production filter uses a pure continued-row identity helper: live current-map
or remote source map, verified cell, native ID when nonnegative (including0),
and raw identity when nonempty. Before pruning siblings, the exact pending,
eligible exit wins as the sole representative for its destination ID. An
unreachable world row remains ineligible, so it cannot erase a usable sibling.
Indoor blocked explanations and existing manual-clear/manual-selection behavior
remain intact. No native read, hook, capture, timer, pointer/thread change,
persistent setting, string, or bake change was added. Worker inputs remain owned
snapshots; existing publication still checks exact pending identity and map
generation, and manual cancellation still invalidates stale work.

Offline evidence: `build_check/grouped_exit_continuation_20261026/before_filter_report.txt`
reproduces the two production-filter failures before the fix (grouped live handoff
and categories). `tools/entity_filter_check.cpp` covers remote-to-live identity,
distance changes, ID0, raw mismatch, absent/story/unavailable/unreachable targets,
sort modes, grouping off, preserved physical audio rows and cancellation.
`tools/entity_scanner_check.cpp` exercises capture through worker and owner
publication, including replacement of a remote objective with its live handle,
manual clear and stale map-generation publication. FFIII town77 bake siblings
at (16,31) and (17,31) target World1; the distance-flip fixture uses these real
identities/coordinates, not claimed live route measurements. The scanner fixture
also corrects FFIII context. Cancellation coverage performs 12,001 checks.

One 10,000-row offline elapsed sample changed from 7.920ms before to
6.446–8.906ms after across samples. This is not a controlled speedup or native
frame measurement. An earlier preserved canonical attempt stopped after FFII
`route_worker_check` at timing; the unchanged direct retry and final canonical
build passed, as recorded in final source acceptance below. Installation and
gameplay remain pending combined delivery. Native frame, audible-response and
live transition timings remain unmeasured.

Own log parts are preserved in build_check/grouped_exit_continuation_20261026/.
The Gysahl sequence at218616..218622 preserves a World Map destination with grouping
off; toggles appear218733/218734. The source town77/Map_20150 has many real boundary
GotoMap objects to World1/Point23, while pub80 reaches it through Town Point3 at8,15.
This illustrates sibling changes across a floor handoff; the exact user failure's
floor/route was not isolated from that log. The separate before/after production
filter report and actual-source worker/owner-publication fixtures above establish
the failure and exercise the correction without attributing that old log to it.

Gameplay handoff (after all baseline checks and authorized installation): enable
Group exits, select a remote World Map exit, cross floors and confirm the same
physical objective remains selected and routeable. Manually select another exit
and confirm the prior pin releases. Confirm the physical exit audio list remains
independent of grouping. Use ordinary progression; this check reveals no reward.
Implementation and offline regressions are complete; canonical checks, installation,
native/frame/audible measurements and gameplay confirmation remain pending. Sol owns
final integration and closure. This record does not authorize game launch/control/
probe, installation, publication or a closed-system refactor.


Implementation contract clarification: an unreachable world row remains ineligible
for pinning under the existing world route-distance guard. It must not replace a
usable sibling and then prune the whole group. Indoor blocked rows retain the
existing explanation behavior. Terra owns the bounded pure-filter and actual
owner-publication regression package; no production pointer/capture change was needed.

## Final source acceptance

Sol reviewed Terra's implementation and actual filter/scanner fixtures, and Luna's
three-file documentation. Final canonical build exit0:8287 names,0 missing; wiring
141 used/348 defined/0 undefined. Filter/scanner0 failures, wrong-owner spy0. The
owner handoff runs in FFIII context and restores the fixture default afterward.
Explicit grouped handoff in each other game remains untested; the shared path
and full canonical all-six regressions passed. Final10,000-row worker3.520ms;
100 rapid obsolete requests retire100/100 in2.355ms aggregate. Neither this nor
the uncontrolled7.920ms baseline establishes native lag or a speedup. Native
frame/audible and gameplay checks remain pending. The first canonical stop has
no captured process exit; unchanged direct retry and subsequent canonicals pass.
No cause is inferred. Evidence/recovery: build_check/grouped_exit_continuation_20261026/checkpoint.json.
The four source checkpoints are Closed; next active task is the requested chocobo
circuit, with no prize disclosure. Combined delivery is still open.

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
