# Remaining map-audit findings and evidence limits

## Current milestone and scope ledger

This is the single checkpoint, scope and Closed-system record; feature topics
hold evidence and link here rather than create parallel plans.

### Active October30: source-history removal and prevention

| Field | Current decision |
| --- | --- |
| Capability/scope | User explicitly authorizes rewriting shared native repository history to remove every recovered game-source/disassembly body, retain mod history and release history, and enforce the never-in-Git rule. Do not delete/recreate the repository. On a GitHub permission error, stop GitHub attempts without any browser/authentication fallback and report it; continue independent navigation work. |
| Boundary/evidence | Current shared HEAD71d53fb is clean and deployed in all6 games;30 historical raw files/53757 native instruction lines are already confirmed. Inspect all reachable local/remote heads and release tags for additional recovered bodies, preserve a verified recovery bundle outside every Git tree, sanitize only the identified game-source blobs, preserve current tree, names, ownership, legitimate vendor code and release assets. User authorization supersedes the previous defer-until-at-PC instruction for the rewrite itself. |
| Done/bounds | Source-free reachable-history check; exact pre/post current tree and mod/dependency parity; original refs/bundle/remotes preserved externally; unchanged release metadata/assets and tag names; explicit lease/atomic remote update when permitted; durable hard rule plus staged/build prevention. No tool installs, game launch, source dump inside Git, new release or unrelated visibility changes. |
| Effort/status | In progress (~10%);45–90min expected including full reachable-blob review, protected recovery, rewrite/verification and guarded remote attempt. A network sandbox DNS/thread error is not a GitHub permission denial; next read uses authorized network access with all interactive credential/browser prompts disabled. |
| Recovery | Existing all-six cf805c2e/11374080 bytes remain unchanged; canonical/security/recovery receipts frozen. Start from71d53fb and preserve all87 reachable commits/8 tag names before rewriting. Sol only; no delegation or browser fallback. |

### Queued October30: Maze and vehicle/world-map journeys

User-triggered follow-up to Closed navigation, completed one slice at a time:

1. Ancients' Maze/Crystal Tower reachability while on foot or aboard another
   vehicle, using an actual reachable parked Invincible when required; investigate
   the latest reported Invincible landing26 tiles west despite legal nearer
   landing at the Maze. Preserve actual terrain, statues, wind and native actions.
2. Choose required vehicles from live parked positions and real capabilities,
   including Invincible -> land -> Nautilus for Doga's Manor winds. Preserve manual
   boarding/landing actions and consistent browse/selected-route eligibility.
3. Connect actual world-map transitions including surface/underwater, prefer
   flying near an underwater entrance then diving when legal, preserve a valid
   slower underwater route, and support Doga's Village's tunnel/surface journey.
   Add a fourth World map filter limited to destinations on connected world-map
   bakes, including town/dungeon entrances there; do not flood it with interior
   town/dungeon contents. Existing literal Current map mode retains its contract.

Fixed acceptance: actual source/data/production-consumer before/after checks,
current/browse/retained route consistency, locked/missing/stale vehicles, actual
map boundaries, bounded owned-worker cancellation and cost, affected12-language
controls/docs, then serial canonical/smoke/exact-payload scan/guarded deployment.
No native probing/game launch or broad all-game redesign. Initial45–90min proof
checkpoint for the Maze/landing and existing transport planner, then scope the
necessary connected journey slice from its actual missing consumer/source facts.
Installed candidate remains frozen until required connected functionality passes.

### Closed: final combined candidate and coverage reconciliation

| Field | Current decision |
| --- | --- |
| Capability | Deliver the completed finite all-six map/event/reachability audit with current affected-feature documentation, canonical/offline/release/security evidence and verified applicable installation. |
| Boundary | Reconcile the original fixed findings/Closed interfaces and current source-matched receipts; run build.bat, smoketest.bat, exact-payload scan/recovery/deploy checks. No new features/refactors or game launch/probe. Any actual integration regression receives only its minimal accepted-design repair. Startup needs a naturally failed own log and remains a separate excluded known issue. |
| Coverage | Affected feature topics must be current and indexed. Unrelated whole-mod documentation catch-up is a separately finite coverage finding, not a new integration prerequisite without an actual missing safety/correctness contract. No duplicate scope ledger or user code-review gate. |
| Done/bounds | All required finite findings and source/consumer checks reconciled; one compiler and /m:1 scheduling; no C++ or embedded-data edits during compilation; source-free repository/evidence review, exact final payload hashes/scan and allowed recovery/deployment. Keep native/frame/audio and gameplay limits explicit; cumulative25ms rule remains. |
| Effort/status | Source/offline/all-six delivery Closed. Completed within the30–60min final milestone estimate, including21m42s full serial compile and two minimal legacy test corrections; final canonical2m37s pass. All finite cases,8290 native names/0 missing,142 used/349 defined/0 undefined, isolated smoke,67 exact resources/12 locales/667 source inputs pass. Defender22 exact files/stable hashes/no threats;132 installed hashes and144 protected files verified. Affected documentation is current. Final receipt: build_check/map_audit_final_20261029/completion.json; [delivery topic](MAP_AUDIT_DELIVERY_20261029.md). No native-frame/audio/gameplay certification. |
| Recovery | All six installed cf805c2e/11374080 bytes. Verified previous hid.dll/SDL3.dll recovery copies for every game, unchanged protected/dependency files, read-only Prism prerequisite checks. Prior candidate receipts frozen. Source cleanup200 external originals/30 current references is complete. User-directed commit follow-up verifies current deployment and stages all cleaned changes; historical source cleanup remains deferred until user is at PC. Sol only; no game probe/agents/history rewrite/repository recreation. |

Current history finding: [source-content review](SOURCE_CONTENT_REVIEW_20261029.md).
Thirty recovered-body files are confirmed in this shared native repository's
October12 history, with external originals matching after Git normalization.
User October29 directs committing all cleaned current changes and verifying the
shared mod in all six games.132 installed file hashes were reverified. Ordinary
current commits proceed; history rewrite/recreation waits until user is at PC.

Combined-check integration note: first serial compile completed in21m42s, then
the old navigation-followup assertion failed twice on FFV map284/actor4. That
door has two real world destinations. The prepared native identity lookup
deliberately returns no single destination for an ambiguous door; the owned
state-aware planner already selects both actual branches in the dedicated
6/6 check. The legacy assertion incorrectly demanded one destination for each
companion. Its minimal test correction preserves unique-exit matching and now
asserts ambiguity rather than an arbitrary destination. It passes0 failures;
no production source/data/DLL change was needed. Original failed log, exit,
assertions and source manifest are preserved under the final receipt directory.
The next complete pass found six more outdated assertions in mapstate_check:
the generic harness applied each destination-scoped predicate to its native
Event rather than its matching Warp companion. It now retains the exact native
identity check and verifies the predicate against its exact destination arc.
All-six mapstate assertions now pass0, including unavailable-flag controls;
the separate conditional-door test retains native-door/nonconsumption and6/6
real routes. Again only the harness changed. The second failed canonical log,
mapstate report and candidate manifest are preserved. No unchanged failed retry
or new production system was added.

Affected-feature coverage is current in FEATURE_INDEX, THREADING, TESTING and
the linked finite feature topics. Unrelated whole-mod documentation catch-up
remains excluded: reconciling older PORT_STATUS/AUDIT summary wording and the
historical FEATURE_INDEX journal with their existing feature topics. This is
documentation coverage only, not a missing implementation prerequisite or a new
automatically started task. No Closed subsystem is reopened by that finding.

### Completed source/offline: final script closures and source ownership

| Field | Current decision |
| --- | --- |
| Capability | Resolve the last two continuation-closure cases and five local/shared/missing script-owner cases without guessed event meaning, false gates or omitted real interactions. |
| Fixed inventory | VI sc_e_1055 native trigger-script lookup and sc_e_0650 cycle; II sc_transition_20041_7; IV sc_e_0006/sc_savepoint; V sc_e_0076_3/_4. Existing source inventory: build_check/map_event_audit_20261028/source_ownership_resume_20261029.json. |
| Boundary/bounds | Exact native selector/dispatcher and matching extracted assets; existing metadata/label/bake mechanisms for actual incorrect data only. No native capture/hook, global revival, route redesign or reopening of Closed systems. Preserve real source uncertainty and the literal-map inventory contract. Native code/source archives remain outside Git. |
| Done checks | Each case has an explicit native/source-owner/closure disposition, actual consumer evidence for any repair, localized output and current feature documentation. Existing finite VM/graph/cancellation bounds; no native game-thread work added. Final coverage/combined/security/applicable all-game delivery follows. |
| Effort/status | Source/targeted offline Closed. Native local/resident banks resolve all five cases; save marker uses Resident7, dormant IV demo is Action0, Fynn helper preserves exact real exit, V dance55/56 have no bank and sibling54's foreign barrier binding was retired. All other active reviewed roots are exact local/Resident. VI live contact is a native actor ScriptId dispatch; apparent SCC changes shared-area variable and terminates on both post-win paths. Actual15 actor/native/remote cases8→0,1362 functional/source/12-language and245 scene/41 narrative checks pass. All six bake/state hashes unchanged. Completed within45–75min estimate; combined/security/delivery pending. [Detailed evidence](FINAL_SCRIPT_CASES_20261029.md). |
| Recovery | Installed FFIII b61153fc unchanged. Source and targeted evidence frozen at build_check/silent_context_20261029/completion.json and final_inputs.json. No compiler/session/question pending; Sol only/no game probe/delegation/publication. |

### Completed source/offline: remaining245 silent scene families

| Field | Current decision |
| --- | --- |
| Capability | Give physical mechanisms, barriers and ordinary travel points useful functional labels, while retaining Scene trigger for actual narrative scenes. Keep real map travel in Map Exits and preserve unavailable-source uncertainty. |
| Reason/boundary | Review the finite245 silent families from the original954 inventory against full script/continuation context and actual current category/provider. Some III native actors are intentionally replaced by accepted world providers. Existing exact identity/hash reviews; no runtime script interpretation/new hooks/route redesign. Necessary actual dependencies:3 V Main scripts have2 story-flag destinations but only the first is baked; IV Devil's Road transfers through an intermediate world scene and sets the final player at6,6 rather than its native PointIn1. Reuse typed bake companions, existing bounded guard projection and landing overrides; preserve other Closed interfaces. |
| Done/bounds | Every family has an explicit supported contextual disposition, source-hash witnesses and actual label/category checks for changes. Only concrete incorrect bake/reachability data is corrected through existing producers. Existing bounds/native ownership and localization preserved. |
| Effort/status | Source/data/targeted offline Closed:all245 dispositions,41 narrative families preserved;381 native/remote cases0 (initial368 cases756 before assertions→0);1362 functional/source/12-language0. Three conditional doors3/6→6/6 actual entrance plans, native and unknown guards/terminal boundaries pass. Devil Road0/2→2/2 and exact final6,6 continuation/lock pass. Six underwater mounted routes/batches and actual prior foot parity pass (first extra test wrongly forbade native Player1 movement; corrected from real before bakes). All-six planner and native transport/pressure-switch regressions0.134 promotions/3 native companions/1 exact chain remap/1 virtual arrival; other geometry/masters/objects/arcs preserved; only six FFV scoped guard rows, IV state byte-identical. Completed within the60–120min contextual estimate. Final combined canonical/security/delivery pending. Detailed contracts/limits: [silent-event evidence](SILENT_EVENT_CONTEXT_20261029.md). |
| Recovery | Installed FFIII b61153fc unchanged; NPC, Doga and arrival/power source/offline receipts frozen. Sol only/no game probe/delegation/publication. |

### Completed source/offline: conditional arrivals and trigger power ownership

| Field | Current decision |
| --- | --- |
| Capability | Preserve real arrival coordinates and reachability for the seven remaining conditional door transfers; distinguish genuinely consumed triggers from triggers rearmed by native lifecycle. |
| Reason | All eight original variant sets have PointIns; one V pair uses the same cell. Remaining III four Amur returns/world4-point11, Map_30231_10 actor3→290-point1; V Map_20050 actor60→81-point1 and Map_30130 actor2→world1-point12 need exact native choice/position evidence. |
| Boundary | Existing state/PointIn/travel producer only for proven errors. Native default-list/first-match lookup resolves all8 sets: no arrival bake change. Necessary actual omission:17 VI SwitchEvent30 actors inherit EventTriggerEntity but were absent from capture and step_event; admit that native type and preserve exact source matching. No guessed arrivals, grid edits, global trigger revival or route redesign. |
| Done/bounds | All8 real doors/24 known-and-unknown choices/24 fresh owned continuations0fail. All-six native Hide/Reset/Exit rearm versus property/suspend/overlap facts reconciled. Real17 switches:86 before assertions→0,34 powered/rearmed captures/17 remote hazards, unpowered guards/native-owner0. All-six route_worker0,98 vehicle models/582 world candidates. No new loop/hook/native invocation; existing capture/worker/cancellation/graph bounds. |
| Effort/status | Source/targeted offline Closed at build_check/trigger_power_20261029/completion.json; detailed contracts and native locators in [arrival/power evidence](ARRIVALS_TRIGGER_POWER_20261029.md). Original30–60min estimate extended for the actual omitted subtype and shared regression; no architectural branch. Canonical/security/delivery remain queued. |
| Recovery | Installed FFIII b61153fc unchanged. Preserved NPC and Doga receipt/input hashes precede this milestone. Sol only/no game probe/delegation/publication. |

### Completed source/offline: Doga surfacing marker browse annotation

| Field | Current decision |
| --- | --- |
| Capability | A reachable Surface near Doga’s Village marker must show its usable distance when browsed, rather than Not directly reachable solely because surfacing is the destination action. |
| Reason/boundary | Source-proven integration issue: plan_browse_snapshot marks every action_after indirect; the marker adds its own manual surface action. Preserve the delivered marker/provider/native action and route architecture. Change only exact marker endpoint annotation, after an actual before assertion. |
| Done/bounds | Actual marker provider→category→browse→route check, current mounted arrival/partial-blocker/other vehicle paths preserved; no native work/hook or extra search. Existing route work bounds/cancellation unchanged. |
| Status/effort | Source/offline Closed: actual provider/category/route/browse2→0,147-step and zero-step endpoints, wrong vehicle/other-game abstention and post-surface landing pass. Completed within10–15min estimate; canonical/security/delivery pending final audit milestone. Accepted provider/native design preserved. |
| Recovery | FFIII b61153fc remains installed; NPC source/offline closure receipt frozen. Broader canonical/security/delivery follows the remaining audit. |

### Completed source/offline: remaining six NPC region/scene contexts

| Field | Current decision |
| --- | --- |
| Capability | Resolve the remaining three South Figaro movement-region NPCs, bedroom NPC, Darill guide and FFII scene-owned Emperor; retain valid approaches and exclude invented cross-layer/counter access. |
| Reason/evidence | Current geometry has one entry-connected region position for each of the three Figaro patrols, but their homes lack approaches. Exact native region/line semantics now verified; actor movement/entrance/target-local controls pass but graph witness is missing. The inherited test host also disabled the real III–VI hidden-passage flag; production-matched controls confirm the same missing graph witness. Original eleven before/after proof remains11→0. Decisive trace locates the missing root stair edge: real Upper masks159→31 allow SE, but four cardinal neighbors are zero. Local A* passes; distance_to_object only checked cardinal predecessors. Minimal integration adds only validated native indoor diagonals to that terminal-edge estimate; world movement unchanged. Temporary trace has been removed. Bedroom and Darill already pass actual local planner; Emperor has three reachable scene controls. |
| Boundary/bounds | Reuse existing metadata/owned remote interaction goal pipeline if an actual defect is proved. No new capture/hook/framework or relaxed layer. Pin exact source identity/native conversion and at most32 verified positions; existing graph cancellation and limits remain. Three source-hashed default Area2/Route3 rows add12 positions in one existing state record; prior fields/program unchanged. Route0 is allowed only for a known initializer/active wandering actor, with exact fingerprint. |
| Done checks | Actual source region→goal→cross-floor first leg→native floor actor, active/hidden and state guards; scene owners/real controls recorded; targeted current and remote regressions. Combined canonical/security/delivery remains the final audit milestone. |
| Effort/status | Source/data/targeted offline complete. Three region routes/native continuations and three FFII active mixed-state scene controls pass; bedroom/Darill source-matched planner pass; all-six planner/initializer0. About35min against30–45min estimate for this region/scene slice. Source/data integrated: native stair edge is usable and3 region-selected routes pass. All11 assigned-route regressions now replay every real intermediate floor; seven Zozo fixtures previously treated first arrival389 as final floor390. Corrected bounded receipt replay and final guards pass. |
| Recovery/closure | Source/data/targeted offline Closed at build_check/npc_future_20261029/completion.json and final_candidate_inputs.json. Canonical/security/delivery pending final audit milestone; installed FFIII b61153fc unchanged. Exact17-function archive outside Git in ff6_region_native. Existing Closed interfaces preserved. |

### Completed source/offline slice: eleven assigned-route NPCs

| Field | Current decision |
| --- | --- |
| Capability | From another floor, select any of the eleven reviewed native moving NPCs and retain a valid first-floor route; use the actual actor once its floor loads. |
| Reason | Actual collector/planner11 failures with reachable entry and native-pose controls. Exact initializer branches own all11 assignments, including turtle bank2/76 and dragon107=1,108=0. |
| Boundary | Minimal integration in existing read-only initializer projection and remote interaction goal producer. Preserve live capture, blocker handling, masks, graph, controls, labels and native ownership. No new hooks, runtime probes or route framework. |
| Bounds | Existing8192-instruction/64-branch initializer bounds;64 assignments/metadata rows and32 positions per actor. Native counts are11 actors/67 distinct positions across5 records; no per-position graph search. Cancellable existing graph retains400 visited-state bound. Current-floor native timing/work unchanged. Measured collector+first-leg compute8.248ms total/1.269ms peak for11 requests; no runtime frame/audio claim. |
| Done checks | Actual collector→selection→first leg→pending identity→live floor continuation; inactive/unknown assignment, wrong actor, malformed data, cancel and ordinary route controls; exact data delta, affected docs, combined canonical/security/applicable delivery. |
| Effort/status | Source/data/targeted offline integration complete:11→0 actual collector/first-leg and all11 live-floor continuations; all-six planner0,98 vehicle models/582 world targets; inactive/unknown/cancel/retained-state controls pass. Combined canonical/security/all-game delivery stay on the final audit milestone. Initial30–45min proof checkpoint overran to~45min; valid11-case proof found necessary integration. Revised30–45min for source/checks, excluding combined serial build. Not gameplay confirmed. |
| Recovery | Installed FFIII b61153fc remains unchanged. Original FFV/VI states and11 before failures retained under build_check/npc_future_20261029/. No game launched or other-game deployment. |
| Closure | Source/data/targeted integration Closed after production-flag requalification: retained original states still11 selected-route failures; corrected states0/all11 native continuations. Canonical/security/delivery pending on the final milestone. Live NPC capture, counters, vehicle routing, current statuses, Doga marker and Crystal approach remain Closed at their linked accepted boundaries; this patch does not refactor them. |

The finite assignment has no remaining implementation, affected-documentation,
offline or installation work. All original source-owner/closure cases are
resolved at the Closed cards above. Natural-play/audio/frame confirmation,
the separate startup failure log, historical-source migration authorization
and excluded unrelated documentation coverage retain their stated boundaries.
Whole-mod unrelated documentation catch-up is separately tracked at final coverage
reconciliation; it is not a prerequisite for this scoped fix unless an actual
missing safety/correctness contract is found. No additional feature is added.


## Earlier inventory and checkpoint history

The inventory below preserves the original findings, estimates and receipts.
Its remaining/open wording is superseded by the current cards above; it is not
a second active plan or an unresolved-source count.

Current FFIII installed b61153fc/11357184 bytes: [live ally statuses](BATTLE_LIVE_STATUS_20261029.md),
[Doga surfacing marker](DOGA_SURFACE_MARKER_20261029.md) and
[Crystal Tower via Ancients' Maze](CRYSTAL_TOWER_APPROACH_20261029.md) source/offline/
FFIII delivery Closed. Exact cleared flags hide8 statues and relocate4 triggers;
Tower current-map/selected/landing/pending/maze continuation/gates pass. Canonical
18m04s shared-header serial rebuild,8290 names0missing/67resources/12locales/
22-file Defender scan pass;22 installed hashes/144 protected/other5 unchanged.
Recovery0a511191. No compiler running. Frozen receipts are under
build_check/crystal_tower_20261029/; do not rerun its exact-source verifier after
later audit changes. Sol only/no game launch/probe/delegation/publication.
Resume full map audit at22 NPC-context acceptance/future-floor moving targets,
then7 conditional arrivals/power,245 silent families,2 closures/5 source owners.
Istory and FFVI mask now pass canonical; applicable other-game delivery remains
pending. Source-owner resume inventory is saved in
build_check/map_event_audit_20261028/source_ownership_resume_20261029.json.




Current audit checkpoint: [FFVI Imperial Camp mask projection](NPC_MASK_PROJECTION_20261029.md)
is source/data/targeted offline Closed (8→0, exact2 opcode delta, all-six tracking
2454 roots/2523 identities0fail). Other metadata/terrain preserved; canonical/
applicable delivery pending. Resume the remaining NPC contexts, starting with
the actual Istory sheep prerequisite, then arrivals/power/scene-owner cases.

[Invincible preference](INVINCIBLE_PREFERENCE_20261029.md) is source/offline/FFIII
delivery Closed in0a511191/11348480 bytes. Canonical/smoke/scan and installed22/
protected144/other5 preservation pass. Human confirmed Manor's Nautilus-only
wind restriction; no change. Recovery63a24c76; Sol only, no game probe.

Active finite [Ghido shore landing](SCRIPTED_SHORE_LANDING_20261029.md): actual
NPC/entry planner4 before failures; native scripted endpoint12,32 reaches both
variants. Existing override adds one same-map edge/landing with exact preservation
delta; after4→0 and combined canonical pass. No Closed route/capture redesign.
Included in FFIII0a511191 payload; actual FFV installation remains pending.

Human-priority interruption: [cross-floor animated objects](REMOTE_INTERACTIVE_OBJECTS_20261029.md).
Shared collector omitted AnimEntity10. Lute/Fang real-data20 before failures→0;
capture/real label/publication and all-six regressions pass.
FFIII installed63a24c76 passes canonical/smoke/8458 names/67 resources/12 locales/
22-file scan; human closed the game and guarded deployment verified22 installed
files/144 protected/other5 unchanged. Object inclusion and bounded instruction
capture are source/offline/FFIII delivery Closed. Resume the remaining NPC
context evidence; no map data was changed for a speculative same-map connection.

Active [22 NPC context evidence](NPC_GEOMETRY_CONTEXT_20261029.md): original
inventory and finite geometry host pin all22 cases. New real classification
defect: active RoutePuppet102 in III/V/VI was mistaken for stationary; exact
native constructors/reporting/restoration differ from I/II/IV. Completion retains
102, so the initial scalar-only candidate is superseded, never deployed. Current
bounded ready-controller observation checks remaining movement; completion/
animation/overload/Wait/manual and canonical checks pass,8458 names0missing,
included in installed FFIII63a24c76.1000 complete8-actor overload episodes stay
bounded; exact final timings are in the linked context record. Eleven exact V/VI route assets prove78 native live
positions/51 entry-connected approaches, covering11 of22 contexts. Remaining11
scene/entry/region cases remain open. No compiler running.

Next active audit capability:22 NPC geometry context cases. Estimate45–90minutes
for this finite list plus any concrete minimal regression correction. Do not
reopen Closed navigation designs. [Field effects](FIELD_EFFECT_COLLISION_20261029.md)
is now source/offline Closed:101 actual records,14 Karnak route witnesses/11
detours, actual world Enterprise13/Invincible11 movement and native flight path
pass. Final extension changed only checks/docs; installed75de3594 hash unchanged.
Then retain7 arrivals/power,245 silent families/2 closure/5 source owners and
startup's separately open naturally-failed-log diagnosis. No compiler running.

Current milestone: FFIII75de3594/11340288 bytes installed/hash-verified after human
closed/deploy. Artillery/command-commit/confirmed suffixes/ladder/Elder are source/
offline/FFIII delivery Closed; Torna source/offline is Closed, FFV installation
pending later all-six delivery. Full serial142s, smoke,8419 native names0missing,
67 resources/12 locales/22-file Defender scan pass;22 installed hashes match,
144 protected/other5 unchanged. Recoveryad0d80c1 retained; no game launched/probed.
Authoritative receipt: build_check/ff3_oct29_followups/completion.json.

Resume the open finite audit at field-effect native flight applicability and
actual Karnak route acceptance, then22 geometry,7 conditional arrivals/power,
245 silent families/2 closure/5 source owners. Earlier active build/candidate
statements below are superseded checkpoints; do not reopen Closed designs.

Human-priority interruption: [non-target command reset](COMMAND_COMMIT_FOCUS_20261029.md),
native/source complete, all-six48→0, existing focus regression0. Keep artillery/
exit/ladder/Elder/Torna/effect source and all broad-audit cases. No candidate has
been installed since ad0d80c1. Build22383 failed an isolated scan-host stub link;
the host is repaired. Canonical checks must rerun after this new scoped fix.

Latest finite priority checkpoint: [Invincible artillery](INVINCIBLE_ARTILLERY_20261029.md)
12→0 and [exact confirmed exit suffixes](CONFIRMED_EXIT_SUFFIXES_20261029.md)8→0;
source/offline complete, canonical serial combined build22383 running. Ladder/
Elder and Torna ship-entry/scene eligibility are included. Installed FFIII remains
ad0d80c1 pending final checks/scan/delivery. No C++/embedded-data edits while the
compiler runs. Broader audit resumes at field-effect flight applicability, then
the retained22 geometry,7 arrivals/power,245 silent families/2 closure/5 owners.
Torna221 scripted-world converter cases and101 effect capture/projection cases
pass; this does not settle every story-state or world-aircraft physics case.

Active human steering: [Torna Canal and IV/V/VI world-script capture](TORNA_WORLD_EVENTS_20261029.md).
Code-only diagnosis; explicitly no tester-log search. Solid-effect source/data
checkpoint is preserved uncompiled, no compiler running; FFIII ad0d80c1 remains
installed. Resolve tester event/ship-entry pipeline, then resume effect acceptance
and the remaining finite audit. Closed designs stay Closed except scoped actual
regression integration repairs.

Current milestone: FFIII ad0d80c1/11327488 bytes installed/hash-verified.
[Invincible mountain crossing/Falgabard](INVINCIBLE_MOUNTAINS_20261029.md) is
source/delivery Closed: actual owned provider/category/manual-action/landing
checks, all3 entrances, floating model11, cancellation and other-game abstention
pass. Canonical build/smoke/67 resources/12 locales/22-file scan pass;22 installed
hashes match,144 protected files/other5 unchanged. Source-bearing native evidence
remains outside Git. No game launched/probed, agent or publication.

Next active finite checkpoint: the8 FFV Karnak fire-effect entities omitted by
ordinary visual-effect filtering. Resolve whether their native collider/action
actually blocks traversal, then test the minimal route-only correction if needed.
Native/asset proof now confirms real solids and expands this same omission to
101 collision-enabled effects across III4/IV1/V96, including nonscripted walls.
The other4234 decorative effects remain excluded. Exact scope and before/after
acceptance: [field-effect collision](FIELD_EFFECT_COLLISION_20261029.md).
Estimated45–75minutes plus applicable combined/release checks. Other closed
navigation systems retain their accepted design. Then22 geometry cases,7
conditional arrivals/power rearming,245 silent families,2 closure/5 owner cases.
Do not describe the full audit or startup root cause as complete.

Current milestone: FFIII d3053479/11311104 bytes installed/hash-verified after
the human's [page/vehicle follow-ups](ITEM_BUFFER_PAGES_20261028.md). Native page
readiness, Can surface, cold-discovery exact vehicle names and retained-route
movement validation are source/delivery Closed. The per-leg vehicle selector
implementation also passes canonical current/parked, scanner/filter, continuation,
cancellation, transition and all-six metadata checks;8406 native names/0 missing,
smoke and22-file scan pass.144 protected files/other5 installations unchanged.

Resume finite audit work: remaining trigger-power lifetime scope, then Invincible
mountain crossing/Falgabard, FFV fire effects,22 geometry cases,7 conditional
arrivals,245 silent scene families,2 closure/5 owner questions. Startup's exact
original cause still awaits naturally produced failed-launch evidence. No new
game launch/control/probe or agent. Preserve Closed designs; minimal actual
regression/integration fixes remain scoped to this human-requested audit.

Historical checkpoints below are superseded by the current milestone.

Human interruption [turn-order display](TURN_ORDER_DISPLAY_20261028.md) is Closed:
no displayed list/no Shift+T added. [Vehicle applicability](VEHICLE_APPLICABILITY_20261028.md)
resumes with incomplete/uncompiled later source preserved, not a ready candidate.
Capture4→0; all-six metadata/arc tests have one corrected, not-rerun fixture
assumption. Finish phase validation/warning integration and later checks on resume.
Installed FFIII9146ae03 remains unchanged. No new deployment/game probe/agent.

Latest milestone: FFIII9146ae03/11107328 bytes installed/hash-verified after
human save/close. Full serial/offline/smoke/67 resources/12 locales/22-file scan
pass;22 installed hashes match,144 protected/other5 unchanged. Battle feedback,
category applicability and the passing parked/unfiltered provider corrections
are source/delivery Closed. Full audit checkpoint1 resumes: retained vehicle-hidden
actors, exact per-leg trigger/collider target IDs and native automatic-canoe order,
then Invincible crossing and every later recorded uncertainty. No game probe,
delegation, public release or unrelated Closed-design refactor is authorized.

Additional human follow-up [save-point categories](SAVE_CATEGORY_20261028.md)
passes source/integration24→0 and is delivered. It changes only category cycling;
no native capture or masks. It does not close the full audit.

Completed intervening steering: [Boost use/Guard feedback](ABILITY_FEEDBACK_TARGETS_20261028.md).
Provider/unfiltered3→0, continent/Saronia and native scanner/filter adapters pass;
these navigation changes are now installed in FFIII9146ae03. No native
trigger-ID/collider-mask changes have yet been made. Resume the whole checkpoint1,
Invincible crossing and all six-game uncertainties; no automatic agent or
closed-design refactor.

Checkpoint1 completed provider checkpoint (installed in FFIII9146ae03): unfiltered synthetic discovery and
parked-flight provider3 before failures→0, actual full category/provider host.
Native Player1 is valid; the old valid-vehicle branch consumed it, add() rejected
it as non-aircraft, and its else skipped every parked aircraft. The correction
branches on an actually supported mounted flight and allows foot/automatic
movement to consider parked models, preserving native boarding/terrain proof.
It does not invent a walk off an unsupported mounted ship. Current map separately
retains one physically valid nearest native aerial trigger with unknown distance
when no ride is proved; real story presence, completed Saronia exclusion and
nearest-marker aggregation remain. Reachable modes keep strict flight proof.
Native-owner capture includes parked vehicles in unfiltered mode so a usable
route may still determine the representative. Existing continent and Saronia
regressions and scanner/filter adapters pass; broader mode combinations remain.
Evidence build_check/vehicle_applicability_20261028/ and combined delivery
build_check/boost_use_20261028/. Exact current FFIII hash is recorded above.

Current milestone: FFIII b2cacd6f/11103232 bytes installed/hash-verified after
full serial build/smoke/67 resources/12 locales/22-file scan. Dive/underwater/
ordinary-landing checkpoint is source/delivery Closed; details in
[Nautilus evidence](NAUTILUS_ACCESS_20261028.md). Full audit checkpoint1 resumes,
with all-unlocked vehicle capture/per-leg trigger/collider applicability Active.
New exact integration case under the existing Current map request: ordinary
blocked exits are retained now, but synthetic aerial providers still require a
reachable flight and can omit their deliberately suppressed native source when
unfiltered/on foot. Preserve nearest-marker aggregation and real story presence;
provide discovery independently from route proof. Do not declare literal Current
map coverage complete until this actual capture/provider/publication path passes.
Initial next-checkpoint estimate60–120minutes including native order/applicability
proof and actual-source checks; report new dependencies/overrun. No agents/game
launch/control/probes/publication. Startup root-cause diagnosis awaits naturally
produced user launch evidence; the unsafe ordering repair is not causal proof.

Active human follow-up before startup deployment: [Nautilus dive/underwater](NAUTILUS_ACCESS_20261028.md).
The logged underwater origin113,118 reaches every real entrance after correcting
classification and mounted-entry policy; all4 arcs and256x256 terrain were already
baked. No synthetic surface dive markers are needed for that reported state.
Native9 uses end_landing358 and confirm terrain16 (not down_script358). Underwater
14's real transfer scripts disembark inside entry. Add Can dive on actual native
guide availability, then combined verification/delivery. Full all-unlocked vehicle
applicability and the six-game audit remain open; do not mark them complete here.

Latest human clarification: the parked-vehicle requirement covers EVERY unlocked
vehicle, not only Nautilus. Carry actual ownership, legal boarding, movement and
landing through capture, browse eligibility, publication and selected routes.
Also inspect Nautilus underwater capability: missing dive/surface or underwater
entrance arcs are part of checkpoint1, with native source-to-bake witnesses.
Current map must retain all real destinations on its bake even when unreachable;
reachability and story-gate pruning cannot silently override this mode. Existing
explicit physical grouping/category preferences and native story presence remain
separate from accessibility. Do not invent access to an unavailable vehicle.
Startup candidate0a6c18a3 passes full serial build/smoke/67-resource verification
and exact22-file Defender scan. Deployment is held only by the running-game
guard; the human has been asked to save/close. Navigation investigation continues
independently while installation waits. No new game probe or delegation.

Dalg report withdrawn by human: destination is already present; it was misheard.
Do not implement a Dalg change for that report. Startup diagnosis remains active;
resume the preserved full audit after the startup checkpoint.

Next confirmed FFIII progress regression after startup: Doga's Manor requires
landing then walking, but current Nautilus directions fly through the entry.
Restore the landing assertion in the existing boarding check; it was weakened
after observing the incorrect one-phase native-mod route. Check ordinary exits
for every manual aircraft, preserving real airborne/ship-scene/pad exceptions.
This is user-initiated scope; do not force foot entry for actual airborne scenes.

Human-prioritized native-mod startup fault now defers this audit; preserve all
current classification checks/data and native crossing evidence. On completion,
fix Doga's Manor landing/walking and parked-Nautilus reachability: Current map
with vehicles must list destinations obtainable by legal boarding, rather than
requiring the player already aboard. Test actual parked/mounted capture, list
eligibility and selected route together. Dalg report is withdrawn. Then resume
Falgabard/Invincible/capture/applicability and all geometry/effect/scene cases.
Also review existing ordinary_world_exit flags: the older explicit _to_world
name branch bypasses its opcode grammar. This is an unverified classification
risk to inspect against all real story/physical bodies, not a confirmed defect
or permission for an unrelated redesign. No automatic delegation.

Newest exact case: [Falgabard scripted entrance](SCRIPTED_ENTRANCES_20261028.md).
Human requires ordinary transfers in Map Exits, real scenes in Events. Reviewed
generic Map entrance labels were not promoted to named/category destinations;
Falgabard also exposes an unmodelled native Invincible mountain-crossing route.
Trace category, capture, vehicle-specific eligibility and selected route together.
Latest urgent NPC/nearby walking/Book suffix fixes are deployed de3e075d; full
audit has resumed at this case within checkpoint1. Sol only/no game probe.

Latest human steering: include [nearby walking and object suffixes](LOCAL_GUIDANCE_20261028.md)
with the cross-floor NPC deployment. These narrow fixes take priority; the full
audit remains authorized and resumes immediately after their FFIII installation.

New urgent human regression: cross-floor route to the Saronia1F chest now says
No path while entering that floor correctly produces Interact: Soldier. Restore
the previous cross-floor blocking-NPC guidance before continuing checkpoint1.
Vehicle audit is preserved at its native/capture reproduction; no production
vehicle-rule changes have been made. Scope is a minimal route integration fix,
with real-map before/after evidence and FFIII delivery, Sol only/no game probe.

Human scope update October28: after deploying the Boost wording, continue the
full audit and address every uncertainty in this register. This explicitly
reopens the necessary vehicle/trigger applicability, effect-collision and
reachability/label follow-ups; it does not authorize unrelated refactors, agents,
game launch/control/probes or publication. Sol only. Sequential checkpoints:
1) vehicle applicability/capture-to-route/publication;2) scripted field effects;
3) remaining NPC/layer/arrival cases;4) all remaining scene/source/continuation
contexts;5) combined verification, documentation and applicable delivery.
Initial estimate3–6hours, dependent on native ownership/continuation complexity;
report overruns and retain concrete unresolved facts rather than certifying
unsupported runtime states. Boost wording/delivery is the only active checkpoint
until FFIII installation completes. That installation now passes: FFIII
a7a97a14/11097600 bytes,22 installed scanned hashes match,144 protected unchanged,
other five games unchanged. Audit checkpoint1 is now Active. No game probe.

The [six-game audit](MAP_EVENT_AUDIT_20261028.md) has delivered its proven minimal
repairs. This queue distinguishes incomplete context from actual mod defects.
It does not certify all story states, make a runtime failure claim from a baseline
survey, or reopen a Closed vehicle/routing design automatically. Sol only.

## Vehicle-specific trigger applicability

Current checkpoint details: [typed capture and native automatic-mode order](VEHICLE_APPLICABILITY_20261028.md).
The strengthened actual-source host reproduces4 failures with real-shaped
selector lists; native-owner violations0. Exact canoe dispatch/coroutines are
exported outside Git. Production masks/capture remain unchanged pending the
native seam-order proof and complete per-leg integration.

Confirmed source gap: capture retains powered triggers but not each trigger's
native target-transportation ID list for later planned vehicle legs. Collider
applicability is also frozen to the captured vehicle. Native exact-ID restrictions
and overlap/power semantics were reviewed. Dropping an inapplicable trigger at
capture would incorrectly discard a hazard that activates after boarding.

A separately scoped follow-up must carry owned applicability data through every
planned leg, discovery/category publication and selected routing. Preserve genuine
ground gates and native vehicle transitions. Test aircraft/ship/canoe/foot legs
with both applicable and inapplicable triggers. Do not use present player overlap
as proof of whole-map trigger availability. Saronia's reviewed scene has no native
transportation-ID restriction; its complete-capture regression is unaffected.

## Scripted field effects omitted from collision classification

FFV Map_20060, group ev_e_fire, native entity IDs90–97: all eight EffectEntity
records carry collision=true, action2 and script1043/sc_carnac_fire. The source
script is a directional fire-response animation, not a dialogue scene. Type11 is
absent from BAKED_TYPES, and current live_blockers has no type11 collision branch;
the display detector intentionally filters field effects. These are a concrete
coverage gap, not proof that every visual effect should become a destination.

Inspect FieldEffectBoardEntity.Init/Show/Hide, its actual collider ownership and
the native movement/grid fallback before a minimal collision repair. Preserve
scripted enable/disable and rectangular footprints in current/future maps. A
source-to-output fixture must include actual type11 objects, not an Entity stand-in.
The six FFIV omitted effects have collision=false and script_id0 and are visual
effects; they are not equivalent missing interactions. Raw/native material stays
outside Git in parent tools/map_event_audit_20261028/ and tools/mapdump/.

## Remaining geometry context

Production Resolver/effective objects/obstacles/interaction goals/flood were run
for all270 baseline suspects with at most64 copied threshold samples per room.
Persistent flag banks use only native bit values0/1. The samples are not a proof
that their combined global story values are a valid gameplay trajectory.

| Outcome | Records | Meaning |
| --- | ---: | --- |
| Reachable in a sampled projection | 23 | Baseline all-group result is insufficient to call these blockers |
| Inactive in sampled projections | 82 | No static reachability conclusion without a relevant active state |
| Staged outside map bounds | 143 | Inspect actual scripted/live placement; never route to off-map homes |
| Native/dynamic context still needed | 22 | Exact remaining source locators below |

The22 remaining cases are FFII Map_30131_7/entity4; FFV Map_20041_9/5,
Map_20090/22,103 and Map_30381_7/2,16; FFVI Map_20051_3/12,14,15,
Map_20051_4/12, Map_20090/24, Map_30410/21, Map_30511_7/7–13,
Map_30641_1/16, Map_30671_1/2 and Map_40005/2. Check native layer assignment,
same-cell interaction, scripted approach and actual active positions. Do not
manufacture a counter or open a story barrier from a disconnected static cell.

The original two noncardinal-only witnesses are FFIV Map_30161_3/entity23 and
FFVI Map_40005/entity2. Native interaction explicitly allows matching X/Y;
strict-cardinal survey output alone does not authorize removing production
fallbacks. The former is inactive in sampled states; the latter remains among22.
Detailed cell/group/native-property locators are retained in state_geometry.tsv
and external ffN_context_triage.json, with exact source provenance.

## Remaining scene context and arrival variants

All954 initial real-map Scene script families have complete source-graph inventory:
243 have pinned functional reviews;464 contain dialogue/encounter context;
245 need further distinction between silent narrative staging and physical
mechanics;2 have unresolved closure. This is not a claim that every remaining
Scene label is correct. The two closure cases are FFVI sc_e_1055 (native
trigger-script lookup) and sc_e_0650 (script cycle). Full graphs/body hashes and
occurrences are outside Git, not embedded game-source fixtures.

Some FFIII continent transfers in that inventory are already owned by the
synthetic Floating Continent/Surface World provider; the native Scene-labelled
actor is deliberately not published. Check the actual producer/category before
treating a raw catalog label as a missing destination. Underwater/ship travel
families with native SysCalls remain contextual candidates, not automatically
approved generic scene replacements.

The initial58 missing local source occurrences include53 FFV save points now
resolved through both pinned global variants. Five local-owner/source questions
remain: FFII sc_transition_20041_7; FFIV sc_e_0006/sc_savepoint; FFV sc_e_0076_3/_4.
Do not confuse these occurrence ownership gaps with the two closure cases above.

All eight initially ambiguous door arrivals have actual PointIn records; none is
missing. One FFV variant pair uses the same cell. Seven have conditional/dynamic
coordinates: FFIII's four Amur returns to world4/point11 and Map_30231_10/entity3
to290/point1; FFV Map_20050/entity60 to81/point1 and Map_30130/entity2 to1/point12.
Trace group/native selection before changing a valid conditional arrival.

No game launch/control/probe, new runtime capture, other-game deployment or
publication occurred during this follow-up queue's preparation. Source-bearing
analysis remains outside Git. Native/frame/audible timing and gameplay remain
separate from the offline evidence.
