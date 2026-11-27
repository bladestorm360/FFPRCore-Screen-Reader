# November 26 bug reports and narration audit

## Scope and checkpoints

1. Close the reported FFIV and FFV defects through the normal field, battle
   and navigation producers. Verify the candidate offline before expanding
   the narration review.
2. Reuse the fixed 82-sequence campaign inventory for FFI through FFVI and
   prioritize visual beats without a cue in the reported FFIV finale and the
   larger FFV and FFVI branches. Each added cue requires a native source
   anchor, visibility or presentation evidence, and all twelve languages.
3. Pass the focused offline, smoke and exact-payload checks, then install the
   combined candidate in the established Steam targets. Record audible
   confirmation separately.

The reported behaviors passed offline checks and the verified candidate is
installed in all six games. The additional campaign pass closes the
anchors listed below. Independent presentation gaps and live confirmation
remain open.

## Reported behavior and source changes

| Report | Production change and evidence |
| --- | --- |
| FFIV Bahamut | AI 191 calls 372, 371, 226, 149 and 370 through its action/counter path. Battle sprite 162 spreads the wings and pushes the head forward before Mega Flare. Sprite 160 pulls the wings in afterward. Existing native sprite completion observes the actual replacement art. Rydia's summon uses ability 75 or 459 and effect 91. Its native attack and return-fade entry points now cue the visible light buildup and return to battle. Dark Bahamut has a different action table and no equivalent sprite command. |
| Lunar Subterrane arrival | Map 154, `sc_e_0073`, actor 20, animation 79 starts the crystal transport effect. Map 296, `sc_e_0073_1`, color filter 9 begins the arrival flickers after the 1.4-second fade and 0.7-second wait. The party's raised arms were set by the arrival puppet tracks before the fade. |
| First sealed weapon | Map 298, `sc_e_0116`, actor 20, animation 14 plays the ring, light rays and stars after page `E0116_00_298_a_01` and before encounter 508. Its one-second native animation remains independent from speech. |
| Exit and chest browsing | The browse worker keeps a usable same-map leg's instructions when no named floor, prerequisite or direct bearing conveys it. The list worker places current-floor rows first even before it has the player pose. Local distance still orders those rows when available. Exact selection and continuation identity survive sorting. |
| I in NPC dialogue | FFIV was still running the earlier `7b9a893` DLL. The field-page owner repair in `d1becca` was absent. This candidate includes it and adds 527 pages across all six games, 201 of them in FFIV. These use a native speaker name and a unique placed actor. Each manual request still requires the current visible page and renderer before it reads the costume. |
| FFIV epilogue | Native epilogue dialogue and map transitions reached the credits. The log records confirming Skip at 348815.421, which switched to `sc_e_0074_22` and bypassed the existing end-card Show command. There were no cues for Kain's hair or the wedding's silent gestures. Those now have native PlayAnim anchors. |
| FFV water currents | Ninety event contacts in nine maps force movement away from the contact or return the party to the Fire-Powered Ship's earlier passage. The baked router rejects those contacts even when a route intentionally allows ordinary event triggers. Native layers, existing passable desert currents and verified Istory travel edges retain their own behavior. |

Evidence comes from the user's actual FFIV and FFV journals, the installed
game's field script corpus and art, and the behavior-specific fixtures.
Source-bearing extracts are external to Git under `../tools/mapdump`,
`../tools/visual_audit_20261119` and `../tools/boss_visuals_20261112`.

## Campaign animation review

The prior [campaign inventory and episode dispositions](VISUAL_DESCRIPTIONS_OPENINGS_20261119.md#november-23-major-scene-re-audit)
establish 82 major sequences. The current pass revisits silent action
signals, including alternative endings and native transitions. Running
sprites, repeated flashes in one beat and shots already identified by a
native description do not generate a second announcement.

| Game | Additional visual beat | Playback boundary |
| --- | --- | --- |
| FFI | The barrel surfaces and the party gets out at the Sunken Shrine. | Map 98, `sc_e_0031_1` at 59, after the blue arrival fade and with the disembark track starting in parallel. |
| FFII | The Deist wyvern glides down toward the party. | Map 41, `sc_e_0080_2` at 55, following the visible landing motion and before the native award. |
| FFIII | A dragon hovers just above the party. | Map 1, `sc_e_0026` at 37, after the dragon and the party reach adjoining positions and before the white transition. |
| FFIV | The sealed lance chamber darkens blue. The later chamber flickers yellow-green and white. | Map 302, `sc_e_0117` at 4 and map 312, `sc_e_0118` at 7. Each emits one description for its filter sequence. |
| FFV | Shards burst in Exdeath's castle. Bartz rides a chocobo across the ending montage. | Map 406, `sc_e_0125_1`, render of actor 12 after animation 156. Map 644, `sc_e_0162_48`, display of actor 4 at 134. The drake has its separate existing cue. |
| FFVI | The first shock wave crosses the land, then the planet fractures. | Map 1, `sc_e_0607_7` actor 253 at 34 follows the fade and pause. Map 584, `sc_e_0607_9` actor 229 at 25 precedes an authored 1.4-second fade. Its observation starts after that interval and still requires visible particles. |

The largest inspected effect now has 74 emitters. The owner still observes
at most eight per frame within the existing 16-observation limit. A changed
event, map, or script closes the observation. Battle phases use exact native
action identity and the existing ready-battle gate. No scene is held for
speech. The field cue and the native caption share the queued speech lane.

The six-game census still contains historical evidence exceptions. In
particular, the independent FFVI opening march shares its field script with
the title demo, and FFV ending results overlays may begin before the ready
presentation state. Those paths need a safe, independently verified visible
boundary. The recorded dispositions identify other gaps. Installed data and
fixtures can establish cue binding and queue order. Audible timing still
requires gameplay confirmation.

## Verification and short gameplay checks

The combined build from `616d697232d172abd5973740e0f25d02fc8ee2c8` passed
`entity_filter_check`, `browse_route_check`, `ff5_fire_ship_route_check`,
`boss_visuals_check`, `dialogue_appearance_check`, and
`modal_speech_check`. It verified 9,146 managed names and all 73 embedded
resources. The asset check matched 121 actor or effect commands and seven
color filter commands to native data, including all 15 new field cues.
Each cue has all 12 game languages. The maximum direct browse fixture
took 0.285 ms. The fake speech SDK reported 0.910 ms cold and 0.131 ms
worst warm submission. These are fixture results. Game-frame impact
and time to audible speech have not been measured in this candidate.

1. In FFIV, visit the Lunar Subterrane through the Crystal Palace. Expect
   the crystal transport, then the cavern reveal. Browse Treasure Chests.
   Current-floor chests should come first. Use Backslash or P on a B1 exit.
   Its browse label and directions should agree on whether a route exists.
2. In FFIV, let Bahamut use Mega Flare and try Rydia's Bahamut summon.
   Listen for the enemy's wings opening and folding, or for the summon
   gathering light and the battle returning. At the sealed sword, advance
   the challenge dialogue and listen before battle starts. During a visible
   NPC page, press I for the speaker's appearance.
3. In the FFIV ending, listen for Kain at Mount Ordeals and for the wedding
   gestures. Let the credits reach the end card to test its separate cue.
4. In FFV's Fire-Powered Ship, request directions past a water current.
   The route should lead around the contact or report the route blocked.

[Exact-payload scan and installed verification](../SECURITY_AND_DISTRIBUTION.md#november-26-narration-and-routing)
