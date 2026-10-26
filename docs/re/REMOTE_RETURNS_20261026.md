# Remote return-exit listing, October26

Status: implementation complete, offline verified and installed/hash-verified
all six with the [Warehouse update](WAREHOUSE_20261026.md). Gameplay pending.
No game launch, control or runtime probe.

## Report and root cause

The user's own FFIII log shows current Dwarven Hollows map84 with19 Map Exit
entries:2 current exits plus17 remote exits. It includes B2-to-B3 and B3-to-B2,
room returns, and Subterranean Lake returns before the player reaches those
floors. The prior bake audit correctly restored internal GotoMap sources after
a group-membership filter dropped real room entrances. Publishing all those
directions remotely also added the redundant reverse doors.

The fix changes the remote listing only. The binary bakes, complete directed
routing graph, native transitions, current-floor exits and exact selected goals
are preserved. No text-content deduplication, new setting or control.

## Presentation policy and source flow

baked_nav.cpp::connected_floors retains directed breadth-first discovery from
the current submap, without crossing world maps, through active/unconsumed native
doors and script warps. Floor.depth now records minimum transition depth; it does
not use translated names, floor numbers, current distance or visitation history.

connected_objects_snapshot prepares remote_return_doors on the existing worker.
One bounded prepass over these floors plus the current floor copies active
ordinary GotoMap footprints and exact PointIn identities. Known state is required
on both maps. Duplicate/missing PointIn IDs remain uncertified; the planner's
fallback to a first/default arrival is never used as suppression evidence.

field/remote_exit_policy.h owns the pure listing rule. A remote door is omitted
only when it leads to a smaller discovered depth and exactly one opposite native
ordinary door matches both endpoints: each exact arrival is on or cardinally
adjacent to its counterpart's footprint on the same layer. Ambiguous matches
remain. The mask becomes a set of immutable baked object identities consumed
only by the remote GotoMap case; it never enters route or transition construction.

Preserved cases: current-floor doors, forward room/floor entrances, same-depth
loops, other physical entrances between the same maps, world/external exits,
unpaired one-way exits, script warps, unknown state and uncertain arrival geometry.
When the current floor changes, depth and presentation are rebuilt; a physical
return on the current floor remains native/live-owned and usable.

This is deliberately conservative. Dwarven Inn/Item Shop spawns two cells from
its return footprint, beyond the direct doorway witness, so that uncertain
return remains listed. It is not safe to shorten the list by treating all edges
to an earlier floor as redundant. The exact reported B3-to-B2 stair return is
confirmed and removed remotely.

## Exact results and regression coverage

Dwarven B1 now projects11 remote exits instead of17; with2 live exits,13 instead
of19 total. The B2-to-B3 entrance remains; its B3-to-B2 return and the remote
B2-to-current-B1 return are omitted. The Subterranean Lake entrance stays while
its ordinary paired return is omitted. Inn/Item Shop entrance and its uncertain
return both remain. Real routes from B1 onward and B3 back through B2 still work,
and changing the root to B3 exposes the onward B2-to-B1 entry.

tools/remote_exit_check.cpp exercises the actual projection/planner and pure
policy: forward/current doors, reciprocal returns, same-depth loops, missing/
ambiguous metadata, differing layers, distinct physical exits, unpaired one-way
connections and cancellation. Real all-six largest graphs retain destinations;
zero worker native access. Existing Kazus/Sasune/Mines/Tozus/counter and all-six
map-exit/trap/transition/gate tests pass. Sasune's obsolete expectation that its
own Spring entrance remain a remote row was corrected; genuine future Spring
entrances and Sara's NPC remain covered from other floors.

The all-six event audit repeats2454 master roots in7362 state projections with
all2504 unique projected event identities and exactly the same per-game event-row
counts as the archived baseline. Scripted world exits remain covered. All six
maps.bin hashes match the pre-change baseline; no bake entry was deleted.

## Ownership, bounds and comparable measurements

No new native capture, callback, timer, pointer transfer or wait. Immutable baked
objects, copied flags, map indexes and worker cancellation are the only inputs.
The existing connected-floor bound is400 maps. PointIn and direction indexes
cover only objects in those discovered maps; reciprocal matching is indexed by
source/destination pair. Temporary policy/index/mask storage retires with the
request. Cancellation is checked in map/object/index and match loops. Incomplete
discovery leaves unclassified destinations visible.

The archived baseline executable and changed event_tracking_check use the same
real data,2454 roots and three flag states. Worst projection ms, baseline/changed:
I0.460/0.460, II0.688/0.708, III0.460/0.533, IV0.921/0.960,
V1.185/1.143, VI1.935/2.588. Largest list counts change114/112,122/110,
165/165,209/193,256/224,630/618. A separate largest-graph smoke covers
13/36/18/33/52/43 remote floors respectively. These are harness compute/timing
observations, not native capture, engine frame, queue-tail or audible latency.
Existing obsolete-job cancellation/publication guards remain authoritative.

## Verification and delivery

Canonical8275 native-name checks/0 missing, wiring141 used/348 defined/0 undefined,
full offline checks,47-export smoke and49data/18sound resource checks pass.
The initial test incorrectly expected the non-adjacent Inn return to be hidden;
that expectation was corrected to preserve the uncertain native geometry.
The legacy Sasune current-Spring expectation was also updated. Both failure
reports are retained. No failing intermediate DLL was installed.

Source: field/baked_nav.cpp/.h, field/remote_exit_policy.h; production tests
remote_exit_check.cpp and existing npc_floors_check.cpp; build.bat runs the new
regression. The small pure policy is separate from the legacy shared planner;
the collection adapter remains with its existing connected-floor responsibility.

Combined DLL10983424 bytes SHA256
bc93b87173047349cf8d6e74ca3a7b1eb654445c72d5812f7fbafa740db3b8e4,
scanned and installed all six after the user closed FFIII.132 installed hashes/
144 protected files verified. Exact scan/signing/exclusion limits are in the
[Warehouse delivery](WAREHOUSE_20261026.md) and
[security record](../SECURITY_AND_DISTRIBUTION.md).
Evidence/recovery:build_check/remote_returns_20261026/, including the initial own
log, baseline executable/projection report, bake hashes, test failures and final
reports/scan/receipts. No game source snippets, release or public upload.
[Gameplay checks](../TESTING.md) remain pending.
