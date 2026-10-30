# Invincible mountain preference and the confirmed Manor restriction

Human-scoped follow-up, October29: ordinary flight must take priority over
mountain actions; use the latter only when required. This reopens only that
route-policy boundary of the closed mountain feature. No new hook, native
observation, map rewrite, movement automation or unrelated refactor. Sol only.

Status: preference source/offline/FFIII delivery **Closed**. Installed0a511191,
11348480 bytes. Canonical/smoke/8458 names/67 resources/12 locales/22-file scan
pass;22 installed hashes match,144 protected files and other5 games unchanged.
Recovery63a24c76 retained. The related
Manor report is resolved by the human: only the Nautilus passes the winds. The
human parked the Invincible and returned to the Nautilus; **no Manor change is
required**. No new failure-capture diagnostic is retained.

## Confirmed native restriction and investigation correction

Map4's native Event898/editor365/sc_e_0358 is explicitly restricted to Invincible10;
Event899/editor367/sc_e_0357 targets Enterprise6. Both are at baked97,212
(Tiled anchor97,213). Their complete reviewed scripts move the player south and
restore the first-party puppet at97,217, rather than admit the aircraft. Nautilus8/9
has no equivalent trigger there. Source hashes/complete source remain in the
existing external mapdump and functional-event review; see
[the reviewed passage barriers](FUNCTIONAL_EVENTS_20261028.md).

An important distinction: the transportation terrain group is shared by all
three aircraft. My intermediate commentary incorrectly said that this terrain
rule excluded Enterprise; the separate native events enforce the restriction.
The independent real-grid survey cannot reach the Manor while avoiding97,212,
even when Invincible mountain actions are included. From the southern approach
97,217, native-selector-aware survey admits Nautilus8/9 and rejects Enterprise6
and Invincible10. The human's gameplay confirmation agrees. No game was launched
or controlled by the agent.

Earlier diagnostic fixtures in before.txt/live_before.txt/identity_before.txt/
profiles.txt could reach the Manor because their authored actor stand-ins did
not model the scripted forced movement. They are **not** route-validity proof.
Their nominal twenty-crossing path passes the rejecting wind trigger. Those
positive assertions have been removed from the final preference test; no wind
restriction or Manor gate was opened. The first full build was interrupted to
check this human steering; its log remains in first_build_interrupted_for_wind_check.txt.

## Preference correction and discriminating evidence

The prior A* priced mountain actions only by their two/three-tile displacement.
`baked_router.cpp::find` and `flood` now compare manual mountain-action count
first, then the existing physical-distance/turn metric. Any ordinary flight wins
over an optional crossing. Required crossings remain legal and are minimized.
`baked_nav.cpp::boarding_route` uses the same priority for legal landings and
vehicle candidates. An ordinary walking route cannot be replaced by an optional
mountain shortcut. Spoken distance remains physical steps; existing localized
manual-facing guidance, beacon boundaries and autowalk handback are preserved.
No terrain, collision, selector, trigger, story flag, control or setting changes.

Actual surface ridge175,20→175,23 has a direct native three-tile action and a
five-step ordinary flight. The old source prices the direct action30 and the
ordinary path at least50 before turns; the corrected router/flood/publication
use the five-step flight without a mountain cue. This is real baked terrain.
`tools/invincible_preference_check.cpp` tests the actual source and publication.
Existing full Falgabard provider/category/boarding/landing/beacon/autowalk tests
retain required mountain actions. Its preferred valley route now has87 physical
tiles, replacing the earlier85-tile shortcut. The floating-model test isolates a
real required edge with explicit obstacles: its old first-edge fixture allowed
an ordinary detour and incorrectly required an optional action under the new
human policy. All-six capability abstention and cancellation checks remain.

## Bounds and verification limits

World A* retains its four-heading/cell state space and cancellation checks. Only
Invincible requests allocate the extra two-byte action count per state (512KiB
at256×256). Its flood adds one two-byte/cell plane (128KiB); other modes keep
empty action-count vectors. Queue keys add one integer within the same finite
search/state lifetime. There is no new persistent history, queue, native read,
worker pointer, callback, polling or game input. Current code remains within the
router's existing cohesive responsibility (under1000 lines).

Paired diagnostic fixture episodes in identity_before.txt/profiles.txt include
geometry, two full plans and a flood: approximately266→286ms without authored
actors and412→440ms with actors in those individual runs. Those Manor stand-ins
have the semantic limitation above; these are only background owned-work
measurements, not a speedup or measured game impact. Thread scheduling/cold
caches affect the samples. Final canonical checks supply current evidence;
actual frame, audible response and sustained native impact remain unmeasured.
The user's cumulative25ms application-lag rule is not certified by worker
placement or these harness numbers.

Evidence/recovery: build_check/invincible_preference_20261029/ retains relevant
own FFIII logs, before/after reports and installed63a24c76 recovery. No agents,
game probe, dependency install, security-setting change or publication. The
six-game map audit remains open; Ghido's source/data correction is included in
the next combined candidate, without installing FFV. Resume Imperial Camp mask
registration and the retained geometry/arrival/power/scene cases after this
finite preference delivery checkpoint.

Final receipts: completion.json/deploy_after.json under the evidence directory.
The resumed serial build ran18:33:43–18:50:44 UTC (17m01s); a shared-header
rebuild affects many check executables. The first build was intentionally
interrupted for the wind investigation; another session ended without an exit
result. A default hidden launcher did not persist; the authorized isolated hidden
runner resumed the saved build and recorded exit0. One observed compiler had
232.1MB peak working set; this is not a whole-build peak claim. No compiler is
running now. Source/offline/delivery Closed; broader audit continues separately.
