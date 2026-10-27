# Repeated map-name components, October26

Status: complete, Closed; implemented, Sol-reviewed, offline verified and
installed/hash-verified all six. Combined delivery acceptance is recorded below.
Gameplay/native/frame/audible confirmations remain separate; public1.0 preserved.
All five scoped source checkpoints and local delivery are Closed.

## Scope ledger and ordered checkpoints

| Capability | State and boundary |
| --- | --- |
| Map labels | Source checkpoint Closed after canonical verification. Collapse identical resolved area/title fields when composing a map label; preserve distinct room/floor labels and repeated legitimate announcements. Shared current-map, entry, exit, route and remote consumers must agree. |
| Enterprise flight | Closed, installed. Native upgraded13/6 and16/15 conversions/landing rules; [evidence](ENTERPRISE_FLIGHT_20261026.md). |
| Chocobo circuit | Closed, installed. Mounted Shift+G native-rule loop and exact completion flag; prize withheld. [Evidence](CHOCOBO_CIRCUIT_20261026.md). |
| Gysahl pub Elder | Closed, installed. Inert Talk actor excluded without geometry change; [evidence](GYSAHL_ELDER_20261026.md). |
| Grouped exits | Closed, installed. Exact physical objective preserved across floors; [evidence](GROUPED_EXIT_CONTINUATION_20261026.md). |
| Earlier systems | Closed; release1.0/source0746410 and documentationd59a5d2 remain preserved. No release replacement, speech-wide deduplication, bake/route rewrite or other feature work is authorized here. |

Map-label checkpoint: exact producer evidence is established and the bounded source
change is implemented and reviewed. The canonical full build and offline suite pass under Sol;
8275 native names/0 missing,141/348/0 wiring,4944 retained floor combinations
and all new exact-part cases pass. Expected15-25 minutes for
the checkpoint including combined validation; report any overrun or new dependency.
Enterprise received a separate scoped design and estimate. Final combined
smoke/security/install passed after all five source tasks; gameplay remains
separate. No game launch/control/runtime probe.

## Established cause and baseline

The user's current FFIII log contains one speech submission saying Exit to Caanan
Caanan, Entering Caanan Caanan, Kazus Kazus, Ur Ur, Gysahl Gysahl and others.
This is duplicated content within a label, not two competing speech producers.
field/map_names.cpp::resolve_map_name_by_id resolves the area and the selected
authored/master/native-floor/numeric title, then unconditionally joins both.
Identical native area/title message keys therefore become the same name twice.

Factual all-six native map/area/English-message audit finds identical field values
in56 map-master candidates: FFIII52, FFIV4, other games0. These are master-data
candidates, not claims that every plot variant is currently active. One FFVI
title has distinct prefixed content and must remain intact. Generated floor and
authored room precedence remain unchanged. Source-bearing native masters stay
in parent tools/mapdump outside Git; this task's records contain factual IDs/
labels/hashes only. No native function capture is necessary to settle composition.

This user request authorizes removing the duplicated area/title component only;
it does not authorize text-content speech suppression, token/prefix heuristics,
case/whitespace normalization, scene-event guards or loss of a different title.
The existing resolver/cache/language lifecycle and thread ownership remain.

## Implemented correction and verification state

Terra changed only `src/core/field/map_names.cpp` and
`tools/map_names_check.cpp`; Sol reviewed the source diff. The map-label composer
now returns a nonempty resolved area once when area and title are exactly equal.
Distinct resolved labels and the existing title/fallback precedence remain intact.
This is equality between the existing localized components; it adds no native
reads, scan, hook, worker, timer, setting, control, string-catalog entry, resource
data or bake change. The existing localization/messages, resolver consumers,
cache behavior and language-switch behavior remain shared and unchanged. A cache
or language-cache redesign remains outside scope.

The existing `current_map_name()`, registered
`MapNameResolver.GetCurrentMapName` query, `map_exit_name()` and its cache, and
route/entry/remote consumers use this shared resolver. The correction affects the
composed label, not announcement cadence: repeated M requests must still speak.
There is no global speech deduplication, prefix/token/case/whitespace heuristic,
or new event guard.

The reviewed fixture additions cover equal text from the same and distinct keys,
repeated current/query/exit-cache reads, all six resource loops and twelve locales,
distinct prefixed-title behavior, title-only/empty/area-only cases, and retention of
the 4,944 native-floor combinations plus Ur well/shed/back-room and authored,
master, native and numeric precedence. The actual production resolver fixture and complete canonical
suite pass; the derived added assertion count is1152 across six roles/twelve locales.

Performance evidence is bounded: equality compares already-resolved localized
strings, and native lookup work and event cadence are unchanged. Whole-pipeline
native/audio cost, end-to-end usable/audible response and sustained application
lag have not been measured. No runtime timing or universal performance
certification is claimed; the cumulative mod-caused 25 ms blocker still applies.

Implementation/source review and canonical/offline verification: complete.
Source checkpoint Closed with checkpoint.json under the evidence directory;
combined delivery remains pending. Gameplay is separate. Terra implemented the
two-file source/test change without running tests; Sol reviewed it and ran the
full suite. Luna updated three documents from facts; Sol reviewed them and recorded
final results. No delegation has an unresolved implementation issue. Enterprise
flight is now active; chocobo circuit remains queued and prize-free. Evidence is under
`build_check/map_name_parts_20261026/`: `ff3_user_log.txt`,
`native_name_field_audit.json` and `source_before.json`. Native source-bearing
masters remain outside Git in `../tools/mapdump`; no game snippets are copied into
this documentation.

Evidence/recovery:build_check/map_name_parts_20261026/, including the user's own
log, native_name_field_audit.json and pre-change source hashes/copies. Scope checked
at start and before the bounded implementation handoff; no closed subsystem is
redesigned. Sol retains research/integration; Terra owns only the named coding slice.

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
