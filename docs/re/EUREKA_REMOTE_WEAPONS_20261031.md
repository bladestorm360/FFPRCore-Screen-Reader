# FFIII Eureka weapon destinations from other floors

Owner-authorized progression-block fix, October31. The all-six puzzle audit and
document-only Doga/underwater/moving-flight reports remain paused. Authoritative
status is the single ledger in MAP_AUDIT_FOLLOWUPS_20261028.md.

Current status: source/offline/security/FFIII delivery Closed. After the owner
saved and closed FFIII, guarded installation completed and verified all22
installed files.144 protected files and the other five games are unchanged.
Natural cross-floor case confirmation remains pending. Preserve the isolated
delivery checkout and the durable source/output checkpoint.

The owner reports all weapon objects say Destination is no longer available
from other floors, including the beginning of Crystal Tower, while returning
valid directions on their own floor. The installed11c25405fff5f1eb source
explicitly excludes AnimEntity from the selected remote goal's physical-object
validation. The remote provider already admits actionable animated objects.
On the target floor, the existing live-instance fallback supplies availability;
other floors have no native instance to supply that fallback. This explains the
reported difference without changing native puzzle, inventory or story state.

## Minimal correction and source isolation

The correction removes only that AnimEntity exclusion. Exact ID/raw name/cell,
initializer visibility, consumption and real route gates still apply. PointIn,
TelepoPoint and Warp metadata remain excluded from this physical-actor check.
No new hook, native read, polling, input, game-thread search or worker queue is
introduced. The unchanged captured data and worker route pipeline own the work.

The paused working tree already contains the equivalent animated-weight change
from the unfinished FFVI puzzle package. To avoid shipping that package, a
separate temporary delivery checkout recovers all688 inputs of the actual
installed FFIII build by SHA. Source comes from unchanged working files, HEAD
with verified original line endings, the preserved baked_nav.cpp and exact
hash-matched reconstructions. No game implementation is copied or executed.
Recovery: build_check/eureka_fix_20261031/baseline_sources.json.
Delivery checkout: ../FFPRCore-Eureka-Delivery/.

All runtime/data inputs match the delivered baseline except the one removed
condition. The only other differences are regression-fixture/build wiring.
All67 resource inputs are preserved. Runtime/source delta:
build_check/eureka_fix_20261031/runtime_delta.json and release_input_delta.json.
Original paused source changes remain in the maintained checkout.

## Actual production-consumer regression

tools/eureka_destination_check.cpp uses the real connected-floor provider,
bakes, initializer/consumption program and plan_snapshot consumer. The actual
provider yields five guarded weapon objects: map320/entity7,321/28,322/6,
323/8 and323/7 (Staff). All are AnimEntity. Original source reproduces50 remote
failures with Destination is no longer available and five valid local routes.
The original report is retained as reference_report.txt.

The corrected source passes50 remote and five local paths from Crystal Tower
entry and every Eureka submap. Exact native retirement guards for all five
weapons still reject already-taken targets; mismatched raw identity is rejected.
Closed key gates still return Story gated. Open-tower cases use the actual
sc_e_0090/sc_e_0258 key-use receipts94/178; the earlier all-zero fixture wrongly
expected an open entry door. That five-case expectation failure is preserved in
fixed_closed_gate_fixture.txt. The first reference fixture also searched only
event guards for retirement; final coverage includes actual initializer guards.
These fixture corrections do not modify native flags or relax routing gates.

Final focused job:55 paths, five native retirement guards,0 failures; zero
native worker access. Peak429,690,880bytes,46.530s including compilation;
childrenDrained true. This resource-throttled tool time is not game latency.
Budget: focused_final_budget.json. No game launch/control/probe or gameplay
confirmation is claimed.

## Build-budget integration correction

The cold canonical MSBuild run stopped at the memory ceiling after128 linked
targets: exit1,3878.880s,reported peak1,000,017,920bytes, enforced allocation
limit999,997,440bytes. The CLR reported StackOverflowException at the end of
the long run. The requested budget was not increased. The failure log and
receipt are preserved in build_check/eureka_fix_20261031/failed_cold_build/.

tools/build_projects_serial.py now reads every actual generated ALL_BUILD
dependency and builds each in a fresh serial CMake/MSBuild process, within the
same verified kernel job. This releases retained MSBuild state between targets;
it preserves complete target coverage and already valid compilation outputs.
Clean builds clean once before the target sequence. All142 dependencies are
included, with one compiler/node, no reuse, bounded CPU and unchanged1 GB cap.
The canonical build entry uses this helper; the isolated delivery checkout
also receives the already qualified resource runner/entry guard. These are
build-only changes, separate from the one-condition runtime correction.

The resumed full compilation completed with peak887,574,528bytes. Its canonical
sequence then exited because the isolated copy lacked the FFV before-map
fixture used by contextual_submarine_check. Existing factual ff5_maps.bin was
restored by SHA; the already-present15-case identity table was also verified.
The first restoration attempt rejected that existing companion unnecessarily;
its receipt is preserved. No native implementation was copied and no routing
or test expectation changed. Evidence: missing_fixture_build/ and
restored_fixture_inputs.json in the main task evidence directory.

## Verified candidate and delivery boundary

Final canonical exit0: all142 projects,158 fresh reports,8304 native-name checks
with0 missing,143 used/350 defined references with0 undefined,47 exports.
All55 Eureka routes and five native retirement guards pass. Frozen690 source/
data/check inputs, all67 embedded resources and12-language catalog verified.
The single runtime-condition delta remains the only runtime/data change.
New build driver and qualified runner hashes are recorded separately.

DLL:11,495,936bytes; SHA256
5bb84691fc69a688b8ec5ccdf48ca22fd57644ea79ca2018dd716991d9909052.
Final canonical job:289.396s,750,182,400bytes peak,exit0/drained. The earlier
resumed compilation peak is887,574,528bytes. These are throttled build/tool
measurements, not game response or native frame evidence.

Smoke confirms DLL loading,47 forwarded exports and boot logging without a
game. Defender reported no threats for22 exact payload files, no matching
detection/remediation events and stable hashes. Scanner signature valid;
Defender enabled/current. Exclusion-query coverage remains unverified under
its existing ACL; no signing certificate is available. No protection setting
was changed. The first scan launcher used an incompatible Windows PowerShell
module environment and stopped before scanning; preserved separately. The
verified installed PowerShell7 host completed the scan in the same enforced
budget (85,008,384bytes peak,8.909s,exit0/drained).

Canonical/smoke/scan/input/recovery records are in
../FFPRCore-Eureka-Delivery/build_check/eureka_fix_20261031/; main task budgets
and the preserved failed attempts are in build_check/eureka_fix_20261031/.
Only FFIII was installed. Its exact process was still running at preflight;
installation waited for the owner to save and close it. guarded_deploy.ps1
rechecked process absence, verified the scanned SHA, preserved recovery/config/
waypoints, deployed ff3 and verified all22 files plus the other five games unchanged.
Do not build/deploy the paused maintained-tree puzzle package for this fix.

Durable main-checkout checkpoint:
build_check/eureka_fix_20261031/verified_candidate/ contains the exact DLL,
all690 frozen original-mod/navigation inputs plus qualified resource runner,
158 canonical reports and smoke/security/recovery receipts. All872 copied
files in the initial candidate were SHA-verified against their producers.
files_installed.json now records876 preserved files including completion.json,
installed22-file receipt,144 protected-file checks and process-absence evidence.
Deployment budget:117,411,840bytes peak/22.018s,exit0/childrenDrained true. No
game launch/control/probe. The sibling delivery checkout remains preserved.

The owner's separate October 31 confirmation records FFII/FFIII as 100% playable
with mod features/map bakes and FFI as tentative after an old-mod completion.
That broad playability record is in docs/TESTING.md; this new specific Eureka
cross-floor correction still awaits its natural-play case confirmation.

Natural test after installation: from a different Eureka floor, select any
unclaimed weapon and request directions. Expect a valid cross-floor route;
already claimed weapons and actual story gates must remain unavailable/gated.
