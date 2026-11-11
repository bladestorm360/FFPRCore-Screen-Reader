# Enemy defeat and confirmed status transformations

November 6 KO correction. The FFIV game log at 66897.437 shows Gatlinger's
final damage, then battle results at 66899.109 without an enemy KO line.
Porom's KO in the same battle was spoken. The November 3 raw-status filter
also excluded enemy KO from the existing condition Add path.

KO is ordinary battle feedback in all six games. The combined correction
admits the audited UnableFight IDs independently of the visual enemy status
setting and verifies the resolved condition type before speaking. Consecutive
applied KOs can have identical text. No last-announcement or time check is
used. Other raw enemy conditions, including Critical, are excluded before
condition reads. The former hidden Critical opt-in is superseded. Party
Critical and rendered critical-hit text retain their producers.

The targeted fixture uses all ten KO master rows, both Critical preferences,
and the FFV Add signature. It replays two different enemies with the same
name and tests the second callback with only a master identity. Twenty cases
failed before the change and all pass after it. The combined fixture also
passes the existing hidden Regen and party-status cases. Evidence is under
`build_check/enemy_ko_20261106/`.

## November 6 renderer recheck

The installed all-six GameAssembly binaries match the retained native
hashes. The recheck covered all 525 condition master rows and inspected
85 installed effect bundles. There are 252 rows with a dedicated effect
asset and 273 without one. The asset counts describe native visuals. The
owner limited status speech to confirmed enemy replacement sprites and KO.
Particle effects and material tints remain game visuals.

| Game | Poison or Venom particle asset | Sleep asset | Paralysis asset | Dedicated enemy replacement art |
| --- | --- | --- | --- | --- |
| I | 87 | 86 | 84 | None of the requested forms |
| II | 51 and 378 | 50 | 48 | Toad 129 |
| III | 40 | 39 | 37 | No Mini or Toad replacement found |
| IV | 51 | 50 | 48 | Mini 26, Toad 131, Pig 237 |
| V | 43 | 42 | 297 | Mini 296, Toad 297 |
| VI | 69 | 236 | No type 8 master row | Imp 352 |

The last column lists monster asset IDs rather than battle effect IDs.
The condition-specific native loader has one direct caller in each binary.
That caller selects a form from the current condition list. The loader
uses the native type-to-art dictionary, a game constant and the installed
monster asset. Generic enemy sprite loads, including ordinary boss phase
changes, do not establish a condition rendering event. FFII's Mini and
FFIII's Toad handlers have explicit enemy branches that request KO
(FFII RVA 9DD670 and FFIII RVA 9B0A90). Enemy immunities and actual
rendering still apply. A native class name or shared effect alone cannot
qualify a specific status label.

FFIV logs at 67559.093 and 67562.562 record paired Gatlinger Attack up and
Attack down immediately before Needle. The speech came from comparison of
temporary combat fields. Native parameter refresh at 5B7C00 and the typed
write at 5BB800 update combat values. Those producers supply no visible
Attack up/down cue. The combined change limits modifier capture, direct
parameter comparisons and action-completion comparison to party units.
Native writes still run. The optional enemy transformation path and
rendered damage/recovery numbers remain separate.

The staged combined fixture reproduced 42 hidden-Critical or enemy-stat
failures before the visibility correction and passed afterward. It
checks raw Add, removal, native status refresh, paired Attack writes,
action completion and party preservation in all six adapters. The
existing native renderer fixture also passes. Evidence is under
`build_check/enemy_ko_20261106/` and the matching native analysis is outside
Git under `../tools/enemy_visual_review_20261106_extended/`. This is
offline evidence. It does not confirm every effect in gameplay.

November3 [Regen/raw-producer correction](ENEMY_REGEN_VISIBILITY_20261103.md):
actual FF1 Vampire Lord/Troll logs exposed a separate raw Add/removal bypass.
The November 6 combined correction permits ordinary KO feedback through
that shared path. The sprite observer owns confirmed transformations.
Native recovery-number popups remain readable from actual displayed digits.

The battle build passed the full offline checks, isolated smoke test and
exact 22-file Defender scan. The installed files in I, II, III, IV and VI
matched the scanned hashes. FFV installation is on hold by owner request.
The deployment receipt is `build_check/enemy_ko_20261106/deploy_after.json`.

October26 [all-six danger animation audit](ALLY_STATUS_20261026.md): generic enemy
animators inherit empty Dying/UpdateDying methods in all six builds; players have
their own dying pose. No generic enemy low-health pose is available as a visible
Critical witness. Unique scripted boss changes remain separate. The prior
hidden EnemyCriticalStatus preference is retired by the November 6 correction.

## Optional transformation speech

**Enemy status announcements** remains in Speech, default off. It controls
confirmed transformations only. KO has its own ordinary battle producer.
The hidden Critical menu option was removed. Its legacy saved value has
no effect on speech. The revised setting help covers all 12 languages.

The observer uses the actual BattleEnemyData owner and records condition
labels only for the seven verified game-and-form pairs. In each binary,
InitEnemyEntityFromCondition selects the asset and synchronously calls
InitEnemyEntity. The latter calls BattleSpriteEntity.SetSprite before it
returns. Speech eligibility requires that exact nested call, matching
enemy and verified asset ID, a changed assigned sprite, and visibility of
the actual body mesh. Bare SetSprite calls, different art, ordinary
asset loads and boss phases cannot establish a condition. Native
restoration or a new load retires a queued old transformation. The
localized native status name is preferred, with the existing localized
form label as fallback.

The former particle and material observers were removed. The historical
effect inventory in ffN_condition_visual_audit.json and condition_visuals.json
documents the audited assets and uniqueness, independent of speech. The
actual native rendering and gameplay behavior remain game-owned. This
change leaves damage and recovery speech on their existing rendered-digit
path.

The sprite observer retains 64 enemy bindings, up to three labels per
binding, 256 characters per copied name or label, and at most 64 pending
publications. It uses one body renderer per accepted sprite change.
Generation and appearance revision protect pending results. Only the
owner continuation touches Unity handles. The worker joins owned names
and labels. This change removes three hook registrations and adds no
native hook or wait. Existing transition and battle lifecycle retirement
are reused.

The isolated production fixture tests all seven confirmed forms and all
six adapters. Hidden, unchanged, mismatched and out-of-scope sprites,
ordinary enemy loads, disabled speech and stale worker results stay
silent. Enemy KO, party feedback and the hidden enemy modifier regression
use the separate battle feedback fixture. The offline checks do not
measure native rendering, audible delivery or gameplay timing.

[All-six native presentation locators](enemy_presentation_native.json)
and the November 6 receipts link the exact binary revisions and
condition-to-art chain. The raw native analysis is outside Git. Exact
sprite and stat fixture output is under
`build_check/enemy_ko_20261106/visual_stage/`.
