# October23 navigation, defense and speech follow-ups

This is the current implementation/design register, supplementing
[FF6 story navigation](FF6_STORY_NAVIGATION.md). No game was launched, controlled
or probed. Gameplay confirmation remains pending; [test defense first](../TESTING.md).
Final build/install verification and recovery are recorded below.

| Feature | Implementation | Offline evidence | Gameplay |
|---|---|---|---|
| Saved story-gate filter | Complete | Actual owned graph traversal on all six bakes, NPC/gate/cancel, world layers and parked vehicles | Pending |
| Save Points category and one real FF6 destination | Complete | All42 native points,42 tutorial companions and42 animations | Pending |
| Brief state/party-aware FF6 guidance | Complete | Actual recipes/model/controller, pressure release and manual party changes | Pending |
| Terra/Banon danger awareness | Complete | Native-update occurrence controller and actual legal route comparisons on both maps | Pending |
| Moved-event warning correction | Complete | All-six current-cell fixtures and exact FF6 three-step log route | Pending |
| Nearby “Interact: Jun” cue | Complete | Actual cue producer and developer-prefix fixture | Pending |
| Native name-entry letters | Complete | Actual popup/key detours and repeated-character/close/owner fixtures | Pending |
| Multipart boss target names | Complete for10 verified identities | Current master/message adapter; real sprites and FF5 part-script titles | Pending |

## Controls, data flow and story filtering

Ctrl+Shift+Backslash toggles StoryGateFilter in either navigation layout.
F8 > Navigation > Hide story-gated destinations exposes the same independent,
persisted default-Off setting. Existing Auto/manual language selection applies.
Preferences -> Mod.ToggleStoryGateFilter/mod menu -> Settings.Changed -> scanner
reapply -> owned FilterInput -> batch story_gate_mask -> category views.

entity_filter builds one pair of bounded graph traversals for the whole list,
not a full route search per row. Current-floor geometry uses copied native
obstacles; remote geometry uses the shared map-state resolver. Both passes keep
NPC/chest blockers, real movement edges, script travel, PointIn eligibility,
transport/landing/parked-vehicle rules and interaction approaches. Only story
gates are removed in the second pass. A row is hidden only if a completed closed
search cannot reach it and gate-free search proves reachability. Unknown,
cancelled or bounded-out evidence retains the row. Remote browsing does not cross
world maps. World candidates use walking, riding and legal owned-vehicle landings.

The graph has the existing400 arrival-state cap; requests inherit cancellation
checks inside floods/vehicle searches. Targets are bounded by the scanner's
owned rows. NPC numbering precedes story pruning, so filtering cannot renumber
same-name people. The filter changes visibility, not native world state or the
remembered manual route. Setting Off restores available original rows. Native
capture remains sliced with CaptureBudget; only owned values cross to workers.
Core source: field/entity_filter, entity_scanner, baked_nav; input/mod_actions,
input_manager; ui/preferences/mod_menu. Existing large baked_nav remains the
cohesive shared routing boundary; no separate algorithm or geometry is introduced.

## Save Points

SavePoints is appended as category6; existing categories0–5 keep their identity.
All category arrays/iteration use kCategoryCount. Native ObjectType9 is the real
save-point identity, even if its GameObject name says EventTriggerEntity. FF6
savepoint and セーブポイント_エフェクト decorations are native type10/asset256,
with no script/action, and are excluded. Arbitrary names containing “save” do not
establish an FF6 save point.

field/save_points consolidates only the same map/layer/cell: one actual point,
plus suppression of its exact script20/action2 tutorial in the same group. It
retains different physical points and the script619 event leading to a save room.
Infer routing lets the point's own overlapping tutorial be its selected action,
so it is not described as an intervening event. No interaction script is changed.

## Brief guidance and defense mechanics

Shift+G requests directions and necessary actions; Ctrl+G stops the guide.
Controller field mod mode retains right-shoulder guidance/left-shoulder stop.
Introductions and parked-party location summaries no longer prefix directions.
Native manual party changes cancel stale routes/audio/selection and announce only
“Party n”. Each party keeps its own pending objective. Phoenix/Tower held switches
still say “Hold switch. Switch parties.”; safe departure uses the existing release
prediction. A defense has no compulsory party-switch sequence.

Terra map318 / Map_30011_2: scenario13 set,14 clear; defend native NPC32,
contact enemies18–23, completion Guard Leader9. Later Narshe map19: scenario137
set,138 clear; defend Banon64, enemies24 and38–48, completion Kefka25. All belong
to the authored ev_e_0257 defense group on map19; sc_e_0259 is Kefka's script,
not a different entity group. This is the army defense requested alongside Cyan;
Doma's earlier story is not treated as a three-party timed defense.

FF6.DefenseWatch/order516 detours FieldNonPlayer.UpdateEntity after native processing.
FF2550 tail-calls FieldCharaEntity.UpdateEntity/FE1510; the observer runs after
that completed producer, independent of intermediate base-update calls.
Only these two maps, relevant active FieldNonPlayer IDs and transformed cells are
observed. Arrays are bounded to64 IDs; metadata permits at most16 enemies and
three parties. A same-layer enemy entering Chebyshev distance3 from the protected
actor creates an occurrence. It announces once for that enemy while in range;
leaving and re-entering permits a new warning. Speech text is not deduplicated.
A new Shift+G always repeats current danger, even with identical text.

The alert is brief: “Terra in danger. Party 2 closest.” Banon uses his own name.
The worker chooses the live enemy closest to the protected character, then
compares actual legal interaction-route costs from all three party positions.
Current party wins equal-cost ties. Missing any party pose suppresses the closest
claim; the danger still speaks. Shift+G prioritizes that immediate threat, then
otherwise prefers the commander who ends the event. All other enemies remain
fallback reachable encounters; the guide never requires defeating every mob.

Native-owner capture copies SaveMapManager.SaveData.otherPartyDataList, exactly
three slots indexed by the same zero-based corps slot used by SwitchCorps.
SaveMapPartyData.mapId and PlayerEntity.position supply parked poses; playerEntity
JSON is a bounded16384-character fallback, parsed only by the worker. The current
slot is always replaced with the live player cell. Ground layer1 and native
16-unit local-to-cell conversion apply to these two maps. Unknown/foreign-map
poses remain unknown. No mod-maintained historical party pose is substituted.

The watcher owns one pending sliced capture/job, independent of manual and guide
route receipts. Further entrances mark it dirty and coalesce a follow-up. It
uses the bounded Navigation lane, owned ExitSnapshot and worker_job epochs. No
worker retains an engine pointer or calls speech/native APIs. Map/title retire;
battle/party admission interrupts; ready owner cleanup retires callbacks. Native
battle completion resets proximity observations but retains announced IDs.
Publication rechecks map, readiness, live phase flags, player origin and current
enemy cell/range; stale movement requests a fresh capture. No old warning can
publish after completion on the same map. It never waits for work on the game
thread, alters input, pauses enemies or writes scenario state.

Sources: games/ff6/defense_watch.cpp, story_guidance.cpp, story_model.{h,cpp};
field/baked_nav.{h,cpp}; tools/author_ff6_story_guides.py; data/ff6/puzzles.json.
Native ABI/party indexing evidence is external at ../tools/ff6_story_20261023/native.txt;
SaveMapData.AddOtherPartyData RVA B34150 and SaveMapManager position overload
B345A0 construct and deserialize the slot's SaveEntityData with its native pose.
Dump fields confirm otherPartyDataList array, position Vector3 and serialized
field identity. No source-bearing evidence is copied into Git.

## Live event positions and interaction speech

The requested latest FF6 log initially measured3,876,822 bytes. Lines56789–57590
show the false north-route warning;58278–58285 show it clear after returning.
Map323 is Map_30021_1; ev_e_0011/Event11/script22. Its sc_e_0011_1 continuation
moves the event instead of destroying it. live_blockers previously resurrected
its baked cell20,8. All-six one-tile trigger/trap footprints now use the captured
current native cell. The exact logged route20,10 to20,7 becomes three clear steps
without re-entering. An actually active trigger is still avoided or warned about.
Multi-cell footprints retain baked origin/extent until native rectangle relocation
semantics are established; this fix does not claim that unverified case.

Native TriggerPower remains the future-trigger observation. IsTriggerEnable
requires power AND current overlap; using it would wrongly discard future events
whenever the player was not standing on them. Fourteen bounded native methods per
game were inspected; evidence/hashes remain outside Git under
../tools/event_trigger_20261023/, produced by tools/re/event_trigger_native.py.

interaction_cue keeps the existing native icon/owner occurrence guard and uses
npc_display_label(entity_name_from_property(...)).base. “ev_e_0013: Jun” becomes
“Interact: Jun”. No numbers from developer prefixes or NPC ordinal are spoken.
The shared cue path applies to all six games. Translation remains centralized.

## Native name entry and battle parts

UI.NameInput/order117 observes all-six KeyboardManager.ShowKeyboard ->
Last.UI.KeyInput.InputPopup.Open -> tracked UnityEngine.UI.InputField.KeyPressed.
Native KeyPressed returns an int32 EditState; its original return is preserved.
Arrow KeyCodes273–276 speak the actual crossed UTF16 letter; Backspace8 speaks
only the native removed text. Repeated equal letters each speak. Native selection
removal speaks the removed selection; surrogate pairs stay intact. A removed
space uses the existing localized “space” key. No automatic “deleted” prefix.

Only the active tracked field is read. Deselect, scene and Title retire identity;
speech Off suppresses echo. Text copies reject lengths above128 before allocation,
using shared ref::rd_str_bounded; party fallback uses the same helper with16384.
There is no key polling, injected text, new keyboard mode or retained worker pointer.
Native metadata/disassembly locators are in the external event-trigger evidence.
Core source: ui/name_input.cpp/name_input_echo.h; refs.h/.cpp.

battle/enemy_parts qualifies current BattleEnemyData.MonsterMaster.get_Id plus
its current message key, so form changes cannot inherit a stale qualifier.
FF6: Ymir257 Shell/309 Head, Angler Whelk258 Shell/310 Head, Chadarnook287
Goddess/329 Demon. FF5 Neo Exdeath387 Center/388 Lower right/389 Upper right/
390 Lower left: the game's actual script titles explicitly identify those areas.
Ten identities and source hashes are in [boss_part_identities.json](boss_part_identities.json),
regenerated by tools/audit_boss_parts.py. Sprites/scripts remain outside Git.

These names reach target, status and unit-name reads through enemy_name. Meaningful
part names remove unnecessary duplicate letters; identical names still get their
usual A/B distinction. Opening formation roster and bestiary retain native display
names. Soul Cannon already names Launchers separately. No real/false-clone identity
is revealed for Stalker/Erebus; invisible Neo Exdeath511 is excluded. Crane and
Tentacle positional variants have no new anatomical adapter in this change.
Eight scoped “Boss part:” keys avoid corrupting the existing Shell magic or Head
gear translations. All31 added keys have12 locales (372 values), including game-
verified Terra/Banon proper names.

## Verification, performance and remaining limits

Canonical build.bat gates include actual story model/controller, defense advice,
Opera producer, story filter, savepoint classification, native name edits and
boss identity checks, plus existing all-six route/native owner/input/speech checks.
The state model resolves107 physical recipe references and30 state plans. Runtime
checks park/cancel old work, test party switches without unsolicited directions,
20 equal native updates without warning repetition, explicit Shift+G repeats,
leave/re-entry, map retirement and same-map completed-phase rejection.

The real defense fixture compares all three legal interception costs, verifies
commander preference after threats disappear, and retains unknown-party safety.
Banon tests start on the battlefield side; cells behind his blocking NPC are not
valid party starts. The active Ymir positive control permits a legal five-step
detour, while the moved-trigger case reproduces the three-step user route. No
barriers were removed to make fixtures pass. All42 save points are checked against
both tutorial and animation companions. Gate fixtures span all six games, native
one-axis colliders, unknown state, cancellation, world layers and parked vehicles.

Offline fixture cost is not live/audible latency. Native capture uses0.5ms slice
admission, without claiming that an indivisible engine call cannot exceed it.
Defense diagnostics report aggregate native capture, capture wall time, queue,
compute, usable publication and oldest snapshot age; speech carries original
request timing into existing Directions latency reporting. Queue tails, native
callbacks under real load, actor motion during speech and audible device latency
remain unmeasured. A constantly moving actor/player may require repeated snapshots;
no universal response deadline is claimed. Existing lane capacity/cancellation
prevent inline native work or unbounded pending jobs.

Luna (gpt-6-luna) performed only the17 brief-message and4 boss-position catalog
tasks. Sol reviewed all locales/placeholders against actual names, corrected
Korean Banon, Russian Terra and Thai Terra, and integrated/validated the changes.
The updater scripts were run twice to verify idempotence. Sol retained mechanics,
RE, architecture, implementation, review and testing. No Astra was used.

Safe extension requires a traced native phase, protected/hostile identities and
actual completion objective; do not invent party locations or infer hidden boss
weaknesses. Preserve owner/thread boundaries, cancellation, count limits and
identity occurrence guards; extend actual-data fixtures, localized keys and docs.
Final delivery hashes, preserved files and test reports are recorded below.


## Final delivery

Canonical build.bat and every offline gate pass. Name checks:7,179/zero missing;
actions/queries:125 used,333 defined,zero undefined. Isolated smoketest.bat loaded
and forwarded all47 HID exports and exited normally. No game was launched.
All125 registrations match FEATURE_INDEX;31 keys/372 localized values pass
coverage/placeholders/proper-name review. All19 embedded maps/state/puzzle/catalog
resources match current source bytes.

Final DLL SHA256:
ad901861a8d98f3e523aed5d0b2e3fb96af26e1c8ad5219ae9cf4b5b16d096c1.
Canonical deploy.bat installed/hash-verified the same DLL in all six games.
The144 existing Prism/OpenAL/settings/waypoint files retain bytes and timestamps.
Previous DLLs and SDL files are retained per game under
build_check/navigation_followups_20261023/recovery/. The delivery.json,
protected_before/after manifests, canonical build log, smoke output and every
check report are in that same receipt directory. No commit/push/public release.

This run's offline fixtures: defense advice worst0.274ms;30 model plans1.157ms
aggregate; small gate batch worst1.559ms;103 boarded-vehicle cases worst route
28.098ms/filter16.338ms; six-category world preparation worst2.807ms for582
candidates. These fixtures have fake native boundaries and do not measure live
callback/audible cost. Existing parked-vehicle bulk/reference comparisons and
cancellation checks remain in route_worker_report.txt. A final native review
narrowed the hook from the broad base update to the exact completed NPC producer;
all checks/smoke/deploy were repeated after that change. Intermediate failed
builds/fixture assumptions are not counted as delivery evidence.

Gameplay remains unconfirmed. Test brief Terra defense guidance and danger
warnings first, then Banon/Phoenix/Tower/Opera when progression permits, using
TESTING.md. Existing progression/native controls remain authoritative.
