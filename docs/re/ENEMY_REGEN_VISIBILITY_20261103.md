# Enemy Regen labels and native recovery numbers

The owner's actual FF1 logs show Vampire Lord Regen announcements before the
enemy-formation announcement and later recovery15HP; Trolls similarly announce
intrinsic Regen and recovery9HP. The current and five rotated game-directory
logs were opened with shared read/write/delete handles, with a maximum4MiB tail
per file. No tester logs or game runtime probe were used. Selected evidence and
source-tail hashes are in build_check/ff1_enemy_regen_20261103/actual_game_log_evidence.json.

The existing renderer-based enemy_status.cpp is identical in Main and the
source-qualified installed FF1 Stage. This is a second producer bypass, rather
than evidence that the earlier visual-effect implementation was not installed.
BattleConditionController.Add's shared postfix announced every named condition
except optional enemy Critical. Native condition-function removals had the same
hole. Both could expose hidden status names independently of the optional
EnemyStatusAnnouncements setting and actual rendering.

FF1 condition29/type25 is intrinsic Regeneration: icon0 and battle-effect0,
without a native name/description key. It provides no native status icon/effect
for the renderer-based observer. A raw condition identity cannot establish that
a sighted player can identify it. The fix applies to all raw enemy conditions,
across all six games, without a Regen name blacklist or text deduplication.

The raw Add and removal paths now reject enemy conditions. The separately
owner-approved EnemyCriticalStatus opt-in remains limited to type4, preserving
its default-Off behavior and the documented explicit approval in
[NPC patrol/target follow-up](NPC_PATROL_TARGETS_20261025.md#enemy-critical-health).
Party condition additions/removals retain their existing producer. Actual enemy
effects, changed sprites and changed materials remain owned by
[the renderer-backed observer](ENEMY_STATUS.md); a shared visual still says
Status effect rather than guessing an indistinguishable hidden name. No new
hook, native mutation, worker pointer or polling loop was introduced.

November 6 follow-up. The filter also suppressed enemy KO. The
[enemy KO correction](ENEMY_STATUS.md) restores ordinary defeat feedback on
Add while preserving the hidden Regen rule and removal policy.

Recovery amounts are a different native presentation. FF1's actual regeneration
routine explicitly requests a recovery damage view before applying the HP gain.
The full native chain is GenerateRegeneHeal60B9B0 → BattleUtility.CreateDamageView
8695E0 → DamageViewUIManager.CreateDamgeView4E3A60 → ViewDamageEntity.CreateDamageValue
612460. The manager instantiates the ordinary ViewDamage UI; recovery digits get
the native recovery material through SetColor612C10. EnemyPositionMove612930
positions enemy popups. The chain does not exclude enemies or suppress their
recovery digits. Thus recovery-number presentation is native game behavior,
even though the intrinsic Regen status label is hidden.

The mod retains those numbers: rendered_feedback.cpp waits for the actual
visible digit images and decodes their sprite glyphs; it does not speak the
hidden heal value, HP difference or condition rate. Ordinary damage, recovery,
Miss, HP/MP resource context and party conditions are preserved. Static tracing
and log evidence establish these producers; no screenshot or new gameplay
confirmation is claimed.

Matching FF1 binary/dump hashes and21 exact native function locators are in
../tools/enemy_render_20261103/ff1/provenance.json, outside every Git tree, beside
the recovered native analysis. tools/re/battle_feedback_native.py --enemy-render
reproduces that bounded extraction. The normalized condition row is in the local
checkpoint. No recovered implementation is included in this repository.

Verification uses the actual battle_message.cpp and all-six native adapters.
New cases cover intrinsic Regen, other statuses and unresolved fallback labels,
direct/bulk removals, both Critical preferences and unaffected party feedback.
Before the fix,216 assertions fail; the same cases pass after it. Existing actual
enemy-effect fixtures cover hidden/delayed/unique/shared renderers, sprite/material
changes, cancellation and worker ownership. Actual rendered-digit checks and
Scan/Boost/Invincible result integrations are included. Exact final reports,
source identities, finite resource receipts and remaining delivery limits are
in build_check/ff1_enemy_regen_20261103/completion.json once qualification ends.

No native DLL, deployment or gameplay confirmation is part of this focused
checkpoint. Final combined build, smoke checks, exact-payload security scans
and guarded delivery remain required before these source changes are installed.

November3 delivery update: this producer/fixture correction and the initial-item
target fix are now built/scanned and installed only in FF1 at9c2ba647d1e5408a.
Native visible recovery digits remain audible. The other five installed games
are unchanged; all-six source correction awaits the final audit candidate.
Exact source/scan/recovery/installed evidence and remaining natural confirmation:
[scoped FF1 delivery](ITEM_TARGET_ENTRY_20261103.md#november3-scoped-ff1-delivery).
