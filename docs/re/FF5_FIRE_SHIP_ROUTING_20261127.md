# FFV Fire-Powered Ship pathfinder

## Return routes after a platform carry

The later November 27 Steam log shows a platform operation followed by the
party's arrival at map 270, tile (12,21). Pathfinder then repeated “No path”
for the two chests on 261 and the chest on 256. The replay reproduced the
failures from that tile and from the three floors immediately behind it.
The same-floor chest on 270 still used its own platform route.

The return journey needs the lower current in the chamber on 268. Entering
that contact from its south side invokes the native script that moves the
controlled player north to (9,8), beside the route to the elevator. The
bake records the exact terminal contact and landing. It keeps the upper
contact and unrelated pushbacks out of walking routes. The lower contact
is usable as an explicit ride and ends that route segment. The planner
continues from the resulting native position.

The separate chest behind the one-switch door on 256 can still require
that switch when the lower exit on 270 is already reachable. Pathfinder proves
the route to the actual switch before offering the lower exit as the
first leg.

The final replay found routes for eight earlier ship chests from all 28
native layouts compatible with the logged landing. It separately checked
the return current's entry direction, the exact landing, and the onward
route to the elevator. The ship check passed 1,308 assertions across 492
route legs. The all-six browse check passed. The verified build was
installed in FFV. [Installation evidence](../SECURITY_AND_DISTRIBUTION.md#november-27-ffv-platform-chest-return-routes)

The gameplay follow-up is to select either earlier Engine Room chest
after the platform carries the party back down. Directions should lead
back through the previous room and then tell the player to ride the return
current where needed. After landing, request the next directions or use
automatic continuation.

## Initial routing fix

The November 27 Steam log records Storyline asking for the Liquid Flame
contact on map 271 from the save room on 269. It repeatedly returned “Story
gated.” Navigation also reported that the chest and controls on 270 and the
side-room chest on 259 were gated. Progress flag 44 was set and the Liquid
Flame completion flag 45 was clear. The ship floor with changing passages
uses separate local layout flags 301–307.

## Route ownership

- The native door on 269 leads to the passage controls on 270. On 270,
  Pathfinder selects a reachable switch and the exact side to use based on
  the seven native layout bits and the party's actual tile. It plans the
  approach against live collision and keeps the player's original
  destination. The approach stops before operating the switch. A new
  request uses the resulting layout. Storyline keeps the Liquid Flame
  objective through the approach and its native transitions.
- The offline graph now supplies different policies for the eight individual
  controls, the treasure chest, the return exit and the onward boss route.
  Across 200 qualified layouts and walking regions, every modeled target can
  be reached with its own sequence of native actions. The original Shift+G
  passage guide keeps its separate ownership and progress announcements.
- The side-room chest route offers the existing control on map 256 when its
  door is closed. The native control hides its collision actor; its script
  writes no scenario flag. The actual door state remains authoritative.
- Five elevators connect paired floors in both directions. The bake
  validates the departure and arrival script identities and preserves each
  native upper-layer approach and arrival. Directions stop at the control
  and tell the player to interact.
- Four pipes on 265 carry the party to real landing cells on 261. Those cells
  are route endpoints for the native transfer. The intermediate pipe is
  never traversed as walking geometry. The other source-qualified
  pushback triggers still block ordinary walking.
- The shared Storyline planner uses “No path to the next step” if it cannot
  supply a verified approach to the active objective. Ordinary destinations
  retain their independently verified story restrictions.

## Evidence and replay

The departure and arrival hashes are in the FFV arrival overrides. Exact
gate and tile positions come from the existing map bake and native script
review in [the Fire-Powered Ship passage study](FF5_FIRE_SHIP_20261105.md).
The original game inputs remain outside the repository.

The bounded policy replay covers 200 layouts and movement regions, 713
edges and 5,376 native actor comparisons. The production route check uses
the baked FFV data and the same planner as the directions keys. Its
additional cases cover the observed Storyline failure, the selected chest
and switch, an exact lift approach, and all four pipe arrivals. The browse
check separately preserves the behavior of true pushbacks.

## Local checkpoint

The ship fix passed the focused production build, real ship route and all-six
campaign and browse checks. The ship fixture passed 1,046 assertions across
233 actual route legs and 4,280 walking-cell observations. The isolated
load test resolved all 47 exports. Windows Defender reported no threats on
the 22 exact payload files. FFV was installed and all 22 hashes matched.
The pre-install and post-install comparison preserved 138 protected file
states and the other five games' payloads. The first installed DLL has
SHA-256 4c6319c2b5146b2425f4384010605fd0a1b4e263fac1c3d18e83fbf7d6231a01.
The scanner's exclusion list could not be inspected with the available
permissions. The isolated test and route replay do not establish in-game
movement timing or spoken playback.

The final shared candidate additionally replayed both directions of all
five elevators. Its route check passed 1,055 assertions across 242 legs.
[Final installation record](../SECURITY_AND_DISTRIBUTION.md#november-27-fire-powered-ship-and-manual-route-readout)

For gameplay confirmation, select Storyline at the Engine Room save point
and request directions. Follow each approach and operate the named control
when the directions give a facing. Continue requesting directions until
the Liquid Flame encounter. Before leaving, try the chest and an elevator
in both directions. Report the spoken directions if a control, doorway or
pipe leads somewhere unexpected.
