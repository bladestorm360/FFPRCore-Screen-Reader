# Gysahl pub Elder, October 26

Status: complete, Closed; implemented, Sol-reviewed, offline verified and
installed/hash-verified all six. Combined delivery acceptance is recorded below.
Gameplay/native/frame/audible confirmations remain separate; public1.0 preserved.
All five scoped source checkpoints and local delivery are Closed.

## Finding and accepted behavior

The user's log at `build_check/gysahl_elder_20261026/own_log_snapshot.txt`
records map 80 (`Map_20151_3`), player positions (10, 8) and (10, 11), and the
Elder at (7, 8) as not directly reachable. The production plan has two
interaction goal cells but no route. Repeated across-floor fallback work took
30 to 42 ms of worker elapsed time; this is not a measurement of game-thread
stall or application lag. Static source, map, collision and object evidence
show the Elder in a sealed four-cell pocket with no hidden-passage reference or
relay. Do not open walls or infer a usable interaction from appearance.

The Elder is entity 11 with `EventAction.Talk` (action 4), script ID 0, an empty
message key, status-change ID 0 and empty close-message key. The nearby girl is
entity 12 and has message-backed dialogue. The Elder's Talk dispatch cannot
produce text or change character status, so listing it as a reachable NPC
promises an interaction that does not exist. This is a discovery/classification
issue, not missing walkable terrain. Native evidence and the authored bundle
remain outside Git under `../tools/gysahl_elder_20261026`; provenance and exact
contracts are recorded in `build_check/gysahl_elder_20261026/`.

Native contract: `EventAction.Talk` at `271BA0` forwards `MessageKey` and
`ChangeCharacterStatusId`. `EventTalkTask.MoveNext` at `29D530` passes the key
to `MessageWindowClient.Play`. `MessageManager.GetMessage` at `2C0FF0` checks
for null or empty before lookup and returns `String.Empty`; there is no actor
name fallback. The matching all-six `GetMessage` bodies and field availability
are recorded in `native_message_contract.json`. Both optional `PropertyTalk`
fields exist in all six matching dumps. No quest reward dialogue was inspected.

## Implementation and data flow

The shared native-owner predicate is `npc_navigation_available` in
`src/core/field/field_internal.h`. It retains the established empty-RunScript
classification and additionally rejects only a confirmed inert Talk actor:
exact action 4, script ID 0, confirmed `PropertyTalk`, all required field
references resolved, empty message key, status-change ID 0 and empty close
message key. Missing or unknown metadata conservatively retains the actor. The
predicate is used by live discovery in `entity_scanner.cpp` and selected-target
lifetime validation in `navigable_entity.cpp`, so a cached objective cannot
outlive its native actionability. The two named references are declared in
`field_internal.h` and resolved in `field_refs.cpp`.

The baker in `tools/bake_maps.py` applies the matching authored-data predicate
and removes only the NPC/ShopNPC discovery bit 8. It preserves collision bit 1,
script bit 4, movement bit 16, layers, geometry, entity identity and exits. The
byte comparison reports exactly 104 one-bit bytes removed: FF1 0, FF2 13, FF3
25, FF4 25, FF5 16 and FF6 25. FF1 is identical; the five additional FFIV
blank-Talk rows with nonzero scripts remain. On the pub map, entity 11's
collision remains while discovery changes from 25 to 17; girl entity 12 remains
25, and both exits are unchanged. These counts describe the reviewed bake diff,
not final combined delivery acceptance.

The authored records and matching native archive are outside every Git tree; only approved compiled map bakes are stored in this repository. No raw game source or source snippets belong in Git. No mod-owned text needs localization.

## Thread, cost and extension contract

This change adds no hook, observer, scan, timer, native write or cross-thread
pointer transfer. The new property reads are fixed and occur only on the native
owner after the candidate's lifetime check and exact action, script and
property-type guards. Existing discovery and selected-target checks remain the
call sites. Worker route and projection code receives its existing copied/baked
data; it does not inspect native pointers.

The acceptance threshold is cumulative mod-caused application lag above 25 ms
across hooks in a frame or full action/startup/loading/transition episode,
including sustained calls and backlog. Do not treat that as a per-call
allowance. Current offline fixture coverage does not measure actual native-frame
or audible response cost. The 30 to 42 ms worker elapsed observation is not
comparable evidence of application lag or a measured speedup.

Safe future changes must preserve the exact conservative predicate,
owner-thread reads, live lifetime revalidation, physical collision and geometry,
service and contact actions, and scripted/message/status-backed actors. Do not
classify by visual appearance or broaden the bake predicate to unknown metadata.
If native field availability or dispatch behavior differs, return the unresolved
evidence to Sol before changing the contract.

## Verification and remaining limits

`tools/entity_scanner_check.cpp` covers empty-Talk removal and retention of
message-, script-, nonzero-status-, close-message- and unknown-field cases,
services and contact actions. It also checks that metadata changes can restore
a target, reads remain owner-only, and a counted 1,000-detection batch stays
bounded. `tools/gysahl_elder_check.cpp` uses the real route worker fixture,
production collector and router with the pub and other-floor projection. It
checks the two forced goals, unreachable cells from both logged player starts,
retained girl and exits, and the remote projection. Final production results pass: collector Elder 1 before/0 after, girl 1 and
exits 2 in both; two forced goals remain unreachable from both logged cells.
All 104 raw identities and five scripted FFIV controls match independently.
The scanner batch performs 1,000 owner detections, 13,000 fake native reads and
0.309 ms cumulative fixture compute, with no worker native reads. This does not
measure actual native frame or audible latency.

A canonical baseline passed 8,275 checks with 0 name failures and 141/348/0
wiring results. A separate baseline stopped at `route_worker`; an unchanged
direct retry reported 0/0, with cause unestablished. Preserve
`build_check/gysahl_elder_20261026/before/route_worker_stopped.txt` and the
direct retry report. An initial scanner fixture accidentally used a Wyvern
placeholder and passed an unrelated name guard; it was corrected before
production changes. A duplicate-variable compile failure in a changed fixture
was fixed and its `compile_failure_log.txt` retained. These are history, not
final acceptance results.

The final source checkpoint passes the canonical all-six suite: 8,287 bound
names, 0 missing; 141 used/348 defined/0 undefined wiring; scanner, Gysahl and
all-six bake verification pass. Sol corrected the new fixture to select FFIII
explicitly rather than inheriting its parent fixture's FFII default, and the
canonical rerun passed. The two required equipment-party fixture refs were
also linked; the earlier fixture compile/link failures are preserved.
Receipts/hashes: `build_check/gysahl_elder_20261026/checkpoint.json`. Smoke,
scan, installation and gameplay remain separate combined delivery steps. No game launch,
control, probe, install or gameplay confirmation was performed for this task.
After those checks and installation, use the short nonvisual procedure in
[TESTING.md](../TESTING.md#october-26-gysahl-pub-elder). The player confirms
gameplay; Sol records final technical acceptance and any limits.


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
