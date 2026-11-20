# Remaining map-audit findings and evidence limits

## November 19 gameplay fixes and accessibility follow-up

The owner authorized implementation and local deployment of the documented
gameplay fixes, followed by the all-six descriptive audit and Storyline work.
Karnak world-map progression is the first delivery. The completion records
below track implementation, offline evidence, installed verification, and
player confirmation. Preserve earlier closed work and reconcile outstanding
reports in the existing feature and maintenance records as each system opens.

| Checkpoint | State and acceptance |
| --- | --- |
| [FFV Karnak world map](#november-17-ffv-karnak-world-map-routing-progress-blocker) | Closed for source, offline verification and FFV installation. Player confirmation pending. |
| FFV routing and labels | Closed for source, offline verification and all-six local installation. Player confirmation pending for Karnak Pub 2F, both save-point cases, [Walse jumps](TRAPS.md#november-17-ffv-tower-of-walse-jumps-labeled-trap), and [shop names](OBJECT_IDENTITIES.md#november-17-ffv-shop-labels-expose-the-temp-suffix). |
| Field menu speech | Closed for source, offline verification and all-six local installation. Player confirmation pending for the [FFV target producer](NPC_PATROL_TARGETS_20261025.md#november-17-ffv-duplicate-spell-target-on-menu-entry) and the six [field pause menus](MENU_DETAILS.md#november-17-ffv-field-pause-menu-opens-silently). |
| FFIV world and floor routing | Closed for source, offline verification and FFIV installation. Player confirmation pending for Moon entrances, Sealed Cavern rope prerequisites, [airborne selection](VEHICLE_ROUTING.md#november-15-ffiv-airborne-vehicle-interaction-loses-selection), [duplicate markers](OBJECT_IDENTITIES.md#november-15-ffiv-duplicate-navigation-markers), and [hovercraft journeys](VEHICLE_ROUTING.md#november-15-ffiv-falcon-and-hovercraft-routes). |
| [Visual descriptions](#november-20-six-game-completion-goal) | In progress. The frozen artwork review covers 1,380 visible enemy sprites, 762 field candidates, 233 battle art and portrait variants and 371 distinct representative equipment appearances. The broader runtime catalog, status effects, scene review and delivery are open. |
| [Storyline mode](#november-20-six-game-completion-goal) | In progress. All six itineraries extend to their final encounters and have focused offline replays. The campaign text is localized. Integrated travel verification and delivery remain open. |

## November 17 FFV Karnak world-map routing progress blocker

**Highest-priority gameplay fix.** Fix and deploy before the other pending
fixes and the visual descriptions and Storyline work. The owner reports
that many destinations near Karnak are marked unreachable despite being
accessible, blocking guided story progression.

The current FFV Steam log records the party on world map 1 at (84,78),
layer 1. At 434600.234 and 434600.859 the planner finds no goal cells for
Karnak Castle at (84,77) and Fire-Powered Ship Above Deck at (84,79).
Both are announced as “Not directly reachable.” The log then records
native entry into Karnak Castle, map 95, at 434625.968. Some other exits
still get routes from (84,78), including the town of Karnak at 434619.156.

Verify the native approaches and progression conditions for the affected
world-map exits. Browsing and directions should recognize usable contacts
from the logged position and guide the party into Karnak Castle. Check the
ship contact and other nearby exits against their actual story and vehicle
requirements. Retain legitimate restrictions.

The installed FFV misc bundle matches the Walse reference hash in
[map exit evidence](MAP_EXITS.md#ffv-walse-terrain-november-10). Its world
1 terrain table converts raw ID 62 at the ship contact from 20 to water
16 at scenario flag 989. It converts raw ID 48 across the castle footprint
from 20 to 6 at flag 992. Both conversion windows end at flag 1023. The
existing FFV terrain resolver now applies those rules from the copied
scenario state before goal discovery and movement checks. The change adds
no native hook and retains transition groups and transportation requirements.
Other dynamic terrain entries remain outside this focused fix.

The new cases in `ff5_story_contacts_route_check` reproduced both failures
on the previous source. The same test covers the two one-step approaches,
the ship contact after it becomes water, and unknown ship progression.
The corrected build passed 38 assertions across 27 route legs and both
shared browse and route-worker checks. The 67 embedded resources and
47 forwarded exports verified. FFV's 22 installed files matched the exact
scanned payload, and 138 protected file hashes across the six installations
matched the initial inventory. This closes focused delivery. Game-frame and
audible response timing remain unmeasured. [Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-ffv-karnak-world-routing).
Remote `master` was verified at `f7d0043820bd2f45dbb713cd45f753d861f18d15`.

## November 17 FFV Karnak Pub 2F exit classified as an event

**Priority gameplay fix.** Fix and deploy before the
[visual descriptions and Storyline follow-up](#november-17-visual-descriptions-and-storyline-follow-up).

The current FFV Steam log lists “Scene trigger (Karnak Pub)” under Events
at 432833.062. The selected marker routes to (22,4) on Karnak Pub map 92
at 432846.390. Following it enters Karnak Pub 2F, map 93, at 432857.218.
The party returns to the pub at 432867.265. The owner suspects the contact
is stairs. The log confirms the floor transition, and the exact native
contact type remains to be verified.

Verify the transition and list the 2F entrance under Map Exits with its
destination name. Check that directions and the return from 2F follow the
native approaches and that independent scenes keep their Event classification.

Both placements of the same native contact, entity 33, run script 146.
Every branch enters map 93 at PointIn 1. Two branches play a scene after
that arrival. The bake promotes the exact two existing warp records at
(18,4) and (22,4) to named Map Exits and preserves the native story scripts.
The staged FFV bake differs by only the two promotion flags. The focused
replay verifies both exact map-exit entries and the one-step approach
from the native return tile. [Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-shared-save-point-and-ffv-navigation-fixes).

## November 17 FFV directions while standing on a save point

**Deferred.** On Tower of Walse 5F, the FFV Steam log
`ffpr_screen_reader.1.log` lists the selected Save Point at 0.0 steps at
429972.734. A directions request says “1 North” at 429974.406. At
429996.000 a fresh plan records one step with both the player and goal at
(7,8), layer 1, on map 238. It again says “1 North” at 429996.015.

Save Point selection now uses a spot goal so its own tile is the arrival.
The existing zero-movement response says “Already here” in English with
equivalent speech in the other 11 languages. The focused replay verifies
zero steps on the circle and two steps from the original nearby position.

## November 17 save points are passable during routing

**Deferred.** Treat ordinary save-point tiles as passable on a route to
another destination. Reserve “Event required” for an actual scene, a
scripted stop that takes movement control away, or an event that transports
the party. The save circle's normal shine animation leaves the party free
to keep walking. The first visit can show a brief tutorial pop-up.

On Tower of Walse 5F, the FFV Steam log at 429840.421 says “Event required”
before directions from (7,10) to the selected chest at (10,5) on map 238.
That route crosses the save point at (7,8). At 429844.765 the native
consecrated-circle tutorial is spoken.

The route worker omits the reviewed IV–VI tutorial contacts from hazards
only when they share the native save point's tile and layer. The existing
native readiness gate governs any pause from the pop-up. The original
chest route replay crosses the save point without an “Event required” warning.
The Save Points destination and independent scene and relocation triggers
keep their roles.

<a id="november-15-ffiv-moon-route-to-crystal-palace"></a>

## November 17 FFIV Moon entrance routing

**Deferred.** The owner reports that none of the attempted Moon entrances
work at the current story point. Western Lunar Path, previously suggested
as the Crystal Palace approach, also fails after approaching on foot and
after flying and landing nearby. The owner has seen “Lunar Cave” in guides
and is unsure whether the selected Western Lunar Path contact is the intended
entrance. Keep the existing in-game map names.

- **November 15, Crystal Palace.** The FFIV Steam log announces “Fly 50.0
  steps, then land” at 268335.031. It routes to Moon map 3 at (22,37) and
  later says one step south from (22,36) at 268459.437. The next scan still
  shows Moon map 3. The contact's direction and story availability remain
  unverified.
- **November 17, Western Lunar Path.** The FFIV Steam log routes on foot to
  Moon map 3 at (12,32) at 428112.890 and repeatedly gives the same last
  steps from (13,34). At 428161.843 it offers a flight and landing route to
  that same contact. After landing at (12,31), requests continue to say one
  step south through 428196.531, and the party remains on Moon map 3.

The installed Moon bundle contains a 64 by 64 `GroundSubCollision` layer
that the bake omitted. All seven native `GotoMap` contacts use collision ID 2,
which closes the north face. The raw transportation grid alone permits the
step south reported in the log. The previous browse replay gave that forbidden
step at all seven entrances. The Moon bake now supplies the same directional
ground plane that the Underworld uses. The round-trip bake comparison found
only the added 1,236 bytes of encoded Moon collision and relocated offsets.
Native contact destinations, approach names and scene conditions use their
existing data. The production replay checks walking from both sides and the
Lunar Whale's landing and final walking leg at every entrance.

An independent flood over the native ground, flight and landing tables found
direct landings for the lower Western Lunar Path contact, Hummingway Home
and Cave of Bahamut. The other four entrance approaches have no landing on
their connected plateaus. The shortest legal continuation uses the lower
Western Lunar Path and, where required, the Eastern Lunar Path. The
cross-floor planner now checks directional faces when proving a world goal
from an adjacent cell, so a reachable tile north of the door cannot end the
cave search before it reaches the usable side.

All seven walking approaches, three direct flight-and-walk routes and four
cave detours passed the production replay. The detours retain the selected
entrance. The updated exact-entry checks use adjacent cells with a legal
edge, and the dedicated Moon cases still verify the blocked approaches.
The focused build and payload scan passed, and the installer hash-verified
FFIV. Gameplay confirmation remains pending. [Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-ffiv-moon-entrances).

## November 14 FFIV Sealed Cavern rope crossing across floors

The owner reported that destinations beyond the B3 rope crossing were announced
as “Not directly reachable” from earlier floors and after entering B3.
Guidance resumed after the player used the accessible rope crossing.

The November 14 FFIV Steam log `ffpr_screen_reader.log` records no route
from map 279 to the B3 rope contact on map 284 at (26,28) at 173633.093.
The other B3 rope contact at (26,10) gets a route from the same map at
173634.015, with a first leg through B2. The player enters B3 at 173683.234.
The rotated `ffpr_screen_reader.1.log` records the same announcement from
map 282 for two B3 chests on map 286 at (18,5) and (12,10), at 172918.875
and 172920.921, and for the B4 save point on map 287 at (15,10) at 172982.218.
The current log also records a failed B4 save-point route from map 284 at
173156.078.

The route bake records the controlled player's final Bottom-layer pose for
12 rope contacts on maps 269, 274, 284 and 288. The two B3 contacts at
(26,10) and (26,28) lead to (26,29) and (26,9) respectively. The other
contacts use the native relative movement of the player puppet. For B2's
first crossing, the player moves five tiles south and the separate party
puppet moves six. Each route-data entry pins the object identity and full
script hash in `data/ff4/warp_arrival_overrides.json`.

An active rope contact ends its walking leg. Directions tell the player to
step on the named rope bridge. The ready-route capture confirms the native
landing and retains the selected destination. The native Assault Door state
still governs the two side-room chests. Flags 264 and 266 track the earlier
opened doors needed on the route from the save room, and flag 269 tracks the
side-room door on B3. The existing object and map names are used throughout.

The route worker passed 20 route replays from five origins, two closed-door
cases, and four native transition start and landing replays. The fixture
supplies opened door receipts for the reachable chest cases. The route still
reports a closed door when its receipt is absent. FFIV installation is
verified. Gameplay confirmation remains pending.
[Delivery record](../SECURITY_AND_DISTRIBUTION.md#november-19-ffiv-sealed-cavern-rope-crossings).

## November 13 shared walk-open doors and FFIV route reports

The six-game door bake matches step-on events whose complete supported
script hides exact, noninteractive collision actors. It records contacts
on 14 FFIV floors, four FFV floors and one FFVI floor. FF I–III had no
matching scripted collider release in this pass. Native walk-in map
exits already have ordinary transition semantics in all six titles.
Inventory, guarded script branches, manually operated objects and
sequence-sensitive puzzles keep their own navigation rules.

Live routes require the copied powered contact and the closed actor at
their authored cells. Future-floor routes use initializer projection.
The worker first tries ordinary walking. It may then route to an opening
contact and continue through the exact cells it releases. Reachability
expands from the actual contact with its accumulated cost. Closed cells
still block teleport landing. Geometry and resource ownership stay with
the game. Same-floor refresh runs after the native gimmick hands control
back.

The Dwarven Base log at 114973.546 could not route to the active throne
scene. At 115119.031 the same scene became reachable from B1. B1's event
24 runs script 1014 and hides collision actor 18. The production replay
now finds a 36-step first leg from the logged Base position through B2.
The actual throne-room event remains a scene and follows the native
story flags. The earlier chest guards still block before Giott's scene.

The four Underworld Dwarven Castle contacts share map 118 and PointIn 1.
The selected fourth tile at (109,87) failed from the Falcon at (112,88).
The route checks the selected tile first, then uses a legal landing and
walking approach to an equivalent arrival. The partial wall remains
blocked. The reported parked Falcon at (114,128) is beyond the terminal
Kokkol's Smithy contact. The log records the player entering the Smithy
and returning at PointIn 8 before boarding. The route uses the normal
interior-return graph. The offline replay verifies two steps into the
Smithy, one step back outside, and one step to the recaptured Falcon.
The selected destination stays available inside after the observed
arrival. The world route requires the craft's live model on return.

Tomra's two town contacts lead to distinct PointIn 1 and 2 inside the
shop. From the logged spot the offline replay reaches the doors in one
and nine steps. Both indoor return exits remain separately selectable.
A moving actor blocking the outdoor approach can produce a local wait
instead of an unnecessary shop detour.

The focused build runs `browse_route_check`, `mapstate_check`,
`route_worker_check`, `fade_ready_check` and `boss_visuals_check`.
The browse replay checks 571 world exit objectives across six games.
Gameplay confirmation remains open. See the [combined delivery record](../SECURITY_AND_DISTRIBUTION.md#november-13-combined-door-route-and-boss-update).

## November 13 FFIV Castle Baron walk-open doors

The FFIV log at 102493–102581 says Story gated for both engineers from
the courtyard at Bottom (20,30). At 102584–102589 the player reaches the
rear door contact, the game enters Gimmick, and the player crosses into
Castle Baron 1F. The rear contact at (19,20) or (20,20) runs script 1034
when stepped on. It removes collision actors 46 and 47 before the player
reaches the closed doorway at row 19. The front door has paired contacts
on both sides, using scripts 1033 and 1001 to remove actors 43 and 44.
The rear door only has the automatic approach on its south side.

Current-floor routing matches the exact live powered contact to the exact
door and approach tile. Future-floor routing uses the same rule with
initializer-projected contact state. Known door-opening contacts count as
ordinary travel. The router keeps the approach restriction in reachability
and story-gate checks, keeps unrelated collisions, and refuses direct
teleport into a closed doorway. Live autowalk still checks the native
movement predicate and waits for field control by its existing rules.
The player can request fresh directions after the native door animation.
No new pathfinding hook or game-state write is involved.

Verification uses the production browse worker, same-floor directions,
and story-gate filtering against the actual courtyard and engineer actors.
Both destinations now get a 13-step first leg through the rear door to 1F.
The route keeps the engineer selected through the normal floor continuation.
The replay also checks missing contact power and the rear door's unsupported
approach. In-game confirmation is pending.

## November 12 FFIV Underworld west route

The FFIV log at 22901–22961 shows the route beginning four steps west from
(110,89), then continuing to say two steps west at (108,89) after native
movement stops. A north step to (108,88) succeeds. Three northwest scene
triggers reproduce the problem on Underworld map 2 (`Map_10020`).

The installed map bundle's `sub_collision` asset carries `GroundSubCollision`
with a west face at (108,89). Both the terrain attribute and group-one
transportation layer allow the tile immediately to its west. The map bake
now packs the extracted directional layer into the Underworld's ground
collision slot alongside its transportation grids.
Every on-foot route checks the source and destination faces before accepting a
step. Mounted routes keep their own transportation rules. The data and checks
run on the existing owned navigation worker with no new hooks or game-thread
reads.

The baseline production worker chose the blocked west edge in all nine
position-to-cell replays. The corrected worker reports Requires vehicle
from the three positions to each of the three active scene triggers.
A one-step north route from the wall and the airship edge both pass.
The replay took at most 2.2 ms per route in the offline worker fixture.
The rebake adds only 1,113 bytes for the Underworld mask, with a round-trip
decode and byte comparison of all other data. Reproduce with
`build_local.bat browse_route_check map_geometry_check route_worker_check`.
Source asset locators, hashes and the extraction receipt are in the external
workspace `tools/ff4_underworld_20261112`. Focused delivery took 98.4 seconds
with the compiler cache and verified 67 embedded resources. All three named
checks passed. The exact scanned payload is installed in FFIV.
[Installation record](../SECURITY_AND_DISTRIBUTION.md#november-12-ffiv-underworld-routing).
In-game confirmation is pending.

## November 12 FFIV Dwarven Castle chest loop

The FFIV log at 20691–20707 shows the B1 chest at (26,5) routing
from the Inn & Item Shop to 1F and back. At 20769–20777 the West
Tower 2F chest at (9,11) follows the same cycle. The production
worker replay reproduces those directions with the early-story guards
at their authored cells and the normal current-floor collision checks.

Before King Giott's throne-room sequence completes, the native
initializers on maps 119 and 121 select group `ev_e_0047`. Its
stationary guards occupy (24,9) on 1F and (17,5) and (8,13) in the
Inn, on layer 0. Their native dialogue says the rooms are off-limits.
The remote-floor NPC retry previously removed them from its planning
model. The loaded floor still had their actual collision, so it sent
the route back. The exterior map's central landing and tower
approaches occupy separate walkable regions.

The three exact guard identities now use the existing story-barrier
classification for current and future floors. The route reports
Story gated before the scene and keeps the native stairs available
after `sc_e_0047_4` sets scenario 46. Guard dialogue and NPC selection
remain native. The correction uses the existing worker snapshot and
adds three authored rows. It adds no native hooks or frame polling.

The fixture replays both targets from five actual route positions
on both sides of scenario 46. The production replay, map-state
and worker fixtures are selected with
`build_local.bat browse_route_check mapstate_check route_worker_check`.
Gameplay confirmation is pending.

## November 11 Tower of Zot 5F chest

The FFIV log at 226512.359 shows the selected chest at (16,9) after
the native landing at (21,10) on map 206. The route to 5F continued
through that stair and then reported No path. The chest at (16,10)
occupies the southern approach. The western approach at (15,9) is
reachable in two steps from PointIn 2 at (13,9), through the western
4F stair at (13,10).

Static geometry replay verifies that only this 5F landing can reach the
usable approach. The old graph can count a reachable tile beside the
occupied approach as proof of arrival. The production baseline reproduced
No path from (21,10) and chose the wrong stair at (21,7) from 4F. From
the western landing it reached (15,9) in two steps. The fix carries
the exact interaction-approach requirement into both the remote route
and same-floor recovery searches. The focused production replay passes.
It returns to 4F in one step, takes the western stair in 18 steps and
finishes two steps from that 5F landing. Each transition keeps the same
chest selected. The first combined offline run exposed a regression in
FFVI's uncollected Dream pickup. Ordinary solid treasure chests now use the
stricter approach proof. Animated and loose pickups use their existing
native action rules. The corrected Zot replay and all 34 Dream assertions
pass. The full canonical build, smoke, exact-payload scan and FFIV
installation passed. Gameplay confirmation remains pending.
Reproduce with `build.bat --check remote_exit_check` under the enforced
runner. The baseline and corrected receipts are in
`build_check/ff4_zot_chest_20261111`.
[Delivery evidence](../SECURITY_AND_DISTRIBUTION.md#november-11-ffiv-tower-of-zot-chest).

## November 17 visual descriptions and Storyline follow-up

The documented gameplay fixes have been installed. The [visual description
audit](VISUAL_DESCRIPTIONS_20261119.md) records the six-game inventory,
1,380 visible enemy sprites across the six games and the initial scene
and actor review. The
[Storyline mode plan](#november-11-all-six-storyline-category)
tracks the next objective and the routes to it.

| Scope | Requested behavior |
| --- | --- |
| NPCs and party characters | Read the actual sprite. Describe facial features and expression where visible, apparent build, clothing and equipment details, and visible armaments. Cover both idle and distinct battle sprites. Keep the description concise and include every discernible feature. |
| Enemies | Describe body type, limbs, claws, fangs, expression, and any obvious mode of attack supported by the visible art or animation. Include the full set of discernible features in concise language. |
| Items | Describe distinctive item sprites, including weapons and armor with their own art. Skip generic weapon, armor and item sprites. |
| Animations | Describe visual actions without an equivalent in the game's text. Prioritize opening cutscenes and prologues, then review the rest of the games for events where character animation conveys unexplained action. For repeated work such as an airship upgrade or bridge construction, describe the first meaningful action once for that sequence, for example, “Cid begins installing the upgrade with a hammer.” |

**Controls.** In dialogue or an event, I describes the relevant NPC. During
battle, I describes the currently targeted enemy. Ctrl+I describes the
acting party character. Resolve the visible sprite for that context, and
verify that [existing detail controls](../INPUT_BINDINGS.md#i-key-cascade-globalhotkeyhandlerhandleitemdetailskey)
still work in their menus. Item-description controls remain to be defined.

Plan several coverage passes. Record the game, exact sprite variant or event,
and the description or native text that covers it. Use the existing
[boss animation work](BOSS_VISUALS_20261112.md) when auditing battle scenes.
Verify that event descriptions follow the visible action and preserve the
native dialogue order. Localize added descriptions in each game's supported
languages.

## November 20 six-game completion goal

The owner requests a complete visual audit and Storyline mode for FF1–FF6.
Use this ledger for the active goal, superseding the starter checkpoint
below. The November 19 installed gameplay fixes are closed to unrelated
changes. Keep the source audit, integrated speech, offline verification,
local delivery and gameplay confirmation distinct.

| Milestone | Acceptance and current status |
| --- | --- |
| FF1 campaign | Objective coverage reaches Chaos. Opening, item, teaching, transport, boss and ending receipts select the next objective. Optional native conversations use the same controls. Nearby routes and native encounters pass offline. Full cross-area transport replays and integrated delivery remain open. |
| FFII campaign | Objective coverage reaches the Emperor in Pandaemonium. The sequence reads both saved scenario receipts and the native learned-word list. Native contacts, selected approaches and late-save recovery pass the focused replay. Full cross-area transport replays and integrated delivery remain open. |
| FFIII campaign | Objective coverage reaches the Cloud of Darkness. Mini and Frog gates, vehicle changes, the four Fangs, Crystal Tower and the four Warriors of Dark have verified contacts. The Nautilus dives for both native Temple of Time entrances, with guidance to bring Magic Keys or an active Thief. The Lute remains the objective inside. Selected approaches and a Nepto resume case pass. Integrated delivery remains open. |
| FFIV campaign | Objective coverage reaches Zeromus. Surface, underworld and lunar transfers use distinct native contacts. The Lunar Whale route to FuSoYa leads to the Western Lunar Path. The focused stage and approach replay passes. Integrated delivery remains open. |
| FFV campaign | Objective coverage reaches Neo Exdeath across all three worlds. The focused replay covers 150 native objectives, including the ship boarding that restores the airship. The merged-world terrain and the real flying Rift contact pass. Integrated delivery remains open. |
| FFVI campaign | Objective coverage reaches Kefka. The focused replay covers the three scenarios, a Sabin-first branch and the native victory callback. The Falcon uses Kefka's Tower's actual vehicle action pad. Storyline uses the existing guides for multi-party, timed and stateful map sequences. All 126 labels and instructions have all 12 locales. Integrated delivery remains open. |
| Database appearance coverage | The English and Japanese catalogs each match all 2,789 visible keys in the frozen enemy, field, battle and equipment inventories. The status audit has a disposition for all 525 rows. The 85 distinct assigned battle effects, 13 icons, alternate battle bodies and applied palette cues have 12-language wording. The remaining base artwork translations and integrated delivery remain open. [Artwork coverage and evidence](VISUAL_DESCRIPTIONS_20261119.md#november-20-six-game-artwork-catalog). |
| Cutscene and event coverage | Fixed inventory review closed. All 119 battle bodies and 6,110 field bodies have a disposition. A cue covers at least one stage in 65 field bodies, including all six endings, and 768 bodies are ordinary, repeated or covered by native presentation. The remaining 5,277 carry the recorded visual timing or presentation exception. The final reconciliation included all 1,109 silent priority bodies. Integrated delivery and natural timing confirmation remain open. [Scene evidence](VISUAL_DESCRIPTIONS_OPENINGS_20261119.md). |
| Verification and delivery | Open. Run source-to-output, locale, exact route, focus, lifecycle, skip and timing checks. Preserve the loading gates. Verify the final DLL and local install for all six games with the existing build, smoke, exact-payload scan and installer. Record gameplay-only confirmation separately. |

The connected travel checkpoint passed the six campaign replays, world
map journey, navigation follow-up, battle focus and dialogue appearance
checks. The runner completed in 150.7 seconds with a peak process-tree
working set of 525,361,152 bytes and confirmed all 73 embedded resources.
This is offline fixture evidence.

FFV's first tablet script places ship 17 in the merged world. Its first
boarding runs the scene that restores the airship and records 139. The
installed field table changes the merged-world terrain at 988, 999 and
1000. Resolving those cells allowed the actual route from the Guardian
Tree's native return point to the ship. The native Rift group requires
the first tablet and the airship recovery. The route recognizes only its
exact script or the corresponding baked transfer and uses the native
airship mask for models 7 and 19. The replay also rejects a foot approach
and an attempt before airship recovery. FFVI's actual tower pad carries
the native terminal vehicle action after the flight.

Retain the exact installed input identity in
[audit baseline](visual_inventory_baseline_20261120.json).
The original local evidence is under `../tools/visual_audit_20261119`.
It includes 8,504 executable field bodies, the 6,163-body visual/native-call
queue with 6,110 distinct same-map traces, the structural dispositions
and the 119 original battle script dispositions.
See [visual coverage](VISUAL_DESCRIPTIONS_20261119.md) and
[scene evidence](VISUAL_DESCRIPTIONS_OPENINGS_20261119.md).

The active capability is FF1 Storyline navigation. The first integration
block uses current scenario receipts, the Cornelia dancer, Matoya's actual
entrance scene, and the dwarf who names the castle treasure room. It adds
no native hooks. Receipt reads and one optional conversation are bounded
per requested interaction. The existing owned route worker plans movement
and the same category generation cancels obsolete speech. The initial
advice and canal block is estimated at one to two hours including route
replays. The remaining FF1 arc and five subsequent campaigns need their
own finite integration blocks. The six-game goal spans multiple sessions.
Push each substantive checkpoint and keep the goal active through the
remaining acceptance and delivery.

### FF1 NPC advice and the canal route

The Storyline category offers a verified conversation for the active quest
when one is available. Previous and next destination switch between the
conversation and the quest destination. Normal name, directions and
Autowalk controls use that selection. Ordinary advice remains optional.
Talking to a hint NPC never writes a game or mod completion flag. A new
quest or a changed native hint resets the browse choice.

| Phase | Actual contact and saved progression |
| --- | --- |
| Bridge, ship, Crown, eye and tonic advice | Cornelia town map 4, dancer entity 83 at Upper (23,18), script `sc_npc_20020_1`. Verified branches use the bridge, ship, Crown, crystal eye, Matoya introduction and tonic receipts. Before the Matoya introduction, the native dancer redirects the party to Matoya. The Storyline hint uses that entrance scene in the Crown phase. |
| Matoya introduction | Cave map 12, event 17 at Bottom (17,16), script `sc_e_0007`. The native scene sets receipt 8. Optional before the Crown. Required before the crystal eye handover if the save skipped it. |
| Crystal eye handover | Cave map 12, Matoya entity 15 at Bottom (17,7), group `ev_e_0012`. The entrance selector requires receipts 8 and 11. Script `sc_e_0012` awards jolt tonic, sets receipt 12 and treasure flag 247. |
| Elven prince | Castle map 32, attendant 116 at Bottom (19,15). Using the tonic runs `sc_e_0013` and grants the Mystic Key with receipt 13 and treasure flag 248. |
| Nitro powder advice | Mount Duergar map 21, dwarf 1 at Bottom (30,20), script `sc_npc_20051_6`. Before receipt 14 the native conversation names the castle treasure room. |
| Nitro powder | Cornelia Castle map 2, chest 10 at Bottom (27,14). The key opens the actual castle lock. Script `sc_e_0014` sets receipt 14 and treasure flag 505. |
| Canal | Mount Duergar map 21, Nerrick entity 45 at Upper (26,52). Script `sc_e_0015` sets receipt 15 and starts the Overworld canal sequence. |

The focused build verified the 67 embedded resources and the production
planner passed 136 assertions and 31 path legs. The new fixture cases
cover native quest advancement, description of the appropriate advice
choice, skip and selection retention, route capture from the relevant
entrances and the chest behind the castle lock.

### FF1 canal to Chaos

The campaign reads one bounded prefix of the initialized native scenario bank
on explicit input. No worker accesses game pointers or runs scenario actions.
Storyline follows the recommended quest order, skips any completed receipts
and treats an unavailable required receipt as unknown. The final ending
continuation records receipt 42 and ends Storyline.

| Segment | Verified native contacts and receipts |
| --- | --- |
| Earth Crystal | Vampire and star ruby at map 78, entities 54 and 22, receipts 16 and 17. Give the ruby to the giant at 81/11 and speak to Sadda at 40/9, receipts 18 and 19. Open the earth-rod slab at 78/52 and fight Lich at 80/33, receipts 20 and 21. The Melmond villagers at 34/67 and 34/66 explain the cave location and what the giant eats. |
| Canoe and airship | The post-Lich sage at 41/57 awards the canoe, receipt 22. Marilith is 87/33, and the levistone is 91/8, receipts 23 and 24. The native desert scene at 1/82 awards the airship in its return continuation, receipt 25. The Crescent Lake sage and resident at 41/42 and 41/32 explain Mount Gulg and the ice cavern. The Elfheim sage at 22/47 explains raising the airship. |
| Sunken Shrine | The Caravan at 59/8 supports the native purchase gated by receipt 27. Gaia's release row enables the spring faerie at 60/90, receipts 28 and 29. The Onrac woman at 52/67 sets receipt 30 and enables the repeatable submarine at 52/64. The native map transfer reaches the shrine. The Rosetta Stone at 103/42 and Kraken at 96/38 use receipts 31 and 34. The Onrac resident at 52/40 identifies the camp and the pirate at 60/78 explains the faerie's capture. |
| Wind Crystal | The Onrac scholar at 52/42 identifies his brother Unne. Unne at 34/126 teaches Lufenian with receipt 32. The Chime comes from Lufenia at 70/131, receipt 33. The Mirage Tower robot at 106/20 explains where the warp cube went. The Waterfall Cave robot at 104/23 gives it with receipt 26. Tiamat at 112/3 records receipt 35. |
| Past and final boss | The bat at 72/12 explains the Black Crystal. Its first interaction at 72/39 sets receipt 36 and enters the past. Reentry uses the separate step trigger at 72/64. The lute slab at 115/9 sets receipt 37. Encounters at 116/20, 117/52, 118/69 and 119/14 set receipts 38–41 on victory. Garland at 120/6 begins the final Chaos encounter. Its victory continuation leads to the native ending. |

The focused replay passed 231 assertions and 58 physical legs. It addresses
every recommended objective and the separate Onrac and past-shrine reentry
contacts. The first new replay used a blocked start position for past Kraken.
The retry starts the four rematches at their native floor entries and passes.
Forty new strings cover all 12 supported languages. Actual gameplay, extended
cross-area vehicle and departure replays, and local delivery remain open.

### FFII Altair to Pandaemonium

Storyline is available after the native Altair opening completes. It reads
that save's scenario receipts and the actual learned-word list. An opened
secret door, accepted handover or later receipt can supersede an earlier
keyword or optional item step. Native Learn, Ask and Item menus remain
manual. Directions and Autowalk use the verified contact and then hand
control back for the plot choice or battle.

| Segment | Progression reviewed against the installed scripts |
| --- | --- |
| Rebellion | Learn Wild Rose from Hilda before using it with the Fynn barkeep. Scott's ring goes back to Hilda. Learning and asking about Mythril are separate. Minwu supplies the canoe. Talk to Josef for optional context, free Paul and Nelly, defeat the sergeant, and take the mythril to Tobul. |
| Dreadnought | Ask the Bafsk agent about Dreadnought, follow the launch and collect the optional pass. Return to Hilda, learn Airship from Cid, ask him about it to learn Sunfire, and ask the king about Sunfire. Josef opens the snowcraft store. The Bell, Kashuan door, Gordon, Egil's Torch and Sunfire use separate receipts. The actual cell bars are at 169/28. On the Dreadnought, free the captives before using Sunfire on engine 175/12. |
| Deist and Fynn | Return to the king, learn Dragoons from Gordon, and arrange transport through Leila. Learn Wyverns using the native conversation at Deist. Obtain the pendant, speak to the wyvern, defeat the guardian and place the egg at the Life Spring. The Coliseum rescue leads to the camp and the defeat of Gottos in Castle Fynn. |
| Mysidia | Learn Mysidia, Mask and Ekmet Teloess from Hilda at 43/23. Open the secret wall, recover the two masks, and collect the crystal rod. Continue through Leviathan with Ricard and the Roundworm encounter. Enter Mysidian Tower, defeat the three Gigas guardians and reach Minwu and the Ultima Tome. |
| Cyclone and final boss | Learn Cyclone and use the pendant at the mirror. The first Emperor fight at 273/5 sets receipt 75. Cid's scene enables the airship and Palamecia landing. The pit and Leon lead back to Fynn. The Jade Passage entrance and its exit into Pandaemonium are separate native scripted transitions. The final Emperor is 316/46, and ending receipt 82 completes Storyline. |

The authoritative keyword bindings are in the
[FFII native keyword inventory](ff2_keyword_mechanics_inventory_20261101.json).
Exact local scripts and map placements remain under `../tools/mapdump`.
The candidate has 85 localized FFII labels in all 12 supported languages.
The first contact replay found that the mirror and Leon keep their native
Entity and NPC types on the baked floor, even though their later continuations
transfer the party. Their Storyline routes now use those types.
The second replay exposed a separate Palamecia failure. Its pad uses FFII's
native vehicle-action inference for landing, with Airship 3. The next
candidate uses that inference and the matching boarding semantics for
Leviathan's ship-only entry.

The focused replay reaches all 84 FFII objectives with zero failures and
keeps the 58 prior FF1 movement checks passing. FFII approach checks include
the actual Altair and hidden Fynn arrivals, the Leviathan ship approach,
the two Emperor fights and the Palamecia landing pad. An advanced save
skips obsolete learned-word, pass and cell-bar prompts. The native-shaped
fixture confirms one bounded list observation and no Storyline word read
during field loading. Full cross-area transport and gameplay confirmation
remain open.

### FFIII Altar Cave to the World of Darkness

The next objective uses the save's scenario receipts and the live map
contacts. Entering as Mini or Frog remains a native spell action. The
existing form-aware puzzle guides contain the matching instructions and
safe follow-up. The Storyline labels identify when a form is needed.

| Segment | Verified quest operations |
| --- | --- |
| Floating Continent opening | Approach and examine the rocky switch, defeat the Land Turtle, and receive the Wind Crystal's jobs. Topapa, the traveler, Cid and the king lead to Sara and the Djinn. Cid's first airship is at 21/3 with receipt 194. An older save with the boulder already broken can continue. |
| Nepto and the Fire Crystal | Flee Bahamut and receive Mini, enter Tozus, then help the Vikings and retrieve the Nepto Eye through the statue. A loaded state with receipt 32 and no Eye receipt 33 targets the exposed Eye at 173/5. Toad opens the Owen and Underground Lake entrances. The Ice Horn, Fire Crystal and Hein sequences use their separate native receipts. |
| Surface World and airships | Wheel of Time and Cid lead to the Enterprise, Aria and the Water Crystal. Gill and the old men open and guide the sewer sequence. Goldor unlocks the Enterprise, Saronia introduces Alus and the Nautilus, and Doga's first magic circle upgrades that craft. Once Unei joins, the Ancient Ruins lead to the Invincible. Hecatoncheir guards the Fang of Earth. |
| Crystal Tower and finale | Doga and Unei's victory continuation enables the automatic two-key handover at 298/5. The Fangs remove the four statue checkpoints. The Earth Crystal and Syrcus door lead to the Crystal Tower mirror. The mirror transfer sets slot 95 of native ScenarioFlag2, absolute receipt 351. Xande and the first unwinnable Cloud of Darkness encounter do not end Storyline. The four Warriors of Dark and the final fight at 335/4 lead to ending receipt 105. |

The first focused replay verified the full sequence of receipt changes and
found two native types to correct. Gill uses an NPC contact and the Crystal
Tower mirror uses an Entity contact. Their later transfer continuations do
not change the types on the loaded floor. The Djinn approach now starts
at the actual Sealed Cave floor arrival. The revised build verified 67
resources and all 86 Storyline objectives. Six representative movement legs,
including the Djinn and the final Cloud of Darkness approach, passed with
zero failures. The 87 FFIII labels have all 12 supported languages. Full
cross-area transport, gameplay and integrated delivery remain open.

### FFIV Baron to Zeromus

The recommended objectives start with Cecil's return to Baron and end with
Zeromus. Native battles, item selection and vehicle controls remain under
the player's control. The story contact uses the active room and quest state.

| Segment | Verified quest operations |
| --- | --- |
| Baron to Fabul | Start with Rosa, Cid and Cecil's room. The scene-gated attendant after receipt 2 is 14/5. The Mist Dragon, Kaipo, Tellah, the waterfall, Edward and the Sand Ruby lead to Rosa. Rydia's fire scene and Yang lead to Fabul's defense. The actual Fabul departure trigger is 1/148. |
| Mysidia and return to Baron | Mysidia and Mount Ordeals lead to both Scarmiglione fights and the mirror chamber. The elder, Yang and the Baron waterway precede Baigan and Cagnazzo. Edward and the Twin Harp open the Dark Elf's two encounters. Earth Crystal receipt 98 precedes returning it to Troia. Kain's contact is 1/135, an Entity whose later script takes the party to Zot. |
| Underworld | Zot and Agart lead to the Enterprise's descent. The Dwarven Castle defense, Dr. Lugae and the cannon door precede Yang's scene. The lock uses ScenarioFlag2 slot 17, absolute receipt 273. The Baron engineers prepare the Enterprise. Edge's parents and Rubicante are separate steps. The floor initializer places Rubicante 10 at Bottom (21,16) after receipt 52. The Falcon, Luca's Necklace, the Sealed Cavern and the drill lead back to the surface. |
| Moon and finale | Mysidia raises the Lunar Whale. Its Earth-side cabin is 357, with crystal 1. Its Moon-side cabin is 161, with the plot return crystal 14 after FuSoYa. The Crystal Palace uses the actual lunar approach. The archfiends and CPU scenes lead to the Lunar Subterrane, then Zeromus at 317/3. Ending receipt 70 closes Storyline. |

The initial contact replay resolved the authored end-to-end receipt order.
It identified the correct attendant variant, two native contact types and
the projected Rubicante spawn to fix. The fixture now models his visible
position after the native initializer moves him from the off-map staging
tile. The final replay passes all 72 objectives and six physical approaches,
including both Lunar Whale crystals, FuSoYa and Zeromus. The retained FFV
Exdeath fixture also passes 227 assertions and 109 legs. The 72 new labels
have all 12 supported languages. Full campaign travel, gameplay confirmation
and integrated delivery remain open.

### FFV meteorite to Neo Exdeath

The 149-objective itinerary follows the opening meteorite, the Wind Crystal,
Walse, Karnak, the airship and the meteorite transfers to Galuf's world.
The Karnak courtyard encounter and the timed escape use separate native
contacts. The second world route includes the Big Bridge, Xezat's fleet,
the Barrier Tower, Moore and the first Exdeath battle. The merged world
sequence includes the recommended tablet order and the Rift. The native
world-change and Rift receipts retire earlier objectives, including missed
tablet stops once the party enters the Rift. The final Exdeath contact
is 569/3 and native victory receipt 161 ends the itinerary.

All 149 labels are present in the 12 supported languages. The focused
replay checks every ordered receipt, actual baked target types, selected
physical approaches, the Karnak escape and a late save entering the Rift.
The separate Karnak regression passed all 29 assertions over 12 route
legs. The build verified 67 embedded resources. Full campaign travel,
gameplay confirmation and integrated delivery remain open.

### FFVI Narshe to Kefka

The recommended itinerary follows Terra and Locke through Narshe, Figaro,
Mount Kolts and the Returners' Hideout. The three scenario selections use
map 584. The current unfinished branch follows the native selection receipt
51, 65 or 103 and its own completion receipt 64, 69 or 135. A player who
chooses Sabin first receives that branch's objectives. In Sabin's scenario the
Mobliz item shop is 87/14. A ready read of normal-inventory content 24 switches
the objective from buying Dried Meat to using it on Gau after a Veldt battle.

The World of Balance campaign covers the defense at Narshe, Zozo, the opera,
Magitek Factory, Sealed Gate, the banquet, General Leo's ship, Thamasa and the
Floating Continent. The pilot control at 119/9 owns the ascent choice. The
existing local guides handle specialized scenes, puzzle states and player
choices such as waiting for Shadow. Native receipt 297 starts the ruin
itinerary, from Cid and the raft to Gerad, Setzer and the Falcon. Returning
to the Falcon from Kefka's Tower selects its actual world entrance again.
The three-party guide uses the current party and the tower's plates and
boss receipts. Summit receipt 501 starts the final sequence. Encounter 472
calls the native victory continuation 1006, which sets ending receipt 591.

The first focused build passed 116 ordered objectives, the first Narshe
approach, 156 story model plans and 709 existing guide identities. The
branch and typed-contact checks use exact baked identities. Complete
campaign travel, new label localization and integrated delivery remain open.

### Storyline travel between areas

A Storyline goal uses the current world vehicle snapshot regardless of the
browse filter. From inside an area, the production planner first attempts
the full route with the current evidence. When a different-area objective
needs a connection the current snapshot cannot prove, the fallback reaches
the first legal world-map departure. It retains native gates, contacts,
arrival points and the original objective. A new request on the world map
uses observed vehicle placement. Autowalk can replan on its existing
native-ready continuation. Other categories and same-area Storyline routes
retain their usual gate and reachability decisions.

An actual FF1 replay from Gaia to Onrac verifies the real Gaia departure
with foot-only indoor evidence and the actual Onrac approach with the
mounted airship after arrival. The ordinary route remains unreachable from
that indoor snapshot. All six campaign checks, the six-game navigation
regressions and the native-state fixture passed. The latter also checks
FFVI's exact Dried Meat inventory entry, quantity change and loading/worker
exclusion. This verifies the shared departure behavior offline. Broader
campaign travel and gameplay confirmation remain open.

## November 11 all-six Storyline category

The owner requests the next main-story objective in a Storyline category
in FF1–FF6, with directions and Autowalk from any map. A save's progression
selects the objective. The player's current map and transportation select the
route to it. When more than one valid objective is available, use a reviewed
recommended order and skip steps the save has already completed.

Initial inspection found the saved scenario bank and its existing copied
navigation state in all six games. A shared current-objective field has
yet to be identified. Use exact persistent native receipts for the stages
that need authored objectives. A walkthrough may suggest the order.
Qualify each event, prerequisite and completion against the Pixel Remaster
data. In FF1 the original numbered world map is a reference for order.
[Nintendo's original FF1 world map](https://www.nintendo.co.jp/clv/manuals/en/pdf/CLV-P-NABJE.pdf).

The existing route request stores a destination map and native target,
and Autowalk can replan after an expected map change. Prove this for
interior exits, world travel, transportation, return trips and destination
floors. Audit world-changing transfers and one-way story transitions where
ordinary floor-to-floor routing does not supply a valid route. The player
still performs story actions that require native confirmation. Completion
comes from the resulting game state.

The existing evidence qualifies particular cases that can become campaign
tests. FF1's Chaos Shrine seal and Garland use separate receipts. FFII
already reads a learned keyword on an explicit guide request. FFIII has
owned route replays between the Floating Continent, Surface World and
underwater. FFIV has verified fetch-and-return legs for the Twin Harp,
but the full three-world route matrix needs coverage. FFV and FFVI need
world-phase checks that retire prior-world objectives. FFVI also needs
save cases for its parallel scenarios and active party. Treat these
existing receipts as evidence for individual stages.
[FFI story gate](FF1_GATE_FOLLOWUPS_20261101.md).
[FFIII world journeys](WORLD_MAP_JOURNEYS_20261030.md).
[FFIV campaign interactions](FF4_PUZZLE_GUIDES_20261101.md).
[FFVI story sequences](FF6_STORY_NAVIGATION.md).

| Checkpoint | State and exit condition |
| --- | --- |
| Progression sources and order | Initial source review. Define start-to-finish cases for each game, including branches, existing saves, skipped steps and unknown state. |
| Storyline category and global route | Early FF1 implementation uses the existing category, directions and Autowalk entry points. Offline route replays select the same objective from different origins. Multi-map autowalk and later chapters remain open. |
| Campaign coverage | Pending. Qualify every main-story stage and its native action in FF1–FF6, including optional branches that can already be complete. |
| Verification and delivery | Pending. Cover state reload, readiness, route cancellation, bounded work, 12 locales, canonical verification and local deployment. Commit and push the verified change. |

### First FF1 objective sequence

The following draft uses the actual ScenarioFlag1 receipts. Selection uses
the save's progress and the next required action. The player's position
affects how the route reaches that action. The startup script finishes after
setting receipt 0. The playable opening stages follow that receipt.

| Starting state | Objective and native contact | Completion evidence |
| --- | --- | --- |
| Opening complete and receipt 1 clear | Meet the King of Cornelia. Use the throne-room soldier, map 3 entity 14 at Upper (20,13), script 7 in group `ev_e_0002`. | The audience and its continuation set receipt 1 after the dialogue. |
| Receipt 1 set and receipt 2 clear | Rescue Princess Sarah. Garland is map 72 entity 32 at Bottom (38,29), script 9 in group `ev_e_0003`. | The combat continuation sets receipt 2 and sends the party back to the throne room. The return sequence ends with receipt 3. |
| Receipt 3 set and receipt 5 clear | Receive the Lute from Princess Sarah. Speak to her on map 3, entity 7 in `ev_e_0004`. The native step contact at entity 42 can also begin the gift as the party leaves the room. | Both gift branches set receipt 5 and treasure flag 244. The production replay exposed this prerequisite for the castle departure group. |
| Receipt 5 set and receipt 6 clear | Leave Cornelia Castle to see the bridge construction. Use the three step contacts on map 2, entities 44–46 at Upper (25–27,43), script 12 in group `ev_e_0005`. | The construction scene on map 121 returns to the castle and its continuation sets receipts 4 and 6. |
| Receipt 6 set and receipt 7 clear | Cross the new bridge. Use the active Overworld contact on map 1, entity 89 at (145,145), script 17 in group `ev_e_0006`. | The prologue plays on map 124 and returns the party to the Overworld. The return continuation sets receipt 7. |
| Receipt 7 set and receipt 9 clear | Get a ship from Bikke. Speak to entity 38 on map 13 in Pravoka, Ground (13,11). The native NPC contact runs script 22. | The encounter continuation `sc_e_0009_2` awards the ship before it sets receipt 9. Keep this objective through the battle and award sequence. |
| Receipt 9 set and receipt 10 clear | Find the crown in Marsh Cave. Use chest entity 40 on map 75, Bottom (33,47). From Pravoka, the first leg reaches the real Ground exit at (27,39) so the next route can use live Overworld ship state. | The chest starts the encounter in script 24. The victory continuation opens chest 40, records receipt 10 and sets treasure flag 501. |
| Receipt 10 set and receipt 11 clear | Bring the crown to the Western Keep. Speak with actor 105 on map 33 in group `ev_e_0011`. | The script begins the encounter after the transformation. Its continuation awards the crystal eye, sets treasure flag 246 and records receipt 11. |

Receipt 2 can become true inside the rescue return chain. The model withholds
the next objective until receipt 3 confirms the return. The production
fixture passed 88 assertions and twenty movement legs with the installed FF1
map and scenario data. It checks objectives from the castle, Overworld,
Chaos Shrine and Pravoka, a castle floor transfer, the Lute prerequisite,
missing state and later saves. The ship quest starts from the real
post-prologue position (145,145) and the Pravoka arrival on Ground (27,37).
The Crown cases check the departure from Pravoka, the Overworld approach
near the cave and genuine arrivals on all three cave floors. The full
voyage from Pravoka with a relocated ship still needs confirmation.
The Western Keep route is checked from the keep, the Overworld approach
near Marsh Cave and beside the just-opened Crown chest.
The bridge
uses the existing selected-event route to its actual world foot tile. The
Storyline category calls the same native-safe route request and Autowalk
entry points. The selected objective comes from ten saved receipts
when it is requested. A delayed directions response must still match the
category and objective. All eight objective labels have all 12
localizations. This starter is outside the installed mod. Full cross-world
journeys, native gameplay acceptance and later campaigns remain open.

The request arrived during the FFIV airship-label build. The November 17
priority places its implementation after the outstanding gameplay fixes and
local deployment, in the [visual descriptions follow-up](#november-17-visual-descriptions-and-storyline-follow-up).
Gameplay and frame timing are separate verification steps.

## November 11 FFIV Kain airship label

The world-map airship beside Troya uses the native Tower of Zot travel
contacts. Their Objects entries and interaction cues use the localized name
“Kain's Airship.” The resource and event identity check covers both story
variants and all 12 locales. The full offline suite, smoke, exact-payload
scan and local FFIV installation pass. Gameplay confirmation is pending.
[Identity evidence](OBJECT_IDENTITIES.md#kains-airship-on-the-ffiv-world-map).
[Delivery evidence](../SECURITY_AND_DISTRIBUTION.md#november-11-ffiv-kains-airship).

## November 10 FFIV world entrance directions and remote returns

The actual FFIV log from world map 1 at (45,66) identified Town of Troia
trigger 56 at (40,71) and Troia Castle trigger 59 at (39,69). Browse announced
the same destination as a required intermediate map at 151074.609 and
151075.718. These are physical world trigger tiles. The owner wants every
ordinary list row still governed by the existing Map Exit Filter preference.
The routing correction checks for an active entrance to the same map and
explicit native PointIn. The closest reachable equivalent entrance can
complete the trip. Different PointIns and ordinary cross-floor routing
remain separate destinations.

For remote indoor exits, compare the active native arrival cells per source
floor when the destination is the floor the player is standing on. One
proved arrival cell makes those remote returns unnecessary in the list.
Multiple distinct arrival cells or ambiguous arrival metadata keep the
choices available, including with exit grouping enabled. This is independent
of the world route change. The current floor owns its own exit rows and the
route graph keeps its full connectivity.

Focused cases are the Troia weapon shop, B3 to B2, the three real map 5
returns into different map 4 positions, and the two logged world entrances.
The previous production replay failed six assertions, including both
recursive directions and the grouped three-landing case. The fixed replay
passes all assertions. Exit 56 now ends at (40,70) in nine steps. Exit 59
ends at (40,68) in seven. Both browse results are direct and the literal
list retains all physical entrance rows. Directions plus browse took
7.842 and 6.675 ms before and 4.402 and 3.828 ms afterward in the bounded
offline fixture. These are worker times. The fixture observed zero native
owner violations. The first full build compiled but its raw event inventory
check rejected 190 missing scripted returns. Keep those raw records and
apply their suppression flag when formatting the pathfinder list. The
focused event inventory check now retains all 190 returns and passes over
2,454 master roots. The scanner and exit checks also pass. The corrected
canonical build and all required checks passed. The scanned DLL is
installed in FFIV and hash-verified.
[Installation evidence](../SECURITY_AND_DISTRIBUTION.md#november-10-ffiv-navigation-and-battle).

Reproduce with `build.bat --check remote_exit_check` in the enforced build
runner. The raw and displayed list boundaries also run through the
`event_tracking_check` and `entity_scanner_check` targets. Focused receipts
are in `build_check/ff4_navigation_20261110`.

## November 10 FFIV necessary airship boarding

The owner reports optional Enterprise boarding and an immediate landing. The
actual game log records the exact Town of Troia world exit at (40,70) from
(46,41), (50,65) and (51,65). The routes at 147175.296 and 148792.218 say to
board the Enterprise facing south, land, then walk one tile west. The parked
model is on (41,70). That candidate counts the native boarding position change
as zero travel and saves one step in the planner's score. Static terrain and
terminal-cell evidence has ground approaches of 39, 15 and 16 steps for those
starts when the retired Zot contact is absent. The fixture must qualify the
same poses with the production terrain and initializer.

Keep a proved FFIV ground route ahead of any optional parked-airship plan.
Exclude zero-distance flight landings during candidate selection so a usable
longer flight remains available when required. Preserve aboard routes,
vehicle-specific entries and the deliberate Agart-to-Mysidia crossing. The
affected production scope is the FFIV world route and selected browse result.
The production replay passes from all three logged starts, with 39, 15 and
16 steps in one walking phase. At the entrance it reports zero steps. The
Agart-to-Mysidia replay has 67 steps and three phases with a nonempty flight
between board and land. Reproduce with `build.bat --check route_worker_check`
under the enforced runner. Gameplay confirmation remains separate.

## November 10 FFIV directions during airship movement

Scope is spoken directions for selected world-map targets while riding an
FFIV airship. The live Enterprise route to Troia Castle produced ten complete
plans at 145258.671–145259.718 before publishing. Exact start-tile validation
discarded each plan while the player was moving. A single stationary request
used 0.169 ms of worker compute from the kept route. Fresh full plans took
about 100–157 ms each. These are route timings, separate from game-frame lag.

The correction should reuse the validated landing and walking approach and
recompute only the flight leg once on the existing worker. A second pose
change needs a prompt, explicitly approximate bearing with no stale route
publication. Airship identity, current map, readiness, route epochs and the
manual-only boundary govern the result. Walking, water and ordinary mounts
keep their existing path. Compare against the recorded moving sequence in the
worker replay, then run the canonical delivery checks.

The production fixture passed from the reported Enterprise pose, across the
world seam and through a vehicle change. The final canonical replay measured
63.070 ms for the full plan and 1.080 ms for the bounded flight adjustment
with immediate fixture delivery. No native-owner violations were observed.
The earlier two-fix candidate passed smoke and scan while FFIV was open.
The final combined candidate passed verification and is installed. Live
audible and application-frame timing remain unmeasured.

## November 10 FFIV Troya Castle treasury guide

Scope is the B1 four-switch guide from native entry through completion, with
repeat and resumption at each recorded stage. The current game log records
B1 entry at (12,11), the reachable entrance switch at (7,10), and guide
requests for later blocked contacts at 142177.156, 142190.171 and 142197.140.
The ordinary pathfinder reached the correct intervening contacts. The guide
recipe paired each completion receipt with the switch behind the gate it opens.

The correction uses the FFIV guide data and the existing ordered navigation
request. If the room reload hides a numbered contact, the worker tries the
entrance contact with the same snapshot and native gate rules. No new hooks,
scans or native actions are needed. The finish line is native
source qualification, the existing guide/controller and production route
replays, canonical build, smoke, exact-payload scan and verified FFIV install.
Budget one focused implementation and replay pass, then the serial delivery
build. The last canonical run took about 130 minutes. Gameplay is separate.
[Native sequence and replay scope](FF4_PUZZLE_GUIDES_20261101.md#november-10-troya-castle-treasury).

The treasury source, focused replay and canonical checkpoint passed. The
owner then reported an FFIV Study cast that spoke only a weakness. The
second scope is the displayed HP and weakness lines from FFIV Study and
Scan. The native result and existing FFIII producer guard define the
boundary. The combined FFIV build passed the battle replay, canonical, smoke
and payload checks. The verified installation includes both corrections.
[Native battle evidence](SCAN_DISPLAY_20261027.md#november-10-ffiv-study-and-scan).

## November 10 FFV reports and shared navigation/audio

This delivery covers the workspace BugReports file and the owner's two
follow-ups. The finish line is the focused checks, one canonical serial build,
isolated smoke, exact-payload scan and verified installation in all six games.
The compiler uses the enforced two-GB runner. Receipts use
`build_check/bug_reports_20261110/`.

| Capability | Acceptance and evidence | State |
| --- | --- | --- |
| FFV Walse progression | Reach the active tower and meteorite. Keep the sunk tower and unknown-state restrictions. [Native terrain](MAP_EXITS.md#ffv-walse-terrain-november-10). | Implemented. 34 assertions across 23 route legs pass, including the Wind Drake. |
| FFV boss categories | Siren and the other reviewed boss contacts use NPCs from the live list and other floors. The original contact owns activation. [Encounter evidence](FF5_EVENT_CONTACTS_20261106.md#november-10-boss-contacts). | Implemented. Catalog and scanner checks pass. |
| Shared previews | Play the selected cue when the master and cue switch are off. Preserve volume, spatial mode and automatic mute. [Audio evidence](SOUNDSCAPE.md#manual-previews-and-the-navigation-audio-master). | Implemented. Menu and mixer checks pass. The owner confirmed normal audio after enabling the master. |
| Shared story-gated bearing | Add approximate same-floor distance and bearing, including across map seams. Preserve route validity and independent visibility settings. [Navigation evidence](MAP_EXITS.md#straight-line-bearing-for-story-gated-targets). | Implemented. Speech checks pass. |

The first mounted fixture exposed the stale transport tile in addition to
the raw attribute. The second replay includes the native transport-cell
conversion and passes. Source, offline checks and local delivery are Closed.
The current shared working tree was built, including the integration already
present before this task. The canonical build, isolated smoke and exact-payload
scan passed, and all six installed hashes match. [Delivery evidence](../SECURITY_AND_DISTRIBUTION.md#november-10-ffv-and-shared-controls).
Changed gameplay and physical audio remain for the player to confirm.
[Player checks](../TESTING.md#november-10-ffv-and-shared-controls).

## November 10 FFIV Troya installation and pub admission

The November 10 canonical build includes `troia_passages.inl` and the FFIV
event labels, as recorded in `build_check/bug_reports_20261110/source_inputs.json`.
The Steam FFIV `hid.dll`, the verified build and the delivery receipt matched
SHA-256 `4b4d23779b2444a0819891e97b50f5f72ab1367d789ba3acd462131e5501530c`
at that deployment. The later FFIV update includes those corrections.
The November 7 build was build-only. The actual FFIV log from the November 10
session confirms the installed changes by announcing “Passage switch” and a
straight-line bearing.

From the latest quick save in Town of Troia Pub (map 91, Bottom 18,15), the
far-side passage contact at 18,4 says “Story gated. 11.0 steps North in a
straight line” at 140599.812. Pub attendant script `sc_e_0139` accepts key
item content 60 and opens the passage. The Tavernkeeper sells the pass for
10,000 gil. The player reported finding the attendant and identifying the
pass handover. Successful passage traversal has not been confirmed.

The town entrance provides a separate route to the Black Chocobo Farm. Its
examined switch opens its local passage. The farm and pub also have return
contacts with their own effects. The deployed replay exercises reachable
controls and opened states. It did not cover selecting the Pub return contact
from this quick-save position. The current announcement there gives a bearing
but omits the pass requirement.

## November 7 FFIV Troya guidance and event availability — queued

The owner subsequently resumed this issue as a progression blocker.
The narrow fix is in progress. Native script review distinguishes five
step contacts from the ordinary examined farm switch and the pub attendant.
The four step-contact scripts remove their own triggers and exact passage
colliders. The pub attendant separately checks Club Pass content 60.
Town controls are ordinary navigation interactions.

The route worker now tries an actual reachable control when a Troia
passage collision prevents the selected journey. A copied observation
with only that control's released actors removed proves its relevance.
Directions end at the real control with every current collision still
enforced. The selected final destination is retained, and a fresh request
after the native action establishes the next leg. This adds no native
flag writes or automatic interactions. The five step contacts are named
“Passage switch” in all 12 languages.
Source effects are checked by `tools/review_troia_passages.py`.
Production route replays are in `tools/ff4_puzzle_journey_check.cpp`.
Build and installation receipts use `build_check/ff4_troia_20261107/`.

Owner requested documentation for a later fix. The puzzle guide did not
activate, several Events entries called “Door” may actually be floor
switches, and walking over some of those events made destinations
unavailable. The switch identities and cause of the disappearing
destinations have not been established.

The actual FFIV game-directory log and rotated parts 1 and 2 were read
using shared file handles. Part 1 records five “Door” events across the
Troia pub, King's Pub, Black Chocobo Farm and Chocobo Farm B1 at
158152–158161. The current log records Door events on map 95 changing
from “Not directly reachable” at 158754.984 to “Story gated” at
158799.390. The Event list subsequently changes from five entries to
three. The King's Pub Door says “Destination is no longer available”
at 158810.359 and 158890.375, with an explicit direction request
returning the same at 158890.546. Shift+G says “No puzzle guide for
this map” at 158823.328 while the party is on map 95, Town of Troia
Chocobo Farm B1.

The source currently registers the Troia Castle treasury guide for
map 102. Keep the town map observed in these logs distinct from the
castle treasury during the later investigation. Review the actual
event scripts, activation and consumption receipts, and guide entry
conditions. Verify whether the door-labelled events are doors,
floor switches or another mechanism, and whether an event replacing
its native instance wrongly retires a usable target. Check routing
and listing before and after stepping on each affected contact.
Apply the owner's complex-puzzle scope when assessing guide coverage.
Implementation and gameplay confirmation remain pending.

## November 12 FFIV Castle Baron front gate and Overworld exit

Scope is the front drawbridge and its route through the west 1F hall. The
reported log shows exterior map 4 at Bottom (16,33), a Story gated Overworld
exit at (18,34), and a route to Gate via the hall. Upon arriving in map 9 at
16278.453, the destination was retired. Keep the real crossing available
until opened and retain the chosen Overworld exit through the floor change.

The courtyard initializer Map_20010_1 has two map-local trigger-deletion
guards. ClearMapLocal resets those locals before a new floor initializes.
The remote projection now uses those exact zero-on-entry facts. The current
floor still evaluates the captured locals. ScenarioFlag2/24 (absolute 280)
remains the native front gate receipt. The independent rear gate receipt
and all other maps retain their existing semantics.

The gate has two contact cells on each side. Native script IDs 1001 and
1033 remove the same front gate collision and write the same receipt.
FFIV directions to the four exact Overworld exit rows can route to a
reachable active contact when the closed gate blocks the selected exit.
The existing worker planner supplies any required floor change. The
destination and native arrival receipt survive that change. On the
courtyard the path ends at the gate contact, and the next request
replans from the native position and collision. No hook, game-state
write or extra work on the game thread was added.

The pre-change production replay reproduces an unavailable gate from map
9 and Story gated exits from both maps. The changed browse_route_check
passes all eight open/closed route cases and their continuation checks.
The focused production build, four selected offline checks, isolated smoke
and exact 22-file Defender scan passed. All 22 installed FFIV files matched.
The other five mod installations and 138 protected file checks passed.
Implementation and offline delivery are Closed. Gameplay confirmation
remains separate.

## November 6 FFIV Castle Baron East Tower 1F west chest

The installed build returned “No path” to map 17 treasure entity 9
at Bottom (2,5) from (6,7), (4,5) and (8,5). The actual game log
at 107853–107895 and 108090–108103 records the failures, including
after the other three chests were opened. The remote route used a
different 1F entrance and failed again upon arrival.

Map_20011_13 places a wall north and west of the chest and another
solid chest to its east. Its southern interaction side is the Bottom
(2,6) stair to B1. Map_20011_16 PointIn 1 lands on the reciprocal
stair. That stair returns to 1F PointIn 3, directly south of the chest.
No switch or item is required.

The pathfinder accepts this one interaction side as a native arrival
only. The same route graph reaches B1 and then selects the return.
When the player starts on the B1 return stair, the worker validates
the cardinal step off and back on using the existing obstacle and
transition rules. Repeated directions and the beacon retain the outbound
step until the player leaves the stair. At the 1F landing the normal
interaction direction faces north with zero walking steps.
Both stairs stay terminal for ordinary walking routes.

The passing production replay in `tools/browse_route_check.cpp` covers all three
logged starts, the B1 return, the repeated request and beacon, the 1F
interaction, and an ordinary neighboring chest. The initial replay identified
an interaction-layer mismatch, corrected before the successful rerun.
The combined canonical build passed at 802,615,296 bytes peak aggregate
memory in one capped job. The final DLL and dependency set passed the
smoke and Defender checks. The FFIV installation matched all 22 scanned
files and preserved 24 protected files. Gameplay remains separate.
Build and install evidence is recorded under
`build_check/ff4_baron_stair_chest_20261106/`.

## November 5 FFIV Fabul East Tower landing

The owner's FFIV log records four route requests that speak five East
from the central landing of East Tower 2F and then leave the floor.
The targets include Woman (Purple) and Man (Blue). Transfers go to
either 1F or 3F. At 16840.203 the same first five East is followed by
successful eastward movement toward the chest, confirmed by the position
at 16840.687. At 17136.875 the reported NPC route is followed by over
two seconds before the next logged movement and a transfer to 3F. A
request during a previous step cannot explain every failure.

The installed FFIV map resource and current source bake agree exactly
on the grids and objects of all three tower floors. Map 55 has the
stairs at Bottom (5,5) and (5,7), with the east corridor at (5,6).
The matching FFIV native key movement producer uses its current pixel
position and the supplied input axis. Foot-event dispatch compares the
native player and trigger positions. The log records only the rounded
route cell and map transition, so it cannot establish the failing
movement's axis or exact destination. Retain the native stairs in the
bake. The offline production planner passes both reported NPC routes
from the central landing and both arrival tiles. Every NPC route leaves
the landing east, and each intentional stair route ends on its selected
stair. The replay uses the reported actor identities and the baked home
positions.

November 6 owner clarification. A single East press after “5 East”
triggered the floor change. The stairs are ordinary transition tiles.
The release-and-retry workaround was withdrawn before installation
and removed from source. A supported detour belongs in the initial
route as ordinary counted directions.

The installed and source collision masks show a wall to the west of
the logged start, stairs to the north and south, and the corridor to
the east. Both reported NPCs are present on 2F. There is no alternate
same-floor first step in the verified layout. The native input
dispatcher reads directional keys into an axis. Its field listener
calls the handler named OnTouchPadCallback, which proposes the next
position. The native foot checks use positions and trigger state.
The retained logs record the planner's tile and the resulting map,
with no exact native departure or trigger identity for the failed
press. The prior off-center arithmetic reproduced a blocked neighbor
but did not reproduce a floor change. No route or bake correction is
supported yet. The owner declined live capture for this issue.
The restored planner fixture passes both NPC routes and both intentional
stairs. The installed FFIV DLL and baked resource match the verified
November 5 deployment. Evidence is in
`build_check/fabul_rollback_verified_resource.json` and
`build_check/browse_route_report.txt`.

November 6 successful gameplay replay. The current FFIV log at
65406.328–65416.046 follows Man (Blue) from the 1F stair arrival
through one North and the east corridor to the interaction position
on 2F. A single East step advanced from (5,6) to (6,6) without a
floor change. At (6,6), a request spoke “1 East, 1 South, 3 East,
1 North,” then a later request from that same tile spoke “4 East.”
At (7,6), a similar South detour coincided with “Interact: Boy
(Blue).” The direct path returned while the player was still on
the same tile. The boy is the room's one moving NPC in the route
log. This supports a changing live obstacle. It does not establish
the boy's exact tile or explain the earlier floor transfers.

The directions formatter groups consecutive adjacent steps only.
The observed detour and straight route came from separate planner
results. The planner rechecks saved route cells against the current
obstacles and stair transitions when the player requests directions.
Keep the successful stepped replay as gameplay evidence. The
intermittent floor change remains open.

## November 5 final source scope and delivery

The last 17 FFV identities cover Island Shrine, Western Valley, Black
Chocobo Forest and Moogle Village. Both focused fixtures pass, with 33
assertions over 17 route legs. The only production differences since the
Great Sea Trench checkpoint are eight corrected vent arrivals and three
corrected Western Valley flag operands. No hooks were added in either
slice. Island Shrine levers stay with ordinary object navigation while
the separate guide scope decision is pending. Tycoon treasury stays with
ordinary navigation, including the verified exit correction.
[Island Shrine](FF5_ISLAND_SHRINE_20261105.md) ·
[Story contacts](FF5_STORY_CONTACTS_20261105.md)

The all-six finite source inventory now has 1,812 logical decisions and
1,844 physical candidates, with zero unreviewed entries. The separate
keyword, party-condition and key-item checks cover 63 FFII, eight FFIII
and 18 FFIV callers. The functional review verifies 1,362 labeled
occurrences and 245 silent families. The source boundary scan rejected
zero files. The resource receipt records a 759,619,584-byte peak for the
all-six source verification tree. Combined build and isolated smoke are
separate from these checks.
[Semantic ledger](puzzle_semantic_review_20261101.json)

**Local delivery verified at 23:24 UTC.** All 281 canonical dependencies
built serially under the enforced 2 GB tree limit. The full offline suite
passed by resuming at each corrected fixture on the unchanged production
binary. The scanner's production correction keeps same-floor scripted
movement selectable as an Event or Object. Focused scanner coverage
passed 382 reviewed objects. Canonical fixtures retain exact same-floor
arrivals, recognize guarded automatic currents, preserve FFV desert
arrival indices, and model Mobliz after the native raft completion.

The isolated smoke host passed all 47 proxy exports and a forwarded
call. All 67 embedded resources matched source. Defender completed the
scan with zero matching detections for 22 files. Exclusions could not be
verified and no signing certificate was available. The closed-game
installer verified all 132 installed payload files against the scanned
manifest and confirmed 144 protected files unchanged. The installed
13,676,032-byte DLL has SHA-256
`cbaad8ed8f5328245b773eadbb5d1ef74b928ba1a0cc5bddf556bdd322a45260`.
Receipts and recovery files are under
`build_check/broader_audit_20261104/`. No game launched.

The owner directed local installation. Native startup readiness and
gameplay validation remain open. The existing bootstrap initializes
logging, settings and speech before observing an IL2CPP API call.
That observation does not establish that launch and resource preparation
have ended. [Bootstrap](../../src/dllmain.cpp) ·
[Runtime gate](../../src/core/runtime_gate.cpp)

**November 5 Great Sea Trench verified offline.** The guide covers all
eight switch states, early chest knockback recovery and the tablet passage.
The current-room projection keeps the chest available after the third
switch. Remote previews retain native reload behavior. Sixty assertions
and 18 route legs pass, including preservation of the ordinary side controls.
17 FFV discovery leads remain. [Evidence](FF5_GREAT_SEA_TRENCH_20261105.md).

**November 5 Death Valley verified offline.** The three native falls keep
their trap warning and resume on the actual post-battle tile. The final
rescue resolves to Boko's world pose. The exact copied boarding correction
preserves the active fall group. Ten route legs pass. 29 FFV discovery
leads remain. [Evidence](FF5_DEATH_VALLEY_20261105.md).

**November 5 Forest of Moore verified offline.** Seven route edges now use
the true playable arrivals after the fire and refuge scenes. Both ordinary
tree controls and the native refuge prerequisite pass the focused replay.
26 exact source dispositions and 119 canonical guard producers pass.
40 FFV discovery leads remain. [Evidence](FF5_FOREST_OF_MOORE_20261105.md).

**November 5 Kuza and Boko checkpoints verified.** All twelve independent
weapon choices retain their native tablet and pickup receipts. The four world
checkpoints are already restricted to Boko and accept any order. Production
hashes are preserved. 66 FFV discovery leads remain.
[Evidence](FF5_REWARDS_AND_LAP_20261105.md).

**November 5 ordinary FFV passages verified.** Bal, the pirate hideout,
Phantom Village, Ronka and the Rift use their actual local or story receipts.
The Bal basement door stays reachable before the passage behind it opens.
Ten source dispositions and ten focused assertions pass. 88 FFV discovery
leads remain. [Evidence](FF5_SHORT_PASSAGES_20261105.md).

**November 5 Walse jail verified offline.** The route preview now places the
native lock collider in the doorway. Local and remote routes respect the
closed cell and open after the native receipt. The opening contact remains
reachable. One exact state row changed and four ordinary interactions were
reviewed. 98 FFV discovery leads remain. [Evidence](FF5_WALSE_JAIL_20261105.md).

**November 5 Ship Graveyard verified offline.** Twelve exact native jump
approaches and landings repair ordinary routing. Shift+G follows the required
story contacts, main crossings, bridge chest and encounter. The optional
timed crossing gives a wait instruction. The connected replay passed 47
assertions over 26 route legs. All-six route and guide checks passed.
102 FFV discovery leads remain. [Evidence](FF5_SHIP_GRAVEYARD_20261105.md).

**November 5 ordinary FFV story review verified.** 47 identities and 49
full scripts cover ordinary scenes, pickups, confirmations and reward choices.
Walse crystal pickups can occur in any order, and the Moogle Village outfit
is one preparation action before a conversation. All 481 production hashes
match the prior checkpoint. 103 FFV discovery leads remain.
[Source review](FF5_ORDINARY_STORY_20261105.md).

**November 5 Karnak escape source and focused replay verified.** The timed
Shift+G guide reaches the courtyard through six stairs, resumes after battle,
and uses the three actual escape contacts. It finishes after the full native
world return. Exact music-only projection corrections make the native encounter
available to routing. The replay preserves the original pre-blast wall gate.
29 route assertions and 149 owner cases pass. The source pins 120 field
programs, one battle timer asset, and ten native methods. 1662 logical and
1694 physical identities are reviewed, with 150 FFV discovery leads remaining.
Combined build, installation and gameplay are pending.
[Karnak evidence](FF5_KARNAK_20261105.md).

**November 5 Istory Falls routing correction verified.** Six actual waterfall
transfers now end their walking leg at the current and resume from the native
landing. The optional timed chest reports Event required while the water runs.
The lower room uses one ordinary exit switch. No puzzle guide was added.
The source review qualifies 39 field programs and ten native movement methods.
56 assertions over 68 routes pass, including all eight spike layouts, both
waterfall states and unknown-state handling. Retained Exdeath, Desert and
all-six state and route replays pass. The audit has 1656 logical and 1688
physical identities reviewed. 156 FFV discovery leads remain. Combined build,
installation and gameplay are pending.
[Routing evidence](FF5_ISTORY_FALLS_20261105.md).

**November 5 Castle Exdeath source, focused checks and documentation Closed.**
The guide uses the actual stopped layout, safe retries, north reset, bridge
collapse and restoration, and native lower-floor returns. One exact missing
same-floor passage on map 400 now supports ordinary and guided routes. The
final checks pass 227 assertions over 109 physical legs, 159 native-owner
cases, retained Fork Tower ownership, and all-six state, router and guide
checks. The source review qualifies 43 field programs, five timing methods,
four placement methods and one animation asset. Four keys cover all 12
languages. Earlier native geometry, arrivals, state and guides are preserved.
The audit has 1640 logical and 1672 physical identities reviewed, with 172 FFV
discovery leads remaining. Combined build, installation and gameplay are pending.
[Topic](FF5_EXDEATH_20261105.md).

**November5 Fork Tower source/focused/documentation Closed:**coordinated
assignment,twin climbs,paired encounters,exact timed final spell and full world
return.47 complete field programs/three battle assets/four native placement
functions;238 assertions/110 physical legs/85 entry points and actual owner160
pass. All-six state/router/entry and105 canonical/four guards pass. Four exact
scene arcs corrected;five pre142 pushbacks block onward travel while contacts
remain reachable. Earlier guides,655 native maps,arrivals,translations and474
other production files preserved;zero hooks. Six keys/all12 locales,115 recipe
classifications.1621/1653 reviewed identities;191FFV discovery leads remain,
not191 puzzles. No Main/install/game/probe. Owner-scoped audit continues.
[Topic](FF5_FORK_TOWER_20261105.md).

**November5 conditional Tycoon floor-exit routing fixed:**a no-return
story branch no longer fabricates an immediate floor change;the door itself
stays reachable,and ordinary exit travel has no false trap warning.32 actual
planner assertions/all-six state/router checks pass. One exact derived-arc
guard/flag correction;native geometry/actors/masters/initializers and all
existing guides preserved. Treasury is an ordinary single switch:no new guide.
15 related ordinary source dispositions;1610/1642 identities reviewed,202FFV
discovery leads remain (not puzzle counts).102 canonical/four guards pass.
No Main/install/game/probe. Owner-narrowed complex puzzle/route audit continues.
[Routing evidence](FF5_TYCOON_20261105.md).

**Owner November5 scope clarification — governs remaining work:**actual
puzzles require at least two complex steps (required order/warp sequences,
safe-tile or detection constraints),not travel plus an ordinary switch press.
Simple one/two-switch room/passages stay Objects/navigation coverage and do
not create new guides or puzzle-completion milestones. Ask about a genuinely
puzzle-like fewer-step exception before adding it. Preserve existing working
guidance and Closed systems;this clarification does not authorize removing them.
The draft Tycoon single-switch guide and its completion extension were fully
withdrawn;production hashes match the preceding town checkpoint exactly.

Historical1595/1627 counts are reviewed mechanic identities,not puzzle counts.
The156 remaining FFV identities are a discovery pool,not156 required solutions.
Classify before deep solution work and concentrate on actual complex families
(Karnak escape and other source-proven complex sequences),plus other
genuine constrained mechanics. Ordinary unlocks already handled by pathfinding
need no further guide-writing pass. Correct Objects labels,interaction access,
unlock/connectivity and independent story/vehicle routing remain in scope.
Tycoon's conditional suite exit routing is corrected;the treasury is not a
puzzle. Full combined delivery is pending. See maintained
features.md#puzzle-scope-and-audit-classification.

**November5 ordinary town contacts source/documentation Closed:**19 exact
identities/14 root classes/18 full bodies,including complete Crescent timer,
tutorial ejection/initializers and Regole/Tule directional conversation cases.
Ordinary scenes,confirmations and gifts need no extra puzzle solution. Whole
production/all earlier decisions preserved;100 canonical/four guards and
functional/source/content checks pass.1595logical/1627physical reviewed;
217FFV remain. No Main/install/game/probe. Full audit/combined delivery active.
[Topic](FF5_TOWN_CONTACTS_20261105.md).

**November5 Gohn source/focused/documentation Closed:**eight search contacts
plus actual final collapse/full471-operation underground callback. Guide
honors either middle sighting and full61;one exact final landing corrected.
272 assertions/82 physical legs/nine states,current owner156,retained Desert
and all-six state/router/entry checks pass. Existing12-language text reused,
114 classifications,99 canonical/four guards. Terrain/native actors/masters/
initializers/earlier guides/64 arrival indices preserved;zero new hooks.
1576logical/1608physical reviewed;236FFV remain. No Main/install/game/probe.
Full audit and combined delivery remain active.
[Topic](FF5_GOHN_20261105.md).

**November5 Desert of Shifting Sands source/focused/documentation Closed:**
19 original controls plus274 previously missing current areas. Native current
directions,guarded story crossings,four NPC poses and full encounter/current/
south-exit guidance corrected.16797 assertions/1567 physical legs/current
owner156,retained owners and all-six state/router pass. Five keys/all12 locales,
113 classifications,97 canonical/four guards. Terrain/native actors/earlier
guides/arrivals/decisions preserved;zero new hooks.1568logical/1600physical,plus
independent Gogo and274 areas;244FFV remain. No Main/install/game/probe. Full
audit and combined delivery remain active.
[Topic](FF5_DESERT_20261105.md).

**November5 FFV piano source/documentation Closed:**18 exact identities/eight
locations/26 complete bodies and8192 actual-choice/proficiency cases. Full
Crescent bard/temporary receipts and125 story eligibility retained. Ordinary
automatic music confirmations have no additional puzzle input or map unlock.
Whole production/all previous guides unchanged;93 canonical/four guards pass.
1549logical/1581physical reviewed,plus independent Gogo;263FFV remain. No Main/
install/game/probe. Full audit and combined delivery continue.
[Topic](FF5_PIANOS_20261105.md).

**November5 Fire-Powered Ship source/focused/documentation Closed:**eight
passage controls/seven bits/exact rider sides and complete native reset/return.
4096 episodes/200 legal layouts/1029 assertions/224 physical legs/current owner157
and retained owners/entry/all-six state/router pass. Six exact moved barriers
corrected;one key/all12 locales,111 classifications,92 canonical/four guards.
All previous guides/catalogs/native terrain/arrivals/other-game production
preserved;zero new hooks.1531logical/1563physical,plus independent Gogo;281FFV
remain. No Main/install/game/probe. Full audit and combined delivery stay active.
[Topic](FF5_FIRE_SHIP_20261105.md).

**Active finite capability — FFV remaining map/story mechanics,November4:**
156 original/continuation identities. Review complete source/callback chains,
actual puzzle inputs,story/key/vehicle eligibility and native physical returns.
Add complete navigation and Other solutions where uncovered;preserve independent
toggles,Minimal/entry notices and all Closed designs. No game/live probe or early
installation. Full audit and final combined delivery remain active.

**November5 Gogo source/focused/documentation Closed:**full sunken-tower
air reset,second-choice/inaction battle and safe native622 return. Two exact
music-only projections/four full vine arrivals fixed.36 assertions/575 route
legs,current owner156,all retained owners/entry/all-six state/router and87
canonical/four guard paths pass. Three keys/all12 locales;110 classifications.
Earlier guides/catalogs/terrain/actors/masters preserved;zero new hooks.
1519logical/1551physical,plus one independent Gogo solution;293FFV remain.
No Main/install/game/probe. Full audit and combined delivery remain active.
[Topic](FF5_GOGO_20261104.md).

**November4 FFV ordinary early source/documentation Closed:**42 exact
interactions have complete root/callback/source dispositions. Five distinct
jump/bridge/moving-vehicle controls remain pending. Whole production/all-six
data and earlier decisions unchanged;82 canonical/four guard paths pass.
1518logical/1550physical;294FFV remain. No Main/install/game/probe. Full audit
and combined delivery remain active.
[Topic](FF5_ORDINARY_EARLY_20261104.md).

**November4 Istory source/focused/documentation Closed:**four actual flower
contacts/all saved subsets/full299,including native sheep-yard upper exit and
world re-entry.106 assertions/136 route legs,current owner163,retained
Pyramid189/Jachol172,all entry notices and81 canonical/four guard paths pass.
One key/all12 locales;109 classifications. Earlier guides/catalogs/geometry/
state/arrivals preserved;zero hooks.1476logical/1508physical;336FFV remain.
No Main/install/game/probe. Full audit and combined delivery remain active.
[Topic](FF5_ISTORY_GUIDE_20261104.md).

**November4 Jachol source/focused/documentation Closed:**complete wall/random
skull/chest-door/three-chest/full two-world climb guide. Fixed two exact timer
initializer projections;native live pose stays authoritative.50 route/model
assertions,current owner172,retained Pyramid/five-leaf189,entry notices,all-six
state/router and78 canonical/four guard paths pass. Six keys/all12 locales;108
classifications. Geometry/old guides/catalogs/arrivals preserved.1471logical/
1503physical;341FFV remain. No Main/install/game/probe. Full audit/delivery active.
[Topic](FF5_JACHOL_GUIDE_20261104.md).

**November4 FFVI continuation supplement source/documentation Closed:**85
additional exact identities/176 full scripts/1486 actor facts;all42 save tutorial
companions verified. Accepted43 guides preserve actual choice/reset/party
semantics;the separate Lethe leaf supplied the missing raft solution. Complete
FFVI semantic/source census and74 canonical/four guard paths pass. Production
unchanged.1465logical/1497physical;347FFV remain. No Main/install/game/probe.
Full audit and combined delivery remain active.
[Topic](FF6_CONTINUATION_REVIEW_20261104.md).

**November4 Lethe River source/focused/documentation Closed:**complete raft/
first-Straight/second-Left/boss/full ready50 return guide. Three new instructions
all12 locales;43 FFVI guides/107 classifications.14 route assertions,current
native-owner,727 guide identities,entry notices and73 canonical/four guard
paths pass. Earlier guides/catalogs/geometry/state/arrivals preserved;zero new
hooks.1380logical/1412physical;432 remain (347V,85VI). No Main/install/game/probe.
Full audit and combined delivery remain active.
[Topic](LETHE_RIVER_20261104.md).

**Surgate continuation identities source/documentation Closed,November4:**25
additional exact shelf contacts (eight correct/17 wrong) match the accepted
54-program/302-object native archive,17 required comparison bodies and14 writer
assets. Model,author,route fixture and primary/comparison archive hashes match
the frozen148-episode/974-operation/419-contact/27-wrong-choice/28-reset proof.
Current three-book/eight-correct-contact resources and all12 locales pass;
Other ownership and current shared-owner regression are preserved. Durable
factual fingerprints avoid a canonical dependency on ignored reports. All
production source/data unchanged;70 canonical producers/four guard paths pass.
1377logical/1409physical;435 remain (347V,88VI). No reopened design,new guide,
qualification loop,Main/game/probe/install or actual responsiveness claim.
Full audit and combined delivery remain active.
[Accepted behavior and new identity review](FF5_SURGATE_BOOKS_20261102.md).

**Continuation supplement II–IV source/documentation Closed,November4:**eleven
new exact identities/15 complete scripts/238 actor facts. Native Minwu keyword
interaction,recruitment and automatic III key handover retain their accepted
semantics. Sea/air taxi choices,Leviathan's full Ship return,and the existing
optional Yang item exchange are ordinary native transport/story/quest systems.
All II60/III158/IV113 identity dispositions and keyword63/condition8/item18
censuses pass exact source checks. Full production source and II–IV data hashes
are unchanged;69 canonical producers/four guard paths pass.1352logical/
1384physical reviewed;460 remain (372V,88VI). No new guide/hook/route/gate/input,
Main DLL/game/probe/install or responsiveness claim. Full audit/combined delivery
remain active. [Evidence and boundaries](CONTINUATION_PUZZLE_REVIEW_20261104.md).

**Pyramid of Moore source/focused/documentation Closed, November4:**complete
42-target guide covers native needle101/reset returns,reachable snake switches
and live enemies,coffin436/325,northern door open/close,live versus reloaded
quicksand391,side-room recovery,timed invariant holds,wrong eastern upper-room
recovery and the full seven-map tablet return137. Flagless corridor observes
one/three exact controls;all other new actor observation is one conditional
third-switch lookup. Two exact original-first scalar phase hooks cancel stale
crossings/previews/autowalk;unknown partial advice publishes no targets.
70 physical-route/model assertions and current native-owner cases pass,retaining
all four earlier FFV leaves. Eight keys/all12 locales;106 global classifications.
Six exact initializer programs,one trap classification/full8,17 Ground landing
and one graph-only arrival change. Terrain/native actors/masters/all prior
guides/catalogs/state metadata/old arrival indices preserved. All-six state,
router,entry notices,1362 functional occurrences/245 source closures and68
canonical producers/four guard paths pass. The old functional checker now
checks source-qualified conditional exits and the one exact trap correction.
Failed investigation/fixture receipts are retained. No Main DLL/install/game/
probe or actual audible/frame timing claim.1341logical/1373physical reviewed
across original inventories plus159 new continuation/choice leads.471 remain:
9II,1III,1IV,372V,88VI. Full audit and combined delivery remain active.
[Source,ownership and verification](FF5_PYRAMID_20261104.md).

**Mt. Kolts source/focused/documentation Closed, November4:**the last two
FFVI discovery controls now have a complete mountain/sighting40/actual Vargas/
mandatory Blitz/Raging Fist Left-Right-Left-Confirm/full ready41 guide. Other
owns the combat solution independently from Navigation. A real route failure
was fixed:only the native40/not41 initializer projects the boss from its off-map
Upper home to23,32 Ground. Existing opcode10 accepts one optional bounded layer;
earlier six-field records,VM bounds and native owners are unchanged.51 exact
field scripts/164 actors/three battle-AI assets/three placement functions.
42 FFVI guides/105 classifications,one key/all12 locales.29 route/model
assertions,724 guide identities,full native-owner and all-six state/router pass.
All41 prior guides/full catalogs/geometry/native actors/masters/arrivals and
unrelated state preserved;one exact initializer pose changes. Complete FFVI
semantic/source census and63 canonical producers/four guard paths pass. Failed
spawn/fixture evidence retained. No new native query/hook/timer/input or live
responsiveness claim. No Main DLL/install/game/probe.1303logical/1335physical;
350FFV pending;FFVI discovery review has no missing entries. Broader audit and
final combined delivery remain active.
[Topic](MT_KOLTS_VARGAS_20261104.md).

**Later Opera source/focused/documentation Closed, November4:**five exact
controls;separate complete later operator/local2/correct lever3/boss/full ready
outside434 guide. Earlier performance and rightmost-lever solution are intact.
Wrong-lever recovery uses actual117,27/local2. Only completed434 enables the
manual repeat exit to full native66,42 Ground/baked26,20,not the intermediate
boss arena. Exact Move2E98E0 proves third operand is speed;SetPos2ECBA0 proves
layer. Physical lever and normal foot exits remain valid.41 FFVI guides/104
classifications,one key/all12 locales.30 production-route assertions,721 guide
identities,current native-owner and all-six state/router pass.55 exact scripts/
130 actors/two native functions;all40 prior guides/full catalog,638 geometries/
native actors/masters/original points/old arrivals/unrelated state preserved.
One manual arc/one graph-only arrival;finite guide limit48 rejects49. Exact
manual census114/89 and60 canonical producers/four guard paths pass. Failed
fixture/census evidence retained. No new query/hook/poll/timer/input/VM bound.
No Main DLL/install/game/probe.1301logical/1333physical;350V+2VI=352 pending.
Broader audit and final combined delivery remain active.
[Topic](LATE_OPERA_DRAGON_20261104.md).

**Cave on the Veldt source/focused/documentation Closed, November4:**two
exact controls now have complete Interceptor371/rock533/rescue/battle/voyage/
dream/full inn-return guidance for both Shadow372 and Relm373. Unrelated926
visit is not completion. One strictly selected-rescue-qualified repeat arc
prevents the initial uncontrollable scene ending311 from masquerading as free
travel to intermediate491. Its actual Upper PointIn3 and physical trigger stay
intact. Optional trapped chest stays optional.40 FFVI guides/103 classifications,
one key/all12 locales.50 production-route assertions,718 guide identities,both
native-owner full-return branches,current all-six state/router and Cyan39 pass.
124 exact scripts/444 actors;all38 earlier guides/full catalogs,638 geometries/
native objects/masters/positions/arrivals/unrelated state preserved. Prior
Jidoor evidence restored byte-identically after context-writer naming error;
wrong initial switch receipt caught and fixed before delivery.57 canonical
producers/four guard paths pass. No new native query/hook/poll/timer/input/VM
bound. No Main DLL/install/game/probe.1296logical/1328physical;350V+7VI=357 pending.
Broader audit and final combined delivery remain active.
[Topic](VELDT_CAVE_GUIDANCE_20261104.md).

**Remaining Zozo/Jidoor source/focused/documentation Closed, November4:**
seven exact controls;real animated-pickup availability defect corrected. After
Ramuh150,unrelated optional party checks formerly discarded guaranteed groups
for the remaining three magicites. The exact room’s existing-VM tail preserves
independent149/150–153 receipts and marks only six party NPCs uncertain. No new
query,hook,VM opcode or bound.155 assertions cover all16 pickup masks,conservative
NPCs,four complete remote journeys,early denial and the earlier lamp. That
scriptless611 artwork is actual Talk/default0/literal description;four exact
native functions distinguish it from the accepted later442 light switch. All
nine auction bodies/eleven variants are ordinary visible optional bid shopping,
with no missing map/story solution or travel gate.166 exact scripts/755 actors;
all38 guides/101 classifications/full catalogs/638 geometries/native objects/
masters/arrivals/unrelated state preserved. Current all-six state/router,Cyan39
and712 guide identities pass;54 canonical producers/four guard paths pass.
No Main DLL/install/game/probe.1294logical/1326physical;350V+9VI=359 pending.
Broader audit and final combined delivery remain active.
[Topic](ZOZO_JIDOOR_REMAINDER_20261104.md).

**Cyan/Zozo source/focused/documentation Closed, November4:**seven exact
discoveries;three complete letter/retry/delivery,purchase/manual-door/Cyan/key/
chest and optional-dragon guides. The native merchant requires actual359;
declined529 is not accepted358. Owned Rust Rid361 validates the onward journey,
which stops at the physical door;unknown/absent361 cannot. Door/chest fronts
remain approachable. One strictly365-qualified repeat summit arc fixes the
real missing key route without turning the initial reunion into free travel.
Actual full359/367/364 complete their sequences. Existing moving-dragon route
poses are reused.38 FFVI guides/101 classifications,four keys/all12 locales.
39 production-route assertions,712 guide identities,current owner and all-six
state/router,Narshe/FFIII/FFI lock consumers pass.229 exact scripts/622 facts;
all35 earlier guides/full catalog,638 geometries/native objects/masters/old
arrivals/unrelated state preserved.52 canonical producers/four guard paths
pass. No new native getter/hook/polling/timer/input. No Main DLL/install/game/
probe.1287logical/1319physical;350V+16VI=366 pending. Broader audit and final
combined delivery remain active.
[Topic](CYAN_ZOZO_GUIDANCE_20261104.md).

**Tzen rescue source/focused/documentation Closed, November4:**four exact
discoveries now have their own complete quake/start/six-minute timer/house/
basement/child311/return/full312 guide. Thamasa's accepted burning_house remains
unchanged. The warning precedes native timer activation;ordinary retreat
neither restarts the timer nor finishes the quest. Only full ready312 after
the rescue scene and Sabin joining completes guidance.35 FFVI guides/98 purpose
classifications,two keys/all12 locales.18 full production-route assertions,
700 guide identities and current native-owner completion checks pass.34 exact
scripts/71 actors;all34 previous guides/full catalog,geometry/native actors/
masters/map-state/arrivals/native timer behavior preserved. Current unchanged
all-six state/router evidence reused;49 canonical producers/four guard paths
pass. No new reader/hook/polling/clock/input. No Main DLL/install/game/probe or
live timed-run claim.1280logical/1312physical;350V+23VI=373 pending.
Broader audit/final combined delivery remain active.
[Topic](TZEN_RESCUE_20261104.md).

**FFVI ordinary story controls source/focused/documentation Closed, November4:**
19 inspected/18 completed across Cabin,Kohlingen,free-order scenario selection,
Esper elder,forest recovery,Darill entrance and Jidoor introductions.95 exact
connected bodies distinguish ordinary shop/party/dialogue from real puzzles.
Jidoor119 introduction returns to611;the letter is not a travel edge. Scenario
NPCs likewise are not automatic exits;Banon's complete three-battle chain ends
at World1/34. No mod source/data/guide/gate/geometry/actor/arrival/catalog byte
changes. Exact current Yeti consumer evidence reused;46 canonical producers/
four guard paths pass. One real finding remains pending:Tzen's child434/9 is
outside Thamasa's burning_house maps412–420 and needs a complete timed-rescue
guide with Tzen's three markers. No false completion for that child. No Main
DLL/install/game/probe.1276logical/1308physical;350V+27VI=377 pending.
Broader audit/final combined delivery remain active.
[Topic](ORDINARY_FF6_CONTROLS_20261104.md).

**Yeti's Cave source/focused/documentation Closed, November4:**three exact
discoveries;complete late-temple/jump/cave/correct-switch/magicite-choice/battle/
Mog11 recruitment398 guide. Navigation and Other preferences remain separate;
34 FFVI guides/97 classifications,four keys/all12 locales. Ordinary conversation
stays reachable without Mog. One scalar reuses the existing role pass with no
extra getter/hook/timer. All seven full fall animations match their original
native PointIns;positions/layers are preserved. Only two examined-switch graph
companions become manual;wrong-switch and four automatic-pit recoveries pass.
24 route assertions,13 actual Mog-reader assertions,Cyan integration,695 guide
identities,current owner controller/final398 and all-six state/router pass.
87 scripts/168 actors,638 geometries/native objects/masters,all map state,
previous arrivals,all33 older guides/full catalog preserved. Exact manual
census113/89.45 canonical producers/four guard paths pass. Initial wrong-layer
fixture retained;unfinished synthetic falls removed before delivery. No Main
DLL/install/game/probe.1258logical/1290physical;350V+45VI=395 pending.
Broader audit/final combined delivery remain active.
[Topic](YETI_CAVE_20261104.md).

**Dreamscape/Cyan source/focused/documentation Closed, November4:**twelve
discoveries across Dreamscape/Doma and the final pickup;three new surrounding
guides preserve the accepted train. Actual Cyan3/four-member entry,northward
bed approach/full22,54 landing (baked22,21),both real437/439 joins,brothers,
armor-loop450,full Doma451,family/possession battle,waking455 and final456 pickup
are covered. Optional452/453 memories never become gates. Exact temporary
puppet-alias projection fixes phantom obstacles/unavailable remote encounters;
unknown party state cannot certify entry. One scalar reuses the existing role
pass with no extra getter/hook/timer.33 FFVI guides/96 purpose classifications,
three keys/all12 locales.34 route assertions,13 actual reader assertions,
690 guide identities,current owner controller and all-six state/router pass.
95 scripts/300 actors/seven resolved battle assets/12 native functions;638
geometries/native actors/masters,all unrelated state,previous arrivals,all30
earlier guides/full prior catalog preserved. Canonical-check failure-only
nesting repaired:42 producers/four success-failure paths pass;full canonical
delivery still pending. Final evidence/binaries/failed fixtures retained. No
Main DLL/install/game/probe.1255logical/1287physical;350V+48VI=398 pending.
Broader audit and final combined delivery remain active.
[Topic](DREAM_EPISODE_20261104.md).

**Solitary Island source/focused/documentation Closed, November4:**five exact
discoveries;complete thirtieth FFVI guide uses actual beneficial fish/held-food/
reroll receipts,correct Cid variants,both outcomes/cliff bird/hidden passage/
manual raft/full ready306. Fishing is Other and later navigation is Navigation;
93 classifications,four keys/all12 locales. No hidden HP read/announcement or
new getter/hook/timer/input. Existing outcome gate is preserved. One animated
raft companion now reaches actual World2 cell146,212 without a false trap;
shared World2 ownership yields to the accepted next guide after306.36 actual
route assertions include all three later animated-pickup states;669 guide
identities,current native-owner controller and all-six state/router checks pass.
44 scripts/93 physical facts,638 geometries/native objects/masters/all state/
previous arrivals/29 guides/full prior catalog preserved. Final reports/binaries
and failed fixtures retained. Exact manual census111/89. No Main DLL/install/
game/probe.1243logical/1275physical;350V+60VI=410 pending. Combined delivery/
gameplay pending. [Topic](SOLITARY_ISLAND_GUIDE_20261104.md).

**Imperial travel source/focused/documentation Closed, November4:**sixteen
discoveries across Vector, Observation Post, Albrook and Blackjack. Two complete
guides (29 FFVI/92 purpose classifications),2 keys/all12 locales. Real Yes/crate/
factory396 arrival,6 strict194-selected factory variants,6 actual detection/
stationary-guard barriers; exact Terra/member initializer and20 blocked camp
identities retain repeat-party requirements until225. Albrook's local pushback
is avoided while its real alternate port route remains valid. Blackjack uses
actual saved craft4/world1 position, correct Fly option and native guest22,93;
the fallback Narshe edge is excluded. Ordinary coin/voyage/reunion stay separate.
120 route assertions,19 role-reader assertions,saved-reader/current controller,
659 guide identities and current all-six state/router checks pass.119 scripts/
nine exact native functions,638-room geometry/native-object/master/prior-arrival/
27-guide/full previous locale preservation pass. Failed fixtures and final
reports/executables retained. No Main DLL/install/game/probe.1238logical/
1270physical;350V+65VI=415 pending. Combined delivery/gameplay pending.
[Topic](IMPERIAL_TRAVEL_20261104.md).

**Esper Cave / Thamasa source/focused/documentation Closed, November4:**
eleven exact discoveries; complete27th FFVI guide covers sightings/statues,
native Ultros/Relm Sketch (Other), both pit-branch treasures and full Esper
return281. Three manual bridge arcs added; all previous pit landings preserved.
31 route assertions/29 legs including all three accessible bridge approaches,
649 story identities/current controller and all-six state/router checks pass.
119 native scripts/exact battle asset,one key/all12 locales/90 classifications,
638-room geometry/native-object/master/state/prior-arrival/26-guide/full prior
locale preservation pass. Optional Thamasa scenes and ordinary introductions/
Gung-Ho dialogue remain separate. Retained duplicate-author, newline-proof and
stale-census failures; exact routing census is now110 manual/89 directional.
Reports and executables are retained under the scope directory. No Main DLL/
install/game/probe.1222logical/1254physical;350V+81VI=431 pending.
Combined delivery/gameplay pending. [Topic](ESPER_CAVE_GUIDE_20261104.md).

**Crescent Mountain / Serpent Trench source/focused/documentation Closed,
November4:**eight exact discoveries; complete26th FFVI guide covers active
Gau12/129/helmet133, both timed Right branches, both treasures112/113, current-
area drain and full ready96/521. Five exact graph-only crossings now reach
playable arrivals with manual choice instructions; final native PointIn2 Upper
layer is preserved.28 route assertions/11 legs,15 role-reader assertions,
19 arrow/controller assertions,642 story identities and current all-six state/
router checks pass. Four keys/all12 locales,89 purpose classifications,29 native
scripts/current fork/UI ABI and638-room geometry/native-object/master/previous-
arrival/25-guide preservation pass. Retained repeat-gate, factual identity,
waypoint diagnostic and native-layer failures; no game/probe/Main DLL/install.
1211logical/1243physical;350V+92VI=442 pending. Combined delivery/gameplay pending.
[Topic](CRESCENT_SERPENT_GUIDE_20261104.md).

**Mobliz source/focused/documentation Closed, November4:**15 exact discoveries;
two complete guides for five letters/payments/actual shop-close replies/final517
and native353-selected hidden house/missed first-visit prerequisites/true area
exit-return/dog354/discovery355/full final356.35 real-route assertions/62 legs,
630 story identities/current controller/choice repeat pass; two keys/all12
locales,79 native sources, all23 old guides and terrain/objects/state/arrivals
preserved. Fixed only the new recipes' strict indoor versus non-strict world
return pairing; retained both failed fixtures. No new hook/native getter or
Main DLL/install/game/probe. Exact broader_audit_20261104/mobliz_completion.json.
1203logical/1235physical;350V+100VI=450 pending. Combined delivery/gameplay pending.
[Topic](MOBLIZ_PUZZLES_20261104.md).

**Mobliz — source/focused/documentation Closed:**15 remaining frozen discoveries.
Qualify mail/conversation/payment/story mechanics and connected native receipts,
then add any required complete guide or minimal fixes and affected checks.
Initial10–20min source/control decision. Sol only, same finite audit; no flight
performance investigation or game execution.
Mobliz source/control decision: two original guides are needed. Five letter
read/payment stages use absolute502..517; real reply producers are shop-close
callbacks0226_1..4 or inn rest, not a guessed elapsed timer. All five payments
use the first option and require the native paid receipt (insufficient funds
must not advance). Hidden-house navigation becomes available with actual
story353; it retains any missed316..320 first-visit prerequisites, current-area
reset/re-entry, dog354, discovery355 and full final356 native battle scene.
World2 ownership is selected only after353 and before356. Ordinary earlier
Terra conversations stay separate; no new native hook/party read is needed.
Raw connected sources remain external; factual mobliz_native_20261104.json.
Next: two complete recipes, current-choice repeat for exact88/10, all12 locales,
actual letter/secret-passage routes/state/native continuations, preservation
and the15 semantic reviews. No game/probe/capture or flight investigation.


Mobliz integration accepted:two complete recipes (25 total FFVI),88 purpose
classifications,two new keys/all12 locales,35 actual route assertions/62 legs,
630 identities and current controller pass. Both failed metadata pairings are
retained in the completion receipt; indoor legs now use strict/no-world, the
actual world return uses non-strict/world. All23 older recipes and unchanged
terrain/native objects/state/arrivals are verified. Fifteen semantic decisions
are Closed; no Main DLL/install/game/probe. User gameplay remains separate.


**Imperial Palace source/focused/documentation Closed, November4:**five exact
discoveries; twenty-third FFVI guide covers the240-second24-soldier tour, all
current banquet questions,120-second break battle, full257 arrival and final
ready258. Scoped native first-clock Select observer copies at most six32-char
keys/current Msg; no pointer retention, speech/queue/timer/native write in hook.
159 actual producer/controller assertions,36 real-route assertions/66 legs,
589 guide identities and retained Opera pass; current native ABI/eight nested
fields, all12 locales, source boundary and22 old guides/terrain/objects/state/
arrivals preserved. Exact broader_audit_20261104/palace_completion.json.
1188logical/1220physical;350V+115VI=465 pending. Combined delivery/gameplay pending.
[Topic](IMPERIAL_PALACE_GUIDE_20261104.md).

**Returners' Hideout source/documentation Closed, November4:**eight exact
ordinary conversation/choice identities and22 native producers. Three real
party talks43/44/45 precede exit46/Banon; first/second refusal544/545 do not
finish recruitment, whose full final callback owns47. Optional paper/sleep-talk
remain separate. No runtime/bake/guide change.1183logical/1215physical;
350V+120VI=470 pending. Exact broader_audit_20261104/returners_completion.json.
[Topic](RETURNERS_STORY_CONDITIONS_20261104.md).

**Duncan source/focused/documentation Closed, November4:**seven exact
discoveries. Optional kettle/shelf/flower clues are alternatives, not three
required operations. Later training now projects actual active Sabin6 and
unfinished464 through one exact missing initializer branch. Existing party
pass adds no native calls; reserve/unknown/complete states and route signatures
checked.15 actual owner-reader assertions,13 production-route assertions,
seven state assertions, old17-case Locke/Strago reader and current all-six
route suite pass. Other state/guides/geometry preserved.1175logical/1207physical;
350V+128VI=478 pending. Exact broader_audit_20261104/duncan_completion.json;
combined build/install/gameplay pending.
[Topic](DUNCAN_PARTY_GATE_20261104.md).

**FFVI rental-stable source/documentation Closed, November4:**seven exact
merchants/21 native producers; native first-option payment and full outside/
mounted-world arrivals qualified. No runtime/bake/guide change or free travel
arc.1168logical/1200physical;350V+135VI=485 pending. Exact evidence:
broader_audit_20261104/stables_completion.json.
[Topic](FF6_STABLE_RENTALS_20261104.md).

**Gerad source/focused/documentation Closed, November4:**18 exact remaining
Figaro/Nikeah/cave identities. Twenty-second FFVI guide covers four pub talks,
native575/576 stopped-actor receipts, first/repeat ferries, inn, both recurring
turtle banks, fleeing Siegfried, jail, Tentacles and final249→2→249 ready336.
Two exact manual arcs reuse the actual turtle/counters/layer and known332;
physical controls stay valid targets. No new hook/poll/native getter/input or
worker protocol.50 actual assertions/28 continuations,556 story identities,
controller/ferry/final-scene checks, all-six107 manual/86 directional routes
and all12 translations pass.638 rooms' terrain/native objects/masters, all old
initializers and21 older guides preserved. Exact failed attempts and source/
report/executable/resource hashes:broader_audit_20261104/gerad_completion.json.
1161logical/1193physical;350V+142VI=492 pending. Combined build/install/gameplay
remain pending.
[Topic](GERAD_PASSAGE_20261104.md).

**Puzzle entry notice source/focused/documentation Closed, November4:** owner
update retains “Puzzle guide active” on eligible map entry in Minimal and
Verbose. Shift+G stays directions-only in Minimal. Shared ready-event notice
does not activate/reroute; stale/same-map reload, actual FFVI selection and
all-six static checks pass (27 new assertions), plus all-six map state and
all12 translations. Combined build/install/gameplay pending; exact
broader_audit_20261104/puzzle_entry_completion.json. Audit counts unchanged:
1143logical/1175physical;350V+160VI=510 pending. Return to active Figaro/Gerad.
[Topic](PUZZLE_VERBOSITY_20261103.md).

## Current milestone and scope ledger

**Puzzle verbosity source/focused/documentation Closed:** all-six F8 >
Navigation, Minimal default/Verbose opt-in. Activation/step metadata optional;
directions/actions/choices/timing and lifecycle preserved. Shared Trap required /
Event required notices are short in both modes. Six actual producer/settings/
route targets and12-language keys pass. No hook/native capture/worker API added.
The old broad manual-arc census was reconciled to the already Closed Darill3 +
Sealed Gate1 additions; all approach/landing checks remain. Exact source/report/
executable/resource identities and failed fixture expectations retained in
puzzle_verbosity_20261103/completion.json. No DLL/install/game/probe/agent;
combined release/security/delivery and natural audible timing remain pending.
Continue the current finite audit capability below.

**Ebot’s Rock source/focused/documentation Closed, November4:** twentieth guide
uses actual current-room coral/chests/layout, the native22 threshold, active
Strago first-feed check, correct keep/feed choices and full first/repeat Hidon
completion. Fresh random warps have12 real outcomes; first entry stays
deterministic. Normal routes label random attempts and retire on actual arrival
without consuming the pad. No terrain/native object/other-room change, new hook
or native getter call. Actual152 route/state assertions,27 pocket/layout cases,
442 guide identities/current controller, role readers17/18/37 and all-six route/
Magitek regressions pass. Five keys/all12 locales. Exact source/report/resource
identities: broader_audit_20261104/ebot_completion.json.1117logical/1149physical;
350V+186VI=536 pending. No Main DLL/install/game/probe/agent; combined release/
delivery and natural gameplay remain required.
[Topic](EBOTS_ROCK_GUIDANCE_20261104.md).

**Keyboard remapping source/focused/documentation Closed, November4:** existing
per-game catalog/editor now checks new gameplay assignments against current
native keys/modifiers, preserves rejected values, and uses the effective
recovery/capture-cancel keys. Separate custom navigation profiles and inactive-
profile conflicts pass; existing default sharing/context precedence is retained.
Five battle-only party slots/native2/3, text/focus/capture/controller independence
and all ten native input paths pass. Held remapped keys stay owned through
modifier/context handoff until release. No new hook, worker or per-frame native
configuration read. Exact all-six native evidence,109 key mappings,8403 names/
zero missing,43 reader/chord assertions and all five actual input/settings targets
pass. Eight keys/all12 locales. Combined release/install/gameplay pending; exact
broader_audit_20261104/keyboard_completion.json. Audit remains1117logical/
1149physical,350V+186VI=536 pending. Return to the finite remaining map audit.
[Topic](KEYBOARD_REMAPPING_20261104.md).

**South Figaro source/focused/documentation Closed, November4:**26 original
town/cave identities; twenty-first guide covers merchant/soldier/cider Steal,
boy/password retries, draft, costume removal, Celes/key/clock, native world/cave
aliases,582→583→63 warnings and full Tunnel Armor64 completion. Two camera-only
passage destinations corrected;638 rooms' terrain/native objects/masters/state
and20 earlier guides preserved. Only this ordered recipe opts into first-match
ownership. No new hook/native read/worker API.34 actual assertions/43 native
continuations (31 normal),535 guide identities, six choices/both scene branches,
all-six route suite and five keys/all12 locales pass. Failed fixtures and the
active-script runner mistake are retained; fixed-script rerun passes. Exact
broader_audit_20261104/south_figaro_completion.json.1143logical/1175physical;
350V+160VI=510 pending. Combined release/install/gameplay remain pending.
[Topic](SOUTH_FIGARO_INFILTRATION_20261104.md).

**Owner update, November4 — puzzle-type toggles source/focused/documentation Closed.**
Keep auditing/building solutions for all puzzle mechanics, including choices
and other non-navigation game mechanics. The clarification does not narrow the
audit. Add persistent Navigation puzzle guidance and Non-navigation puzzle
guidance toggles to F8; both default on, preserving the current enabled behavior.
Mixed sequences must qualify the relevant step (Opera lyrics versus catwalks,
Palace banquet versus timed walking), not disable the whole mixed episode.
Each disabled kind must withhold its solution/automatic continuation and its
availability notice where no enabled kind remains. Ordinary UI access and
ordinary pathfinding retain their own settings. Minimal map-entry notice stays
enabled for eligible enabled kinds. This owner request permits this focused
extension of the previously Closed verbosity presentation; no other Closed
design is reopened. Initial10–15min inventory/control decision, then the full
source/localization/lifecycle/settings and affected-consumer checks. Sol only.
Completed:85 explicitly classified recipes/native policies, mixed-stage policy,
owned worker type masks, quiet guide-only cancellation and current native-choice
output. Seven actual-consumer targets, all12 translations and source-boundary
checks pass. Stale Opera fixture duplicate definitions were corrected to reuse
current shared fake-native helpers; failed build retained. Exact source/report/
executable/resource receipt: broader_audit_20261104/puzzle_types_completion.json.
No Main DLL/install/game; counts unchanged1183/1215,470 pending. Resume Palace.

**Imperial Palace dinner — Closed at the accepted source/focused boundary:**five remaining
identities. Qualify the timed soldier/banquet mechanics, actual talk/battle and
choice outcomes, then the required complete guidance/fixes/checks/documentation.
Initial15–25min source/control decision; one active capability, Sol only.
No flight-performance investigation is active. Type-toggle integration is
Closed; this source/control checkpoint resumes without repeating discovery.

Palace source/control decision: native tour is240s,24 actual soldier receipts
230..253 across165..179 (including the adjacent observation rooms). Finish-tour
regions176/16,17 can end it early; banquet then has actual scored option IDs,
three distinct question locals, remembered first question local4, optional
120s break battle256, resume257 and final reward258. The repeated question menu
uses the same IDs for asking and remembering: actual preceding Msg ID must
distinguish them. All90 connected sources/38 physical controls/16 choice sites
are indexed in palace_native_20261104.json; raw bodies stay external.
Matching native External.Message.Select B63E60 inspects Core.IsFirstClock
(current program-counter clockCount1), then consumes the current mnemonic's
six bounded string operands. Core.get_pc/GetNextMnemonic and pointer equality
allow a bounded preceding-Msg observation for empty Yes/No and reused menus.
Native symbol/layout/body provenance is external in native_select/native_core.
Source/control decision is complete. Next concrete integration: twenty-third
guide with conditional unvisited targets; narrowly scoped first-clock choice
observer/current question advice; full timed/battle/arrival, old-guide,
ownership/performance, localization and preservation checks. No native writes,
timer alteration, actor scan or game execution is authorized or needed.

Palace implementation accepted: conditional per-target predicates preserve all
older targets; the full recipe and scoped current-question observer are wired,
built and focused-verified.159 producer/controller assertions,36 real-route
assertions/66 legs/24 soldiers, all16 current-question sites, final-ready258,
native nested bindings,12-language catalog and old source/data preservation
pass. Exact Closed receipt above replaces the earlier unwired checkpoint.
The existing 32-guide/128-rule/32-target bounds remain; no native timer/input
write, capture/probe/game launch or worker pointer was introduced.

**Magitek Factory source/focused/documentation Closed, November4:**27 exact
Factory/Research identities reviewed. Eleven native ladder/conveyor links
(ten manual/one automatic) restore missing lower/upper routes on396; no absent
46/31 operation is granted on643. Exact22-object addition preserves all terrain,
old objects/other rooms/initializers/19 story/five static guides. Normal same-floor
manual directions now retain the planner's exact receipt beside the control;
actual start and exact landing own completion. No new hook/native read/write/
queue/search or VM design. Hidden passage397/PointIn11 already matches17,28.
Actual11 arcs/11 approaches/50 assertions, all-six routing105 manual/84 directional
and trap regressions pass. Stale long-warning fixture expectations corrected;
actual concise warning policy retained. Source/semantic/canonical/docs/evidence:
broader_audit_20261104/magitek_completion.json;
[topic](MAGITEK_PASSAGES_20261104.md).1114logical/1146physical reviewed;
350V+189VI=539 pending. No Main DLL/install/game/probe/agent. Combined release/
delivery and natural gameplay remain required.

**Imperial Camp/Baren Falls source/semantic/focused/documentation Closed:**78
exact original identities classified without runtime/guide/navigation-data change.
Thirty camp mask counterparts retain the Closed native mask design; current actual
mask consumer passes both modes/Hide21/unknown locals/three real dog poses. Six
other camp interactions are ordinary native chest/scene/story-exit operations.
Forty-two waterfall conversation/jump/reset counterparts retain first battle80
and victory376/0218_2/125, distinct from later world1/PointIn37 travel. Both current
source initializers match accepted data; no early return warp or vehicle check
introduced. Native source versions match; previous decisions/all19 story/five
static guides/terrain/state/arrivals preserved.1087logical/1119physical reviewed;
350V+216VI=566 pending. Exact source/report/resource identities, missing-helper
failure and proof: sabin_scenario_20261103/completion.json;
[topic](SABIN_SCENARIO_REVIEW_20261103.md). No Main DLL/install/game/probe/agent.
Combined release/delivery and natural gameplay remain pending.

**Remaining Narshe interactions source/focused/documentation Closed:** all43
exact original identities now have explicit dispositions:20 chase objects and23
ordinary/native-scene interactions. The19th story guide covers native treasure
room208, empty chest209, chase210..213, motionless native214 wait and Mog rescue215;
alternate216 retires rescue guidance. Exact map18 entry handles remote-local
positions; all intermediate mine floors and earlier defense19 remain covered.
Actual full journey16 assertions/11 continuations, current+older model identities
and actual controller pass. Earlier defense selection and all18 old story/five
static guides, native geometry/state/arrivals and catalog values are preserved.
Four instructions/all12 languages; canonical author/locale/semantic/route checks.
No native hook/read/write or mod timer; only owned state/cell predicates. Closed
light320 retains15 stages/recovery/ready317. Coverage1009logical/1041physical;
350V+294VI=644 pending. Failures/exact source/output/resource identities retained
in narshe_interactions_20261103/completion.json; [topic](NARSHE_CHASE_GUIDANCE_20261103.md).
No Main DLL/install/game/probe/agent; combined release/security/delivery and natural
gameplay remain required. Closed designs remain Closed.

**Narshe party locks source/focused/documentation Closed:** six exact native
doors115–119/114 require active canonical Locke2, not owned reserve. Door fronts
remain reachable; behind-door goals require the ability or opened411–416.
Confirmed ability stops directions at the still-solid door for manual unlocking.
Native flag353 enables the lock group. One hash-qualified visual initializer
row21 is a private projection no-op; native visuals and unsupported86 remain.
Bounded owner capture adds only an owned optional boolean; no clone/script/write,
hook/polling or worker native read. Actual reader18 cases, six-door route/browse93
assertions, unchanged FFIII reader37/routes246859 assertions/203 journeys and
FFI all-map143 routes/Cornelia277 checks pass. Readers84.993s/331444224B; final
four-target regression205.219s/486232064B,2GB/25%CPU/one worker,exit0/drained.
One concise prompt/all12 languages; canonical authors/actual checks wired.
Six exact original discoveries resolved; all earlier decisions/guide recipes/
geometry retained.966logical/998physical reviewed,350V+337VI=687 pending.
Source/report/resource/native identities and failed fixtures are retained in
narshe_gates_20261103/completion.json; [topic](NARSHE_PARTY_LOCKS_20261103.md).
No Main DLL/install/game/probe/agent. Combined release/delivery and gameplay remain
pending. Closed light guide320/15 stages/recovery/ready317 remains intact.

**Floating Continent source/focused/documentation Closed:** complete normal
and timed-escape guide on431/631 plus save floor432, all four treasures, native
stay/wait choices and exact final-ready191 continuation. The alternate collapse299
now explicitly clears encounter18/area7 before returning to the safe treasure
route. Eight actual guide/planner cases/66 actions,382 identities/current+older
models and actual choice/completion controller pass. Four exact initializer rows
and12 native automatic arcs/12 graph arrivals corrected; native objects/geometry/
masters/other rooms retained. All17 earlier story/five static guides, old catalog
and semantic decisions preserved; actual18th guide stays inside32. Three new
instructions/all12 languages.69 original identities/69 physical instances resolved:
960logical/992physical reviewed,350V+343VI=693 pending. Exact source/report hashes,
failures, preservation and finite resource receipt: floating_continent_guide_20261103/
completion.json; topic [Floating Continent](FLOATING_CONTINENT_GUIDANCE_20261103.md).
No Main DLL/install/game/probe/agent; full combined release/security/delivery and
natural gameplay remain pending. Closed designs stay Closed.

Owner gameplay confirmation, November3: **FFI is confirmed beatable with the
current mod**, superseding the earlier tentative completion using the old mod.
FFII/FFIII remain confirmed100% playable with mod features and map bakes. This
is completion/progression confirmation, separate from remaining individual bugs.

**FF1 initial item target source/offline/FF1 delivery Closed:** native all-six
PlayerInit and VI side-entry proof identify TargetPlayerList as the current
eligible owner; private playerDataList is the older click cache. The minimal
reader correction preserves correct movement and self-target behavior. Thirty
new assertions fail before and pass after; actual battle_focus/party_esper pass,
93.682s/316248064B/exit0/drained/2GB/25%CPU. Actual shared-handle logs, native facts
and exact source/report closure: ff1_item_target_entry_20261103/completion.json;
topic ITEM_TARGET_ENTRY_20261103.md. Isolated FF1 Stage ports only this and the
raw hidden enemy condition correction onto confirmed-beatable b62; maps and
independent choice ownership remain unchanged. Canonical169 targets/178 reports,
8320 names0missing/67resources/12languages/47exports/22exact Defender files pass.
Installed only FF1 at9c2ba647d1e5408a/11625984B;22 hashes/144 protected files and
other five payloads verified under fresh game-closed guards.642.078s canonical,
654.272s qualification/797401088B/2GB/8.33%CPU/exit0/drained. Existing Stage cap
retained, no active job altered. Main delivery.json links exact receipts/recovery.
Natural target gameplay confirmation pending; no game/probe/agent/push. Cave resumes.

**Sealed Gate source/focused/documentation Closed:** complete405–411 guide,
eight short cardinal crossings using two native phase callbacks, safe waiting/
recovery, forced fall, door/stairs/floor caches/bridges/islands/chest/story225.
Exact native puppet layer2 means native10/Ground/baked1, fixing all16 earlier
landing failures without shared routing changes.37 physical journeys/six
continuations/313 identities and all five actual model/controller/guide/state/
route targets pass; prior16 story/five static guides and catalog values remain.
Validated32-guide bound for actual17th; four instructions/all12 languages.
Only six exact timer initializer rows and one manual arc/one synthetic arrival
change map data; all earlier semantic decisions preserved.69 original identities/
69 physical instances now explicitly reviewed:891logical/923physical covered,
350V+412VI=762 pending. Final integration58.770s/119754752B/exit0/drained/2GB/25%CPU;
corrected bake/routes112.427s/451936256B. Exact completion.json and native/failed/
preservation evidence in sealed_gate_20261103; topic SEALED_GATE_GUIDANCE_20261103.md.
No new Main DLL/install/game/probe/agent; combined release/gameplay still pending.

**User-required FF3 key integration verification Closed:** all seven actual
consumer targets pass against current shared collider0.37 native-reader cases,
13 doors/1853 cells/two-key topology,246859 assertions/215 chest profiles/65
door profiles/203 full journeys/61 manual unlocks/490 transitions. No missing-
key access, closed-lock crossing, key overspend or door-front/content regression
found. FF1 all-map key/277 Cornelia/locked approaches/126 trench-owned-ship cases
also pass. Production inputs unchanged; no FF3 implementation change or replay.
529.036s/485986304B/exit0/drained/shared2GB/25%CPU/one worker. Exact record
ff3_key_regression_20261103/checks_complete.json; topic FF3_KEY_REGRESSION_20261103.md.
Audit counts remain822logical/854physical,350V+481VI=831 pending.

**Enemy Regen visibility source/focused Closed:** actual FF1 Vampire Lord/Troll
logs demonstrated a separate raw condition Add/removal bypass, not an outdated
enemy_status.cpp installation. All-six raw enemy conditions now remain silent
except the explicitly approved optional Critical disclosure; party conditions,
actual visible enemy effects and native recovery digits retain their owners.
FF1's21 exact native functions establish ordinary recovery-number popups for
enemy Regen, distinct from the hidden intrinsic status label.216 assertions
failed before the fix and pass after; six actual production consumer targets,
all-six condition policies and labels pass.116.639s/328404992B/exit0/drained,
shared2GB/25%CPU/one worker. Exact source/report identities and native facts:
ff1_enemy_regen_20261103/completion.json; topic ENEMY_REGEN_VISIBILITY_20261103.md.
No new native DLL/install/game/probe/agent; FF1b62 unchanged. Combined final
build/smoke/security/guarded delivery remains required. Broader audit resumes
with822logical/854physical reviewed and831 V/VI candidates pending.

**Zozo clock source/focused Closed:** actual clock6:10:50/choices3,1,5 through
hidden chest115; old door-waypoint completion no longer ends the active guide.
Hint-floor approach, native wall visibility and inside-side reward resume use
existing interfaces; exact clock instruction repeats during native choices,
512 owned chars maximum, no flags/routes/workers on repeat. No new hook or shared
routing/key/vehicle/geometry change.12 real route cases/two continuations/282
identities/actual controller/input/all150 native choice combinations pass.
All15 earlier story/five static/catalog/old semantic values retained. Ten added
original dispositions:822logical/854physical covered;350V+481VI=831 pending.
Focused122.781s/444719104B; source18.481s/285908992B, exit0/drained/2GB/25%CPU.
Topic ZOZO_CLOCK_REWARD_20261103.md; source design Closed. No owned build/new
native DLL/install/game/probe/agent. The current16-guide bound is saturated;
additional actual guides must preserve an explicit finite validated capacity.

**Active next finite integration verification:** the owner's November3 FF3
door/key contract below, against the actual shared collider0 correction.
Initial15–20min source-matched no-key/Magic-Key/Thief/opened/prerequisite,
door-front versus behind-door, one/two consumed-key topology and complete
journey checks. Preserve accepted key owners; only an actual demonstrated
regression authorizes a minimal fix. No FF3 replay/game launch/probe required.

Owner reaffirmation, November3: the unlocking door itself always retains its
valid approach path. The destinations behind it own the story-gate label.
Actual opened state OR currently owned usable key/item/ability may validate
the contents' route, while earlier independent prerequisites remain enforced.
No missing-key shortcut, blanket re-gating of doors or early content access.
FF3 is gameplay-complete and must not require a replay to detect regressions.
After the active Zozo slice, explicitly rerun source-matched real-bake FF3
no-key/Magic-Key/Thief/unlocked/prerequisite and door-versus-content scenarios
against the shared collider integration correction. Check actual consumed-key
counts if a journey crosses multiple still-locked doors. This is the user's
required integration verification, not authorization to redesign Closed owners.
Apply the same contract to the broader all-six item/story/vehicle audit.

**Remaining census/cinematic cohort source/semantic Closed:**163 exact original
FFVI discoveries reconciled:161 non-interactive lights on movement-disabled
movie maps578/586, plus two already-indexed Opera world entrances. Fixed native
identities/properties/current object/script-master/map hashes and both baked
exit tuples checked. No area/name-based disposition rule, dropped instance,
production change or Closed redesign. All prior decisions and selected entire
production source/data inputs retained. cohort_completion.json in
remaining_census_20261103;1.526s/exit0/drained/2GB/25%CPU/shared admission.
Topic FF6_CINEMATIC_WORLD_COHORT_20261103.md.812logical/844physical reviewed;
350V+491VI=841 pending. No owned job/native output/install/game/probe/agent.

**Active next finite capability:** Zozo clock puzzle, native choice sequence
and actual hidden-reward route. Initial15–20min exact source/entrance/receipt
proof, reusing the existing static guide and accepted controller interfaces.
No speculative new hook, generic framework or Closed redesign. Scope other
Zozo story/reward records separately when the exact native facts establish it.

**Owzer's Mansion source/focused Closed:** full warning/diary/lights/paintings,
native timed left door, four hanging-chest encounters, final painting/boss and
bookshelf391. Late unfinished encounters remain guidable after391.33 physical
cases/eight complete continuations/279 identities and actual controller pass;
all14 earlier story/five static/catalog/semantic values and native geometry
retained. One exact one-second callback supplies a bounded native-owner cue
after preview and explicit readiness; no polling, timer change or input.
Native evidence in all six binaries proves enabled object-collider0 is full,
unlike terrain None0. The demonstrated shared integration fix retains partial
collisions/vehicle selectors; Chocobo/Owen and all-six selector checks pass.
Integration229.795s/465936384B, source14.985s/286130176B, final exact-pose
closure55.612s/452501504B, all exit0/drained/2GB/25%CPU/shared admission.
Exact checkpoint owzers_mansion_20261103/completion.json and topic
OWZERS_MANSION_20261103.md. Eight original discoveries added:649logical/
681physical reviewed;350V+654VI=1004 pending. Source design Closed. No owned
job/new native DLL/install/game/probe/agent. Final combined checks remain pending.

**Next finite capability:** bounded census of remaining FFV/VI candidates,
then one next complete unique map-puzzle family. Initial10–15min for exact
remaining identities/areas and reuse of source-matched earlier reviews; no
automatic catch-all dispositions, Closed redesign or speculative code work.
All prior installed FF1/FF4 and custom300ff canonical payloads remain unchanged.

**Cultists' Tower source/focused Closed:** switch421, hidden chest179, summit
423, forced battle424 and actual entrance476.24 physical cases/63 continuations/
227 identities and actual controller cases pass. One exact unknown-membership
initializer row fixes common392 barrier absence without a membership reader.
Owned finish_through list retains only five reviewed ready floors; default
wrong-exit cancellation, battle/stop/stale-output behavior and all prior guide/
roof/Darill cases retained. Thirteen story/five static/catalog/physical bake/
old semantic values preserved. Native source17 hashes; all-six static authors
and wiring pass. Repair42.589s/444583936B, integration45.952s/363900928B,
source7.321s/285573120B,exit0/drained/2GB/25%CPU. Exact identities:
cultists_tower_20261103/completion.json; topic CULTISTS_TOWER_20261103.md.
Nine exact discoveries added;641logical/673physical covered,350V+662VI=1012
pending. No owned job/new native DLL/install/game/probe/agent. Source design Closed.

**Active next finite capability:** Owzer's Mansion complete painting/door map
puzzle and its actual story endpoint. Initial20–30min source/state/actor and
production-route proof, then the smallest demonstrated guide/integration repair
through existing interfaces, focused model/controller/locale and semantic
closure. Preserve Closed source designs and earlier delivery evidence.

Owzer proof exceeded the initial slice after concrete timer/collider and
same-floor landing failures. Two exact initializer timer rows, four decorative
facing facts, three automatic landing arcs and one native return are now
qualified against matching source; native geometry and other floors retained.
Correct post-scene poses reach all four hanging-chest encounters, the final
painting and bookshelf. Continuing one bounded integration slice for the full
receipt-driven recipe and one native door-opening cue: no polling, timer change,
automatic input, unrelated Closed redesign or new worker. Cue observation is
needed because the true left doorway only opens for one second. Native ABI,
owner, focus/cancellation and stale-cue checks precede closure. Canonical DLL,
installation and gameplay remain separate; all earlier payloads unchanged.

**Darill's Tomb source/focused Closed:** full first gravestone/water, all four
inscriptions, verified native answer4/1/2/1, actual chest, second gravestone/
water/turtle, boss and moving memory-guide journey through ready world2.
Three same-floor manual turtle arcs and one actual return landing corrected;
all native objects/geometry/map-state/masters retained.28 physical/resume/
negative cases/44 continuations/222 model identities pass. Native-choice
repeat retains at most512 owned characters, exact destination/context only;
no flags/routes/workers on repeat. Actual controller/shared input and89 static
controls/52 recipes/all earlier roof cases pass. Three new keys/all12 languages,
12 old story/five static guide values and old semantic/catalog rows preserved.
Exact checkpoint darills_tomb_20261103/completion.json, topic DARILLS_TOMB_20261103.md.
Seventeen original discoveries reviewed;632logical/664physical covered,350V+
671VI=1021 pending. Native output/install/gameplay remain separate. No owned job,
game/probe/agent. Closed interfaces retained; no further internal refactoring.

The Cultists closure above supersedes its initial review/integration status.

**Ancient Castle source/focused Closed:** complete enabling statue428, hidden
switch429, final statue432; old518/14,32 construction waypoint is unreachable
and removed. Thirteen physical/resume/negative cases plus six real floor
continuations pass, including missing-prerequisite returns across Ground/Bottom.
Actual generic controller waits for native receipts/readiness, rejects stale
routes and resumes completed432 on all three floors.89 controls/52 recipes and
all-six references pass; both original87 census failures are preserved and fixed
only in the tests. Eight exact source bodies/one new key in12 locales. Twelve
story/four other static guides/catalog values retained. No runtime/bake change.
Final references42.101s/442626048B; source/semantic2.970s/284950528B; closure
receipt under1s, all exit0/drained/2GB/25%CPU/one worker. Exact identities:
ancient_castle_20261103/completion.json; topic ANCIENT_CASTLE_20261103.md.
615logical/647physical original records covered;350V+688VI=1038 remain.
No owned job/native payload/install/game/probe/agent. Source design Closed;
continue the next remaining unique-puzzle family.

The Darill closure above supersedes its initial review/integration status.

**Ordinary Phantom Train source/focused Closed:** eleven pre-engine stages,
retained1/0/1 engine, boss control and actual ready world1 completion.16 witnesses
plus seven intermediate-floor continuations pass23 production route plans;
21 new model cases/207 identities/27 source hashes and reused12-language keys
pass. Four exact first-car directions/landings and transfer-only109 guards are
corrected (ten bake bytes); two initializer rows retain the unconditional boss
group across unknown optional ghost-membership branches. Actual B5B680 jump
operand3 is statically verified against matching0029/38aa game/dump hashes.
No new runtime hook, membership read, VM opcode or native-thread work. Eleven
other guides/catalog/all geometry/old semantic rows are preserved.
Initial proof stayed within its20–30min evidence bound; full integration exceeded
that slice after two actual transfer/projection counterexamples. Reported scope
review retained minimal offline corrections, no architecture branch. Stage/model
62.866s/445837312B; full-continuation/controller61.661s/445775872B; final boss
53.274s/126930944B; source5.195s/495939584B; final frozen-data consumers and
preservation2.986s/277204992B, all exit0/drained/2GB/25%CPU/one worker. Exact completion:
phantom_train_20261103/completion.json; topic PHANTOM_TRAIN_JOURNEY_20261103.md.
Combined native build/install/gameplay remain pending. Source design Closed.
589logical/621physical covered;350V+714VI=1064 original candidates remain.
No owned build/game/probe/agent. Next finite remaining unique-puzzle review.

Earlier checkpoint notes below retain their original status; the current entry
above supersedes older Active/Next statements.

**Accepted FFVI discovery reconciliation Closed:**139 exact native-behavior
decisions added, preserving all prior rows and accepted source/data/catalog
identities. No production behavior changed or rebuild needed. Source-backed
review covers Narshe light path, Phoenix, Tower, Opera, Dreamscape train,
pre-defense party selectors and the burning-house entrance. Native field phases,
gate/puppet/trap/ordinary-return distinctions are explicit, never inferred from
map membership. Reused complete-episode receipts remain source matched. Exact
completion: accepted_ff6_mechanics_20261103/completion.json; qualification
0.731s/exit0/drained under2GB/25%CPU.571logical/603physical original records
covered;350V+732VI=1082 remain. No native payload/install/gameplay/agent.

**Active next capability:** ordinary Phantom Train entrance-to-engine journey,
preserving the accepted engine1/0/1 solution. Eighteen early/native counterpart
discoveries remain unreviewed. Bound20–30min initial production-planner proof
from actual entrances with real gates/actors and exact script flags; decide
whether the existing ordinary route already supplies each jump/detachment or
whether a minimal additional pre-engine stage is needed. No speculative defect
or Closed engine redesign is inferred. Then finish any required source/focused
integration and record exact limits before moving to another family.

**Zone Eater crossings source/focused Closed:**24 actual manual jump legs,
11 complete objectives, three ordinary boundaries,13 new model state/unknown
cases,196 physical identities, eight exact state-script hashes,65 directional
arrival rows and four instructions in12 languages pass. Native-ready controller
waits through Gogo naming/dialogue even if410 is already set. Ten earlier guide
records/catalog values preserved; no new hook or ownership change. Receipts:
66.178s/443686912B and23.274s/349323264B,exit0/drained,2GB/25%CPU/one worker.
Exact source/report identities: zone_eater_bridges_20261103/completion.json;
topic ZONE_EATER_CROSSINGS_20261103.md. Source design Closed; combined native
output/installation and actual moving-guard/audible/frame confirmation pending.
No game/probe/agent. The finite evidence/implementation slice completed within
its20–30min scope; initial diagnostic fixture errors remain preserved.

**Next original-audit milestone:** inspect the remaining FFV/VI discovery
families by exact native behavior and existing guide/route ownership.432logical/
464physical reviewed;350V+871VI=1221 original candidates remain. Zone Eater roof,
contact, manual-jump and animated-platform supplements are separately qualified.
Preserve every Closed guide and the custom-audio design. This is the continuing
authorized all-six audit, with no new runtime/deployment/refactor scope.

**Active finite reconciliation:** remaining FFVI discoveries on the twelve
accepted story-guide maps. Expected20–30min for exact identity/native-behavior
and prior complete-episode evidence review, explicit semantic dispositions and
source/preservation checks. No accepted guide redesign is authorized. Exit:
every selected discovery has its exact source-backed disposition, or an actual
new missing capability is recorded separately before another implementation.
The immutable inventory is discovery evidence, never an automatic classifier.

**Roof source/focused Closed:** final context/native-name/copy-cost checks pass,
1498 FFVI names0missing;10000 timer copies mean1.103us/peak17us in capped fixture.
Actual targeted build/check34.075s;518.802s resource receipt includes admission
waiting, peak345141248B/exit0/drained/2GB/25%CPU. Closure/wiring passes; exact20
source identities/reports/retained-nine-guide/catalog proof are in
zone_eater_20261103/completion.json. No native payload built or installed for this
slice; physical/native-hook/audible/frame confirmation remains after combined
audit delivery. Source estimate overrun was limited to supported context checks
and shared admission, with no architecture/support branch. This guide design is
Closed; no internal roof refactor or new optional check is queued.

**Next active original-audit milestone:** Zone Eater bridge contacts and jump-room
controls, separate from the Closed ceiling. Begin one bounded20–30min primary
fact/physical-route slice: exact fall/recovery/jump operation, actor ownership,
reset and completion. Then implement the complete required Shift+G sequence and
finite producer/model/route/localization checks, or record an exact essential
blocker while continuing independent authorized work. Current original inventory
350V+874VI=1224 pending,429logical/461physical covered; roof supplement complete.
No new model, game launch, capture, deployment or closed-guide redesign implied.

**Current roof integration:** model/controller/shared-input and12-language
checks pass. The guide previews a cardinal leg, waits for a second deliberate
readiness request, then uses the real native cycle to release a short Go cue.
Arming during animation changes only owned intent; new input-query integration
is confined to that pending FFVI request. Nine accepted guides and all prior
catalog values are preserved.211 real connected-cell recoveries/34 safe resumes/
two incoming floor connections pass; actual505 arrival owns completion.
Boundaries/source/semantic93.657s/445239296B and controller/shared/model65.881s/
348430336B,exit0/drained,2GB/25%CPU. Peak model0.071ms is capped-fixture evidence.
Two automatic entry counterparts are now explicit:429logical/461physical reviewed,
350V+874VI=1224 still unfinished.34 roof markers/unbound0850 are additionally
source-checked; they are not fabricated inventory actors. Final focus/menu/
native-copy-cost/native-name qualification session33836 is waiting for a shared
slot; no child launched. No native DLL rebuilt or installed for this source
slice. Topic ZONE_EATER_ROOF_20261103.md; no other system reopened.

**Custom controls output Closed, November3UTC.** Source/interface/focused work
and all required integrations now pass canonical build,242 fresh reports, smoke
and the exact22-file payload scan. SHA300ff3dfbebb647ae814baa13a171545533a2ae915236b33b1daeada9cc9d484,
12146688B;834 frozen inputs/205targets/67resources/12languages/8340names0missing/
149used354defined0undefined/47exports. The canonical run passed1272.039s. Its
outer job ended1278.267s/801210368B/exit1/drained only at missing scan-snapshot
setup; no scanner ran. Preserved that evidence. The same immutable output then
passed read-only dependency inventory, exact scan and final verification38.070s/
101339136B/exit0/drained under2GB/25%CPU/one worker. Defender no threats/0 matching
detections/stable hashes; exclusions unverified/signing0. Exact custom checkpoint
canonical_completion.json; source/interface remains Closed, physical listening/
frame confirmation and final all-six installation remain separate. No game launch.

**Next active original-audit milestone:** resume the recorded Zone Eater roof
discovery, preserving every Closed guide/design. First bounded20–30min evidence
slice settles native roof ownership/cycle, safe footprint, reset/death and exact
script binding; then connect the complete Shift+G journey and production route/
state/localization checks. No new runtime capture or architecture is authorized.
This is part of the existing required VI unique-puzzle scope. Remaining inventory
still350V+876VI=1226,427logical decisions/459physical reviewed; unbound0850 main
must also receive an explicit disposition. No further custom-audio refactor.

Zone roof evidence slice now establishes34 safe markers, the automatic5-second
callback, death/reset branches and exact native/point/layer ownership. The real
planner passes all nine forward legs, at most9 cardinal steps between safe areas,
43.437s/444329984B/exit0/drained, owner0. First test-print atomic compile error is
preserved and fixed only in that print. No ceiling controller is implemented yet.
The evidence slice reached its expected boundary while checking native layer
changes; next connect one exact timer-reset observation, owned-value roof model,
fresh-output guard and existing ready continuation, then route/state/producer/
localization checks. Estimate45–60min source/focused integration, full delivery
separate. No additional observation framework or per-frame scroll hook. Exact
topic ZONE_EATER_ROOF_20261103.md; checkpoint zone_eater_20261103. This preserves
the accepted story-controller design and every Closed guide.

**Active output continuation41305/resume9:** prior24779 completed170 project
outputs then reached1200s,504606720B/exit124/drained, no compiler error. All834
freezev7 inputs verified unchanged. Preserved compilation now continues with
a finite2400s canonical window so the remaining compile/tests can complete;
the aggregate2GB/25%CPU/one-worker limits are unchanged. Exact upcoming receipt
canonical_resume9_resource.json. This is the same required end-to-end output
checkpoint, not another architecture/support branch. Original helper retained
as resume_shared_v8.py. Canonical/smoke/scan closure still pending; no audit work
or new game installation.

**Latest custom-output checkpoint:** resume7 ended at1200s,514572288B peak,
exit124/drained, without a compiler error; completed outputs are retained.
Independent later-check diagnostics in an isolated mod-only working copy passed
all available later checks apart from the stale plain-text README companion.
The existing release converter regenerated it and the actual asset check passes.
These diagnostics do not replace source-matched canonical qualification.
At the ended-job boundary refreshed only Main ReadMe.txt; every prior frozen
source/input hash is unchanged. Freezev7 adds both README forms (834 inputs).
Original/prepared/current companion evidence is in document_companion_completion.json.
ACTIVE Main24779/resume_shared.py8, canonical_resume8_resource.json, unchanged
1200s/2GB/25%CPU/one-worker limits. Full canonical/smoke/scan remains the finite
exit condition before custom-output closure and return to the original audit.
No new game installation, runtime probe, delegation, architectural branch or
audit reopening. FF1 Shrine fix remains installed; natural confirmation pending.

**Latest custom-output integration:** run81049 passed trap/key checks, then
event projection found180 absent exit rows representing the two reviewed Kefka
manual transfers (546 Entity12 roller→544;544 Entity13 duct→546). Current-floor
scanner already promotes these exact physical arcs; remote manual Entity/Anim
branch now does likewise. No graph/guide/game data/hook/thread change. Focused
parity source Closed95.212s/481476608B/exit0/drained: all2454 roots/7362 states,
2416 distinct projected events, explicit two-floor witnesses, scanner/remote
returns/FF6 story runtime pass. Diagnostic40.692s/456437760B/expected exit1,
original files/failure retained. Scope15–25min focused integration complete;
whole output qualification separate. Exact event_projection_integration/completion.json;
topic KEFKA_MANUAL_EXIT_PROJECTION_20261102.md. Freezev6 changes only projection
and its fixture fromv5, every other input verified unchanged. ACTIVE full
canonical/smoke/scan62929, same1200s/2GB/25%CPU/one worker; receipt
canonical_resume7_resource.json. No further source edits or audit work before
custom output closure. None of the corrected shared candidate installed yet.

**Current custom-output integration correction:** all205 targets compiled, but the
unchanged fourth canonical run failed trap_route_check after Floating navigation
passed with owner0. Exactly two FF3 map4/Map_10040 bottomless-bog Event objects,
IDs182/183 at186/187,205 on native Upper2, are stored in g.traps[2] while world
trap lookup reads plane0. Source primary object metadata is outside Git in
tools/mapdump/ff3_objects.jsonl; factual identity only recorded here. Existing
current/projected routing sets live_traps=true and owns story/transport state;
preserve that authoritative override. Minimal reader normalization should match
its existing world transition-plane rule; room masks/layers stay unchanged.
Strengthened baseline session82358 reproduced four failures under2GB/25%CPU. Canonical failure,
original two mod inputs, linked DLL and source manifest are preserved in custom
checkpoint/trap_regression. Minimal world-plane normalization now passes the
four focused checks104.886s/218763264B/exit0/drained. Exactly the reader and
fixture changed; all other frozen inputs verified unchanged. Source/focused
integration Closed; facts/source/report hashes in trap_regression/completion.json.
Fresh canonical/smoke/scan session17738 is ACTIVE, same1200s/2GB/25%CPU/one worker;
receipt canonical_resume5_resource.json. Topic FF3_WORLD_TRAP_MASK_20261102.md.
The required acceptance was:
reproduce explicit world-plane failures, one-line mask fix, all-six255-trap and
198 reviewed-arc checks plus live disabled/active and foot/air selectors, retained
transport guard/FF1 Ice/worker checks, then fresh canonical/smoke/scan. Scope
15–25min focused integration, whole warm qualification separate; no new design,
hook, pointer owner, game runtime probe or resource exception. Custom source/
focused remains Closed; its output cannot be certified until this repair passes.

**Latest output state:** corrected canonical run17738 passed trap and Floating
checks, then stopped at the old FF3 key guard expecting1861 instead of the
reviewed1853-cell physical topology. Guard integration source/focused is Closed:
actual planner ablation reproduces1861 only by opening exactly eight cover-walls;
current1853 loses no ordinary floor, all13 door/two-key prerequisites still hold.
Exact map71 Bottom x9/26,y6/7; map223 Ground x9/16,y5/6. All owned arrays restored.
246859 assertions/203 complete journeys/61 unlocks/490 native hops pass,
worker-native0;11 pinned native owners unchanged. No runtime/game data changed.
Proof77.181s/461361152B and final39.233s/474140672B,exit0/drained/2GB/25%CPU.
Artifacts key_guard_integration/final_completion.json; topic FF3_KEY_GATES.
Freezev5 changes only the author guard and topology fixture fromv4. ACTIVE full
canonical/smoke/scan session81049, same1200s/2GB/25%CPU; receipt
canonical_resume6_resource.json. No further source edits or audit work yet.

**Human priority update, November2: Sunken Shrine, then custom sound controls.**
Actual FF1 log809340–809347: map103/5F, start19,29 L0, chests38/39/40,11
are incorrectly Not directly reachable/No path;13 chests/0 gates/10 moving
NPCs. Native master103/Map_30081_8 has horizontal loop1, while baked routing
only treats world maps as wrapping. Verify the exact connection, then correct
physical search/browse/directions/continuation without relaxing walls/gates.
Source/focused Closed: actual three-failure baseline now passes all chests,
follower/beacon and real one-step native collision-query/veto cases. Seven
affected checks/all-six router/follower pass; full FF1 bake reproduces the56
metadata-byte changes, geometry/world bytes unchanged.269.539s/498778112B
integration; final125.386s/497926144B/exit0/drained,2GB/25%CPU. Source estimate
slightly overran while qualifying the native handoff; no redesign. Topic
SUNKEN_SHRINE_LOOP_20261102.md; exact source/reports completion.json in the
matching checkpoint. Scoped sibling ff1_sunken_shrine_20261102 is now frozen
at724 inputs. Stage Shrine and retained Ship preflights pass. Job13172 reached
its2400s canonical limit near projects163–165/169, no compiler failure;
2533.176s overall,498688000B peak,exit1/drained. Preserve that incomplete run.
An unchanged incremental canonical/smoke/security resume passed681.412s canonical,
710.831s whole/801751040B/exit0/drained. Guarded FF1 installation Closed21:57UTC:
DLLb62ed342de0f6896,11625984B;169 targets/178 reports/8326names0missing,
47exports/67resources/12languages/0undefined. Defender22files no threats/stable,
exclusions unverified/signing0. Install12.235s/115920896B,22 hashes match,
144 protected/other five games unchanged, no game/probe. Natural confirmation
remains pending. Exact delivery_completion.json in Main Shrine checkpoint.722
prepared inputs and719 qualified inputs/DLL are fully archived.

Before returning to the broader audit, implement the owner's requested custom
sound controls in the shared six-game system. Scope: submenu appears only when
custom sounds are enabled; per-sound enable/volume plus timing for sounds with
preset repeat timing, clamped200–5000ms. Disabling an individual custom sound
restores its generated default, respecting existing global sound/channel toggles.
Wall tone offers clean looping or, with loop off, timed ping-style playback;
custom loop seam quality belongs to the sound designer. Preserve event-only
one-shots, defaults and native/thread ownership; no timing changes to generated
sounds. Inspect existing custom-sound producers/settings before implementation.
Done: persisted/localized controls, actual menu visibility/input handback,
per-sound fallback/volume/timing/loop/cancel bounds and source-to-output offline
checks, then qualified delivery. This queue entry is not implementation or a
new game/probe permission. Shrine source/offline is Closed and its sealed
delivery checks run independently; custom-sound source is the next active system.

**Custom-sound milestone, canonical/offline Closed; installation pending.** Existing18 WAV
cue IDs now retain original PCM, with a global toggle and per-cue submenu.
Preserve existing file opt-in behavior/default gains and cadence until
the owner adjusts an interval. Numeric overrides200–5000ms,100ms steps; unset
uses original timing, Confirm restores it. Walls default to existing clean loop;
when loop is off, a custom ping uses the chosen interval (unset1s). Event-only
Footstep/WallBump do not acquire repeat timers. Applicable periodic owners:
four walls/Counter/entity emitters on Audio; Beacon/blocked and Landing on the
existing native audio-loop owner. HRTF routes blocked beacons through the normal
Beacon cue, so actual selected PCM owns timing/volume. Long wall/counter/entity
clips finish before repeat; native Beacon/Landing retain one-shot retriggering.
Preserve per-emitter fairness/eight-start budget/voice limit/stale expiry.
Boundary: audio custom policy/bank/runtime/configuration, existing timing call
sites and F8 menu/12-language text; no new hook, native pointer transfer, device
owner or game action. Expected45–75min source/offline checkpoint; actual menu,
fallback PCM/legacy/HRTF, timing/loop, settings persistence/ranges, cancellation,
overload and callback/whole-menu cost checks precede closure. Source/UI/offline/
installation remain separate. Existing audio engine and renderer design stay.
Actual custom mixer/retained stereo/HRTF suite passes. Corrected all-six menu,
producer and native Beacon checks pass72.535s/326422528B,exit0/drained.
First menu fixture failure used obsolete positional row assumptions; preserved.
Final HRTF fallback/Landing/cancellation checks pass; corrected pending fixture
first establishes a completed route. Actual settings snapshot persistence and
capture pass,30.040s/270471168B. Native capture18 mean10.285us/peak232.700us;
all-six menu construction mean0.022ms/peak0.024ms in capped offline fixtures,
not in-game/audible responsiveness. Exact hashes/reports: custom checkpoint
completion.json. First shared canonical job30885 reached its1200s bound at
67–69/205 targets,486436864B/exit124/drained, no compiler error. Preserved
resume75897 reached115–117/205,1200.698s/990343168B,exit124/drained without
compiler errors; core/all-six libraries built. Unchanged next resume25365 uses
the same shared admission and1200s finite bound,2GB/25%CPU/one worker. It reached
166–168/205,1256.078s whole/933588992B/exit124/drained without compiler errors;
DLL linked. Next unchanged resume72324 is active, resource receipt
canonical_resume3_resource.json. It compiled all205 targets, then timed out in
canonical regressions:1212.324s whole/934993920B/exit124/drained. All compiler
outputs/source preserved; ACTIVE unchanged resume19259 revalidates cached
compilation and runs all canonical regressions/smoke/scan with a fresh1200s bound,
same2GB/25%CPU/one worker. Receipt canonical_resume4_resource.json. No game changes.
Every incomplete log/exit/source snapshot retained.
Minimal twelve-language help correction removes an overstatement about native
Beacon/Landing long clips; no runtime change. Original manifest/completion v1
retained. Source guard rejected a slash-alias metadata mistake before compilation;
repaired manifest verifies only the exact two text/author inputs changed.
The45–75min checkpoint estimate has overrun in combined205-target output
qualification; source/focused controls are complete. No runtime scope expansion,
resource exception or parallel compiler was added. Incremental progress retained.
Localization passes; exact help_integration.json/resource receipts preserved. Fourteen
new keys are authored in12languages; controls, ownership and dependencies are
documented in CUSTOM_SOUND_CONTROLS_20261102.md. No custom payload installed.

**Current owner FF1/EXP follow-ups, November2.** Ship source/focused Closed:
correct Ship4/Canoe2 baseline reproduces six failures; nine corrected profiles
and retained FFIII/shared routing pass. Exact legal Airship landing/Ship boarding
continues through the real shore. Initial swapped-ID fixture is preserved as
inadequate evidence. Source/report/limits: ff1_ship_airship_20261102/ and
FF1_SHIP_PICKUP_20261102.md. FF1 installed21:57UTC in b62ed342 above.

**Service naming source/focused Closed.**485 physical service records cover all
382 ShopNPC17 plus103 scripted actors.210 exact actor/raw corrections in12
languages (16/14/11/23/77/69);275 usable/personal labels retained. Live/remote/
copied obstruction labels share @actor/map/id/raw, then existing fallback.
Gaia/Onrac Innkeeper/Priest are corrected without global NPC aliases. All6962
NPC/shop records/83544 localized labels pass;485 services also test exact scope,
live/snapshot agreement and retained names. Scanner/worker and nine Ship cases
pass, owner0.234.217s/498970624B/exit0/drained under2GB/25%CPU/one worker.
Data author/check is wired canonically. Exact source/reports/resource:
shop_npc_audit_20261102/completion.json; SERVICE_NPC_LABELS_20261102.md.
The finite source/offline work completed within its20–30min estimate. Scoped
FF1 canonical/smoke/security/guarded delivery is Closed21:57UTC,b62ed342 above;
prior qualified source/payload is archived. Other-game naming delivery pending.
Do not import unfinished guide ownership or deploy the older sibling to other
games. Natural gameplay confirmation remains separate.
User also requests EXP skip acknowledgement in FF1/III/IV/V/VI, excluding FFII.
EXP source/focused checkpoint is now Closed: actual shared native producer
reproduced four missing acknowledgements, then all-six phase/Autodetail/repeat
and five-EXP-game early/close/advance cases pass. Existing detours now enable
the acknowledgement for every game except2; no new hook/polling/deduplication.
21.416s/314617856B/exit0/drained under aggregate2GB/25%CPU/one worker.
Exact source/reports: build_check/exp_skip_20261102/after.json; affected behavior,
dependencies and evidence limits are in RESULTS.md. FF1 installed21:57UTC in
b62ed342. Other newly enabled adapters await their qualified delivery, without
deploying the older sibling source wholesale.
Background unique audit is checkpointed at Zone Eater discovery (nine source
scripts, including unbound roof504 main0850 and native drop/rise SysCalls).
No Zone module yet; safe cells/timing/reset/kill and complete route still unknown.

**FFV completed-family semantic linkage source/focused Closed, November2.**
52 exact candidates/44 current body hashes/six primary hashes match.31 guided
controls/5counterparts/1optional hint/5ordinary rewards/5story scenes/4simple gates/
1message-only examination. Existing Library/Catapult/Ghido/Surgate guide designs
are unchanged. Author/--check and earlier scenery-order preservation pass.
All-six completeness still refuses1226 unfinished cases: FFV350+FFVI876 remain;
427logical decisions cover459physical records. Exact input/report freeze:
library_witness_20261102/completed_families_completion.json; topic
FF5_COMPLETED_FAMILY_SEMANTICS_20261102.md. Next prioritize remaining FFVI unique
families, preserving the requested four and other Closed guide evidence.

**FFIV priority delivery Closed, November2 at18:16UTC.** Installed
011f0dc2ce20e815fc3aa3f5a2b5fb4f3844331e3f3ee28a00f34e943d1a3a45,
11550208B: five battle-only party keys plus Damcyan/Fly, initial event-item focus,
save categories, Kaipo returns and status name/level heading.719inputs/167targets/
174freshreports/8326names0missing/144used350defined0undefined/47exports/67resources/
12locales. Canonical resume737.683s/full757.530s/798998528B, install7.811s/
115642368B; both2GB/25%CPU/one worker/exit0/drained. Defender22exactfiles/no
threats/stablebytes; exclusionsunverified/signing0.22installedhashes/144protected/
otherfive unchanged. Fresh guard found IV closed, no launch/probe; owner told
may reopen. Exact sibling Class-Delivery/build_check/ff4_party_keys_20261102/
completion.json. Natural gameplay confirmation remains separate. The old finite
attempt is preserved, not counted as a pass. Keep qualified Stage719 frozen;
do not deploy this older tree to II/III/V/VI. Main original all-six audit remains
open. Next finite slice:52 FFV candidates around four completed families, with
explicit native/guide dispositions; estimate10–20min metadata/source checks.

**Physical semantic-instance tool slice source/focused Closed, November2.**
FFVI911 physical records include35 scenery lights sharing three zero-ID keys;
32 extra records must not disappear through logical-key coalescence. Validator
now retains every physical instance and requires exact declarations for repeated
keys; immutable inventories stay unchanged. Three exact non-interactive lighting
groups reviewed with all35 cells/current object+script-master hashes. All372
prior I–IV decisions/source checks pass; FFVI still rejects876 unreviewed cases.
375 logical decisions/407 physical records are covered; FFV402+FFVI876 remain.
Existing typed FFV guide fields now validate as real references. No production
guide/navigation/hook changes. Evidence library_witness_20261102/; topic
PUZZLE_SEMANTIC_INSTANCES_20261102.md. Final all-six semantic wiring remains due.

**Library ordinary-witness integration source/focused Closed, November2.**
Five former successes assumed unopened shelves were ordinary floor; actual
native geometry requires intermediate controls. Nine static profiles now assert
four real routes/five explicit Story gated results, and all nine same targets
must occur on native physical journeys connected to fresh275/PointIn1.18 checks
pass;441states/1000edges/188observations, zero dead/greedy/worker-native failures.
117.838s/467316736B/exit0/drained under2GB/25%CPU. Only the obsolete test changes;
Library production design and521 complete guide episodes remain Closed.
Prior failing source/report/legacy-cover comparison retained. Evidence
build_check/library_witness_20261102/; topic FF5_LIBRARY_WITNESSES_20261102.md.
Main canonical remains required; this resolves the old witness discrepancy.

**Owner priority, November2: five battle-only party keys.** Slash/Period/Comma/
M/N map to slots1–5, freeing native keyboard L/R2/3. Native battle item/magic
submenus must admit the checks, while outside-battle waypoint/map/menu functions
remain intact. Implemented as an appended read-only BattleParty scope with one
owner-cached native ownership bit; no context promotion, new hook or party-reader/
ATB redesign. Per-slot labels/scope cover12languages. First actual party/Esper,
dispatcher/native10paths, binding editor, transition and modal checks all pass:
167.544s/332988416B/exit0/drained,2GB. FFIV fifth row/non-active status behavior
pass. Source/focused Closed: four supplemental checks also pass117.847s/
328200192B/exit0/drained, including10k native callbacks with zero extra key polls.
Minimal isolated port has719 frozen inputs/52 matching header-cache consumers;
all11 focused delivery checks pass. Canonical/security resume77440 passed and
the exact IV candidate is installed at18:16UTC. Stage stays frozen as evidence.
Evidence build_check/battle_party_keys_20261102/, topic
BATTLE_PARTY_KEYS_20261102.md. No new key deployment yet. This human steering
takes priority over the remaining FFV/VI audit; prior accepted designs remain
Closed. Finite source/integration slice estimate20–30min; installation has its
own fully qualified checkpoint.

**Build-cache dependency source/focused Closed, November2.** The full-fixture
PCH trial failed duplicate mutable definitions and disabled itself; evidence
103.718s/490332160B/exit1/drained is retained. Headers-only reuse passes actual
browse/remote/Ice assertions, ABBA40.578→37.627s (7.3%), first fill3.325s;
whole256.896s/481107968B/exit0/drained. Exact Release contracts match76 main/52
isolated consumers. The option is now ON for compatible offline targets only;
native DLL and per-test state/functions remain independent. Expanded isolated
consumers are in the current canonical job. Prior startup/shared-object/entry
designs remain Closed. Evidence build_check/route_pch_20261102/; no more cache
experiments are planned.

**Owner CPU allowance, November2.** Multiple cores are now explicitly allowed,
never unlimited. New local jobs use min(25% machine, three logical cores'
equivalent):25% on this12-thread host; at most50% combined across both slots.
Hard aggregate2GB per tree, below-normal priority, admission/cleanup and serial
compiler/server settings remain. Adoption is source/focused Closed: all14 real
small-heap guard checks pass with the new cap, including two admissions/third
refusal, process/child/aggregate memory denial, verification and cleanup.
Evidence build_check/cpu_allowance_20261102/. No speedup claim is inferred from
the allowance alone. Frozen IV job54197 kept8.33% and reached group154–156/167
before its finite2400s limit:2400.439s/998813696B/exit1/drained, no installation.
Its exact source/logs/resource receipt and valid compiled outputs are preserved.
Job77440 resumed the unchanged719-input candidate at25%, same2GB/one worker;
all canonical checks, smoke and scan passed. IV installed at18:16UTC. Stage
remains frozen; actual resume/build timings are not a comparable speedup claim.

**FF1 Citadel/B3 source/offline/delivery Closed, November2 at15:33UTC.**
Installed42fe55ac0e6d793332eb7918bbc38d924378e332b9cf72cd96707cc50ea237c7,
11538432B.713inputs/165targets/171freshreports/8317native names0missing/
47exports/67resources/12locales; all22installedhashes match/144protected and
other five unchanged. Defender22exactfiles/no threats/stablebytes; exclusions
unverified/signing0. Canonical1553.620s/full1573.499s/800321536B, install7.432s/
115027968B, hard2GB/8.33%CPU/oneworker/exit0/drained. Fresh guard found FF1
closed; no launch/probe. Owner told may reopen. Exact source/scan/install evidence
in sibling Class-Delivery/build_check/ff1_trial_reward_20261102/completion.json;
topic FF1_TRIAL_REWARD_20261102.md. Rat Tail/B3 natural confirmation remains
separate; original physical Levistone remains user-confirmed. Keep these accepted
systems Closed; next queued IV delivery/status then remaining FFV/VI audit.

**Kaipo return listing source/focused Closed, November2.** Exact town32 and
interiors33/34/36/37 reproduce four off-floor returns. Native arrivals stand
three cells inward; the old policy recognized at most two. Existing listing
proof now accepts a straight three-cell inward placement, retaining explicit
arrival IDs, unique reciprocal pair, known state/layer/depth. All four returns
are hidden from town and still physically route on their own floor; retained
FFIII/negative cases and largest graphs in all six games pass.61.722s/
445018112B/exit0/drained,2GB/oneworker. Four additional initial failures came
from an invalid local-goal next_map assertion; corrected actual exit-cell
assertions pass and the old report remains. No graph/bake/native hook/string
change. Exact evidence: build_check/kaipo_returns_20261102/completion.json,
topic KAIPO_RETURNS_20261102.md. IV canonical/security/install pending; keep this
minimal accepted policy intact. FF1 candidate remains frozen independently.

**Owner Citadel reward correction, November2: source/focused Closed.** Existing
elder/maze route retained; appended exact Rat Tail chest9 Ground27,29, scenario45
for reward and whole-recipe completion.3F arrival, adjacency, battle completion
and unknown reward state cannot finish the trial. Already-completed resumes on
all three floors and stale route retirement pass. Same actual controller before
10failures/after0;87controls/52recipes,12maze resumes,48reward routes across four
guardian states,24actual encounter handoffs, all0failures. Three native author/
locale checks pass, unchanged12-language instructions.47.938s/451661824B/exit0/
drained under2GB/8.33%CPU/oneworker. Exact source/report hashes in
build_check/ff1_trial_reward_20261102/completion.json; topic
FF1_TRIAL_REWARD_20261102.md. No native runtime changes in main. Installation
pending: isolated713-input delivery ports the receipt/identity subset into the
older installed guide, keeps the confirmed maze and adds the qualified B3 exit
identity correction. Other unfinished all-six puzzle work is not imported.
The prior Damcyan candidate timed out at1800s/512606208B, exit1/drained, without
qualification/deployment; logs and warm outputs preserved before the new build.

**Owner FF1 B3 cross-floor correction: source/focused Closed.** Actual latest
log786930–786994 loses remote paths after the fall while local B1 stairs work.
Scanner's destination-only dedup discarded B1's reachable southern world exit19
in favor of disconnected northern13. Physical identities now survive until the
existing reachability-aware grouping. Actual Upper30,42 reaches19/no trap and
cannot reach13; same scanner regression before1failure/after0.58.552s/367980544B/
exit0/drained; original30Levistone journeys remain passing and user-confirmed.
Evidence build_check/ff1_cross_floor_20261102/. Minimal delivery pending with
Citadel; this does not reopen the accepted physical Levistone route design.

**FFIV initial item-selector source/focused Closed, November2.** Actual Sand Ruby
log first-row silence comes from the adapter absent in the installed branch.
Existing main native KeyInput reader is correct and unchanged. Corrected proof
pins828350 Open→828690 SelectInit→828460 SelectContent slot9, initial cursor;
older metadata8091D0 described Touch rather than KeyInput. Prior proof retained.
Four native bodies,16actual adapter cases and retained FFIII suite pass, with I,
Autodetail, equal-text events and lifecycle guards.35.919s/116199424B/exit0/drained;
contract1.824s/115802112B,2GB/oneworker. Evidence
build_check/damcyan_20261102/ff4_initial/completion.json and FF4 puzzle topic.
Port the existing adapter and delayed generic-cursor suppression in the next
qualified IV candidate; no new hook/design is needed. Not installed yet.

**Save-category source/focused Closed, November2.** Actual IV game log
already proves Waterway point in both Events/Save Points. Exact external census
finds19 IV points with19 scripted companions,32 V points with30 scripted
companions and42 VI points with42 tutorials. The current canonicalization is
VI-only; extend the existing physical/identity rule, retaining unrelated events,
native tutorial interaction and one Save Points destination. Check local and
remote rows/all native points, cancellation and invalid/coincident identities.
All93 native points/91 companions pass in186 local/remote profiles; unrelated
same-cell events retained. Before98failures/after0.48.725s/444260352B/exit0/drained;
actual scanner integration39.966s/346603520B/exit0/drained, no worker-native reads.
Exact inputs/reports: build_check/save_category_20261102/completion.json; topic
SAVE_CATEGORY_20261102.md. No new strings/native hooks. IV layer capture hint
and V Ground layer are retained even with other event filters off. No save-category
install; FF1 delivery tree remains frozen separately. Preserve this minimal
accepted design through the next qualified IV installation.

**Owner resource update applied, November2 UTC:** future heavy jobs use two shared
machine-wide slots with a default hard2GB aggregate ceiling per complete tree,
up to4GB combined; one worker per tree, bounded CPU/below-normal, no servers/
parallel compilers. The old active Ghido1GB matrix completed unchanged; prior
receipts retain their original limits. Maintained runner+local AppContainer guard
and .NET enforcement merged;14 real small-heap guard checks pass. Exact source
merge/report: build_check/resource_slots_20261102/. No active or unrelated job
was modified/terminated. Scope and all other permissions are unchanged. This policy integration does not
reopen Closed game subsystems.


**November1 FF1 three-route source/offline/delivery checkpoint Closed.**
Installed at21:52 UTC after two fresh closed-game guards. DLL11525120B SHA256
378838e78c5ada5dd4cc1e473a19bf333612dd071fc8ff9bb733345027a14f85.
All22 installed payload hashes match;144 protected files and all other five
game payloads unchanged.708 frozen inputs/170 fresh canonical reports,
151 targets/8308 native names0missing/143used350defined0undefined/47exports/
67 exact resources/12 locales. Final warm canonical400.662s/763187200B,
verification25.055s/85323776B and guarded install8.582s/115425280B, all serial
1 GB/exit0/children drained. Defender22 exact files/no threats/0matching events/
stable hashes, valid scanner signature; exclusions unverified/signing identities0.
No game launched, controlled, attached or runtime-probed; gameplay/audio/frame
confirmation remains natural and separate. Owner told FF1 may reopen.

The four-point half-tile Levistone passage keeps every real pit intact; six
room origins/native-axis/repeat-corner/beacon/browse/full B1 journey checks pass.
Ship directions choose an actually boardable source shore. Sage directions walk
through Giant81, with Ruby17/Giant11/opening18 semantics preserved. The failed
76-minute canonical and test-only invalid-shore amendment remain archived; final
runtime DLL is byte-identical to that built binary, now fully qualified. Exact
recovery, protected-file checks, source delta and completion:
../FFPRCore-Class-Delivery/build_check/ff1_giant_route_20261101_final/.
Do not rerun its preparation/deployment helpers or reopen the accepted design.
Minimal regression integration and new owner-triggered findings remain allowed.
Separate Levistone label information finding remains with the original audit.

**FFIII key-gate source/focused checkpoint Closed.** Expanded integration passes
371.189s/524156928B/exit0/drained, serial1 GB.13locks/246859assertions/215chest
profiles/65door profiles/203full journeys/61manual unlocks/490native hops,
zero failures/worker-native reads. Complete205..217 writer/reset census and11
unchanged native owners pass, preserving the reviewed external archive without
re-export. FF1 key doors6rooms24variants118chestcases143routes, trench126cases,
Eureka50remote5local5collected guards and FFIV item-owner16cases pass. Exact
inputs/reports/budget: build_check/ff3_keys_integration_20261101. Reader/topology
evidence is retained. All-six canonical/security/deployment and natural runtime
confirmation remain pending; no new FFIII DLL installed. Do not refactor this
accepted design; preserve it through minimal shared integration.

**FFV Library source/focused checkpoint Closed, November1 23:43 UTC.**784 inputs
and exact reports/receipts are bound in ff5_library_production_20261101/completion.json.
Actual production704 complete episodes/9740 native operations/277 layouts/
307 walking regions/7112 cells, upper8/lower5 native owner83 assertions,
2496 plans/12 locales,12 physical entries/14 hops, shared86 controls/52 recipes
and51 preserved FFIII owner cases pass. Closure504states/1152edges/208
observations has no dead states, loops or worker-native reads. All reachable
inactive baked-contact positions pass without a speculative runtime change.
Native53-script/two-owner contract and canonical wiring complete. Final
97.896s/459747328B/exit0/drained; extra positions53.951s/478453760B/exit0/drained,
serial1 GB. Retained owner pass comes from a cohort that later failed locale
coverage; matching source and independent exit0 preserved. Old850/357 proof,
omitted371 reset, projector/fixture failures and missing readiness translation
are retained honestly. Topic FF5_PUZZLE_GUIDES_20261101.md. No new DLL installed;
actual frame/audible timing and final all-six qualification remain separate.
Do not reopen the accepted Library design except authorized minimal integration
or a new human-triggered task.

**Shared Ready integration source/focused Closed, November2 UTC.** All27
actual consumers pass,790 frozen inputs and exact report/receipt hashes in
build_check/puppet_ready_integration_20261101/completion.json. The6 native
Ready owners all activate their actor before its new body. Compiler/projector
now retain that fact and retire stale Hide receipts on opaque bodies. Only402
initializer records change (5/17/49/40/140/151), with authored metadata, PCs and
actor slots preserved. Current Library441states/1000edges/188observations,
521 complete episodes/6946operations/210layouts/240regions/5431cells,
owner83/2256plans12locales/12entries pass. The all-six native-state/route/filter,
FF1 route/key/trench/slab/boarding, FFII gates/Deist, FFIII keys/forms/Eureka/
CrystalTower, FFIV items and FFVI story/party/Opera/train/light consumers pass.
Cohorts127.219s/350109696B,1328.317s/541196288B,165.563s/505450496B all exit0/
children drained, globally serial enforced1GB/8.33%CPU/below-normal. No new
DLL built/installed or game launched/probed. The accepted Library design remains
Closed; this necessary projection integration supersedes only its earlier
native-equivalence evidence. No further redesign is authorized by this closure.

**FFV Catapult source/focused Closed, November2 UTC.**799 frozen inputs,
exact reports/receipt in build_check/ff5_catapult_20261102/completion.json.
85 complete/resumed episodes/229 native operations/six room variants,18 ordinary
book routes/browse checks,100 shared-owner assertions,60 Catapult+2256 Library
plans/12locales. Retained Library521episodes/6946operations/188observations,
12entry profiles/14hops and shared86controls/52recipes pass. The guide crosses
the real rest contact with No when necessary, then follows five native clue
receipts309..313. Paid clues and occupied transit contacts never repeat. Book23
uses exact native10,14 pose and successful North/South sides; the complete bake
delta is one byte15→5. No geometry/trap is relaxed. Native UI-phase owner240B
is pinned; its comparison-only neutral treatment does not broaden the general
projector. One existing FFV owner and worker/result/lifecycle boundary remain.
Final111.091s/454230016B/exit0/drained, serial enforced1GB/8.33%CPU/below-normal.
No new DLL built/installed or game launched/probed; source/native/audio/frame
and final shipping claims remain separate. Failed candidates/invalid fixtures
are retained honestly in FF5_CATAPULT_20261101.md. Do not refactor the accepted
guide internally; further required leaves use minimal interface-preserving
integration. Original45–75min estimate overran its shared Ready and precise
entry dependencies; this player capability is now complete offline.

**FFV Ghido source/focused Closed, November2 UTC.**810 frozen inputs,
exact reports/receipt: build_check/ff5_stone_20261102/completion.json.16 complete/
resumed episodes/61 manual actions/63 native hops/both return resets;113 shared
FFV-owner assertions,108 Ghido+60 Catapult+2256 Library plans/12locales. Seven
current flags identify the stone, including inverted center176. Native return
callbacks restore center/SW; the guide recaptures actual completed state. Map372
uses current hidden gate11, and physical373 ends the guide. One existing owner/
Navigation worker, no new hooks or geometry/trap changes. Retained Catapult85
complete episodes/18 book directions+browse, Library521episodes/6946actions/
188observations/12entries14hops and static86controls52recipes pass. Final72.074s/
126779392B/exit0/drained, slot1/2/hard2GB/8.33%CPU/below-normal/one worker.
Current21-script/102-object bindings and eight available flag-writer assets
match; two uncoded declarations outside Ghido remain with broader FFV review.
Source/frame/audio/real-native and final shipping claims stay separate; no new
DLL built/installed or game launched/probed. Topic FF5_STONE_GUIDE_20261102.md
retains successful/failed fixtures and the corrected untracked source-report
incident. Do not internally refactor the accepted guide/owner boundaries.

**FF1 cardinal Levistone source/offline/delivery Closed, November2 at11:51UTC.**
Installed DLL11529728B SHA68bed68cc7304d8b5fe9a197475056f507979e69031d58600f8d179b78f65402.
712 inputs/170 fresh reports/151 targets/8308 names0missing/143used350defined
0undefined/47exports/67resources/12languages. Warm canonical329.002s, full
tree345.800s/761004032B; guarded install5.409s/108437504B, hard2GB/one worker/
8.33%CPU/below-normal/exit0/drained.22 installed hashes match/144 protected and
other five payloads unchanged. Defender22files no threats/0matching/stable bytes;
exclusions unverified/signing identity0. Fresh guard confirmed FF1 closed; no
game launch/probe. Owner told it may reopen. All six passage edges cardinal,
19 pits intact, explicit order/repeats/native inputs/beacon pass. Failed74min
scratch fixture is preserved; corrected guard-copy fixture passes, same DLL.
Final evidence: sibling FFPRCore-Class-Delivery/build_check/ff1_cardinal_ice_20261102_final/.
Do not refactor the accepted path internally. Next active capability is guarded
FFIV delivery of the qualified party-only/command-ready turn fix. Other queued
status/return/save-category cases and remaining FFV/VI audit stay separate.

**Historical owner-prioritized checkpoint, November2:** Levistone guidance must use only
ordered cardinal directions, including needed half steps. Actual current game
log772862..772928 speaks0.5 Northeast twice in each attempt, and the owner
fell after trying North first. Correct the authored path and all actual output/
beacon/input consumers, qualify real pit clearance and explicit step order, then
prepare a guarded FF1-only delivery. No game launch/probe. Broader audit pauses
behind this regression. The prior diagonal passage is not an acceptable handoff.
Main source/focused now passes six cardinal half edges/390 independent samples/
19 pits/8px clearance, all15 arrival contexts/two complete journeys/six stairs,
24 actual follower moves, five internal recaptures, exact ordered speech and
actual fractional beacon cache/native-input/shared-route consumers. Isolated
712-input candidate building; installed baseline still378838e7. Topic
FF1_CARDINAL_ICE_20261102.md preserves failed formatter fixture and source/recovery.

**FFV Surgate source/focused Closed:**148 episodes/974 actions/419 correct-facing
shelf contacts/27 wrong choices/55 hops/28 resets;133 shared-owner assertions,
96 Surgate/108 Ghido/60 Catapult/2256 Library plans across12locales pass.
54 archived source bindings/302 objects,17 required bodies/14 available writers
match. Frozen inputs/reports/receipt: ff5_books_20261102/completion.json.
110.531s/387244032B/slot1of2/hard2GB/8.33%CPU/below-normal/exit0/drained.
One existing FFV owner/worker, no new hooks or geometry changes. Initial invalid
return waypoint failure is retained; actual12,16 floor waypoint fixes recovery.
No new DLL built/installed. Closed design permits necessary leaf integration,
not internal refactoring. Topic FF5_SURGATE_BOOKS_20261102.md.

**Owner-requested build performance follow-up, November2:** routine incremental
builds should return to minutes without consuming the machine. Preserve the
two shared2GB slots, one worker/node, bounded CPU/below-normal/no servers/reuse.
Do not change the active isolated candidate or active jobs. Confirmed sources
of potential overhead are151 fresh serial MSBuild invocations and repeated
compilation of common check sources/embedded route fixtures. Measure before/
after on representative source-matched cold/warm inputs under identical limits;
reduce repeated startup/compilation, preserve all required validation and final
shipping checks, and keep development work scoped to affected consumers. The
one-hour estimate is not a routine build target or blanket future expectation.
Finite next decision: can small serial MSBuild target groups reduce startup
overhead inside2GB without changing parallelism or retaining servers? CPU/RAM
changes above the maintained contract are not inferred from the request.
Initial warm three-target ABBA comparison: separate4.284/4.060s versus combined
CMake4.016/3.961s, both still start MSBuild three times; no adopted speed fix.
Direct solution6.097/4.352s changes a test executable hash and is not accepted as
an identical-artifact warm comparison. Receipts preserved; active FF1 candidate
untouched. Next finite decision is bounded serial exact-project task groups;
see BUILD_SPEED_20261102.md. Common check sources repeat88–109 times in main.
Startup slice is now source/focused Closed in main: the observed CMake driver
uses amd64 MSBuild/VisualStudioVersion18.0; the rejected probes used x86.
Exact-host serial three-project groups take2.145/1.941s versus4.110/4.102s
separately, keeping identical warm executables under the same caps. Production
helper defaults to at most3 projects per process, /m:1 /nr:false, no parallel
work/servers, with group1 available for reduced retention. Actual group3, residual
2+1, group1 and first-failure/across-group stopping pass. No required check or
feature removed; active FF1 candidate untouched. Whole/cold timing remains to
be observed on later required builds; do not claim a universal2x speedup.

**Owner-queued status follow-up, November2 (after Levistone):** every status
buffer in all six games must start with the current character name and displayed
level, e.g. Cecil. Lv.12, before favored hand or other status details. Even games
without explicit character levels must put the character name first; do not
invent a level. Owner's FFIV screenshot shows Cecil/Dark Knight/Lv.12 in the
native header and Right-handed below HP/MP, while the mod buffer starts at favored
hand. Source/integration work is queued, outside the active isolated FF1 candidate.
Check actual FFIV game-directory logs before asking gameplay context; no tester
logs/game launch/probe. Cover all six applicable status producers, character
switching, current buffer/manual/Autodetail and displayed localization.

**Owner-prioritized FFIV turn delivery, November2 (after Levistone):** owner
hears Desert Sahagin's turn. Existing ATB party-only fix is source/focused
Closed, not installed in current FFIV a6d9b4ce. Do not reopen the accepted
command-ready design merely because the old installed binary still exhibits
the defect. Verify exact current candidate inclusion, preserve command-menu
readiness/party-only T and automatic speech, complete exact shipping checks,
then prioritize guarded FFIV delivery after Levistone. No enemy turn names or
enemy turn-order schedule belong in these announcements. Scope includes the
existing all-six applicable adapters; source/offline and installed/gameplay
claims stay distinct. Contracts: ATB_COMMAND_TURNS_20261031.md.
Now active after completed FF1 install: exact qualified source port modifies
only battle_command/party/state.cpp, battle_internal.h and three existing test
sources over the712-input isolated baseline. All14 relevant main inputs still
match the Closed checkpoint; utility/public battle/dedup/message owners remain
unchanged. Current IV/V/VI native hashes/unique ready bodies/typed fields match
the retained primary archive. Focused stage integration running; then canonical/
smoke/scan/recovery and guarded IV-only install. Estimate15–30min source/affected/
shipping checks, finite capped execution and owner closure wait separate. No
game probe or source-design reopening. Stage record ff4_party_turn_20261102/.
Isolated focused integration now passes all three actual consumers. The first
battle-focus port also included newer main choice-menu lifecycle assertions
for an unrelated owner absent from the isolated baseline, causing7 failures.
Its failed report/source are retained; the staged test restores the existing
baseline choice checks and retains every ATB assertion. No production choice
code is imported or changed, and the four production ATB files remain exact
qualified source. Focus/source-version adapted test hashes are in source_delta.
Full candidate/scan/guarded IV-only install remain pending; no gameplay claim.
First canonical failed15 legacy command-commit fixture expectations after all151
targets compiled: its IV/V/VI setup still called data preparation as if native
input were ready. Production remains unchanged. Every relevant direct setup/
turn-reader consumer was inventoried; commit fixture now uses the real ready
entry with all commit/cancel/synchronous-handoff/exception assertions intact.
Current all-six commit test passes. Failed source/report/295.053s322473984B
receipt retained. New _final warm candidate is frozen; same6170394f DLL and
22-file successful exact scan. Shipping remains guarded/pending. This is a
required test-version integration, not a source-design reopening or skipped
failure. Initial15–30min estimate is near its bound; final warm validation and
physical IV closure remain, with no further production rewrite required.
**IV candidate ready, November2 at12:24UTC:** final full build/smoke/scan binding
pass,712inputs/170freshreports/151targets/8317names0missing/143used350defined
0undefined/47exports/67resources/12languages. DLL11531776B SHA6170394f8218e38aa387e2e891ee21633f0e55462bbee59c044201b0069bfc56.
Four production files remain exact; only dependent test setup/isolated unrelated
choice coverage is adapted. Same runtime binary as already-scanned22file payload.
Guard reports FFIV PID13980 running; owner asked to save/close via async input.
Installation pending, not claimed complete; preserve frozen inputs in sibling
Class-Delivery/build_check/ff4_party_turn_20261102_final/. No game launch/probe.

**IV delivery Closed, November2 at12:28UTC:** owner confirmed saved/closed;
fresh guard allowed IV-only install.22payload hashes match6170394f candidate,
144protected files/other five games unchanged. Resource receipt
build_check/ff4_party_turn_deploy_resource.json:8.931s/115068928B/exit0/drained,
hard2GB/8.33%CPU/one worker. Final completion/install receipts in sibling
Class-Delivery/build_check/ff4_party_turn_20261102_final/. No game launch/probe;
natural audio/frame confirmation remains distinct. Owner told FF4 may reopen.

**Owner FFIV off-floor return regression, November2:** while in Kaipo32, the
remote Weapon Shop→Kaipo return exit is listed and says the player must travel
through Weapon Shop first. It must be absent off its own floor and present when
actually inside the shop. Actual FFIV game-directory log774592.703/774595.625/
774597.875 confirms the Weapon Shop case and the same Inn & Item Shop return.
Installed IV DLL SHA256 a6d9b4ce19dfb6adb33fb0051851814a62b4fcd0848c041c0ef23625b466998f.
Read bounded512KiB via shared handles; no tester log or game probe. Determine
installed-vs-current-source coverage, add this exact town/interior regression,
and audit equivalent return exits across all six games, preserving legitimate
entrances, onward dungeon exits and one-way transitions. Source status not yet
claimed; Levistone stays active. This is a human-triggered relevant audit case,
not authorization to refactor the accepted remote-filter architecture.

**Owner save-point category follow-up, November2:** confirmed in FFIV; owner
suspects V/VI. Save points currently appear in both Events and Save Points and
must appear only in Save Points. Audit all six applicable live/baked/remote
category producers and save-marker/tutorial/decorative counterparts; preserve
real save-point navigation and separate unrelated events. Distinguish missing
installed coverage from a current-source classifier gap with actual regression
checks. Source/integration/deployment not yet claimed. Queued behind urgent
Levistone and FFIV party-turn delivery; no game launch/probe or tester-log access.

**Active save-category milestone, November2:** make actual save destinations
exclusive to Save Points across all applicable live/baked/remote producers;
preserve separate map/story events and saved selection. Reuse established owned
worker classification/consolidation, no polling/new hooks or text deduplication.
Acceptance: actual IV reported identity/log case, all-six applicable real-data
inventory, duplicate/native companion distinctions, Events absent/Save Points
present locally and remotely, existing navigation regression. Boundary: field
save classification and its affected fixtures/documentation only. Estimate
20–40min investigation/source/focused evidence; capped execution separate. Other
queued returns/status and original audit remain separate. Deployment is later
combined qualification, not claimed by source-only checks.

**Owner-prioritized Levistone recurrence, November2 after IV installation:**
save-category investigation checkpoints here, without production edits. Actual
FFIV rotated .5 log776531.656/.532.593 confirms the same remote Underground
Waterway Save Point as Event and Save Point; existing consolidation handles only
FFVI tutorial companions. Resume this slice after Levistone, not concurrently.
Owner confirms installed cardinal Levistone route still falls when moving North
or East from its start. Latest actual FF1 log781564.875→781571.421 and
781587.437→781593.437 confirms cardinal instructions followed by B3 entry;
later attempts781600.593→781606.578 also fall. This supersedes geometric-only
acceptance; do not reuse it as physical safety proof. Native triggers and full
room entrances/trap-free path must be traced before another candidate. Scope:
minimal route/geometry/trigger correction plus exact affected consumers and
guarded FF1 delivery; cardinal-only remains required. Establish actual triggering
and movement/entrance semantics, compare all real pits and both sides, then
source/focused/canonical/security/install checkpoints. Estimate30–50min source/
offline investigation, capped builds/closure separate. No game launch/probe.
Actual topology now established:75 outside/four island cells, all9 island
boundary contacts,19 B2 pits plus B1hole20 native scripts. Normal outer route
requires B2→B3→B1→B2 island, then North/West, no teleport/half steps. Real B3
actor26/battle123/post-battle Hide is an explicit handback, not a skipped
obstacle. Main30 complete episodes/100transitions/50drops/24encounters/4116
cardinal steps pass; same corrected fixture rejects old bake75failures. Old
diagnostic fixture133 failures (double-counted raw Event/Warp and native layers)
and first after70 failures (unmodelled encounter) retained with explanations.
Five owned inputs changed; no runtime C++ redesign. Isolated712-input candidate
ff1_ice_physical_20261102 is running canonical/smoke/scan; source/focused evidence
and exact native-data locators in FF1_ICE_PHYSICAL_ROUTE_20261102.md. Installation
not claimed yet; current68bed68c remains until fresh guarded replacement.

**FF1 physical Ice delivery Closed, November2 at12:56UTC:** exact adb4a5c1 DLL,
712inputs/151targets/170freshreports/8317names0missing/47exports/67resources/
12locales; full/smoke/22-file scan pass.22 installed hashes/144 protected/other
five unchanged. Canonical390.849s, full402.159s/761212928B; install6.280s/
109187072B, hard2GB/8.33%CPU/one worker/exit0/drained. No game launch/probe or
gameplay success claimed. Topic FF1_ICE_PHYSICAL_ROUTE_20261102.md and sibling
Class-Delivery/build_check/ff1_ice_physical_20261102/. Owner told may reopen.

**Levistone gameplay Closed, November2:** owner explicitly confirms the installed
adb4a5c1 path is working. Prior B3 warning was for the still-required later B1
drop; latest actual log queued correct B3 directions to the selected stone.
No warning suppression/change needed. Keep the accepted route and evidence;
the separate stone label finding remains in the broader original audit.

**Owner Damcyan Jail regression, November2:** actual latest FFIV game-directory
log782364.296/782365.468/782368.312, current map42, gives6East/1North/5East/
5South/12West/1South then exit to Jail B1 for a remote unopened chest. Owner
cannot take the first East movement because an obstacle blocks it. Capture
shared/bounded game log first (done), then check exact current installed6170394f
geometry/live blocker/native layer contracts and reproduce the actual start
from route/exit facts. Do not ask hidden coordinates or assume a flat grid.
Queued directly after active urgent Levistone source/delivery; fix and retain
affected castle/jail routes before broader audit. Not fixed or deployed yet.
Now active after completed FF1 physical-route delivery. Scope: exact map42
origin/obstacle/layer/bake contract and shared affected routes, plus the owner
hovercraft wording row below; no unrelated routing refactor. Establish the
specific wrong native/baked fact before editing; verify affected castle/jail
journeys and vehicle-label consumers, then canonical/security/guarded FFIV-only
delivery. Estimate20–40min investigation/source/focused checks; finite capped
execution/physical closure separate. Existing map/bake/worker owners retained.

**Owner hovercraft wording, November2:** FFIV hovercraft's Antlion Cave B1 row
says Sail41.0steps then land. Use Fly for this craft in pathfinding browse and
directions; preserve Ship sailing and actual landing/boarding semantics. Include
with the queued FFIV navigation correction, localized through existing catalog.
Not implemented or deployed yet; active FF1 candidate remains frozen.
Source/focused now passes: all-six native10-owner traces show HiddenPassageGroup
changes tile UV animation only; physics remains independent. Routing formerly
opened955 actual wall cells under cover markers. Corrected shared physical masks
preserve3075 real hidden-floor cells, start Damcyan South and retain the entire
exit route. Main route_check reproduces7 failures before/0after. Actual browse
consumer passes all six games and real Antlion Hovercraft2/type7 from146,42 says
Fly21.0steps then land; current-frame native/latency confirmation remains open.

**Necessary bounded build-efficiency dependency, same owner request:** the
affected browse target took79.575s and recompiles the same13 common check sources
independently for many executables, despite identical compile settings. Before
the full changed-navigation qualification, share only those13 original object
compilations across offline checks. Preserve every executable/check and native
DLL build, per-executable runtime state, exact defines/includes/options, one
worker and hard2GB/CPU caps. No production architecture or Closed startup-group
redesign. Scope CMake check-source wiring plus affected documentation; verify
real consumers/link symbols/object reuse and unchanged compiler contracts before
shipping. Estimate10–20min implementation/focused evidence, capped execution
separate. Original grouping startup remains Closed; no PCH/new server/parallelism.
Common-object wiring now source/focused Closed: all13 generated compile contracts
match, three actual consumers pass129.769s/531828736B/exit0/drained. Eight scoped
files ported into712-input isolated ff4_damcyan_20261102 candidate; its canonical/
smoke/scan is active. Twelve affected main III–VI puzzle/route consumers run in
the second capped tree with frozen input hashes. No IV replacement yet.
Remaining compilation cost is the wrapper fixtures themselves. Read-only own-code
inventory finds53 direct wrappers and zero calls to their renamed base entry.
After current main qualification ends, test the existing entry guard's omission
of that unused body with a same-source single-TU baseline/after comparison plus
standalone base check. This preserves all actual checks and the accepted common
objects/startup design, no PCH/production architecture change or active-job edit.
Affected12-target build completed445.741s/476344320B/exit1/drained: III puzzle/
keys and IV journeys pass; an older FFV Library9-witness check fails5. All eight
remaining independent checks were then run and pass (Catapult/Ghido/Surgate,
Phoenix/Tower, Opera, Narshe light, Dream Train/first car). The failing Library
rooms275/281 have no native hidden-cover layer. An explicit owned-grid replay
of the old cover override still fails the same5 witnesses; do not attribute
this to the new physical rule without evidence. Actual complete Library guide
is now being checked separately, session54003. Retain failed reports and
distinguish a stale ordinary-route witness from the qualified stepwise guide;
full original audit remains open. No candidate installation before this safety
question is resolved. Diagnostic inspect.py renamed room_facts.py after its
name shadowed Python's stdlib inspect during one failed data read; first failure
and corrected source fact receipts retained, no game-source export.
Complete Library guide now passes441 profiles/2288 controls/521 episodes/
6946 operations/210 layouts/240 regions/5431 cells under the corrected rule.
The old9-witness fixture still fails5 with the former cover override, so its
disposition is a separate ordinary-route-fixture audit finding, not a demonstrated
shared-rule regression. Preserve actual Library design and full guide evidence.
New --legacy-cover diagnostic changes owned offline grids only, no native memory.
Source/focused wall/hovercraft qualification is complete; isolated full candidate
continues. No new installed IV wall fix or full original-audit closure claimed.

**Owner dialogue key-item opening, November2:** actual FFIV log785360.500 and
785375.031 speaks Cecil:Rosa!, then only785376.031 Sand Ruby /785376.484 Bomb
Ring after user navigation. Opening the dialogue-owned key-item screen must
announce its initial focused item as soon as the native list is ready, including
the Sand Ruby use on Rosa. Inspect actual menu producer/readiness/initial focus;
no polling, text deduplication or early unready reads. Queued after current
collision qualification. Not implemented/deployed; no question needed.

**Remaining original audit:** FFV402
and FFVI911 semantic/unique coverage, using the existing supported consumers.
Surgate's45–75min source/focused family is complete; capped execution/shared-slot
waits remain separate. Optional minigames stay excluded. Earlier whole-FFV90–150min
estimate has overrun; original all-six audit and final candidate remain open.
Original all-six final build/deployment/commit/push remains authorized, unfinished.
Sol only; one active system. Current two-slot/hard2GB-per-tree resource rule
applies; historical1GB receipts remain unchanged. Surgate facts, implementation
and acceptance: FF5_SURGATE_BOOKS_20261102.md.

November1 urgent FF1 delivery is **Closed**: class descriptions with I/Autodetail,
24 locked/story-door variants across six rooms, Melmond trench15 classification,
and animated dungeon destinations including Earth80 crystal34 and Sunken98
barrel19's NPC/Anim alias. Source/focused and isolated canonical/smoke/security/
recovery/guarded installation pass. Owner confirmed closure; FF1 installed at
15:19 UTC and owner told it may reopen. Do not ask for closure for this completed
candidate. DLL11502080B SHA256
2b7c5f50f0a1c30ba0367d36d67c0fcdeedc3eb33a553b4c8f6eb6d17e929284.
All22 installed payload hashes match;144 protected files and the other5 game
payloads unchanged.699 frozen inputs/165 fresh reports,8308 names/0 missing,
143 used/350 defined/0 undefined wiring,47 exports,67 resources and12 locales.
Final build4075.394s/930578432B peak; verification/deploy38.671s/124035072B,
both serial1 GB/exit0/children drained. Defender22-file scan no threats/0 matching
detections/stable hashes; exclusions unavailable/signing identity0. No game
launch/probe, dependency update, public release, commit or push. Exact source,
failed preparation receipts and recovery: FF1_GATE_FOLLOWUPS_20261101.md and
sibling FFPRCore-Class-Delivery/build_check/ff1_class_description_20261101/.
The earlier cache configure failure and deliberately cancelled pre-crystal
canonical remain preserved; they were never installed. Leather Cap screenshot
matches native Monk/Master empty-slot defense; no comparison reversal introduced.

**Current FF1 route checkpoint, November1:** source/focused corrections now
qualify in main and the isolated delivery tree. The isolated Giant consumer
passes75 assertions/eight complete journeys/16 native hops; the route walks54
steps through Giant81 instead of the installed112-step Ship detour. The actual
Giant NPC11 remains a story gate without Ruby17/opening18; Ruby ownership
validates the prospective passage while retaining its manual interaction.
Unused actor13 was an incorrect first assumption; its three failed assertions
and resource receipt remain in the topic record.

Native boarding evidence covers the four exact owners in all six games. Ship/
Submarine types2/6 require their landing table to accept the source shore.
The logged southern shore240,203 has attribute3 and is rejected;239,202 has1
and works. The old consumer chooses four invalid North approaches; the shared
predicate passes22 assertions/six routes with no invalid boarding side.

Levistone routing now uses four explicitly baked half-tile points across the
narrow gap. Independent bake verification preserves all19 native pit triggers,
finds minimum8px clearance and reproduces the full129-room bake byte-for-byte.
All six relevant room starts,12 fractional native-axis moves and complete B1
ordinary journeys pass. Main also checks its pending authored-safe API; the
isolated baseline deliberately lacks that API and reports the distinction.
The native input follower, repeated half-corner requests, beacon/browse agreement
and solid/directional vetoes pass; no gameplay/audio/frame claim is made.

The first isolated serial1 GB qualification passed the story author, independent
bake, Giant, boarding, Ice, native follower and all-six transitions, then stopped
on a mistakenly requested main-only target absent from the isolated baseline.
Its374.973s/512720896B exit1 receipt is retained. No runtime failure was inferred.
The existing baseline vehicle/world journey checks then passed in216.638s with
519872512B peak, exit0/drained. Their throttled fixture timings are not game
responsiveness. After five finite queue waits the canonical started in session37129.
708 inputs are frozen: the699-input baseline has exactly15 intended modified
inputs and nine additions, with the actual prior DLL and all modified prior
sources retained. Qualification reports and both resource receipts are hash-
bound in isolated_qualification.json. Do not change isolated src/data/tools or
either build.bat while compilation is active.
That full canonical compiled all151 targets, then failed two navigation_followup
assertions. Its invented FF1 Ship146,6 shore146,7 is native terrain3 and fails
the actual landing table, the same missing predicate as the owner's report.
The fixture now independently requires native landing/direction validity when
generating its identity witness; all original route/name/browse/missing-model
assertions remain. It explicitly proves the old witness is native-invalid.
Focused amendment passes all six-game cases with zero failures in59.801s,
495529984B peak/exit0/drained. Runtime, maps and build entry point are unchanged.

The failed canonical4584.039s/628228096B exit1 and exact DLL378838e78c5ada5d
are preserved under isolated build_check/ff1_giant_route_20261101/failed_canonical_1.
It was never installed. retry_route_candidate.py completed the fresh708-input
candidate under build_check/ff1_giant_route_20261101_final:151 targets and the
full canonical pass in400.662s,763187200B peak, exit0/children drained. Native
names8308/0 missing,143 used/350 defined/0 undefined wiring,47 exports. DLL
11525120B SHA378838e78c5ada5dd4cc1e473a19bf333612dd071fc8ff9bb733345027a14f85
is byte-identical to the first built candidate; the amendment changed only the
test source. Fresh report/resource/locale/smoke/security qualification is queued
in session94557, using finish_verification.py and the final candidate directory.
It changes only the one test source from the failed freeze, retaining16 modified
original baseline sources/nine additions and the actual installed2b7c5f50 recovery.
No new FF1 DLL has been installed. Next: smoke/resources/locales/security/recovery
and a fresh closed-game guard. Do not rerun
the original preparation helpers or overwrite failed evidence. The first full
build took76 minutes; the source-only fixture amendment should reuse compilation
outputs, but the same1 GB/8.33% CPU envelope remains mandatory.
The original source/focused estimate overran because exact native half-tile
movement and the real Giant prerequisite were unresolved; both now have pinned
source evidence. Finite capped build time/global-slot waits remain separate.

Hard rule: inspect relevant actual game-directory logs before asking the user;
never tester logs without explicit permission. Current shared-handle log reads
recover414442B despite a0B metadata listing, including map91 starts18,25 Upper,
23,18 Bottom and18,13 Bottom; the original quoted route began on map88 at12,7
Upper. Do not ask the user for normally undisplayed coordinates. Source topics:
FF1_GIANT_ROUTE_20261101.md and FF1_BOARDING_ICE_20261101.md.
FFV Library and all-six audit progress remain preserved and separate below.

Historical owner trigger: Sage's Cave exit is
correctly described as via Giant's Cave, but explicit directions detour to the
Ship and immediately land. Prioritize this before further FFV implementation.
Installed isolated source still has the older world-first-hop predicate that
forces vehicle assistance for every valid transportation record, including
Player1/type1. Main has the source-qualified all-six native Player correction;
the isolated urgent FF1 candidate intentionally omitted that then-pending shared
audit slice. Reproduce the actual Sage/Giant entrance journey, port only the
required proven correction and qualify both directions and list labels, legal
floor arrivals, closed passage and genuine mounted/vehicle cases. Estimate30–60min
source/focused, serial capped build time separate. Do not publish a new FF1 DLL
without fresh canonical/smoke/security/recovery and a closed-game guard.
Topic: FF1_GIANT_ROUTE_20261101.md. FFV remains at its preserved prototype;
no production Library adapter/resource has been written.

Owner added two connected FF1 route defects before the next candidate: requested
Ship North is physically unusable, while West then North then Ship East works;
and Levistone directions appear to use an unwanted Ice Cavern trap. Include
these in the same active source-to-output checkpoint, preserving the Giant
walking correction and real story gates. Do not start the next canonical payload
until boarding and Ice consumer evidence qualifies the expanded requirements.
Additional estimate45–90min source/focused; capped build time and external global
slot waits separate. Existing passive log pins Ship240,202, the rejected southern
approach240,203 and working western approach239,202; world Ice entrance192,176.
Narrow-passage dependency review adds30–60min source/focused: whole-tile cardinal
and diagonal models both omit the real half-tile path. Independent exact geometry
proof passes all19 pits, zero center/solid/wall crossings, minimum8px. Source now
authors four owned PrecisionPoint bake records and bounded precision phases for
directions/world path/cross-floor proof/filter/beacon/native-axis follower;
the first actual-consumer check ran in session88911, serial1 GB. This historical
stage predates the qualified port above; no gameplay confirmation was inferred.
Owner confirms a trap-free narrow approach exists. Actual game-directory logs
recover map91 starts18,25 Upper;23,18 Bottom;18,13 Bottom, and prior map88 start
12,7 Upper. Hard rule: inspect relevant game-directory logs before asking the
user; no tester logs without explicit permission. A shared current-log handle
reports414442B despite a0B listing. Preserve narrow physical movement, not a
guessed mandatory-drop exception. Source topic:
FF1_BOARDING_ICE_20261101.md. No new FF1 DLL compiled/installed after2b7c5f50.
The twenty-minute Giant final-check queue expired without a child; later native
boarding exports, precision diagnostics and consumer checks superseded that wait.

**Historical FFIII source progress before the Closed checkpoint above.** Owner explicitly
requires an owned qualifying key to validate access where possible. Inventory
identifies13 native Magic Key/Thief doors; eleven exact native owners now pinned.
Thief visitor tests job3 but its wrapper writes map-local scratch and logs;
never invoke those wrappers for navigation. Establish bounded read-only native
inventory/active-party equivalence and real door geometry/consumption budget,
then connect directions/browse/story filtering and manual unlocking prefixes.
Estimate60–90min source/focused, capped build time separate. Preserve exact
counts and stop at the first actual unresolved producer; no inventory scan or
main-thread path solver. Source/evidence continues in key_door_inventory_20261101.json
and tools/re/ff3_key_capability_contract.py. Source now connects bounded key/
active-Thief snapshots, exact door identities and two nested prerequisite costs
to current/remote obstacles and manual physical prefixes. First consumer check
passes215 chest profiles/153 complete journeys/55 unlocks/410 hops, zero worker
native reads. Door-destination expansion and shared integration are queued;
no new FFIII capability DLL installed yet. Underwater vehicle confirmed Nautilus; exact origin map still
unreported. Reuse qualified world, Mist, ATB and existing puzzle slices below.

The first shared batch passed native mapstate/37 key-reader cases/13-door
topology, then failed on a live-edited batch-file offset; its119.003s/514281472B
receipt is retained. The corrected resumed queue reached its five-minute bound
without a child/receipt because another project's version build owns the global
slot. No unrelated process is changed. FFIII code is held fixed; close the queued
integration when the slot frees. No uncapped retry is permitted.

After the active FF1 regression and queued FFIII integration, resume the
preserved FFV Library production adapter, then
remaining V/VI semantic/
unique-guide audit:7 complete native-shaped episodes and850-state/1963-edge/
357-observation original policy close with no dead states/loops/worker native
reads. No production FFV adapter/resource exists yet. Continue from
FF5_PUZZLE_GUIDES_20261101.md; do not restart Closed families. The new FFIV
item-description visibility guard and bounded FFV hidden-entity name read still
need affected integration checks. Original all-six final build/deploy/commit/push
remains authorized and unfinished. Optional minigames remain excluded. Sol only,
one active system, serial enforced1 GB and bounded CPU; no game launch/probe.

This is the single checkpoint, scope and Closed-system record; feature topics
hold evidence and link here rather than create parallel plans. Further work on
Closed families is limited to owner-triggered defects/minimal integration.

### Owner resume, October31: documented fixes, then complete the audit

The owner explicitly resumed implementation and the all-six map/story-puzzle
audit. This supersedes the documentation-only/pause restrictions on the queued
reports below; their original reports remain evidence. Sol only, one active
system at a time. All builds/heavy tools retain the serial enforced aggregate
1 GB ceiling, bounded CPU and below-normal priority. No game launch/control or
runtime probe is authorized. Optional minigames remain excluded.

Ordered finish line:

1. **Source/focused checkpoint qualified; native-context confirmation open: FFIII world navigation.** Fix the reported selection/direction lag
   across Doga and other route classes; complete legal dive/surface and
   underwater-through routing; investigate repeated moving/diagonal aircraft
   requests while retaining worker-owned searches. Reproduce the actual
   provider/input/capture/planner path, not only manually assembled route goals.
   Estimate60–120min for this source/offline checkpoint, build time separate.
   Permitted boundary: shared navigation and FFIII native adapters, their
   existing actual-consumer checks and affected data/docs. Establish pre-change
   cost/correctness evidence; preserve story/vehicle gates and the Eureka fix.
2. **Source/focused Closed; combined delivery pending: FFIV Mist vehicle classification.** Correct legal walking
   through Mist Cave and audit the shared classification's applicable adapters.
   Demonstrated native Player ID1/type1 counterexample: an ordinary valid
   transportation record wrongly forced boarding after the cave graph already
   proved a walking leg. Eight post-boss failures reproduce the exact label;
   minimal shared correction passes48 route/browse pairs and two full cave
   continuations with four unopened chests and the exact northern arrival.
   All six native Player first hops,98 boarded cases,582 exit candidates and
   native-Player parked-craft regressions pass;1200 category inputs have no
   redundant scan backlog. Frozen14 inputs/five reports and resource receipts:
   build_check/navigation_resume_20261031/mist_checkpoint/. No new DLL installed.
   Estimate30–45min source/offline, capped build time separate.
   [Cause/evidence](MIST_WORLD_FOOT_20261031.md).
3. **Source/focused Closed; combined delivery pending: ATB character-turn speech.** Correct automatic character-ready
   announcements and party-only T output through actual command-menu ownership;
   audit all applicable ATB adapters and preserve unrelated battle behavior.
   Estimate45–75min source/offline plus capped build time. Boundary: shared
   battle turn state and native command-menu ownership, affected producer/
   speech/ATB adapters and meaningful actual-consumer regression checks.
   Baselines reproduce12 automatic/re-entry and21 overlap failures. Native
   current IV/V/VI command-input entry bodies/typed root fields are hash-matched
   and unique. Source now admits ATB turns at that actual ready entry, resets
   the local guard on accepted commands, and gives T the ready party owner.
   All four command/party/equipment/action checks pass;33 demonstrated assertions
   are corrected. Actual native readiness, hidden rows and battle teardown are
   also covered. No new DLL installed. [Contracts/evidence](ATB_COMMAND_TURNS_20261031.md).
   Final15-input/three-executable source checkpoint:
   build_check/atb_turn_fix_20261031/qualified_checkpoint/. Native readiness,
   initial visible-command ordering/64-slot bound, old-focus preservation and
   lifecycle regressions pass. Source/focused receipts remain separate from
   pending canonical/delivery and natural audible confirmation.
4. **Active/resuming preserved work: all-six map/story audit completion.**
   Reuse Closed map/gate and puzzle slices; finish semantic triage of the1634
   broad mechanic candidates, implement missing required step-by-step Shift+G
   guides, and fix actual reachability/story-gate findings. Preserve the native
   Opera/Phoenix/Tower/Narshe proofs and optional-minigame exclusion.
   Also qualify mounted world-to-interior first hops explicitly; existing98
   same-world boarded cases alone do not establish that distinct adapter path.
   Mounted first-hop source/focused slice Closed:50 actual local-entrance to
   interior-arrival profiles across all six. Reproduced two discarded legal
   routes (FFV Boko/FFVI Magitek); minimal native ground-entry correction now
   passes50/50 plus native Player, Mist and98 boarded/582 exit regressions.
   [Evidence/limits](MOUNTED_WORLD_HOPS_20261031.md); final serial job151.844s,
   peak484921344bytes, exit0/children drained. FFIII saved-vehicle singleton
   names match exact metadata; that hypothesis is rejected, not a found defect.
   Active finite slice: semantic triage by actual mechanic family, then complete
   missing control/sequence guides and their entrance/intermediate/reset/end
   scenarios. Estimate90–150min source/offline, capped builds separate. Reuse
   Closed Narshe/Opera/Phoenix/Tower/static guides rather than restart them.
   Additional simple-control slice source/focused Closed:14 new III/IV guides,
   35 operation receipts/32 recipes,14 closed-state native entrance approaches,
   35 binary identities and12 full Citadel resumes pass. Evidence frozen in
   build_check/puzzle_audit_20261030/additional_controls_20261031/; no deployment.
   Active family: FFVI Dreamscape train (timed crates, chest memory, levers),
   exact native state/route semantics before authoring and full-episode checks.
   Its maze prototype passes24 native wall-state comparisons,64 native chest
   patterns and16 complete maze/exit episodes (58 production route legs).
   Moving-object integration source/focused Closed:3 off-grid live Entity
   destination failures corrected;26 actual-consumer pose/guard cases, six
   stationary game witnesses and shared/mounted regressions pass. Frozen evidence
   in build_check/puzzle_audit_20261030/moving_object_checkpoint/. Ordinary
   opened-chest navigation is preserved. Complete train guide authoring and
   controller integration, timed crate/retry and localization remain active.
   First-car/model checkpoint now passes173 states/187 references, ten actual-
   type episodes with50 route legs/three movement refreshes, plus the preserved
   762 maze legs. Native second pull stops the four-second timer and holds the
   crates; guidance preannounces both operations, uses actual treasure receipts
   and survives timeout/local reset. Full522–527 authoring and13 instructions
   in12 languages are complete. Frozen23 inputs: dream_train_first_car_checkpoint/.
   Two stack-overflow fixture runs and a moved-helper compile failure are kept;
   separate sequential default-stack workers pass without raising the1 GB cap
   (462966784B/92.546s). Active finish line: actual-type maze, real car arrivals/
   departure and controller/shared integration, then source/output closure.
   Full Dreamscape train family is now source/focused Closed: ten first-car
   episodes, eight actual-type maze episodes and seven car/engine handoffs
   pass94 route legs/three native movement refreshes; preserved762 maze legs,
   controller/lifecycle, Phoenix/Tower, Opera and35 static controls pass. All13
   instructions cover12 locales and the four checks are canonical-wired.
   Frozen36 inputs: dream_train_family_checkpoint/. Shared job124.096s,
   peak499576832B, exit0/drained. No new deployment. Active: remaining all-six
   semantic triage. Independent animation/state discovery adds51 identities
   (I9,II1,III1,IV2,V15,VI23), with zero overlap and identical primary source
   hashes to the preserved1634 baseline. Supplement:
   docs/re/puzzle_mechanics_animation_inventory_20261031.json. Review both;
   discovery counts are not an exhaustive-solution claim.
   Active finite family: FFI Earth Rod and Lute stone slabs. Exact native owners
   identify the operative slabs, excluding same-cell decorative variants. The
   Rod guide includes guardian victory16, actual chest acquisition17, Giant18,
   Sarda19 and slab20; Lute slab9 uses completion37. Explicit authored-only
   world transit corrects a reproduced fetch/return rejection while preserving
   ordinary indoor route policy. Estimate30–60min source/offline plus capped
   builds. Qualification must replay complete production routes and native
   arrivals, prerequisite resumes, cancellation, controller receipts and12
   languages before source/output closure; no deployment yet.
   Corrected native-arrival replay now passes nine complete slab episodes/84
   production legs and41 static operation controls in34 recipes. A further
   actual-consumer policy switch reproduced a kept-route leak: the ordinary
   same-target request borrowed the guide-only world detour. Active/pending
   route identity now includes the opt-in; before failure and after success are
   preserved. Shared route-worker regressions pass; mounted-distance/underwater
   integration remains running. Fresh Citadel1F entrance coverage must also be
   checked: the preserved12 maze resumes cover actual2F arrivals, not that
   earlier entrance. No claim of complete FFI puzzle coverage yet. Topic:
   docs/re/FF1_SLAB_GUIDES_20261101.md.
   FFI source/focused slice now Closed: Citadel's initial elder8/native44 was
   missing from the guide and is now included. Ten complete episodes/95 real
   production route legs pass, alongside42 operation controls/34 recipes and
   twelve preserved2F maze resumes. Seven instructions cover12 locales. All52
   FFI candidates have explicit semantic dispositions, exact identities and
   matching primary-source hashes in puzzle_semantic_review_20261101.json.
   Shared route-worker, twelve mounted-distance witnesses and eighteen water
   interior pairs pass after the cache-policy correction (236.919s/496308224B).
   Final guide/controller/static job90.740s/486920192B, exit0/drained. Current
   original source/data/reports are frozen in ff1_guides_checkpoint/. No game,
   new DLL or deployment. Active next: FFII hidden passages, item/keyword gates
   and existing Deist guide against exact native decisions, then remaining
   games. Estimate30–60min source/offline plus capped builds for that game slice.
   The broad semantic audit has exceeded the earlier90–150min estimate; do not
   weaken its acceptance or call the remaining five games complete from counts.
   FFII Deist family source/focused Closed: reproduced premature completion at
   B5 PointIn1; now guides all seven native operations through actual Life
   Spring41/25,8 and egg placement46. A bounded explicit-request learned-word
   reader uses native typed field90, with malformed/other-game/owner guards;
   movement performs zero word scans. Keyword-menu repeat/cancel uses owned
   text only. Nine full journeys/130 production legs, three moving-boy captures,
   zero B3 pitfalls and zero worker native reads pass. Controller49 operations/
   34 recipes, static identities and shared input checks pass. Native travel
   flag40 and all three Wyvern variants were corrected in fixtures, not weakened
   in production gates. Seven instructions/two official terms cover12 locales;
   four recovered binding tables and the exact native producer/DLL are pinned.
   Job89.411s/486166528B exit0/drained; canonical checks are wired. Freeze:
   deist_checkpoint/. Topic: docs/re/FF2_PUZZLE_GUIDES_20261101.md. Remaining FFII
   Four additional FFII families are now source/focused Closed: Fynn
   incantation, Kashuan Sunfire, Mysidia masks and Dreadnought escape. Nineteen
   complete journeys/135 production legs pass, including three legal patrol
   capture updates. Bar helper19 reproduced No path; actual bars28 correct
   the guide target. Guard instructions include Pass or Wild Rose plus battle.
   Exact final escape35 outranks earlier dependencies. All65 controls in38
   recipes, static/entrance/resume, input and route-worker regressions pass;
   native escape chain/bindings and21 instructions/12 official terms in12
   locales pass. Freeze ff2_gates_checkpoint/; same FFII topic. No deployment.
   Active finite slice: finish remaining FFII hidden controls and explicit
   dispositions for51 original/animation candidates plus63 independently found
   keyword callers, then III–VI. Reuse the completed guide/route slices.
   Estimate30–45min for this remaining FFII review, capped builds separate.
   FFII semantic/physical slice is now source/focused Closed: all51 baseline/
   animation and63 independent keyword-caller dispositions match exact source
   hashes. Four more guides cover Paul88→114, Semitt22, Coliseum48→49→50 and
   mirror73, preserving native story prerequisites. Paul's clue actor retires
   at75; unfinished clue guidance now reports Story gated without an obsolete
   route. Existing clue88 remains usable later. Native Cid76 is a manual NPC
   route handoff, not passage completion. Final31 gate episodes/154 legs, three
   patrol waits/one story handoff and72 controls/42 recipes pass. Deist9/130 is
   preserved. All26 instructions/14 exact terms cover12 locales. Final shared
   guide/static/source job59.58s/486117376B; handoff job37.601s/434622464B,
   both exit0/drained. Freeze ff2_remaining_checkpoint/. No main DLL deployed.
   Active next finite slice: FFIII157 original/animation candidate dispositions
   and remaining Mini/Toad/Nepto/Eureka unique mechanics, then IV–VI. Estimate
   45–75min source/offline plus capped builds; reuse the Closed map/gate slices.
   FFIII first form-family integration now passes26 frontier cases,27 actual
   controller/menu/cancel/circuit cases and17 full Nepto/Owen episodes/133 real
   production route legs. Nine form actions/zero worker native reads. Native
   Owen42 is independent of wall225 and gates the top-floor boss; an opened-wall
   unavailable-destination failure is corrected by retaining that required event.
   Seven instructions/five native terms cover12 locales;76 static controls in46
   recipes pass. Four new static entrance witnesses now pass (18 total), with
   source/locale contracts,76 identities and12 Citadel resumes. This Nepto/Owen
   plus four-control slice is source/focused Closed; freeze
   ff3_nepto_owen_checkpoint/. Final job43.583s/473579520B exit0/drained.
   Job74.789s/460877824B exit0/drained. Source topic:
   FF3_PUZZLE_GUIDES_20261101.md. The condition census identifies8 scripts/9
   predicates, including Tozus/lake/Hein/Magic Circle families still open.
   Continue those and157 FFIII candidate dispositions, then IV–VI. No deployment.
   Next finite slice: remaining native Mini/Frog passages at underground lake90,
   Doga Manor133/Magic Circle, Hein194 and Tozus, with exact source/story/form
   receipts, then FFIII semantic closure. Estimate45–75min source/offline plus
   capped builds. Preserve the qualified frontier/owner design and closed routes.
   FFIII remaining-family and semantic source/focused checkpoint is now Closed.
   All157 candidates and8 exact condition callers have explicit source-hashed
   dispositions. Six families/32 actions include Tozus, Hein prison, lake/Horn
   placement and Magic Circle. Mixed casts target only affected/missing members.
   Reproduced Nepto32/33 resume now targets interactive Eye5, not retired boss6
   or cutscene7. Final42 episodes/222 route legs/27 form operations,52 model and
   51 owner cases pass; no worker-native reads. Twelve-language authoring and
   shared guide/circuit/input pass. Qualification61.690s/465076224B; semantic/
   shared close23.730s/123883520B; both exit0/drained. Source/output freeze:
   ff3_remaining_checkpoint/. Earlier Nepto/Owen checkpoint remains immutable.
   Active next finite slice: FFIV112 candidates, item gates and magnetic-cave
   mechanics, exact native receipts/entrance/resume/end guidance, then V/VI.
   Estimate45–75min source/offline plus capped builds. The expanded semantic
   review has exceeded the original all-game estimate; this is required missing
   guide coverage, not reopening Closed route designs. No game launch or new DLL.
   FFIV source/focused semantic/unique puzzle checkpoint is now Closed:112
   candidates/18 item callers with exact hashes; six new families/ten operations.
   Full magnetic sequence includes separate initial fight, automatic harp97,
   re-equipped boss36 and animated crystal98. Baron31/33, Babel47/48 and sealed57
   availability/retirement are native-qualified. Scoped FFIV item-selector owner
   supplies speech/I and safe owned Shift+G/menu repeat; generic deferred rows
   recheck that owner. No text deduplication, game writes or worker pointers.
   All23 journeys/33 route legs,15 owner cases/preserved FFIII reader, shared86
   controls/52 recipes,18 earlier entrance witnesses,12 Citadel resumes and8330
   reference names pass. Final native/source job10.086s/261992448B; native/owner
   chain68.993s/456949760B; post-scene pose qualification38.290s/432295936B, all
   exit0/drained under unchanged1 GB cap. Freeze ff4_guides_checkpoint/; topic
   FF4_PUZZLE_GUIDES_20261101.md. Initial stale-count, incomplete57 fixture and
   pre-regeneration reports are preserved; no production route change for them.
   Active next finite slice: FFV402 broad/animation candidates and missing unique
   environmental controls/sequences, then remaining FFVI911 candidates/families.
   Estimate90–150min per game source/offline plus capped builds, subject to
   distinct native mechanism complexity; no reopening of Closed designs.
   Active FFV Library slice: five failures across nine direct target witnesses
   establish missing intermediate layouts. An external-native prototype passes
   all10 native entrance/395 cases (main9/10/15/16/20, lower13/15/14), zero worker
   native reads. Narrow return shortcut21 coordinate proof preserves the global
   projector rule; three native placements equal existing cells. First fixture
   Show assumption was corrected to unchanged visibility and retained. Final
   return comparison38.634s/453865472B, exit0/drained. Native programs/disassembly
   stay outside Git. No production guide/policy yet. Remaining source finish:
   full hub277 regions/floor resets, flagless encounter and49/50/return episodes,
   then bounded owner/controller/localization/source-output qualification.
   Estimate45–75min for this Library source/offline slice plus capped builds;
   all402 FFV and911 FFVI semantic decisions remain separate active work.
   Topic FF5_PUZZLE_GUIDES_20261101.md. No main DLL/game probe/deployment.
   Combined build,
   smoke/security/delivery and natural gameplay remain pending.
   Maze/chest model integration checkpoint now passes64 complete initial-pattern
   episodes,762 production routes,161 identities/156 states and preserved
   controller/Opera/multi-party checks;7 new instructions cover all12 locales.
   Freeze: build_check/puzzle_audit_20261030/dream_train_model_checkpoint/.
   Active minimal regression: longer mounted approach→interior parity corrected
   five reproduced failures (I/II ships, IV black chocobo, V ship/black chocobo).
   Existing nearby50 cases stay qualified; all eleven distance witnesses and
   the five-check shared chain pass. Exact FFIII Event2/scripted entrance
   observations then reproduce a submarine-to-Temple interior mismatch; riding
   reach corrects it, passing all twelve longer profiles. No foot-mask defect
   was inferred from the earlier GotoMap-only missing witness.
   Actual final-interior consumers expose eight additional water-room failures
   from Surface/Continent and five Doga-interior failures. The manual-world
   adapter now proves the native indoor continuation and retains the final
   destination. Owner capture also lost saved craft outside World map filter
   (one reproduced failure, corrected). Full dive/surface replay catches one
   extra Village loop after surfacing: the adapter must defer to the now-legal
   ordinary entry rather than force another dive. Correction is in the active
   five-check integration run. Estimate20–35min has been exceeded because these
   concrete continuation/capture counterexamples expanded the necessary fix.
   Source/focused slice now Closed: five-check integration passes,272.363s,
   486039552B peak, exit0/children drained. Eighteen actual interior route/browse
   pairs, five moving repeats, six dive/surface arrival continuations, missing
   craft/cancellation/owner guards and the capture regression pass. Preserved
   twelve world-marker pairs, twelve longer/fifty nearby mounted profiles and
   the98 boarded/582 exit corpus also pass. Both new checks are wired into the
   canonical build. Exact native departure-save producer and IMapAccessor slot4
   prove grid-cell X/Y units; that suspected ABI defect is rejected. Frozen26
   current inputs/reports/receipts: navigation_resume_20261031/
   underwater_interior_checkpoint/final/. No deployment. Resume active FFVI
   first-car/full-train authoring; keep this navigation design Closed.
5. **Pending combined delivery.** Complete affected documentation, canonical
   source/output/ownership/cancellation/resource/language checks, smoke and
   exact-payload security/recovery verification, then guarded all-six deployment
   when games are closed. Record implemented/offline/gameplay states separately.

The first checkpoint closes only when real request/publication paths and
representative route classes pass correctness and bounded-work checks. Native
frame/audible timing stays user-side; the cumulative25ms game-lag blocker still
applies. Unrelated historical issues remain outside this finite assignment.

FFIII checkpoint progress: ff3_world_request_check uses all four real underwater
entrances plus both Doga doors from world_objects_snapshot through actual route/
browse consumers, preserving entity ID/raw identity. All12 pairs, latest-pose,
vehicle-change and owner checks pass. A repeated moving dive request previously
lost its manual arrival receipt: reproduced failure, minimal receipt-preservation
fix, now passes. No legal-snapshot reproduction of the owner's underwater No path
yet; native context remains an explicit gap, not permission to weaken gates.
Original source and intermediate approaches are preserved under
build_check/navigation_resume_20261031/. Reverse adjacency agrees with actual
successors over262144 cells. World layer normalization dropped native layer1
sparse blockers:15 failures before, zero after across24 worlds/39 profiles and
2257408 distance cells. Weighted queue and A* tie experiments had negligible
benefit and were retired; their evidence remains preserved.

Timing under the enforced CPU/RAM budget: reverse water search and earlier
qualified Village fallback reduce Surface Village roughly360–410ms to35–42ms;
underwater roughly43–55ms to17–20ms. Manor remains roughly60ms Surface/227ms
continent; continent underwater roughly112ms. Router function profiling proves
ordinary searches dominate the remaining cost. These are fixture compute costs,
not native game-frame/audible responsiveness. Latest profiling job48.9s,
466788352bytes peak, exit0/children drained. The60–120min source estimate is
exceeded; remaining bounded work is the measured searches, stationary category
reuse, affected integration qualification and unresolved native-context record.

The moving-request source counterexample is confirmed: the request sampled
origin before multi-frame capture, while still_current requires an exact tile
match at publication and restarts otherwise. An owner-only latest-pose sample
now runs at the end of browse/direction capture; original vehicle validation and
strict stale-result rejection remain. Fixture cases cover diagonal capture-age
drift, changed vehicle and off-owner abstention, all passing. This does not claim moving-flight or
native responsiveness solved; planner/queue time can still retire a result.

October31 source/focused checkpoint: all ten serial actual-consumer integration
checks pass, including all-six route workers,98 boarded-vehicle cases,582 world
exit candidates and63 aircraft landing witnesses.84 original/adaptive search
pairs retain exact turn/distance/mountain-action objectives. Stationary worlds
1/3/4 pass900 category inputs without recapture backlog; the post-directions
consumer initially failed, its remaining world1-only branch was corrected, and
now passes. Frozen13 inputs and all ten reports are in
build_check/navigation_resume_20261031/zero_action_source/.

Representative throttled selected requests now: Manor Surface46ms/continent164ms;
Village Surface40ms/continent102ms; underwater Surface20ms/continent74ms. Actual
1524-border observation0.911ms aggregate/5.652ms full fixture episode, zero Unity
transform/name calls. Fresh world-category preparation remains1181ms under the
CPU quota (warm category changes reuse views). This is a visible remaining cost,
not evidence of natural-game instant response. The reported underwater No path
still needs the actual origin/vehicle context; the owner question remains pending.
No game probe is authorized and legal-snapshot success does not resolve that
native-context gap. Preserve these limits for combined delivery/natural retest;
continue the independent Mist/ATB/audit scope. No new DLL/deployment yet.

### Closed owner fix, October31: remote Eureka weapon availability

Status: source/offline/security/FFIII delivery Closed. Implementation,
158-report canonical verification, smoke,67 resources/12 languages and22-file
exact Defender scan complete.
Candidate11,495,936bytes/SHA256
5bb84691fc69a688b8ec5ccdf48ca22fd57644ea79ca2018dd716991d9909052.
Owner saved/closed FFIII; guarded FFIII-only installation completed. All22
installed files match,144 protected files and the other five games unchanged.
Deployment117,411,840bytes/22.018s,exit0/drained. No game launched/probed;
natural cross-floor case confirmation pending. Exact source/output and876-file
installed evidence checkpoint: build_check/eureka_fix_20261031/verified_candidate/.
Preserve sibling FFPRCore-Eureka-Delivery and its frozen690-input source record.

Owner reports all Eureka weapon objects, including Staff on B7, say Destination
is no longer available from other floors/the beginning of Crystal Tower. The
same destinations return valid paths on their actual floor. Scope: reproduce
the actual remote-provider-to-selected-route mismatch, preserve native active/
consumed guards and correct exact object identity, qualify the minimal fix and
deliver FFIII separately from the paused puzzle work. Estimate30–60min including
bounded build/check/release uncertainty. Sol only;1 GB aggregate/serial guard;
no game launch/probe. Doga lag, underwater routes, moving-flight requests and the
broader puzzle audit remain paused. Cause confirmed: the installed remote check
excludes AnimEntity; the target-floor native fallback masks the exclusion.
An isolated delivery checkout recovers all688 installed-build inputs by SHA;
only one runtime condition changes. Paused source/data are not included.
Actual provider/planner:50 remote+5 local routes pass, five native retirement
guards, identity mismatch and closed key gates preserved. Focused job peak
429,690,880bytes/46.530s,exit0/drained. The earlier all-zero open-door expectation
and incomplete guard-witness fixture failures remain preserved separately from
the actual50 remote failures. Canonical/smoke/security checks now pass;
FFIII-only installation is complete. [Detailed evidence](EUREKA_REMOTE_WEAPONS_20261031.md).

At06:54LA the30–60min estimate was exceeded by the fresh isolated canonical
build. Compilation is progressing under the unchanged1 GB aggregate cap,
one worker and bounded CPU. No scope or investigation has been added; remaining
work is the canonical checks, smoke/security verification and guarded delivery.

Cold-build integration blocker: the long MSBuild run hit the hard memory
ceiling after128 linked targets (exit1,3878.880s; failure evidence preserved).
No cap increase. The minimal build-only correction launches each of all142
ALL_BUILD dependencies in a fresh process serially within the same enforced
job, preserving valid outputs. The bounded canonical build completed using
this driver: all142 targets; resumed compilation peak887,574,528bytes, final
canonical750,182,400bytes/289.396s,exit0/drained. One absent historical FFV
comparison fixture was restored by SHA; existing15-case metadata verified.
Scan's incompatible PowerShell host was replaced with the verified installed
host, preserving the failed attempt; no protection changes. This necessary
integration correction does not reopen the audit. All implementation/offline/
security/delivery work is complete. Other five games and paused audit source
remain unchanged.

### Owner gameplay confirmations, October31

Owner confirms FFII and FFIII are 100% playable with the mod features and map
bakes. FFI is tentatively 100% playable to completion; its completed playthrough
used the old mod. [Gameplay record](../TESTING.md#owner-gameplay-confirmations-october-31).
Specific documented route/performance reports below remain preserved and paused;
the broader all-six puzzle audit remains paused. The newly installed Eureka
correction retains its specific natural-play case as pending.

### Owner report, October31: ATB character-turn announcements

**Document only; separate battle follow-up. Investigation/fix not started.**
Defer until the owner resumes work with more usage; the audit remains paused.

Reported symptoms across games using ATB:

- A character's automatic turn announcement is often silent when their command
  menu appears and they are ready to act.
- Enemy turns are also being tracked, causing problems with identifying the
  active party character. The owner suspects that when a character becomes
  ready during an enemy action, the announcer retains the enemy as the active
  turn even though the character's command menu is open. This causal explanation
  is the owner's hypothesis and remains unverified; the owner believes much of
  the intended character-ready behavior is already implemented.

Required behavior for the deferred ATB fix:

- T's spoken turn information must include party-character turns only.
- Automatically announce the correct character promptly when they can act,
  identified by their command menu appearing.
- An overlapping enemy action must not suppress that announcement or replace
  the ready character's identity with an enemy in spoken turn information.
- Timely, accurate actor identification is needed to make command selections
  promptly. When work resumes, specifically verify a party command menu opening
  during an enemy action and T's party-only output in that situation.

No source investigation, implementation, build, test, game probe or deployment
is authorized by this documentation request. Other battle announcements are
outside this reported turn-announcement issue; non-ATB behavior is not audited.

### Owner report, October31: Mist exit marked Requires vehicle

**Document only; investigation deferred until the owner resumes work.**

FFIV pathfinder announcement reported by the owner:
"Exit to Mist (Requires vehicle) - Map Exit (11 of 53)".
The owner suspects the vehicle requirement is incorrect: the destination
should be reachable on foot by routing through Mist Cave.

Future investigation: check the legal route through Mist Cave and the
Requires vehicle classification. Expected behavior, when current story gates
permit that walking route: directions should guide through the cave to Mist
without requiring a vehicle. The starting map, exact story state and cause
remain unverified. No code change, investigation, build, test, game probe or
deployment is requested now. The broader audit remains paused.

### Owner report, October31: Doga lag and underwater world-map routes

**Document only. Audit/fix work remains paused to conserve usage.** No new
investigation, implementation, build, check, game probe or deployment performed.

- Selecting Doga's Village in the FFIII pathfinder causes significant lag.
  Selecting Doga's Manor has the same problem. Owner clarification: other
  destinations also lag; these two are the biggest offenders, not the complete
  affected set. Exact timing, cause, selected
  vehicle and action breakdown are not established by this report.
- When the owner resumes this fix, check other routes as well; do not limit
  regression coverage to these two destinations. Retain the cumulative25ms
  mod-caused application-lag blocker and distinguish offline timings from play.
- Owner confirms world-map mode works between Floating Continent and Surface
  World. This supersedes the earlier blanket cross-world-gameplay-untested note
  for that connection; it does not establish every vehicle/route/direction.
- Underwater maps appear in world-map mode, but requesting their routes returns
  "No path". Underwater routing is therefore a reported gameplay failure despite
  the prior source/offline journey checks.
- Required future behavior: full legal diving and surfacing support, with
  underwater maps usable as intermediate route legs as well as destinations.
  Routes must be able to pass through the underwater map to reach places such
  as Doga's Village, respecting actual story/vehicle access and native exits.
- Potential additional pathing issue: requesting directions multiple times
  while moving in a flying vehicle, especially during diagonal movement. The
  exact symptom/reproduction sequence is not yet characterized. The owner
  suspects avoiding hotpathing may contribute; this remains an unverified
  hypothesis and needs investigation when resumed.
- Preserve pathfinding off the main game thread. If no improvement is possible
  without returning pathfinding to that thread, the owner accepts this movement/
  repeated-request limitation. This is not authorization to move searches onto
  the game thread. No investigation is requested now.

Future acceptance, after an authorized fix: responsive selection/directions for
both Doga destinations and other routes; usable Floating Continent/Surface
continuation; underwater destination and through-route plans with real dive/
surface steps and the Doga's Village approach. Preserve prior valid evidence;
do not treat prior offline passes as confirmation of these reported failures.
Also characterize repeated direction requests during flying movement, including
diagonals, and determine whether a safe worker-side improvement is possible.

**Paused by the owner's explicit request, October31 11:42UTC.** Save usage for
possible FFIII progression blocks. Stop audit work; do not automatically resume,
build, run checks or deploy the puzzle candidate. Source changes and focused
evidence remain preserved. No game launch/probe occurred and no audit job remains
running. Checkpoint: build_check/puzzle_audit_20261030/owner_pause_20261031/.

The installed FFIII correction remains11c25405fff5f1eb/11,495,424bytes. Other
games retain the prior installed candidate; none of the new puzzle changes are
installed. Natural FFIII responsiveness/progression and cross-world mode remain
gameplay facts. Keep the1 GB aggregate hard maximum and serial resource guard.

Capability at pause, started October31 11:11UTC: finite all-six unique map/story
mechanic inventory and exact remaining solution gaps. Estimate30–60min for the
inventory/triage, followed by individually scoped implementation checkpoints;
do not equate candidate counts with solved puzzles. Reuse the Closed map/gate
audit and completed puzzle slices. Combined release remains open. The owner
answered the scope question: map and story puzzles; optional minigames excluded.
The initial inventory is saved at docs/re/puzzle_mechanics_inventory_20261031.json
with hashes for six source sets, labels, stateful physical-script effects and
named interpreter-call discovery.1634 broad candidates include ordinary loot,
scenery, travel and narrative events; they are not1634 puzzles or completed
solutions. Counts I–VI:43/50/156/110/387/888. Semantic triage/remaining guides are
unfinished. Two unparsed FFV script records are explicit; no current candidate
uses either. Latest inventory job101,978,112bytes/4.472s,exit0/drained. Preserve
the two diagnostic parser failures separately; no resource failure/cap increase.
Next work only if the owner resumes the audit: finish semantic family triage,
then finite complete solution slices (including remaining FFVI chest/levers,
timed ceilings and other map controls). Do not start a broad redesign from these
candidate counts. Canonical build/smoke/security/all-six deployment remain open.

Owned choice repeat/cancel is source/focused-offline Closed at11:11UTC, within
its30–45min estimate. Existing native choice Show/Close publishes one flag;
map/battle/title retire it/generation, while additive scenes preserve ownership.
Static repeat copies its owned instruction during choices/dialogue, including
a lingering ready field flag, with zero puzzle-state reads or route jobs.
Static start still requires the ready field. Cancel works during blocked scenes
and retires both guide owners if necessary. A new static guide retires an older
module continuation. Opera retains its distinct native event qualifier.
Once per existing input tick, copied repeat/stop availability qualifies only
those bindings. Native input paths do not query native guide/game state; typing,
foreground, modal/interception, remapping and held-key release guards remain.
Controller choices expose repeat/stop; movies/loading expose stop only. Normal
battle/menu controller routing remains. Seven production-consumer checks pass:
core guide, transition/input,10-path interception, actual choice/battle focus,
circuit, Opera producer and FFVI story runtime.29 frozen inputs/reports:
guide_input_checkpoint/. Budget receipts: first314,425,344bytes/66.893s;
shared348,041,216bytes/65.965s; final owner-integration258,547,712bytes/14.707s.
All exit0, childrenDrained true; native frame/audio unmeasured. No new install.

Existing18 static recipes' operation/identity correction is source/focused-
offline Closed at10:51UTC.21 exact controls in III–VI require native scenario
bits, Zozo's exact current-room local1=3, or the flagless V control's retained
native Hide state. Proximity/unknown state cannot complete operations. Controls
route by exact ID/raw name/layer and Interact versus Spot semantics. Completed
native stages are skipped on resume; stale callbacks retire after advancement.
No full snapshot/Mini observation or native actor scan is added to movement.
The V retained-actor lookup runs only on explicit repeat, refuses lists above64
before calling native GetFieldEntity, and rejects missing/destroyed/mismatched
actors and hidden parents. Exact FFV source279020/EF6740 remains external.
Zozo now targets actual386/entity11, not the clue clock387/entity3. The false V
second switch is removed. FF1's return-warp endpoint is corrected to actual3F
entry Upper35,32;12 real intermediate arrivals each replay to map95 in1–9
transfers. The native maze ends on3F; subsequent quest/treasure interactions are
separate from this maze recipe. Deist's layer-specific safe-stair receipt remains.
Production-controller, native-shaped reader/all-six state and complete-maze/
21 binary-identity checks pass.22 frozen inputs/reports: static_checkpoint/.
Final route/controller job peak476,741,632bytes/82.202s; earlier native/controller
job310,013,952bytes/44.479s; both exit0 and childrenDrained true. No cap increase
or new puzzle installation. Temporary instruction-subset facts were initially
written under ignored build_check; corrected by moving all six to the external
non-Git static_control_facts archive, preserving them and fixing the helper.
No game code is in the frozen checkpoint or staged for Git.

Opera source/focused-offline is Closed at10:20UTC. Eight complete trajectories
pass: initial11 actions/22 transfers, three lyric retries11/20 each, three timed
retries9/14 each and inside resume8/12. Each uses three real contact-rat goals
before Ultros and ends at actual177.90 model states/150 identities, runtime,
actual choice producer, both source contracts, all-six route worker and current
Phoenix/Tower integration pass.25 keys/12 locales have0 missing.26 frozen
source/report inputs: opera_checkpoint/. Final eight-case job peak469,458,944
bytes/41.483s; all descendants drained. The prior shared package's495,423,488byte/
160.474s receipt correctly remains exit1 because its last fixture mixed an
inside-only area6 receipt with a fresh outside entrance. Its independent checks
passed and remain valid; the native-valid replacement and separate inside resume
pass without repeating unchanged checks. Native MapInitialize30ACD0 and storage
ClearAreaLocal D122C0/ClearMapLocal D12320 verify the boundary.
Only four initializer opcodes change, all Opera backdrop calls; all other559
records/guards/native PCs and all geometry/objects are preserved. The122-rule
data uses success165, actual retry/letter/director phases, native room entry,
stagehand movement and current reachable encounters. Final177 checks reuse
existing ready-state snapshots with no new native read, hook or polling system.
No puzzle DLL is newly installed; full canonical/release/gameplay remain open.

Phoenix source/focused-offline is Closed at08:54UTC. The final consumer passes
15 actions/29 transfers from the entrance and4 actions/8 transfers from the
demonstrated premature-plate state, both through native408 and final map262.
Tower still passes27 actions/58 transfers/final501. The116-rule package passes
78 model states/144 identities, all-six routes/layers, full reunion movie-chain
runtime and24 keys in all12 languages. Four exact directional automatic arcs
retain all638 geometries/native objects/old landings.31 frozen source/report
inputs and three budget receipts are in phoenix_checkpoint/; all tree cleanup
passes. Route/model package peak507,224,064bytes/169.077s. Runtime/contracts
pass individually; its aggregator failed only on the checker's guessed Chinese
locale names, corrected to repository zh c/t identifiers (zhc/zht) and checked
separately without repeating already-passed functionality.
No native party read is added to Phoenix. The old incomplete search, traces,
center hypothesis, numeric frontier and unused498 observation are retired.
No new puzzle installation or natural gameplay/native timing claim is made.
Source/failed evidence and remaining all-six
coverage are in [the puzzle topic](SHIFTG_PUZZLE_AUDIT_20261030.md). The original
45–90min package estimate is substantially overrun; report this functional
investigation separately from the resolved compiler/build-resource detour.
The hard aggregate1 GB maximum remains enforced for every expensive process
tree; latest complete package peak524,713,984bytes, children drained. No higher
limit, game execution or delegation has been requested or used.

### Closed October30: explicit1 GB build-process memory maximum

The human reports build RAM makes the computer unusable and requires a total
1 GB maximum, even with substantially slower builds. Any necessary increase
requires explicit permission. This priority interrupts the Phoenix/Tower
checkpoint at~83min; preserve its34-hop Tower failure and current source.
No compilers remain from the uncapped invocation. Scope: canonical and focused
build entry, whole-tree Windows allocation/resident quotas, child inheritance,
single-build exclusion, failure/cancel behavior, durable instructions and one
actual compile. Estimated20–40min; no game/probe/deploy or limit increase.
Owner replacement at00:29UTC requires the maintained global runner, CPU cap/
below-normal priority, no servers and finite budgets for expensive tools too.
The temporary limiter's compile was cancelled/preserved, and its source retired.
Exact maintained run-build.py and supplied MSBuild defaults/enforcement are now
adopted; all11 real64-MB checks pass. Check receipts stay in the workspace after
the sandbox TEMP path rejected writes. Normal/focused/smoke/deploy entries use
the runner. A cross-tool test at00:50UTC exposed that AppContainer isolated
Global mutex names: another invocation acquired an independent lock while our
bounded compile was running. The same-parent11 checks were insufficient for
this host. Our compile was stopped/preserved and its known children exited.
The project copy now refuses AppContainer before child creation, requiring the
normal user context while retaining every memory/CPU/priority limit. A normal
user probe correctly returned busy without launching a child because another
resource job holds the actual global lock; leave unrelated processes alone.
The maintained copy changed concurrently (GC fraction only), so do not overwrite
it or another chat's work. Merge its current defaults locally, preserving the
necessary host refusal. The20–40min checkpoint has overrun on this real global
exclusion gap; source integration is complete, outside-context guard/actual
compile qualification is waiting for the existing job. Revised remaining
10–30min after the slot becomes available, with compiler-duration uncertainty.
Final tool qualification completed at01:08UTC: all11 maintained checks pass in
the normal user context, and a separate normal-user tool invocation sees the
active mutex as busy258 without launching a child. The45-second holder exits
cleanly with childrenDrained true,64-MB effective cap,8.33% CPU and~7.5-MB raw
peak. AppContainer refusal is independently recorded before child creation.
Normal/focused/smoke/deploy and direct focused MSBuild verification are wired;
supplied defaults and /m:1 /nr:false /MP1 cover production/check/smoke compilers.
No cap increase, administrator/system-settings change or unrelated process
termination. Build-resource source/tool-offline qualification is Closed; the
current full C++ route-fixture result belongs to the preserved puzzle package.
[Design/evidence](LOCAL_GUIDANCE_20261028.md#build-memory-preference).

The preserved puzzle checkpoint now has two reviewed movie545 roller/duct chains
and an all-six native layer-switch correction; those changes advance Tower34
hops/eight actions. The next actual blocker is premature central-plate holding
before weights fall. Minimal conditional release rules are implemented but
their complete-consumer check remains pending. No puzzle closure/delivery claim.
The90min package estimate also overran once its resumed fixture compilation
was counted; do not claim the source/focused slices are the finished all-six
assignment. The actual consumer is active again under the qualified normal-user runner.

At02:02UTC, the required multi-party fixture is still blocked in MSVC code
generation rather than a memory-limit failure. One Release driver attempt ran
>30min without an object; driver-only optimization suppression then ran>10min
without clearing it. The source places fake reflection/native definitions in
the same translation unit as the planner, unlike production's external ABI.
The local fixture now explicitly excludes those fakes from automatic inlining
and leaves actual route/model definitions Release optimized. The initial
optimization-only pragma did not explicitly establish that boundary; corrected
using Microsoft's documented auto_inline pragma. Current bounded check is the
decisive return to the complete episode. Preserve all cases/owner checks and
the1 GB/one-worker budget; no production rewrite, unbounded retry or delegation.
Archives: multiparty_release_driver_compile_stall.txt,
multiparty_driver_only_attempt.txt and multiparty_facade_optimization_attempt.txt
under build_check/puzzle_audit_20261030/. No complete guide delivery is claimed.

At02:38UTC the stall also reproduced in Debug with explicit single-worker
code generation. It is not an allocation-limit failure; the compiler stayed
below100MB private in that run. The older signed14.44 compiler is installed,
but CMake rejected its version under the installed v145 platform toolset before
the comparison could compile; retain that failed diagnostic, not a toolchain
qualification claim. The smallest next isolation uses the existing production
translation-unit boundary: story_model.cpp compiles separately from the route
consumer. All route/model cases remain and normal Release optimization applies.
Unused optimization pragmas and diagnostic flags are retired after validation.
MinHook, core, game, check and smoke compilations now explicitly use one code-
generation worker as well as one compiler/node. The unchanged aggregate memory
cap is999,997,440bytes after page rounding; no increase is authorized or needed.

At02:46UTC that Release comparison also stalled in the route fixture's unit;
the separately compiled story-model object completed. Stopped only the verified
FFPR CL process so its parent recorded failure/cleanup normally. Whole-job peak
434,905,088bytes,456.319s,childrenDrained true. This rules out memory exhaustion
and does not establish that model separation fixes the compiler. A direct
MSBuild VCToolsVersion override is the next installed-compiler comparison; its
first launch correctly returned busy without creating a child. No tool install,
global toolchain change, optimization reduction or limit increase is authorized.

The installed14.44 override is confirmed in the actual CL path and produced
every other object, including the model, but the route fixture object remained
empty through the181.246s diagnostic window (whole-job peak487,165,952bytes).
Only that verified compiler was stopped; the runner reports exit1 and drained
children. Source chronology offers a narrower control: the last working host
retained a renamed base route-suite entry function; the first cleanup excluded
it. Restore that fixture boundary for one default-compiler comparison without
executing the retained suite, dropping cases or changing production. Do not
infer a fixed compiler or upgrade requirement from these incomplete attempts.

The retained-entry control also did not finish in612.293s; its model/other
objects completed, route object stayed empty, whole-job peak469,622,784bytes,
exit1/childrenDrained true after stopping only the verified owned CL. Do not
perform another guessed source/optimization rewrite. The next bounded diagnosis
emits Microsoft's /FAu listing for this single unit to identify its last
generated function before changing a consumer or fixture. This produces only
original mod/host assembly in the ignored build folder, never game assembly.

CMake translated /FAu to Unicode selection without enabling a listing, so the
actual CL response lacked that flag. Corrected to /FA. Its121.376s diagnostic
produced12,575bytes of constant/TLS declarations and no function assembly;
the partial listing cannot identify an unfinished function. Preserve it as mod
assembly, not native game evidence. Peak470,032,384bytes,childrenDrained true.
Next decisive isolation is temporary tools/native_codegen_check.cpp: compile
the actual baked_nav.cpp separately without any same-unit native stand-ins.
Linking that diagnostic intentionally lacks the application and must fail;
object generation is the observation. Retire the temporary check immediately
after that observation, before any canonical full-suite candidate. Keep the
default toolchain and all memory/CPU/worker limits.

Production-object isolation completed in34.355s: optimized baked_nav.obj is
15,822,286bytes; whole-job peak380,895,232bytes. The expected153 missing-app
link references are not a functional-test failure or a passing executable.
Temporary native_codegen_check.cpp is archived and removed from the check glob.
The separate stand-ins compiled too, but the combined driver/consumer still
did not produce an object in334.906s (435,146,752byte peak; drained). This new
evidence warrants one coherent test-only boundary: compile actual navigation,
native stand-ins, story model and the episode driver separately. A small owned-
value bridge calls the original private consumer; no algorithm is copied, case
dropped, production API/hook/optimization changed or higher limit requested.
Existing tests retain their original same-unit stand-ins through an include;
types and definitions now have one maintained source. The active full episode
must still compile/run before this can close. No further guessed compiler flags.

The split bridge produced a concrete C4716 despite complete return paths, then
compiler AV0xC0000005 with an output parameter. Both installed14.50 and14.44
reproduce the AV (roughly49s/421MB aggregate), so no toolchain switch is adopted.
Opaque planner call and bridge-only optimization suppression also fail. The
45–90min package remains substantially overrun; no complete guide is claimed.
Finite body bisection now supplies actual new facts: an empty plan bridge
compiles/links in74.451s and intentionally fails route assertions; restoring only
construction/capture (no planner call or result handling) crashes compilation
in28.426s at369,614,848byte job peak. Next control keeps snapshot construction
but omits observation capture. All controls/reports are explicitly diagnostic;
restore the full body before acceptance. No production algorithm changed.
Normal-user memory observation:~5.8GB physical/~38.8GB commit headroom. No explicit
out-of-memory compiler result has appeared, and no larger budget was requested.

Compiler blocker resolved at03:47UTC. Body bisection narrowed the failure to
snapshot setup; the last working host passed flags directly, whereas the new
release prediction used a conditional expression on the large Flags structure.
Selecting the owned Flags address before passing its value compiles the complete
bridge in32.456s/369,442,816bytes. Retired all bridge/facade/optimization/listing
scaffolding and restored the original actual-consumer fixture with that minimal
pointer-selection correction. It compiles in41.223s under the same1 GB/one-worker
budget. Temporary unsupported-toolchain/native_codegen targets are absent from
the canonical glob. No compiler install, production optimization change or RAM
increase. Preserve failed diagnostics as failed evidence, not current design.

The now-running full Tower episode reaches34 hops/10 actions, instead of the
previous eight, and still fails its final501 assertion. Central release works;
the next exact issue is availability of hidden-before-animation weights39/45 on
map549. Reuse the existing pinned Puppet.Hide contract and inspect actual Anim
entity Hide/action behavior before changing visibility or manufacturing another
control. Native collision, phase and current-floor observations stay authoritative.

Correction at03:56UTC: the weight initializer has CueAnim, not Hide. Its compiled
visible marker-1 means unchanged/unknown, not false. Exact bounded native Hide
checks confirm immediate Hide deactivates the GameObject; do not bypass it.
The actual destination validator explicitly excluded every AnimEntity even though
the bake, capture and interaction-goal code already support that physical type.
Remove only that exclusion. Weight39/45 retain exact ID/raw/cell, group, actual
visibility, consumption and approach-side checks. No projection/hook/visibility
policy changes. This is the next full-episode run under the enforced1 GB budget.

At04:04UTC the complete consumer advances41 hops/17 actions after both weight
pushes, the three-button bridge latch and Guardian. Four-way anonymous off-plate
goals had stranded parties north of the falling weights; actual native falling
direction requires south-only departure, now validated by the model. The final
501 assertion still fails after Guardian. Preserve the exact failed episode;
verify the encounter's native field-position operand before treating the
fixture's post-battle approach cell as the real player position. No event/gate
policy relaxation, gameplay claim or package closure. Shared resource slot
contention correctly refuses new expensive work; unrelated jobs stay untouched.
While that slot is occupied, the already-confirmed shared arrival regression
receives only a layer-equality guard and an actual Deist wrong-layer acceptance
case. Its focused run is pending; examined-step completion remains open. Narrow
source reads establish the old III/IV switch receipts and V's nonfunctional
second engine-room target, recorded in the puzzle coverage topic. No second
architecture/refactor or unbounded tool is started.

At04:27UTC the shared slot resumes the bounded export and complete consumer.
Exact native callbacks settle that EncountBoss cellPos focuses the battle effect,
not player movement. The ordinary-policy diagnostic isolates the mandatory
save-point prompt after Guardian; no generic event relaxation is accepted.
An explicit566:51 south-approach waypoint, owned Y/layer predicates and12-language
instruction advance the episode to43 hops/19 actions and the next boss. Native
victory continuations then establish actual final player positions/layers and
the three-statue497 conjunction. The fixture is being corrected to those facts;
the final501 assertion remains open. Current8-guide/103-rule author succeeds.

Further actual integration findings, still within the human-requested puzzle
recheck:565:49 had only its pre491 PointIn4 landing. Exact native source instead
selects PointIn6 after the bridge latch. One existing conditional companion and
two destination-pair guards repair that data; all638 geometries and original
landing behaviors are preserved. The full episode advances49 hops/21 actions,
including the central switch, Guardian, Fiend and Doom. Goddess's two real
interaction layers then expose coordinate-only goal inference choosing the
first overlapped actor. Both native targets are authored (8 guides/104 rules),
and the selected-object identity/layer repair is under complete-consumer check.
This is a minimal proven route-validity integration fix, preserving the accepted
NPC/native-owner/worker/cache design. No arbitrary hazard/layer bypass, further
Closed-system refactor, gameplay claim or package closure.

Tower focused qualification now passes58 hops/27 actions through native501,
with all required bosses, prompts, manual steps and final held parties. Final
focused regressions also pass:72 model states/136 identities, all-six route
worker, static wrong-layer arrival, story lifecycle, input interception and
transitions. Frozen evidence/source hashes: puzzle_audit_20261030/tower_checkpoint/.
The93-arc author attempt included two impossible lateral Doom stair approaches;
real geometry rejected them. Final91 manual arcs/71 masks retain only actual
approaches. The same-room landing test now recognizes its native sentinel-1.
Source/focused Tower slice is complete; the package remains Open for Phoenix,
remaining all-six puzzle coverage and final combined output/delivery. The
original22:52UTC45–90min package estimate remains overrun; do not reset it.
Phoenix's full two-party episode is the next finite source-to-consumer work.

### Closed October30: priority Floating Continent lag correction delivered

| Field | Current decision |
| --- | --- |
| Capability/scope | Human reports unchanged FFIII Floating Continent lag on foot and vehicles; browsing, category changes and directions must respond comparably to other maps. Cross-world mode remains untested. This scoped human follow-up takes priority over the preserved puzzle audit below. |
| Evidence | Installed and local DLL hashes both equal a6d9b4ce19dfb6adb33fb0051851814a62b4fcd0848c041c0ef23625b466998f. Own latest session27292/546924906 is the delivered October30 build. Browse337–521ms, category543ms, capture286–438ms across9–11 slices despite only4–5ms aggregate capture; repeated full scans351–648ms. Main-owner scanner callbacks26/40ms cross the25ms blocker. Prior fixture-only improvements did not meet user acceptance. |
| Boundary/next decision | Trace actual complete input/capture/filter/planner/publication and cold conversion; remove demonstrated repeated world-inventory work through qualified native state/lifetime ownership, preserving all gates/targets/vehicles. First60–90min proof/integration checkpoint; select a bounded production repair after source/native contracts, then actual-consumer offline before/after plus canonical/smoke/security/recovery and FFIII guarded delivery. No increased callback budget, stale guessed state, omitted routes, native waits, game probe or agents. |
| Status | Source/offline/FFIII delivery complete at11c25405fff5f1eb/11,495,424bytes,21:10UTC.688 frozen inputs,157 fresh reports,67 exact resources,12 languages,8304 native names/0missing,143used/350defined/0undefined; serial canonical21m03s,47-export smoke and exact Defender22-file scan pass. All22 installed files match;144 protected entries and other five payloads are unchanged. Final114-pair worker browse median/max: foot25.086/61.982ms, Nautilus13.254/49.483ms, Invincible15.981/84.221ms; paired reference maxima181/671/192ms.100 stationary category inputs total0.008–0.013ms with no backlog; fresh1524-border fixture observation1.056ms aggregate. Existing0.5ms budgets remain. These do not certify natural frame/audio timing or cross-world gameplay. First90min checkpoint overrun and earlier candidates remain below; final verification/delivery finished within revised20–40min estimate. Recovery/completion build_check/floating_retest_20261030/. No public release, game launch/probe or agents. The original puzzle audit resumes as the single active system. |

The following paragraphs retain the first candidate decision and overrun receipt.
Expanded endpoint review:114 exact selected/browse pairs pass tile/action/order
parity, but native Player1/8/11 worst browse55/222/106ms remains a performance
gap; do not deploy the first canonical candidate as finished. Nautilus failures
are Bahamut's Lair276/278 (verified MSG_MAP_NAME_62), whose upper entrance
requires the real lower-floor connector. Preserve that connector, the separate
surface Crystal Tower/Maze route and every gate. First serial canonical build
continues unchanged for correctness evidence. At~55min, the next bounded
decision is settled by actual bake graph:48 world1 interior edges,38 complete
single-world-return closures; the ten others include Maze, tunnels and other
real connections. Facts in build_check/floating_retest_20261030/
connector_before_facts.json. After the ongoing build finishes, integrate only
conservative initial-reach dominance for these complete return closures and
one owned current-world flood per identical request/policy (with the existing
synchronous retry borrowing that same owned result). No geometry, pruning of useful
destinations or route policy changes. Re-run all114 parity/cost cases and Maze/
vehicle/gate controls, then final canonical/security/delivery. Revised remaining
estimate45–90min including serial rebuild, with compiler duration uncertainty;
no further supporting branch or generic framework is planned.

At the90min checkpoint the serial build has completed compilation and is
progressing through canonical checks. The second patch remains outside the
frozen production inputs until that run finishes. The earlier estimate has
overrun because every-exit measurement exposed the remaining worst-case cost
and the shared-header rebuild is substantially longer than the previous warm
build. Preserve this correctness run, apply the already bounded patch, then
measure all114 pairs before the final build. No gameplay handoff or performance
closure is claimed while those outliers remain.


### Active October30: human-requested all-six gate and complete puzzle recheck

| Field | Current decision |
| --- | --- |
| Capability/scope | Recheck all six games' unreachability/story-gate audit and provide beginning-to-end Shift+G puzzle guidance, especially Opera House, Phoenix Cave, Kefka's Tower and Narshe mines' light path. User explicitly initiates this follow-up to the Closed audit. |
| Boundary/evidence | Reuse source-matched prior map/gate evidence; inspect original mod consumers plus matching external map/script/native evidence. Existing static guides cover only18 named recipes; FFVI's seven story guides explicitly excluded other puzzle solutions. Narshe's light path is absent. Inventory all applicable mechanics before implementation. Preserve accepted route/worker/input designs; necessary minimal guide/state/data fixes only. No unrelated menus/battle, startup-hang work, public release, game launch/control/probe or delegation. Game implementations remain outside every Git tree. |
| Ordered milestones | 1. Inventory mechanics, source-match prior gate coverage, identify exact missing contracts. 2. Complete one guide package at a time, including progress/retry/leave/return/completion and localized output through Shift+G. 3. Reconcile all-six acceptance; canonical build/smoke, exact-payload security/recovery and permitted all-game delivery. |
| Bounds/done | Owned snapshot rules and cancellable Navigation lane; no game-thread scan/wait or new polling. Check actual production routes and state transitions, not target counts alone. Preserve cumulative25ms lag rule; distinguish offline cost from natural gameplay/audible/frame timing. Update affected feature docs/comments and concise nonvisual checks. |
| Effort/status | Active again after21:10UTC FFIII delivery. Narshe's17-rule source/focused-offline slice is complete. Opera's performance choices, separate rendered choice reader and read-only event keyboard/controller route are implemented and focused checks pass within its45–90min checkpoint. Native two-choice answers1/2/1, cursor lifecycle, remaps/held release, actual story dispatch and suppression are covered. Combined integration/delivery remains pending. Next finite package: Phoenix/Tower complete native-state and actual-route entrance/party/release/continuation/completion acceptance,45–90min checkpoint, then remaining all-six inventory and shared proximity correction. No premature handoff. [Coverage](SHIFTG_PUZZLE_AUDIT_20261030.md). |
| Recovery/closure | Starting clean master and installed all-six a6d9b4ce19dfb6ad/11460096-byte DLL; prior recovery build_check/navigation_delivery_20261030/. Open. Other Closed systems retain their accepted boundaries. |

Phoenix/Tower checkpoint began22:52UTC,45–90min. At23:39UTC it remains in
progress: two missing Tower controls, event-safe reviewed crossings and the
loaded-floor manual-ladder self-sealing repair are implemented. The middle
party episode advances through13 hops/two native controls; outer conveyor
continuation and full party completion remain unresolved. No weakened gate or
guessed landing is accepted. Fixture optional-initializer AV and corrected
development failures are preserved in build_check/puzzle_audit_20261030/.
Current8 guides/96 rules are implementation counts, not completion evidence.
Source/offline/combined delivery/gameplay remain separate. See the coverage
topic for native provenance, rejected assumptions and the remaining checks.


### Closed October30: source-history removal and prevention

| Field | Current decision |
| --- | --- |
| Capability/scope | User explicitly authorizes rewriting shared native repository history to remove every recovered game-source/disassembly body, retain mod history and release history, and enforce the never-in-Git rule. Do not delete/recreate the repository. On a GitHub permission error, stop GitHub attempts without any browser/authentication fallback and report it; continue independent navigation work. |
| Boundary/evidence | Current shared HEAD71d53fb is clean and deployed in all6 games;30 historical raw files/53757 native instruction lines are already confirmed. Inspect all reachable local/remote heads and release tags for additional recovered bodies, preserve a verified recovery bundle outside every Git tree, sanitize only the identified game-source blobs, preserve current tree, names, ownership, legitimate vendor code and release assets. User authorization supersedes the previous defer-until-at-PC instruction for the rewrite itself. |
| Done/bounds | Source-free reachable-history check; exact pre/post current tree and mod/dependency parity; original refs/bundle/remotes preserved externally; unchanged release metadata/assets and tag names; explicit lease/atomic remote update when permitted; durable hard rule plus staged/build prevention. No tool installs, game launch, source dump inside Git, new release or unrelated visibility changes. |
| Effort/status | Complete within45–90min estimate. Full scan2703 blobs/36 body versions across30 paths; prepared backup88 commits verified. Exact36 blob substitutions preserve all88 commits/11 local refs, current tree byte-for-byte and8 tag names. Old36 bodies removed from local object store; rewritten reachable history2711 blobs/0 rejects. Atomic leased GitHub update9 refs succeeded, all7 releases/7 assets and metadata unchanged, no PR refs; GitHub content CI41053005400 succeeded. No permission error/browser, no repository deletion/recreation. Durable stage/build/CI checks and external known-body rejection control pass. |
| Recovery | External before/prepared bundles and refs/config/commit maps at ../tools/history_rewrite_20261030/; factual completion build_check/history_rewrite_20261030/completion.json. Guard commit remaps6a24252 ->2a214aa, shared remote master2a214aa; previous71d53fb also remapped in external commit-map. All-six installed cf805c2e unchanged. Other clones/GitHub internal unreferenced caches were not inspected and are not claimed purged by this normal-ref rewrite. Sol only/no game launch. |

### Closed October30: vehicle/world journeys, Exchange and lag delivery

Source/offline/all-six delivery is Closed at a6d9b4ce19dfb6ad/11460096 bytes.
All132 installed payload hashes match;144 protected files are unchanged.
Completion/recovery: build_check/navigation_delivery_20261030/completion.json.
683 frozen inputs,67 exact resources,12 languages,154 current reports;8304 native
names0missing,143used/350defined/0undefined,47-export isolated smoke pass.
Defender reported no threats in22 exact files with stable hashes; exclusion
query is unverified and no signing certificate is available. Final warm canonical
3m23s uses /m:1 /MP1. FFIII was deployed first, then the other five, with successful
closed-process queries before every copy. No game launch/probe or agents.

Accepted interface boundary: bounded owned world/craft graph and saved values,
exact current live capture, four filter modes, source-floor grouping, action-aware
browse, contextual ordinals, existing Exchange producers, and world1-only native
root/view reuse. Current menu/NPC/pose/vehicle/script invalidation remains fresh.
All finite acceptance/offline/release checks are complete. Actual game frame,
audible timing and natural play remain separate confirmation, with the25ms
cumulative mod-caused lag rule unchanged. Startup-hang follow-up remains separate.
Further refactoring needs a human-triggered scoped task. Ordinary source-safe
commit/push is authorized; publication receipt belongs with this completion.

The following paragraphs preserve implementation decisions and failed-attempt
receipts. Their preparation/future-tense wording is historical, not open work.

Current checkpoint update: Maze landing and same-world craft-change source/
targeted checks complete. Connected continent/surface/underwater routes, real
saved off-map craft and fourth World map controls are implemented and in final
integration verification. Actual forced-wind rejection is retained; a negative
future-world test caught an inappropriate ignore-hazards proof, now removed.
No terrain/gate is relaxed. Current-map/world-mode scanner transitions pass.
World category baseline2.554s owned worker; deferring unused unique remote-row
routes reduces it to1.732s with identical grouped rows/order. It remains owned
asynchronous compute, not game-frame latency certification. Native saved reader
abstains outside ready FFIII owner and performs no clone/serialization/new hook.

Human steering: Group exits must retain one destination per source floor,
so Maze1F/5F World Map exits remain independently listed. Own current log shows
map301 changing from the local World Map entry to only the5F remote entry;
the old grouping table was keyed solely by destination. The minimal source-floor
key correction passes the actual Maze bake regression. Same-floor
nearest/selection/continuation rules and independent physical soundscape remain.
Raw log archive ../tools/navigation_journeys_20261030/maze_exit_group_20261030.log.

Navigation source is frozen for combined verification while the latest human
request is implemented: FFIII Magic -> Exchange character picker is silent.
Native Exchange5 uses useController, while the mod erroneously treats it as a
spell-list phase and only accepts character cursor events in UseTarget2. Exact
native Init/SetCursor/state/source-change evidence is external at
../tools/magic_exchange_20261030/ff3/. Six failing production-consumer checks
reproduce the gap. Finite25–45min slice: same existing12 hooks, entry/source
header and recipient navigation, displayed vitals and existing I/Autodetail MP
charges; cancellation/visibility/readiness and no extra text dedup or polling.
Other games' equivalent target mechanisms are inspected without unrelated edits.

Nautilus browse now describes the first manual mountain boundary instead of
claiming unreachable while directions succeed; exact own-log origin/parked-cell
reproduction passes. NPC name parsing removes inner developer ordinals before
adding the actual duplicate suffix, retaining descriptive job text.

Latest human steering, October30: significant category/destination cycling lag
and smaller Backslash lag is confirmed only on Floating Continent world1, on
foot and in Nautilus/Invincible; surface/underwater are unaffected. This is the
active performance blocker before delivery. The in-progress canonical build was
explicitly interrupted, its logs retained, and no candidate was installed.
Own six rotating logs are preserved outside Git at
../tools/floating_navigation_lag_20261030/. Observed world1 route capture154–184ms
over10–12 native slices at about5–6ms aggregate native observation; selected
response216–270ms. All-map browse p95~422ms/max1482ms, category p95~453ms/max709ms
are contextual log statistics, not isolated world1 certification.

Actual production world1 fixture has1524 captured air-border actors/3174 baked
objects. Before change, category119/295/315ms (foot/Nautilus/Invincible), selected
foot219–236ms and aircraft23–30ms, browse foot239–253ms and aircraft41–50ms.
Pure worker/native-owner calls0. Repeated full object-ID scans per actor are
being replaced by a prepared immutable identity index, preserving nearest,
duplicate/zero-ID, raw-name and conditional-companion semantics. First finite
60–90min blocker checkpoint: prove that consumer improvement, qualify/reduce
redundant native capture reads without caching NPC/gate state, then replay all
three real-data modes and current/browse/category/cancellation controls. Keep
the native copy/episode bounds, with no game launch/probe or broad redesign.

Floating Continent source/offline checkpoint is complete within its60–90min
estimate. Legal foot/parked-craft, Nautilus and Invincible runs preserve every
route tile/action/browse result/category order. Cursor boundary reads24628 ->4713;
stationary category inputs cause no rescan backlog. Pose/vehicle/script/live-NPC
controls refresh. Rejected bake-no-NPC and borrowed-byte lifetime fixtures are
preserved with corrections. Eligibility uses current native actor facts; no
callback budget was raised. [Evidence/limits](FLOATING_NAVIGATION_LAG_20261030.md).
Final serial candidate is frozen at683 source inputs in
build_check/navigation_delivery_20261030/. Earlier interrupted evidence remains
in navigation_journeys_20261030/. No new payload is installed yet.

Final integration: MainMenu5 can change Mini without movement or a field-script
revision. The existing native producer now calls a field-owned action retiring
only FFIII/world1 routes/views, with no scan in the menu. Actual producer/action
checks cover native entry/return and other-world/game controls. Eight top-level
legacy build guards now check exact nonzero status: benign negative, positive
and zero controls pass. Nested NPC and smoke guards already handle both signs.
The initial combined build compiled all129 programs but failed eight FFV parked
ship list/route parity controls. The new ExitSnapshot vehicle policy is now
FFIII-only, preserving the other five games' existing policy; the actual all-six
route_worker_check rerun passes0 failures. Initial683 inputs/build/exit/report/
scan remain preserved. Refresh only these explicit integration deltas, rerun
canonical checks and scan the exact final DLL. No C++/embedded-data/build-script
edits while a compiler runs. No terrain/gate relaxation or new audit branch.

The corrected C++ candidate compiled and passed every code/name/wiring check
(8304 native names0missing;143used/350defined/0undefined). Its19m serial
recompile/test run returned1 only because the newly added development-check
label followed normal endlocal without an exit. Add the explicit successful
return; no C++/data change or further compiler work is required. Preserve that
entire input/compile/test/exit receipt in before_build_exit_fix/, then rerun the
normal warm canonical command. Initial full129-program compile35m48s and this
19m integration run remain separate from the final warm-run duration. These are
offline-suite builds, not evidence that the game DLL alone takes that long.

Final canonical/smoke/security/guarded all-six installation and affected
feature documentation are complete. All five new journey/reader/capture/cost
checks are in the ordinary canonical runner. Sol only, no game launch/probe.
[Current topic](WORLD_MAP_JOURNEYS_20261030.md).
The following paragraphs preserve the initial before-proof and finite estimate.

The next complete production slice is the user's Maze/Crystal Tower approach,
including on-foot/other-vehicle origins and the reported26-tile-west landing.
Pre-change proof uses the preserved own session and actual production planner,
native landing grids/real entrances and parked vehicle snapshots. Preserve
existing gates, actual aircraft capabilities and owned-worker/interface bounds.
First45–90min proof/integration checkpoint; implement only the actual missing
consumer, with current/browse/retained-route checks and no new native hook.
No compiler runs concurrently with C++/embedded-data edits. Frozen all-six
cf805c2e remains installed until the required connected candidate passes checks.

Pre-change proof: actual Maze route from151,74 and46,57 lands165,41 and adds34
walking tiles, although equal-cost legal land197,42 leaves1 tile. Airship
landing ties lacked the existing chocobo short-walk preference. Minimal worker
comparison correction produces1 final tile at unchanged total81/265 and unchanged
mountain priority; two failures become0. Ground approach197,42 already lists
Tower on foot (334 steps). A real native-legal parked Invincible150,71 also
works before boarding from on-foot151,74 (selected route and category416).
The same captured craft while currently on Nautilus9 fails selection and list:
2 failures, selected0/list-1. Exact consumer gap: boarding_route deliberately
considers only the current craft when mounted, and first-world graph coverage
uses only its own landfalls. This is the required vehicle-change dependency,
not a reason to relax terrain or force an aircraft onto an ordinary foot route.

Next bounded integration: FFIII-only transitive parked-vehicle coverage and a
small deterministic vehicle-journey search reusing the qualified single-vehicle
leg builder. Same immutable captured positions/names, existing boarding/landing
phases and cancellation; no new native hook/read for this same-world slice.
At most32 captured candidates (actual masters<=19), one vehicle node per
physical craft, no cycles, explicit cancellation, and all-six legacy behavior
preserved outside FFIII. Initial60–120min implementation/offline estimate for
the shared selected/list/browse/retained-route connection; whole-world-mode
transitions remain the next declared slice. Actual cost must be measured,
with owned worker time distinct from game-thread impact.

Development proof runs now use build.bat --check NAME with the same one-compiler
CMake configuration and exact production-consumer executable. Before whole
canonical177.203s; one changed targeted check27.166s, warm9.716s. This development
mode explicitly does not certify or install a payload; final full canonical,
smoke/security/deployment remain required. Records under
build_check/navigation_journeys_20261030/ preserve before/after and source limits.

### Ordered October30 vehicle/world-map journey scope

User-triggered follow-up to Closed navigation, completed one slice at a time:

1. Ancients' Maze/Crystal Tower reachability while on foot or aboard another
   vehicle, using an actual reachable parked Invincible when required; investigate
   the latest reported Invincible landing26 tiles west despite legal nearer
   landing at the Maze. Preserve actual terrain, statues, wind and native actions.
2. Choose required vehicles from live parked positions and real capabilities,
   including Invincible -> land -> Nautilus for Doga's Manor winds. Preserve manual
   boarding/landing actions and consistent browse/selected-route eligibility.
3. Connect actual world-map transitions including surface/underwater, prefer
   flying near an underwater entrance then diving when legal, preserve a valid
   slower underwater route, and support Doga's Village's tunnel/surface journey.
   Add a fourth World map filter limited to destinations on connected world-map
   bakes, including town/dungeon entrances there; do not flood it with interior
   town/dungeon contents. Existing literal Current map mode retains its contract.

Fixed acceptance: actual source/data/production-consumer before/after checks,
current/browse/retained route consistency, locked/missing/stale vehicles, actual
map boundaries, bounded owned-worker cancellation and cost, affected12-language
controls/docs, then serial canonical/smoke/exact-payload scan/guarded deployment.
No native probing/game launch or broad all-game redesign. Initial45–90min proof
checkpoint for the Maze/landing and existing transport planner, then scope the
necessary connected journey slice from its actual missing consumer/source facts.
Installed candidate remains frozen until required connected functionality passes.

### Closed: final combined candidate and coverage reconciliation

| Field | Current decision |
| --- | --- |
| Capability | Deliver the completed finite all-six map/event/reachability audit with current affected-feature documentation, canonical/offline/release/security evidence and verified applicable installation. |
| Boundary | Reconcile the original fixed findings/Closed interfaces and current source-matched receipts; run build.bat, smoketest.bat, exact-payload scan/recovery/deploy checks. No new features/refactors or game launch/probe. Any actual integration regression receives only its minimal accepted-design repair. Startup needs a naturally failed own log and remains a separate excluded known issue. |
| Coverage | Affected feature topics must be current and indexed. Unrelated whole-mod documentation catch-up is a separately finite coverage finding, not a new integration prerequisite without an actual missing safety/correctness contract. No duplicate scope ledger or user code-review gate. |
| Done/bounds | All required finite findings and source/consumer checks reconciled; one compiler and /m:1 scheduling; no C++ or embedded-data edits during compilation; source-free repository/evidence review, exact final payload hashes/scan and allowed recovery/deployment. Keep native/frame/audio and gameplay limits explicit; cumulative25ms rule remains. |
| Effort/status | Source/offline/all-six delivery Closed. Completed within the30–60min final milestone estimate, including21m42s full serial compile and two minimal legacy test corrections; final canonical2m37s pass. All finite cases,8290 native names/0 missing,142 used/349 defined/0 undefined, isolated smoke,67 exact resources/12 locales/667 source inputs pass. Defender22 exact files/stable hashes/no threats;132 installed hashes and144 protected files verified. Affected documentation is current. Final receipt: build_check/map_audit_final_20261029/completion.json; [delivery topic](MAP_AUDIT_DELIVERY_20261029.md). No native-frame/audio/gameplay certification. |
| Recovery | All six installed cf805c2e/11374080 bytes. Verified previous hid.dll/SDL3.dll recovery copies for every game, unchanged protected/dependency files, read-only Prism prerequisite checks. Prior candidate receipts frozen. Source cleanup200 external originals/30 current references is complete. User-directed commit follow-up verifies current deployment and stages all cleaned changes; historical source cleanup remains deferred until user is at PC. Sol only; no game probe/agents/history rewrite/repository recreation. |

Current history finding: [source-content review](SOURCE_CONTENT_REVIEW_20261029.md).
Thirty recovered-body files are confirmed in this shared native repository's
October12 history, with external originals matching after Git normalization.
User October29 directs committing all cleaned current changes and verifying the
shared mod in all six games.132 installed file hashes were reverified. Ordinary
current commits proceed; history rewrite/recreation waits until user is at PC.

Combined-check integration note: first serial compile completed in21m42s, then
the old navigation-followup assertion failed twice on FFV map284/actor4. That
door has two real world destinations. The prepared native identity lookup
deliberately returns no single destination for an ambiguous door; the owned
state-aware planner already selects both actual branches in the dedicated
6/6 check. The legacy assertion incorrectly demanded one destination for each
companion. Its minimal test correction preserves unique-exit matching and now
asserts ambiguity rather than an arbitrary destination. It passes0 failures;
no production source/data/DLL change was needed. Original failed log, exit,
assertions and source manifest are preserved under the final receipt directory.
The next complete pass found six more outdated assertions in mapstate_check:
the generic harness applied each destination-scoped predicate to its native
Event rather than its matching Warp companion. It now retains the exact native
identity check and verifies the predicate against its exact destination arc.
All-six mapstate assertions now pass0, including unavailable-flag controls;
the separate conditional-door test retains native-door/nonconsumption and6/6
real routes. Again only the harness changed. The second failed canonical log,
mapstate report and candidate manifest are preserved. No unchanged failed retry
or new production system was added.

Affected-feature coverage is current in FEATURE_INDEX, THREADING, TESTING and
the linked finite feature topics. Unrelated whole-mod documentation catch-up
remains excluded: reconciling older PORT_STATUS/AUDIT summary wording and the
historical FEATURE_INDEX journal with their existing feature topics. This is
documentation coverage only, not a missing implementation prerequisite or a new
automatically started task. No Closed subsystem is reopened by that finding.

### Completed source/offline: final script closures and source ownership

| Field | Current decision |
| --- | --- |
| Capability | Resolve the last two continuation-closure cases and five local/shared/missing script-owner cases without guessed event meaning, false gates or omitted real interactions. |
| Fixed inventory | VI sc_e_1055 native trigger-script lookup and sc_e_0650 cycle; II sc_transition_20041_7; IV sc_e_0006/sc_savepoint; V sc_e_0076_3/_4. Existing source inventory: build_check/map_event_audit_20261028/source_ownership_resume_20261029.json. |
| Boundary/bounds | Exact native selector/dispatcher and matching extracted assets; existing metadata/label/bake mechanisms for actual incorrect data only. No native capture/hook, global revival, route redesign or reopening of Closed systems. Preserve real source uncertainty and the literal-map inventory contract. Native code/source archives remain outside Git. |
| Done checks | Each case has an explicit native/source-owner/closure disposition, actual consumer evidence for any repair, localized output and current feature documentation. Existing finite VM/graph/cancellation bounds; no native game-thread work added. Final coverage/combined/security/applicable all-game delivery follows. |
| Effort/status | Source/targeted offline Closed. Native local/resident banks resolve all five cases; save marker uses Resident7, dormant IV demo is Action0, Fynn helper preserves exact real exit, V dance55/56 have no bank and sibling54's foreign barrier binding was retired. All other active reviewed roots are exact local/Resident. VI live contact is a native actor ScriptId dispatch; apparent SCC changes shared-area variable and terminates on both post-win paths. Actual15 actor/native/remote cases8→0,1362 functional/source/12-language and245 scene/41 narrative checks pass. All six bake/state hashes unchanged. Completed within45–75min estimate; combined/security/delivery pending. [Detailed evidence](FINAL_SCRIPT_CASES_20261029.md). |
| Recovery | Installed FFIII b61153fc unchanged. Source and targeted evidence frozen at build_check/silent_context_20261029/completion.json and final_inputs.json. No compiler/session/question pending; Sol only/no game probe/delegation/publication. |

### Completed source/offline: remaining245 silent scene families

| Field | Current decision |
| --- | --- |
| Capability | Give physical mechanisms, barriers and ordinary travel points useful functional labels, while retaining Scene trigger for actual narrative scenes. Keep real map travel in Map Exits and preserve unavailable-source uncertainty. |
| Reason/boundary | Review the finite245 silent families from the original954 inventory against full script/continuation context and actual current category/provider. Some III native actors are intentionally replaced by accepted world providers. Existing exact identity/hash reviews; no runtime script interpretation/new hooks/route redesign. Necessary actual dependencies:3 V Main scripts have2 story-flag destinations but only the first is baked; IV Devil's Road transfers through an intermediate world scene and sets the final player at6,6 rather than its native PointIn1. Reuse typed bake companions, existing bounded guard projection and landing overrides; preserve other Closed interfaces. |
| Done/bounds | Every family has an explicit supported contextual disposition, source-hash witnesses and actual label/category checks for changes. Only concrete incorrect bake/reachability data is corrected through existing producers. Existing bounds/native ownership and localization preserved. |
| Effort/status | Source/data/targeted offline Closed:all245 dispositions,41 narrative families preserved;381 native/remote cases0 (initial368 cases756 before assertions→0);1362 functional/source/12-language0. Three conditional doors3/6→6/6 actual entrance plans, native and unknown guards/terminal boundaries pass. Devil Road0/2→2/2 and exact final6,6 continuation/lock pass. Six underwater mounted routes/batches and actual prior foot parity pass (first extra test wrongly forbade native Player1 movement; corrected from real before bakes). All-six planner and native transport/pressure-switch regressions0.134 promotions/3 native companions/1 exact chain remap/1 virtual arrival; other geometry/masters/objects/arcs preserved; only six FFV scoped guard rows, IV state byte-identical. Completed within the60–120min contextual estimate. Final combined canonical/security/delivery pending. Detailed contracts/limits: [silent-event evidence](SILENT_EVENT_CONTEXT_20261029.md). |
| Recovery | Installed FFIII b61153fc unchanged; NPC, Doga and arrival/power source/offline receipts frozen. Sol only/no game probe/delegation/publication. |

### Completed source/offline: conditional arrivals and trigger power ownership

| Field | Current decision |
| --- | --- |
| Capability | Preserve real arrival coordinates and reachability for the seven remaining conditional door transfers; distinguish genuinely consumed triggers from triggers rearmed by native lifecycle. |
| Reason | All eight original variant sets have PointIns; one V pair uses the same cell. Remaining III four Amur returns/world4-point11, Map_30231_10 actor3→290-point1; V Map_20050 actor60→81-point1 and Map_30130 actor2→world1-point12 need exact native choice/position evidence. |
| Boundary | Existing state/PointIn/travel producer only for proven errors. Native default-list/first-match lookup resolves all8 sets: no arrival bake change. Necessary actual omission:17 VI SwitchEvent30 actors inherit EventTriggerEntity but were absent from capture and step_event; admit that native type and preserve exact source matching. No guessed arrivals, grid edits, global trigger revival or route redesign. |
| Done/bounds | All8 real doors/24 known-and-unknown choices/24 fresh owned continuations0fail. All-six native Hide/Reset/Exit rearm versus property/suspend/overlap facts reconciled. Real17 switches:86 before assertions→0,34 powered/rearmed captures/17 remote hazards, unpowered guards/native-owner0. All-six route_worker0,98 vehicle models/582 world candidates. No new loop/hook/native invocation; existing capture/worker/cancellation/graph bounds. |
| Effort/status | Source/targeted offline Closed at build_check/trigger_power_20261029/completion.json; detailed contracts and native locators in [arrival/power evidence](ARRIVALS_TRIGGER_POWER_20261029.md). Original30–60min estimate extended for the actual omitted subtype and shared regression; no architectural branch. Canonical/security/delivery remain queued. |
| Recovery | Installed FFIII b61153fc unchanged. Preserved NPC and Doga receipt/input hashes precede this milestone. Sol only/no game probe/delegation/publication. |

### Completed source/offline: Doga surfacing marker browse annotation

| Field | Current decision |
| --- | --- |
| Capability | A reachable Surface near Doga’s Village marker must show its usable distance when browsed, rather than Not directly reachable solely because surfacing is the destination action. |
| Reason/boundary | Source-proven integration issue: plan_browse_snapshot marks every action_after indirect; the marker adds its own manual surface action. Preserve the delivered marker/provider/native action and route architecture. Change only exact marker endpoint annotation, after an actual before assertion. |
| Done/bounds | Actual marker provider→category→browse→route check, current mounted arrival/partial-blocker/other vehicle paths preserved; no native work/hook or extra search. Existing route work bounds/cancellation unchanged. |
| Status/effort | Source/offline Closed: actual provider/category/route/browse2→0,147-step and zero-step endpoints, wrong vehicle/other-game abstention and post-surface landing pass. Completed within10–15min estimate; canonical/security/delivery pending final audit milestone. Accepted provider/native design preserved. |
| Recovery | FFIII b61153fc remains installed; NPC source/offline closure receipt frozen. Broader canonical/security/delivery follows the remaining audit. |

### Completed source/offline: remaining six NPC region/scene contexts

| Field | Current decision |
| --- | --- |
| Capability | Resolve the remaining three South Figaro movement-region NPCs, bedroom NPC, Darill guide and FFII scene-owned Emperor; retain valid approaches and exclude invented cross-layer/counter access. |
| Reason/evidence | Current geometry has one entry-connected region position for each of the three Figaro patrols, but their homes lack approaches. Exact native region/line semantics now verified; actor movement/entrance/target-local controls pass but graph witness is missing. The inherited test host also disabled the real III–VI hidden-passage flag; production-matched controls confirm the same missing graph witness. Original eleven before/after proof remains11→0. Decisive trace locates the missing root stair edge: real Upper masks159→31 allow SE, but four cardinal neighbors are zero. Local A* passes; distance_to_object only checked cardinal predecessors. Minimal integration adds only validated native indoor diagonals to that terminal-edge estimate; world movement unchanged. Temporary trace has been removed. Bedroom and Darill already pass actual local planner; Emperor has three reachable scene controls. |
| Boundary/bounds | Reuse existing metadata/owned remote interaction goal pipeline if an actual defect is proved. No new capture/hook/framework or relaxed layer. Pin exact source identity/native conversion and at most32 verified positions; existing graph cancellation and limits remain. Three source-hashed default Area2/Route3 rows add12 positions in one existing state record; prior fields/program unchanged. Route0 is allowed only for a known initializer/active wandering actor, with exact fingerprint. |
| Done checks | Actual source region→goal→cross-floor first leg→native floor actor, active/hidden and state guards; scene owners/real controls recorded; targeted current and remote regressions. Combined canonical/security/delivery remains the final audit milestone. |
| Effort/status | Source/data/targeted offline complete. Three region routes/native continuations and three FFII active mixed-state scene controls pass; bedroom/Darill source-matched planner pass; all-six planner/initializer0. About35min against30–45min estimate for this region/scene slice. Source/data integrated: native stair edge is usable and3 region-selected routes pass. All11 assigned-route regressions now replay every real intermediate floor; seven Zozo fixtures previously treated first arrival389 as final floor390. Corrected bounded receipt replay and final guards pass. |
| Recovery/closure | Source/data/targeted offline Closed at build_check/npc_future_20261029/completion.json and final_candidate_inputs.json. Canonical/security/delivery pending final audit milestone; installed FFIII b61153fc unchanged. Exact17-function archive outside Git in ff6_region_native. Existing Closed interfaces preserved. |

### Completed source/offline slice: eleven assigned-route NPCs

| Field | Current decision |
| --- | --- |
| Capability | From another floor, select any of the eleven reviewed native moving NPCs and retain a valid first-floor route; use the actual actor once its floor loads. |
| Reason | Actual collector/planner11 failures with reachable entry and native-pose controls. Exact initializer branches own all11 assignments, including turtle bank2/76 and dragon107=1,108=0. |
| Boundary | Minimal integration in existing read-only initializer projection and remote interaction goal producer. Preserve live capture, blocker handling, masks, graph, controls, labels and native ownership. No new hooks, runtime probes or route framework. |
| Bounds | Existing8192-instruction/64-branch initializer bounds;64 assignments/metadata rows and32 positions per actor. Native counts are11 actors/67 distinct positions across5 records; no per-position graph search. Cancellable existing graph retains400 visited-state bound. Current-floor native timing/work unchanged. Measured collector+first-leg compute8.248ms total/1.269ms peak for11 requests; no runtime frame/audio claim. |
| Done checks | Actual collector→selection→first leg→pending identity→live floor continuation; inactive/unknown assignment, wrong actor, malformed data, cancel and ordinary route controls; exact data delta, affected docs, combined canonical/security/applicable delivery. |
| Effort/status | Source/data/targeted offline integration complete:11→0 actual collector/first-leg and all11 live-floor continuations; all-six planner0,98 vehicle models/582 world targets; inactive/unknown/cancel/retained-state controls pass. Combined canonical/security/all-game delivery stay on the final audit milestone. Initial30–45min proof checkpoint overran to~45min; valid11-case proof found necessary integration. Revised30–45min for source/checks, excluding combined serial build. Not gameplay confirmed. |
| Recovery | Installed FFIII b61153fc remains unchanged. Original FFV/VI states and11 before failures retained under build_check/npc_future_20261029/. No game launched or other-game deployment. |
| Closure | Source/data/targeted integration Closed after production-flag requalification: retained original states still11 selected-route failures; corrected states0/all11 native continuations. Canonical/security/delivery pending on the final milestone. Live NPC capture, counters, vehicle routing, current statuses, Doga marker and Crystal approach remain Closed at their linked accepted boundaries; this patch does not refactor them. |

The finite assignment has no remaining implementation, affected-documentation,
offline or installation work. All original source-owner/closure cases are
resolved at the Closed cards above. Natural-play/audio/frame confirmation,
the separate startup failure log, historical-source migration authorization
and excluded unrelated documentation coverage retain their stated boundaries.
Whole-mod unrelated documentation catch-up is separately tracked at final coverage
reconciliation; it is not a prerequisite for this scoped fix unless an actual
missing safety/correctness contract is found. No additional feature is added.


## Earlier inventory and checkpoint history

The inventory below preserves the original findings, estimates and receipts.
Its remaining/open wording is superseded by the current cards above; it is not
a second active plan or an unresolved-source count.

Current FFIII installed b61153fc/11357184 bytes: [live ally statuses](BATTLE_LIVE_STATUS_20261029.md),
[Doga surfacing marker](DOGA_SURFACE_MARKER_20261029.md) and
[Crystal Tower via Ancients' Maze](CRYSTAL_TOWER_APPROACH_20261029.md) source/offline/
FFIII delivery Closed. Exact cleared flags hide8 statues and relocate4 triggers;
Tower current-map/selected/landing/pending/maze continuation/gates pass. Canonical
18m04s shared-header serial rebuild,8290 names0missing/67resources/12locales/
22-file Defender scan pass;22 installed hashes/144 protected/other5 unchanged.
Recovery0a511191. No compiler running. Frozen receipts are under
build_check/crystal_tower_20261029/; do not rerun its exact-source verifier after
later audit changes. Sol only/no game launch/probe/delegation/publication.
Resume full map audit at22 NPC-context acceptance/future-floor moving targets,
then7 conditional arrivals/power,245 silent families,2 closures/5 source owners.
Istory and FFVI mask now pass canonical; applicable other-game delivery remains
pending. Source-owner resume inventory is saved in
build_check/map_event_audit_20261028/source_ownership_resume_20261029.json.




Current audit checkpoint: [FFVI Imperial Camp mask projection](NPC_MASK_PROJECTION_20261029.md)
is source/data/targeted offline Closed (8→0, exact2 opcode delta, all-six tracking
2454 roots/2523 identities0fail). Other metadata/terrain preserved; canonical/
applicable delivery pending. Resume the remaining NPC contexts, starting with
the actual Istory sheep prerequisite, then arrivals/power/scene-owner cases.

[Invincible preference](INVINCIBLE_PREFERENCE_20261029.md) is source/offline/FFIII
delivery Closed in0a511191/11348480 bytes. Canonical/smoke/scan and installed22/
protected144/other5 preservation pass. Human confirmed Manor's Nautilus-only
wind restriction; no change. Recovery63a24c76; Sol only, no game probe.

Active finite [Ghido shore landing](SCRIPTED_SHORE_LANDING_20261029.md): actual
NPC/entry planner4 before failures; native scripted endpoint12,32 reaches both
variants. Existing override adds one same-map edge/landing with exact preservation
delta; after4→0 and combined canonical pass. No Closed route/capture redesign.
Included in FFIII0a511191 payload; actual FFV installation remains pending.

Human-priority interruption: [cross-floor animated objects](REMOTE_INTERACTIVE_OBJECTS_20261029.md).
Shared collector omitted AnimEntity10. Lute/Fang real-data20 before failures→0;
capture/real label/publication and all-six regressions pass.
FFIII installed63a24c76 passes canonical/smoke/8458 names/67 resources/12 locales/
22-file scan; human closed the game and guarded deployment verified22 installed
files/144 protected/other5 unchanged. Object inclusion and bounded instruction
capture are source/offline/FFIII delivery Closed. Resume the remaining NPC
context evidence; no map data was changed for a speculative same-map connection.

Active [22 NPC context evidence](NPC_GEOMETRY_CONTEXT_20261029.md): original
inventory and finite geometry host pin all22 cases. New real classification
defect: active RoutePuppet102 in III/V/VI was mistaken for stationary; exact
native constructors/reporting/restoration differ from I/II/IV. Completion retains
102, so the initial scalar-only candidate is superseded, never deployed. Current
bounded ready-controller observation checks remaining movement; completion/
animation/overload/Wait/manual and canonical checks pass,8458 names0missing,
included in installed FFIII63a24c76.1000 complete8-actor overload episodes stay
bounded; exact final timings are in the linked context record. Eleven exact V/VI route assets prove78 native live
positions/51 entry-connected approaches, covering11 of22 contexts. Remaining11
scene/entry/region cases remain open. No compiler running.

Next active audit capability:22 NPC geometry context cases. Estimate45–90minutes
for this finite list plus any concrete minimal regression correction. Do not
reopen Closed navigation designs. [Field effects](FIELD_EFFECT_COLLISION_20261029.md)
is now source/offline Closed:101 actual records,14 Karnak route witnesses/11
detours, actual world Enterprise13/Invincible11 movement and native flight path
pass. Final extension changed only checks/docs; installed75de3594 hash unchanged.
Then retain7 arrivals/power,245 silent families/2 closure/5 source owners and
startup's separately open naturally-failed-log diagnosis. No compiler running.

Current milestone: FFIII75de3594/11340288 bytes installed/hash-verified after human
closed/deploy. Artillery/command-commit/confirmed suffixes/ladder/Elder are source/
offline/FFIII delivery Closed; Torna source/offline is Closed, FFV installation
pending later all-six delivery. Full serial142s, smoke,8419 native names0missing,
67 resources/12 locales/22-file Defender scan pass;22 installed hashes match,
144 protected/other5 unchanged. Recoveryad0d80c1 retained; no game launched/probed.
Authoritative receipt: build_check/ff3_oct29_followups/completion.json.

Resume the open finite audit at field-effect native flight applicability and
actual Karnak route acceptance, then22 geometry,7 conditional arrivals/power,
245 silent families/2 closure/5 source owners. Earlier active build/candidate
statements below are superseded checkpoints; do not reopen Closed designs.

Human-priority interruption: [non-target command reset](COMMAND_COMMIT_FOCUS_20261029.md),
native/source complete, all-six48→0, existing focus regression0. Keep artillery/
exit/ladder/Elder/Torna/effect source and all broad-audit cases. No candidate has
been installed since ad0d80c1. Build22383 failed an isolated scan-host stub link;
the host is repaired. Canonical checks must rerun after this new scoped fix.

Latest finite priority checkpoint: [Invincible artillery](INVINCIBLE_ARTILLERY_20261029.md)
12→0 and [exact confirmed exit suffixes](CONFIRMED_EXIT_SUFFIXES_20261029.md)8→0;
source/offline complete, canonical serial combined build22383 running. Ladder/
Elder and Torna ship-entry/scene eligibility are included. Installed FFIII remains
ad0d80c1 pending final checks/scan/delivery. No C++/embedded-data edits while the
compiler runs. Broader audit resumes at field-effect flight applicability, then
the retained22 geometry,7 arrivals/power,245 silent families/2 closure/5 owners.
Torna221 scripted-world converter cases and101 effect capture/projection cases
pass; this does not settle every story-state or world-aircraft physics case.

Active human steering: [Torna Canal and IV/V/VI world-script capture](TORNA_WORLD_EVENTS_20261029.md).
Code-only diagnosis; explicitly no tester-log search. Solid-effect source/data
checkpoint is preserved uncompiled, no compiler running; FFIII ad0d80c1 remains
installed. Resolve tester event/ship-entry pipeline, then resume effect acceptance
and the remaining finite audit. Closed designs stay Closed except scoped actual
regression integration repairs.

Current milestone: FFIII ad0d80c1/11327488 bytes installed/hash-verified.
[Invincible mountain crossing/Falgabard](INVINCIBLE_MOUNTAINS_20261029.md) is
source/delivery Closed: actual owned provider/category/manual-action/landing
checks, all3 entrances, floating model11, cancellation and other-game abstention
pass. Canonical build/smoke/67 resources/12 locales/22-file scan pass;22 installed
hashes match,144 protected files/other5 unchanged. Source-bearing native evidence
remains outside Git. No game launched/probed, agent or publication.

Next active finite checkpoint: the8 FFV Karnak fire-effect entities omitted by
ordinary visual-effect filtering. Resolve whether their native collider/action
actually blocks traversal, then test the minimal route-only correction if needed.
Native/asset proof now confirms real solids and expands this same omission to
101 collision-enabled effects across III4/IV1/V96, including nonscripted walls.
The other4234 decorative effects remain excluded. Exact scope and before/after
acceptance: [field-effect collision](FIELD_EFFECT_COLLISION_20261029.md).
Estimated45–75minutes plus applicable combined/release checks. Other closed
navigation systems retain their accepted design. Then22 geometry cases,7
conditional arrivals/power rearming,245 silent families,2 closure/5 owner cases.
Do not describe the full audit or startup root cause as complete.

Current milestone: FFIII d3053479/11311104 bytes installed/hash-verified after
the human's [page/vehicle follow-ups](ITEM_BUFFER_PAGES_20261028.md). Native page
readiness, Can surface, cold-discovery exact vehicle names and retained-route
movement validation are source/delivery Closed. The per-leg vehicle selector
implementation also passes canonical current/parked, scanner/filter, continuation,
cancellation, transition and all-six metadata checks;8406 native names/0 missing,
smoke and22-file scan pass.144 protected files/other5 installations unchanged.

Resume finite audit work: remaining trigger-power lifetime scope, then Invincible
mountain crossing/Falgabard, FFV fire effects,22 geometry cases,7 conditional
arrivals,245 silent scene families,2 closure/5 owner questions. Startup's exact
original cause still awaits naturally produced failed-launch evidence. No new
game launch/control/probe or agent. Preserve Closed designs; minimal actual
regression/integration fixes remain scoped to this human-requested audit.

Historical checkpoints below are superseded by the current milestone.

Human interruption [turn-order display](TURN_ORDER_DISPLAY_20261028.md) is Closed:
no displayed list/no Shift+T added. [Vehicle applicability](VEHICLE_APPLICABILITY_20261028.md)
resumes with incomplete/uncompiled later source preserved, not a ready candidate.
Capture4→0; all-six metadata/arc tests have one corrected, not-rerun fixture
assumption. Finish phase validation/warning integration and later checks on resume.
Installed FFIII9146ae03 remains unchanged. No new deployment/game probe/agent.

Latest milestone: FFIII9146ae03/11107328 bytes installed/hash-verified after
human save/close. Full serial/offline/smoke/67 resources/12 locales/22-file scan
pass;22 installed hashes match,144 protected/other5 unchanged. Battle feedback,
category applicability and the passing parked/unfiltered provider corrections
are source/delivery Closed. Full audit checkpoint1 resumes: retained vehicle-hidden
actors, exact per-leg trigger/collider target IDs and native automatic-canoe order,
then Invincible crossing and every later recorded uncertainty. No game probe,
delegation, public release or unrelated Closed-design refactor is authorized.

Additional human follow-up [save-point categories](SAVE_CATEGORY_20261028.md)
passes source/integration24→0 and is delivered. It changes only category cycling;
no native capture or masks. It does not close the full audit.

Completed intervening steering: [Boost use/Guard feedback](ABILITY_FEEDBACK_TARGETS_20261028.md).
Provider/unfiltered3→0, continent/Saronia and native scanner/filter adapters pass;
these navigation changes are now installed in FFIII9146ae03. No native
trigger-ID/collider-mask changes have yet been made. Resume the whole checkpoint1,
Invincible crossing and all six-game uncertainties; no automatic agent or
closed-design refactor.

Checkpoint1 completed provider checkpoint (installed in FFIII9146ae03): unfiltered synthetic discovery and
parked-flight provider3 before failures→0, actual full category/provider host.
Native Player1 is valid; the old valid-vehicle branch consumed it, add() rejected
it as non-aircraft, and its else skipped every parked aircraft. The correction
branches on an actually supported mounted flight and allows foot/automatic
movement to consider parked models, preserving native boarding/terrain proof.
It does not invent a walk off an unsupported mounted ship. Current map separately
retains one physically valid nearest native aerial trigger with unknown distance
when no ride is proved; real story presence, completed Saronia exclusion and
nearest-marker aggregation remain. Reachable modes keep strict flight proof.
Native-owner capture includes parked vehicles in unfiltered mode so a usable
route may still determine the representative. Existing continent and Saronia
regressions and scanner/filter adapters pass; broader mode combinations remain.
Evidence build_check/vehicle_applicability_20261028/ and combined delivery
build_check/boost_use_20261028/. Exact current FFIII hash is recorded above.

Current milestone: FFIII b2cacd6f/11103232 bytes installed/hash-verified after
full serial build/smoke/67 resources/12 locales/22-file scan. Dive/underwater/
ordinary-landing checkpoint is source/delivery Closed; details in
[Nautilus evidence](NAUTILUS_ACCESS_20261028.md). Full audit checkpoint1 resumes,
with all-unlocked vehicle capture/per-leg trigger/collider applicability Active.
New exact integration case under the existing Current map request: ordinary
blocked exits are retained now, but synthetic aerial providers still require a
reachable flight and can omit their deliberately suppressed native source when
unfiltered/on foot. Preserve nearest-marker aggregation and real story presence;
provide discovery independently from route proof. Do not declare literal Current
map coverage complete until this actual capture/provider/publication path passes.
Initial next-checkpoint estimate60–120minutes including native order/applicability
proof and actual-source checks; report new dependencies/overrun. No agents/game
launch/control/probes/publication. Startup root-cause diagnosis awaits naturally
produced user launch evidence; the unsafe ordering repair is not causal proof.

Active human follow-up before startup deployment: [Nautilus dive/underwater](NAUTILUS_ACCESS_20261028.md).
The logged underwater origin113,118 reaches every real entrance after correcting
classification and mounted-entry policy; all4 arcs and256x256 terrain were already
baked. No synthetic surface dive markers are needed for that reported state.
Native9 uses end_landing358 and confirm terrain16 (not down_script358). Underwater
14's real transfer scripts disembark inside entry. Add Can dive on actual native
guide availability, then combined verification/delivery. Full all-unlocked vehicle
applicability and the six-game audit remain open; do not mark them complete here.

Latest human clarification: the parked-vehicle requirement covers EVERY unlocked
vehicle, not only Nautilus. Carry actual ownership, legal boarding, movement and
landing through capture, browse eligibility, publication and selected routes.
Also inspect Nautilus underwater capability: missing dive/surface or underwater
entrance arcs are part of checkpoint1, with native source-to-bake witnesses.
Current map must retain all real destinations on its bake even when unreachable;
reachability and story-gate pruning cannot silently override this mode. Existing
explicit physical grouping/category preferences and native story presence remain
separate from accessibility. Do not invent access to an unavailable vehicle.
Startup candidate0a6c18a3 passes full serial build/smoke/67-resource verification
and exact22-file Defender scan. Deployment is held only by the running-game
guard; the human has been asked to save/close. Navigation investigation continues
independently while installation waits. No new game probe or delegation.

Dalg report withdrawn by human: destination is already present; it was misheard.
Do not implement a Dalg change for that report. Startup diagnosis remains active;
resume the preserved full audit after the startup checkpoint.

Next confirmed FFIII progress regression after startup: Doga's Manor requires
landing then walking, but current Nautilus directions fly through the entry.
Restore the landing assertion in the existing boarding check; it was weakened
after observing the incorrect one-phase native-mod route. Check ordinary exits
for every manual aircraft, preserving real airborne/ship-scene/pad exceptions.
This is user-initiated scope; do not force foot entry for actual airborne scenes.

Human-prioritized native-mod startup fault now defers this audit; preserve all
current classification checks/data and native crossing evidence. On completion,
fix Doga's Manor landing/walking and parked-Nautilus reachability: Current map
with vehicles must list destinations obtainable by legal boarding, rather than
requiring the player already aboard. Test actual parked/mounted capture, list
eligibility and selected route together. Dalg report is withdrawn. Then resume
Falgabard/Invincible/capture/applicability and all geometry/effect/scene cases.
Also review existing ordinary_world_exit flags: the older explicit _to_world
name branch bypasses its opcode grammar. This is an unverified classification
risk to inspect against all real story/physical bodies, not a confirmed defect
or permission for an unrelated redesign. No automatic delegation.

Newest exact case: [Falgabard scripted entrance](SCRIPTED_ENTRANCES_20261028.md).
Human requires ordinary transfers in Map Exits, real scenes in Events. Reviewed
generic Map entrance labels were not promoted to named/category destinations;
Falgabard also exposes an unmodelled native Invincible mountain-crossing route.
Trace category, capture, vehicle-specific eligibility and selected route together.
Latest urgent NPC/nearby walking/Book suffix fixes are deployed de3e075d; full
audit has resumed at this case within checkpoint1. Sol only/no game probe.

Latest human steering: include [nearby walking and object suffixes](LOCAL_GUIDANCE_20261028.md)
with the cross-floor NPC deployment. These narrow fixes take priority; the full
audit remains authorized and resumes immediately after their FFIII installation.

New urgent human regression: cross-floor route to the Saronia1F chest now says
No path while entering that floor correctly produces Interact: Soldier. Restore
the previous cross-floor blocking-NPC guidance before continuing checkpoint1.
Vehicle audit is preserved at its native/capture reproduction; no production
vehicle-rule changes have been made. Scope is a minimal route integration fix,
with real-map before/after evidence and FFIII delivery, Sol only/no game probe.

Human scope update October28: after deploying the Boost wording, continue the
full audit and address every uncertainty in this register. This explicitly
reopens the necessary vehicle/trigger applicability, effect-collision and
reachability/label follow-ups; it does not authorize unrelated refactors, agents,
game launch/control/probes or publication. Sol only. Sequential checkpoints:
1) vehicle applicability/capture-to-route/publication;2) scripted field effects;
3) remaining NPC/layer/arrival cases;4) all remaining scene/source/continuation
contexts;5) combined verification, documentation and applicable delivery.
Initial estimate3–6hours, dependent on native ownership/continuation complexity;
report overruns and retain concrete unresolved facts rather than certifying
unsupported runtime states. Boost wording/delivery is the only active checkpoint
until FFIII installation completes. That installation now passes: FFIII
a7a97a14/11097600 bytes,22 installed scanned hashes match,144 protected unchanged,
other five games unchanged. Audit checkpoint1 is now Active. No game probe.

The [six-game audit](MAP_EVENT_AUDIT_20261028.md) has delivered its proven minimal
repairs. This queue distinguishes incomplete context from actual mod defects.
It does not certify all story states, make a runtime failure claim from a baseline
survey, or reopen a Closed vehicle/routing design automatically. Sol only.

## Vehicle-specific trigger applicability

Current checkpoint details: [typed capture and native automatic-mode order](VEHICLE_APPLICABILITY_20261028.md).
The strengthened actual-source host reproduces4 failures with real-shaped
selector lists; native-owner violations0. Exact canoe dispatch/coroutines are
exported outside Git. Production masks/capture remain unchanged pending the
native seam-order proof and complete per-leg integration.

Confirmed source gap: capture retains powered triggers but not each trigger's
native target-transportation ID list for later planned vehicle legs. Collider
applicability is also frozen to the captured vehicle. Native exact-ID restrictions
and overlap/power semantics were reviewed. Dropping an inapplicable trigger at
capture would incorrectly discard a hazard that activates after boarding.

A separately scoped follow-up must carry owned applicability data through every
planned leg, discovery/category publication and selected routing. Preserve genuine
ground gates and native vehicle transitions. Test aircraft/ship/canoe/foot legs
with both applicable and inapplicable triggers. Do not use present player overlap
as proof of whole-map trigger availability. Saronia's reviewed scene has no native
transportation-ID restriction; its complete-capture regression is unaffected.

## Scripted field effects omitted from collision classification

FFV Map_20060, group ev_e_fire, native entity IDs90–97: all eight EffectEntity
records carry collision=true, action2 and script1043/sc_carnac_fire. The source
script is a directional fire-response animation, not a dialogue scene. Type11 is
absent from BAKED_TYPES, and current live_blockers has no type11 collision branch;
the display detector intentionally filters field effects. These are a concrete
coverage gap, not proof that every visual effect should become a destination.

Inspect FieldEffectBoardEntity.Init/Show/Hide, its actual collider ownership and
the native movement/grid fallback before a minimal collision repair. Preserve
scripted enable/disable and rectangular footprints in current/future maps. A
source-to-output fixture must include actual type11 objects, not an Entity stand-in.
The six FFIV omitted effects have collision=false and script_id0 and are visual
effects; they are not equivalent missing interactions. Raw/native material stays
outside Git in parent tools/map_event_audit_20261028/ and tools/mapdump/.

## Remaining geometry context

Production Resolver/effective objects/obstacles/interaction goals/flood were run
for all270 baseline suspects with at most64 copied threshold samples per room.
Persistent flag banks use only native bit values0/1. The samples are not a proof
that their combined global story values are a valid gameplay trajectory.

| Outcome | Records | Meaning |
| --- | ---: | --- |
| Reachable in a sampled projection | 23 | Baseline all-group result is insufficient to call these blockers |
| Inactive in sampled projections | 82 | No static reachability conclusion without a relevant active state |
| Staged outside map bounds | 143 | Inspect actual scripted/live placement; never route to off-map homes |
| Native/dynamic context still needed | 22 | Exact remaining source locators below |

The22 remaining cases are FFII Map_30131_7/entity4; FFV Map_20041_9/5,
Map_20090/22,103 and Map_30381_7/2,16; FFVI Map_20051_3/12,14,15,
Map_20051_4/12, Map_20090/24, Map_30410/21, Map_30511_7/7–13,
Map_30641_1/16, Map_30671_1/2 and Map_40005/2. Check native layer assignment,
same-cell interaction, scripted approach and actual active positions. Do not
manufacture a counter or open a story barrier from a disconnected static cell.

The original two noncardinal-only witnesses are FFIV Map_30161_3/entity23 and
FFVI Map_40005/entity2. Native interaction explicitly allows matching X/Y;
strict-cardinal survey output alone does not authorize removing production
fallbacks. The former is inactive in sampled states; the latter remains among22.
Detailed cell/group/native-property locators are retained in state_geometry.tsv
and external ffN_context_triage.json, with exact source provenance.

## Remaining scene context and arrival variants

All954 initial real-map Scene script families have complete source-graph inventory:
243 have pinned functional reviews;464 contain dialogue/encounter context;
245 need further distinction between silent narrative staging and physical
mechanics;2 have unresolved closure. This is not a claim that every remaining
Scene label is correct. The two closure cases are FFVI sc_e_1055 (native
trigger-script lookup) and sc_e_0650 (script cycle). Full graphs/body hashes and
occurrences are outside Git, not embedded game-source fixtures.

Some FFIII continent transfers in that inventory are already owned by the
synthetic Floating Continent/Surface World provider; the native Scene-labelled
actor is deliberately not published. Check the actual producer/category before
treating a raw catalog label as a missing destination. Underwater/ship travel
families with native SysCalls remain contextual candidates, not automatically
approved generic scene replacements.

The initial58 missing local source occurrences include53 FFV save points now
resolved through both pinned global variants. Five local-owner/source questions
remain: FFII sc_transition_20041_7; FFIV sc_e_0006/sc_savepoint; FFV sc_e_0076_3/_4.
Do not confuse these occurrence ownership gaps with the two closure cases above.

All eight initially ambiguous door arrivals have actual PointIn records; none is
missing. One FFV variant pair uses the same cell. Seven have conditional/dynamic
coordinates: FFIII's four Amur returns to world4/point11 and Map_30231_10/entity3
to290/point1; FFV Map_20050/entity60 to81/point1 and Map_30130/entity2 to1/point12.
Trace group/native selection before changing a valid conditional arrival.

No game launch/control/probe, new runtime capture, other-game deployment or
publication occurred during this follow-up queue's preparation. Source-bearing
analysis remains outside Git. Native/frame/audible timing and gameplay remain
separate from the offline evidence.
