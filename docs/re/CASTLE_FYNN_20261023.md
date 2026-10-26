# Castle Fynn chest and passage investigation, October23

Current status: **complete**, implemented/offline verified and deployed to all
six closed games on October23. The user requested deployment, lifting the FFII
hold. Gameplay confirmation remains pending. No game launch/control or runtime
probe was performed. [Shared NPC contract](NPC_BLOCKERS.md).

## What the log establishes

Read-only snapshots of FFII session1103039828, part1 are retained in ignored
build_check/castle_fynn_20261023/latest.log and latest_2.log. At the time of that session, FFII still ran the released V0.9.5, rather than
the locally built Ship correction.

- At1104597.937 the selected chest is map42, cell(6,8), reached from 2F/map39
  through a 1F leg. At1104660.828, after entering 1F/map31, the planner still
  successfully routes to that exact chest, via another 2F entrance. The automatic
  continuation is spoken at1104660.859. This original route has not become No path.
- At1104662.328 and1104663.187 directions report No entity selected. The route
  continuation exists, but the navigation list has lost its selected entity.
  Cycling then selects the first and last chest entries. The first subsequent
  No path is map75, cell(9,15), not map42's selected chest. Further failures target
  map38's side room and basement maps33/34. Repeated generic Castle Fynn labels
  conceal the target change. The exact selection-restoration failure still needs
  an isolated reproduction before a repair; the log alone proves the loss and
  changed identity, not its internal timing cause.
- In the throne room/map43, the return exit to 3F is cell(26,29). A native Event
  named Passage, entity96/script1504, occupies that same cell as GotoMap entity84
  to map40/3F. At1104859.015 it is separately listed as Scene trigger. The exit
  also receives Event trigger on the route. This is an actual overlapping native
  event, not an invented coordinate. Whether its separate transition script has
  additional active effects must be established before suppressing it wholesale.
- At1104859.765 the passage exit at(42,7) is Story gated, with one live gate.
  The mod appends Reachable scene trigger: 1 South, pointing to the return exit.
  That advice does not establish any relation to the locked passage.

## Game mechanics and implementation evidence

The original map42 chest is in Map_20041_6, a side room off 3F/map40. The later
map75 target is deep in the basement chain. Map38 is a separate side room reached
from an isolated portion of 1F; the hidden passage connects the throne room via
maps44/45/32 to the east portion of 1F and the basement entrance. Distinguish
these destinations rather than treating the chest ordinal or bare floor label
as stable identity.

The throne room's Hidden passage actor22 is at(41,7), immediately before the exit
at(42,7). Its native properties make it solid and interactive, script87/sc_e_0063.
The script's successful interaction hides that actor and records scenario flag56;
the map initializer restores its opened state from that persistent flag. This is
a real progression interaction, so a correction must preserve the closed passage
and cannot turn every listed treasure into an immediately reachable destination.

Primary source locators are the matching extracted ff2_objects.jsonl,
ff2_scripts.jsonl and ff2_master/map.txt/script.txt under ../tools/mapdump/.
These source-bearing game records remain outside Git. Factual mapstate projection
is maintained in data/ff2/mapstate.json. The transition script1504 is named
sc_transition_20041_7 in the master but absent from the existing map-script dump
and the inspected map20041/script bundles. Its precise body remains unresolved;
do not infer an unconditional no-op from the name or overlapping position.

The former field/navigation_progress.h reachable_event_route flooded to the
nearest currently active reachable step-on event. The former
field/baked_nav.cpp plan_single_snapshot appended that route to a Story gated
result. Both that fallback and its unused helper have been removed. No dependency on the destination's gate is checked, so an
exit-side event can be suggested as though it advances progression. Keep the
real blocker; progression guidance must require an established causal connection.

Selection is separate: field/entity_filter.cpp restore_selection and
field/entity_scanner.cpp apply_filter/FilterPublish match pending destination
identity to rebuilt rows. A missing restored identity can temporarily leave no
selected row. entity_nav::announce_ready_entity now falls back to the stable
pending goal only in that gap, retaining map/cell/entity/raw identity and target
kind/name. New browse/category input still clears the pending destination. The
completion callback accepts a restored row only when its identity still matches;
map/category/readiness changes or a different selected target retire the reply.
No replacement chest is selected and no engine pointer crosses to a worker.
This covers the observed symptom without claiming the unlogged publication timing
has been reproduced in the live game.

## Repair design and implementation

Remote rows now use the same owned-snapshot planner as local browsing, supplying
the target's actual map/cell/entity/raw identity. Entity::format_description keeps
the floor and the planner's brief reason. route_to_goal tests the full graph with
only story gates removed even when the current floor has no gate of its own;
previously that local condition hid a gate several floors away. A positive first
leg requires reaching the real chest's interaction footprint on its final floor.
The separate map42 chest stays reachable with the throne passage closed.

All six native interpreters use SetPuppet/ExecPuppet to apply Hide/Show to the
specified actor. The map-state generator previously modeled these only for FFVI;
therefore the FFII initializer could not hide passage actor22 when flag56 was set.
Visibility is now shared, while absolute SetPos projection stays FFVI-only with
its established coordinate evidence. Unknown/branching or asynchronous effects
remain conservative. The interpreter never executes game scripts or writes saves.
Current-floor native instances still override off-floor projection.

The generic nearest-event suggestion is removed: spatial proximity is not proof
that an event unlocks a destination. Authored scenario guidance remains the owner
of established progression steps. Event96's missing transition body is still an
explicit evidence limit; this repair does not suppress the native event wholesale.

NPC geometry, manual interaction behavior, native collision evidence and checks
are documented in [NPC obstruction routing](NPC_BLOCKERS.md). Both features use
the existing capture budget, Navigation worker and map/request cancellation.
The live owner copies the minimum state; no worker keeps engine pointers, waits
on native processing, runs game scripts or changes collision/flags.

## Verification plan

Actual-source npc_gate_check uses real FFII bakes and initializer programs, with
post-Gottos flags50/51/52, both passage flag56 states and origins1F/2F/3F. It
checks the original map42 chest and gated map38/map75 targets independently,
including browsing and preserved gate reasons. The log's seven known soldier
positions plus the eighth actor's authored home reconstruct a successful route;
this does not recover every unlogged live collider or prove why the intermittent
failure occurred. Controlled solids/passable actors and moving obstructions test
the correction without claiming a runtime reproduction. Canonical build and all regressions pass, with7,206 native-name checks/zero
missing and125 used/333 defined actions/zero undefined. Isolated DLL smoke resolves
and forwards all47 exports and exits normally. The first sandbox build hit the
known MSBuild worker-startup stall and was interrupted; the first external build
found a missing fake map_handle test binding, corrected before the passing build.
No shipping native-link failure remained.

Actual-source checks cover a distant true NPC choke, adjacent zero-movement facing,
a manual autowalk boundary, passable colliders, moved actors, cross-floor destination
retention and a separate closed gate. The complete3-origin/2-state/3-chest matrix
and browse reasons pass. navigation_speech_check reproduces a missing selected row
with a valid pending goal, verifies continued directions, and rejects an intentionally
cleared pending destination. These are offline state/producer checks, not a replay
of the precise unlogged live publication timing.

All-six initializers retain the baseline known/conservative map counts in the
zero-flag corpus. Non-FFVI changed initializer counts are17/24/71/50/185; many rows
only retain slot ownership with no inferred navigation effect. All puppet slots
fit the existing0..31 bound. FFVI data is unchanged. All12 locale values for the
reused Interact/face/NPC/Story gated keys are present.

Tested DLL: 10647552 bytes, SHA256
3883266ace174e8b81fb0808260ec2b11cb3ae9caf7a7c21711fc107f101cd40.
All six installed DLLs match. All144 protected settings/waypoint/dependency files
retain hashes and timestamps. Previous DLL/SDL recovery, before/after receipts,
passed reports, baseline state counts and delta audit are retained under ignored
build_check/npc_fynn_20261023/. Prism was verified read-only. The local published
V0.9.5 ZIP still matches its prior SHA256346c943d6835852ed1de4dc25254fd2d8a9ec8642f728f9962ac10a1e51be6aa;
no public source/tag/asset was changed by this follow-up. No delegation or new
runtime evidence. [Short user checks](../TESTING.md#october23-castle-fynn-and-npc-follow-up).
