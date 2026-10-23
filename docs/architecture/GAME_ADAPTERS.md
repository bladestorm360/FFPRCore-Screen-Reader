# Game-specific feature adapters

October23: FF6.DefenseWatch/order516 observes native FieldNonPlayer.UpdateEntity
for only the two defense maps and their authored enemy IDs. It owns a separate
warning capture/worker; FF6.StoryGuide remains the requested route producer.
Shared battle/enemy_parts qualifies verified FF5/6 master/message identities.
See [follow-up mechanics and evidence](../re/NAVIGATION_FOLLOWUPS_2026_10_23.md).

Current audit: October22. Shared core supplies common engine/UI/navigation/battle
contracts. Each game folder owns differences in native classes, enums, controls,
vehicle mechanics and rendered panels. All registrations below are game-masked;
PORT_STATUS preserves the detailed port/legacy evidence and source locators.
Implementation is present; offline fixtures and gameplay limits are recorded in
TESTING and the subsystem references, not inferred from another game's results.

| Game | Adapter features and actual source families | Mechanics/extension boundary |
|---|---|---|
| I | new_game.cpp, job_selection.cpp, puzzle.cpp; shared FF1 magic/battle-magic adapters | Four-member creation/job/name flow and puzzle controls use FF1 native fields. Do not apply FF2 skill or FF3 tier mechanics. Creation summary's displayed-data witness is in BASELINE_FIXES |
| II | new_game.cpp, keywords.cpp, magic_menu.cpp, battle_magic.cpp, shop_command.cpp, details_key.cpp, ff2_util.cpp | Keyword learn/ask/item selection, individual spell levels/uses, magic forget/sort/targets and FF2 shop commands. Native results weapon/spell gain rows are shared Battle.Result with FF2-specific rendered fields |
| III | new_game.cpp, job_menu.cpp, magic_menu.cpp, battle_magic.cpp, event_item_select.cpp, line_fade.cpp, movement_speech.cpp | Job transitions, tiered magic/equip/use, event-item choice and native airship controls. Manual airship boarding remains a documented boundary; own typed native signatures, not inherited managed-hook assumptions |
| IV | naming.cpp, ability_menu.cpp, party_setting.cpp, battle_targets.cpp, timers.cpp, vehicles.cpp | Ability lists, party placement, group-target semantics and rendered timers; vehicle position/party lifecycle differ. Shared quantity abilities include IV/V |
| V | job_ability.cpp, naming.cpp, battle_target.cpp, timers.cpp, ff5_mod.cpp | Jobs/ability selection and mastery, group targeting, timers and FF5-specific field commands. Preserve game-specific status, native descriptions and existing variable ability counts |
| VI | ability_menu.cpp, ability_espers.cpp, battle_abilities.cpp, battle_targets.cpp, battle_message.cpp, party_setting.cpp, status_list.cpp, orderly.cpp, keys.cpp, timers.cpp, opera.cpp, airship.cpp, vehicles.cpp, coliseum.cpp | Espers/learned abilities, formations/order, Opera choices, multiple parties, airship heading/altitude and Coliseum item/opponent panels. Third-party BlindGuyNW/ZKline ownership and inclusion permission remain |

module.cpp files connect details/actions to shared dispatch after game-specific
registrations. Central input contexts own routing; adapter actions must not create
keyboard mod mode. UI and battle hooks observe native completion/focus rather than
polling independent timers. Timers read actual displayed game widgets with native
owner/lifetime checks. IV/V/VI group-target and ability panel differences have
actual-source fixtures in battle_focus/target/menu checks; those checks retain
specific holes and page/group transitions, not simply the first party row.

FF6 story_guidance.cpp/story_model.cpp own the state-based Terra defense,
Phoenix Cave, Kefka's Tower, Opera, later Narshe, burning-house and train-engine
objectives. Native corps-slot changes invalidate old-party routes/audio/movement;
each party keeps its own pending objective. A Phoenix party leaving a pressure
plate plans against the exact released geometry, retaining live NPC positions;
otherwise it is told to stay. These are separate from persistent rock toggles.
See [story contracts, native evidence and limits](../re/FF6_STORY_NAVIGATION.md).

Vehicle adapters capture native state on the owner, then shared baked routing uses
owned IDs/groups/positions. Preserve exact boarding/landing/story rules and the
manual-boarding exceptions for III/VI. No fallback game-thread MapRouteSearcher,
grid mutation or arbitrary route cap is allowed. See VEHICLE_ROUTING, THREADING,
AUTOWALK and MAP_EXITS. Puzzle-guide data and map labels are game-specific; shared
formatting does not authorize another game's translations or mechanics.

For native offsets/RVAs, follow the relevant per-feature evidence, matching dump
and external local research. Dump-name presence alone is not proof of ABI or
behavior: III's three-argument condition Play and IV–VI's isCopyEffect illustrate
why typed adapters and exact native callers are required. Each new adapter must
record completed/unsupported cases, state owner, count/work limits, invalidation,
controls/defaults, locale handling and focused offline evidence before player tests.

FF6 parked-party actors are captured from the separate native corps array by
field/party_entities and the final EntityCursor stage. Their hard occupancy,
partial approach boundary and closest-party comparison are documented in
[parked parties](../re/PARKED_PARTIES.md).
