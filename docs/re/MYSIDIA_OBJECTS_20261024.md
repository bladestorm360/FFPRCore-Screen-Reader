# Mysidia, object categories and world-route consistency

User reports, October24: Mysidia's underground statue room was named after a
staged Masked Figure; manual inspections were grouped with walk-on Events;
the figure appeared only in Current map and offered an unusable interaction;
Tropical Island disappeared near Mysidia although it had appeared near Snow
Cavern; Ship browsing announced its eventual boarding heading before a long
walking approach. The accidentally pasted fourth screenshot is excluded.

## Design and status

| Item | Implementation | Offline evidence | Gameplay |
|---|---|---|---|
| Exact map98 Statue Room title in12 languages | Complete | Exact stairs/statue matched; authored title and12 locales checked | Pending |
| Objects category from native activation semantics | Complete | Actual manual/automatic class and remote category regressions added | Pending |
| Masked Figure action/filter consistency | Complete | Native dispatch and all-six action tables verified; live/baked checks corrected | Pending |
| Tropical Island and unlimited map-distance search | Complete | User confirms vehicles filter; same-state real-map route regression added | Vehicles filter confirmed by user |
| Ship approach versus boarding list instruction | Complete | Actual approach distance precedes eventual boarding action | Pending |

## Scope and ownership

Existing native observations remain bounded on their owning thread. Filtering,
full-map search, category preparation and text processing use owned snapshots
on the Navigation worker. No new game launch, control or runtime probe is
authorized. User gameplay logs and extracted assets are read offline; native
function text is stored outside Git in ../tools/mysidia_objects_20261024/.
Recovery, copied user logs and verification receipts belong to ignored
build_check/mysidia_objects_20261024/.

Category Objects is appended as identity7, preserving all existing category
identities. Internal Kind::Event remains shared by script/consumption tracking.
FieldMapObjectDefault/IInteractiveEntity manual activations and baked Entity
rows belong to Objects; EventTriggerEntity and automatic script triggers stay
Events. Script contents and names such as "trigger" do not decide activation.
Existing visibility, native CanAction, story-state and placeholder guards remain.

Exact FFII map98/Map_20101_8 is reached from the central Mysidia outdoor stairs.
Its Goddess Statue and underground stone layout support Statue Room. This
authored exception supersedes owner-name inference for this map alone. Shared
map_names::override_title resolves it for the room and its entrances; geometry
does not change. See ../MAP_LABELS.md.

## Activation and native dispatch evidence

EntityTriggerEntity/EventTriggerEntity automatic triggers and manually examined
FieldMapObjectDefault/IInteractiveEntity objects both invoke the game's script
system. The script is not an activation discriminator: extracted placements
reuse scripts in both modes (FFIII1, FFIV10, FFV1, FFVI1). FFI/II have no such
reuse in these extracted placements. The category decision lives in
navigable_entity.cpp Entity::category; entity_nav.cpp category_name localizes
the heading. entity_filter.cpp creates all eight prepared views without another
scan. entity_scanner.cpp interactive_detector and build_remote already preserve
activation in event_type. navigation_labels.cpp groups only automatic Events;
script consumption stays keyed to Kind::Event/entity identity. Audio's
route_kind_of_category keeps Objects on Infer so each object's examined or
travel semantics remain authoritative. Native manual interactions remain manual.

All six master action tables map2 to RunScript,4 to Talk,8 to Shop. FFII native
EventAction.CraeteTask RVA26B910 dispatches by ActionName;
EventActionScript.<EventRunScript>d__9.MoveNext RVA295020 loads PropertyEvent's
ScriptId at0x80, looks up that script and skips starting it when lookup fails.
The script table starts at1 and contains no0 row. FieldNonPlayer.CanAction
classifies action kind rather than proving that script assignment exists.
The earlier shared CanAction/body evidence is in NPC_BLOCKERS.md.

Map98 has two Masked Figure NPC placements: ev_e_0065 entity5 at(-2,6) Bottom,
and ev_e_0068 entity3 at(14,7) Ground. Both have RunScript action2, ScriptId0
and no MessageKey. They are staged actors rather than working talk actions.
The live predicate npc_navigation_available now rejects that empty assignment,
shared by discovery and selected-target lifetime checks. Reassignment to a real
script restores availability. Message-backed actors, service actions and
scripted action9 contact encounters retain their existing rules. This is not a
room/name-specific exclusion or an attempt to change the game interaction.
An interaction goal alone cannot demonstrate an executable action.

tools/bake_maps.py mirrors this known RunScript condition in navigation-action
flag8 for NPC/ShopNPC only. Collision, movement and identity are unchanged;
staged actors still obstruct movement when native collision says so.
connected_objects_snapshot already requires action/script flags for remote
NPC discovery. All six bakes were regenerated and collision round trips pass.
Byte comparison against the prior bakes proves only navigation-action bits
changed: FFI4, II70, III25, IV373, V49, VI122. Counts include baked aliases;
raw placement counts are4/70/24/369/49/122. No game code is in these records.

## World filtering and Ship speech

The user later confirmed Tropical Island is present in Current map with
vehicles and requested leaving that behavior. Preserve the modes: on the
single world floor, Current map and Current floor overlap; the vehicle mode
also permits reachable owned parked transport/landing legs. Current map's
world exit-distance filter does not silently include the parked Ship.

baked_router::find defaults max_expansions to0 (whole map); no production
caller assigns a finite expansion count. flood enumerates the finite native
grid without a radius/distance deadline. kLongRouteSpeechThreshold31 and
kMaxSpokenLegs6 limit the short spoken summary only. They never trim the
returned cells, full directions or destination inclusion. Reach's16-bit
distance representation saturates at65534 while still visiting later cells;
it is not a reachability cutoff. The400-state cross-floor loop guard is a
separate bounded graph/loop defense, not a tile distance or world-map radius.
Exhaustion remains incomplete evidence, not proof of a block. No search or
filter limit changed in this task.

plan_browse_snapshot previously copied the first vehicle action from a later
phase, e.g. board the Ship East after a long walk. It now sums preceding
walking-phase cells and says Walk N steps, then that action. Adjacent boarding
retains its immediate heading. Explicit directions, boarding edges and native
movement are unchanged. The36 values for Objects, Statue Room and approach
speech cover all12 supported languages and retain formatting placeholders.

## Verification and safe extension

tools/oct24_navigation_check.py audits actual all-six action tables, activation
placements and new localized keys. tools/navigation_followup_check.cpp uses
the production snapshot planner/filter with FFII's real world terrain,
Tropical Island at(120,231), identical owned Canoe/Ship/Snowcraft state and origins near
Mysidia(40,168) and Snow Cavern(177,14). It separately checks the logged Ship
origin(11,163), target(189,44), both real staged NPC placements and remote
discovery. The live obstacles/story flags of that session are not reproduced;
these are owned offline fixtures, not gameplay confirmation.

tools/entity_scanner_check.cpp distinguishes actual manual/automatic native
classes with the same name/script and covers empty/restored scripts, shops,
message-backed actors and contact encounters. equipment_party_check.cpp
exercises the actual Entity category method for current/remote objects;
entity_filter_check.cpp verifies separate Objects/Events views and selection.
Existing all-six route, NPC, autowalk and cancellation checks remain required.
Performance numbers must distinguish owned computation from native capture
and physical speech; final build/run receipts will record measured limits.

Future object additions must use native activation, preserving special
MapExit/SavePoint/Vehicle identities. Future room corrections should be exact
authored context exceptions with evidence, rather than expanding owner
inference. Do not remove collision when excluding a staged NPC's interaction.


## October24 delivery and limits

Canonical build and every required offline check pass; isolated smoke resolves
and forwards all47 exports. Native-name audit:7,373 checks, zero missing; action
wiring:131 used/339 defined, zero undefined. All49 embedded data and18 sound
resources match current sources. DLL10793984 bytes, SHA256
 3c8562206a2fe22ca9540a4349cff82ec04c68ee645e34db8f7e5f8df73b5b82.
Installed and hash-verified all six after the user closed FFII. All144 protected
files match the fresh applicable deployment baseline. Recovery, reports,
source/resource hashes and receipts: ignored build_check/mysidia_objects_20261024/.
No game launch/control/probe, dependency install, delegation or release/push.
Gameplay/listening confirmation remains separate and pending; see TESTING.


## Logged B3 chest and confirmed offline routes

The latest user trace retains Unopened Treasure Chest (Tropical Island B3) at
1161607.625 and after battle at1161627.562. Its current leg says3 South then
exit to B4; browse says travel through B4 first. This is an intermediate floor,
not evidence of selecting a B4 chest. Production planner fixtures identify the
actual B3/map202 chest entity20 at(26,16), from(18,13). Before and after a
battle-style fresh plan, the final map/cell/entity remain202/(26,16)/20 and the
next map is203 (B4). Native map traversal/gameplay remains user-side.

Real-world vehicle fixtures retain Tropical Island395 steps from Mysidia and380
from Snow Cavern, with the same owned Canoe/Ship/Snowcraft. Their paired
filter/full-plan host cost was about47-51ms in one run, excluding native capture,
frame scheduling and speech. Walking-only mode rejects the sea crossing; the
vehicle mode retains it. Logged Ship has220 approach steps plus1 boarding step.
No arbitrary world-distance restriction was added.
