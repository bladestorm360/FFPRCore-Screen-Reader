# Zone Eater bridges, recovery and stepping stones

This is one remaining FFVI map/story audit slice, separate from the Closed
ceiling guide. Source/focused qualification is Closed: model, production routes,
source, localization and native-ready controller checks pass. Combined native
output, installation and gameplay remain pending.
The ten previously accepted guide records and all prior catalog values are
preserved in build_check/zone_eater_bridges_20261103/before_*.json.

## User journey and ownership

Shift+G uses the existing FFVI story owner, input command, native readiness
continuation and Navigation worker. No new hook, polling loop, input injection,
native object lifetime or timer is introduced. Two new data recipes cover the
entrance/bridges/recovery/corridor and the later corridor/stepping stones/Gogo.
The independently Closed roof guide remains between them, with its own
preview/readiness/native-cycle timing contract unchanged.

On the bridges the guide selects the real forward exit. The planner stops at
each reviewed manual jump, gives its approach and interaction, and replans from
the actual landing when requested again. Moving guards keep their copied live
positions and collision. They may require waiting or a fresh route; a guard
collision is not a deliberate shortcut. If the player falls, guidance selects
the actual recovery switch, then the return exit only after the native local
receipt. The remaining corridor leads into the roof's real first safe area.

After the roof, guidance uses the next corridor, then the stepping-stone switch
and the final exit. Native jump programs include decorative chest animations
and temporary layers; the existing reviewed travel edges preserve each final
Ground landing. The guide never labels those animated platforms as collected
treasure. On the last floor it selects Gogo's current physical NPC. Completion
requires the native joining receipt and normal field readiness after naming
and conversation, rather than arrival beside the NPC.

## Primary facts

All game implementations remain outside Git in ../tools/mapdump. Normalized
actor/family facts and original probe tools are in the checkpoint above.
tools/author_zone_eater_crossings.py pins eight exact state-script fingerprints
and verifies all65 previously reviewed direction-qualified manual arrival rows
against their external script records. The author appends only these two guides;
the canonical build checks the exact recipes rather than regenerating older ones.

| Room | Native fact | Guide consequence |
| --- | --- | --- |
|500, Map_30791_1 | Entrance corridor; next actual arrival501 Point1 | Begin the bridge journey |
|501, Map_30791_2 | Forward exit entity6 at Ground7,13; three contact guards run0833 | Stop for manual jumps; preserve guard collision |
|502, Map_30791_3 |0833 falls here at Point2;0849 Event8 at Ground19,12 hides six barriers and sets local1 to1 | Operate the floor switch before returning through entity2 |
|503, Map_30791_4 | Automatic roof entry504 Point1 continues0850_0 | Reach the Closed roof guide through the real boundary |
|505, Map_30791_6 | Ordinary onward corridor to506 Point1 | Continue after the roof |
|506, Map_30791_7 |0857 Event25 at Ground17,14 sets scenario409, raises footings and hides barriers28/29 | Request each jump; use the switch before selecting exit entity2 |
|507, Map_30791_8 |0865 NPC1 at Ground10,8 opens naming/joining; scenario410 hides it after joining | Talk to the NPC; finish only on the actual receipt and readiness |

Native point IDs are object IDs, not entity numbers. The bridge has three real
arrivals: Point1 Ground46,5, Point2 Ground45,11 and Point3 Ground7,12.
Recovery Point2 is Ground10,12, while its normal Point1 is Ground25,9.
Stepping-room Point1 is Ground20,11 and Point2 Ground10,7. Final Point1 is
Ground10,15. Source-derived coordinates are implementation evidence, never a
requirement for the blind player to report undisplayed coordinates.

The current-floor planner must include the copied physical obstacle list.
An initial intentionally incomplete no-gate diagnostic appeared to permit
walking through the recovery barrier. The corrected primary-state fixture
proves that the same exit is story gated before local1, and reachable after it.
No production geometry or router policy was relaxed. The initial fixture
member-name compile error and both diagnostic outputs are preserved separately.

## Verification and limits

The source recipes preserve ten accepted guides and all existing localization
values. Four new instructions have all twelve supported languages and preserved
placeholders. Actual recipe/production-planner replays pass24 manual legs and11
complete objectives, including both stepping-room arrivals, all three bridge
arrivals, a populated guard case, fall recovery, the last NPC and three ordinary
floor boundaries. All manual legs have real non-trap landing receipts and stop
for native interaction. No route deliberately falls. The recovery exit fails
before its local switch and succeeds afterward. The actual model passes all13
new state/unknown/completion cases;196 physical references resolve with no
failures, retaining the prior173 train/other state plans. The actual controller
waits when scenario410 is set during naming/dialogue and finishes only when
normal field control returns. No new native observation is needed.

The source/route/model resource receipt is66.178s/443686912B/exit0/drained;
source/localization/semantic/controller receipt23.274s/349323264B/exit0/drained.
Eight state-script hashes and65 native directional arrival rows pass. Three
original discovery candidates now have explicit guided decisions. Coverage is
432 logical/464 physical original-inventory records,350V+871VI=1221 pending.
The bridge contact/manual-jump and animated-platform supplement is qualified
separately, not represented as invented original-inventory candidates.
Exact source/report hashes and preservation witnesses are in
build_check/zone_eater_bridges_20261103/completion.json. This design is Closed;
new internal review does not authorize further refactoring.

All build/tool trees use shared resource admission,2GB aggregate memory,25%
machine CPU, below-normal priority and one compiler/worker. Offline fixture
results do not establish real moving-guard response or audible/frame latency.
After the final combined audit delivery, natural acceptance is: request Shift+G
at the entrance, repeat after each native jump, recover with the floor switch
if knocked off, complete the separately guided roof, then continue through the
stepping stones and finish Gogo's dialogue. No game/probe was launched here.
