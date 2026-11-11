# Native Player records and world-to-interior foot routes

Owner report: FFIV Exit to Mist incorrectly says Requires vehicle; walking
through Mist Cave should be available when story state permits. The retained
gameplay log reproduces that label at world1 cell107,145, target101,106, with
zero current chests/gates/NPC obstacles. After physically crossing the cave,
the same target is directly reachable from89,107. Installed FFIV DLL is
a6d9b4ce19dfb6ad, matching the October30 all-six delivery receipt.

## Confirmed cause and minimal correction

navigation.cpp::current_vehicle marks every ready native transportation record
valid, including Player ID1/type1 while walking. The shared cross-map planner
already proved a foot journey but its first-hop reconstruction interpreted any
valid record as requiring a successful boarding planner. No parked craft meant
that reconstruction returned failure and the generic world vehicle explanation
masked the valid cave route. The source and installed-source hashes match the
recorded a6d9 inputs; this is not merely a missing cave-detour deployment.

mist_route_check now uses the actual logged pose, exact baked Mist door/identity,
all eight cave Scenario6/102/103 combinations, both cave-side arrivals, and both
the native Player record and the former invalid-record fixture. The latter
always passed and hid this defect. The native Player case reproduces the exact
Requires vehicle label on the Baron side, including eight post-boss assertion
failures. Before reports/executable/source are retained under
build_check/navigation_resume_20261031/mist_player_before_*.

The reconstruction now requires boarding only for an actual mounted movement
group or when walking has no route. The related root reach check recognizes a
valid Player record as foot mode for optional owned parked-vehicle assistance.
Native modes, ownership, terrain, cave layers, story gates and strict publication
remain unchanged. No hook, native call, main-thread search or wait is added.
This is shared all-six behavior; world_player_hop_check uses each game's actual
Player master, world entrance and native interior arrival to compare first hops.
Mounted vehicle and all-six route-worker checks pass, including native Player
parked-craft changes. All six actual world/interior first-hop witnesses pass;
1200 stationary category inputs schedule no redundant scan. Frozen14 inputs,
five reports and capped resource receipts are in
build_check/navigation_resume_20261031/mist_checkpoint/.

Current focused Mist result: all48 route/browse cases pass with zero native
worker calls; the eight demonstrated post-boss assertion failures are zero.
Baron-side requests publish Mist Cave162 as the next map, while the far-side
control keeps its direct13-cell path. Both real cave arrivals also retain the
correct northern world arrival with four unopened chests kept solid. Source/
focused qualification is Closed; combined delivery is pending, not installed.

## Rejected approaches and verification limits

Fully clear/set flags and an invalid-record walking fixture passed without
reproducing the report. The already corrected native layer contract was tested
against its exact old incoming-layer rule; both still passed these Mist routes,
so that hypothesis was rejected and its offline-only switch was removed.
The mixed-state invalid-record tests also passed. These are preserved evidence,
not additional runtime fixes. The native Player distinction is the demonstrated
counterexample. No game was launched/probed. Current source/focused qualification,
combined canonical/security/deployment and natural confirmation remain separate
in the single scope ledger.
