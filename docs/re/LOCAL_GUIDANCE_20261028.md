# Nearby walking routes and repeated interactive objects

Human scope October28: include these two small corrections with the urgent
[cross-floor NPC repair](CROSS_FLOOR_NPC_20261028.md), deploy FFIII, then resume
the [full map audit](MAP_AUDIT_FOLLOWUPS_20261028.md). Sol only; no game probe.
Initial estimate20–35minutes exceeded while correcting fixture assumptions and
rebuilding the shared offline programs; overrun reported. Source/delivery Closed:
full checks/smoke/scan pass and FFIII installed. Existing Closed
vehicle/label designs remain intact apart from these explicit follow-ups.

## Nearby walking policy

`field/baked_nav.cpp::may_shorten_with_vehicle` now declines optional vehicle
comparisons when the successful ground route is under15 tiles, or a legal final
approach is under15 wrapped Manhattan tiles away. A ground route must already
have succeeded. The proximity clause matters for the actual Saronia coordinates:
from83,62 to91,65/74,64 the legal terrain routes are17/19 tiles, despite both
entrances being11 tiles away geometrically. A strict actual-route-length-only
guard would leave the user's exact examples unchanged. Required crossings retain
the existing no-ground-route fallback; an already-boarded vehicle keeps its own
movement rules. Explicit vehicle targets keep their own boarding contract.

`find_path` also revalidates a retained optional flight near its target against
the current legal ground route. This uses the existing cancellable owned-data
search, with current gates/NPCs/terminal transitions. No geometric shortcut can
declare a blocked walking path open. Vehicle identity changes already invalidate
retained plans. No new native getter, hook, capture or game-thread search.
The rare retained-flight check may add one worker search; total native/frame/
audible cost remains unmeasured. Initial fixture assumptions omitted canoe
ownership and incorrectly required a separate landing phase from the existing
boarded-aircraft path; failed reports are preserved as test corrections.

`tools/navigation_followup_check.cpp::check_saronia_short_walks` uses actual FFIII
bakes and production full/browse planning. It checks both reported northern
entrances, walking despite an adjacent aircraft, retirement of a retained flight,
and changed guidance after boarding. Existing all-six Ship/shore and FFII
Palamecia fixtures preserve required-vehicle, missing-model and longer-shortcut
behavior. No game execution is involved.

## Repeated interactive objects

`entity_scanner.cpp` captures each interactive object's original translated
label, rather than its previously numbered output. `entity_filter.cpp::build_impl`
feeds exact duplicate labels to the existing `npc_names::renumber` service in a
separate object group. Books become Book1 through Book9; a unique Switch stays
Switch. NPCs and objects cannot consume each other's suffixes. Each map numbers
independently; source geometry provides stable order before category/distance/
reachability pruning. Native identity, route target and selection stay unchanged.
Numbers append to translated labels; no untranslated phrase or new setting.
Existing NPC developer-ordinal cleanup is unchanged; meaningful digits in an
object's label are not stripped. No new native read or callback. The existing
worker adds a bounded grouping/sort over the captured object rows.

`tools/entity_filter_check.cpp` checks nine Books, a unique Switch, two Books on
another floor, a same-named NPC, pruning, distance resorting and repeated builds.
Gameplay confirmation and actual native/audio timing remain separate.

Delivery: FFIII only,11099136 bytes, SHA256
de3e075d6bf3a089f2a8fde529ccbb8fc75c67f68c1a31a85ed1374eb0f211c9.
22 scanned installed hashes match;144 protected files and other five games'
payloads unchanged. Full build8318/0 native names,142/349/0 wiring, smoke47
exports,67 exact embedded resources,136 registrations,12 languages/six unchanged
bakes. Defender no threats/matching detections, unchanged staged hashes;
exclusions OS-denied/unknown, no signing identity. No game launched/probed.
Recovery, failed/accepted fixtures and receipts:build_check/short_walk_20261028/.

## Build memory preference

**Owner build sequencing update, November 6.** FFPR shares one build across
all six games. Only one version of this mod may build at a time across both
machine slots and all chats. Batch fixes that are close to ready into one
verified build and deployment. Finish a specifically requested game-first
deployment before starting the queued work.

**Owner CPU update, November2:** multiple cores are allowed, never unlimited.
Future local jobs use min(25% machine, three logical cores' equivalent),25% on
this12-thread host. Both slots can use at most50% CPU.2GB aggregate per tree,
admission, priority, cleanup and serial compilers remain. Active jobs keep their
original settings. This explicit local owner override supersedes only the
maintained runner's old single-core CPU-share default. Real child verification:
build_check/cpu_allowance_20261102/; all14 real guard checks pass at the new cap,
including two admissions/third refusal and aggregate child memory/cleanup.

**Current owner memory update, November1: two shared slots,2GB per complete tree.**
The maintained runner/checks were merged November2 UTC after the existing Ghido
1GB job completed unchanged (99.656s/441663488B/exit0/drained). Each slot admits
one complete tree with a2,000,000,000B hard aggregate cap, including descendants;
both slots can use up to4GB combined. Child workers remain serial/below-normal
and CPU bounded. No shared compiler/MSBuild servers, node reuse, /MP or project
parallelism. Independent projects may occupy both slots; no uncapped retries or
admission bypass. Larger target-specific budgets still need explicit owner
approval. Existing game/deploy/installation/publication permissions are unchanged.

The exact maintained runner/check hashes and the only local delta are recorded
in build_check/resource_slots_20261102/source_merge.json. The local delta retains
the already-qualified AppContainer namespace refusal before any child starts.
Use the normal user context via require_escalated here, without administrator
rights; actual job membership/limits are verified. Slot one keeps
Global\Codex.Build.ResourceBudget.v1; slot two appends .Slot2. Active old jobs
remain in slot one under their prior smaller limits. Future commands use
scripts/run-build.py directly, with optional finite --wait-seconds; the old
wait_resource.py is historical and is not the new admission route.

All14 maintained real guard checks pass: small64MB process/child/aggregate
allocation denial, page rounding/minimum verification, inside/outside-job
verification, two admissions/third rejection and descendant cleanup. The2GB-
default check allocates no large heap. Final guard report and source identities:
build_check/resource_slots_20261102/. Existing .NET property/concurrency settings
were preserved and the enforcement message updated; native CMake single-worker
settings remain. Preserve prior1GB receipts as valid evidence under their actual
budgets. Resource-throttled timings do not establish game responsiveness.

### Historical one-slot adoption and host evidence

October30 owner replacement applies globally across chats/projects: one build
tree, default hard aggregate1 GB, one worker/node, bounded CPU and below-normal
priority. Explicit approval is required before any necessary increase. Longer
builds are acceptable. Expensive bakes/extraction/tests/probes/benchmarks use
the same serial enforced runner; no unchanged multi-GB background jobs.

The authoritative [contract](C:/Games/Dev/codex/templates/accessibility-mod/references/build-resources.md)
and portable runner are adopted as `scripts/run-build.py`. The supplied
`Directory.Build.props`/`.targets` were added because
neither file existed, preserving CMake/project settings. They disable shared/
Razor compilation, MSBuild servers, node reuse and parallel work, and check
actual kernel containment. Native /m:1 /nr:false /MP1 remain in force.

`build.bat`, `smoketest.bat` and `deploy.bat` wrap their entire operation and
verify an existing job before nested execution. Direct `build_one_check.py`
also verifies. A global named mutex excludes other chats/projects using the
maintained runner. The child starts suspended, receives and verifies the kernel
job, then resumes; its descendants inherit the same aggregate quota. No debugger,
injection, administrator rights or global system-settings change is required.
The local temporary debugger-based limiter was retired before adoption; its
source/failed/cancelled development evidence remains under build_check/.

The maintained contract defines the ceiling as total committed memory, rounded
down from1,000,000,000bytes to pages. CPU is capped at the smaller of one logical
processor or25% of the machine; this machine's receipt is8.33%. Lower priority
and smaller GC limits apply alongside the hard job quota. Failure/cancellation
drains descendants before releasing the global lock. No automatic increase,
uncapped fallback, omitted required check or unrelated-process termination.
Windows raw peak counters may include refused allocation attempts; preserve
them as counters, not a claim that refused memory was available. See
[Microsoft's job-memory definition](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_extended_limit_information).

All11 maintained real guard checks pass under64 MB: normal/page-rounded launch,
inside/outside membership, process/child/aggregate allocation refusal, over-budget
refusal, global busy exclusion and descendant cleanup. The project check's only
adaptation keeps receipts in build_check/resource_guard_checks/ because the
host's sandbox TEMP path rejected receipt writes. No test allocates1 GB.
The first actual compile ran under1 GB/8.33% CPU, with observed compiler private
memory227.2 MB and resident237.4 MB. It was cancelled after a stronger cross-tool
global-exclusion check failed; no new fixture outcome is claimed.

Host-specific correction: AppContainer isolated even the Global mutex between
tool invocations. A separate invocation acquired its own lock while our compile
was active (TokenIsAppContainer1). Same-parent busy checks therefore did not
establish cross-chat exclusion. `api()` now rejects AppContainer before any
child starts. Execute bounded commands in the normal user context here via
`exec_command`/`require_escalated`; all quotas remain, and no administrator rights
or machine-settings change is required. Normal-context probing found the real
global lock busy and launched no child. Do not terminate that unrelated job.
Outside-context requalification passes all11 checks, and a separate normal-user
tool invocation observes busy258 while a45-second64-MB holder is active. That
holder exits0 with childrenDrained true and~7.5-MB raw peak. The earlier30-second
holder was already finished before observation; do not count that missed timing
as either an exclusion pass or failure. The local refusal
is the only runner logic difference from the current maintained copy; its
concurrent GC-default update is merged without changing the aggregate ceiling.
The maintained package itself has not been overwritten. Evidence includes
build_check/build_memory/{appcontainer_refusal,normal_user_probe}.json and the
preserved cancelled compilation report.

All new fixture wall times must be labeled resource-throttled; prior valid
source-matched evidence remains valid. Neither resource-limited wall time nor
earlier unconstrained offline worker time proves native/audible responsiveness.
This changes engineering tools only; nothing is added to the game payload.
