# Connected NPCs, treasure state, map ownership, landing and shops

Status: implementation complete, offline verified and installed/hash-verified in
all six games after the user closed FFIII. Gameplay listening is pending.
No game launch, input or new
runtime probe is authorized or performed. Earlier startup diagnostics remain a
separate investigation; a successful launch does not establish its root cause.

## Requirements and observed evidence

- Current Map must retain genuinely active NPCs across connected floors in all
  six games. Castle Sasune's post-curse initializer contains a visual mist call
  which currently invalidates the entire remote state projection. Unknown story
  gates must remain conservative; do not assume every conditional group exists.
- Investigate Sara's Room disappearance/route delay and Sara's own-room route.
  Existing FFIII log reports live target (1884,-1865) on map27, whose bake is
  18 by21. A baked home position is not evidence that a staged live actor is
  currently there. Preserve actual identity, visibility and route validity.
- Can land applies only to the currently mounted transportation, with explicit
  landing/dismounting. Foot/boardable previews and automatic Content vehicles
  such as canoes must be silent in every game. Preserve loading/fade silence,
  guide edges and FFVI's shared priority timestamp.
- The FFIII armor-shop screenshot visibly has green upward arrows beside the
  Warrior and Red Mage. Establish arrow/animation meanings from the matching
  native producers before describing equip eligibility. Audit the shared
  ShopPartyReader and its controls/details across all six games.

## Sources and ownership

Shared production: field/map_state_program.cpp, field/entity_scanner.cpp,
field/baked_nav.cpp, fieldnav/vehicle_landing.cpp, shop/shop_party.cpp and
shop/shop_tracker.cpp. Initializers: tools/bake_map_state.py and the all-six
mapstate resources. Existing authoritative records: THREADING.md,
LANDING_LOAD_20261025.md and the feature index.

Native extraction is read-only and outside Git at
../tools/npc_floors_20261025/; original extractor
tools/re/map_location_native.py records DLL/dump hashes and exact source locators.
Current.HideMist calls map accessor HideScenery (id1, scenery3), and
CheckWithCompanion conditionally writes map-local0. Renderer/lifetime evidence
and safe projection rules are resolved below. No proprietary source or
disassembly is copied into this repository.

## Verification and remaining work

Canonical build/smoke, final payload scan and installed
hash/protected-file receipts pass. Gameplay/listening and real frame/audible latency
remain separate from offline checks. No arbitrary route-distance limit is added.

## Resolved state, floor and companion contracts

October25 [later companion correction](NPC_PATROL_TARGETS_20261025.md) supersedes
the AssetId lookup described in this historical deployment: generated followers
leave PropertyNpc.AssetId0, while the actual native sprite source is the owned
SaveCompanionEntityData base identity. The earlier fixture did not reproduce that
synthetic property. Collision fixes remain valid and user-confirmed.

The native fog path is visual only: Current.HideMist (III0x91CCA0) calls accessor
HideScenery (id1, scenery3), FieldController.HideScenery (0x281430), both
MainViewMapRenderer.HideScenery methods and MapMultipleScroll.Hide. Only that
exact FFIII SysCall becomes a navigation no-op. CheckWithCompanion (0x91B700)
conditionally writes map-local0; its output is unknown, not a guessed follower or
an unconditional reset. All-six SysCall surveys retain unrelated unknown commands.

map_state_program.cpp intersects completed branches: guaranteed additive groups
and exactly agreed actor effects survive, while divergent actor IDs are uncertain
and excluded as destinations. Whole-selection known remains false when branches
disagree. Unsupported/malformed flow or exhausted8192-instruction/64-state/16-call
bounds fail closed. Default actors, agreed Hide and conservative conditional
obstacles retain their rules. Map/area locals stay with their native owner.

connected_objects_snapshot now retains internal GotoMap sources even when their
destination is already a connected floor; presentation filters/grouping still
own duplicate exits. Existing400-visited-map work bound and story gates remain.
Map27/sc_e0017 is Sara's ring/spring ritual, returning to throne26; its name is
Underground Spring. Bedroom35 has the actual Sara variant. Known-grid bounds
reject live actors staged at30000,30000 instead of routing to fictitious homes.

party_entities.cpp recognizes the actual FieldCompanionEntity class in all six
games. CanEnter permits player position exchange (III0x8521C0); obstacle capture
marks it known non-solid while retaining talk identity. III PropertyNpc.AssetId
and Current.CompanionId name Sara33, Cid34, Desch35, Aria36, Alus37, Doga38,
Unei39, Gutsco41 through existing native translations. Other games keep established
name fallbacks; VI's separately parked party actors keep their collision contract.

Actual-worker tests cover Kazus13 to inn2F15, reverse, Mines159 to upstairs bed,
and Woman14/id9 at logged11,7 from18,11. The same stair follower is traversable as
a companion; marking it solid reproduces the blocking/wait prefix. Sasune23..35
all retain remote Sara35/id5 and source entrances where applicable. No new edge,
native search or distance cap is invented.

## Interaction layers, loose treasure and saved chest state

Caanan map36/id39 is Ground33,31. The old goal builder also accepted Bottom33,30.
Cardinal and diagonal interaction goals now respect the real layer unless an NPC
has native AllLayers; synthetic spots retain prior semantics. Five logged starts
reach Ground32,31 facing East. Existing side/diagonal fallback remains; this does
not certify every arbitrary native interaction. Native get_AllLayerTarget is
false for the default map object and reads the NPC flag; older I/II/IV layouts
carry the equivalent protected field rather than that getter.

bake_maps.py stores native PickUp in unused flag bit32, independently of the
content_id above bit8; all six maps.bin files are regenerated. Live discovery
reads PropertyTresureBox.PickUp; remote rows copy the baked bit. Disappearing
pickups say Treasure with no Opened/Unopened and leave discovery when collected.
Ordinary chests retain state prefixes and remain listed opened. Contents-spoiler
preference, underlying treasure category and target identities remain unchanged.
Treasure, Can equip and Underground Spring have12 locale entries.

The later Tozus log109500.734/109822.703 says Treasure Chest remotely, while
109815.750/109818.031 says Opened locally. This predates installation of the new
pickup labels. Its cause is the older mapstate generator excluding flag_id0.
Native GetFlag accepts zero as the first saved bit and PropertyTresureBox copies
it unchanged. The generator now requires an explicit integer in0..1023, including0;
missing/invalid properties remain unknown. It restores eight mappings: I1, II0,
III1 (Map_20081_2/id6), IV1, V4, VI1. All1894 extracted treasure objects now have
mappings (249/235/415/396/279/320). Missing copied bits still stay unknown.
All-six native export hashes/locators are outside Git at
../tools/treasure_flags_20261025/ffN, from map_location_native.py --treasure-flags.
mapstate_check tests zero/set/missing flags for every chest, including all eight.

## Routine borders, landing and shop visual equivalents

The1535/1536 events contain three reviewed508-tile world-border families, for
Enterprise/Nautilus/Invincible. Their immutable event identity is World map
transition. event_route_only plus the live world detector omits these before
filtering/grouping: no routine border rows/pings, independent of Alt+Backslash.
Baked transitions, graph arcs and vehicle predicates remain for routing; meaningful
scenes stay. Actual-source discovery checks all1524 sources against the real
catalog/bake. Removing their per-row work addresses unnecessary browsing load;
no measured game-frame/audible improvement is claimed.

vehicle_landing.cpp now requires the actual valid mounted transportation id>0
and manual types2,3,6,7,8,9,10,11,12. Foot, None, Symbol, automatic Content/canoes
and unknown types stay silent. Model changes reset edges. Visible-guide
Show/Switch edges, loading/fade silent seeding, Hide/map/battle resets and FFVI
priority timestamp are preserved. Nearby boardable vehicles cannot grant a cue.

All-six ShopCharaStatusContentController.Comparison checks compatibility before
SetCanEquipAnime. CharacterImageController stores win for compatible equipment,
Default_00 otherwise; arrows independently describe stat comparison. The shop
celebration pose has native shop meaning, independent of town celebration;
different portraits may occupy different animation frames in a screenshot.
Keys1..9 now read the visible portrait's assigned name if text is absent, then
Can equip/Cannot equip separately from Stronger/Weaker/No change and Equipped.
Only visible equipment portraits (types2/3) permit bounded animation lookup.
Consumable/magic idle poses do not imply incompatibility; hidden/closed panels
expose no stale assigned character.

## Gnomish Bread and Magic Learn context ownership

Log107370..107387 already copies a seven-entry buffer, Chests8/8 and Items6/6.
Event12 was blocking map shortcuts. game_state.cpp now permits a positively active
MapPanels owner as event UI, while retaining native state-depth/map-transition,
fade and loading guards. Field/MainMenu navigation remains blocked during events.
Immediate and deferred generic cursor reads yield to that owner without text
deduplication. Native WASD still selects/scrolls; arrows review the mod buffer,
I requests detail, Autodetail follows its preference and Shift+I reads controls.
Items/Chests may appear together or individually on the detailed location screen;
basic maps never inherit those counters and retain relative marker review.

Log108812.828/.843 speaks old Cure/Blind previews before Learn. FFIII magic_menu.cpp
now retires spell/target ownership before native list/command/hide initializers
rebuild previews, and accepts only lists owned by the cached current window.
Learn3 uses tomes; Use1, Remove4, Exchange5, Forget6 use spell slots. Target2,
Command7, popups8..11, unknown/closed states cannot fall back to stale lists.
Existing native UpdateController first-focus observation (maximum10 updates) and
command-generation cancellation remain. Commands require positive MainMenu state;
legitimate character selection remains audible. II already suppresses init and
checks its different phases; I has no equivalent Learn and IV/V/VI use different
ability/job/Esper flows. No speculative blanket muting or new timer is added.

## Verification coverage, lifetime and performance limits

Canonical tests exercise actual production source with fake native boundaries:
mapstate/native storage, npc_floors, npc_geometry, shop_party, party_entities,
vehicle_landing, entity_scanner, battle_context, map_panels, cursor_handoff,
battle_focus and equipment_party. The geometry survey covers13 all-six flag
profiles, including saved Kazus, and reports staged/gated/unknown/no-entry cases
without guessing them open. It omits transient NPC positions; it is not exhaustive
story-state certification. Focused floor projection measured about0.24..1.1ms
including test-thread join, excluding native capture, queue/publication and speech.

No new game-thread waits, worker-held native pointers, script calls, scene search
or game input are introduced. Owner observations copy bounded identities/flags;
owned workers project/route/format with existing epoch/context cancellation.
Existing large navigation files retain cohesive documented exceptions, without
unrelated mass splitting. Test-only stack/fake-method failures were corrected;
no intermediate failed build was installed. Remaining listening priorities are
in docs/TESTING.md. Recovery/check/scan/install evidence is under
build_check/npc_floors_shop_landing_20261025/. No delegation, dependency upgrade,
signing identity, release/publication or commit change is part of this task.

## Final verification and installation

Canonical build and all offline checks passed after the exact Tozus cross-floor
row regression was added: saved0/1/unavailable bit each retains the corresponding
remote state and ordinary-chest identity. All1894 mappings pass; all-six native
storage tests also pass. Actual world discovery removes1524 border sources and
retains14 meaningful sources. Name audit7945/0 missing, wiring133 used/341 defined/
0 undefined; isolated smoke forwards47/47 exports. Embedded49 data and18 sound
resources match their current source hashes. DLL10932224 bytes SHA256
a9f44da03d374418d02e031f17256ee0f34190915c0d33d4c8516a6e42aaefee.

security_final_predeploy stages the exact22 shipped files: current enabled
Defender reports no threats, zero matching detections and unchanged hashes.
Exclusion enumeration remains unverified and no signing certificate is available;
this does not certify download reputation or a future archive. No protections
were changed and no samples uploaded. The initial evidence verifier's nonexistent
header path was corrected before the successful scan; no payload change resulted.

deploy_partial.json preserves I/II/IV/V/VI installation and III's process-guard
refusal. After the user closed III, deploy_preclosed.json captured a fresh
protected baseline, deploy.bat ff3 succeeded and deploy_after.json verifies all
six DLLs/144 protected files. installed_payloads_verified.json compares all132
installed hashes to the exact scanned manifest. Recovery, latest own log, focused
excerpts, native provenance index, source hashes and final build/check/smoke
reports are retained in the archive. Gameplay listening remains pending; no game
was launched, controlled or probed, and no dependency/release was changed.
