# Restore cross-floor guidance to blocking NPCs

Human explicitly reports the regression and requests prior behavior restored:
Saronia chest directions say No path from another floor, while entering the
blocking Soldier's floor gives1 North, Interact: Soldier. This scoped regression
is the active checkpoint; full audit checkpoint1 is preserved, with no production
vehicle-rule changes. Sol only, no game probe. Estimate30–50minutes including
real-map reproduction, minimal correction, affected/full checks and FFIII delivery.

Latest user log retained in build_check/cross_floor_npc_20261028/user_log.txt:
map105/start22,8/L0 to map106/chest12,33 repeatedly fails. On map104 the same
destination produces an interaction prefix for Soldier/entity24 at17,29/L0.
Those maps are Map_20211_1, Map_20211_2 and Map_20210, all area35. Native
initializer flag74 must be set to retain the actual Soldier; flag75 remains clear.
An initial fixture lacking74 did not reproduce the obstruction and is preserved
separately; it is not accepted as regression evidence.

The corrected real-data fixture reproduces2 failures: remote directions fail
and remote browse loses its required floor, while entered-floor guidance already
works. Exact bake/IR/native actor identities and the production plan/preview
functions are used; no invented game map or native code fixture enters Git.

Cause: cross-floor fallback only tries a current-floor NPC interaction prefix.
The proof's with_npcs parameter controls current live obstacles, while remote
add_obstacles always retains NPCs. A Soldier on an intermediate floor prevents
proof from finding any continuation, so the planner never routes to that floor.

Minimal correction: try a continuation proof that retains all current-floor
NPCs/gates/terrain/traps but treats later-floor NPCs as potential interaction
stops. Only its physically valid current leg is issued. Entering the next floor
recaptures native actors and the existing NPC approach stops at the first real
blocker. A separate all-NPC-relaxed proof may feed that existing current-floor
approach; it never grants movement through a current NPC. Story barriers,
terminal transitions, trap history, original objective and native interaction
remain intact. No graph/route-optimization redesign or new hook/capture is needed.

Source/delivery Closed: included in FFIII de3e075d/11099136 bytes with the
[nearby walking/object-label fixes](LOCAL_GUIDANCE_20261028.md). Full build,
smoke and22-file scan pass;22 installed hashes match,144 protected unchanged,
other five payloads unchanged. Earlier c23a8fa9 was an uninstalled candidate
blocked by the running-game guard. Preserve the accepted battle wording and
prior fixes. Full audit now resumes; no game probe or gameplay confirmation.

The initial correction passes the matched Saronia direction/preview/entered-map
fixture with0 failures. Expanded checks retain the original chest after a live
NPC approach, verify the one-step North stop beside Soldier24, and resume the
chest continuation when native capture removes him. Displaced current blockers
must be avoided, with either a legitimate alternate exit or an interaction stop.
The first preservation assertions incorrectly required No path/forced interaction
even when the real map has another exit; those fixture assumptions were corrected
without changing production behavior. Earlier failure logs are preserved.

Final expanded regression passes0 failures, including legal detours, native gate
avoidance, chest retention and continuation after Soldier removal. It is now in
every canonical build. Shared all-six native/route/state checks also pass:
8318 names/0 missing,142 used/349 defined/0 undefined, smoke47 exports,
67 embedded resources/136 registrations,12 locales/six unchanged bakes.
Candidate11098112 bytes, SHA256
c23a8fa9bca219e5c41b104eb6a9984822d661a67391c2ab775a7c7e63a78b59.
Defender reports no threats for22 exact staged payload files, stable hashes and
no matching detections. Exclusions remain OS-denied/unknown; no signing identity.
Source complete; delivery waits for the player to save/close FFIII. The deployment
running-game guard rejected the copy; no update was installed. Recovery, scan,
failed/successful checks and receipts remain in the evidence folder above.

The change adds no native getter, hook, owner capture, job, lock or wait. The
existing cancellable worker may retry at most once after a complete failed
cross-floor search, retaining its400-state bound per attempt. Existing caller
trap/NPC/gate diagnostics retain their finite attempts. Actual frame/combined
native episode and audible timing are unmeasured; asynchronous placement alone
does not certify those costs. User log samples of the failing prior candidate
record capture about0.44–0.49ms and17ms request-to-result, without an audio measure;
these are not a controlled before/after benchmark for the correction.
