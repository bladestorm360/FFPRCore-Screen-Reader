# Ally battle danger and status review, October26

October29 scoped human follow-up supersedes the presentation-only status source:
[current ally conditions](BATTLE_LIVE_STATUS_20261029.md). H, number-row and ally
targeting now read current native ally conditions; stale rotating text/icons and
animation visibility no longer own status presence. The two old observers/caches
are removed. Critical retains the native red-HP equivalent. The implementation
and release facts below describe the historical October26 checkpoint.

Status: implementation complete, offline verified and installed/hash-verified in
all six games. Gameplay/audio confirmation remains pending. No game was launched,
controlled or probed. The user's latest correction is authoritative: Mini was
confirmed in targeting, **not H**. Poison has not been gameplay-tested by the user.

## Visible evidence and all-six findings

The FFIII screenshots show Rae21/297 with red current HP and a crouched body.
Critical is a spoken equivalent of those cues; no written Critical label is shown.
The normal command HUD shows selected commands rather than condition icons.
Robyn and Ryco's small bodies remain visible outside targeting and while another
member chooses a command. The targeting panel also shows Mini icons. A queued
command named Mini is never used as proof of a condition.

All six native player animators implement a dying pose. Generic enemy animators
inherit empty Dying/UpdateDying methods from BattleAnimatorBase: no standard enemy
low-HP danger pose was found. Particular bosses may have scripted transformations.
EnemyCriticalStatus remains an independent default-off disclosure.

The audit covers525 condition master rows and85 referenced effect bundles:

| Game | Conditions | Effect bundles | Supported forms | Paralysis type8 |
| --- | ---: | ---: | --- | --- |
| I | 32 | 12 | none | yes |
| II | 158 | 15 | Mini, Toad | yes |
| III | 37 | 11 | Mini, Toad | yes |
| IV | 88 | 14 | Mini, Toad, Pig | yes |
| V | 108 | 18 | Mini, Toad | yes |
| VI | 102 | 15 | Imp | absent from master |

Poison has applied skin/material tint and/or a looping body effect. FFII temporary
battle type10 is Venom; persistent204 remains Poison, following existing corrected
names. Paralysis has immobilized animation and a visible effect where supported.
Sleep, Blind, Silence, confusion and stone conditions have native pose, tint or
effect paths. Mini/Toad/Pig/Imp replace body motions. Protect/Shell/Haste/Slow can
use body shader reactions rather than particle sprites. Not every visible change
replaces a sprite sheet, and a shared handler class alone does not establish that
a game supports that condition.

All Critical rows have icon0/effect0. I-V use system4; VI uses system1 and can also
render native condition text. Native HP formatting uses red current HP for
Critical, gray values for KO and white otherwise. No mod-created HP threshold is
needed. Existing automatic status add/remove and native icon producers remain.

## Implementation and ownership

battle/battle_party.cpp::read_row is shared by H, party-number/controller review
and ally targeting. Existing visible native text/icons remain the first producer.
New battle/party_status.cpp::extra_lines adds the rendered HP color equivalent.
When the native condition column is absent, it also observes the same visible
ally body's completed form, applied tint and active effects. H remains status-only;
targeting retains the separate command column. MP-only I/Autodetail ownership stays.

Battle.PartyStatus installs at109 before Battle.Party110:

* After synchronous InstantiateManager.ChangePlayerMotions completes, retain the
  entity/motion-dictionary identity and form type. A failed load cannot certify a
  new form. Compare the current dictionary; never dereference stored dictionaries.
* After BattleConditionController.SetConditionText, retain controller, text-field
  identity and condition type. VI's visible native Critical text owns its label;
  hidden text yields to red HP. This is producer ownership, not speech-text dedup.
* Clear fixed tables on BattleEntering, BattleEnd and Title. BattleStart preserves
  forms loaded during the same entry sequence.

HP reads copy existing TextColors Red/Gray Color32 fields and Graphic color.
Visible gray zero becomes KO; red becomes Critical. Hidden/transparent HP,
uninitialized palette or low values in white cannot produce Critical.

Body evidence requires the bound BattlePlayerData's BattlePlayerEntity, live
character sprite and visible mesh. Native condition-function masters supply
name/identity only after a presentation witness is established:

* Supported form type and completed/current motion dictionary.
* ChangeSkinColorConditionFunctionBase.changedSkinColor and its visible renderer.
* Matching ConditionEffectEntity with visible renderer, or playing particles
  whose renderer is visible.
* Started, unfinished EffectReactionInspector of reviewed body-material types
  2/3/9/10/13/25, whose currentReaction.entityMeshRenderers includes this exact
  visible body mesh. Effect ID or raw condition presence alone is insufficient.
* VI Silence68 is a verified three-object EffectPrefab with one child sprite
  renderer and no particles. Only this exact game/type/asset permits one local
  child-renderer lookup; no general hierarchy/scene scan is added.

Native colors, shaders, forms and lists are never changed. Stacked functions with
one native condition identity contribute one body entry; localized speech text
is never compared or deduplicated. Existing local event guards remain. Names use
native localized labels and the existing12-language Critical/KO catalog.
No new setting or control is introduced.

## Bounds, threads and performance

Observation and ready-object reads run on the existing native owner thread;
no worker pointer transfer, timer, task wait or game-thread blocking. Tables hold
16 forms/32 text identities. Per request:128 functions,64 effect entries and64
total renderer/particle/reaction-target work units. Particle arrays above64 and
reaction targets above9 fail closed. The largest audited prefab has25 objects/
18 particles (V), below the per-array bound. Overload may omit unproven body
conditions; existing displayed text/icons and HP remain readable.

Actual H formatting/dispatch fixture,1000 requests: text baseline5.782ms aggregate/
232.000us peak/137 fake-native calls per request; visible Mini9.090ms/44.700us/
173 calls;128 functions+64 effect entries with budget64:63.661ms/88.100us/
1065 calls. These include formatting and fake speech dispatch, not real Unity,
native callback/frame tails, screen-reader queues or audible latency. No real
responsiveness claim is inferred from harness speed or worker placement.

## Evidence and safe extension

Original tools/re/map_location_native.py exports build-matched functions and
DLL/dump hashes outside all Git trees: parent tools directories
ally_status_20261026, ally_body_status_20261026, party_status_visuals_20261026,
party_effect_readers_20261026 and party_reaction_readers_20261026, each ff1..ff6.
No game source/disassembly is stored here. FFIII anchors: SettingCondition0x6A5120,
ConditionEffectEntityController.Play0x4BFC60, EffectReactionInspector.Init0xD04130.
The six generic base danger methods begin with return at0x2698F0/2698F0/26D8F0/
26B8F0/2715A0/2F10A0.

tools/re/party_status_assets.py repeats the read-only525-row/85-bundle audit using
existing masters and installed assets. Output contains counts, IDs, hashes and
topology only. verified_asset_topology.json supersedes initial exploratory
effect_prefab_facts/effect_reaction_types reports in build_check/ally_status_20261026/.
Zero-renderer shader prefabs are not treated as invisible: native runtime reactions
target the body mesh.

Actual-source party_esper tests cover repeated H, numbers, targets, red/gray/white/
hidden HP, VI text ownership, completed/restored/hidden forms, lifecycle retirement,
applied/unapplied poison tint, visible/hidden/stopped particles, shader start/end/
foreign mesh, VI Silence bubble and bounds. The generic VI particle fixture uses
a synthetic Paralysis label to exercise shared plumbing; this does not establish
VI supports Paralysis8. Existing battle_feedback and battle_results tests retain
all-six add/remove and actual visible-icon coverage. Isolated equipment_party uses
a service stub; party-wrapper fixtures share production party_status.

Canonical8191 native names/0 missing,137 used/344 defined/0 undefined wiring,
complete regressions and47-export isolated smoke pass. Two fixture integration
failures (namespace closure, missing child-renderer stub) and an internal metadata
catalog-check failure were corrected and preserved. Failed builds were not installed.
Extend only with exact native presentation evidence and hidden/stale/overload tests,
not a new raw internal-condition inventory.

## Delivery and gameplay limits

Combined payload includes [world events](WORLD_EVENTS_20261026.md) and the earlier
[shop correction](SHOP_COMPARISON_20261026.md). DLL10959872 bytes, SHA256
b88ebc08a531a1bb841aa99942da0ba54d85750fb0c49a5fbf7d8bfcd669cf74.
All49 data and18 sound resources match source. Defender1.461.211.0 reported no
threats, exit0/no matching detections/all22 staged hashes unchanged. Existing
unsigned/no signing certificate and protected exclusion-query limits remain;
outer-package/download reputation was not tested. No protection or dependency
change, upload, release, commit/push or delegation occurred.

After the user closed FFIII, all six canonical deployments succeeded. Every one
of132 installed payload hashes matches the scan stage;144 protected files match
the fresh predeployment baseline. Logs, recovery, exact scan, verification and
deploy_after.json reside in build_check/ally_status_20261026/. User-confirmed
Mini targeting and prior arrow behavior are distinct from unconfirmed new H/
number-row readouts and natural status/audio timing. Prioritized simple gameplay
steps are in [TESTING](../TESTING.md).


The final mounted-vehicle hardening was built, smoke-tested, rescanned in
security_predeploy_v2/ and redeployed/hash-verified all six. The first passing
build/scan/install records remain historical. Reaction-owner audit additionally
verifies all40 serialized condition reaction inspectors are on EffectPrefab roots.
