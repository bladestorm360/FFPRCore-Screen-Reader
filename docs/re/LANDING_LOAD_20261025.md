# Silent landing-guide initialization, October25

Status: complete/offline verified and installed/hash-verified in all six games.
The own log of user-started FFIII1448 confirms the cave save loads without Can
land initialization speech; listening and subsequent flight edges remain user
checks. No delegation, game launch/control or dependency change. The separate
[startup investigation](STARTUP_HANG_20261025.md) includes an explicitly approved
read-only snapshot, recorded in its own scope.

## Report and native context evidence

The user reports Can land while loading a save after the combined
[landing-producer correction](FF3_FEEDBACK_20261025.md). The user's own FFIII log
confirms the line at100445.640, after ChangeMap1 at100445.343 and before
FieldReady2 at100445.718. Player3 and Entering Sealed Cave B3 occur at100447.937.
This is initialization before player control, including a cave/on-foot context;
an active UI object alone does not establish a usable landing guide.

The prior implementation correctly covered ShowLandingGuide bypassing
SwitchLandable, but interpreted a construction-time false-to-true state as a
live availability change. The older fake fixture assumed a ready field context
for every visible guide and therefore did not catch this error. The new fixture
explicitly models loading and fading as well as ordinary flight.

The relevant own-log excerpt/provenance is retained in
`build_check/landing_load_20261025/own_log_load_excerpt.txt` and
`own_log_provenance.json`. The native Show/Switch/Hide evidence remains outside
Git in `../tools/landing_guidance_20261025/ffN`; source locators and all-six ABI
details are linked from the preceding record. No new capture was necessary.

## Current behavior and ownership

`src/core/fieldnav/vehicle_landing.cpp::observe_landing_guide` still validates the
actual MapUIManager and its current active guide after the native original.
A newly landable state speaks only when the existing shared
`system::is_field_navigation_ready` returns true and
`system::input_transition_blocked` returns false. Those services already reject
unknown/boot state, queued or executing map changes, Init0/ChangeMap1/FieldReady2,
non-field states, battle entry and fades. Ready Player3/Transport4 remains valid.

Every observed visible guide still updates the remembered landability, even
when speech is blocked. Consequently control/fade completion does not replay a
loading-time true state. A later unavailable-to-available change during normal
flight still speaks Can land. Hidden-guide ownership, hide/map/battle resets,
native producer calls and the stale movement-cache correction remain intact.
No delayed speech or text-content comparison is added. FF6's shared priority
timestamp changes only when a line is actually submitted; silent initialization
cannot reserve its heading/altitude interruption priority.

Native objects stay on the owner thread. This adds at most the two established
constant-size context checks on a potential false-to-true edge; stable states do
not repeat them. There is no new hook, collection, scene search, polling timer,
queue, sleep, input action or native-thread wait. Only the existing copied
localized Can land line reaches the speech worker. No new strings/settings or
localization keys are required. Actual callback cost, first audible latency and
game-frame impact remain unmeasured; no performance guarantee is inferred from
the fake-native harness.

## Verification and safe extension

`tools/vehicle_landing_check.cpp` uses actual production Show/Switch/Hide detours
in each of the six game modes, with deliberately stale on-foot movement state.
It retains valid live edges, repeated-state silence, hidden-owner separation,
hide/reshow and original-call counts. Added cases cover construction-time true,
no replay when control returns, loading with either initial bool, fades with
either initial bool, unchanged FF6 priority timestamp while silent and fresh
post-load/post-fade edges through both producers. Native observations must stay
on their owner and every original operation must still execute exactly once.

The existing actual-source `battle_context_check` verifies the real shared
readiness/input gates against queued/executing transitions, old reported states,
field/vehicle readiness, menu/battle contexts, boot/title and fades. Its native
state constants are the already inspected all-six enum. No separate per-game
loading heuristic, timeout or movement-cache gate should replace this contract.

Canonical checks remain build.bat then smoketest.bat, followed by an exact22-file
Defender stage and separate deploy.bat. Recovery/build/source/resource/security
and deployment receipts are collected in `build_check/landing_load_20261025/`.
The previous tested/scanned DLL is preserved per game before replacement.

Gameplay confirmation, after installation: load the reported cave save and a
boarded save when available. Neither initialization should announce Can land.
Then enter landable terrain during ordinary flight: one Can land is expected;
remaining over that terrain must stay quiet. Returning from a menu/battle should
not replay an already observed true guide. Failure symptoms are loading-time or
delayed unsolicited speech, or loss of a genuine new landing-availability cue.
These are user-side checks; the agent does not start or control games.

## Final combined delivery

Canonical build/all checks,47-export smoke and exact22-file Defender scan pass.
The startup diagnostic build carries the unchanged landing correction:
DLL10918912 bytes, SHA256
07d54d2c8069fa309cd739e8c558e95d47ccbac1949c047fd3a4121fc407ece3.
All132 installed hashes match in six games;144 protected files unchanged. Final
receipts:build_check/startup_hang_20261025/. Earlier landing-only DLL SHA256
c12ade01674a06bb18d81a644fc497db85d8b84562b3ce313303ab710af9de02 and its
scan/build evidence in landing_load_20261025 are preserved; not separately deployed.
User-started FFIII1448 loaded the cave save with no initial Can land submission;
later live flight behavior still needs listening.
