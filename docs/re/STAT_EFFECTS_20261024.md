# Applied battle statistic changes, all six games

October25 [visibility correction](SAVE_CONTEXT_VISIBILITY_20261025.md): raw Luck
changes from critical-health/KO bookkeeping are excluded in all six games; dead
target and Dying/KO/Stone cleanup no longer produces invented stat feedback.
Applied visible buffs/Imbibe/Swap retain their effects. Full offline checks pass;
the newer combined delivery is tracked in that record.

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation/offline verification complete. No game launch/control or runtime probe.
The user expanded the Imbibe request to all status-modification spells that
directly modify statistics, across FF1–VI. This scope permits their applied
statistic/direction descriptions; it does not expose hidden totals or unrelated
enemy parameters. The earlier Roundworm/H and Imbibe fixes remain included.

## Native sources and application paths

Matching all-six native/disassembly receipts are outside Git in
../tools/stat_effects_20261024/ff1..ff6/{native.txt,provenance.json}, produced by
tools/re/battle_feedback_native.py --stat-effects. Existing battle-feedback
receipts establish BattleStatusControl.UpdateTempParamter's unit ABI and callers.
Every game's condition Add invokes that unit refresh. Field names resolve
against each installed game's dump; no cross-game numeric offsets are copied.

Conditions can carry ready signed parameterGroupDic entries, including unnamed
buffs/debuffs and later-turn increments. BattleTempParameter.UpdateParameter
merges those contributions. Its aggregate dictionary uses master parameter IDs:
1..25 in all six, with FFII26..28 for additional counts/rates. GroupMasterUtility
maps vertical2 to a negative value; native periodic functions can also update
contributions directly. A named condition alone is not evidence of a numerical
change. Haste/Protect and other formula/behavior statuses retain their existing
native condition speech when no direct numerical modifier changes.

Separate direct functions (TempParameterFixed/Range and persistence variants)
call BattleUtility.ChangeTemporaryParameter. Its int-master-ID overload resolves
the native ParameterType overload. The latter performs the actual write, so it
is the single observed direct producer. Native HP/MP cases change current
resources; they are excluded and remain in rendered damage/recovery speech.
FF4's coefficient-physical function is HP damage, not a statistic boost.

## Implementation, order and ownership

src/core/battle/stat_feedback.cpp/.h owns the feature. BattleStatusControl's
unit refresh is observed in five adapters. FFII reuses its existing
battle_message.cpp stat-refresh detour, avoiding a second hook on the same body.
Before/after snapshots read the rooted current native modifier dictionary and
54 verified base/temporary scalar fields;
an owner/parameter replacement invalidates the pair. Numerical differences here
are exact native modifier changes, never HP/damage arithmetic or spell-name
inference. Individual direct writes read the matching native temporary field
before/after their own original operation; native no-ops remain silent.

BattleConditionController.Add has a stack-only scope. Nested native refreshes
accumulate its changes, then condition_added supplies the existing condition
name to one combined result. Unnamed applied modifiers can still describe their
statistics. Imbibe type203 selects the structured result instead of repeating
its already descriptive fallback; this is semantic operation ownership, not
text-content deduplication. Later native refreshes/direct changes can announce
outside Add, so continuing effects do not require another action-name event.
Same-state recalculations produce no change event.

Native callbacks retain29 scalar slots and reject dictionaries above64
entries before iteration. Current HP/MP writes and unsupported master slots stay
excluded; actual maximum HP/MP statistic changes remain described as Maximum HP/MP.
Up to64 parameter/name bindings warm through native unit refreshes and the actual
action actor/target array (at most64). Identities are comparison-only and never
dereferenced later. InitActionState binds even direct-only effects;
EndFinishingState observes before native teardown to catch inlined/unique base
statistic writes. Previously observed typed writes update the cache so completion
does not repeat them. Names are bounded to256 units. BattleEntering only changes flags/epoch;
safe BattleEnd/Title cleanup retires storage. Battle-end recovery suppresses
restoration noise. Pointer replacement, cleanup and epoch changes cancel stale
output. Metadata offsets prewarm during install. No new polling, scene scan,
native parameter write, blocking wait or worker-held native pointer is added.

The Data worker receives only copied target/condition/locale, game ID and a
29-element signed change array. The existing worker_job owner retries a busy Data
lane through mod-only frame admission, formats localized stat up/down labels, and
publishes on its main-owned continuation. Up to64 owned pending results retain
occurrence order; overload counts rejections and the next formatted result
announces the localized notice that some changes could not be read. Cleanup
cancels queued epochs. Native callbacks never wait; there is no unbounded backlog. Real native/frame/audible timing
and overload delivery remain unmeasured; offline fake-output timing is separate.

## Verification and localization

battle_feedback_check exercises the actual native detours and captured data in
all six adapters, including FFII's shared refresh entry: named buff plus direction,
unnamed Imbibe, direct mutation/no-op, untouched HP/MP feedback, modifier removal,
later increments, unchanged refresh, replaced ownership, oversized dictionaries,
paused formatter, cleanup/cancellation and whole capture-to-fake-output timing.
Its first run exposed stale synthetic Protect removal state in the later Imbibe
scenario. That earlier Add-history tracker is now replaced by the separate
native removal correction. The fixture is isolated; the failure remains in the evidence
archive. No production speech guard was added to hide it.

Luna (gpt-6-luna, medium) performed only the bounded13-key/12-language catalog
addition. Sol reviewed preservation, labels, placeholders and syntax; existing
entries are unchanged. Native-language listening remains unconfirmed. The shared
catalog has localized stat labels/templates, and per-game terminology continues
through the existing T/LocaleScope path. condition_labels_check checks every
new value and the Imbibe/master chain. No control, setting or default changes.

Evidence/recovery: ignored build_check/stat_effects_20261024/. Canonical build and payload scan pass; all six games are installed. Player-side confirmation remains pending.

## Further native coverage and offline cost

Verified unit refresh / typed write / aggregate refresh RVAs, respectively:
I865380/8688E0/866F10; II90C4C0/90FB20/90DB70; III6E98C0/6ED3E0/6EB8F0;
IV5B7C00/5BB800/5B9E00; VA17450/99CEC0/A193E0; VI919850/91DC70/91B540.
VI Dischord at56A6E7 calls the same typed writer at56A787. FFV ongoing
TakenSongConditionFunction.UpdateValue at476150 calls the typed writer at
47622E/4762C5/47635C/4763EF. These cover unique base changes and later song ticks
without added polling. The all-six master audit covers170 actual parameter-group
rows,54 fields per binary and24 catalog keys/288 localized values.

A pre-final fake-native fixture measured unchanged action binding/capture plus
completion at median11.6us/p9942.3us/max61.5us, and26 changed modifiers plus
worker/main/fake output at23.3/54.2/88.3us. The old first label misleadingly said
first fake speech despite its unchanged-state case: it produces no speech. These
are offline fixture costs only, excluding real native operations, frame cadence,
SDK/audio delivery and audible completion. Current combined reports supersede
these retained pre-final measurements. Busy-admission,64-result overload and
cleanup cancellation have explicit fixtures; actual runtime tail delays remain
unmeasured.
