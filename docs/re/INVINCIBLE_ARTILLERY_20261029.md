# Invincible opening artillery announcement

Source/offline/FFIII delivery Closed in75de3594. Combined build/smoke/native
names/resources/12 locales/exact22-file Defender scan and installed hashes pass;
144 protected/other5 installations unchanged. Human confirmed closed/deploy;
no game launched/probed. Exact receipt: build_check/ff3_oct29_followups/completion.json.
Audible confirmation remains pending; broader map audit stays open.

Human-scoped FFIII follow-up: the Invincible's weapons display “The artillery is
providing cover!” at the beginning of a random encounter. Keep its first read,
remove the repeated conclusion, preserve actual damage/misses and any distinct
result. Sol only; no game launch/control/probe or general battle refactor. Initial
estimate20–40minutes plus combined release checks. Status investigation complete,
source/offline checks complete. Installed FFIII is still ad0d80c1; combined
build/scan/delivery pending.

## Exact producer evidence

FFIII content399 is Ability141 with displayed name MSG_SYSTEM_023. Ability141's
process is INVINCIBLE, battle function group24/effect161. MessageConclusion508
maps BATTLE_HIT_ABILITY_141 to the same MSG_SYSTEM_023. This is a second producer
for the same artillery cue, not another displayed ability description. The own
FFIII log shows the action heading at437623.515 and the bare conclusion at
437626.234, generation2176; another occurrence repeats that2.719second separation.
The first heading also incorrectly prepends Unknown to this anonymous support.
No tester logs were searched; the FFV report remains a code-only investigation.

Source/native facts stay outside Git at
../tools/invincible_artillery_20261029/ff3/. Input DLL SHA256 is
8db9ab31a893ca4b13e5e4d2e2b1b086ac6e3fb5699b9be1d8037dafaaa3ae31;
matching dump cf74887b07f39807182c95701b3888ebfe171f0c9f7afbe8f33811b47c3d4b15.
Original extractor: map_location_native.py --invincible-artillery.
ActFunctionProvider.HitViewMessage38BC80 is static bool(Ability,BattleUnitData),
not a BattleActData receiver. It looks up the ability conclusion, writes the
single message ID and shows the panel; its alternate enemy lookup is only a
fallback when the ordinary row is absent. The actual141 row exists. ViewMessage
is38C1D0; InitActionStateA33420 and EndFinishingStateA32730 own action lifetime.

## Narrow correction and acceptance boundary

Reuse the already-read action ability ID to recognize141 and prepare a copied
anonymous opening heading. Suppress only the command-message observation inside
native HitViewMessage for ability141 when that opening cue was prepared. Native
UI/original call/return/exception behavior remains unchanged. No text comparison,
time window, renderer observation or global identical-message suppression.
Unknown/unprepared actions retain the native conclusion fallback. Reset receipt
at the next action and battle/scene replacement; retain no game pointer in it.

Registered-hook checks must cover KeyInput/Touch, early and late conclusions,
multiple callbacks, repeated battles/actions, missing initial description,
different ability with identical text, native returns/exceptions, ordinary damage
and misses, and all other games' abstention. Existing Boost/Guard/Draw and message
pairing checks remain required. One rare FFIII producer hook and bounded cached
metadata reads are the maximum new native work; no polling or worker wait.
Offline host timing is separate from unmeasured native/game-frame/audio timing.
Combined build/smoke/exact scan/FFIII guarded deployment remain pending.

## Implemented and checked

action_effects.cpp caches the141 classification from its existing action read.
battle_message.cpp's single new FFIII static-two-argument HitViewMessage detour
uses an owned prepared-cue boolean and the already-resolved Ability.get_Id.
Only the two shared KeyInput/Touch message observations abstain while this exact
producer runs; native calls always run once. The first cue has no Unknown actor.
message_line's existing delayed fallback now handles an empty actor consistently
with its immediate paired path. Damage/status/result producers are unchanged.
The receipt resets at action entry, BattleEntering, BattleEnd, SceneLoaded and
the existing battle-start reset. Native stack scope unwinds on exceptions.

tools/invincible_artillery_check.cpp runs the actual registered detour and action/
message consumers:12 before failures become0. Boost child-use and shared all-six
damage/status checks remain0. Early/late, repeated callback, other ability with
the same text, missing initial cue, both native returns, exception unwind,
all three lifecycle boundaries and other-game abstention pass.1000 independent
whole action/conclusion episodes have no queued task backlog; fake native/UI/
speech median0.700us,p99 0.800us,max3.300us. These are host costs, not target-game
or audible measurements. Per outcome: no additional read unless a cue is prepared,
then one Ability.get_Id; one owned boolean, no unbounded history or polling.
Evidence: build_check/torna_world_20261029/artillery_before.txt,
artillery_final.txt, battle_after.txt and artillery_after_build.txt.
