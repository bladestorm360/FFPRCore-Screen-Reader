# Custom sound controls

Owner priority: finish this feature before resuming the V/VI map/story audit.
**Canonical/offline output Closed.** SHA300ff3dfbebb647ae814baa13a171545533a2ae915236b33b1daeada9cc9d484,
12146688B,834 frozen inputs/205targets/242freshreports/67exactresources/12languages/
8340names0missing/0undefined/47exports. Canonical and smoke passed. Scan setup
initially lacked its dependency inventory; no scan ran and the failure evidence
is preserved. The unchanged output passed read-only inventory/exact22-file
Defender scan/final verification38.070s/101339136B/exit0/drained,2GB/25%CPU.
No threats/0matchingdetections/stablebytes; exclusions unverified/signing0.
Exact canonical_completion.json/final_scan_resource.json in the checkpoint.
Installation and physical listening/frame confirmation remain pending; the
original map/story audit resumes. Further audio refactoring is user-triggered.

Active continuation41305/resume9 follows170 completed project outputs and the
prior1200.339s/504606720B/exit124/drained checkpoint, without compiler failure.
Every freezev7 input remains identical. Finite canonical time allowance2400s;
aggregate2GB/25%CPU/one-worker limits unchanged. Full output closure pending.

Latest output checkpoint: resume7 reached its1200s limit,514572288B peak,
exit124/drained, with further compiler progress and no compiler error. An isolated
later-check diagnostic passed the remaining available checks except the stale
plain-text README; the existing release converter fixes that companion and the
actual asset check passes. No source-matched qualification is claimed from the
diagnostic. At the ended-job boundary only Main ReadMe.txt was refreshed; all
prior frozen source hashes are unchanged. Freezev7 includes both README forms,
834 inputs. Original/prepared/current document hashes are preserved in
document_companion_completion.json. ACTIVE canonical/smoke/scan24779/resume8,
same1200s/2GB/25%CPU/one-worker bounds. Nothing newly installed.

Source/focused verification is Closed. The first canonical job reached its
1200s bound at targets67–69/205,486436864B peak/exit124/drained, without a
compiler error. Resume75897 reached115–117/205,1200.698s/990343168B,
exit124/drained without a compiler error; native core/all-six libraries built.
Resume25365 reached166–168/205,1256.078s whole/933588992B,exit124/drained,
without a compiler error. DLL linked; incomplete logs/exit receipts are preserved.
Resume72324 compiled all205 targets successfully, then reached its1200s bound
during canonical regressions (1212.324s whole/934993920B/exit124/drained).
All source/DLL/compiler outputs and incomplete reports are preserved. ACTIVE
unchanged resume19259 revalidates cached compilation and runs all canonical
regressions/smoke/scan under the same1200s/2GB/25%CPU bounds. Its receipt is
canonical_resume4_resource.json. Nothing from this feature is
installed yet; final delivery and natural listening remain separate.

Integrated qualification then found a prior world trap-mask mismatch in FF3's
two bog objects. Floating navigation passed owner0; trap_route reported two
failures. A strengthened baseline reproduces four, then the minimal reader
normalization passes all255 traps/198 arcs and transport/Ice/worker checks,
104.886s/218763264B/exit0/drained. Native room layers and authoritative live
story/vehicle overrides remain. [Exact correction](FF3_WORLD_TRAP_MASK_20261102.md).
Freezev3 changes only reader+fixture fromv2; old inputs/DLL/failure retained.
ACTIVE corrected canonical/smoke/scan session17738, canonical_resume5_resource.json,
same1200s/2GB/25%CPU/one worker. No new game runtime or installation.

That run passed trap/Floating checks, then found an old FF3 declaration guard
expecting1861 cells after the reviewed collision correction now proves1853.
Independent per-cell planner ablation confirms exactly eight cover-walls were
removed, no ordinary floor; all13 doors/two-key prerequisites/203 full journeys
and11 pinned native owners pass. Only author guard/topology fixture changed;
no runtime/data change. Source freezev5 and key_guard_integration/final_completion.json
pin the correction. ACTIVE canonical/smoke/scan session81049, same finite caps,
canonical_resume6_resource.json. [Exact geometry evidence](FF3_KEY_GATES_20261101.md).

That run passed both corrections and exposed the remote category mismatch for
two already-reviewed Kefka roller/duct transfers. Matching the existing local
scanner in the manual-object branch passes2454 roots/7362 states and all four
affected consumer checks95.212s/481476608B. No graph/guide/data/hook change.
[Exact projection parity](KEFKA_MANUAL_EXIT_PROJECTION_20261102.md). Freezev6;
ACTIVE full output62929/canonical_resume7_resource.json, same finite caps.
The45–75min checkpoint estimate has overrun during the combined205-target
output qualification. Source/focused work is complete; the broad rebuild also
contains earlier shared navigation/header and guide changes. Completed compiler
outputs are retained across finite capped checkpoints; no extra runtime scope,
RAM exception or compiler/project parallelism was introduced.
Inspection also confirms field_api.h imports soundscape.h and the routing PCH
imports field_api.h through baked_nav.h, so this audio Options addition itself
invalidates navigation consumers. The concrete dependency is recorded in
BUILD_SPEED_20261102.md; no active-source refactor or claimed speedup was added.

F8 → Navigation audio → Custom sounds controls use of the existing installed
WAV files. Its settings submenu appears only while enabled. Each applicable
sound has a submenu with Use custom sound, volume and Preview. Off restores
the generated sound rather than muting the channel. Missing/invalid WAV files
also retain the default. Existing Navigation audio master, individual sound
switches, soundscape volume and stereo/HRTF volume banks still apply.

There are eighteen existing cue IDs: movement, bump, four walls, two beacon
cues, EXP counter, landing, and eight entity categories. FF2 omits Counter;
FF4/VI omit Landing, matching the current feature availability. Custom sounds
defaults on to preserve the existing act of installing WAV files as opt-in.
Each sound defaults enabled, private volume50 (original gain). No new files,
file format or live file watcher are introduced.

Periodic custom sounds expose Repeat interval,200–5000ms in100ms increments.
Unset displays Original timing. Left/right selects a numeric interval; Confirm
restores the original timing. Event-only Footstep/Wall Bump gain no timer.
Walls default to continuous looping; Loop custom wall sound Off reveals the
interval and plays discrete pings, with1s as its unset default. Long wall,
counter and entity clips finish before repeating, including under voice pressure.
Loop seam quality belongs to the WAV designer. Native Beacon and Landing
retain their existing one-shot retrigger semantics at the selected interval.
Active HRTF uses the normal Beacon PCM even for the blocked-route signal, so
that cue's custom controls own its effective timing and gain.

The shared policy in audio/custom_cues captures eighteen fixed records on the
existing native settings owner. Only values enter the existing configuration
queue. Audio owns WAV loading, retained default PCM, voice state, mix and
wall/counter/emitter clocks. Disabling a custom sound stops affected active
voices before switching timbre; volume updates retain existing loops. No new
game hook, native pointer transfer, blocking wait or physical device is added.
The loaded-file mask is a scalar atomic published after Audio has loaded clips;
native producers use it to avoid changing default timing for absent files.

Existing limits remain: four snapshot buffers,8192 observation rows,128 voices,
eight entity starts per worker pass with rotating fairness,64 requests and
eight configuration slots. One emitter cannot overlap itself at a short custom
interval. Field/master generations and stale observations retain their existing
cancellation boundaries. Original PCM is retained only after a valid custom
load; the existing4s/PCM16 loader bounds are unchanged. Native settings remain
owner-thread-only; Audio never reads them or calls game APIs.

Verification artifacts: build_check/custom_sounds_20261102. Initial actual
loader/settings/scheduler/stereo/HRTF baseline and custom cases passed. The
first menu check failed because old fixtures assumed Soundscape was always the
fourth audio row and the first numeric range. Nonpositional lookups fix those
assumptions; no menu production repair was required. Corrected full menu,
producer and Beacon checks pass under2GB/25%CPU,72.535s,326422528B peak,
exit0 with children drained. Final additional HRTF fallback, Landing interval
and cancellation checks pass. The initial pending-route fixture lacked a prior
completed route; corrected pending and truly blocked HRTF cases both pass.
Original failures are preserved. Actual settings snapshot persistence/capture
and all-six full menu construction also pass30.040s/270471168B,exit0/drained.
Real settings capture18 mean10.285us/peak232.700us over10000 calls; complete
menu construction mean0.022ms/peak0.024ms in the fixture. These are throttled
offline measurements, not game-frame or audible response. The retained400
emitter suite hears all400 with478 deferred starts, peak worker chunk0.454ms.
Exact source/report/resource receipts: completion.json in the checkpoint.

Minimal information integration: the generic help originally said every long
clip finishes before repeating, but native Beacon/Landing preserve their prior
retrigger semantics. Removed that overstatement in all12languages; the specific
behavior above/readme remains documented. No runtime design changed. Original
source completion/manifest are archived as v1. A forward-slash alias mistake in
the first metadata update was rejected by the source guard before compilation;
the malformed manifest and admission receipt are retained. The repaired manifest
verifies exactly two text/author inputs changed and every other input is intact.
Localization check passes; help_integration.json records the exact hashes.

All fourteen new labels/help strings are authored and checked in the twelve
supported UI languages. The shared Auto/manual language contract is unchanged.
Canonical build wires actual custom checks and catalog validation. This topic
does not claim installed payloads, physical audio latency or in-game cumulative
frame responsiveness from capped fixture timing. Natural acceptance after
delivery: toggle a custom entity sound off/on; adjust200ms and5s; test wall
loop/pings and dialogue/battle cancellation; verify separate stereo/HRTF gain.
