# Opening and prologue visual actions

## November 23 visual descriptions switch and Cid on deck

F8 → Speech → Visual descriptions saves one switch for the authored
appearance and scene speech. It defaults to On to preserve installed
behavior. On-demand field and battle appearance, dialogue and prologue
additions, silent field effects and boss visuals all use the existing Prism
speech queue. The owner's FFV log selects NVDA. Dialogue, native captions,
ordinary menu descriptions and standard battle announcements keep their
existing controls. Switching off also retires an outstanding field-effect
observation before it posts a description. Boss render tracking still owns
the observed forms. There are no new native hooks, timers or voices.

The FFV log records the Fire-Powered Ship deck scene between `66920.171`
and `66944.328`. The description catalog indexes Cid's visible speaker
sprite in map 250 and `sc_e_0045`. The automatic cue list has no action in
that scene. The script turns and moves the actors, groups the party at the
door, then runs the door animation before transfer to map 251. Cid's native
line already explains where the door leads. The distinct door opening
would make a useful single cue, while routine glances and movement can
stay within ordinary dialogue pacing. A door cue would need a verified
visible door animation presentation point. The current switch uses the
existing cue producers.

The focused checkpoint verifies the saved control in all six menus, the
appearance readers and the caption, field-effect and boss cue paths.
The checks, isolated smoke test and exact-payload scan passed. The same
build is installed and verified in all six games. Gameplay confirmation
is pending. [Delivery](../SECURITY_AND_DISTRIBUTION.md#november-23-visual-descriptions-switch).

### Speech order after an automatic description

Standalone animation descriptions, ordinary dialogue pages, captions and
scroll lines enter the existing speech queue without interruption. A
description tied to a specific page precedes that page's text in one
utterance. For a standalone cue the following page queues in order, both
when the cue is pending in the mod and when it has reached the screen
reader. Prism passes the noninterrupting flag to NVDA. The pinned backend
uses cancellation only for an explicit interrupt or stop.

The focused regression exercises the real dialogue and caption producers,
the cue bindings, the production queue and a fake Prism backend. It
checks both queue states and verifies that each follow-up carries
the queue flag. The checks passed against the code already installed in
all six games. Audible timing was not measured. Compact evidence is in
`../tools/scene_speech_order_20261123`.

### FFIV Crystal Palace reveal

The owner’s FFIV log has the opening greeting at `81961.250`, Cecil’s
question at `81976.718` and FuSoYa’s introduction at `82006.328`. The installed
configuration had no override for Visual descriptions, so the default was On.
The build playing that scene had no cue for the animation between the greeting
and question. Crystal Palace map 153’s `sc_e_0069` darkens and flashes the room
around the reveal puppet and effect asset 266 before the second dialogue page.
The field costume audit identifies the white beard and purple robe.

The exact second page adds the cue before speaking Cecil’s question, in the
same queued utterance. The introduction still supplies the name. The fixture
checks the two neighboring pages and the saved switch. The cue uses the
existing visible-page and transition checks.

### FFIV Giant of Babel awakening report

The owner’s FFIV log goes from FuSoYa’s warning at `82809.562` to Rydia’s
reaction at `82865.781` with native event changes and no spoken animation
description. The awakening is the silent overworld cut `sc_e_0070_5`.
The audit recorded smoke effect actor 1087 at instruction 97 as an evidence
exception. The same body shows the Giant’s blue armored sprite at 103,
then moves it through several steps with smoke, red tints and eruptions.
The recorded review of an effect placement did not cover this full action
or deliver a cue. The earlier “described” disposition was for the later
counterattack at `sc_e_0070_14`, whose cue binds the first explosion at
126. The log also has no matching speech for that cue before the party
boards the Giant. Its runtime admission remains unverified.

## November 20 dialogue cues

The mod now reads eight additional cues on the first page of the exact
native event message. The page has to be visible after the existing
transition gate opens. The lookup matches the title, live map, running
script and message. NPC placements also match the currently drawn atlas.
The cue is part of the existing page utterance in all 12 locales. It adds
no scene polling or native hook.

| Game | Stage and spoken addition |
| --- | --- |
| III | Altar Cave follows the leader's drop and the shake to the first page of `sc_e_0001_1` on map 153. The cue describes the four adventurers in the cavern. |
| IV | The third page on the Red Wings deck in `sc_e_0001_1` on map 342 follows Cecil's lowered head and the soldiers turning. The first finished-work page of `sc_e_0054_1` on map 325 carries one description of the engineers' work movement. |
| V | The pirate ship's first page in `sc_e_0001_1` on map 198 follows the dozing pirate and Faris's hair animation. In the crystal chamber, `sc_e_0001_2` on map 190 stages Galuf spreading his arms before the unique page. The cue checks actor 9's actual atlas, which has both the open and resting poses. The Wind Shrine page of `sc_e_0001_4` on map 209 follows the shaking chamber while the crystal is still visible. |
| VI | The first actual Narshe arrival in `sc_e_0001_5` on map 3 describes the armored party after the mounting sequence. The third page in `sc_e_1033_1` on map 122 follows Setzer's movement beside the engine. |

The focused replay passes the exact cues and rejects another script, a
transition, a hidden actor and a replaced costume. The linked build and
battle focus check also passed. These cues await the integrated install.

The FF1 bridge prologue uses the existing scroll reader to describe the
completed bird shot. The native `PrologueMessage` runs after the puppet
track on map 124, script `sc_e_0006_1`. It starts the 90-second scroll at
font size 52. That exact runtime path adds one localized sentence to the
first native line. Credits, unrelated scrolls, fades and map changes do
not claim the cue. Dialogue and modal speech checks passed, including
the native scroll path. The extraction remains outside Git under
`../tools/visual_audit_20261119`.

FFIII describes four opening shots with their existing native captions.
The first prologue caption names the dark stone passage and the figure's
green clothing and red face covering. The next shots introduce the
faceted blue crystal, the moving view of the rocky coast and clouds,
and the four adventurers standing apart in the mist.

The native prologue wrappers select their exact keys from the installed
eight-message list. The FadeMessage client records that key before
handing a 52-point caption to the current speech producer. The lookup
also checks the active field script, map and transition state. Two
shots have a visible atlas and placed actor check. Later narration,
skip and loading paths keep their original producer. All four cues use
the 12 game locales.

## November 20 battle scene audit

The frozen review accounts for all 119 installed battle script bodies
from the six games. It follows each title's encounter table, direct
native script references, the operations delivered to the scene, and
the native speech or existing boss cues. The local
`finite-battle-dispositions.json` retains every body hash and disposition.
Its SHA-256 is
`e0d4b52ffd206a3e53673f59b0a1545484dc577868271bf43652f81c1577a4d3`.

| Disposition | Bodies |
| --- | ---: |
| Battle control or ordinary battle loop | 37 |
| Native dialogue, result or battle action conveys the stage | 28 |
| Specific unverified visual boundary | 26 |
| No installed encounter or confirmed native dispatch | 17 |
| Mod speech covers the stage | 10 |
| Mod speech with a recorded earlier-stage exception | 1 |

Four FFVI descriptions enter the original dialogue utterance in all
12 locales. They describe Kefka wagging his finger, the pink-capped girl
stepping into the Ultros fight, Kefka walking and turning, and winged
Kefka descending in front of the glowing cloud spiral. The game already
voices the laugh. The final descent's earlier tier scrolls have no
verified dialogue witness. Their rendered enemies have manual art reads.

The native `EventCommand.Init` identifies the installed TextAsset by
name and starts the same Integrator in battle mode. The four messages
use the visible normal message controller, including in these battles.
The cue checks that controller, the ready battle gate, the exact native
script and the first native page. It uses no new hooks or frame work.
The remaining exceptions retain their scene-specific reason, such as
FFV's ending report overlay, Neo Exdeath's emergence after the last page,
or Atomos's displacement without a stable page. Natural cue timing has
not been tested in a game.

## November 20 pose reconciliation

The first finite pass rechecked all 6,110 frozen trace hashes. It
assigned 381 routine dispositions from exact operations and placements.
The local `finite-field-first-pass.json` retains row-level evidence. It
keeps the remaining scene judgments visible in the same fixed inventory.

The structural classification now covers 604 routine and repeated
bodies, including 72 copies of Teleport's shared effect and 42 inn
sequences with the same stay pages and motions. Other reasons include
native keyword conversations, master-selected setup and transfers to a
separately reviewed scene. The local `finite-field-structural-pass.json`
preserves the evidence against the same frozen trace hashes. Its SHA-256
is `11d3c0651c919fd2ce12d6759ba85c89ae6216756289b6d9e16ad992bd3be674`.

The local `finite-field-reconciliation.json` joins the same 6,110 bodies
to confirmed caption, scroll and rendered-scene cues. It retains exact
actions, actors, art, native calls, pages and scene handoffs.
Its SHA-256 is `03290c0f7b0ca1934efd895fef36dfa982599aebcb430b319beb67c27296f8a1`.

The final reconciliation records a disposition for all 6,110 same-map
trace groups, including all 1,109 silent priorities. It also retains the
opcodes and placed actor art behind each exception. The evidence archive
`final-field-dispositions.json` has SHA-256
`945674a48d887674558328bed9f4b73ebeb555835baa4a82b484e24ff97d9274`.

| Field disposition | All fixed bodies | Silent priority bodies |
| --- | ---: | ---: |
| At least one stage with a verified cue | 65 | 28 |
| Ordinary, repeated, or covered by native presentation | 768 | 1 |
| Additional art or action could not be timed to a verified render or speech stage | 5,277 | 1,080 |

The exception rows include the authored instruction, actor and placement
or native page where available. They do not establish a visible shot.
For example, the FFVI Magitek march initializes its sprites before the
fade, then begins three moving tracks shared with the title demo. Those
tracks have no verified new-game-only render witness. FFII's Dreadnought
capture similarly changes to a distinct frame during vehicle movement,
outside the verified show and animation producers. The fixed event
inventory review ends at this reconciliation.

The targeted reconciliation matched 42 authored pose occurrences to
unique native pages. The 24 distinct atlas and pose combinations were
decoded from the installed sprites. Fourteen stage bindings now carry
13 new cues. Celes's ending pose has two native map branches using one
spoken line. The selected moments include the fallen king in Saronia,
Exdeath's fallen armor, a moogle gesture, Krile's reunion, Locke at the
statues, and four different actions in the FFVI ending or later scenes.
Later occurrences of the same pose in a scene retain their original
dialogue. `field-pose-dialogue-dispositions.json` records all 42 choices.
Its SHA-256 is
`0216a6bb62bcce8c4a88b787aa1e485ea8399844eaa72a14347c140a4acd657c`.

These cue bindings check the exact displayed actor's art and pose at the
native first page. The pose name can have Unity's atlas clone suffix.
The focused fixture admits the current exact pose and rejects another
pose, actor, script or transition. Modal and dialogue speech checks
passed in the linked build. All 13 spoken lines have all 12 locales.

The local scene review indexes 8,504 executable field script bodies across
the six installed games. It keeps the ordered visual operations, speech
locators, presentation branches, puppet entries and placed sprites.
6,163 bodies contain at least one visual candidate or native scripted
call. This broad set includes ordinary movement, fades, hidden setup and
dialogue that already explains an action. It is a review inventory.

The local `scene_review_queue.py` groups candidates only when the installed
instruction bodies, operands, entry segments and system flags match on
the same map. It retains every script name and keeps each variant's entry
condition open for review. The queue contains 6,110 distinct bodies. Of
these, 1,109 are named story motion bodies with no local message or
general native call. The earliest audit gives these priority while
preserving the remaining bodies for review. Dialogue in a neighboring
scene can still explain what a body shows.

`scene_animation_inventory.py` resolves the live bundle and exact native
animation table for 1,762 script bodies. It records 4,586 custom clip
bindings at 10,918 instruction positions, with the frame names, offsets,
visibility and order. Built-in atlas motions, raw effect assets and
actor visibility still need their own source checks. A clip's label is
insufficient for prose. FFII's `hammer` binds to actual hammer poses,
while FFV's `Sand` binds to the ordinary frames of Boko stepping.

The table below records reviewed opening actions and a possible order for
their narration. The numbers are positions in the local compiled script
extract. Effects with direct sprite art were checked from the installed
bundles. Particle-only effects were checked from the prefab components
and their script placement. The FF1 bridge cue has an offline-checked
speech binding. All cue work is still outside the installed mod.

| Game and source | Visual action and native text comparison | Proposed cue placement and wording |
| --- | --- | --- |
| FF1 construction, `Map_20170/sc_e_0005_2` and `sc_e_0005_6` | Both scripts give the first group of ten soldiers matching arm motions during the opening camera pan. The first dialogue is about the king's orders. The following work montage repeats the arm motions. | “Red-armored soldiers raise and lower their arms as they build the bridge.” The first visible dialogue page is the verified speech anchor. The cue follows the initial pan and precedes the recurring work montage. |
| FF1 world, `Map_10010/sc_e_0001` | The opening calls the rough-sea effect at 2, switches on a sandstorm at 15 and switches it off at 22. It changes the sky at 43 and restores it at 54. The opening narration already mentions raging seas, dying winds and decaying earth. A generic darkness or rough-sea cue would repeat that narration. | At the visible start of the sandstorm, “Sand sweeps over the landscape.” A single later sky cue can cover the first visible flash beneath the dark clouds if the sky transition is visible when the native text clears. |
| FF1 bridge prologue, `Map_40001/sc_e_0006_1` | Entity 1 uses `ef_ff1_e_017`. The renderer bundle and its shared texture dependency contain pale blue-white birds in alternating raised- and lowered-wing frames. Main starts the puppet block after the fade at 3. The effect starts at 21 and hides at 22. The native `ExecPuppet` wait completes before the prologue scroll begins. | “A flock of pale birds crosses the screen.” Once when the effect becomes visible. The later scroll cannot establish that the birds are currently visible. |
| FFII Fynn, `Map_20020/sc_e_0001` | The scenario-101 branch stages the Emperor and imperial soldiers separately from the earlier ordinary town movement. Puppet 72 uses the Emperor's violet-and-gold outfit and `HandsUp` at 173. Puppet 106 starts the ring, fire and trail effect at 182, followed by the separate flame placements. The native opening narration covers the attack and the Emperor calling forth monsters. | “The Emperor raises both arms. Fire spreads across the town.” Once on the arm and fire sequence. |
| FFII Fynn Castle, `Map_20041_7/sc_e_0001_1` | Hilda, Gordon and Scott have separate named placements 51–53. Guards 54–55 and imperial soldiers 56–59 occupy opposing paths. The opening text already explains the rebels and the assault. The Hilda-specific body at 84–90 turns and lowers her head before the imperial approach. | “Hilda steps forward and lowers her head.” Use the first settled pose when that staging is visible, with no second announcement of the assault. |
| FFII Altair, `Map_20011_3/sc_e_0001_3` | The healing effect is placed above the resting actor at 111 and played at 113. Its prefab contains star particles and a glow. The dialogue already says Minwu's sigil strengthens the wounded party member. Hilda and Minwu leave in the 136 and 142 puppet bodies. | “Points of light shimmer over the bed.” Once when the healing effect appears. Keep the native description of what the sigil does. |
| FFIII `Map_40001/sc_e_0001` and its prologue branches | The Gulgan shot stages actor 3 in `Map_20141_2/sc_e_0001_3` between dark rocky walls, in a green garment with a bright red face covering. The return to `Map_40001` stages one faceted bright blue crystal (`mo_ff3_b002_c00`). The world branch moves the camera anchor north from (96,44). The local tile crop shows rocky coastal edges and waves. The far layer is a brilliant blue field with small white clouds. The cave branch places the four adventurers apart with a fog foreground. Native text explains their exploration, the earthquake and the prophecy. | A short cast or setting cue can name the green-robed Gulgan and the bright blue crystal as each appears. “The view pans across a rocky coast and white clouds” follows the world pan. “Four adventurers stand apart in the mist” describes the cave shot. |
| FFIII Altar Cave, `Map_30011_1/sc_e_0001_1` and `sc_e_0001_2` | The first camera shake at 6–9 precedes the explanation of the sinkhole. The party members turn toward one another, then three of the four move into the group and their separate puppet sprites hide. The first battle begins at 27. The post-battle dialogue identifies the monsters and the intent to leave. The individual puppet bodies briefly reappear shoulder to shoulder, then gather again. | “The four adventurers draw together” can accompany the first visible regroup before the fight. Keep the existing sinkhole and monster dialogue. The second regroup repeats the same visual information. |
| FFIV Red Wings, `Map_10010/sc_e_0001` and `sc_e_0001_6` | Five red airships use one exact rotor sprite, in the V formation. The landing continuation repeats a sequence of bird-view motion and ends with the five ships turning together. Shipboard dialogue later calls for landing. | “Five red-and-gold Red Wings airships fly in a V formation, their pale rotors turning.” Use the first grouped flight. Describe the change in formation only if the landing shot visibly changes it. |
| FFIV Mysidia, `Map_20241_1/sc_e_0001_2` | The flashback has no `Msg` instruction. Seven red-uniformed guard puppets face robed townspeople. The first strike stages twin blue slash emitters at instructions 226–235. A later impact moves the elder, and the blue crystal disappears at 402 with a bright effect placed on the same tile at 405–406. The surrounding shipboard dialogue gives the reason for the raid. | “Red-armored soldiers close in on the robed townspeople. Blue slashes flash among them.” Once at the first assault. Later, “The blue crystal vanishes in a flash.” Use the crystal's visible change. |
| FFIV shipboard, `Map_20230/sc_e_0001_1`, `_3`, `_4` and `_5` | The deck exchange uses exact poses of Cecil's dark armor (`mo_ff4_p001_c00`). Cecil lowers his helmet at `_1` 58 after the sentry's report. He raises a hand at `_3` 115 after the lookout rushes forward. The native dialogue gives the crew's mood, warning, combat order and landing approach. | “Cecil lowers his helmet” at the visible pose, and “Cecil raises a hand. The soldiers run into position” if the battle preparation is visible before the native transition. Preserve the attack and landing warnings. |
| FFV Castle Tycoon, `Map_20250/sc_e_0001` | The king and two Wind Drake body pieces use the coordinated puppet entries for king actor 32 and dragon actors 48 and 50. The native exchange gives the Wind Shrine destination. The text leaves the mount and exact takeoff to the presentation. | “A gray Wind Drake spreads its broad teal wings and lifts King Tycoon into the air.” Once at the first visible takeoff after the castle exchange. |
| FFV pirate ship, `Map_30030/sc_e_0001_1` | Actor 44 uses `Snooze` at 54, showing closed eyes and folded arms. Faris's `Hair` frames at 60 and 153 show the purple hair shifting. The dialogue at 29 already identifies the wind stopping. The two audio settings converge before the same scene animation. | “A pirate dozes on deck while Faris's long purple hair flutters.” Once in the visible setup. Keep the native wind line at its original point. |
| FFV meteor, `Map_40000/sc_e_0001_5`, `Map_40020/sc_e_0001_6` and `Map_40000/sc_e_0001_8` | The campsite sequence has Bartz and Boko, followed by the quake. The meteor effect is actor 2 on `Map_40020`, played at 21. Its prefab contains a tumbling rock, a layered cloud and violet shockwave components. The later campsite return includes a separate sand effect and more motion. | “A meteor hurtles down, trailing light and clouds.” Use when the meteor effect appears. The campsite reactions still need comparison against visible puppet positions. |
| FFVI opening narration, `Map_40037/sc_e_0001`, `Map_20010/sc_e_0001_1`, `Map_20260/sc_e_0001_2` and `Map_40037/sc_e_0001_3` | The text introduces the War of the Magi, the development of machines, the return of magic and the expedition to Narshe. The cutaways show six separately placed townspeople in Narshe and the colorful jester figure in the interior. The next branch stages three riders in armor before the line about the city. | Briefly describe the jester standing alone in the interior when the camera reveals him. The local text establishes the question about restoring magic. Spoken intent and the riders' body art can use the normal current context. |
| FFVI Magitek march, `Map_40000/sc_e_0001_4` | Walkers 2–4 use `mo_ff6_n076_c00`, and actor 5 is the snow and drifting snow effect. Positions 60, 63 and 66 start their long coordinated movement. Native local variable 7 gates the title-demo exit. The new-game branch can set the skip destination to the first Narshe scene. | “Three dark Magitek walkers stride through falling snow in a staggered formation.” Use once in the new-game march after the native opening exchange. Skip paths and title-demo ownership still need runtime verification. |

## Opening handoffs and additional poses

| Game and source | Reviewed action and placement |
| --- | --- |
| FFII opening text, `Map_40001/sc_e_0001_10` through `_16` | The seven branches render the native opening text in ten calls. The end-title prop is a hidden staging actor in these scenes. Keep the native narration and use separate cues for the town fire, castle staging and other visible actions. |
| FFII reunion and audience, `Map_20011_1/sc_e_0001_4`, `Map_20011_2/sc_e_0001_5` | The dialogue identifies the reunited companions and the person still missing. Hilda's exact actor 49 lowers her head in the audience, with two separate `LookDown` poses. A single cue can describe the first visible pose. Her dialogue supplies the rebels' decision and explains the wild rose. |
| FFIV Baron arrival, `Map_20010_1/sc_e_0001_7`, `Map_20011_1` through `_3` | Red-uniformed guards and Baigan escort the party in ordinary walking poses. The native speech says why the captain is summoned and when Baigan goes ahead. Keep the scene's escort context with those lines. |
| FFIV throne room and departure, `Map_20011_4/sc_e_0001_11` and `Map_20011_2/sc_e_0001_12` | Cecil steps toward the throne and tilts his helmet down at 247. The crystal handoff, the king's order and Kain's intervention have native dialogue. “Cecil lowers his helmet before the throne” adds the physical gesture. Kain joins Cecil again outside in ordinary movement with a spoken exchange. |
| FFV Galuf and the crystal, `Map_30011_1/sc_e_0001_2` and `Map_30041_8/sc_e_0001_4` | With the room shaking and repeated full-screen flashes, Galuf uses `HandsUp` at 56. Later the Wind Shrine closes on the mounted crystal at actor 30. Actor 30 hides at 111 as actor 75 starts the shatter effect. The king's short reaction leaves the shattering to the animation. Use one cue for Galuf's raised arms and a separate cue at the crystal break. |
| FFV camp, `Map_40000/sc_e_0001_8` | Bartz and Boko exchange nods in the two branches at 68 and 75. The native script's animation table binds both nods to lowered-head and upright poses. Bartz hops at 84. Boko then turns and alternates two stepping frames. The `Sand` alias uses those ordinary stepping frames. “Bartz and Boko nod to each other. Bartz hops and Boko steps in place” follows the distinctive part of the shot. |
| FFVI Narshe arrival, `Map_20010/sc_e_0001_5` | The introduction uses native Magitek dismount and mount calls before the first playable field. The party enters armored as the native voice directs the woman to lead. Describe a dismount or remount only at the actual visible transition. The tutorial and Narshe title continue through their existing readers. |

The traces are stored under `../tools/visual_audit_20261119` beside the
actor, sprite and effect evidence. Remaining opening action candidates
and the wider scene review remain open. A finished cue needs a verified
native presentation and playback point that respects cutscene, skip and
loading gates, as well as localization and offline verification.

The first Altair audience pose is bound and offline checked in the existing
dialogue reader. The first page follows the completed `LookDown` and
`Default` sequence. It speaks a single short line before the native text.
The later head-lowering animation retains its native dialogue. Tobul's
first scene already gives the reason for forging, then plays five hammer
sounds over the fade. His later visible hammer cycles at `sc_e_0016` 204
and 211 and `sc_e_0016_1` 11 need their own presentation point. In the
second smithy, the corresponding native talk page is at 2, before the
hammer animation starts.

## Work and repair scenes

| Game and source | Visual action and native context | Placement |
| --- | --- | --- |
| FFII Altair smithy, `Map_20011_9/sc_e_0016`, plus the smithy continuation `Map_20131_3/sc_e_0016_1` | Tobul tells the party he will make weapons and armor from the mythril. The first forging passage fades out during five hammer sounds at 30–38. Both scripts' animation tables bind `hammer` to two mirrored poses of `mo_ff2_n022_c00`, showing the actual tool raised and lowered. | The first visible cycle at 204 speaks once after the raised hammer frame appears. The later cycle at 211 and repeated smithy interaction keep their native audio. |
| FFIV Baron Enterprise upgrade, `Map_20011_20/sc_e_0054_1` | Engineer actors 2 and 3 use the two palettes of the same overalls costume. They move in paired puppet bodies at 93–101 and separately at 104–117. Actor 3 starts the first named jump pose at 112. The engineers' dialogue covers the finished attachment, hovercraft controls and onward travel. | At the first visible work motion, describe the two engineers moving into position and the engineer hopping during the work. The atlas lacks a dedicated tool frame. Keep the later repeated motions within that one cue. |
| FFV Adamantite airship upgrade, `Map_20231_9/sc_e_0071_1` | Cid wears a red coat and has white hair and beard. Mid has blond hair and a violet coat. Their paired puppets show them moving across the scene, with entrances, pauses and repositioning. The first coordinated move begins in the pair at 52–62. | “Cid and Mid move around the airship as they work on the upgrade.” Use one cue across the montage after verifying the first clearly visible shot. The exact body tracks establish the actors' movement. |
| FFVI optional Blackjack repair, `Map_20171_3/sc_e_1033_1` | The native dialogue explains the engine damage first. Setzer actor 6 uses `DyingR` at 104. The atlas binds that cue to the exact forward-bent sprite. Setzer is hidden and repositioned to the engine, then shown using that pose at 110. | “Setzer leans forward beside the engine.” Use the reveal after 110. The dialogue supplies the reason for the repair. |

## First FF1 event pass

This pass reviews the eight FF1 story-motion bodies without a local
message or generic native call, their relevant continuations and a set
of treasure pickup bodies. The movement and effect findings use actor
keys, decoded sprites and the installed effect prefab components.

| Game scene | Visual action and placement |
| --- | --- |
| Cornelia bridge handoffs, `Map_10010/sc_e_0005_1`, `_5` and `sc_e_0006_3` | The first two scripts place their camera and player actors for the transfer into the bridge construction. The third restores the party's appearance and control on the world map after the prologue. The distinctive visuals belong in the construction and prologue themselves. |
| Canal opening, `Map_10010/sc_e_0015_1` | One flash and two shakes accompany particle emitters for soil and stone and actor 102's distinct blue water plume. Candidate once as the plume appears: “A flash lights the ground. Dirt and blue water spray up as the canal opens.” The following room dialogue covers the opening's outcome. |
| Fairy release, `Map_20150/sc_e_0028` | The field leader raises a hand before using the bottled fairy. Actor 119 then plays `ef_ff1_e_010`, which contains fairy light and star effects. Candidate: “The leader raises a hand. Points of light rise as the fairy is released.” Use the effect's first visible sequence. |
| Levistone and airship, `Map_20180/sc_e_0025_1` and `Map_10010/sc_e_0025_2` | The subterranean shot stages the party around a small silver-blue diamond-shaped object and flashes the screen twice. The world shot uses nine sand effects, a camera change and the native airship reveal. The award message later says the party has the airship. Candidate pair: “The party gathers around a small silver-blue stone. The screen flashes white.” Then describe the airship appearing among clouds of sand at the first world reveal. |
| Crystals, `Map_40000/sc_e_0001_5` | The opening hides the player and plays actor 1's full-screen effect at 18 between fades. Its prefab contains blue, green, red and yellow crystal components, stars and flares. Candidate: “Colored crystals and points of light appear against the darkness.” Verify the effect timeline before describing their order. |
| Ending, `Map_40004/sc_e_0044_3` and `_4` | The first body shows actor 3 at 67 after the credits. Its blue outlined graphic reads “THE END.” The `_4` body hides the graphic and the preceding effect before the final save. Speak “The End” once at the actual reveal if the credits reader has not already announced those words. |
| Named treasure pickups, `Map_20011_1/sc_e_0014`, `Map_30021_3/sc_e_0010_1`, `Map_30031_3/sc_e_0017`, `Map_30071_3/sc_e_0047` and `Map_30081_8/sc_e_0033` | Each uses the ordinary treasure pickup pose and a native item announcement. The reviewed bodies use the standing character pose and standard chest interaction. No bespoke item sprite is staged in these five scenes. |
| Pravoka after the pirate fight, `Map_20040/sc_e_0009_2` | The crew's two bodies play `FlashingSurprise` at 43 and 59. The installed clip alternates opaque and transparent `Surprise_00` frames. Each actor then hides. “The two pirates flicker and disappear beside Bikke” uses the first surrender page at 11 after both blocking puppet bodies. The subsequent ship award keeps its native text. |
| Western Keep, `Map_20081_1/sc_e_0011` | The crowned actor uses the `Laugh` clip before the first line. After the first reveal dialogue, the next puppet block flashes and shows actor 106 at the same tile. The first page after those actions describes the change to the small, dark green figure. The following encounter has a separate battle description. |
| Adamantite, `Map_30111_2/sc_e_0051` | Actor 31 is a visible cluster of upright violet crystal points with bright white edging and a dark base. The cluster is hidden as the party takes it. The native award names the item. Describe the violet cluster at its visible pickup, using the actual prop instead of the ordinary treasure pose. |

## First FFII world-scene pass

| Game scene | Visual action and placement |
| --- | --- |
| Dreadnought launch and attack, `Map_10010/sc_e_0019_1` | The dark blue and gold Dreadnought rises through near and far steam emitters, with debris and wind effects around the launch. A sequence of explosions and white fade cuts follows. “Steam and debris billow as the dark Dreadnought rises” describes the first lift. One subsequent cue can cover the attack, without announcing every explosion. |
| Airship capture, `Map_10010/sc_e_0035` | Actor 232 is the small golden airship. The much larger dark craft moves alongside it, changes to the `Holding` sequence at 91 and carries a small golden shape beneath its hull in the following frames. “The Dreadnought catches the smaller airship and carries it beneath its hull” at the first visible capture. |
| Dreadnought escape, `Map_10010/sc_e_0039_8` | The world cutaway positions separate small-airship and Dreadnought actors. Steam, flame explosions, a shake and a white fade accompany the escape. “The smaller airship pulls away as explosions flash across the Dreadnought.” Use one cue at the first visible separation and explosion sequence. |
| Wind Drake flight, `Map_10010/sc_e_0080_1` and `_3` | Actor 343's decoded frames show a gold-bodied drake with broad blue wings, viewed from behind and from the side. The departure and arrival are separate scripted flights. Introduce the first departure once as the drake spreads its wings. Read a new cue only for a distinct arrival action or setting. |
| Palamecia and Pandemonium, `Map_10010/sc_e_0086_1` | The drake moves past the tall gray castle. Palamecia actor 294 has narrow towers over yellow-brown ground. The arrival effect plays on actor 384 immediately before the castle sprite hides. “The Wind Drake flies past the tall stone castle. The castle vanishes in an eruption.” The later Fynn dialogue explains the Emperor's return and the party's losses. |
| Dreadnought interior escape, `Map_30051_1` and floors `_4` through `_8`, `sc_e_0039_1` through `_7` | The seven bodies form one escape sequence. Gray-blue imperial soldiers with red plumes move in groups through several passages. Shake and overlay events recur. Describe the first visible rush through the shaking corridors once. Preserve the separate world shot as the escape moves outdoors. |
| Pirate ship, `Map_10010/sc_e_0044` | Actor 176 is a brown boat with large blue-white sails. The world shot shows it move before the fade into the pirate scene. “A small wooden sailing ship crosses the water.” The following dialogue supplies the attack and the pirates' intent. |
| Mysidian Tower seal, `Map_10010/sc_e_0074` | Actor 333 uses a full-screen effect with a circular staff glow, rising tower beam and wide wave. “A ring of light brightens and a beam rises at the tower” describes the seal animation before entry. Narrate this once across the effect's repeated glows. |
| Prison, `Map_30081_2/sc_e_0054_2` | The captive is staged in a down pose above a green spiked grate, then stands and turns from side to side. The preceding audience dialogue already orders the party jailed. “The captive rises and looks from side to side. Green iron spikes line the grate” supplies the cell's visual detail. |
| Deist and Palamecia drops, `Map_30071_3/sc_e_0112` and `Map_30141_2/sc_e_0085` | Both script-local `Turn` clips cycle the four facing poses of the field leader before transferring floors. A single cue can describe the turn and drop on the first visible occurrence. The exact native destination and route remain in navigation speech. |
| Cyclone approach, `Map_10010/sc_e_0081` | Actor 346 is the blue-winged drake. Separate wind, smoke and funnel effects surround the flight into the Cyclone. “The drake flies toward a swirling funnel” at the first full view of the wind effects. The `_2` continuation is a short positioning move. |
| Jade Passage entrance and exit, `Map_10010/sc_e_0087` and `Map_30151_13/sc_e_0088` | The first effect gathers light in a whirlpool-like circle before the transfer into Jade Passage. The exit uses a different effect with light and lightning at the transition to Pandaemonium. Describe the first visible formation of each effect where it is staged. |
| Pandaemonium ports, `Map_30161` group, `sc_e_0201` through `sc_e_0215` | All 15 scripts bind the same `ef_ff2_e_024` effect, then fade and transfer to their own destinations. Its components combine a twisted aura and lightning. One initial cue can introduce this visual. Later equivalent ports use their ordinary navigation context. |
| FFII ending, `Map_40001/sc_e_0089_6` and `_7` | The credits body shows an icy blue “THE END” graphic at 61 after the scroll. The cue identifies the actual visible card. The next body hides the graphic and its effect before the final save. |

## Additional silent scenes in FFIII–FFVI

| Game scene | Visual action and placement |
| --- | --- |
| FFIII Bahamut, `Map_10010/sc_e_0026` | Actor 72 has a curved gray tail, ribbed pale body and dark claws. It sweeps toward the party at 26. The leader looks around, then the leader's individual sprite disappears just before the flash and transport to map 166. “A bony gray dragon with a pointed wing swoops toward the party.” Describe the actual grab after the field pose or next shot establishes it. |
| FFIV Giant of Babil, `Map_10010/sc_e_0070_14` | The blue armored Giant stands opposite four tanks and a nearby airship formation. The Lunar Whale approaches on a separate track. Multiple fire effects burst around the Giant. “Tanks and airships gather around the towering blue Giant. Explosions flash around it.” Use once after the formations come into view. The following Lunar Whale scene supplies the story context. |
| FFV Ronka ruins, `Map_10010/sc_e_0075_8` | The cutaway isolates the ruins and their shadow. Actor 433 takes the moving-ruins pose, followed by a sequence of many explosion emitters and camera shake. A short cue can describe the flying ruins shaking amid repeated flashes once at the first visible burst. |
| FFV moogle, `Map_10020/sc_e_0093_2` | A white moogle with violet wings appears on the world map. It moves east and back west before turning toward the party. “A little white moogle with violet wings looks back.” Time this to the first turn rather than the routine walking. |
| FFVI Mobliz, `Map_20130/sc_e_0677_1` | The girl zigzags, runs right, then seven small white birds head away along separate paths. The other townspeople also move as the shot advances. “A girl runs through the village. A flock of little white birds scatters.” Use one cue for this shot and retain the later native explanation. |

## First FFIII world and crystal pass

| Game scene | Visual action and placement |
| --- | --- |
| Dragon flyovers, `Map_10010/sc_e_0025`, `sc_e_0026` and `sc_e_0284_1` | The three scripts use the same ribbed, pale-winged `mo_ff3_n038_c00` silhouette. The `sc_e_0026` sequence has its own approach and party disappear pose. Introduce the visible dragon once in each distinct scene, with movement details specific to its path. |
| Whirlpool, `Map_10010/sc_e_0045_1` | Actor 74 uses `ef_ff3_m_010`. The prefab contains a spinning center, layered smoke and an outward ripple. “A whirlpool circles around a churning center” at the visible effect, before the interior transfer. |
| Hein's Castle, `Map_10010/sc_e_0060_3` | Actor 73 is a tall tangled gold-brown form with a rounded top, dangling branches or roots and a shadow below. It is positioned for the moving world shot before transfer to map 205. “A mass of tangled branches moves into view above its shadow.” Use the first clearly visible motion of this prop. |
| Canoe and shadow, `Map_10010/sc_e_0281` | The canoe actor appears with a spin animation. Two independently staged dark and elongated silhouettes use the shadow art. “The canoe spins as a long dark shadow moves nearby” describes the paired movement. |
| Amur and the Enterprise, `Map_10020/sc_e_0067_1_1` | A shake accompanies the world scene, with bubbles and splash components in actor 2339's effect. The Enterprise's striped brown-and-white hull is staged separately. Introduce the appearance of the ship and the climbing bubbles during the first revealed shot. |
| Saronia cannon, `Map_20091_2/sc_e_0264` and the world follow-ups | Actor 25 is a dark, round cannonball with a pale highlight. Actor 24 supplies dust. The shot starts with a shake, then a puff and the projectile prop. “Smoke bursts around a dark round shot” at the first visible firing sequence. The airship's separate world response needs its own view match. |
| Barrier, `Map_10040/sc_e_0207`, `_1`, `_2` and `sc_e_0355` | The same circular glow asset is used for several vessels. The Invincible has multiple winglike projections and bright orange bands. The Nautilus is smaller and oval. The Enterprise uses a separate striped hull. One cue introduces the visible rings when the barrier reacts. Later equivalent approaches can retain their vehicle and route context. |
| Mirror, `Map_30261_2/sc_e_0321` | The effect's installed components contain a ring, rays, filtered light and stars. A separate native turning track moves the party into the transition. Describe the light and the first turn as the party is pulled toward the mirror. |
| Crystal audiences, `Map_30011_3/sc_e_0007_1`, `Map_30251_2/sc_e_0089_1` and the four dark-crystal rooms in `Map_30281` | Upright, faceted blue crystals and separate violet dark crystals are held on their own actors. The nearby shadow effect has a dark aura, bright eye particles and a central sphere. State the actual color and shadow action on first reveal. Later copies of the same interaction keep their native text and ordinary movement. |
## FFIV–FFVI world-scene actions

The local `scene_nodes.py`, `scene_shortlist.py` and `scene_prop_review.py`
retain the source positions, actor bindings and installed art evidence.
Particle-only effects use prefab component evidence. Sprite frames were
checked separately wherever the shot swaps a prop or plays a custom clip.
These rows are editorial cues awaiting a verified presentation point.

| Game scene | Action, visible detail and proposed wording |
| --- | --- |
| FFIV Damcyan, `Map_10010/sc_e_0015` | Three red airships move as a group. A second group starts the first explosion on actor 164 at 82 and stages successive blasts. The burning pale-stone castle at 171 hides at 148, and actor 1019 shows the damaged towers at 151. “Three red airships pass over Damcyan. Fire bursts around the pale towers, leaving them broken.” Use one cue for the attack and its result when the camera reveals them. |
| FFIV Agart eruption, `Map_10010/sc_e_0045_1` | After the shake and fade, actor 1102 starts the eruption at 36. The prefab has rings, rising columns, rock and dust components. “The ground shakes and erupts, throwing rock and dust upward.” Use once as the eruption appears. |
| FFIV sealed passage on the surface, `Map_10010/sc_e_0053_10` | Actor 1113 has a diamond-shaped, rimmed opening. The first four frames advance from a black interior through red-orange glow. Actors 1088–1097 play the smoke prefab. “A dark opening glows red as smoke spreads around it.” Place at the first lit opening. |
| FFV Catapult launch, `Map_10010/sc_e_0066_1` | The four-panel gray surface plays the hatch sequence at 33. Its opening frames reveal the gold-and-orange propeller below. The airship rises at 40, then plays `propeller` at 51. “A stone hatch opens and the airship rises into view, its propeller turning.” Describe this launch once. |
| FFV Ronka rises, `Map_10010/sc_e_0067_2` and `sc_e_0514_2` | Both branches use actor 431 and the same effect with a castle body, shadow, falling sand and an expanding dust ring. The first branch plays it at 35. “The Ronka Ruins rise above a widening cloud of sand.” Treat each branch as an alternate for the same cue. The later collapse uses its separate shot and description. |
| FFV Ghido's shrine, `Map_10010/sc_e_0476_1` | Actor 723 uses an effect with upward-moving stones, broad spray and waves. “Spray and chunks of stone rise as the shrine goes under.” Use at the first clearly visible splash. |
| FFV merged world and Bahamut, `Map_10040/sc_e_0137_2` and `sc_e_0137_4` | World effect 809 combines flying stone, dust, waves and spray. The second branch starts the effect at 33 and reveals the dragon with the `Flybaha` clip at 53. The dark dragon's wings flare blue-violet and its legs and curled tail hang below. “Water and rubble surge upward. A dark dragon spreads blue-violet wings and flies into view.” Let the second shot add the dragon if the water cue already ran. |
| FFV the Void, `Map_10040/sc_e_0138_1` and `sc_e_0138_3`, with later site-specific branches | Actor 800 uses the first sphere prefab with a black core and surrounding wind, trail and lightning. Actor 801 uses the second version. At the Ancient Library, actor 189 then switches to `HoleShift` at 32 and shows the dark navy opening. “A black sphere forms amid flashes and swirling air. A dark opening remains where the library stood.” Use the second sentence when the place actually changes. Later attacks reuse the effect but have their own affected sites. |
| FFVI first Figaro descent, `Map_10010/sc_e_0036_2` | The gray-brown castle, with bright blue central details, plays the shrinking `Hiddhing` silhouette frames at 39 before hiding. Actor 247 plays the spread-sand and dust effect. “Figaro Castle sinks into the sand, its towers disappearing in a cloud of dust.” Once on the first visible descent. |
| FFVI Figaro re-emergence, `Map_10020/sc_e_0673_1` and later travel variants | Actor 156 plays its rising `Appear` frames at 39 and settles on `figaro8` at 40. Dust actor 193 uses the complementary emergence effect. `sc_e_0278_1`, `sc_e_0279_1`, `sc_e_0674_1`, `sc_e_0675_1`, `sc_e_0882_1`–`_3` and `sc_e_1067_1`–`_3` share the descent or ascent vocabulary according to the actual track. “The towers of Figaro Castle rise through the sand.” No cue for every step of the repeated travel sequence. |
| FFVI Blackjack, `Map_10010/sc_e_0558_9` | The craft and shadow are separate actors 255 and 256. Five takeoff clips start at 49 through 58. The airship has a dark rounded balloon, an ochre lower outline and pale propellers. “The Blackjack lifts off, its dark balloon above the gold-edged hull.” Once when the hull first rises. |
| FFVI Light of Judgment, `Map_10020/sc_e_1002_2` and `sc_e_1002_4` | The beam uses two directional prefabs with beam glow, wave and spark components. The first shot plays the left-to-right prefab at 35, with a parallel long movement track. The second uses the reverse prefab. “A beam sweeps across the land, leaving a wave of light and sparks.” Reserve the additional cue for the distinct reverse strike when it plays. |

## Native playback boundaries

Twenty-two silent descriptions now have 23 exact native effect bindings.
The local `scene-effect-bindings.json` records each installed actor, track,
instruction, bundle and emitter count. Its SHA-256 is
`b7c4ace0246e7fdeae71242c40ce9ff45c7f3737e0b5c92c4484aa7b920c3532`.

| Game | Visible cue |
| --- | --- |
| I | Dirt and stones spray upward at the canal opening. The blue-water sprite follows in a separate animation. |
| II | A light ring and a rising beam show the seal opening at Mysidian Tower. |
| III | Rays and points of light spread from the ring at the Crystal Tower mirror. |
| IV | The ground erupts at Agart and throws up rock and dust. |
| V | In either native branch, Ronka rises above a widening cloud of sand. |
| VI | Figaro's first descent is accompanied by a dust cloud. A separate cue describes the first Light of Judgment beam. |

The additional bindings describe the FF1 opening crystals, fairy release
and Levistone sand. FFII adds steam at the Dreadnought, the escape flames
and the Cyclone funnel. FFIII adds the Saronia cannon smoke. FFIV adds the
fires around Damcyan, the opening to the underworld and the explosions
around the Giant of Babil. FFV adds Ghido's sinking shrine, the exploding
Ronka Ruins, the merged-world eruption and the black sphere. FFVI adds
Figaro's first return from the sand. Each cue belongs to the first verified
appearance in its own sequence.

The native `Puppet.PlayAnim` dispatch keeps its original return value.
A matching first clock inside the settled native Event state arms one
bounded render observation. The observer retains the event generation
and the core index. It resolves the current actor on the owner thread,
waits until the fade clears and checks actual particles on a visible
renderer. The installed placements contain between 2 and 66 emitters. The
reader checks at most eight per owner frame and continues at the next
emitter if needed. The request expires after 16 owner frames and ends immediately
on a changed script, map or event generation. Repeated native clocks
share that observation. The loading and map-entry gates close before
any interpreter or renderer reads. All descriptions have the 12
game locales. The focused check covers native return, transitions,
hidden emitters, missing particles, fades and cancelled observations.
Actual gameplay timing awaits the installed confirmation.

FFI through FFIV and FFVI now describe the native end card when its exact
sprite is shown. FFV describes the white birds and the mounted drake in
the closing montage. Its existing gratitude and credits pages retain their
usual speech. For sprite cues the same bounded observation checks the
drawn atlas after native `Show` or `PlayAnim`. FFVI reuses the ending
actor number for a save point in another group, so the atlas check is
required.

The reconciliation also adds the FF1 water plume, FFII's fire over Fynn
and stars above a bed in Altair, Tobul's first visible hammer swing, the
three Palamecia and Jade Passage effects, the FFIII Enterprise bubbles,
the FFV crystal shatter and the incoming meteor. Only the meteor begins
before its shot's fade. It waits for the authored 0.7-second fade interval
without consulting the renderer, then admits speech once the fade clears
and the exact effect has drawn particles. Leaving the event cancels that
read. The installed-art evidence matches all 40 direct scene bindings
and 35 localized phrases in `scene-effect-bindings.json` (SHA-256
`14e747bc1fcc9e3f989088d7eabb159b6c9d58596d820d61d78afd7067163986`).
Dialogue appearance and modal speech passed in 31.0 seconds with 73
embedded assets verified. The fixture covers hidden sprites, a different
atlas, an idle frame from the right atlas, the last of 66 emitters, a
cancelled fade delay and preservation of the original native return value.

The six installed binaries agree on the split between interpreter dispatch
and particle presentation. `Integrator.Update` can execute timers and a
sandbox while its field script is idle. Its working flag therefore only
identifies active interpreter work. `Puppet.PlayAnim` checks the core's
first clock and also handles other play and wait paths for ongoing effects.
`FieldEffectParticleEntity.PlayEffectIfNeed` can enter the same
`PlayEffect` routine on automatic activation. The effect reader takes its
entry from the exact authored animation and settles on an active renderer.

The existing dialogue admission covers visible modal content in the event
state. A silent sprite, camera or effect still needs a verified marker
for its own visible action if the current producers cannot identify it.
The caption and scroll readers use the runtime's displayed-page ownership. Local
`native_scene_path.py` and the six `ff*-native-scene-path.txt` traces
preserve the ABI and relevant paths for the silent cue work.

## Routine event review

The offline pass checked 379 native script bodies against their installed
actor placements and opcode sequences. It records each resolved script
in `scene-ordinary-review.json` beside the source traces. Other actions in
the same map remain individually reviewable.

| Scope | Bodies | Existing visual or spoken context |
| --- | ---: | --- |
| FF1 roof setup | 110 | The sole native operation beyond setup and exit controls a map roof layer on entry. |
| FF1 NPC exchanges | 75 | Native dialogue and ordinary turning to face the speaker. |
| FF1 scripted pickups | 5 | The leader uses the shared pickup pose with a native item award. |
| FF1 monster chests | 40 | The default chest lid is the only animated actor and native text warns of the monster encounter. |
| FF1–FFIV standard chest rewards | 118 | The continuation fades in and awards the chest contents. The chest receiver resolves to the generic art placement. |
| FFV guarded chest continuations | 31 | Success awards the generic chest contents. The other branch resets only the tile animation linked to that chest lid. |

The six-game native event trace also confirms that
`EventActionScript.RunScript` stores its requested name and that
`EventProcedure.EventRunScript` supplies the actual loaded script name
to the interpreter before execution. The visible page remains the
presentation anchor for the existing dialogue cues.

## Opening transitions and establishing shots

The local `opening_family.py` enumerates the 52 visual and native-call
bodies in the primary opening families. The opening tables above and the
rows below cover their main cuts, ordinary handoffs and remaining exact
presentation questions. Adjacent chapters still have their own audit.

| Game and stage | Reviewed material and proposed wording |
| --- | --- |
| FF1 crystal sequence, `Map_40000/sc_e_0001_5` | The first puppet plays actor 1 before the white fade. Its installed effect prefab contains separate red, blue and green crystals, lights and stars against a black layer. “Red, blue and green crystals shine among points of light. The screen fades to white.” Use once before the handoff to the Overworld. |
| FFII Overworld handoff, `Map_10010/sc_e_0001_2` | The authored camera track pans between its origin and the new view before a white fade leads back to the opening text. Preserve the text. A short cue can describe the pan and white fade before the next page. Terrain detail needs the exact full view. |
| FFII transfer, `sc_e_0001_6` on `Map_10010` and `Map_40001` | Both bodies change saved party membership and transfer to the Altair room. They carry no visible character action. The room, healing effect and subsequent reunion have their own records above. |
| FFIII shadowed cave, `Map_30011_6/sc_e_0001_5` | A half-opacity black overlay darkens the cave while the native prologue displays the adventurers' exploration text. The four visible placements and fog use the cave cue already listed above. The later fade returns to the prologue map. |
| FFIV Baron entrance and corridors, `Map_20011_1/sc_e_0001_8`, `Map_20011_2/sc_e_0001_9` and `Map_20011_3/sc_e_0001_10` | Camera and actor tracks continue the ordinary escort through Baron. The dialogue in the third corridor identifies Baigan's departure and the destination. The throne-room helmet gesture remains a separate cue. |
| FFV Lenna after the takeoff, `Map_20250/sc_e_0001_3` | Lenna follows the departing party with the camera and calls out about the wind and her father. This is the reaction to the Wind Drake shot above. Keep the native line and avoid repeating the takeoff. |
| FFV first campsite, `Map_40000/sc_e_0001_5` | The ground shakes. Boko, actor 22, lowers and raises his head in the repeated three-frame nod and uses the surprise pose across the hop. The other track uses a one-arm pose at 43 and a two-arm pose at 45. The decoded custom one-arm clip uses a mirrored right-hand frame. “Boko bobs his head and hops as the ground shakes” covers the visible reaction once. Confirm the other actor's live placement before naming it. |
| FFV Overworld cut, `Map_10010/sc_e_0001_7` and the `Map_10010_1` alias | The world view turns and moves in two native tracks. The first map has a separate ochre Ship Graveyard wreckage prop. “The view turns and glides over the world toward the wreckage” is a candidate where the location is in view. Inspect the alternate map's rendered prop before applying the same wording there. |
| FFV end of the opening, `Map_10010/sc_e_0001_9` | The script sets transportation 3 at (157,150), which the native table identifies as Boko. Translucency and the default pose bridge into the player's travel. A single appearance cue after the fade can identify the yellow chocobo and mounted party if both are in view. |
| FFVI Narshe cutaway, `Map_20010/sc_e_0001_1` | Two helmeted guards flank the group of townspeople. A man turns through the two `Angry` instructions at 61 and 65. The white-bearded man in blue holds `CloseEyes` at 81. The native opening text supplies historical context while this cut shows the town. “Two guards in pale helmets flank the townspeople. One man turns sharply and an older man closes his eyes.” The exact expression frames for the first man remain to verify. |
