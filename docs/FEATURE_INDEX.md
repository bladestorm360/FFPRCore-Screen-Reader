# Current feature and source index

Audited against native source registrations on October23. This is the maintained
feature index; FEATURES.md remains the historical five-game porting checklist.
Every registration below is implemented. Offline evidence varies by subsystem;
gameplay confirmation is separately recorded in [TESTING](TESTING.md). October22
fixes have their own [implementation/evidence register](FIXES_2026_10_22.md).
October23 adds [FF6 story/party/puzzle navigation](re/FF6_STORY_NAVIGATION.md),
with independent offline and gameplay status.
The [October23 follow-ups](re/NAVIGATION_FOLLOWUPS_2026_10_23.md) cover defense
alerts/brief guides, the story-gate filter, save points, native name entry and boss parts.
The [parked-party correction](re/PARKED_PARTIES.md) adds FF6's separate native corps
actors to physical route snapshots and retains directions up to an adjacent
blocker, with a brief terminal party suffix.

| Feature family | Detailed current contract |
|---|---|
| Boot, engine, workers, speech, settings, localization, diagnostics | [Runtime architecture](architecture/RUNTIME.md), [Prism](PRISM_MIGRATION.md), [speech](re/SPEECH_ORDER.md), [logging](re/LOGGING.md) |
| Input, controller mode, remapping and mod settings | [Bindings](INPUT_BINDINGS.md), [baseline changes](re/BASELINE_FIXES_2026_10_16.md), [runtime](architecture/RUNTIME.md), [user guide](../readme.md) |
| Dialogue, popups, title, save/load, menus, shops, status/config, Extras | [Screen-owned readers](architecture/MENUS_AND_DIALOGUE.md), [displayed details](re/MENU_DETAILS.md), [bestiary](re/BESTIARY.md), [title](re/TITLE_PROMPT.md) |
| Battle, formation, targets, messages, conditions and results | [Battle entry](re/BATTLE_ENTRY.md), [results](re/RESULTS.md), [enemy conditions](re/ENEMY_STATUS.md), [menu details](re/MENU_DETAILS.md), [speech](re/SPEECH_ORDER.md) |
| Entities, event/treasure state, labels, pathfinding and exits | [Threading/routing](re/THREADING.md), [map state](re/MAP_STATE.md), [event context](re/EVENT_CONTEXT.md), [tracking](re/EVENT_TRACKING.md), [labels](re/OBJECT_IDENTITIES.md), [exit grouping](re/NAVIGATION_GROUPS.md), [map exits](re/MAP_EXITS.md), [traps](re/TRAPS.md) |
| Waypoints, beacons, autowalk, teleport, puzzles and vehicles | [Autowalk](re/AUTOWALK.md), [routing](re/THREADING.md), [vehicle corrections](re/VEHICLE_ROUTING.md), [puzzle/event context](re/EVENT_CONTEXT.md), [controls](../readme.md) |
| Audio, movement, soundscape, previews and HRTF | [Soundscape](re/SOUNDSCAPE.md), [runtime ownership](architecture/RUNTIME.md), [controls](../readme.md) |
| Every game-specific feature | [Game adapters](architecture/GAME_ADAPTERS.md), [port/native evidence](PORT_STATUS.md) |

The table below maps every active core::Install registration to its actual owning
source/function/order/mask. Helpers without registrations belong to their source
folder's family above (e.g. route_progress, displayed readers, cue/HRTF renderers,
preferences and NavigationBuffer). Source comments retain local mechanics/native
names; detailed references carry decisions, safe extension and verification limits.
This index is checked against source when adding/removing a registration.

| Registration | Source / install function | Order | Game mask |
|---|---|---|---|
| MovementSoundPatches | [src/core/audio/movement_sounds.cpp](../src/core/audio/movement_sounds.cpp) / install | 100 | all six |
| SoundPlayer | [src/core/audio/sound_player.cpp](../src/core/audio/sound_player.cpp) / install | 20 | all six |
| Workers | [src/core/background.cpp](../src/core/background.cpp) / install | 9 | all six |
| Battle.Command | [src/core/battle/battle_command.cpp](../src/core/battle/battle_command.cpp) / install | 110 | all six |
| Battle.Formation | [src/core/battle/battle_formation.cpp](../src/core/battle/battle_formation.cpp) / install | 110 | all six |
| Battle.Item | [src/core/battle/battle_item.cpp](../src/core/battle/battle_item.cpp) / install | 110 | all six |
| Battle.Magic | [src/core/battle/battle_magic.cpp](../src/core/battle/battle_magic.cpp) / install | 110 | game::kFF1 |
| Battle.Message | [src/core/battle/battle_message.cpp](../src/core/battle/battle_message.cpp) / install | 110 | all six |
| Battle.Party | [src/core/battle/battle_party.cpp](../src/core/battle/battle_party.cpp) / install | 110 | all six |
| Battle.Pause | [src/core/battle/battle_pause.cpp](../src/core/battle/battle_pause.cpp) / install | 110 | all six |
| Battle.QuantityAbilities | [src/core/battle/battle_quantity.cpp](../src/core/battle/battle_quantity.cpp) / install | 520 | game::kFF4 | game::kFF5 |
| Battle.Result | [src/core/battle/battle_result.cpp](../src/core/battle/battle_result.cpp) / install | 110 | all six |
| Battle.Start | [src/core/battle/battle_start.cpp](../src/core/battle/battle_start.cpp) / install | 110 | all six |
| Battle.State | [src/core/battle/battle_state.cpp](../src/core/battle/battle_state.cpp) / install | 110 | all six |
| Battle.Util | [src/core/battle/battle_util.cpp](../src/core/battle/battle_util.cpp) / install | 100 | all six |
| Battle.EnemyStatus | [src/core/battle/enemy_status.cpp](../src/core/battle/enemy_status.cpp) / install | 117 | all six |
| BestiaryNavigation | [src/core/bestiary/bestiary_navigation.cpp](../src/core/bestiary/bestiary_navigation.cpp) / install | 131 | all six |
| Bestiary | [src/core/bestiary/bestiary_patches.cpp](../src/core/bestiary/bestiary_patches.cpp) / install | 130 | all six |
| MessageChoice | [src/core/dialogue/message_choice.cpp](../src/core/dialogue/message_choice.cpp) / install | 101 | all six |
| MessageWindow | [src/core/dialogue/message_window.cpp](../src/core/dialogue/message_window.cpp) / install | 100 | all six |
| Popup | [src/core/dialogue/popup.cpp](../src/core/dialogue/popup.cpp) / install | 100 | all six |
| ScrollMessage | [src/core/dialogue/scroll_message.cpp](../src/core/dialogue/scroll_message.cpp) / install | 100 | all six |
| TitleScreen | [src/core/dialogue/title.cpp](../src/core/dialogue/title.cpp) / install | 100 | all six |
| Displayed.Comparisons | [src/core/displayed_comparisons.cpp](../src/core/displayed_comparisons.cpp) / install | 105 | all six |
| Displayed.ConditionIcons | [src/core/displayed_conditions.cpp](../src/core/displayed_conditions.cpp) / install | 105 | all six |
| Displayed.SkillProgress | [src/core/displayed_skill_progress.cpp](../src/core/displayed_skill_progress.cpp) / install | 105 | game::kFF2 |
| GalleryPatches | [src/core/extras/gallery.cpp](../src/core/extras/gallery.cpp) / install | 100 | all six |
| MusicPlayerPatches | [src/core/extras/music_player.cpp](../src/core/extras/music_player.cpp) / install | 100 | all six |
| Field.ConditionFlash | [src/core/field/condition_feedback.cpp](../src/core/field/condition_feedback.cpp) / install | 115 | all six |
| Field.EntityNavigation | [src/core/field/entity_nav.cpp](../src/core/field/entity_nav.cpp) / install | 100 | all six |
| Field.InteractionCue | [src/core/field/interaction_cue.cpp](../src/core/field/interaction_cue.cpp) / install | 116 | all six |
| Field.LiveContext | [src/core/field/live_context.cpp](../src/core/field/live_context.cpp) / install | 105 | all six |
| Field.MapNameResolver | [src/core/field/map_names.cpp](../src/core/field/map_names.cpp) / install | 100 | all six |
| Field.MapState | [src/core/field/map_state.cpp](../src/core/field/map_state.cpp) / install | 110 | all six |
| PuzzleGuide | [src/core/field/puzzle_guide.cpp](../src/core/field/puzzle_guide.cpp) / install | 130 | all six |
| AudioLoopManager | [src/core/fieldnav/audio_loops.cpp](../src/core/fieldnav/audio_loops.cpp) / install | 110 | all six |
| Autowalk | [src/core/fieldnav/autowalk.cpp](../src/core/fieldnav/autowalk.cpp) / install | 120 | all six |
| MapTransition | [src/core/fieldnav/map_transition.cpp](../src/core/fieldnav/map_transition.cpp) / install | 110 | all six |
| MoveStateHelper | [src/core/fieldnav/move_state.cpp](../src/core/fieldnav/move_state.cpp) / install | 90 | all six |
| NavigationTargetTracker | [src/core/fieldnav/nav_target.cpp](../src/core/fieldnav/nav_target.cpp) / install | 90 | all six |
| Teleport | [src/core/fieldnav/teleport.cpp](../src/core/fieldnav/teleport.cpp) / install | 110 | all six |
| VehicleLanding | [src/core/fieldnav/vehicle_landing.cpp](../src/core/fieldnav/vehicle_landing.cpp) / install | 120 | all six |
| WaypointController | [src/core/fieldnav/waypoint_controller.cpp](../src/core/fieldnav/waypoint_controller.cpp) / install | 110 | all six |
| SceneGlue | [src/core/glue/scene_glue.cpp](../src/core/glue/scene_glue.cpp) / install | 90 | all six |
| ControllerLabels | [src/core/input/controller_labels.cpp](../src/core/input/controller_labels.cpp) / install | 60 | all six |
| ControllerRouter | [src/core/input/controller_router.cpp](../src/core/input/controller_router.cpp) / install | 60 | all six |
| GameToggleAnnouncer | [src/core/input/game_toggle_announcer.cpp](../src/core/input/game_toggle_announcer.cpp) / install | 100 | all six |
| GlobalHotkeyHandler | [src/core/input/global_hotkeys.cpp](../src/core/input/global_hotkeys.cpp) / install | 60 | all six |
| InputManager | [src/core/input/input_manager.cpp](../src/core/input/input_manager.cpp) / install | 60 | all six |
| InputPassthrough | [src/core/input/input_passthrough.cpp](../src/core/input/input_passthrough.cpp) / install | 60 | all six |
| ModActions | [src/core/input/mod_actions.cpp](../src/core/input/mod_actions.cpp) / install | 100 | all six |
| Input.PhysicalWorker | [src/core/input/physical_gamepad.cpp](../src/core/input/physical_gamepad.cpp) / install | 15 | all six |
| Menus.CharacterSelection | [src/core/menus/character_selection.cpp](../src/core/menus/character_selection.cpp) / install | 120 | all six |
| Menus.EquipMenu | [src/core/menus/equip_menu.cpp](../src/core/menus/equip_menu.cpp) / install | 120 | all six |
| Menus.ItemDetails | [src/core/menus/item_details.cpp](../src/core/menus/item_details.cpp) / install | 120 | all six |
| Menus.ItemEquipment | [src/core/menus/item_equipment.cpp](../src/core/menus/item_equipment.cpp) / install | 120 | all six |
| Menus.ItemMenu | [src/core/menus/item_menu.cpp](../src/core/menus/item_menu.cpp) / install | 120 | all six |
| Menus.MagicMenuFF1 | [src/core/menus/magic_menu_ff1.cpp](../src/core/menus/magic_menu_ff1.cpp) / install | 120 | game::kFF1 |
| Menus.MagicMenuState | [src/core/menus/magic_menu_state.cpp](../src/core/menus/magic_menu_state.cpp) / install | 110 | all six |
| Menus.UseFeedback | [src/core/menus/use_feedback.cpp](../src/core/menus/use_feedback.cpp) / install | 125 | all six |
| ShopMagicPatches | [src/core/shop/shop_magic.cpp](../src/core/shop/shop_magic.cpp) / install | 100 | all six |
| ShopPartyReader | [src/core/shop/shop_party.cpp](../src/core/shop/shop_party.cpp) / install | 110 | all six |
| ShopPatches | [src/core/shop/shop_patches.cpp](../src/core/shop/shop_patches.cpp) / install | 100 | all six |
| ShopMenuTracker | [src/core/shop/shop_tracker.cpp](../src/core/shop/shop_tracker.cpp) / install | 100 | all six |
| ConfigMenu | [src/core/status/config_menu.cpp](../src/core/status/config_menu.cpp) / install | 125 | all six |
| Status.DisplayedDetails | [src/core/status/status_displayed.cpp](../src/core/status/status_displayed.cpp) / install | 121 | all six |
| StatusMenu | [src/core/status/status_menu.cpp](../src/core/status/status_menu.cpp) / install_status_menu | 120 | all six |
| CommandBar | [src/core/system/command_bar.cpp](../src/core/system/command_bar.cpp) / install | 100 | all six |
| CursorNavigation | [src/core/system/cursor_navigation.cpp](../src/core/system/cursor_navigation.cpp) / install | 100 | all six |
| FieldMenu | [src/core/system/field_menu.cpp](../src/core/system/field_menu.cpp) / install | 100 | all six |
| GameState | [src/core/system/game_state.cpp](../src/core/system/game_state.cpp) / install | 100 | all six |
| KeyHelpReader | [src/core/system/key_help.cpp](../src/core/system/key_help.cpp) / install | 100 | all six |
| SaveLoad | [src/core/system/save_load.cpp](../src/core/system/save_load.cpp) / install | 100 | all six |
| ConfirmationDialog | [src/core/ui/confirmation_dialog.cpp](../src/core/ui/confirmation_dialog.cpp) / install | 100 | all six |
| UI.TimerOwners | [src/core/ui/live_widgets.cpp](../src/core/ui/live_widgets.cpp) / install | 105 | games |
| ModMenu | [src/core/ui/mod_menu.cpp](../src/core/ui/mod_menu.cpp) / install | 100 | all six |
| ModUI.Input | [src/core/ui/modal_input.cpp](../src/core/ui/modal_input.cpp) / install | 490 | all six |
| UI.ReviewBuffer | [src/core/ui/review_buffer.cpp](../src/core/ui/review_buffer.cpp) / install | 90 | all six |
| TextInputWindow | [src/core/ui/text_input.cpp](../src/core/ui/text_input.cpp) / install | 100 | all six |
| FF1.JobSelection | [src/games/ff1/job_selection.cpp](../src/games/ff1/job_selection.cpp) / install | 501 | game::kFF1 |
| FF1.NewGame | [src/games/ff1/new_game.cpp](../src/games/ff1/new_game.cpp) / install | 500 | game::kFF1 |
| FF1.Puzzle | [src/games/ff1/puzzle.cpp](../src/games/ff1/puzzle.cpp) / install | 502 | game::kFF1 |
| FF2.BattleMagic | [src/games/ff2/battle_magic.cpp](../src/games/ff2/battle_magic.cpp) / install | 530 | game::kFF2 |
| FF2.DetailsKey | [src/games/ff2/details_key.cpp](../src/games/ff2/details_key.cpp) / install | 545 | game::kFF2 |
| FF2.Util | [src/games/ff2/ff2_util.cpp](../src/games/ff2/ff2_util.cpp) / install | 500 | game::kFF2 |
| FF2.Keywords | [src/games/ff2/keywords.cpp](../src/games/ff2/keywords.cpp) / install | 510 | game::kFF2 |
| FF2.MagicMenu | [src/games/ff2/magic_menu.cpp](../src/games/ff2/magic_menu.cpp) / install | 520 | game::kFF2 |
| FF2.NewGameNaming | [src/games/ff2/new_game.cpp](../src/games/ff2/new_game.cpp) / install | 510 | game::kFF2 |
| FF2.ShopCommandReader | [src/games/ff2/shop_command.cpp](../src/games/ff2/shop_command.cpp) / install | 520 | game::kFF2 |
| FF3.BattleMagic | [src/games/ff3/battle_magic.cpp](../src/games/ff3/battle_magic.cpp) / install | 506 | game::kFF3 |
| FF3.EventItemSelect | [src/games/ff3/event_item_select.cpp](../src/games/ff3/event_item_select.cpp) / install | 502 | game::kFF3 |
| FF3.JobMenu | [src/games/ff3/job_menu.cpp](../src/games/ff3/job_menu.cpp) / install | 500 | game::kFF3 |
| FF3.LineFade | [src/games/ff3/line_fade.cpp](../src/games/ff3/line_fade.cpp) / install | 503 | game::kFF3 |
| FF3.MagicMenu | [src/games/ff3/magic_menu.cpp](../src/games/ff3/magic_menu.cpp) / install | 505 | game::kFF3 |
| FF3.MovementSpeech | [src/games/ff3/movement_speech.cpp](../src/games/ff3/movement_speech.cpp) / install | 504 | game::kFF3 |
| FF3.NewGame | [src/games/ff3/new_game.cpp](../src/games/ff3/new_game.cpp) / install | 501 | game::kFF3 |
| FF4.AbilityMenu | [src/games/ff4/ability_menu.cpp](../src/games/ff4/ability_menu.cpp) / install | 510 | game::kFF4 |
| FF4.BattleTargets | [src/games/ff4/battle_targets.cpp](../src/games/ff4/battle_targets.cpp) / install | 521 | game::kFF4 |
| FF4.Naming | [src/games/ff4/naming.cpp](../src/games/ff4/naming.cpp) / install | 514 | game::kFF4 |
| FF4.PartySetting | [src/games/ff4/party_setting.cpp](../src/games/ff4/party_setting.cpp) / install | 512 | game::kFF4 |
| FF4.Timers | [src/games/ff4/timers.cpp](../src/games/ff4/timers.cpp) / install | 530 | game::kFF4 |
| FF4.Vehicles | [src/games/ff4/vehicles.cpp](../src/games/ff4/vehicles.cpp) / install | 540 | game::kFF4 |
| FF5.BattleTarget | [src/games/ff5/battle_target.cpp](../src/games/ff5/battle_target.cpp) / install | 540 | game::kFF5 |
| FF5.Mod | [src/games/ff5/ff5_mod.cpp](../src/games/ff5/ff5_mod.cpp) / install | 506 | game::kFF5 |
| FF5.JobAbility | [src/games/ff5/job_ability.cpp](../src/games/ff5/job_ability.cpp) / install | 500 | game::kFF5 |
| FF5.Naming | [src/games/ff5/naming.cpp](../src/games/ff5/naming.cpp) / install | 510 | game::kFF5 |
| FF5.Timers | [src/games/ff5/timers.cpp](../src/games/ff5/timers.cpp) / install | 505 | game::kFF5 |
| FF6.AbilityEspers | [src/games/ff6/ability_espers.cpp](../src/games/ff6/ability_espers.cpp) / install | 521 | k6 |
| FF6.AbilityMenu | [src/games/ff6/ability_menu.cpp](../src/games/ff6/ability_menu.cpp) / install | 520 | game::kFF6 |
| FF6.Airship | [src/games/ff6/airship.cpp](../src/games/ff6/airship.cpp) / install | 510 | game::kFF6 |
| FF6.BattleAbilities | [src/games/ff6/battle_abilities.cpp](../src/games/ff6/battle_abilities.cpp) / install | 510 | game::kFF6 |
| FF6.BattleMessage | [src/games/ff6/battle_message.cpp](../src/games/ff6/battle_message.cpp) / install | 500 | game::kFF6 |
| FF6.BattleTargets | [src/games/ff6/battle_targets.cpp](../src/games/ff6/battle_targets.cpp) / install | 511 | game::kFF6 |
| FF6.Coliseum | [src/games/ff6/coliseum.cpp](../src/games/ff6/coliseum.cpp) / install | 530 | game::kFF6 |
| FF6.Keys | [src/games/ff6/keys.cpp](../src/games/ff6/keys.cpp) / install | 520 | game::kFF6 |
| FF6.Details | [src/games/ff6/module.cpp](../src/games/ff6/module.cpp) / install_details | 900 | game::kFF6 |
| FF6.Opera | [src/games/ff6/opera.cpp](../src/games/ff6/opera.cpp) / install | 510 | game::kFF6 |
| FF6.StoryGuide | [src/games/ff6/story_guidance.cpp](../src/games/ff6/story_guidance.cpp) / install | 515 | game::kFF6 |
| FF6.DefenseWatch | [src/games/ff6/defense_watch.cpp](../src/games/ff6/defense_watch.cpp) / install | 516 | game::kFF6 |
| UI.NameInput | [src/core/ui/name_input.cpp](../src/core/ui/name_input.cpp) / install | 117 | all six |
| FF6.Orderly | [src/games/ff6/orderly.cpp](../src/games/ff6/orderly.cpp) / install | 520 | game::kFF6 |
| FF6.PartySetting | [src/games/ff6/party_setting.cpp](../src/games/ff6/party_setting.cpp) / install | 512 | game::kFF6 |
| FF6.StatusList | [src/games/ff6/status_list.cpp](../src/games/ff6/status_list.cpp) / install | 520 | game::kFF6 |
| FF6.Timers | [src/games/ff6/timers.cpp](../src/games/ff6/timers.cpp) / install | 505 | game::kFF6 |
| FF6.Vehicles | [src/games/ff6/vehicles.cpp](../src/games/ff6/vehicles.cpp) / install | 540 | game::kFF6 |
