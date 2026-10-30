# Enter during EXP counting

## Scope ledger

User-initiated FFIII follow-up, source complete, Closed. Research establishes
the native input distinction below. NPC route oscillation and prior source work
remain Closed. Amur return-exit preview and
Boost display feedback remain separate queues. No game control/launch/probe.

Own screenshot and session-part7 log are in build_check/results_phase_20261027/.
The screenshot shows the Points/EXP panel:480gil/1500EXP, Rae20→21, remaining
EXP0. The matching log at256980.000 announces totals and publishes the Points
buffer, then shows no new native phase entry until257075.171 (Status phase2).
The user clarifies an Enter press may have finished counting without changing
the visible phase. Input timings cannot be reconstructed exactly from the log.

FFIII ResultPointController.PointsUpdate RVA6C7F20 confirms this behavior. After
native input, if perormanceEndCount equals performanceList.Count it completes
the page; otherwise it calls ResultCharacterListController.ForcedEndPerformance
at6C66F0. That method stops the tally coroutines, sets the completion count and
updates the displayed member rows. The panel remains Points. Source-bearing
exports/provenance are outside Git in parent tools/results_counter_20261027/ff3;
original exact-target extractor --results-counter. No native snippets here.

battle_result.cpp already hooks that completion. points_finished refreshes the
final review rows, but speaks them only with Autodetail enabled. A short native
forced-completion acknowledgement can cover the otherwise silent input without
inventing a phase advance or forcing all optional member details. Preserve
ordinary completion, native input timing, review controls and full Autodetail.
The bounded implementation adds a completion acknowledgement only for FFIII's
forced native tally completion with Autodetail off and an existing deferred
Points page. Early completion keeps the initial page announcement; normal tally
completion and other games keep their current behavior. Reuse the existing
next-frame final-row update, owner/generation cancellation and native hook.
No text deduplication, polling, input interception or new native observation.
Estimate15-25minutes through actual-source checks, localization/build and docs.

## Source acceptance at user budget stop

Sol implemented and reviewed the existing `points_finished` path and `dForced3`
producer. The sole added flag is captured by the already scheduled final-row
update; it adds no hook, frame callback or native read. Existing generation,
visible owner and pending-tally checks bound work and cancel stale feedback.
The pending flag is consumed before scheduling, so repeated native completion
cannot repeat the acknowledgement. Speech uses the existing noninterrupting
route and speech toggle. No native input/animation timing is modified. Early
completion lets the initial page headline/totals acknowledge the screen instead.

Actual-source `tools/battle_results_check.cpp` passes all-six Autodetail and
phase/lifecycle coverage, including the new FFIII forced completion, unchanged
normal completion, native-original exactly once, same Points phase, repeated
completion, early initial read and close/phase cancellation. All12 supported
locales contain the new EXP tally complete key, maintained by
tools/modtext_result_tally.py. Auto/manual language behavior is unchanged.

Canonical final_build.txt passes8291 names/0 missing,142 used/349 defined/0
undefined wiring and all registered checks. final_smoke.txt passes47 exports.
Evidence: build_check/results_phase_20261027/final_report.txt and build/smoke.
DLL11067904 bytes SHA256
cad99d69cff18c4e2ea4f0c1c77f100cd87b881c1e6e33873b89c6b3d63211e7.
No native/frame/audible performance measurement or gameplay confirmation; the
existing bounded callback does its existing final-row work plus one conditional
localized speech enqueue. Installation/combined scan remain pending. The user
requests stopping for usage conservation; all remaining work is retained in
MAINTENANCE_REMAINING_20261027.md. Other Closed source checkpoints stay Closed.

Legacy tools/re/results_calls.py still writes disassembly into docs/re and was
not run. Existing source-bearing history/publication limitations remain recorded
in CODEX_MEMORY; the new extractor always writes outside all Git trees. No
history cleanup or unrelated source-system refactor is authorized by this task.
