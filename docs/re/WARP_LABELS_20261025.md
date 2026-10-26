# Pandaemonium warp labels, October25

Status: complete/offline verified. Native-label/baked-arc checks, full build/smoke
and exact-payload Defender scan pass. The [Ur/character follow-up](UR_CHARACTERS_20261025.md)
carries the correction forward and is installed/hash-verified all six after the
user's FFIII deploy reply. Earlier deferrals are history.
Gameplay/listening remains user-side; no game launch/control/runtime probe.

## Cause and result

FFII log85168..85221 repeatedly lists Scene trigger at the current tile and
another18 steps southwest, while route advice says step on the warp. This is
two correct-identity consumers with different evidence: routing already has
the extracted ChangeMap arc, while the label generator treated generic numbered
event scripts as unknown scenes. The names omit the explicit warp keyword.

Reviewed16 step-on pads:15 Pandaemonium occurrences in Map_30161_1..11 and the
Jade Passage connection in Map_30151_13. Pandaemonium's Jade return already had
the right name;15 occurrences were mislabeled through9 shared dictionary keys.
They now resolve the existing12-language Warp Tile label. Category/type remains
Event, because these are automatic step-on triggers. Manual objects, scene
triggers, traps, scripts/activation, exact entity identity, grouping and routes
remain unchanged. No destination, plot outcome or new undisplayed statistic is
added to the name. Both live and copied remote labels use event_labels.cpp.

## Evidence and implementation

`tools/event_warp_reviews.json` records exact native map/submap/group/editor ID,
entity ID, raw name, script ID/name, destination map/point and complete script
SHA256. The sources are FFII sc_e_0088 at Jade's entrance and sc_e_0201..0215
in Pandaemonium. Their Main sequence plays sound183 and the warp puppet, fades,
optionally updates Teleport return cache, then changes map and exits. The puppet
only animates the pad/hides the player. No battle, dialogue, item/stat/flag change
or conditional story action occurs. Full extracted scripts remain outside Git
at ../tools/mapdump/ff2_scripts.jsonl; native placements and masters are in the
same outside-Git mapdump folder. No script body/snippet enters the repository.

`event_label_identities.py::reviewed_warp_identity` checks the exact occurrence,
native activation/action/entity/script identity and entire script hash. Unknown
occurrences do not match; a changed reviewed script fails generation/checks for
fresh review. `audit_event_labels.py` applies this before neutral inference and
emits the existing eventlabels.json lookup format. All six tables/audits were
regenerated and compared with the pre-task copies: only9 FFII values change from
Scene trigger to Warp Tile and16 audit rows gain reviewed evidence. Other five
games' label/audit tables are identical. This is a scoped naming correction,
not a rule that every ChangeMap/one-way scene is a warp pad.

## Ownership, bounds and extension

Production C++ is unchanged. Existing live owner-thread property/label lookup and
owned-worker remote string lookup retain their full phase/lifetime/cancellation
contracts. No extra native read, hook, scan, queue, wait or text deduplication.
Only existing embedded label values change; runtime lookup/format cost and state
count are unchanged. All bakes, route graphs, sounds, settings and source mapping
identities are preserved. New similar pads require exact placement and script
evidence; never infer from a script number or shared sound alone.

## Checks and gameplay handoff

`event_warp_check.py` validates all16 native sources, aliases and actual runtime
lookup keys, rejects an unknown occurrence/changed script, and checks12 locales.
`native_cues_check.cpp` exercises production live/copy label functions for16 pads;
the worker branch rejects native reads. Its per-game JSON fixtures now start
fresh, matching production. `event_semantics_check.cpp` checks each actual baked
native Event and derived step-on Warp arc/destination, excluding traps/world exits.
Full all-six semantic/name/route checks remain mandatory.

Evidence/recovery: build_check/warp_labels_20261025/. Before tables are preserved
under before/ and the constrained difference report is label_delta.json. Final
original build/resource/security/deployment receipts are retained there. The later
combined receipt is build_check/save_paths_20261025/deploy_partial.json. Actual speech,
frame/cold native lookup latency and gameplay activation remain unmeasured; no
runtime performance guarantee follows from offline label tests.

Natural check after deployment: cycle Events on the reported Pandaemonium floor;
the pad underfoot and southwest pad should say Warp Tile, with the same category,
distance, selected identity and route advice. Check the Jade connection when
convenient. No need to replay story battles or search for all16 occurrences.
