# Multiple NPCs and route reversals

## Scope ledger

User-initiated FFIII route follow-up; source complete, Closed. The user's
pub-to-inn journey repeatedly requested north after south and south after north,
despite an accessible route. Check full-route optimality, live multi-NPC collision
and continuation; repair proved defects without bypassing blockers or imposing
an arbitrary detour cap. Scan display source is complete, Closed; earlier source
checkpoints remain Closed. EXP Enter feedback and Amur remote-return previews
are separate queued scopes. No game launch/control/probe.

Own log: build_check/npc_oscillation_20261027/user_ff3_part6.log, same session
pid25300/start249494546. At255913.796 the route begins2North, at255914.343
2South, at255915.125 1North, and255916.218 1South. The recorded selected remote
objective is map101(17,12), reached through the Amur Inn & Item Shop entrance;
the user's journey description and the actual selected-objective identity are
kept separate. Live actor positions are not fully captured in that log.

Finite finish line: real-bake/actual-source pre-change reproduction or decisive
separate defects, shortest-route oracle and repeated-step progression with
multiple solid actors, necessary continuation/lifetime checks, full build/smoke,
docs and combined delivery. Research/code/review estimate45-60minutes; report
uncertainty/overrun before expanding into closed navigation systems.

Initial source finding: find_across stops at the first popped arrival on the
destination floor, before accounting for its final within-floor distance. That
does not prove the shortest whole route. Its remote-goal path also replans each
request, while local find can retain a still-legal walked suffix. These are
investigation findings, not yet proof of the exact dynamic NPC sequence in the
user's log. Do not claim the logged reversals are reproduced until tested.

## Pre-change evidence and refined contract

Terra's actual-source independent indoor tile-graph fixture covers14 connected
Amur floors,24039 tile/layer nodes and5080 directed edges.128 sampled starts
match the shortest whole-route oracle; a static23-step first leg makes steady
progress. The alleged first-arrival-cost issue is not demonstrated in this
topology and is not being changed. It is a separate source finding, not authority
to refactor cross-floor search. Initial home-pose tests found no reversal in104
legal changed-pose pairs (44 moving both actors). Those negative results remain
valid for their sample, not the NPCs' full native patrol area.

Sol then verified the missing native area contract. MoveType2 is Area;
FieldNonPlayerAreaMoveController.WithinRange710570 checks its supplied rectangle,
and SearchDestPositionListInRange70FE80 advances current position by its direction
and MoveStep. Manager.Factory712550 uses GetPropertyMapArea(linkObject). Native
MoveStep1 is not a one-tile radius around the spawn. In Map_20200, actor38 links
MoveArea2 (anchor24,10;224x352pixels), actor39 links MoveArea1 (6,10;272x352).
The fixture uses only safely interior, walk-reachable cells, avoiding uncertain
rectangle-edge/half-cell inclusion. Native bodies/provenance remain outside Git
under parent tools/npc_area_20261027/ff3; extractor --npc-area. Factual master
objects remain in parent tools/mapdump/ff3_objects.jsonl.

The revised bounded fixture finds a decisive legal two-NPC loop: actor38 stays
at(29,21,L2); actor39 moves(14,20)→(14,21)→(14,20) within its native area. The
player steps from(16,19,L2) to(16,20). The original remaining forward path is
clear and next steps to(17,20), but a new snapshot sends the player north back
to(16,19). Moving the NPC back sends the next route south again. The changed
snapshot's instantaneous shortest oracle distances are45 at the original player
cell and46 at the next cell, so the reverse is a reaction to transient geometry;
it does not mean the original forward route became illegal. Keeping a clear
published path matches the accepted local-route policy and the user's priority
of stability before efficiency.

Patrol evidence: build_check/npc_oscillation_20261027/patrol_report.txt. Search
stopped after7 initial plans/27 candidate pairs/6 legal suffixes, one immediate
reverse and one confirmed return toggle. Actor39's constrained home flood has149
cells.74.679ms aggregate offline, wrong-owner0; no native/frame/audio claim.
Actual logged actor positions were not captured, so this is a controlled
reproduction of the failure pattern, not an exact session replay. Canonical
patrol_build passes8291 names/0 and142/349/0 wiring. The original home fixture,
wrong-layer attempt, intentional missing-witness exits, known route_worker stops
and Windows near-macro compile error are retained separately; none is accepted
as a fixed-production result.

## Authorized implementation boundary

Sol supplied the native/geometry contract and reviewed the witness; Terra owns
the bounded baked_nav.cpp change, actual-source fixture and build registration.
Retain an indoor remote foot leg only for the same exact pending objective,
current map/mode/settings and valid incoming physical-exit receipt, with the
player still on its published path. Validate current solid NPC/chest/gate/party,
directional, trap and active transition constraints for every remaining edge.
Only its known final first-hop exit may terminate the leg. Changed/blocked/
unavailable/off-route/cancelled state returns to the normal planner; no blocker
is removed, softened or crossed. Keep the native action/facing and pending
destination through the actual map handoff. World/vehicle plans and the existing
local retention design remain unchanged. No hook/capture/input/native write.

This intentionally preserves a still-clear route when a patrol briefly opens a
shorter alternative. It does not claim to predict future NPC movements or forbid
legitimate rerouting when the committed route is actually blocked. Initial
same-snapshot shortest-route checks and current collision validity remain
separate from that continuation policy. Source acceptance is recorded below.

## Source acceptance, October27

Sol reviewed the completed bounded implementation after the user stopped further
delegation. Terra implemented the supplied contract; no additional delegation is
authorized for the remaining maintenance. `remote_door_candidate` restricts the
change to ordinary indoor walking GotoMap legs. `keep_remote_foot_leg` validates
the objective, physical incoming receipt, current player suffix and every live
edge before retaining it. Invalid ordinary legs are cleared before fresh planning,
including unavailable/empty goals. Local riding vehicles and trap continuations
retain the established planner. Native Player rows and remembered transports
from another map correctly count as walking. No cross-floor search rewrite.

The controlled before-case reverses; the accepted production fixture continues
all23 remaining steps with no reversal. Changed targets, receipts, policy/mode,
blocked suffixes, active events and cancellation are checked. The independent
whole-route oracle and original home-pose samples still pass.100 valid helper
checks take2.517ms aggregate;20 fresh production requests102.347ms;100 cancelled
requests6.549ms. These are separate fake-native offline episodes, not comparable
game-frame or audible-latency measurements. Wrong-owner0, failures0. No added
native hook, snapshot job or game-thread work; continuation validation is bounded
by the already published remaining route and existing cancellation checks.

Canonical final_build.txt passes8291 names/0 missing,142 used/349 defined/0
undefined wiring and all registered checks. final_smoke.txt passes47 exports.
Evidence: build_check/npc_oscillation_20261027/; final_report.txt preserves the
actual-source output. DLL11066880 bytes, SHA256
6cd48ce15560f3dc03c95ac94fdca6bc2011c0cadd35f40da545a23e4df5b921.
Intermediate fixture compile errors, moved-snapshot test correction and initial
Player-row omission remain recorded in earlier build outputs. Sol verified the
final walking-context correction. Gameplay, cumulative native-frame cost and
audible response remain unmeasured. Installation is part of combined delivery.
Other Closed navigation systems remain Closed; the unrelated first-arrival-cost
finding is recorded without changing that algorithm.
