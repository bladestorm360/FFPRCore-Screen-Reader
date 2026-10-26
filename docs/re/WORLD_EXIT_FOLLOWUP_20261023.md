# Scripted world exits, Ship identity and navigation response

October24 [Leviathan sea-scene entry](LEVIATHAN_20261024.md) permits its reviewed
Ship entry without a fictional landing; ordinary native town rules remain.

October23 status: **complete; offline verified and deployed to all six**.
The [battle follow-up](BATTLE_FEEDBACK_20261023.md) covers MP drain, Swap and Fog's
Amnesia. All reports remain part of the combined deployment. Deployment is authorized
for closed games; no game launch, input/control or runtime probe is authorized.

## Established causes and correction design

The current FFII log shows Fynn map20 southern cells(19..21,38) listed as Scene
trigger in Events, rather than Exit to World Map. The shared event-label fallback
labels a script-backed step trigger as a scene unless another supported identity
wins. These exits are native Event actors whose scripts change maps. The native
GotoMap detector still works; the missing case is script travel, not a requirement
to replace all games with FFVI's entity model. Matching per-game script metadata
now marks ordinary world exits separately from scenes, mixed-destination warps,
round-trip cutscenes and environmental traps.

Warp's low byte retains its activation kind; bit16 marks ordinary world travel.
Every marked script has actual ChangeMap targets on world assets plus a direct
travel script identity or a narrow world-exit identifier/travel command set. Raw
names alone cannot create a destination. The existing script graph/arrival data
continues to own physical travel. Native and remote scanner consumers classify
these as MapExit, using localized Exit to World Map, and preserve native entity/
raw/footprint identity. Grids prepares a per-native-ID index on Data; native
conversion only looks up ready facts, without decoding, parsing or a map scan.
Ordinary GotoMap exits remain their existing detector's responsibility.

Real exits terminate travel. Current-floor captured world exits use terminal
cells in the existing trap-exclusion channel, separately from trap identification
and speech; selected exit goals may enter them, ordinary paths cannot walk through
and silently leave the map. Future-floor active projection uses the same rule.
They receive no story-event/trap warning merely for being scripted exits.

The Ship log now supplies decisive evidence: target(189,44), layer-1, Spot, no goal
cells while on foot. A native ship can also be an NPC; selected_live_npc copied its
all-layer collision footprint into a vehicle destination. Earlier loop-coordinate
checks lacked that native NPC representation, so they did not catch this cause.
Spot/Transition targets no longer bind as talk NPCs. Vehicle goals carry their
transport model ID and bind to the same captured model tile used by distant
boarding routes, independently of the rendered sprite origin. Known parked boats
can supply a water goal closed to walking; the normal planner must still prove a
legal shore/boarding edge. Missing models never become arbitrary spot goals.
Directions, browsing, autowalk and beacon handoff preserve that identity.

The log's slow categories include Paul's gated hidden passage and treasure:
roughly400..600ms request-to-submit, with native scan/capture around0.2..0.4ms.
Turning StoryGateFilter on removes those targets and avoids their failing browse
searches. The NPC collision getter is a bounded owner-thread read; pathfinding and
formatting stay on Navigation. The graph search, however, could leave an indoor
floor group, enter the world and flood distant maps while proving a local gate.
Indoor-to-indoor searches now stay on indoor floors, consistent with remote object
discovery and story-gate filtering. Explicit world destinations remain supported.
CrossLeg records visited state counts for offline work evidence. Native, queue,
request-to-submit and audible timing remain distinct; no runtime speedup is claimed
from worker compute alone.

## Sources, ownership and verification

field/baked_map owns prepared identity indexes; bake_maps.py authors the flag from
matching external script/master data. field/entity_scanner's goto_map_detector and
RemoteBuilder/build_remote own visible category/name production. field/baked_nav
owns captured vehicle binding, goals_for, live_blockers, find_across and partial
route/manual handback. MapGoal, ActiveRoute and the beacon selected Target retain
transport identity and original observed tile without moving native pointers to
workers. Native capture retains the elapsed CaptureBudget, ready-state/lifetime
checks and map/input/battle cancellation. No native hook waits for worker output.

navigation_followup_check uses production snapshot planning and all six real
bakes, with water-only boat goals, native NPC collision layer, offset sprite/model
coordinates, missing models and the logged Fynn/Paul gate cases. It reports visited
indoor states and offline compute, without a universal performance threshold.
entity_scanner_check exercises actual live and remote category/name producers.
Existing route worker, NPC handback, story filter, scene grouping, whole-map bake,
beacon, autowalk, native-name and isolated DLL checks remain required. English and
all11 other mod locales share the new World Map key; game destination names retain
their existing native resolution. Detailed reports and retained logs are under
ignored build_check/world_exit_labels_20261023/. Luna supplied seven battle catalog keys; Sol review and corrections are recorded in the battle follow-up.

## Final verification and deployment

Canonical build and all offline checks pass:7,301 native name checks/zero missing,
125 used/333 defined actions/zero undefined and47-export isolated smoke.
All six installed DLLs match:10687488 bytes, SHA256
7dc3d8623711796ea965b219ae278392f517dfe32336042f38a7f6cd52ca8e70.
All144 protected settings/waypoint/dependency files are unchanged. Every one of
43 embedded data resources matches its source. Recovery copies, complete
reports and delivery/deployment receipts are retained in ignored
build_check/world_exit_labels_20261023/. No game was launched or probed.
Gameplay confirmation remains pending. Published V0.9.5 source/tag/ZIP were not
modified; its retained public ZIP still matches the verified original hash.

The final FFII gated Paul fixtures visit11 indoor floor states each, with paired
route computation23.681/16.250ms. These offline figures exclude native capture and
speech and are not a before/after gameplay latency measurement. Marker counts are
FFI0/II79/III3/IV3/V165/VI7. FFIV has no applicable Ship/shore pair for this new
fixture; its existing native transportation checks remain. All2,454 master roots
retain their correctly classified remote events/exits across state projections.
