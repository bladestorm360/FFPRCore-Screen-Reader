# Resource-bounded build speed — startup slice source/focused Closed

November2 combined custom-audio qualification exposes a separate dependency
cost: field/field_api.h includes audio/soundscape.h for the Snapshot reference
declaration; baked_nav.h includes field_api.h, and fixtures/route_headers.h
precompiles baked_nav.h. Adding custom Options to soundscape.h consequently
invalidates the routing PCH and many navigation translation units, in addition
to the earlier navigation/header changes. Source locators: field_api.h7/24,
baked_nav.h8, route_headers.h26. This is an observed dependency, not a measured
speedup or reason to increase RAM. The current frozen205-target qualification
continues unchanged. The accepted startup slice is not reopened; no automatic
new refactoring task is created. A future human-triggered build-dependency
slice can evaluate a forward declaration plus direct includes at actual full-type
users, with pre-change invalidation evidence, compile/ABI checks and measured
whole-build comparison. Do not claim that change is implemented or safe from
this inspection alone. Current custom topic records finite progress/overrun.

November2 owner update: multiple cores are allowed, but unlimited CPU is not.
Future local jobs use a hard cap of25% of the machine, further bounded to three
logical cores' equivalent. On this12-thread host that is25% per tree, at most
50% for the two shared slots together. The2GB aggregate memory ceiling, serial
compilers/MSBuild, below-normal priority and no servers/reuse remain. All14 real
small-heap guard checks pass with the new allowance, including two admissions/
third refusal and descendant memory/cleanup checks. Exact source/report receipts
are in build_check/cpu_allowance_20261102/. The frozen IV build kept8.33% until
its2400s finite limit (998813696B peak, exit1/drained), reaching group154–156/167.
Its evidence/compiled outputs are retained; an unchanged-input resume now uses
25% and reruns every canonical check, smoke and scan. No source/check omission
or memory increase. This removes the single-core CPU-share
restriction for new jobs; measured speed and previous receipts remain separate.
The rest of this document records the settings used by each earlier experiment.

Implemented in main tools/build_projects_serial.py, leaving the active isolated
FF1 candidate untouched. Default groups contain at most three generated projects
in one serial MSBuild process, releasing retained state between groups. Group
sizes1/2/3, a residual group and actual first-failure propagation all pass, with
identical already-built executables. The same2GB/one-worker/CPU/priority/server
limits apply. Source/receipt: build_check/build_speed_20261102/builder_qualification.json
and builder_qualification_resource.json. Full cold/whole-build timing remains
separate; the demonstrated gain is warm startup overhead, not half of every build.

Owner wants routine builds back to minutes without exhausting the PC. Preserve
two shared machine-wide slots, hard2GB aggregate per complete tree, one compiler/
MSBuild worker, bounded CPU/below-normal/no reuse or shared servers. Current
isolated FF1 candidate remains unchanged. Its151-target full rebuild is not a
routine incremental-time target. No increased CPU/RAM budget is inferred and
no active job is altered. Original scope ledger records priority/remaining work.

## Initial factual measurements

Current canonical helper launches CMake/MSBuild separately for every generated
ALL_BUILD project. Many offline executables also compile the same check-support
sources again. Header census finds baked_map.cpp104 times, baked_router.cpp103,
map_state_program.cpp98, and several other common sources88–109 times in the
current main check declarations. These are repeated work opportunities, not a
measured speedup claim. The large included private route fixture is another
cost; do not redesign its accepted feature boundaries or weaken real checks.

Three actual already-built main-tree targets (Ice route, native autowalk, shared
route worker), same enforced job and ABBA order: separate invocations4.284/4.060s;
multi-target CMake4.016/3.961s.
Both approaches still start MSBuild three times. Combined CMake target syntax
alone does not remove the material startup repetition, so it is not adopted as
a claimed fix. All measured executables stayed byte-identical in this experiment.
Receipt26.296s/95318016B/slot1of2/2GB/8.33%CPU/exit0/drained. Source/reports:
build_check/build_speed_20261102/startup_measurement.json.

A first direct solution invocation starts one MSBuild process for three targets but
took6.097/4.352s and changed an offline executable hash. This is not accepted as
a warm identical-artifact speed comparison or production change. The first tool
lookup assumed CMAKE_MAKE_PROGRAM existed; this Visual Studio cache omits it.
Using the verified installed MSBuild path completed the runs, then the artifact
guard rejected the comparison. Failed receipts/logs remain; active delivery and
all game payloads are untouched. Neither initial recipe was adopted; the
exact-host implementation below supersedes this investigation.

## Accepted exact-host project grouping

The exact existing CMake driver command identifies amd64/MSBuild.exe and
VisualStudioVersion18.0. The rejected direct experiments used the installed
x86 host instead. Matching CMake's actual64-bit host/version makes the exact-
project task comparison pass unchanged-artifact guards. ABBA warm results:
separate4.110/4.102s versus grouped2.145/1.941s, same3 targets/one worker/cap.
Combined test receipt16.617s/97140736B/exit0/drained. This demonstrates roughly
half the warm startup time for this three-target sample; it is not a cold or
full-build responsiveness measurement. Earlier failed reports remain intact.

Production helper now reads the configured Visual Studio instance/version,
uses that exact64-bit host and builds each group serially with /m:1 /nr:false,
BuildInParallel=false and StopOnFirstFailure=true. Every generated dependency
still builds; no check/analyzer/feature is removed. The group bound is1–3, with
1 retaining the old per-target behavior. Existing canonical and clean entry
points stay valid; --targets permits only distinct existing generated targets
for focused development. Each invocation retains its owned project-group XML
and records group elapsed time; no active-job/source/output directory is shared
with the isolated delivery.

Actual helper group3, residual group2+1 and historical group1 keep all three
warm executables byte-identical. An actual MSBuild failure fixture stops before
later projects in the same group or the following group. It uses original
owned metadata and the real installed MSBuild, with no game launch or code.

## Limits and remaining measurements

Small groups use exact generated project files through a serial MSBuild task,
rather than loading the entire solution or repeatedly spawning processes.
Microsoft documents that this task shares the existing process and built-target
cache; BuildInParallel=false keeps project execution serial. Reference:
https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild-task
Exact-host/settings/outputs, warm timing, failure behavior and bounded cleanup
are qualified. The first later required cold/whole candidate build supplies
its own timing/memory evidence; do not force a needless cold rebuild now.
Group retention is bounded; never retry an OOM uncapped.

If startup is a small share of cold time, measure/reuse identical common check
compilations with their exact macro/option/registration contracts. Do not assume
all private route fixtures can become one interchangeable object. Keep the
runtime DLL and required affected/full shipping checks intact. Existing focused
development mode already avoids unrelated full builds; improve that ordinary
path and report actual DLL compilation, affected checks and full release
qualification separately. No promise of a two-minute cold full151-target build,
nor use of an hour as the normal expectation. The main startup helper is now
qualified for future builds; current Levistone delivery keeps its frozen recipe.
Current runtime/feature fixes remain preserved independently.

## Repeated common check compilation — source/focused Closed

Owner's build-speed request also applies to the next required full navigation
candidate. Damcyan's changed browse target took79.575s with the same thirteen
common check sources repeatedly compiled by independent executables. CMake now
builds those exact original sources once as offline-only object libraries and
links the COFF objects into each existing check. Each executable still has its
own globals/thread locals; there is no shared runtime/server or production DLL
architecture change. All existing checks, source inputs and serial compilation
remain. Native production libraries keep their original separate compilation.

Before building consumers, the qualification compares every generated Release
compiler property for all13 object projects with the existing browse target;
only object/PDB output locations are excluded. Definitions, include paths,
runtime/exception handling, optimization and compiler options match exactly.
There is no PCH, unity build, worker increase or budget change. Existing mixed
MinHook object expressions retain their prior wiring.

Three actual consumers pass: route32.527s, browse42.007s (39.698 inside build.bat),
navigation follow-up52.546s. These include first fills of the new object cache
and a changed shared routing fixture; they are not an ABBA full-build benchmark
or a promise of a particular cold time. The proven saving is removal of repeated
identical common compilation; unchanged linked objects are reused on later
consumers. Evidence/contracts/reports: build_check/damcyan_20261102/
shared_compile_contracts.json, shared_consumer_timings.json and
shared_objects_resource.json. Source/focused wiring is Closed; the required full
isolated candidate supplies combined linkage/build/behavior/resource evidence.
The earlier startup grouping remains unchanged and Closed.

## Unused wrapper entry — source/focused Closed

All53 direct wrappers own their entry point and never call the renamed base
entry. The existing preprocessor guard now excludes that unused body when main
is renamed, retaining the full standalone suite. Actual browse assertions and
the standalone base suite pass. Same-TU ABBA forced recompilation with cached
common objects measured55.557/50.495s before and48.767/45.004s after: means
53.026→46.885s, about11.6%. Whole guarded experiment288.940s/518279168B,
exit0/drained. Evidence: build_check/damcyan_20261102/entry_guard/. These are
throttled fixture timings, not game responsiveness or a full-build estimate.

The first isolated Damcyan candidate reached its1800-second finite boundary
while still compiling checks: exit1/drained/512606208B. It was not qualified or
deployed. Its source, partial build log and reusable outputs remain preserved;
the next candidate must finish all canonical checks, smoke and exact-payload
security verification before installation. Limits were not increased.

## Headers-only compiler reuse — source/focused qualification

The completed first-header-change FF1 candidate took1553.620s canonical and
1573.499s/800321536B for build/smoke/scan. It filled the retained cache and was
installed successfully; this remains too long for routine changes. Existing
startup grouping,13 shared objects and unused base-entry guard stay unchanged.

A finite full-fixture precompiled-header trial failed at linking duplicate
mutable fixture definitions. It automatically disabled the cache, retaining
the failed logs and103.718s/490332160B/exit1/drained receipt. No runtime code or
installed payload changed. The distinct successful approach caches only common
standard/owned header declarations; every test retains its own state/functions.
Actual same-source ABBA means40.578s before/37.627s after, about7.3% saving;
first fill3.325s is separate. Actual browse, Kaipo/remote and physical Ice
consumers pass; whole experiment256.896s/481107968B/exit0/drained under the same
2GB/oneworker/8.33%CPU cap. This is a modest fixture-build saving, with no claim
that it alone makes a full cold build take minutes or establishes game latency.

Installed exact CMake help documents REUSE_FROM's identical compiler requirement.
Generated Release contracts match for76 compatible offline consumers. CMake
excludes access/SDK-macro-changing fixtures and leaves native production targets
alone. No shared compiler server, PCH of game source, increased parallelism or
removed check is introduced. Exact reports/timings/contracts/receipts live in
build_check/route_pch_20261102/. The next scoped IV canonical build must qualify
the expanded actual consumers before installation. This completes the finite
cache investigation; do not continue speculative cache experiments internally.
