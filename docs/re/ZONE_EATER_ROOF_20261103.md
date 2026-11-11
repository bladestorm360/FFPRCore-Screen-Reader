# Zone Eater roof guide — source/focused Closed

This is the remaining FFVI unique-puzzle audit, after custom controls passed
their complete output checkpoint. The guide is implemented; model/controller/
shared-input/source/localization/focus/menu/performance/native-name checks pass.
Exact source/report hashes are in build_check/zone_eater_20261103/completion.json.
No new roof DLL is built or installed. The existing Opera/Phoenix/Tower/Narshe/
Train designs remain Closed. Further roof refactoring is human-user-triggered.

## Controls and producer connection

Shift+G, or the existing controller guide command, first previews the cardinal
route to the next safe area. A second deliberate request confirms readiness.
The guide waits for a fresh native cycle, revalidates the same physical steps
and says only “Go now.” This avoids guessing how long the player needs to hear
the directions. Native keyboard/controller edge handling requires a new press.
At the next safe area repeat the two requests. The final one-step exit is
ordinary guidance and completes only on actual ready arrival at505.

Readiness may be confirmed during the ceiling animation: only the stored mod
intent changes then. The existing ready continuation defers all timer/player
observation until normal control returns. The shared core grants that context
only to an already pending FFVI roof request. Existing static/event/other-game
guide contexts are preserved and their controller tests pass.

The user-controlled timer freeze remains unchanged and is respected. Unknown,
wrong-script, expired and nonfinite clocks never create a movement window.
Late results keep the request waiting for another real cycle; changed routes
require a new preview. Stop, map/load, battle and title retire pending intent.
Go publication requires game focus and no mod modal owner; an attempted release
without that ownership is disarmed for another explicit readiness request.
Unexpected positions receive a real shortest safe-area recovery, rather than a
blind next waypoint. No automatic game input or timer modification is added.

## Primary facts and route boundary

The room is map504/Map_30791_5, between503 and505. Its34 exact ground-event
safe markers comprise16 calls of0850_1 and18 calls of0850_2. The first kind
sets map-local1 to0; the second sets it to2. The automatic0850 cycle starts
with1, and only0/2 survive its crush check; failure leads to584/sc_e_0258_1.
The marker puppets hide the event actors, rather than moving their coordinates.
The second safety kind temporarily changes the player to Upper and restores
Ground after the ceiling rises; XY is unchanged. Normal input/readiness must
own guidance publication, never a fabricated ground pose during the animation.

The main0850 script is a timer callback, not another object interaction. Both
its ordinary completion and0850_0 arm a native5-second timer after raising.
The map initializer arms the initial cycle or preserves the area-local re-entry
branch. Map503 resets area-local1; map504 initializes its ceiling layer. These
exact automatic callers need a supplemental semantic disposition because the
immutable object-discovery inventory did not bind main0850 to an actor.

Point IDs are object IDs, not entity numbers: native Point1 is entity2 at
Ground37,18; Point2 is entity1 at Ground9,19.503's0850_in1 selects Point1;
505's0850_in2 selects Point2. Exit entity4 at9,20 leads to505; entity3 at37,19
returns to503. Raw/normalized metadata and hashes stay in the checkpoint.
Both automatic entries explicitly continue through0850_0 after changing maps,
so the same native rise/reset owns a fresh five-second cycle in either direction.
The native MapRange is47 by25 cells; it is not the full64-cell room sheet.

The actual production planner passes the complete forward safe-area chain:
37,18;37,12;33,9;29,14;24,10;18,13;13,9;10,15;9,19;exit9,20, all Ground1.
The nine legs contain6,7,9,9,9,9,9,5,1 cardinal steps. No wall/collision change,
diagonal shortcut or invented arrival is used. This proves physical geometry,
not the as-yet-unimplemented cycle controller or human speech/movement timing.

## Native observation boundary

Matching GameAssembly hash0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd
and dump38aa9c54abe1c712ffd3c6e88a893ea6428f6df7001718b754b21afba7f4a61b
identify the existing external evidence. SystemCall.Current's exact Down2A3500,
Up2A40D0 and Init2A3A80 own the visual layer. Each scroll function advances only
at its native completion. No per-frame detour of those functions is planned.

TimerInstanceManager.SetGrobalTimer575200 removes the previous timer, constructs
a new Timer/GrobalTimer and publishes it at the existing manager field. Removal
575060 destroys and clears it. The existing static-pointer reader can observe
the already-created manager without invoking a potentially allocating getter.
Only a bounded current roof timer/one exact script name/finite scalar values
may be copied on the native owner. Workers receive values, never timer pointers.

The connection uses one scoped SetGrobalTimer notification while one requested
leg is armed, the existing ready continuation and Navigation worker. It filters
one exact nine-character callback name and copies at most one timer's scalars.
The already-created singleton is read directly. Off-owner calls perform no
observation. Pending state contains one owned goal and at most128 cardinal
cells, no native pointers. There is no new per-frame timer observation or frame
scheduler. The existing core input pass adds one FFVI-owned readiness predicate.

## Evidence and current limits

build_check/zone_eater_20261103 retains normalized family/actor facts, capped
read/extraction receipts and the physical planner report. Native bodies remain
outside every Git tree under tools/puzzle_audit_20261030/ff6_zone_eater_roof
and ff6_zone_eater_timer. The older SetPos body has matching primary hashes.
The first geometry fixture failed to compile because its diagnostic passed an
atomic counter to printf; the original fixture/log are preserved. Explicitly
loading that counter fixes only the test print. The actual nine-leg geometry
check passes43.437s/444329984B/exit0/drained under2GB/25%CPU/one worker, owner0.

This roof milestone does not complete other Zone Eater bridge/jump mechanics,
the874 remaining VI candidates, or the350 remaining V candidates. Optional
minigames remain excluded. No game launch/probe or new installation occurred.

The model and real planner now pass all211 connected-cell recoveries,34 safe
resumes and both automatic503/505 incoming boundaries; nine forward legs remain
cardinal and collision-valid. The first strict exit-edge assertion omitted the
actual destination from its request; matching the production exit-goal context
fixes only the fixture. Final boundary/source/semantic qualification93.657s/
445239296B/exit0/drained,2GB/25%CPU. Peak model0.071ms in the capped fixture is
not actual game-frame/audible evidence. Actual controller/retained story model/
shared guide input checks pass65.881s/348430336B. The first controller fixture
compile used unqualified ready/speech names that collided with production
symbols; the original source/log are preserved and only fixture qualification
was corrected. Nine keys in12 languages pass with every prior catalog value
and all nine accepted guide records preserved.

Two exact automatic arrival counterparts are reviewed in the immutable-inventory
ledger. The34 safety markers and the otherwise-unbound main0850 callback are
additionally pinned by author_zone_eater_roof.py's seven exact script hashes.
Earlier I–IV semantic checks pass; the full checker still explicitly refuses
1224 unfinished candidates. This is not all-six audit or release completion.

Final controller passes0 failures, including focus/menu ownership and held route
results. Native timer copy over10000 calls: mean1.103us/peak17.000us in the capped
fixture, not game frame/audible timing.1498 FFVI names have0 missing. The targeted
build/check took34.075s; the resource receipt518.802s includes waiting for shared
admission and must not be reported as compilation time. Peak345141248B/exit0/
drained,2GB/25%CPU. Closure/wiring passes, every source identity is pinned and
all old catalog/guide values are preserved. Known remaining confirmation is
physical Go-cue audibility, actual native input/hook behavior and full-game
responsiveness after the final combined build/delivery. No current source bug is
left open in this finite roof capability.
