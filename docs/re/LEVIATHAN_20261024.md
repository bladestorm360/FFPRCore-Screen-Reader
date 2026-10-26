# Leviathan world-map scene entry

Status: complete; implemented/offline verified and deployed/hash-verified all six.
The FFII hold was respected until routing checks passed. FFII was closed when
final deployment ran, so installation completed.
The tested [three-part MP correction](PARASITE_THREE_PARTS_20261024.md) remains
included. Equipment speech is user-confirmed and unchanged.

## Cause and evidence

The current user log repeatedly rejects map1 target(57,132), layer1, Infer,
even aboard transport4/type2 Ship. It reports no goal cells, so the search never
starts. The last logged boarding cell is(30,194). This is not a distance cap.

Extracted Map_10010 object295/entity108 is the automatic Event in ev_e_0070,
script96/sc_e_0070. Raw(57,133) becomes baked(57,132) after the existing MapRange
crop. Its script starts the sea scene, then invokes native disembarking inside
Main before ChangeMap to228/point1/continuation sc_e_0070_1. Thus the player
must sail onto the trigger; a prior land route is fictional. Scenario65 selects
the active group until it is set: the initializer skips completed earlier
scenes, selects this group while65 is0, then skips it when65 is1. Existing script/map metadata remains outside Git at
../tools/mapdump/ff2_{objects,scripts}.jsonl and ff2_master/script.txt.

The generic transition_enterable rule excluded Ship for every world transition,
including scripted sea scenes. Goal discovery also selected a walking footprint
for explicit transitions. That is correct for native town doors but rejected
this verified mounted script entry.

## Design and extension

bake_maps.py marks only this reviewed script identity with Warp.d bit17,
kShipEntry. Its existing step-on activation, exact cell, destination, group and
entity identity remain intact. The recipe asserts the native destination and
the disembark command in Main; it does not infer permission from water alone.
The final bake differs in one byte/one semantic bit. Terrain, grids, object
counts, identifiers, scripts and destinations are otherwise byte-identical.

baked_nav::goals_for obtains that destination's Ship movement footprint from
the existing transport master. This supplies geometry, not reachability. The
normal planner must prove the current sailing route or actual owned-Ship
boarding/sailing legs. No vehicle is granted or teleported. Baked router final
goal edges and transition eligibility permit Ship only for marked step-on
scripts whose terrain its actual group crosses. Native GotoMap doors still
require walking/auto-use transport, and airships gain no Ship-entry permission.

Event eligibility uses the same goal geometry. When on foot with the vehicles
filter, a marked sea scene gets an isolated owned snapshot through the existing
full boarding planner, since the ordinary parked-vehicle flood only returns
land cells. This fallback is limited to reviewed Ship-entry scripts; it preserves
state/cancellation and does not turn a missing Ship into a valid route. The
ordinary mounted Event case still uses one flood and its final goal edge.

All work is existing immutable bake/owned snapshot computation on Navigation.
No native callback scan, scene search, mutation, wait or pointer handoff is
added. The small object/transport checks occur during worker goal/edge handling;
the assisted snapshot is heap-owned. Review new script identities/activation,
vehicle type and script timing before extending the recipe. No new controls,
preferences, speech keys or localization changes.

## Required verification

navigation_followup_check uses the real bake, scenario65 and logged Ship cell.
It must prove full directions, Event browse and eligibility reach the exact
trigger while still aboard; native town and wrong-vehicle rejection remain.
The logged walking origin(30,185)/parked Ship(30,194) must produce real boarding
and sailing. Removing that Ship or disabling the story group must fail.
Existing all-six terrain/vehicle/transition/trap/gate/cancellation tests remain.
No game launch/control or runtime probe is authorized/performed. User listening
and actual scene activation remain pending after installation.


## Final verification and delivery

The boarding planner reuses the native WalkOn edge, sails to the exact selected
sea trigger and ends aboard. The script owns disembarking. Ordinary destinations
retain landing/walking phases. The sea-scene branch omits an unnecessary walking
area flood. Its reviewed script is digest-bound; changed source requires review.
Existing baked_nav organization remains the documented shared-planner exception
in THREADING.md, retaining the central snapshot/state/phase pipeline.

Actual-bake checks pass:375 steps from logged Ship(30,194), the same375 eligibility
distance, direct Event listing and exact(57,132) endpoint without landing. The
shore origin(30,185) can board the actual parked Ship and sail there; removing
that Ship fails, as does completing flag65. Native town doors and wrong vehicle
types remain rejected. All-six existing vehicle/exit/gate/cancel checks pass;
the expanded same-owner physical/MP-drain/recovery regression also passes.

Canonical build/all checks:7409 names/zero missing,131 used/339 defined actions/
zero undefined.47-export smoke passes normally;49 embedded data+18 sounds match.
Targeted digest/bake verification after recipe hardening reproduces exactly the
embedded bytes. All six installed DLLs:10803712 bytes, SHA256
c3a7160030c86c010aad513c139f3a8b9b78d01a3a68c350724657773b3aede7.
All144 protected preferences/waypoint/dependency files are unchanged. Every
installed payload matches the final22-file Defender-scanned stage: no threats,
exit0, no matching detection events and unchanged hashes. Existing unsigned/
no-certificate and administrator-protected exclusion-query gaps remain.

Reports, source/resource hashes, retained intermediate fixture failures, user log,
one-bit bake delta and recovery/deployment/security receipts are in
build_check/leviathan_20261024/. The fixture first tried assigning immutable state,
then used an incorrect completed flag65; both were corrected to constructed
snapshots and the actual earlier-scene progress conditions with65 unset. No
production story gate was bypassed. The valid combined sailing/list/eligibility
fixture measured53.132ms; the earlier fast rejection never searched, so it is
not a comparable usable-route latency baseline. Native capture, queues, frame
tails, physical audio and actual scene activation remain gameplay-unconfirmed.
No game launch/control/probe, dependency install, delegation, commit/push or
public release change occurred.
