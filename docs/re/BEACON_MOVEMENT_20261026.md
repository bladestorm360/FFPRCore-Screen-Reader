# Beacon movement and spatial bearing, October26

Status: complete, Closed: implemented/offline verified/installed and hash-verified
all six after the user closed FFIII. Gameplay remains unconfirmed.
User initiated this focused
follow-up to the completed audio/navigation design. Sol owns it without delegation.

## Scope ledger and finite checkpoint

| Field | Active boundary |
| --- | --- |
| Capability | Route beacon continues its ping cadence while walking in either HRTF or stereo. Active HRTF uses the actual full bearing without rear detuning; stereo retains the rear pitch cue. |
| Evidence | cached_corner keys its worker result by the exact player cell. Each moving ping asks another worker and returns Pending: local targets get the blocked tone, remote targets silence. Actual user log confirms active HRTF and repeated kept routes while moving. sound_player reconstructs north from a saturated pan, losing rear diagonal bearing. |
| Implementation | Existing route follower/cache and beacon producer/front end; existing worker-owned mixer/HRTF are dependencies. Preserve planner, route phases, native movement and soundscape owners. |
| Finish line | Actual source-to-ping tests for continuous movement/turns/arrival/off-route/remote transitions, full HRTF bearing and fallback; complete build/smoke/security/delivery/docs. |
| Exclusions | No template edits, battle command remapping, soundscape rewrite, new hooks/settings, or reopening locked doors, storage, statuses and exit listing. The accidental pasted request was withdrawn. |
| Bounds | Build one immutable corner index per published route on Navigation, cancel during traversal; no per-footstep route scan or wait. Existing bounded audio queue/mixer and lifecycle cancellation remain. Measure full repeated moving episode, not only one lookup; cumulative mod-caused application lag above25ms blocks acceptance. |
| Effort | Expected45-60 minutes for focused implementation, connected regressions and canonical delivery after diagnosis; report a material overrun or new dependency. |
| Recovery | build_check/beacon_movement_20261026/source_before/ plus source hashes, baseline check executables and the user's own current FFIII log. No game launched/probed. |
| Closure | Closed at the final all-six receipt. Further refactoring needs a human-scoped follow-up. Gameplay confirmation remains separate. |

Scope checked at start/resume and before implementation: the existing worker
corner cache is the necessary current beacon dependency; no broader navigation
redesign is authorized. Preserve completed source and user work.

## Implemented source-to-output path

The published route, movement planner, collision and native input owners remain
unchanged. beacon_route_index.h builds one contiguous, sorted lookup from the
immutable route phases on the existing Navigation worker. It preserves the
former remaining()/corner_of semantics: final matching occurrence of revisited
cells, remaining distance, phase boundaries, diagonal/layer edges, world-wrap
projection, partial approaches and patrol endpoints. Both follow and follow_goal
reuse that lookup by route/map/goal identity; changing the player's cell selects
another entry without allocating, scanning a route or queueing another worker.
An off-route cell remains Missing and uses the existing throttled silent planner.

The helper receives the production coordinate converter rather than duplicating
the bake origin/scale calculation. One vector stores at most the number of cells
already present in the route phases. Construction visits cells once, sorts on
the worker, then removes duplicate keys while preserving the last native suffix
occurrence. Lookup is logarithmic with no allocation. Cancellation is checked
during traversal and after the finite sort; no retry or busy loop is introduced.
Old/new route identity and existing owner epochs prevent obsolete publication.

Corner.pending now distinguishes in-flight work from a completed blocked result.
audio_loops.cpp retains only one copied known corner for the same map/selected
goal. During replanning it maintains the normal ping cadence from the CURRENT
player position. Target identity/coordinates/map/entity changes, arrival, completed
failure, context suppression and loop teardown retire that provisional point.
Before any corner is known, pending work is checked after50ms, without calling
it blocked or inventing a local bearing to a remote floor. This is initial data
readiness, not a dependency on the player becoming idle. No native pointer enters
the retained audio state or worker.

sound::play_beacon now publishes the full east/north displacement through the
existing bounded64-request queue. The audio worker's effective backend chooses
the output: active HRTF receives the normalized actual vector and the ordinary
beacon at original pitch; stereo/unavailable HRTF keeps the old clamp(east/100)
pan,0.7x rear pitch and proved-blocked half tone. The side boundary north=0 is
unshifted. No directional/blocked detuning is layered onto active HRTF. Custom
normal beacon clips use the same unshifted HRTF path. The established orientation
is preserved: map north is audio front, map south audio rear, with no head or
character-facing tracking. This scoped repair does not rotate the soundscape or
other cues. It corrects the previous loss of rear diagonals when stereo pan was
saturated before reconstructing the HRTF position.

The existing Audio worker owns synthesis, OpenAL Soft loopback, mode activation,
actual HRTF status, rendering and device submission. Game callbacks publish only
copied scalars. Footstep and beacon retain independent mixer slots0 and6; no new
hook, SDL owner/device, setting, control, localized string or dependency is added.
Existing master/field epochs and bounded mixer/queue cancellation remain.

## Closed-system record and separately reported issues

| System | Boundary and accepted evidence | State |
| --- | --- | --- |
| Beacon follow-up | This user's moving cadence and full HRTF bearing request only; finite checks/delivery still open | Open |
| [Locked doors](LOCKED_DOORS_20261026.md) | Objects interaction policy/native locks unchanged; prior full offline/scan/install receipt574da7fc17ed8c8a0b2dcc7697566b0833370263f8b31f1bf61cd322da77aa2b | Closed; gameplay pending |
| [Storage](WAREHOUSE_20261026.md), [return exits](REMOTE_RETURNS_20261026.md), [statuses](ALLY_STATUS_20261026.md) | Their documented adapters/data/interfaces retained in the same verified installed payload | Closed; their listening confirmations remain separate |

Scope checked before implementation and at the compile dependency: the coordinate
converter belongs to this route-index integration. No closed system is redesigned.
The initial helper build lacked that converter declaration; failure retained and
the helper now accepts the existing converter. A patrol test directly consumed
the former scalar corner result and requires the equivalent index lookup; this
is a test-interface integration change, not patrol behavior redesign.

The user's battle-command question was answered from the supplied page1/2 image
and native job15/message tables: Scholar has Attack, Study and Alchemy, without
Magic; Alchemy replaces normal item use. A first lookup mistakenly inspected
Black Belt's Items entry; the answer was corrected promptly. No controls were
remapped. The accidental template request was withdrawn; no template edits.

The later Mallet16 report is resolved by the user: the supplied image displays
Items with Mallet highlighted and count16, not a spell menu. No battle/menu
production correction was needed. Source-bearing native data remains outside Git.

## Verification and measured limits

The canonical build and full offline suite pass:8275 native names/0 missing,
141 used/348 defined/0 undefined wiring,47 proxy exports and the isolated DLL
smoke host. No game was launched or probed. Existing all-six route, transition,
vehicle, map-state, gate, world-wrap, footstep, audio-master and lifecycle checks
remain enabled. The old patrol boundary assertion now reads its indexed cell;
it still requires missing guidance rather than false completed arrival.

The real route_worker_check compares11,872 cell queries with the former exact
remaining()/corner_of result, including turn boundaries, repeated coordinates,
empty/vehicle-style phases, layers, world seams, partial paths and patrol stops.
Cancellation yields no partial index; an off-route cell yields no stale corner.
The actual Fynn route/cache is published once, then every moving tile is Ready
without an idle publication. A300-pass moving episode performs2,400 actual cache
reads with zero worker submissions in0.061ms cumulative owner compute. This
finite whole episode is below the25ms blocker, not a per-call allowance. The
pre-change archived source keys the result by exact at cell and returns Pending
when it changes, establishing the concrete information/cadence defect before
the index change; no fictional speedup ratio is claimed.

beacon_check drives160 moving iterations through the actual loop in both modes,
and a pending replan after each, requiring ordinary pings rather than silence or
the blocked tone. Existing local/remote/waypoint/wrap/arrival/settings/context
checks also pass. sound_player_check verifies the actual front-end/master and
volume ownership. soundscape_check exercises the production mixer and real
OpenAL Soft loopback: all eight bearings retain normalized front/rear/diagonal
coordinates and original beacon PCM pitch. Fifteen stereo/failure combinations
retain the former pan and rear waveform, including an unshifted side boundary.
These tests reach production consumers; fake boundaries are native game objects,
game-frame scheduling and the physical SDL device.

The unchanged mixer stress test measures128 active HRTF voices at1.031ms mean/
1.348ms peak per20ms render block, with45.752ms cold open on the AUDIO worker.
Owned snapshot to fake SDL is0.023ms;400 entity scheduler pressure remains fair,
with0.324ms worst worker chunk. These timings are not native interception, actual
game frames, physical audible response or a universal backend/device guarantee.
The user's prior log records route native capture0.078-0.140ms and16-34ms elapsed
worker/frame readiness while moving; elapsed asynchronous readiness is distinct
from application blocking. New in-game cumulative callback/native/audio-tail
measurements remain unconfirmed; no demonstrated >25ms application stall is
hidden by the offline timings.

No new callback or worker lane was added. Native per-frame timer checks and
existing ready pointer/scalar reads retain their prior scope. Normal pings remain
paced by distance at0.2-1s; pending first readiness checks at most20/s and shares
one in-flight index job. One contiguous index is retained per published route,
with no per-footstep result backlog. Construction/sort and the optional one-off
replan stay on Navigation; spatial choice/synthesis stay on Audio. Existing audio
queue64, bounded three-chunk submission, epochs, stop/focus/menu/battle transitions
and stale-observation cancellation remain authoritative. Actual native allocator,
driver and scene transition tail limits are not invented from the fake SDK.

## Exact delivery, security and remaining gameplay checks

DLL10993152 bytes, SHA256
ea355feca25bf1c7bad59630a2c994ec0fb711a1c3cdd8d75fab111742c64079.
The exact22-file built payload/dependency stage passed enabled/current Defender
1.461.211.0: no threats, zero matching detection events and unchanged hashes.
Exclusions remain unverified because the query is denied; signing certificates0,
unsigned payload, outer package/reputation and other-scanner acceptance untested.
No protection/dependency change, upload, public release or game control/probe.

Guarded deployment initially installed I/II/IV/V/VI and refused running FFIII.
After the user closed it, deploy.bat ff3 succeeded. All132 scanned payload hashes
match and144 protected entries remain unchanged against the fresh preclosed
baseline. No forced replacement or game termination.
Before-task recovery and the fresh final baseline are retained separately from
the user's live-session changes. All prior completed fixes are included.

Now installed in FFIII, test first during ordinary play:

1. Choose a reachable destination and start its route beacon. Walk continuously
   through a straight stretch and a turn. Expect regular pings while moving in
   HRTF and stereo, no idle-only gap and no temporary blocked pitch each step.
2. With HRTF on, put the target/corner south or diagonally south: expect an actual
   rear bearing at the ordinary pitch. With HRTF off, the rear cue should be lower.
   The established map-north-forward orientation is retained.
3. Arrive, switch targets, open a menu and change floors naturally. Arrival/menu
   should silence output; a new target/floor should not replay the old corner.

Gameplay/listening and native audible/frame timing remain separate confirmations.
Evidence/recovery: build_check/beacon_movement_20261026/, including archived
pre-change sources/check executables/reports, both compile failures, final suite,
exact security stage, smoke and protected/deployment receipts. No failed
intermediate DLL was installed. Scope checked at offline milestone closure:
only the beacon and its necessary follower/cache interfaces changed.

Offline source milestone Closed at payload
ea355feca25bf1c7bad59630a2c994ec0fb711a1c3cdd8d75fab111742c64079:
133 feature registrations documented, all67 embedded resources match, six bake
hashes unchanged,132 installed hashes verified and144 protected entries unchanged
against the fresh preclosed baseline. Further source refactoring requires a new
human-scoped follow-up. Delivery is Closed after the authorized FFIII copy; prior refusal/recovery and
live-session baselines remain preserved. The user then requested release1.0, a
separate packaging/publication checkpoint in docs/release.md.
