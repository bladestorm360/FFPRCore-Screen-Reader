# Island Shrine vent routes

The two native lever bits select one of four vents. The original route graph
stopped on map 428 during the vent animation. The game continues from that
map to a playable branch and walks the character to the final tile. All four
outbound vents and all four returns now use the corresponding final
destination and tile. Map 428's separate stairs and routes remain available.
The native vent groups, appearance and travel semantics are preserved.

| Lever bits 167, 168 | Playable map | Final ground cell |
| --- | --- | --- |
| 0, 0 | 429 | 10, 10 |
| 1, 0 | 433 | 11, 12 |
| 0, 1 | 432 | 6, 12 |
| 1, 1 | 431 | 7, 11 |

Each return finishes at ground cell 7, 7 on map 430. These eight derived
edges are the only route changes. No hooks or game-state reads were added.
The entry and tablet-room doors are ordinary route interactions with their
original receipts. The tablet begins the normal boss encounter. The vent
levers remain available through normal object navigation. No new Shift+G
guide was added for them.

The worker fixture reproduced all eight failures before the change and
passed all eight legs with 16 assertions afterward. The bake comparison
verified that all 655 rooms, terrain grids and unrelated routes stayed the
same. The source guard matches 29 complete program fingerprints, native
entry points and the two controlled puppet poses. See
[source proof](ff5_island_vents_native_20261105.json) and
[fixture](../../tools/ff5_island_vents_route_check.cpp).
