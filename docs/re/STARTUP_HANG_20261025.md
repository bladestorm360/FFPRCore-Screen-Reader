# Boot scene fails to progress, October25

Current delivery: FFIII b2cacd6f/11103232 bytes includes the early-readiness guard,
bounded pre-first-frame singleton request evidence and refined same-three-hook
state/trace observer. Full canonical/offline/smoke/22-file security checks pass;
22 installed hashes match,144 protected/other five games unchanged. Evidence is
build_check/nautilus_access_20261028/; initial candidate0a6c18a3 was never installed
because the game guard refused it, then the human requested Nautilus corrections
first. The exact intermittent fault producer remains open; do not describe this
as a confirmed hang repair. The user's next natural launch can supply the missing
state/trace evidence without an agent launch/control/probe. Full map audit resumes
after this delivered checkpoint; Doga's Manor landing is also corrected.

Most recent human clarification: Dalg destination was already present and had
been misheard. Its reported transition issue is withdrawn; continue startup
diagnosis. No Dalg code/data change is authorized by that withdrawn report.

The next actual navigation regression is Doga's Manor: the user confirms that
Nautilus guidance flies through its entrance without landing/walking. Address
after startup, then resume the full audit. The earlier northern-Saronia boarding
fixture only asserted changed vehicle guidance after its landing assertion was
weakened; this missed the actual general-aircraft landing bug. Retain that as a
test-coverage correction, not evidence that the game's entrance accepts flight.

Latest human steering October28 explicitly reopens startup repair as the active
priority. The user reports many clean vanilla and MelonLoader restarts, with
this failure specific to the native mod. Accept that comparison as strong causal
evidence of native-mod interference; the unresolved question is the exact
producer/null object, not whether to believe the report. Pause the map audit
with source/evidence intact. After startup, fix Doga's Manor landing and the
parked-Nautilus Current map with vehicles list, then resume the full audit.
Dalg was found by the user and requires no change. No agents or game probes.

Finite startup checkpoint: locate the interfering native-mod producer, apply
the smallest correction preserving original game calls/exception behavior,
verify source-to-output and native ownership with actual-source offline hosts,
then serial full build/smoke/exact-payload scan and FFIII delivery. Initial
estimate60–90minutes; exact failed-call evidence may require a narrowly improved
existing diagnostic if retained logs/static code cannot identify it. Do not add
arbitrary startup delays, consume the exception, fabricate missing managers,
retry game initialization or reset native singleton fields. Existing navigation/
battle/input designs remain Closed except minimal actual startup integration.
Current cause investigation remains open (~35%); diagnostic refinement is
implemented and its actual-source offline check passes. No startup repair is
claimed.

An actual-source before/after check now establishes a specific unsafe native-mod
producer: input_transition_blocked queried FadeManager before rejecting Boot
with no main-game manager. A host getter model matching the native first-lookup
cache semantics reproduces2 failures: the readiness read performs the cold
lookup and subsequent native setup inherits its cached null after the prefab
exists. The minimal guard now rejects already-blocked Boot/loading before any
manager query. The stable-title fade path and direct settings exception remain.
Expanded actual battle_context_check passes0 failures across all-six state,
event UI, transition and title-fade cases. This fixes the proven unsafe order;
it does not establish that this path caused pid21320, whose first input frame
was later than its exception. Root-cause/gameplay confirmation remains open.

refs.cpp::instance also records at most16 native-mod singleton requests before
the first input frame in FFIII, plus their actual class/getter/result presence.
Native calls and returns are unchanged; no retry, search, reset, new runtime hook
or native pointer transfer. Logging stops after that phase/budget. The actual
producer fixture bootstrap_singleton_check verifies one original call, null/
present results, capacity, no logs in1000 running-frame calls, missing metadata
and other-game abstention. It passes0 failures and is in the canonical build.
This makes the remaining premature-lookup hypothesis observable rather than
claiming it from missing slow-scope records.

October28 source findings: failed pid21320's first input frame is AFTER the
exception, so an ordinary later frame callback is not evidence for the cause of
that particular failure. The earlier null ResourceManager snapshot likewise
does not prove it was the failing object: CreateInstance can fail earlier and
never reach resource setup. Its native pre-resource dependencies are FadeOut,
tracker/updater setup, SerialTypeManager, SteamSDKManager, ProviderManager and
SystemConfig. Direct-call comparison against the312 installed hooks finds only
the ResourceManager observer among its direct callees, and that boundary never
began. Deeper/earlier native-mod callbacks and bootstrap timing remain open.
No arbitrary delay, exception swallow, manager creation or static-field reset
has been added as a guessed repair.

The existing3-hook observer now copies ready FFIII-only evidence. Native
PrepareForThrow209F90 fills Exception._stackTrace0x38 before Raise20A840;
StackTrace.get_trace(911FE0→1E6240) consumes one MethodInfo pointer per IntPtr
array element. The observer reads only a12-method prefix, validates array length
≤4096 and code addresses against the loaded game module, and logs relative
method addresses. It does not walk OS stacks or call a managed trace/message
getter. This is not native_trace_ips0x80. Exact native exports/provenance remain
outside Git in parent tools/startup_hang_20261028/ff3_exception_native/.

Three game-manager watches (FadeManager, SteamSDKManager, ResourceManager) resolve
only immutable metadata before hook enable. Native owners then read the actual
static instance/lookup-completed storage, with valid/unavailable distinction,
at CreateInstance before/completed/fault boundaries. They never call a singleton
getter or change its cached state. The matching FFIII native singleton getter
searches once and retains even a null result, so premature mod lookups are a
concrete causal hypothesis to test, not an established producer. Required named
field offsets0/8 and verified Class.static_fields0xB8 fail closed. Other games do
not receive this unverified profile. Cached class metadata has process lifetime;
no thread-affine game pointer goes to a worker, only copied booleans/RVAs.

All new work stays on the existing event boundaries: no extra hook/timer/poll,
runtime launch/control, native snapshot tool or direct game operation. Existing
4-exception/process capacity remains; each prefix adds at most12 method records.
Normal construction adds3 before/3 completed scalar records. Actual-source host
checks original call counts/SEH and MSVC propagation, malformed/oversized arrays,
foreign addresses,12-prefix bounds, cached-null versus unsearched state, no getter/
metadata call or native-state write during observation, and other-game abstention.
It passes0 failures;100 independent fake-owner failure captures cost0.808ms
aggregate/0.028ms peak. That excludes real native game, logger worker/frame/audio
cost and does not certify the cumulative25ms application-lag limit. Full serial
build/security/FFIII installation are pending. This is diagnostic refinement,
not a startup repair. Root-cause completion still needs decisive failing-state
evidence and then a minimal producer correction.
allowed scope: dllmain/runtime_gate, startup observer and only a proven earlier
native-mod callback/dependency. Native capture/launch/control still needs its
existing explicit authorization; user log inspection alone is not that approval.

October28 human recurrence: the installed de3e075d build's failed session21320
started12:22:24 and now supplies decisive exception evidence. At startup tick
374334.281 the observer records managed NullReferenceException in
SceneBoot.CreateInstance, then the same propagating exception in SceneBoot.Start,
with the standard null-object message. ResourceManager.Initialize never begins;
no completed CreateInstance/Start is logged. Boot input frames continue through
frame2084, with no logger drops/write failures in the failed archived session.
These two boundary records describe propagation, not two separate failures.
The following session27160, started12:23:19 with the same installed DLL, completes
Boot/CreateInstance/resource initialization and reaches Title/MainGame/FieldMap.
The observer caught a managed setup failure rather than an established stopped
frame-loop deadlock. It does not identify the null object or establish whether
the mod caused the failure. Do not label this as a repaired startup issue.

Both whole sessions and the preceding gameplay tail are preserved under ignored
build_check/startup_hang_20261028/. The newest filename was transiently zero bytes
while rotation/startup occurred, so evidence was correlated by pid/start tick.
Installed payload hash remains de3e075d6bf3a089f2a8fde529ccbb8fc75c67f68c1a31a85ed1374eb0f211c9;
newer scripted-entrance/vehicle audit changes are unbuilt and uninstalled.
This log inspection adds no game launch/control, process-memory capture,
diagnostic hook or startup design change. User asked whether diagnostics caught
the hang; that question is answered. Preserve this open causal finding separately
and resume the authorized full map audit unless the human expands startup scope.

Status: investigation in progress (~55%). Diagnostic implementation is complete,
offline verified and installed/hash-verified in all six games. The user's next
FFIII launch reached the title and loaded a save normally. Root cause and a
startup repair are not established. The independent
[silent landing initialization correction](LANDING_LOAD_20261025.md) is complete
and offline verified, and is carried in the diagnostic build. Do not describe
the boot observer as a hang fix or gameplay-confirmed repair.

## Scope, permission and evidence

The user reports intermittent FFII startup failure historically and now four
consecutive FFIII starts with no logo/title. They clarify the FFII log is not
from a failure, so its normal gameplay tail is not evidence about the hang.
FFIII log rotation required preserving whole sessions by pid/start tick, rather
than treating the newest zero-length file as a complete failed bootstrap.

After the user explicitly approved one read-only inspection of stuck FFIII and
confirmed it was open, Sol captured pid11736, started2026-10-25T15:28:45Z, at
15:32:04Z. No game was launched, controlled, injected into, patched, terminated,
resumed through input or advanced. No new delegation or dependency installation.
The user was told they could close the process once evidence was saved.

Game-memory/native-source artifacts remain outside all Git checkouts in
`../tools/startup_hang_20261025/ff3` and `runtime_11736`. The Windows normal
minidump includes thread contexts/stacks/modules plus thread metadata, not the
full heap. Size411189 bytes; SHA256
b5ade59e67dcc58d222c64c923b1ed8f99ae73ec32e851455fe903edb9347982.
The original capture returned shell128 from the expected negative Git-ownership
test despite successful capture; capture.json/dump establish success. The helper
now exits0 explicitly; that artifact was not recaptured. Native observation
helper assumptions/failures are preserved separately from successful reads.

Original mod tooling:

| Tool | Contract and retained result |
|---|---|
| tools/re/capture_startup_hang.ps1 | Requires explicit approval, exact FFIII exe identity and external non-Git output; read/query process handle, one native stack/module dump; no full heap/input/target writes |
| tools/re/read_startup_state.py | Known metadata/layout pointer paths only, reads at most256 bytes per operation; no scan or native game API execution; saves actual operations/byte counts externally |
| tools/re/analyze_startup_dump.py | Offline dump thread/register parser and conservative direct-call return-address candidates; these candidates are not unwound stacks or proof of call chains |
| tools/re/map_location_native.py --boot-scenes | Original extractor; exact installed native Boot methods/provenance outside Git |

Local own-mod logs, inventory and recovery/delivery evidence are in
`build_check/startup_hang_20261025/`. No proprietary game source or raw dump is
stored in this repository. The capture/scripts do not upload anything.

## Established facts and limits

Failed sessions21248,19468 and16036 install294 hooks with0 failures, detach the
bootstrap thread, start Prism/NVDA and report startup complete. They remain in
Boot with roughly60 input frames per second for21..32 seconds in the preserved
tails. A preceding session9396 using the same installed build progresses through
Boot, Splash and Title into gameplay. This is not evidence of a universal loader
deadlock or that a changed diagnostic build repaired startup.

For the approved live session11736, Windows reports responding; mod input
heartbeats continue beyond two minutes. The main thread4020 was in an ntdll wait
at the instant of capture, consistent with the running frame loop.91 thread
contexts were parsed. Candidate stack addresses do not establish a blocked
call chain; no claim of a complete unwind or deadlock owner is made.

The inspected native Boot sequence has distinct setup and yielding resource/
scene-load phases before Splash. The exact ResourceManager initialization flag
was initially targeted, but its singleton is null, so no flag value exists to
read. A successful bounded observation records the generic class identity,
availability=false and null instance. Follow-up known boot owners show valid
SteamSDKManager, InputSystemManager, AudioManager and LogManager singletons.
SteamSDKManager.isInitialze is true. The native SceneManager exists with no
sceneLoadWaitTask or loadingSubSceneManager. Resource initialization callback
metadata has not been resolved. These facts narrow the missing setup; they do
not prove that no resource object exists elsewhere, why its singleton is
unbound or which earlier operation failed.

The first read helper rejected the null instance rather than reading address38.
A direct ResourceManager TypeInfo lookup was unavailable in this stripped
metadata; the refined lookup uses the exact initialization-callback metadata
and records unresolved/null rather than searching memory. Do not hide these
assumption failures or infer resource readiness from them.

FFIII native locators: SceneBoot.Start8B9260, CreateInstance8B8550,
Load.MoveNext8C59D0, LoadSceneWait.MoveNext8C4AA0; ResourceManager.Initialize6B88C0.
The generic singleton's body is shared; no generic Awake/OnDestroy/get_Instance
hook was added. The installed boot.config disables Unity Player.log, explaining
why a managed boot exception may leave only an input heartbeat in the mod log.
Absence of an exception in these logs does not establish absence of an error.

## Targeted boot observer

`src/core/system/startup_diagnostics.cpp` observes three exact native boundaries:
SceneBoot.Start, SceneBoot.CreateInstance and ResourceManager.Initialize. Each
original runs exactly once. Begin/completed records distinguish a missing
callback from an incomplete operation. A Windows exception filter copies the
first bounded managed exception class/message prefix when the verified
Il2CppExceptionWrapper/System.Exception layout is available, or its exception
code alone otherwise. All six dumps place Exception._message at0x18.

The filter always returns EXCEPTION_CONTINUE_SEARCH: it does not swallow an
exception, substitute a manager, retry an operation or change native control
flow. Four exception records per process bound diagnostic overload; this is
not speech/text deduplication. Native normal boundaries are event-driven, with
no polling. The owner copies at most64 class bytes and192 UTF16 message units;
invalid or unrelated C++ payloads are protected by a nested read guard. No game
API, ToString/message getter, scene scan, synchronous disk write or stack walk
runs in the filter. Existing logger ingress never waits for a busy queue/disk;
its worker owns file writes. No native pointer enters a worker.

The new `startup_diagnostics_check` exercises actual production wrappers,
original-call counts, normal completion, native SEH propagation, real MSVC C++
exception propagation, valid/malformed managed prefixes and overload bounds.
The test launches no game. The canonical build includes it before the existing
battle-feedback checks. Exact native names/signatures are checked against all
six installed dumps; the observer introduces no player-facing string/control.
Actual startup-frame impact and first audible response are not measured.

## Next evidence and delivery

Complete canonical build, all offline checks, isolated smoke and the exact
Defender payload scan before deployment. The user controls any new game launch.
On the next naturally attempted FFIII start, inspect the last Startup begin/
completed record and preserved managed error. This targets the exact unresolved
native failure, rather than repeating an unchanged capture or installing broad
engine exception hooks. If all boundaries complete, continue with the specific
resource/scene continuation supported by that evidence. If an error is recorded,
repair its actual producer and audit the corresponding all-six path. Do not
claim the historical FFII hang has the same cause without evidence.

Deployment remains separate and refuses running games. Existing settings,
dependencies and recovery/history are preserved. Diagnostic deployment/test
results must remain separate from implementation of the landing correction and
from an eventual boot repair; the latter is still outstanding.

## Verified diagnostic delivery and successful user run

Full build/all checks pass:7867 native names/0missing;133 used/341defined wiring/
0undefined. All18 observed native bodies are unique across six matching metadata
sets (native_boundary_audit.json). Actual-source exception propagation, landing
and context-gate fixtures pass.47-export smoke exits normally;49 embedded data
and18 sounds match source. Final DLL10918912 bytes, SHA256
07d54d2c8069fa309cd739e8c558e95d47ccbac1949c047fd3a4121fc407ece3.
Defender scanned the exact22-file payload with no threats and stable hashes.
Unsigned-DLL/no-certificate, protected-exclusion-query and untested outer-package/
reputation limits remain. No security settings changed.

deploy.bat installed all six after the user closed the stuck process. All132
installed hashes match;144 protected files unchanged against deploy_finalbefore.
Recovery/source/resource/build/smoke/security and final deploy_after.json,
installed_payloads_verified.json/completion.json are in
build_check/startup_hang_20261025/. No automatic game launch, input, dependency
installation, publication or new delegation.

The user then confirmed FFIII reached title. Own session1448 records Start begin
102950.187, CreateInstance begin102950.187, Resource.Initialize begin102950.343
and all three completions102950.406, then Splash102950.765 and Title102951.390.
No Startup exception was recorded. The user loaded Sealed Cave B3; construction
through Player contains no Can land speech. See successful_launch_pid1448.json
and its excerpt in the delivery directory. One successful diagnostic run does
not establish that instrumentation fixed the intermittent condition. A future
failure with the new boundaries is required to identify/repair its cause. Do
not require repeated deliberate restarts while the user can play normally.
