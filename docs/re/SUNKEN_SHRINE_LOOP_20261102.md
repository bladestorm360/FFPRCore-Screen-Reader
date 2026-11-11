# Native looping rooms — Sunken Shrine5F

Source/focused checkpoint **Closed**; not installed. The user's actual FF1 log
809340–809347 identifies map103/Map_30081_8, start19,29 Bottom, and the three
chests at38/39/40,11 (entity40/41/42). Browsing reports Not directly reachable
and directions No path, with13 chest blockers, no gates and10 moving NPCs.
No tester logs, game control or runtime probe was used.

## Physical cause and boundaries

The native map master gives this floor horizontal LoopType1. Native enum facts
are None0, Horizontal1, Vertical2 and All3. The existing bake is54 by51 and
already contains the Upper-layer opening on both horizontal edges at row6.
Only world maps previously wrapped in the router, cutting that connection.
All three failures reproduce through the actual planner/browse worker before
the change; the matching layer/collision/treasure geometry was already present.

The maintained bake writer now retains native loop axes in the existing flag
byte (bit0 remains world; bits1/2 are horizontal/vertical). Shared map aliases
must agree. Existing world bytes/behavior remain unchanged. The metadata-only
author changes only the reviewed submap flag bytes:5/8/0/5/23/15 by game,
including unused native scene/placeholder sheets. Every other byte, object,
collision, passage, vehicle and story datum is identical. The ordinary full
FF1 bake reproduces the updated binary and retains the confirmed Ice Cavern
physical drop/arrival proof. Native Flying Fortress4F also correctly retains
its two wrap axes; some FFVI scene/room sheets have one or both axes.

## Implementation ownership

baked_map decodes the immutable axes; wrap_width/height centralize the per-axis
rule and retain old world-bake compatibility. Indoor loops remain collision
maps with their native layers/stairs. baked_router uses these dimensions for
successors, directional barriers, edge validation and its admissible distance
estimate. Flood/browse/search share the same physical graph. Walls, captured
blockers, traps, terminal map changes and story projection keep their vetoes.

baked_nav normalizes copied coordinates only on actual loop axes, formats
directions with the same dimensions and publishes the axes alongside physical
map dimensions. The native follower preserves indoor stair diagonals while
normalizing each wrapped axis independently; world movement remains cardinal.
Its native collision query must still receive one requested tile displacement
at the seam. The actual native-owner adapter now receives a single16-pixel
requested step in both directions and retains the native veto. Its two former
long-displacement queries were reproduced separately before the local fix.

Both beacon corner paths project the next leg in the true direction through
the seam, including native indoor diagonals. Direct beacon aiming, soundscape
capture and map-marker distances share the same scalar wrap dimensions.
These are value-only geometry changes on existing owners; no new hooks,
polling, native pointer transfer, queue, I/O or global cache is added.
Spatial hooks add only bounded scalar dimension tests. Offline timing does
not establish native-frame or audible response.

## Evidence and remaining qualification

build_check/sunken_shrine_20261102 retains original actual-log/factual readers,
the three-failure baseline, metadata author proof and first successful actual
three-chest planner/browse/follower/beacon replay. Both seam directions are one
legal Upper-layer step; copied blockers still veto its destination and horizontal
wrapping cannot invent a vertical edge. All planning remains worker-native-free.

The first broader replay found three obsolete world-only expected-delta checks
in its fixture (Flying Fortress4F and two FFVI sheets). The production follower
already returned the normalized per-axis movement; the fixture now checks those
native axes. Its failed report is preserved separately, not counted as a pass.
All seven affected checks pass, including all-six router/follower replays,
native autowalk, worker ownership, retained nine Ship profiles, map panels and
real audio mixer/output. Integration269.539s/498778112B; final native producer
and retained worker125.386s/497926144B, exit0/drained under aggregate2GB/25%CPU/
one worker. Canonical metadata and three-chest checks are wired in build.bat.
Exact source/report/resource identities: sunken_shrine_20261102/completion.json.
The first20–35min source estimate slightly overran while qualifying the native
handoff; no unrelated architecture branch was added.
FF1 delivery Closed21:57UTC. The724-input isolated candidate has169 compiler
targets/178 fresh reports,8326 names/0missing,144 used/350 definitions/0undefined,
47 exports/67 exact embedded resources/12languages. Canonical resume681.412s;
whole qualification710.831s/801751040B,2GB/25%CPU/exit0/drained. The prior2400s
compiler timeout remains preserved as incomplete evidence, not a pass. Defender
scanned22 exact files with no threats/stable hashes; exclusions unverified,
no signing certificate. Guarded install12.235s/115920896B,exit0/drained:
all22 installed files match;144 protected files and other five games unchanged.
DLL11625984B SHA b62ed342de0f689673ce7a5a629e588e994477fa1aacde48f13ca3529f180a96.
No game was launched/controlled/probed. Natural passage/game responsiveness
confirmation remains distinct. Exact final completion is copied to this
checkpoint's delivery_completion.json; original isolated artifacts remain.
Broader puzzle audit stays behind the custom-sound source/output checkpoint.
