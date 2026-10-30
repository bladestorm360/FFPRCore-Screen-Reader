# Ghido's Cave scripted shore landing

Source/data/targeted offline correction complete in the human-requested all-six map/NPC audit.
Estimate20–40minutes through exact data/source/owned-consumer acceptance. It
uses the existing same-map arrival-override contract, without reopening Closed
routing or instruction-capture designs. FFIII63a24c76 is already installed;
no game launch/control/probe or other-game deployment is authorized here.

## Exact missing connection

FFV Map_30381_7/master375 (Ghido's Cave), native Event10/editor11, ev_e_0436,
script2055/sc_e_0436 is already correctly labelled Passage. The fully reviewed
player_movement family is pinned in world_event_reviews.json. It has no dialogue,
encounter, map change or reward. Its controlled player track first sets the
position to(12,35), moves toY34, then moves toY32 while keeping the Ground layer.
The visible party clone follows the same movement. The final controlled position
is unambiguously(12,32,L1), independent of its previous X/Y. No later command
moves it elsewhere. The script's only final persistent action sets a flag.

Main calls7/18/29; controlled actor0 entries15/26/37; SetPos16 and final MoveTo39
are the source locators. Source and native provenance remain outside Git under
../tools/npc_geometry_context_20261029/ and ../tools/mapdump/. Whole-source hash
cb797d4ea8762cc17cb39e46e1eecda714f0d7e50a1277958219b983d90870e3
matches the existing functional review. Matching FFV native Puppet.MoveTo2D7F80,
GetDirectTargetPosition2D57C0 and its map-range conversion establish absolute Y
and keep-layer operands. The map crop origin is0,0. No native script is executed.

Both NPC variants at(13,15,L1), IDs2/16, have four reachable approaches from that
real landing. Ordinary PointIns1/2 are at(12,37)/(12,40), below the scripted gap.
There was no graph connection for the crossing. Actual owned planner replay of
both native variants and both entrances fails before the fix (4 failures), while
all four replays already succeed after placing the controlled player at the
actual native landing. This distinguishes missing route data from an incorrect
NPC layer, counter, walking grid or general story gate.

## Bounded data integration

review_ghido_landing.py checks the exact actor, complete pinned source, existing
physical-movement grammar and controlled-player entries before appending the
one override. No runtime script interpretation, hook, native call, input, cache
or new UI string is added. bake_maps uses its existing same-map override path.
The source trigger stays a native Event; one companion Warp and one graph-only
PointIn describe its scripted landing. Existing destination/category/selected
route and same-map continuation services remain in charge. Passage stays in
Events and is not turned into a town/world entrance.

check_ghido_bake.py proves exactly two added objects in this one submap, zero
removed/modified originals, and identical terrain, dimensions, transport data,
masters and all original arcs. The graph companion retains the trigger's native
ID/group/name/layer/footprint. The new point is metadata only. Actual game
geometry and state are untouched. Source/config before copies and generated
candidate/delta are in build_check/ghido_landing_20261029/.

## Acceptance and limits

npc_entry_check.cpp runs the actual planner on copied source actor snapshots,
including both NPC groups, both PointIns and the real source trigger. Its native
capture fixture collapses a graph companion and original Event to one actor.
Required acceptance: initial route reaches the real shore trigger, preserves
the original NPC destination, and a fresh snapshot after native movement
finishes at that same NPC. Source bodies are not embedded in the fixture.
After-data acceptance passes4→0, including destination retention, exact source
trigger and fresh post-movement snapshots. All-six tracking passes2454 master
roots/2523 identities; functional source/label checks pass980 exact occurrences
and12 locales. Both source NPC variants retain their separate story groups.
Canonical combined checks and applicable
delivery are later audit milestones; no FFV installation is claimed. Gameplay,
native transition timing and audible output remain unconfirmed.
