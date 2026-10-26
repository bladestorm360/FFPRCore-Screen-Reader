# Live NPC obstruction routing

October25 [routine-based guidance correction](NPC_PATROL_TARGETS_20261025.md):
corridor width never turns an autonomous patrol into a stationary gatekeeper.
Fresh native MoveType/MoveStep facts take precedence over the bake; generated
companions remain nonblocking and now use their actual owned native base identity.
The historical October23 narrow-channel distinction below is superseded.

October23 detour follow-up: **complete; offline verified and deployed to all six**.
FFII was deployed after the user closed the game and explicitly requested it; its
newer preferences are preserved. Gameplay confirmation remains pending.
The earlier Castle Fynn log alternates between
a 23-step route and “Blocked by an NPC” at the same player/exit coordinates. Leaving
and reentering restores it. Exact live locations/collider flags were not logged for
every soldier; a stale position or missed detour is not proven by reentry alone.
[Castle evidence](CASTLE_FYNN_20261023.md) distinguishes the separate story gate.

## Player behavior and routing contract

Directions, browsing, beacons, exit grouping and autowalk share the current-floor
obstacle snapshot. A complete legal detour always wins. When no complete route
exists, an NPC-specific proof removes only current-floor NPCs; it preserves parked
parties, treasure, gates, directional restrictions, trap history and terrain. Only
if that proof reaches the actual destination may guidance approach its first real
NPC obstruction. The walking prefix keeps every actual solid obstacle and uses
the actor's current interaction mask/counter geometry. A stationary area gate
ends beside the actor with “Interact: Soldier” and arrival facing. An autonomous
travelling actor, including one in a one-tile corridor, instead ends with “Wait
for Soldier to move.”, with no invented interaction or arrival. It never walks through
an NPC, claims the selected chest was reached, or promises talking will open a gate.

The partial route retains the original stable destination for a fresh request after
interaction/movement. A manual gate boundary returns control to the player;
temporary occupancy has a separate wait boundary used by autowalk and beacons.
Beacons do not mark this boundary as destination arrival, and scenario/defense
planning cannot count it as a completed encounter or overwrite it with “Fight”.
A moved actor is re-observed on the next request; partial plans are not reused as
completed routes. Without a proved usable approach, retain a normal route/gate
failure rather than falsely identifying a nearby NPC as the cause.

Controls/settings are unchanged: normal directions/navigation actions, existing
Autowalk/Beacon settings and scenario guidance. Central Interact/face/NPC/wait
keys supply all12 mod locales; names use the shared approved snapshot resolver,
never raw developer group IDs. The spoken identifier may use the common role
without a browse-list ordinal; identity and collision remain actor-specific.

## Native evidence and ownership

All-six matching native FieldCharaEntity.CanEnter reads boxCollider2D. No collider
or a trigger collider permits passage; non-trigger collision can block. Previous
live_blockers treated every NPC as solid, irrespective of that native state.
The owner now name-resolves FieldCharaEntity.boxCollider2D and
UnityEngine.Collider2D.get_isTrigger, copying only known/solid booleans. If those
observations are unavailable, a matched bake's collision flag supplies the fallback;
an unmatched actor remains conservative. Parked-party classification remains
separate. The copied flag is shared by all consumers of live_blockers.

FFII concrete evidence: FieldCharaEntity.CanEnter RVA7DE7D0 reads collider field
0x140 and calls Collider2D.get_isTrigger RVA1535BA0; FieldNonPlayer.CanEnter
RVAC4EF60 delegates to that base check before its native movement/overlap rules.
These are evidence locators, not runtime offsets: runtime access is by name.
All-six bounded disassembly/provenance is outside Git at
../tools/npc_fynn_20261023/ff1 through ff6, generated with original
 tools/re/ff6_story_native.py --visibility. No game code is copied into this repo.

Native observation stays in capture_obstacle on the existing field owner after
entity-lifetime checks. The existing elapsed CaptureBudget yields between actors;
workers consume only owned coordinates, identity, text and booleans. Failed native
calls stay conservative. No native collider invocation happens on Navigation.
No callbacks wait for planning, locks, speech or I/O. The existing worker owner,
request generation and map/input/battle invalidation discard obsolete results.

## Source and safe extension

field/baked_nav.cpp owns capture_obstacle, live_blockers, npc_approach, same-floor
find and cross-floor route_to_goal. Blockers keeps physical NPC cells and owned
interaction metadata. field/npc_route.h owns only the pure geometry/interaction
prefix. PathInfo and ActiveRoute carry npc_approach independently of blocking_party.
find_across's optional NPC-free proof affects only captured current-floor NPCs;
remote floors retain their projected actors and gates. MapGoal identity and Pending
retain the original destination. No nearest-actor heuristic or route deprecation is
permitted as a replacement for reachability proof.

The existing large baked_nav adapter stays the central owner of snapshot/route
state; this change puts its cohesive pure NPC geometry in a separate small header.
It does not split shared cancellation/lifetime state into duplicate services.
Future obstacle types must preserve the proof's other blockers, legal approach
masks, manual arrival semantics and all downstream partial-route consumers.

## Original Castle Fynn verification and limits

Canonical build includes tools/npc_gate_check.cpp, which runs the actual production
snapshot planner on FFII Castle Fynn data. Checks cover the logged soldier layout,
true choke, passable actors, actor movement, cross-floor handback and a separate
closed gate. navigation_speech_check covers remote browse goal/reason publication;
route_worker checks native-owner separation. Existing parked-party, grouped-exit,
trap, autowalk and all-six name checks remain required. Canonical build/all regressions and isolated47-export smoke pass;7,206 native
names/zero missing and125 used/333 defined actions/zero undefined. All six installed
DLLs match SHA2563883266ace174e8b81fb0808260ec2b11cb3ae9caf7a7c21711fc107f101cd40,
10647552 bytes. All144 protected files are unchanged; recovery and reports are in
build_check/npc_fynn_20261023/. [Complete evidence](CASTLE_FYNN_20261023.md). No new runtime capture, launch or gameplay
confirmation is claimed. Exact game-thread callback cost, audible response and
moving-soldier collision timing remain user-side; offline timings are computation
and fake-owner evidence only.

## October23 Mysidia detour and temporary occupancy follow-up

User clarification: an actor entering the direct route is not sufficient to report
a blocker. Exhaust legal alternate routes first. Blocker interaction applies to a
true narrow channel or area gate; a travelling actor in an open approach is temporary.
The current FFII log at1121728.093 records map90, start(21,6), inn goal(29,13),
patrol49 at(29,14), followed by an incorrect Interact action. The baked doorway
only has a cardinal entrance from(29,14); west/east approach cells(28,14)/(30,14)
are open, but the wall beside the doorway prevents a cardinal bypass at that instant.
No diagonal connection or false completed route is introduced. Exact interpolated
actor position was not logged; this reconstruction uses the logged occupied cell.
The other seven live patrol positions are not present in that route log; the
Mysidia fixture isolates actor49 rather than claiming a full captured scene.

Historical October23 design, superseded October25: retain the exhaustive complete-route search and live occupancy, then
classify the proved first obstruction. A known travelling controller in an open
approach produces a safe prefix ending with Wait for the named actor to move.
A narrow channel (opposite adjacent sides closed under the real grid, directional
and solid-obstacle rules) or stationary gatekeeper retains manual interaction.
The original destination, partial-route identity and beacon non-arrival stay intact.
Autowalk invalidates a newly colliding edge and requests a fresh owned snapshot;
it pauses input during planning, tries every detour and resumes a usable route.
Temporary occupancy retries at most twice a second, with one request in flight
and a30-second episode bound. Input/focus/menu/battle/map cancellation retires
the generation; no native callback waits for planning. Gameplay confirmation and
actual response/frame/audible timing remain pending.

### Source, lifecycle and extension contracts

field/npc_route.h Actor.wanders now comes first from the copied native routine,
with existing baker flag16 as fallback; None/Stamp remains
stationary. live_blockers supplies current positions, solidity and interaction
metadata once from the owned snapshot. npc_route::approach runs only after all
complete same-floor/cross-floor attempts fail and an NPC-free proof succeeds.
Its removed narrow_channel classifier no longer overrides authored movement.
This changes terminal guidance, not the route algorithm or collision permission.

route_progress::Phase.wait_for_npc is independent of manual_after and physical
facing. autowalk_route::Route::next returns Wait at that boundary, never Action
or Arrived. PathInfo/ActiveRoute.npc_approach continue to mean a partial route, so
pending targets, closest-party comparisons, scenario instructions and beacons
cannot count it as the original destination. The matching goal is always retained.

fieldnav/autowalk.cpp retry_obstacle marks state only from axis. input_updated
checks focus/context/input/map and the saved goal, then invokes existing request_route
for bounded native capture plus Navigation computation. It owns one pending request,
a retry time/deadline and a walk generation. cancel retires that generation and
requests; battle/party events release movement immediately. A late completion
cannot revive a cancelled or newer walk. A native veto or a two-second stalled
tile requests a fresh detour. One semantic notice belongs to the continuous wait
episode; no string comparison or text-content deduplication is used. Publishing
a route alone cannot reset the30-second limit: actual native displacement does.
No interaction/confirm input is synthesized for a wait.

baked_nav build_corner marks the terminal wait corner missing without marking
arrival. Both current-map and remote-goal followers then use their existing
two-second retry throttle. Fresh publication replaces the old partial route;
manual gate approaches stay fixed until a new request/interaction. Autowalk and
beacon use the existing bounded route owner, so concurrent requests cannot build
an unbounded backlog. Directions-only guidance refreshes on the next normal request.

All six share these producers; the game adapters contain no separate NPC route
or autowalk collision implementation. Native collider observation and per-game
bake controller/collision flags remain authoritative. Parked-party, treasure,
hazard, hidden-passage, vehicle, trap-history and remote-gate rules are unchanged.
Future changes must preserve exhaustive detours, legal interaction geometry,
wait/manual/arrival distinctions, request generations and downstream partial-route
consumers. Add new user text through mod_text.json with all12 locales.

### Offline evidence and remaining gameplay checks

The existing tools/npc_gate_check.cpp runs the real production snapshot planner
on the exact FFII map90 entrance and logged patrol position, plus the ordinary
occupied-home detour, a moved actor, a walled entrance and stationary gatekeeper.
It checks no occupied-cell traversal, wait versus action/arrival, beacon refresh
and the original Fynn remote-gate/cross-floor cases. tools/autowalk_native_check.cpp
runs the actual owner/input implementation in all six game modes with a native
edge veto, fresh worker completion, stalled tile, retry cadence, one request in
flight, deadline, manual/focus cancellation and stale completion after a new walk.
These are mod-owned fixtures using real baked geometry; no game function is executed.

The bounded optional --npc-detours mode of tools/re/ff6_story_native.py retains
matching FFII movement/CanMove native disassembly plus binary/dump hashes outside
Git at ../tools/npc_detours_20261023/ff2. No new native API, movement edge, grid edit,
live capture or diagonal assumption was added. Existing MAP_EXITS/native collision
evidence remains the movement contract. Runtime tests should follow
[TESTING: Mysidia first](../TESTING.md#october23-npc-detour-follow-up-test-mysidia-first).
The existing feature index was checked against126 active registrations; none is
unindexed. Unchanged feature families retain their linked authoritative contracts.

### Clean build and delivery record

Sol completed implementation/review/verification without delegation. Canonical
build.bat --clean and all offline checks passed, with7,301 native names/zero missing,
125 used/333 defined actions/zero undefined. The canonical isolated smoke test
passed all47 exports. All49 data resources (43 JSON/six map binaries) and18 sounds
match the current source bytes embedded in hid.dll. DLL size10,693,632 bytes;
SHA25688a59ebf382fe265090833dce43729ebce7ae457b5663c994315b5a5ed6e7cf5.

The sandbox's build stalled at file-glob checking and was cancelled. The first
elevated incremental build passed; final review found a stale fixture object after
a source edit during compilation, so acceptance uses the subsequent clean rebuild
and its newly compiled Mysidia/beacon/native-owner checks. No source changed during
that clean build. Recovery, original user log, current-source hashes, all reports,
resource verification and deployment receipts are in ignored
build_check/npc_detours_20261023/.

The real-geometry fixture's coarse-clock compute sample was0/15/0 ms for ordinary
detour/occupied doorway/cleared doorway. Zero means below this fixture clock's
resolution, not zero work. This is one offline sample under build load, without a
comparable prior-build baseline; native callback/aggregate cost, full snapshot age,
request-to-usable/audible response, tail delay and actual collision timing remain
unmeasured. State/cadence/deadline/cancellation checks establish bounds, not a
claim of measured gameplay responsiveness.

Initial deployment installed and hash-verified I/III/IV/V/VI. The script refused
running FFII before any payload/dependency writes. After the user closed FFII and
requested deployment, Sol verified the DLL and recorded source hashes still match
the clean-build/smoke receipt, captured the current preferences and used deploy.bat
ff2. All six DLLs and SDL payloads now match the tested sources, and all144 protected
files match that fresh pre-deploy baseline. FFII's live-session INI changes were
preserved. Original/partial/final receipts remain separate in the evidence folder.
Gameplay confirmation remains pending. No game was launched, controlled or probed;
no public release was replaced.
