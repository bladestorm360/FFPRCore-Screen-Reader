# Ur locations and concise character rows, October25

The subsequent [I-key and Ur clerk correction](INN_COUNTER_MP_20261025.md)
records a user-reported failure in manual MP details and the missing reachable
clerk. It supersedes the earlier cursor-anchor assumption below.

Status: complete/offline verified and installed/hash-verified in all six games.
FFIII deployment succeeded after the user's deploy reply; the earlier running-game
deferral is history. Gameplay/listening remains pending. No game was launched,
controlled or probed; no new delegation or dependency change.

## Ur well, shed and direct speaker identity

The user's two outdoor pickups and the woman's stash are distinct native
TreasureBox occurrences. The well is a genuine GotoMap entrance, not a missing
interactible. Its room was untitled, so navigation called it Ur. The shed was
incorrectly inferred as a generic elder's House; its connected back room also
fell back to Ur. The native NPC's dialogue identifies a shed and another NPC
identifies the well. Offline reconstruction of the installed map textures
confirms the round stone well, building in the thicket, storehouse interior and
underground room. These are asset reconstructions, not gameplay screenshots.

| Native map | Submap | Authored suffix | Native connection |
|---|---|---|---|
| 336 | Map_20011_8 | Shed | Ur map5, GotoMap entity87 |
| 337 | Map_20011_9 | Shed Back Room | map336, GotoMap entity6 |
| 338 | Map_20011_10 | Well | Ur map5, GotoMap entity89 |

data/ff3/map_titles.json remains the authoritative authored-title resource.
The existing resolver adds the localized area name. Mod-owned Shed has12
localized values and a durable merge recipe in modtext_object_followup.py;
Well and Back Room reuse existing catalog entries. Native map/exit identities,
category, routes, collection flags and treasure names stay intact. No duplicate
synthetic object is added. Unopened treasure labels remain type-only unless the
player explicitly enables the existing content-spoiler setting.

The two outdoor pickups use independent flags366/367; the well's three chests
use377/378/379. The five back-room chests use372..376. Cure's location was checked
against the NPC's specific clue, but no reward is exported into a map/chest label.
The shed's existing candle interaction is already an Object; the elder explicitly
identifies the rightmost candle as a switch. The user subsequently found the
back room in the live log. Other locations' gameplay activation is unconfirmed.

The all-six existing physical Well/Fountain identity audit finds no missing
class in FF1/FF2: FF1's three wells and three fountains and FF2's fountain already
have physical names. Automatic scenes near springs remain Events. Do not infer
new destinations from every occurrence of the word well in dialogue.

Dahn is map11/Map_20011_6, entity11, default NPC, raw name NPC, direct message
N008_C03_011_01_01. His own story_cha entry names him in all12 native languages.
The raw generic name caused the live detector to fall back to the GameObject's
FieldNpc. The scoped translation alias @map/11/NPC now resolves his native name
before that fallback in live and copied remote readers. This map has exactly
one such NPC; the alias does not rename NPCs on other maps. Raw route identity,
numbering and story gates are unchanged. npc_speaker_labels.py audits all-six
bare NPC/FieldNpc/empty direct-message placements; Dahn is the only occurrence
with a populated English speaker entry. It validates the exact native occurrence,
map alias uniqueness and all12 installed speaker tables before applying/checking.
Additional ambiguous maps need entity-specific naming, never a global NPC alias.

## Short rows, MP details and native cursor ownership

The FFIII screenshots show eight current MP counts, separated across two rows.
These are not current/maximum pairs. The same native currentLvMpTextList exists
in FF1's shared character/target layouts. displayed_ui.cpp now separates
charge_lines from vital_lines/character/target. Short reads omit this entire
list. When a visible charge layout exists, a leftover scalar MP field is also
excluded; it is not an additional ninth MP pool. Scalar MP in the other games
retains its actual displayed current/maximum/gauge rules.

Main-menu character selection reads name and vitals/status, then job and displayed
next-level information, followed by displayed character level, job level and the
Formation-only row. Empty/hidden metadata is absent. Item/spell/learning targets
read name, HP and displayed status effects. Existing target command columns in
battle remain subject to their own screen rules; H still excludes that column.

ui/character_row owns I/Autodetail for the native focused main-menu row. Specific
StatusWindow, ItemUse and FF1/FF3 AbilityUse focus producers call its announce
function. Autodetail off speaks the short row once; on queues the eight-level tail
after that row through say_sequence. I reads a fresh short layout and then fresh
visible charge cells. This provider precedes the item/spell description which
opened its target panel. It does not replace a focused item/equipment description.

Only the native UI thread stores/reads the content, cursor and optional overview
window. Detail access requires positive main-menu state5, live active owners,
Cursor.IsFocus, a visible cursor Image and the actual cursor parent matching a
serialized anchor on that row. A visible gray inactive cursor is insufficient;
Cursor.SetActiveImage can hide only its image while leaving the script alive.
Reparenting, loss of native focus, hidden cursor/image, destroyed owner, menu exit,
MapChanging or BattleEntering retires
the old detail. There is one retained focus; no scene lookup, polling, delay,
input interception, text deduplication or worker-owned native pointer is added.
At most eight charge entries are accepted; a malformed larger list is rejected.
Only copied strings reach the existing speech worker and interruption generation.

The full Status review buffer appends these MP lines at its end, after every
other displayed stat/ability, with each level still browsable. Battle party/ally
target short reads use the same charge-free formatter; enabled Autodetail appends
charges last. I on a battle ally target/command reads the matching focused/active
row's charges; Magic/Items descriptions retain priority inside their own lists.
Use-result feedback stays concise and reports actual HP/condition changes.

The native ItemWindow has both TargetSelect5 and InterChangeSelect6, which call
the same ItemUseController.Show in all six games. The old suppression predicate
omitted6, clearing ItemMenu ownership while learning/exchanging a spell. The
generic cursor then spoke a name one frame after the specific target reader.
The trace at88896.296/.328 confirms this second submission. State6 now retains
the specific reader. The delayed generic callback also rechecks item ownership
before reading, covering a native handoff after it was scheduled. Speech text is
never compared; legitimate repeated row visits remain new events.

## Evidence, checks and delivery

Source-bearing map extracts and native disassembly remain outside Git at
../tools/mapdump, ../tools/ur_well_20261025 and
../tools/character_rows_20261025/ff1..6. Exact native producer RVAs/hashes are in
ignored build_check/ur_well_20261025/all_six_character_rows_native.json. The original
extractor is tools/re/map_location_native.py --character-rows. Check scripts and
JSON receipts contain factual identifiers/hashes, not game implementation bodies.

ur_locations_check validates the three real native exits and distinct treasure
flags, plus the all-six well/fountain identities. map_names_check exercises the
three actual resources through current map, map-key and remote/exit consumers in
all12 locales. npc_label_audit_check exercises Dahn's live/copy locale lookup and
negative map scope. party_esper_check uses the actual renderer, row service and
battle-party reader: short overview/target, eight current-only cells, fresh I,
hidden cells, Autodetail ordering, cursor changes/visibility, gray unfocused
cursors, hidden cursor images, menu exit, reset and
oversized-list rejection. Its fake native APIs assert owner-thread access.
cursor_handoff_check executes the actual deferred generic consumer, including
claim-after-schedule, release to fallback, equal-text revisits and destroyed
cursor. Other party/focus fixtures retain explicit service-boundary fakes. Existing
all-six routing/status/condition/speech checks remain required.

These bounded fixtures and native call paths do not measure Unity callback/frame,
input-to-audible latency, actual screen-reader playback or gameplay activation.
The added work is bounded to one focused row/eight cells, without new backlog;
runtime performance remains a player-side confirmation.

Final canonical build passes all offline checks:7791 names/zero missing,
133 used/341 defined/zero undefined wiring. The isolated smoke host resolves and
forwards all47 exports and exits normally. All49 embedded data and18 sound
resources match their current source; data differences from the prior combined
build are exactly the three Ur titles, Shed noun and Dahn alias.

Final DLL10913792 bytes, SHA256
93a86252635f32ded6c2c6cf62f29d791f5cf6efa8baef8d24b01fd3100415f9.
Enabled/current Defender signature1.461.196.0 reports no threats for the exact22
staged payload files; hashes stayed stable. Existing unsigned-DLL/no-signing-cert
and protected exclusion-query limits remain; no scanner settings were changed.
All22 installed files match in all six games, with144 protected files unchanged
against the fresh post-close deploy_preclosed.json baseline. Earlier live-session
and partial deployment receipts are preserved rather than attributed to deployment.

Evidence/recovery: build_check/ur_well_20261025/. verification.json and
embedded_resources.json retain source/build/smoke checks; security_final_predeploy2
is the final scan; deploy_after.json and installed_payloads_verified.json verify
all six installations. deploy_partial.json preserves the earlier five-game
installation, and deploy_finalbefore.json is its baseline. The earlier f56f4067 build/
scan/deployment receipts are retained with first suffixes and
security_final_predeploy, before the gray-cursor focus hardening. The initial
fixture linker failure is preserved under failures; the focus fixture's explicit
service-boundary fakes were corrected, and the actual service tests pass.

The apparent silent magic-shop selection was clarified by the user: the screenshot
contains only Poisona, so navigation does not change focus. No shop reader change
or invented repeat announcement was made for that report.

## Mythril Mines B2 gate review

Complete/offline verified; no production change is required. The user's log
90270.140/90272.015 lists the B2 exit as Story gated on map159/Map_30291_1. The
closed wall is native entity10, ev_e_0016: solid, action0, script0, empty message,
no interaction icon. There is no ignored current wall-dialogue producer.

sc_map_30291_1 keeps that wall while ScenarioFlag1 bit180 is clear. The separate
entity8/ev_e_0016_1 switch, action2/script244 sc_e_0274, exists only after bit16
is set and before bit180 is set. It is already correctly labelled Switch in the
Objects category. Its sound/shake/animation script removes the wall/switch and
sets180; it has no Msg operation. The model does not synthesize this later object
in the earlier story state. The closed wall is non-actionable scenery, so it
correctly contributes route obstruction without appearing as an Object/Event.

An NPC clue does exist: map14/Map_20021_1, ev_e_0017 entity11, message
N106_C03_014_02_01. It describes a concealed passage and checking a particular
part of the wall. This NPC group also requires bit16; the earlier group is
ev_e_0011. The log's curse dialogue is consistent with the earlier state. The
later hint is not a current interaction missing from navigation, and it must not
be spoken early as a fabricated gate warning. No save/game flag was read or changed.

The actual mapstate_check production resolver and baked router now test three
copied profiles: wall present/switch absent, wall plus reachable switch, and opened
wall with switch absent. B2 is unreachable in the first two and reachable in the
third. Removing only gate blockers establishes a valid geometric B2 route even
in the closed states. Thus Story gated is supported by actual geometry/state,
not inferred merely because a named future destination exists. The check runs
in the canonical all-six offline suite. This is not runtime activation proof.

Factual locators, conditions and source/script hashes are retained in
build_check/ur_well_20261025/mythril_mines_review.json. Proprietary scripts remain
in the external mapdump archive. The tested/shipped DLL stays93a86252; this review
adds only the specific offline regression/documentation.

After deployment, prioritized checks: in FF1/FF3 main-menu selection and item/spell
targets, Autodetail off gives a short row and I gives MP levels last. Turn it on
and verify the same order. Spell learning should speak one row per actual focus
change, with no later name-only interruption. Dahn should be named in navigation.
Ur's Map Exits should distinguish Well, Shed and Shed Back Room. Treasure contents
remain undisclosed by default. Closing a child screen must restore its own I action.
