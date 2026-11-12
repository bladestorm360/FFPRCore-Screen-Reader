# Native FFPR accessibility mod

Owner hard rule, November1: check logs in the actual game directory for all
relevant gameplay information before asking the user. Never read/use tester
logs without explicit permission. Do not ask for normally undisplayed coordinates
when the game log supplies them. Open the current log's shared read/write/delete
file handle before trusting a directory listing that reports zero length;
inspect relevant rotated parts afterward. See ../../../AGENTS.md.

Hard rule reaffirmed by the user, October30: game source, recovered/decompiled
implementations, disassembly and implementation snippets must never be written
inside this or any other Git repository, even ignored paths, nor committed or
pushed to GitHub for any reason. Keep all such analysis in verified workspace
directories outside every Git tree; only factual locators/hashes/behavioral
documentation and original mod/tool code belong here. Preserve legitimate
third-party ownership. The user authorizes a history rewrite to remove old
violations while preserving releases; do not delete/recreate the remote. If a
GitHub permission error occurs, stop GitHub attempts without opening a browser
or starting an authentication workaround. The user will handle the alternative.

Active source for all six games is this repository, on `master`. FF1–FF5 belong to bladestorm360; FF6 retains ZKline/BlindGuyNW attribution and inclusion permission recorded in `../OPEN_ISSUES.md`. The older managed repositories are paused references. Preserve existing uncommitted work. User explicitly authorized 2026-10-11: after the full audit, deploy all six games, commit/push everything to the public user-owned `FFPR-screen-reader` remote (create if absent), and make the user-owned FF1–FF5 remotes private. Preserve old history and existing named remotes.

Read `CODEX_MEMORY.md`, the shared `../../../AGENTS.md`, and `../../../codex/MODEL_POLICY.md`. Sol handles research, implementation, documentation, review and verification itself by default (user update, 2026-10-27). No automatic Terra/Luna or other agent handoffs. Every delegation requires explicit user authorization, including Astra; its documented substantive-blocker/two-completed-different-approaches gate permits one permission request, never automatic dispatch. Summarize any authorized delegation, including outcome and remaining issues.

Current repository identity, October 13: local `FFPRCore-Screen-Reader`, branch `master`,
named remote `FFPRCore-Screen-Reader`, public user-owned
https://github.com/bladestorm360/FFPRCore-Screen-Reader. The user explicitly requested this
one-time rename with history preserved. For **prepare release X.X**, follow
[docs/release.md](docs/release.md) through README/TXT, full dependency bundle, 7-Zip
verification, commit/push and GitHub release publication. Do not repeat repository
renaming as part of that workflow. Installed dependency receipt identifiers are stable
compatibility data and are not repository names to migrate.

Prism 0.18.3 x64 is the required current speech backend. Keep every reader behind say/speech
and preserve speech/braille, queue/event identity, interruption, native readiness and worker
ownership. No new Tolk call sites or speech deduplication. Use the committed prism.lock.json
and matching SDK. Install dependencies only through separate `install_prism.bat [ffN]`;
normal builds/deploys must never download, install, copy, repair or upgrade Prism. Deployment
checks are read-only; an unchanged verified prerequisite install performs zero writes.
Detailed contract/evidence: `docs/PRISM_MIGRATION.md`. The user explicitly requested removal
of game-root Tolk.dll after successful migration deployment; retain recovery copies outside
the games, without deleting other dependencies or the preserved historical source.
The previous automatic Luna eligibility/routing rule is superseded by the Sol-only default.
For an explicit user-directed handoff, follow the shared policy for scope/file ownership,
supplied research/contracts, acceptance/offline checks and return-to-Sol conditions.
Preserve the current task, uncommitted work and all native/thread/lifetime/approval rules.

For a scoped local fix, use `build_local.bat CHECK [CHECK ...]` with the maintained checks for the changed behavior. Use `build.bat` for the complete regression build and packaged releases. Both build the same production DLL. Run `smoketest.bat` and the exact-payload security check before the separate `deploy.bat [ffN]` step. These scripts must never launch a game. See [local delivery](docs/re/LOCAL_GUIDANCE_20261028.md#november-12-focused-local-delivery). Gameplay testing and runtime probes are user-side unless explicitly authorized. Complete the current audit and offline checks before requesting gameplay tests.

Owner update, November 6: install authorized local fixes in the affected games
as soon as the build and required verification pass. No separate deployment
confirmation is needed. Keep the closed-game guard and ask the user to save
and close only when a running game prevents installation.

Owner update, November 11: commit repository-eligible work at substantive
checkpoints and task completion, including work awaiting verification.
Push checkpoints to the verified public `FFPRCore-Screen-Reader` remote owned
by bladestorm360 and verify the destination branch tip. Apply the source-content
gate before staging, check the staged content, and review every commit newly
exposed by a push. Follow the separate release workflow for packaged releases.

Owner build sequencing update, November 6. Use at most one shared slot for an
FFPR build across all chats, including separate folders and game targets. Batch
fixes that are close to ready into one build and deployment. If the owner asks
to deploy to a specific game first, finish its fix, verification and deployment
before starting the queued work.

Owner build update, November1: two shared machine-wide resource slots, each
one complete build/tool tree with a default hard aggregate2GB ceiling, including
children (up to4GB combined). This supersedes the one-slot/1GB default. Use the
maintained scripts/run-build.py; one worker/node per tree, bounded CPU,
below-normal priority, /m:1 /nr:false /MP1, no shared compiler/MSBuild servers,
node reuse or project/compiler parallelism. Independent mods may use both slots;
never bypass admission, retry uncapped, change active jobs or terminate unrelated
processes. A necessary larger limit requires demonstrated target-specific need,
the smallest justified budget and explicit owner approval. Bakes/extraction/
tests/probes/benchmarks use the same slots and finite enforced budgets.

Owner CPU update, November2: multiple cores are allowed, but never unlimited.
New local runner jobs use a hard25% machine CPU cap, further limited to three
logical cores' equivalent (25%/three on this12-thread host). Two slots therefore
use at most half the machine CPU. This supersedes the former single-core CPU
share only; aggregate2GB per tree, admission, below-normal priority, no servers
and serial compiler/MSBuild settings remain. Do not change active jobs. This is
an explicit local owner override of the maintained runner's older CPU default.

Host qualification found AppContainer isolates even Global mutex names between
tool invocations. The project preserves its pre-child AppContainer refusal over
the maintained runner. Use the normal user context (exec_command with
require_escalated here), retaining kernel memory/CPU limits and actual job
verification. No administrator rights are needed. Slot one retains the legacy
Global mutex, so old active1GB jobs stay unchanged; new jobs may use slot two.
Current adoption/14 small guard checks: docs/re/LOCAL_GUIDANCE_20261028.md#build-memory-preference.

Use name-resolved IL2CPP references and `hooks::attach`; verify signatures and folded native bodies against matching dumps/analysis. Preserve the custom MinHook coexistence changes. Hooks own UI timing; bounded deferrals start from native events. Game pointers need lifetime checks. Centralize services and use them at call sites; no new deduplication without explicit permission.

Replacement persistent preference, October 12: game-thread hooks may only detect state
or copy bounded ready native data and immediately continue the original operation.
Substantial speech/audio/logging/I/O/text/scanning/extraction/pathfinding work belongs
on workers or genuinely nonblocking snapshot coroutines. The game thread never waits
for workers/queues/locks/I/O/RPC/sleeps/joins, including startup/loading/shutdown.
Keep thread-affine APIs and native pointer lifetimes on their owner. Approved input/
movement actions remain isolated. Record ownership/evidence in docs/re/THREADING.md;
modded/unmodded runtime measurements remain user-side unless explicitly authorized.

October 13 responsiveness clarification: cheap hooks alone do not certify acceptable
mod performance. Measure player request through capture/batching, snapshot readiness,
worker/IPC queue, computation and first usable output, with relevant snapshot age.
Use evidence-based feature/platform budgets and normal/large/busy/cold/warm/rapid-action
checks. Prefer offline preprocessing, owned caches/live deltas and obsolete-event
cancellation; never weaken native affinity, reveal or speech-order contracts. Runtime
measurement remains user-side. Current [speech ordering/budgets](docs/re/SPEECH_ORDER.md)
and [navigation capture/compute evidence](docs/re/THREADING.md) distinguish compute,
fake-SDK first submission and actual audible/game-frame timing.

Speech interrupts once per new input/context; category/heading then initial focus and
same-event callback output queue through the central sequence/continuation services.
Never restore arbitrary speech delays or add text deduplication. Scope queued callbacks
only to known mod-owned operations, never around native game calls or on workers.

All navigable maps must have baked data. Missing bakes are extraction/data bugs;
never restore the deprecated native MapRouteSearcher, its distance cap, breadcrumb
fallback or temporary game-grid writes. Use the cancellable owned-snapshot baked
worker service for routes, floods and cross-floor planning. Current native instances
come from lifecycle/parent observations, never whole-scene searches on callbacks.
See docs/re/THREADING.md for ownership, capture budgets, evidence and validation.

Speak game information only when displayed, or under an explicit scoped user approval. Read active rendered fields, labels, comparison arrows and bar values rather than every parameter in a shared data class. Do not infer approval from old implementation. Results follow native phases and current displayed details; no early disclosure of later rewards. Keep Autodetail persistent, default off, and manual details/review available.

Scoped user approval, 2026-10-11: retain the optional Enemy HP Display modes and saved/default preferences in every game, despite HP not being rendered in native target panels. Numbers adds `HP current/maximum`; Percentage adds `remaining%`; Hidden adds nothing and must not query parameters. This exception covers enemy HP only, not hidden enemy conditions or other statistics. F5 cycles these modes during battle; the Battle submenu explains the disclosure. Scan/Libra follows the ordinary displayed-message reader.

Results follow-up, 2026-10-12: all six growth tables support Battle's changed-stats toggle
(default on). October 13 user correction: I/controller West reads every delta in the
current results phase, independent of highlighted row or automatic-summary preferences.
FF2 alone has nested weapon rows;
keep its full displayed weapon/level/gauge buffer independent from delta summaries and
Autodetail. User explicitly requests current displayed weapon progress comparisons using
the native before/after battle data. Scope that access to matching visible weapon icons,
levels and gauges; no unused/unequipped skill disclosure. Use native ExpUtility integer
operands and ceil only at speech formatting; FF2's verified minimum gain is 1% (16 levels,
20..100 point intervals). Native result updates own pending readiness; no 30-frame page
deadline. Read hit/damage digit sprites, without attack-sound or status-capacity arithmetic.
See `docs/re/RESULTS.md` for evidence, controls, validation and remaining gameplay checks.

Settings use Speech, Navigation, Navigation audio, Battle submenus, followed by Close Menu
(user replacement, 2026-10-14). Navigation contains autowalk, path speech, filters and
stick-click normalization. Navigation audio contains sound toggles and their adjacent
volumes, including the conditional EXP counter. Stat delta detail belongs to Speech.
Preserve saved keys/defaults, direct F8 entry and controller mode/context ownership.
Speak row only on native Formation/Order, from the actual rendered portrait placement;
never append it to other character/target/status menus. FF4 changes the formation as a
whole, so announce the resulting displayed row after the original operation.
For battle commands, if the screenshot has no description, do not expose one with I or
investigate hidden description data (user, 2026-10-14). Esper information buffers follow
the currently displayed panel; keep progress numerals alongside mastery icons and read
Equipped from either visible marker without a hidden owner-name lookup.
Esper buffers contain only the selected Esper's current panel; exclude the active
character header. Rendered unowned blank slots say Empty and replace prior information.
Native panel changes rebuild immediately without requiring cursor movement. Equipment
information uses the shared buffer alone, with one rendered name/quantity entry. Q's
description/stat-summary toggle is separate from the numbered equipment page controls.
All mod modal keyboard input requires active game-window focus, with held-key seeding on
return. Never add keyboard mod mode. Navigation must cancel at battle transition, preserve
live story gates and map state, route between floors to each transition, and explain blocked
routes without filtering away every useful destination.
NPC labels remove developer prefixes/suffixes (ASCII, full-width and circled numbering).
Only exact same descriptive tags on the same floor get sequential suffixes, e.g. Soldier1,
Soldier2; unique names have none. Use the central NPC naming service for live/remote entries;
number before distance/reachability filters. Preserve existing entity inclusion guards.

World navigation includes already-boarded vehicles: use the current riding grid,
legal native landing rules and walking/owned automatic-vehicle legs. World edges wrap
in search, floods, direction grouping and beacon corners. Vehicle identity changes
invalidate pending/preserved plans; all computation stays on the Navigation worker.
Current regression/evidence: docs/re/THREADING.md.

Shared autowalk and simultaneous beacon/directions are implemented in the core. Preserve
the three independent, default-off Navigation settings and the Backslash/P/Slash/controller
LT policy. Native movement/collision owns each tile; never drive a special flight controller
as a tile controller. Manual input and unsafe contexts stop the walk, and only a planned
matching floor transition may resume it. See `docs/re/AUTOWALK.md` for contracts and evidence.
Autowalk must physically face the interactible through the central native facing service
after arrival, including adjacent zero-movement routes. Preserve structured headings;
never parse translated facing speech. Floor continuation waits for native input and fade
readiness, retaining the final destination through asynchronous scans and every planned leg.
Build now includes `tools/autowalk_check.cpp` route replay across all six bakes. Gameplay
confirmation is still separate from offline verification.

Authoritative task/evidence records: `docs/AUDIT_2026_10.md`, existing `docs/PORT_STATUS.md`, `docs/INPUT_BINDINGS.md`, `docs/TESTING.md`, and `../tools/mapdump/`. Keep evidence and unresolved issues current; distinguish implemented, offline verified and gameplay confirmed.


October16 user clarification: treat a reported bug as potentially affecting all six
games. Audit the shared path and each applicable game adapter; correct every occurrence
found, rather than assuming FFII-only scope. Explain genuinely inapplicable screens.
The user subsequently lifted the testing hold: after finishing the current class-field
and exact-exit tasks, commit/push, rebuild and explicitly overwrite the existing V0.9.3
release. Preserve the original source/assets for recovery. Other bugs are deferred.
