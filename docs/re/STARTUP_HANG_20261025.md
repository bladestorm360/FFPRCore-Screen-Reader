# Boot scene fails to progress, October25

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
