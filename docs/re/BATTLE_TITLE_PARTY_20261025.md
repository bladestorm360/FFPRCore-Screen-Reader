# Battle title return and numbered party review

Status: complete, offline verified and installed/hash-verified in all six games.
Listening confirmation pending. Scoped shared repair across all six games.
No game control/probe, dependency change, release operation or delegation.

## Reports, diagnosis and scope

FFIII own rotated session log records native Return14 at121391.984, then
BattleCommand=1 and queued Attack at121392.984, before Title121393.421 and
TitleScreen121393.781. The producer reopens stale battle focus after ownership
ended; this is not duplicate speech content or an old speech queue.
Current session records BattleParty.Slot1 at121669.156, reading Corrad187/254,
and no Slot2..4 actions. Screenshot shows Corrad, Rae, Robyn and Ryco with HP
187/254,239/239,219/245,255/255. The input catalog keyed same-label registrations
only by context/description/layout. All nine party registrations share these,
so every one reused Alpha1's entry; dispatch returns after the first match.

## Implementation and ownership

Battle state main_game_transition already ends battle in the ChangeState prefix
for Return14 and field/title states. Battle.Command now requires in_battle before
SetCommandData/SetCursor, direct/group targets and native submenu notifications
publish state/readiness/speech. Original native operations always continue.
The native opening-ready callback on_battle_start also rejects a known non-Battle
main state, so a late presentation cannot restore ended ownership. Unknown initial
manager state retains the existing fallback. Initial-target bounded retries and existing one-frame cancel reads also stop
when ownership ends. command_reset_state advances the existing cancel generation
so an old callback cannot survive into another battle. No global mute, text
content deduplication, polling, timer, new engine call or worker-held pointer.
Normal turn/target/cancel presentation remains event-owned.

Input register_binding retains the first legacy persistent ID and its saved
remap. Only a different default chord sharing that ID receives a deterministic
.default.key.modifier suffix. Re-registration resolves the same independent ID;
party keys1..9, remap/conflict/capture and native input ownership use the existing
catalog. No settings reset or language strings are introduced. The check occurs
only during installation, not per-frame dispatch. Existing shop slot labels are
already distinct. Battle.Party rendered-row semantics remain unchanged: no hidden
statistics, slot compaction or missing-row fallback is added.

Sources: src/core/battle/battle_command.cpp, battle_state.cpp and
src/core/system/game_state.cpp; src/core/input/input_manager.cpp and bindings.cpp;
src/core/battle/battle_party.cpp. All six use this same native phase/input path.

## Verification and remaining gameplay

Extended actual-source fixtures: battle_context_check replays Battle13/Return14
for every game and verifies release before the native original; battle_focus_check
feeds late commands/targets and queued cancel callbacks, preserving originals and
positive live/new-battle speech. transition_input_check uses actual input manager,
catalog and dispatch for nine same-label keys, repeat registration and independent
slot1 remap. party_esper_check invokes actual Battle.Party registrations against
four rendered rows for every game, rejects absent/hidden slots, preserves slot4
when slot2 is hidden, and checks native calls stay on their owner thread.
The first canonical run caught a second real lifecycle gap: on_battle_start
could reopen Return14 (six failing cases); this is now guarded at that producer.
The new native-original count check also exposed a no-op fixture original; the
fixture now counts SetCommandData calls rather than treating it as an exception.
Final canonical rebuild and every offline fixture pass, including the native
original-call count. Name resolution7994/0 missing and action wiring137/344/0
undefined pass. Smoke loads/forwards all47 exports. All49 data and18 sound
resources match current sources. No additional native or audible latency was
measured; existing fake-boundary timings are not game performance measurements.

After installation, when convenient in battle, press1,2,3,4: hear each respective
party member once. Cancel a spell/item target: hear the focused command normally.
Return to title via pause: hear confirmation/title UI, without Attack during load.
No gameplay confirmation or native/audible timing measurement is claimed.

Evidence/recovery: build_check/battle_title_party_20261025/ has read-only snapshots
ff3_current_log.txt and ff3_previous_log.txt; verification.json and archived reports record the final build/smoke/resource and
source hashes. DLL10949632 bytes, SHA256
ca11df243fda6fb1b90ae5405974080e6ed7facfcc7cf0783f7d4d9495ed56bd.
security_final_predeploy/ records the enabled Defender scan of22 exact staged
files: no threats/zero matching detections, unchanged hashes. Signing certificate
count0 and exclusion-query verification false remain unchanged limitations.
Guarded deploy installed all six without a running-game refusal. deploy_after.json
and installed_payloads_verified.json verify132 matching installed payload hashes
and144 protected files unchanged, including dependency/configuration bytes and
timestamps. Source hashes remain stable. Recovery is retained under recovery/.
No protection change, external upload, signing or release operation occurred.
