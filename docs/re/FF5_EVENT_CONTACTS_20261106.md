# FFV cave routes, recovery springs, boss actors and job confirmation

- The FFV log at 74052.078 reports the first Pirates' Hideout approach as
  “12 North, 2 East, step on the warp.” Native `Map_30020/sc_e_0008` runs its
  conversation before the transfer to Cave 1F. The two alternative contacts
  at the same approach run their own dialogue and travel. The baked graph
  retains all three destinations and carries a separate scene marker. Routes
  speak the existing “Event required” warning and the approach directions.
- The log at 74313.671 names Cave 1F's healing spring “Scene trigger.” Native
  `sc_e_0205` restores HP and MP and removes conditions. A second matching
  event at map 382 has the same restorative operations. Their exact localized
  label is Healing Spring. Both keep their Events category and native activation.
- The log at 75418.343 places Wing Raptor in Objects. Its rendered actor is
  map 205 entity 17, with asset 79 and script `sc_e_0016`. The script starts
  encounter 440. The all-six actor survey also qualified FFV Shiva and
  Necrophobe and the exact FFVI Guardian fight actor. Both live and other-floor
  views browse them as NPCs. Their native action and story checks remain on
  the existing event path. The reviewed orbs, books, tablets and Eureka
  weapons remain their physical object types.
- The job confirmation log at 75876.156 has the body without the focused
  option. `Last.UI.KeyInput.JobChangePopup` stores its own `selectCursor`
  and `commandList`. The existing popup readiness callback now appends the
  rendered button selected by that cursor. Later movement uses the existing
  button reader.
- The log at 76431.203 requests the Cave Entrance from world map 1 at
  (166,107). Cave 3F connects through 2F and 1F to the other world pocket.
  The selected south entrance is the native scripted portal at (165,115).
  The ordinary world fallback previously looked for a GotoMap tile at that
  destination. It now also accepts a reviewed scripted entrance to an interior.

The data and role generator checks live in `tools/review_field_event_contacts.py`.
Exact map preservation is verified under `build_check/ff5_event_contacts_20261106/`.
The production route fixture replays the first Pirate scene and the world
position outside Cave 3F. The actual scanner fixture covers both Wing Raptor
views and the spring label. The speech fixture checks both initial job choices
and later option movement.

The latest result log at 76530.640 reads three rendered rank-one job rewards.
The installed English result template uses “mastered” for an awarded ability.
The native learning table maps rank one to Check for Blue Mage, White Magic
Lv. 1 for White Mage, and Find Passages for Thief. The native skill list
renders the award rows from the result's ability list. Separate job-level and
job-proficiency renderers exist, but the logged award names are correct for
those job ranks. The result reader keeps the game's wording.

Focused checks passed for the scanner, exact bake comparison, the complete
event label compatibility review, both popup choices and the 20-assertion
route replay. The full verification and local delivery are separate.

## Tule's World Map exit

The game log at 77802.531 shows a route request from Tule (21,38) to the
ordinary exit (32,51). Faris's farewell runs when the party reaches (32,49).
The native script transfers to World Map 1 after the dialogue. Tule's alternate
Faris scene uses the same contact tile and its own World Map arrival. Both
scripts and arrivals are reviewed in `tools/review_field_event_contacts.py`.

When a local route to the ordinary exit fails, the planner can end the leg at
the observed powered scene that reaches the same world. It retains the existing
“Event required” warning on repeated requests and retires that endpoint when
the scene is gone. The scene stays in Events. The ordinary exit keeps its
location and works independently after the scene.

## Shop equipment focus

The FFV log at 77598.843 announces the shop's Equipment option. At 77599.000
the game transfers focus to the equipment bar without a new announcement.
The matching native ShopController in each installed title opens its equipment
window and uses the same command initializer, reset and SetFocus contract.
The existing deferred command reader now accepts the shop's equipment handoff
and verifies the active window owns that command controller. The handoff
survives opening equipment slots until control returns to the shop. The
command bar claims speech while focused so cursor movement speaks once.

The six-game command fixture checks initial focus, later movement, slot
selection, reentry and inactive windows. The route fixture covers Tule's
reported approach, repeated requests and the ordinary exit after the scene.
Native instructions are stored outside Git under
`tools/native_evidence/shop_equip_focus_20261106` in the parent FFPR tree.

## Local delivery

The final candidate compiled all 281 projects and completed the canonical
offline checks. The isolated folder initially inherited a Git environment
that confused an external evidence check and lacked an archived comparison
bake. Clearing that environment and supplying the exact archived bake let the
canonical verification continue on the same DLL hash. The Tule fixture passed
27 assertions across 16 route legs. All six shop contexts passed.

The smoke test loaded the DLL and verified the forwarded exports. Defender
reported no detections for the exact 22-file payload. All six games received
the same candidate, with 132 installed files hash-verified and all 24
protected files per game preserved. Recovery remains in the isolated delivery
directory. Receipts are copied to
`build_check/ff5_event_contacts_20261106/delivered/`. Gameplay confirmation
remains with the player.

## November 10 boss contacts

In the Ship Graveyard, map 211 entity 109 is the actionable Siren contact.
Its native sc_e_0030 script stages Siren entity 16 and starts encounter 442.
The staged actor has no separate action. Both current and projected floor
views use the exact contact identity and the localized Siren name under NPCs.
The contact still uses its existing adjacent interaction approach and native
script. The label has no effect on actor visibility or battle prerequisites.

The FFV scan of actionable native type 5 and 10 encounters also identifies
Odin, Titan, the two Sol Cannon approaches, Carbuncle and Famed Mimic Gogo.
The exact catalog keys qualify their map, actor, type and raw identifier.
Examining a book, collecting tablets and opening the pyramid remain physical
object interactions even when their script starts a battle. The retained
all-six source comparison also distinguishes orbs, weapons and scene props.
The maintained catalog checker verifies each scoped encounter and all 12
locale entries. The scanner replay checks live and projected Siren.
