# Menus, dialogue and screen-owned readers

[October26 Warehouse](../re/WAREHOUSE_20261026.md) closes a confirmed screen
coverage gap. The new menus/warehouse.cpp owner follows native command/list states,
reads rendered count/empty/description fields, and directs I/Shift+I to its exact
description/footer. Existing Popup.Warehouse Show now uses a typed text query;
base quantity-change hooks retain its single prompt producer and native clamps.
October27 empty responses belong to their target-init event, with the native
bar restoration consumed once; command focus never appends lingering empty text.
Quantity changes include the displayed stock figure with the selected amount.
Generic cursor/late discovery yield to the positive owner. No inventory writes,
native waits, scene scan or worker pointers. Full checks/scan/all-six install pass.

[October26 party status](../re/ALLY_STATUS_20261026.md) adds owner-thread completed
form/text identity observers and bounded live renderer evidence to the shared
H/party-number/target formatter. Visible native text/icons remain the first producer;
red/gray HP supplies Critical/KO, with body fallback when condition columns are
hidden. No raw status inventory, speech-text dedup, scene scan or worker pointers.
Full checks/scan/install hashes pass all six; new listening remains pending.

[October26 shop compatibility](../re/SHOP_COMPARISON_20261026.md) corrects
product classification to Content.get_TypeId; Weapon/Armor keys are separate.
The number row retains native portrait/arrow speech and gains the missing
Can not equip equivalent. Full offline checks pass; the combined October26
payload is installed/hash-verified all six. One native shop icon can summarize either permitted
hand; Equip owns explicit hand selection.


[October25 field-menu feedback](../re/NPC_PATROL_TARGETS_20261025.md) adds a shared
native range owner: AllInit says All allies; SingleInit restores one rendered
focused row, with FFV retaining its own bounded single producer. Other specific
and generic cursor readers yield during all-target/return transitions. Close and
native context events retire ownership. FFIII event-key-item description follows
Autodetail and has a context-owned I action, with Event12 admission that preserves
loading/fade guards. Enemy critical health is a separate default-off Speech
disclosure; visible enemy effects, party status and critical-hit damage retain
their existing producers. Settings persist through the established settings worker.

[October25 map/shop/Learn ownership](../re/NPC_FLOORS_SHOP_LANDING_20261025.md):
the positive MapPanels owner permits mod review during native Event12 while
loading/fade guards remain. Immediate/deferred generic cursor readers yield to
that owner; basic maps have no location counters. Detailed Items and Chests both
support I/Autodetail/buffer review, with native WASD selection preserved. FFIII
Magic Learn retires old list/target ownership before native preview rebuilding;
only the current window's positive list phase can read rows. Native ready-focus
updates/generation cancellation remain. Other adapters retain their own phases.
Shop1..9 identifies visible assigned portraits and reads Can/Cannot equip from
the native shop animation state independently of comparison arrows/Equipped.
Only equipment portraits permit that lookup; idle magic/consumables do not.

[October25 MP payload follow-up](../re/FF3_FEEDBACK_20261025.md) keeps the
verified native ownership checks, but manual I returns only the visible current
MP levels. Ordinary focus owns the short row; Autodetail appends MP once. This
also covers actual battle command/ally-target charge queries in FF1/FFIII.

[October25 character I-key follow-up](../re/INN_COUNTER_MP_20261025.md) corrects
the mistaken IsFocus requirement: native Select clears it on the active cursor.
Actual selection phase, visible native cursor and exact template anchors own
manual details; passive overview List/None phases cannot steal I from another menu.

October25 [alternate save/context and visible-feedback audit](../re/SAVE_CONTEXT_VISIBILITY_20261025.md)
documents scripted/completed SaveWindow owner/popup hooks, exact slot cursor
identity, positive MainGame Menu5 command gating and FFIII Jobs icon readiness.
[Concise character rows](../re/UR_CHARACTERS_20261025.md) separates the eight current
MP cells into I/Autodetail at the end of each layout. ui/character_row retains one
native cursor/row and validates its visible serialized anchor on demand; Status
review puts MP last. Item learning/exchange owns state6, and delayed generic
callbacks recheck ownership. Raw identities, native input and repeated speech
events remain intact. The same record documents Ur Well/Shed and Dahn naming.

October25 [battle hand slots and controls](../re/BATTLE_GEAR_CONTROLS_20261025.md)
adds the separate native Tab hand-slot owner to Battle.Item. It shares the current
rendered description/I/Autodetail context, while candidates keep actual counts.
Shift+I now includes standalone KeyIcon widgets and bounded on-request native
KeyHelpIconManager registries; all native pointers remain on their owner and
only copied strings reach workers. Existing map/Bestiary/config ownership remains.

October24 [battle Items equipment strip](../re/BATTLE_ITEM_EQUIPMENT_20261024.md)
adds native SelectEquipContent/UseEquipSelectInit observation to Battle.Item.
Its rendered equipment row shares I/Autodetail and context with ordinary Items;
native Tab equipment replacement and item usability remain game-owned.

October23 adds UI.NameInput/order117 in core/ui/name_input.cpp. All-six native
KeyboardManager.ShowKeyboard opens Last.UI.KeyInput.InputPopup; its tracked
Unity InputField.KeyPressed supplies actual before/after text/caret edits.
Arrows speak crossed letters; Backspace speaks removed text. Deselect/scene/title
retire the owner. No polling or replacement input is added. Bounded text copying,
localization, native signatures and verification are in the
[follow-up contract](../re/NAVIGATION_FOLLOWUPS_2026_10_23.md).

Current audit: October22. Implemented source families below supplement the historical
PORT_STATUS checklist. The runtime contract governs affinity, speech, localization,
cancellation and extension. Controls are enumerated in INPUT_BINDINGS/readme; local
native game controls remain native. Existing functionality is not gameplay-certified
merely because its registration is listed here.

| Responsibility | Actual sources and native producer/data flow | State and extension boundary |
|---|---|---|
| Dialogue and choices | dialogue/message_window.cpp observes MessageWindowManager SetContent/PlayingInit/Close, copies displayed speaker/page; message_choice.cpp Show/Close schedules ready default cursor focus | Repeat R reads the current dialogue; close invalidates pending focus. No hidden script playback or blanket wording guard |
| Scrolling/fading stories | dialogue/scroll_message.cpp plus ff3/line_fade.cpp observe native text/lifecycle and copy the current visible page/line | Preserve legitimate repeated pages and native one-frame deferrals; do not invent scrolling timers |
| Popups, confirmation and Game Over | dialogue/popup.cpp follows native popup Open/Close and command focus, including save confirmations, play data and warehouse; ui/confirmation_dialog.cpp is the separate mod-owned question | Native popups keep their context; mod questions own sequence/input until completion. Closing cancels pending work; do not reuse stale parent focus |
| Title, save/load and field menu | dialogue/title.cpp; system/save_load.cpp; system/field_menu.cpp, command_bar.cpp and game_state.cpp observe ready title/slot/command/state events | Save rows use actual visible labels/date/time/map plus the documented full-party exception. Corrupt/empty rows exclude stale detail. Main-game state closes other menu ownership |
| Generic cursor and help | system/cursor_navigation.cpp routes native cursor methods to the active reader; key_help.cpp reads displayed key help | A specialized context owns its cursor. Clear/reset on context transitions; help is current-screen, not a universal static key list |
| Items and use feedback | menus/item_menu.cpp, item_equipment.cpp, item_details.cpp, character_selection.cpp and use_feedback.cpp | Native list/target controllers own selection and quantities. I follows current displayed detail; use feedback follows completed native updates, not predicted HP |
| Equipment | menus/equip_menu.cpp, equipment helpers and displayed_comparisons.cpp | Current numbered page/footer witnesses choose stats/description; native Q/2/3 semantics remain. Named equipment and displayed deltas use the same row owner |
| Magic | menus/magic_menu_state.cpp and magic_menu_ff1.cpp; FF2/3/4/5/6 adapters listed in GAME_ADAPTERS | Shared state selects specialized native list/target family. FF2 levels/uses and FF3 tier slots use their own actual classes; no copied cross-game offsets |
| Shops | shop/shop_tracker.cpp, shop_patches.cpp, shop_magic.cpp, shop_party.cpp plus FF2 command adapter | Native shop state selects buy/sell/quantity/party comparison. Price/quantity/affordability and displayed panel stay with the highlighted item; slots1–9 request comparisons |
| Status and config | status/status_menu.cpp, status_displayed.cpp, status_navigation.cpp and config_menu.cpp | Native character/page/scroll selection owns the reader. Detail buffers use rendered fields/icons under MENU_DETAILS. Config changes actual native values; mod settings are separate |
| Bestiary | bestiary/bestiary_patches.cpp, bestiary_reader.cpp, bestiary_navigation.cpp | Config and title Extras share completed content producer, bounded capture and owned NavigationBuffer; see BESTIARY |
| Gallery | extras/gallery.cpp observes Extras scene state and displayed gallery focus/number/name | Read the focused native entry metadata only. Native view close/scene state clears ownership |
| Music player | extras/music_player.cpp observes sound-scene state, list focus, original/arrange switch and playback key-help icon | Native time/play state and track row are authoritative; repeat/details follow the current track, not an independent player |
| Mod menu and text entry | ui/mod_menu.cpp, modal_input.cpp, text_input.cpp, navigation_buffer.h and input binding editor | F8 ownership, Speech/Navigation/Navigation audio/Battle, help I, capture/edit/confirm flows and per-game settings use central owned speech sequences |

All source paths are beneath src/core except explicit game adapters. The feature
index links each registration directly. Rendering helpers (displayed_ui, displayed
conditions/comparisons/skill progress) verify active/visible owner, sprite meaning,
actual text and gauge. MENU_DETAILS preserves the per-feature native locators,
readiness, comparison/condition/ability/status contracts and disclosure limits.
ReviewBuffer supplies domain-owned arrow/group/detail navigation for results and
other readers; an existing buffer must not turn into a shared stale global page.

Autodetail is independent/default Off; manual I remains available. Menu position
announcements, verbose/brief deltas, results-notifications-once and speech enable
are shared preferences, not reader-specific copies. Unknown/localized labels stay
with the active game's catalogs and UI-language snapshot. Game-state callbacks
invalidate owner/generation; no worker captures native views. Event guards are
listed in DEDUP_INVENTORY and preserve deliberate repeat/focus inputs.

Validation combines modal_input/modal_speech, battle_focus, battle_results,
results_native, title_prompt, bestiary, timing, displayed/native asset and all-six
name/wiring checks. Fakes exercise actual consumers and lifecycle, not real Unity
pooling, accessibility backend audibility or every native panel allocation. TESTING
retains unconfirmed screens and simple player steps. Extend a reader at its native
completed producer and current context; don't add a scene-wide text sweep or infer
rendered fields from a master object when the game still displays placeholders.
