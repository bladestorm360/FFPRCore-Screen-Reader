# Dreamscape train: source/focused checkpoint

Part of the resumed all-six map/story audit in the
[single ledger](MAP_AUDIT_FOLLOWUPS_20261028.md). The full train family is now
source/focused Closed. Combined canonical/smoke/security/deployment and natural
gameplay remain pending. No game was run. Earlier partial checkpoints below
remain historical evidence, not the current completion status.

Final family checkpoint:9 total story guides/148 rules,13 train instructions in
all12 locales;173 model states/187 references. Ten first-car episodes, eight
actual-type maze episodes and seven native arrival/departure profiles pass94
production routes/three movement refreshes. Preserved64 chest-pattern episodes
and16 layout episodes pass762 more route legs. Native-owner/controller lifecycle,
Phoenix/Tower, Opera and all35 static controls also pass. Native contracts,
language catalog and stable recipe writers pass; the four new checks are wired
into build.bat. Frozen36 inputs/reports/receipts:
build_check/puzzle_audit_20261030/dream_train_family_checkpoint/.
Shared integration124.096s/499576832B, exit0/children drained; all work stayed
inside the enforced serial1 GB cap. These timings are resource-throttled fixture
evidence, not native frame, audible-response or countdown-input measurements.

Exact matching FFVI scripts and objects remain external in ../tools/mapdump/.
The narrow source reader is in ../tools/puzzle_audit_resume_20261031/, outside
every Git tree. Only factual identities, hashes and original mod/tool code are
kept in this repository.

Established facts:

- Room524's crate lever changes local0, starts a four-second native timer, and
  resets on timeout. Crossing its actual success trigger106 records local0=3
  and hides the lever/trigger. A route merely reaching the lever is insufficient.
  Directions for this timed part must be available before starting it.
- The first car displays the six-chest example; the next car's six controls
  toggle locals1..6. Exact verification of all64 native check branches gives
  one success: locals0,0,1,1,1,0 (bit mask28), then the checking lever sets448.
  The equality comparisons in the script are rejection branches, not the
  desired answers. The opposite pattern is incorrect.
- Room525's right/middle/left controls41/32/26 toggle445/446/447. All24 exact
  toggle/initializer comparisons agree for all five position-controlled wall
  actors. The native first entrance is Ground29,13. A bounded original
  eight-layout planner finds the chest-check control from every lever state;
  the initial state needs six toggles. It uses actual projected native geometry,
  preserves the player's exact approach, and executes no native script.
- Room527's locomotive controls only animate and do not move the maze walls or
  gate the next map. Do not invent a locomotive lever combination.

First-car source/geometry checkpoint: the sole legal lever approach is30,8.
The first pull moves physical crate28 from27,8 to27,9, but crate31 remains at
29,8. The actual router finds a24-step loop to success trigger106, not four
steps west. The reviewed native second-pull branch explicitly finishes the
four-second timer, sets local0=2, restores crate28 and moves crate31 to29,9.
It hides the lever and leaves a permanent layout with one-step access to
treasure9 from29,8. An untimed25-step route to auxiliary success106 also exists;
its local0=3 receipt is not required to obtain the crate chest. Guide progress
therefore uses the actual treasure200 receipt, preserving completed loot after
map-local reset. The complete two-pull instruction is announced before pull
one; a running timer asks for the second pull, while timeout returns to the
first instruction. No four-second movement race is authored.

The native observer verifies timer start/stop, both physical positions, an
inert extra pull in state2, success from state2, an unstarted crossing, timeout
and retry, alongside the preserved24 maze comparisons/64 chest patterns. The
current geometry prototype passes both the timed and permanent layouts.
Receipt train_first_car_locked_budget.json:38.156s/438972416B, exit0/drained.
Native moving-chest positions are16/17/18,10 (the earlier generic moving-object
guard also used valid test cells on row8; that was not native placement proof).
The next current first-car episodes use actual actor types and row10, including
movement-scene refresh, both arrivals, timeout and saved receipts. Authoring now
covers522–527 through native departure event527/8 to528/intro449;13 instructions
cover all12 locales. Full model/episode/controller qualification is still open.
An expanded model test exceeded its default Windows thread stack. Reusing one
result alone did not cure the aggregate frame; the new cases now run in a
separate sequential worker with the unchanged default stack. A moved local
helper briefly caused a compile error and was corrected at the shared fixture
scope. Failure receipts are train_full_model_budget.json,
train_car_integration_budget.json and train_car_split_stack_budget.json; these
are fixture failures, not game or1 GB RAM-limit failures. The successful
train_car_split_stack_retry_budget.json records92.546s/462966784B, exit0/drained.
It passes173 model states/187 references, ten authored first-car episodes,
50 production routes/three native movement refreshes and the preserved762 maze
legs. Frozen23 inputs: dream_train_first_car_checkpoint/. Actual-type maze and
whole-train arrival/departure profiles, then controller/shared integration,
remain active. No new installation or native probe.

Current evidence: build_check/dream_train_native_report.json contains24 wall
comparisons,64 patterns and source hashes. Targeted receipts/reports are in
build_check/navigation_resume_20261031/dream_train_* and
build_check/dream_train_report.txt. All eight pending-maze initial plans and
their actual route-consumer approaches pass. The first completed-exit prototype
omitted the selected closed doorway footprint from its flood; eight abstentions
exposed that fixture/feature error. The planner now supplies that native goal;
the completed-state rerun passes16/16 initial plans. Full ordinary-consumer
replay now passes16 complete maze/exit episodes and58 actual route legs,
68.516ms total under the enforced CPU quota, native-owner violations0. Unknown
state and immediate cancellation abstain. No change to the shared route planner
was needed: its actual approaches match all selected safe maze approaches.
Receipt dream_train_episodes_budget.json; source/output checkpoint below. This
is an undeployed new-family prototype,
not eight reproduced failures in the installed mod.

Remaining finite acceptance: authored/controller-level episodes through
chest/reset and all train arrivals/exits; timed-crate preparation/timeout/retry;
complete entrance-to-next-map authoring; localization; cancellation/unknown
state/cost bounds; integration with preserved story guides and canonical checks.
The broader inventory also needs an animation-plus-local-state discovery pass:
the generic-named chest controls were not selected by the original label/geometry
candidate pass. Preserve its1634-candidate baseline and record the expanded pass
separately instead of claiming that baseline was exhaustive.

Maze/chest integration checkpoint:9 total story guides,7 new instructions in
all12 languages;161 physical references/156 model states pass. All64 initial
chest arrangements replay the full maze, chest corrections, checking lever and
exit through the ordinary route consumer. Together with the16 layout episodes,
762 route legs pass (758.029ms aggregate under quota, not native game timing).
Controller/cancellation, all preserved multi-party and Opera episodes also pass.
Serial job352.222s, peak493961216B, exit0/children drained. Frozen source/reports:
build_check/puzzle_audit_20261030/dream_train_model_checkpoint/. Complete first-car
timed/moving-chest and exterior-to-engine authoring remains open; this is not a
claim that the entire train guide is complete. The next finite integration fix
is a demonstrated long mounted first-hop mismatch found in I/II/IV/V; preserve
this checkpoint and resume first-car authoring after it.

Additional exact first-car metadata: treasure198/item-control7 sets scenario444;
the timed-crate treasure has flag200/control9; moving-chest treasure is199/control8.
The moving interaction107 is staged at Ground15,-1 and native placement attaches
it to actor24. Actual-consumer snapshots reproduce three failures despite a
positively captured current Entity at16/17/18,8: the planner only rebound NPCs,
so it retained15,-1. A minimal shared correction now uses the captured position
of uniquely identified single-cell Entity/AnimEntity/TreasureBox interactions,
retaining actual type, raw identity, layer, native interaction mask and consumed
guards. No new observation/hook or native worker call. Wide-object anchor
semantics retain the established projected-rectangle path; no center is guessed.
All26 pose/identity/ambiguity/missing/invalid/retirement cases and six stationary native
object witnesses pass, plus16 full maze episodes,50 mounted first hops and the
shared98 boarded/582 exit regression. Opened ordinary chests remain navigable;
their treasure-opened bit is distinct from consumed script actions. An initial
test incorrectly expected otherwise; the corrected test preserves the actual
policy, and the real retired moving action remains unavailable. This moving
object integration slice is source/focused Closed, with combined delivery
pending. Before/final source/executable/reports and receipts are in
build_check/puzzle_audit_20261030/moving_object_checkpoint/.
