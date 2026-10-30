# Boost and Draw Attacks displayed feedback

Human-reported acceptance failure October28: generic actor-wide cues are not
meaningful and repeat on sprite/aura changes. This Closed producer is explicitly
reopened by the user in [target-result follow-up](ABILITY_FEEDBACK_TARGETS_20261028.md).
Do not treat an EffectEntity.Init lifecycle as a new action/result. Prior fixture
coverage did not exercise persistent aura reinitialization after the action.

Human requested completion of both documented reports. Source complete, Closed
October28; combined FFIII deployment pending. Estimate45-75minutes through exact native/master
investigation, bounded implementation, offline checks and docs. Sol only. Return-exit
listing source is complete/Closed, combined delivery pending; Saronia Events is
installed/Closed. Vehicle tracking is the next separate checkpoint.

Scope: speech for actual displayed text or success indicators, preserving misses
and native timing. No hidden charge counts, targeting bookkeeping, formulas or
unsupported result text. Both commands use the same native add-condition/result
pipeline after Boost's random child action, so investigate this one cohesive
producer system together. Runtime game control/probes remain unauthorized.

Verified FFIII master facts: Boost command uses ability94/type306/group146,
function46 random table16, choosing children137/138/139/140. Successful charge
children137/139 use visual effect61 ef_ff0_focus and condition29/type421 (plus
unnamed condition30 on139). Draw Attacks command25/content346 uses ability96,
type304/group148, add-condition function9 with condition31/type308 and visual
effect153 ef_ff3_attract. Conditions29/30/31 have no message name, description,
icon or persistent condition effect. Thus generic condition naming deliberately
has nothing to announce. That does not yet settle whether the action itself
displays a distinct success cue. Native effect hit/miss behavior must be checked.

BattleConditionController.Add already reads confirmed conditions; an empty
native name/fallback remains silent. Rendered native Miss remains spoken by
the existing damage-view producer. Exact effect/result chain investigation uses
tools/re/map_location_native.py --ability-feedback; source-bearing native output
and binary/dump provenance are outside Git under parent
tools/ability_visible_feedback_20261028/ff3. No game snippets here.

Acceptance: identify actual text/visual producer, add speech only for proved
displayed results if needed, preserve misses/overboost native text, repeated
separate uses and lifecycle/cancellation, localize any new mod text in12 locales.
An audit finding no extra success display must explain that evidence rather
than fabricate success information. Native/frame/audible and gameplay remain
separate from static/offline evidence.

## Implemented producer and ownership

`src/core/battle/action_effects.cpp` registers Battle.ActionEffects109 for FFIII
only. The existing battle_message action callback copies the actor and identifies
eligible native ability94/137/138/139 or96, including fixed Command.AbilityId when
the action has no populated abilityList. This context alone never speaks success.
No unnamed condition label or hidden charge/targeting number is added.

Native EffectEntityController.Play (9A62C0) calls PlayTargets (9A5F60), whose
per-target HitType2 exclusion precedes PlayFixedEffect (9A56C0) and the loaded
prefab's EffectController.Play. PlayMagic bypasses that exclusion, but its sole
direct executable-section caller is BattleFunction40.IsFunctionEnd, unrelated
to these two commands. Native CreateEffectAsync's MoveNext (D11EF0) instantiates
the prefab and calls the single-target EffectEntity.Init, which wraps array Init.
Reviewed native exports/provenance remain outside Git; no native code here.

The two exact hooks are EffectEntity.Init's10-parameter array-target overload
(9A6EC0), selected by parameter1 EntityMeshRenderer[], and Update (9A7D80).
Init's ABI order is attack, target array, player array, background, quad, first,
reaction interface, last, enemy side, sound action, plus instance/MethodInfo.
Each detour calls its original exactly once with unchanged arguments. The inherited
myObject field (EffectEntityBase,0x30) is the instantiated effect object;
IsEnd (0x38) retires unseen completed effects. Name/field references resolve at
installation; missing required metadata fails closed.

Init captures only an eligible context on the native owner. The actual root is
named EffectPrefab, so it reviews at most two immediate children for the exact
Focus/Attract prefab markers. Real Focus has five enabled particle renderers;
Attract has two. It captures at most eight renderers and32 pending native effect
instances. Oversized/missing observations stay silent. The next native Update
that observes an active visible renderer consumes that individual effect event
and posts copied actor/result/locale/epoch to the speech worker. It says actor:
Boost takes effect or Draw Attacks takes effect, localized in all12 languages.
There is no mod renderer timer, recursive scene search or content deduplication.
A real new Init at the same address is a new event and may repeat equal words.

Existing native action/message pairing retains its two-frame window only when
that earlier phase is pending. BattleEntering/BattleEnd/SceneLoaded advance the
generation, clear bounded owner handles and cancel queued output. Workers hold
no native handles. A full background queue drops the cue without waiting/retry.
Unrelated actions skip prefab discovery; with no pending effects Update takes
only the owner/pending fast path. Existing Miss and overboost text remain owned
by their established native producers. No new controls/settings or defaults.

## Evidence and verification limits

`tools/re/action_effect_facts.py` verifies the exact overload/field contracts,
ability-effect uniqueness, actual prefab child markers and renderer counts.
Derived facts and bundle/dump/table hashes are stored outside Git at parent
tools/ability_visible_feedback_20261028/ff3/reviewed_facts.json. Only the original
verifier is in the repository. It neither runs nor changes the game.

`tools/action_effect_check.cpp` includes the actual producer and exercises its
detours, native Update readiness, repeated separate uses, fixed command fallback,
wrong/oversized observations, lifecycle cancellation, worker affinity, full queue,
capacity and existing action/message ordering. First fixture run:0 failures;
10,000 unmatched native updates with32 pending effects took0.377ms aggregate
in the fake owner harness. This is a bounded bookkeeping measurement, excluding
Unity renderer queries/allocations, speech queue/audible time and real frame cost.
No native responsiveness or25ms cumulative-lag certification is claimed.

Final canonical build/smoke pass:8303 names/0 missing,142/349/0 wiring,47 exports;
actual-source fixture0 failures,44 original Init/10019 Update calls,0 worker-native
accesses. Final10,000-update measurement0.286ms. DLL11089408 bytes SHA256
52de7af6a23392e526d07b905bab9b78d4dd3194657703a0740397d862e10e0c.
Receipts:build_check/action_effects_20261028/{final_build,final_smoke,final_report}.txt.
Combined FFIII delivery follows the remaining
vehicle-state and specific event-label checkpoints. Gameplay confirmation needs
separate Boost and Draw Attacks uses: each displayed success effect should follow
the command announcement once; misses should retain their current messages.
No game was launched or controlled. Safe extension requires a newly reviewed
visible prefab/producer, explicit bounds and the same lifecycle/owner contract;
an ability/condition ID by itself does not authorize output.
