# Status removal feedback, all six games

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation complete; combined build/checks pass. Gameplay listening
is unconfirmed. The user's FFII log at26192.203 has Ricard using Antidote with
no following removal. Persistent Poison can enter a battle without an Add event;
the previous FFII tracker only remembered Add announcements and reset at entry.

## Native contract and source

The shared implementation in battle/battle_message.cpp now observes both native
BattleConditionController.RemoveFunction(unit,id) and the bulk
RemoveConditionFunction(unit,conditionCounts,functionCounts). All six signatures
match. Native CheckConditionFunction reconciles the current confirmed condition
counts with the old function counts, including conflicts, cures, wear-off and
interrupt removals. Bulk removal may inline the individual operation; hooking
only its standalone body misses that route. RecoveryConditionFunction and
RecoveryConditionMultiFunction edit the parameter list before reconciliation.
The final removal observation therefore uses old functions and resulting native
conditions, independently of any Add speech in the present battle.

| Game | CheckConditionFunction | Direct removal | Bulk removal |
|---|---|---|---|
| I |3DAD70|3DD080|3DCC70|
| II |6B86B0|6BAA20|6BA610|
| III |3A99F0|3ABE20|3ABB40|
| IV |35DB10|3602A0|35FBC0|
| V |3306D0|333260|332F80|
| VI |3ADBB0|3B08C0|3B05E0|

Values are matching installed-binary RVAs, not runtime hardcoded offsets. The
name-resolved hooks use each game's own metadata. Extractor:
tools/re/battle_feedback_native.py --removals. Source-bearing disassembly and
hash receipts remain outside Git at ../tools/status_removals_20261024/ff1..ff6/.

## Lifetime, output and bounds

Prefix capture copies at most128 active function identities/IDs/types on this
synchronous native stack. Immutable master Condition pointers remain rooted by
the master manager throughout the original call and are never retained later.
Postfix compares at most128 remaining functions and128 raw current conditions;
replaced parameters or invalid/oversized lists invalidate the observation.
Only the last actual disappearance of a named condition speaks. Stacked,
permanent or unchanged conditions do not produce removal speech. Unit/name
resolution happens only for actual removals; condition names retain256 units.
No worker receives pointers, and no native data is changed or awaited.

The existing unit/condition/frame occurrence guard is preserved across nested
direct/bulk paths; this is not text deduplication. Existing battle-end and
KO/Stone cleanup silence remains (September24 user decision). A genuine revive
can announce KO removed. Hidden/internal conditions retain their existing name
eligibility. Existing localized target/status/removed templates cover all12
languages. FFII persistent Poison/type204 remains distinct from temporary
Venom/type10. No control, setting or default changes.

The old per-game predicate/prefix/Add-history removal paths are replaced, rather
than layered with the new producer. FFII retains its one stat-refresh adapter
for statistic feedback. BattleStart resets cleanup/event guards, and battle-end
recovery retains the earlier suppression boundary.

## Verification limits

battle_feedback_check executes the actual shared direct and bulk detours for
all six adapters: carried Poison without Add history, native no-op, remaining
stack/function, current condition still applied, battle cleanup and over-bound
storage. Native binary receipts establish reconciliation's callers and order;
the fake mutator models external native list edits and does not execute game
code. Menu readers continue reading current rendered condition icons; this
record's removal-event correction concerns the battle reconciliation paths.
Evidence/recovery: build_check/stat_effects_20261024/. Final build, payload scan,
deployment and natural Antidote/Esuna/expiry/revive listening remain pending.
