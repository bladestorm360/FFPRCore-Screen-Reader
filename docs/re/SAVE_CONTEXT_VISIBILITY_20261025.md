# Save paths, menu preload and visible battle feedback, October25

Status: complete/offline verified. Full build/all checks, smoke, resource matching
and exact22-file Defender scan passed. Payload6c6a8045a0aeeedd33c220f1439837e597b06f5725f3a4cdc2c9fe628fca16c1
is installed/hash-verified in I/II/IV/V/VI; III was deferred by the running-game
guard. The [Ur/character follow-up](UR_CHARACTERS_20261025.md) carries these fixes
forward and is now installed/hash-verified all six after the user's FFIII deploy
reply. The earlier deferral is history. Gameplay/listening remains unconfirmed. No game was launched,
controlled or probed; no delegation or dependency change.

## Scope and sources

The user reported FFII's Completed Game Save list saying File/World Map without
slot data, silent overwrite body and silent completion. The user requested an
all-six alternate-save audit, then added FFIII intro preload speech, Onion Knight
speaker identification, goblin opening text and hidden Luck feedback. The pending
[warp-label correction](WARP_LABELS_20261025.md) remains in the combined payload.

The exact native audit is tools/save_paths_audit.py. Reproduce annotated bodies
with tools/re/map_location_native.py, --save-paths or --dialogue-opening. Proprietary
bodies/provenance stay outside Git in ../tools/save_paths_20261025/ff1..6 and
../tools/dialogue_opening_20261025/ff1..6. Ignored build_check/save_paths_20261025
holds recovery, test, final payload, security and delivery receipts. No game code
or snippets are included in this document.

## All-six save architecture and correction

| Path | Native owner and producer | Reader and lifetime |
|---|---|---|
| Title load | Last.UI.KeyInput.LoadGameWindowController.SetActive/SetPopupActive | Existing title owner and SavePopup body/choice |
| Menu load/save | Last.UI.KeyInput.LoadWindowController/SaveWindowController.SetPopupActive | Existing SavePopup body/choice; menu CompleteInit calls SetPopupActive again |
| Quick save | Last.UI.KeyInput.InterruptionWindowController.SetEnablePopup/InitComplite | Existing in-place completion reader, including the game's misspelling |
| Scripted/completed saves | Last.UI.Save.KeyInput.SaveWindowController.Open/Close/SaveSelectInit/OverwriteConfirmInit/CompleteInit | New exact owner; popup is on its serialized SaveWindowView |
| Other scripted questions | SaveConfirmInit, NoSaveConfimeInit, TitleConfimeInit, SaveSelectNoSaveConfimeInit | Existing CommonPopup Open/Close/own-focus path, including return-to-title question |
| Game Over | GameOverPopupController.InitSaveLoadPopup and GameOverLoadPopup.SetCommandSelectCursor | Existing separate recent-save prompt/choice reader in dialogue/popup.cpp |

These are the PC KeyInput routes used by both keyboard and gamepad. Touch-specific
controllers are not newly enabled. V/VI fold Touch/KeyInput scripted Open with the
same bool/delegate ABI; the detour forwards both and registers only its KeyInput
class. Other new hook bodies are unique. Name-resolved references retain native
ABI; production contains no RVA/offset table.

| Game | Scripted Open | SaveSelectInit | OverwriteConfirmInit | CompleteInit |
|---|---|---|---|---|
| I | 681D70 | 682830 | 681DD0 | 681060 |
| II | 659AB0 | 65A570 | 659B10 | 658DA0 |
| III | 4F0CA0 | 4F1760 | 4F0D00 | 4F00B0 |
| IV | 47A4B0 | 47AF70 | 47A510 | 4798C0 |
| V | 840B10 | 841F30 | 8411B0 | 83FA20 |
| VI | A2BFF0 | A2D410 | A2C690 | A2AF00 |

All six use the same serialized SaveListController and SavePopup. Scripted
overwrite/completion enable the popup directly instead of calling the ordinary
menu SetPopupActive hook. Completed saves also bypass MenuManager.IsOpen. Those
two gaps explain the reported slot/body/completion failures.

system/save_load.cpp now publishes the scripted owner before native Open, since
Open synchronously enters the list state. SaveSelectInit clears only this owner's
previous popup before native list callbacks. The existing one-frame SetActive
reader captures the completed native row. A published list/cursor identity supplies
contentList[index] directly in O(1); another cursor cannot borrow its row. The old
bounded hierarchy fallback remains for unobserved older paths.

Rows keep displayed slot label/number, Empty/corrupt state, timestamp, leader,
displayed level, area/floor and play time. The already approved party-sprite name
exception is unchanged. Empty/corrupt rows return before stale detail. Scripted
save context short-circuits the old MenuManager singleton lookup; no scene search
or new polling was added.

The shared SavePopup reader is reused: actual visible message, otherwise actual
visible title when the message is truly blank, optional submessage, focused button.
A hidden nonempty message does not authorize a title fallback. Completion therefore
reads the native success or failure text, without predicting save success.
Popup identity/current-state cancellation retires stale readers on list return,
replacement or close. The native reparented popup is recognized by its serialized
identity, independent of Transform ancestry. Ordinary CommonPopup focus ownership
continues to avoid a second initial Yes/No announcement.

All native pointers/reads remain on the owner thread; worker speech owns copied
strings. Existing bounded popup readiness ends within60 frames and list readiness
uses one next-frame callback. No save operation, input, file writing or game state
mutation is added. No new setting/key or mod-owned wording: displayed game text
and the existing localized Party/position strings cover all12 languages.

## Intro preload context, all six

FFIII log86297.328 enters Event12, then CampMenu/SecondMenu load; Use is spoken
at86297.625. The old negative field-only gate excludes states0..4, so it permits
an active preloaded item command window during Event12. The same structural gap
applies to FF1's reported new-game world arrival and the other adapters.

system/game_state.cpp now exposes is_main_menu_state: cached main-game owner,
actual Menu5, requested Menu5, and no native state-change depth. All six matching
state enums confirm Menu5/Event12. command_bar.cpp requires that positive context
for Item/Equipment/FF1 Magic speech and cursor ownership. Existing window phase,
bounded readiness, same-index event guards and re-entry reset remain. The helper
uses one cached native state getter and no singleton/scene search. Other screens'
state meanings and generic cursor ownership are not changed.

## Critical health / KO statistic visibility, all six

FFIII log86912.265 says Blue Wisp: Critical, Luck up;86913.375 says KO, Luck down.
This is the automatic critical-health condition (native type4/Dying animation),
not proof that a critical attack exposes a Luck statistic. DyingConditionFunction
Start selects AnimDying; KO retires the state. No native parameter_group row in
any of the six installed masters modifies Luck/id15. Raw base/temporary Luck
comparison is therefore not a conveyed ability effect.

battle/stat_feedback.cpp excludes Luck from dictionary/scalar/direct feedback in
all six games. Baselines still advance, preventing deferred leakage at action end.
Critical-health/KO/Stone Add scopes consume their automatic stat-reset payload and
retain the existing condition label producer. Dead targets' stat bookkeeping stays
silent; cache reconciliation still occurs. Current HP/MP damage/recovery, visible
condition changes, verified applied buff/debuff directions, Imbibe and the separate
Swap semantic presentation producer remain intact. Do not restore a raw parameter
merely because its numerical value changed. A future Luck effect requires a verified
visible producer before it becomes speakable; the master audit detects new groups.

## Onion Knight conversations and goblin opening review

The first log search accidentally matched only interrupting say entries; queued
speech must also be included when diagnosing any producer. The full FFIII trace
contains both cave/rock exchanges and normal battle feedback.

The exact dialogue keys are E0001_01_153_a_01..06 (sc_e_0001_1) and
E0002_00_153_a_01..04 (sc_e_0002). All ten speaker entries in story_cha are blank.
Their Msg calls use ordinary Field windows, with no speaker entity/portrait/bubble
target. Adjacent different lines have no intervening speaker-specific cue. The
scripts move/pose the four party sprites around the conversations; that alone does
not identify a particular speaker for each line. Static evidence found no per-line
identity cue, so no guessed names or script-speaker override was added. The user's
three subsequent cave screenshots confirm a fixed dialogue box with no speaker
header, portrait, speech bubble or per-character marker; party poses/arrangement
appear unchanged across these lines. This supports leaving these lines unnamed.

## FFIII Jobs equipment icons

The selected job's description used to cache only Equippable before the icon
images became ready. job_menu.cpp now retains its native owner/focus generation
and reads the current description on I. Autodetail uses the existing one-frame
native readiness event, then queues the job headline and current detail together.
Its icon list is bounded to32; hidden/unknown sprites are absent, and a visible
All Equipment label supersedes the pooled icons. Close/destroy/newer focus retire
the old generation. displayed_values accepts only the exact UI_Common_IconNN
equipment identity plus Unity's terminal (Clone) suffix. The native atlas returns
cloned sprites; arbitrary names/unknown IDs remain excluded. Actual-source
job_menu_check and the common sprite decoder tests cover these paths, including
I after delayed icon readiness. The icon/native atlas provenance is retained in
build_check/save_paths_20261025/job_icons_provenance.json, with source-bearing
native bodies outside Git at ../tools/job_icons_20261025/ff3.

Goblin battle86751.906 opens with Enemy formation, followed by three Goblin actions
before the party acts. No queued or interrupting ambush/preemptive/back-attack text
appears. FFIII's native StartPreeMptiveMes3B6330 sends its special-condition keys
through BattleUtility.SetCommandMessageAtKey6F5CC0, BattleUIManager.SetCommadnMessage
3D8480, KeyInput BattleMenuController.SetCommadnMessage6A19B0 and the already hooked
BattleCommandMessageController.SetMessage3BC2B0. The existing OpeningPresentation
scope queues that localized text before formation. Hooks installed successfully.
The trace is consistent with ordinary enemy initiative; without a recorded rendered
frame it does not independently prove which opening condition was drawn. No
synthetic ambush wording or hidden condition announcement was added.

## Verification and remaining player checks

save_load_check runs the actual save detours/readers with fake native owners:
nested completed-save entry outside MenuManager, full/empty row, exact cursor,
overwrite body/No, success/failure title, hidden/stale body, list return, destroyed
owner, existing title/menu/quick-save prompts and quicksave in-place completion.
save_paths_audit checks all-six matching native call paths, fields, folded ABI and
state enums. battle_context_check covers every state0..20, pending menu entry/exit
and state-entry depth; command_bar_check exercises actual consumers for all-six
preload suppression, valid menu movement and re-entry. battle_feedback_check adds
Luck/direct/end-action/KO/dead-target regressions alongside existing buffs, Imbibe,
Swap, resources, cancellation, overload and capture-to-fake-output measurements.
condition_labels_check audits all-six masters and unchanged12-language strings.
These are offline fixtures/native inspection, not measured audible game behavior.

When convenient, verify a completed/scripted save: a populated slot should read
its number and visible details; an empty slot should say its number and Empty;
overwrite should speak its question before the current choice; saving should read
the displayed result. Do not overwrite a wanted save solely for testing. On a new
game, Use should stay silent during intro/field construction and remain audible
when the actual Items menu opens. Critical health/KO should not say Luck up/down;
an actual buff or Imbibe/Swap should retain its effect feedback. Special battle
openings should read the game's exact displayed wording before enemy formation.
