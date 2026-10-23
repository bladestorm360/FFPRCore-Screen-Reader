# Visible enemy statuses and optional speech, October 22

Implementation complete; default-Off **Enemy status announcements** is in the mod
menu's Speech submenu, persisted as EnemyStatusAnnouncements. All12 UI languages
are covered. Offline presentation/lifetime fixtures pass; gameplay confirmation is
pending. Exact Critical and other appearance evidence is distinguished below.

## Complete master inventory and native producers

`enemy_status_audit.py` inventories all525 condition rows: FF1/2/3/4/5/6 have
32/158/37/88/108/102 rows and12/15/11/14/18/15 nonzero effect assets. Per-game
ffN_condition_visual_audit.json records each type, message key, asset and target type;
data/ffN/condition_visuals.json is the conservative runtime uniqueness policy.
Asset metadata identifies candidates, never proves an enemy visually displays them.

Native BattleConditionFunction.Init supplies the actual BattleEnemyData owner and
condition identity. Its sprite's ConditionEffectEntityController binds that enemy.
Actual ConditionEffectEntityController.Play creates the visual event. Repeated Play
for an existing type creates nothing and produces no extra announcement. The native
StartConditionEffect async continuation passes BattleEffectAsset.Id to Play in all
six installed binaries; exact argument windows remain outside Git.

| Game | Play RVA | Native parameters after self |
|---|---|---|
| I | 577740 | prefab, effect ID, condition type, isEnemy |
| II | 47FD70 | prefab, effect ID, condition type, isEnemy |
| III | 4BFC60 | prefab, effect ID, condition type |
| IV | A14800 | prefab, effect ID, condition type, isCopyEffect |
| V | 534E20 | prefab, effect ID, condition type, isCopyEffect |
| VI | 4A9DC0 | prefab, effect ID, condition type, isCopyEffect |

Typed FF3 detours preserve its shorter ABI. IV–VI's Boolean is not enemy identity;
all adapters use the Init-bound BattleEnemyData owner. Both differences were found
and corrected by Sol's static/native checks before delivery.

## What may be spoken

A new native effect must actually have an enabled, on-screen, nontransparent renderer,
or a native PlayShader target renderer that meets those checks. Renderer references
are captured once from the effect, including inactive children for delayed activation;
waiting frames never repeat a hierarchy search. A unique effect asset can use the
actual localized condition name. Shared or unknown assets say Status effect rather
than disclose an indistinguishable hidden exact condition. The enemy name and label
are copied to a formatting worker; publication rechecks generation, setting, effect
lifetime/type and visible renderer. Speech queues instead of interrupting.

Critical/Dying type4 has effect0 in every game. DyingConditionFunction.Start calls
AnimDying, but BattleEnemyAnimator and BattleEnemyAnimatorBase do not override Dying
or UpdateDying. Their inherited BattleAnimatorBase methods are return-only in all
six installed binaries (RVA2698F0/2698F0/26D8F0/26B8F0/2715A0/2F10A0).
Player animators do override those methods; player posture is not enemy evidence.
Therefore no enemy Critical cue is inferred from low HP or the hidden condition.
The enemy-HP exception does not authorize undisplayed enemy statuses.

Zero effect-asset IDs do not imply no visible condition. Frog/Child/Pig handlers
call the native enemy ChangeMotionListFromCondition path, which selects
InstantiateManager.InitEnemyEntityFromCondition. That method resolves the native
condition monster asset and starts InitEnemyEntity. The eventual BattleSpriteEntity
.SetSprite callback is the completed producer. The observer associates that request
with its actual enemy binding, accepts only a changed assigned sprite, then waits
for the body's real mesh renderer to be visible. Native base-asset restoration or
a superseding condition increments a revision and cancels obsolete speech. No
condition-list scan or artificial sprite change is performed.

ChangeSkinColorConditionFunctionBase.Start is also observed. Only a new native
changedSkinColor transition with an actual bound enemy effectRender qualifies.
Skin palettes lack a universally unique status mapping, so this path says Status
effect. Native Remove and superseding material changes retire the witness. Only
Unity renderer handles and a native-owned binding revision survive Start; no raw
managed condition-function pointer is retained across frames. All native originals
and their parameters remain unchanged. Independent native visual events can each
be spoken; no blanket condition/text-content suppression was added.

[All-six factual native receipt](enemy_presentation_native.json) records matching
binary/dump hashes, producer RVAs, actual parameter counts and return-only Critical
methods. Additional exact call/disassembly analysis stays outside Git in
status_appearance_calls.txt and condition_arguments_all6.txt. Effects outside the
known icon/shader, sprite-replacement and native skin-material producers remain a
verification limit; hidden or unsupported identities are not guessed.

## Bounds, cancellation and extension

`battle/enemy_status.cpp` uses64 current enemy bindings,64 condition labels/binding,
256 units/name, at most64 pending visible/formatting publications and64 captured
renderers/effect. Full capacity rejects the new observation without blocking native
processing. BattleEntering retires by scalar generation/worker epoch; safe BattleEnd/
Title boundaries cancel callbacks. Initial effects survive BattleStart. Native reads
remain on the owner; workers receive only copied enemy/label/locale strings.
No condition-list scan, HP-derived inference or blanket text deduplication exists.

native_cues_check uses real all-six policies and actual observers with hidden/delayed
renderers, native repeat guards, shared visuals, Critical-without-effect, settings,
zero-asset sprite replacement, unchanged sprites, restoration, material Start/Remove
and parked-worker cancellation. Source/ABI names are checked in all six dumps.
Fixtures do not establish audible latency, real renderer timing or every condition's
appearance in gameplay. See [TESTING](../TESTING.md#october-22-fixes-test-these-first).
Add another visual producer only with exact native presentation/owner evidence;
never read a hidden parameter merely because a status enum exists.

The final [serialized prefab bound audit](status_prefab_bounds.json) resolves all85 condition-effect
bundles. Maximum renderer counts are10/10/12/12/36/22 for I–VI. The observer
limit was raised from32 to64 after finding the real FF5 case, and its actual-size
fixture was added. This bounds the known prefab traversal; it does not measure
native allocation, hierarchy-search cost or actual game-frame aggregate time.
