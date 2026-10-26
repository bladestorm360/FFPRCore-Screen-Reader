# Object identities and interaction appearances, October 22

Implementation/source audit complete for the extracted actionable Entity/AnimEntity
set; combined build and delivery are recorded in the fix register. The first pass
did not finish this audit, and its "unsupported" wording was incorrect. Remaining
identities were traced through sprites, map art, direct messages, script actors
and native interaction contracts. Gameplay activation and physical speech remain
separate player checks.
[Status and remaining gameplay checks](../FIXES_2026_10_22.md).

## Gate and evidence rules

The reported screenshot is FF2's green barred doorway. The user's interaction text
identifies an Iron gate. Static matching resolves Map_30081_2 / ev_e_0055, entity29,
tile12,31, raw identifier 鉄格子, script77/sc_e_0055 and message E0055_00_192_a_01.
No interaction or game script was executed. The native raw-name label is Iron gate.

Tiled GID2025 is an editor/prefab marker outside the chip_108/chip_109 tile range;
it is not the gate's rendered sprite identity. GID-based propagation was rejected.
Actual mapobject assets were resolved through mapobject master asset_name into the
installed mo_ffN_* asset bundles, and the textures were visually inspected outside
Git. FF2 asset72 is the green barred gate; a live property with that asset ID gets
Iron gate even when its developer name is missing or ambiguous.

`tools/event_label_identities.py` contains52 reviewed mapobject sprite IDs across
all six games. The inspected contact sheets and exact extracted assets remain at
D:/Games/Dev/Unity/FFPR/tools/issuefix_20261022/ffN_object_sprites.png (plus two zooms).
The follow-up adds extra_sprites.png and numbered context-sheet iterations in that
same external directory. No copied sprites, game source or script bodies enter the repository. The table is
mod-owned classification metadata, not a game asset distribution.

| Game | Reviewed sprite IDs and physical labels |
|---|---|
| I | 103 Submarine;106/107/109/111/113 Orb;117/128 Crystal;119 Rock;138 Warp Tile;144 Tablet |
| II | 69/157 Orb;72 Iron gate;173 Ship;175 Ladder |
| III | 124/138 Crystal;126 Orb;135 Chakram;127 Horn;130 Lute;131 Fang;132 Staff;133/134/136 Sword;217 Grass;218 Rock |
| IV | 137/140/141 Crystal;142/143 Sword;144 Spear;145 Guillotine;310/346 Airship |
| V | 95 Crystal;101 Rock;105/106 Book;107 Map;108 Letter;117 Tablet |
| VI | 94 Letter;96 Crystal;103 Statue;106 Machine;151 Raft;157 Weight;265 Diary |

## Script/identifier audit and consumers

`audit_event_labels.py` enumerates12,513 Events-category candidates, including
conditional groups. The follow-up explicitly checks every type5 Entity/type10
AnimEntity with an action, script or direct message:115/55/232/168/471/389 in I–VI,
1,430 total. All have specific identities, with zero neutral classifications or
live/remote lookup mismatches. Map aliases produce1,445 checked lookups. There
are1,693 physical-identity rows in the broader audit, including inactive copies.
The receipt is [event_identity_coverage.json](event_identity_coverage.json).

Sources include52 inspected actual sprite IDs,483 exact reviewed occurrences in
`tools/event_identity_reviews.json`, qualified physical identifiers, native direct
Talk message keys, inspected script actors and repeated physical subjects. Direct
Talk actions were a gap in the first pass: they can identify a gravestone or door
without any script_id. Script-rest beds, healing pots versus fountains, pictures,
bookshelves, costumes, machinery, vehicle controls, moving platforms and wall
panels now have physical names. A reward, a Msg opcode or an opening-door outcome
alone is insufficient. For example, FF6 Map_20110's apparently empty proxy runs
sc_e_0221, which puppets native NPC65, asset70, a bird; it is Bird, not a bridge
inferred from nearby map art. FF5's shared item-message scripts serve barrels,
pots and chests, so their words cannot establish container type.

The review table contains factual locators, field values, classifications and
evidence references only. It validates raw/native IDs before reuse. Matching
named subjects may cover repeated copies on the same map. Container types stay
exact unless the reviewed FF6 empty-chest subject applies; opened copies also
match the reviewed chest's layer/tile position when the game changes its entity
ID. Phoenix Tower's inspected repeated wall panels remain Wall, rather than
speaking their future stairs/battle outcome. Full source-bearing analysis and
context sheets remain outside Git. Local map-art reconstruction uses each actual
package's textures/aliases, bottom-origin tile sampling and animation entries;
it is offline evidence, not a captured gameplay frame.

`event_labels.cpp::safe_event_label` checks a typed PropertyDefaultEntity AssetId,
then a key of native map ID, entity ID, object type and raw identifier, then the
raw-name table. RemoteEntity building passes the same copied map/entity/type/raw
fields on the owned worker. The object type distinguishes FF4's healing Pot from
its same-ID scripted Event helper. No-action scenery copies do not veto a valid
physical identity. Ambiguous active context keys are never chosen by row order.
FF5's zero-asset tablet proxy was matched against its same native identity's
asset117; its initial Orb classification was corrected before delivery.

`@name:` values hold reviewed proper actor names and use the existing per-game
entity catalog, including the user's selected Auto/manual locale. Five missing
canonical actor entries were added from installed native monster/system-message
IDs in12 languages, with those locators in the review table. Other object nouns
use the common mod catalog. The follow-up adds60 nouns; together with the first
delivery's39 changed/new entries,99 recipe entries have checked12-language
coverage. Native names reuse verified catalog entries. Coverage/structure is
checked; no human fluency review is claimed.

No selection, interaction, native script, route geometry or treasure-content
policy changes. All native reads stay on the owner; workers perform immutable
table/locale lookups on copied values. The observer adds no native polling or
unbounded object search. The existing scene/cache lifecycle owns all pointers.
Current data resolves all1,430 actionable candidates; neutral fallback remains
defensive handling for an unrecognized game version or new data, not a permanent
"unsupported object" category. Inactive decorations and nonphysical scene helpers
remain separately classified and do not establish new interactable destinations.

`event_identity_check.py` checks the actual extracted candidates, all native map
aliases, sprite agreement and the real lookup key shape. The actual-source
native_cues fixture verifies shared identifiers resolving to different objects,
typed pot/event separation, barrel/pot script sharing, localized actors and no
native reads on the formatter worker. These do not prove runtime activation of
every conditional group or physical audible timing. No gameplay launch/probe was
used; the remaining player procedure is in TESTING.

## Interaction ! producer, ownership and bounds

`field/interaction_cue.cpp` hooks FieldInteractiveIcon.PlayNear/PlayFront after the
native engine decides range/selection. The icon's type, rendererEnable flag and
actual enabled/on-screen/nontransparent sprite renderer must be ready. Owner must
still match at publication. The cue is Interaction available: [object]. NPC names
and chest type-only labels use existing native readers; other objects use the
same safe_event_label as navigation. Full dialogue still belongs to native interaction.

A fixed64-slot registry identifies an occurrence by icon+owner until native Hide or
draw-disable; Near-to-Front within that occurrence does not repeat. Leaving and
reapproaching is a new event, not text deduplication. Hide/draw-disable cancel its
worker. MapChanging/BattleEntering retire epochs; safe MapChanged/BattleStart clear
state. Additive scene loads preserve still-ready field context. Only copied label
and locale go to the formatter; native pointers remain in owner callbacks. Full
capacity leaves an observation unconsumed for a later native update to retry.
No new proximity loop or input automation is added; speech follows the global toggle.

native_cues_check exercises unchanged markers, cancelled parked work, reappearance,
hidden renderer, actual Iron gate data and sprite fallback. oct22_catalog_check
verifies99 changed/new entries in12 locales and all generated labels' locale sets.
These do not prove physical screen-reader timing or every map instance. The
prioritized nonvisual procedure is in TESTING. Keep the observer tied to native
appearance/Hide ownership, not repeated wording or an independently guessed range.
# October24 passage-label refinement

[Concealing-object evidence](HIDDEN_PASSAGES_20261024.md) now identifies Paul's
wall near his bed, Castle Fynn's wall and Narshe's rock at exact native map/entity
identities. Shared raw names stay context-qualified. Existing Clock/Fireplace and
treasure/category rules remain unchanged; final combined verification is pending.
