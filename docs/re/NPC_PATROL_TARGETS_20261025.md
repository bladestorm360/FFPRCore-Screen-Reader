# Patrol guidance, companion identity and field-menu feedback

October29 human-initiated all-six NPC audit follow-up:
[native instruction patrol correction](NPC_GEOMETRY_CONTEXT_20261029.md).
The earlier blanket stationary treatment of RoutePuppet102 is superseded only
for III/V/VI, whose active controller sets102, reports moving and restores the
original type on destruction. I/II/IV's older controllers do not publish102 and
their extracted scripts contain no AssignMoveRoute; retain their existing policy.
All-six native getters report moving even after one-shot completion;102 alone
therefore does not certify a patrol. The later correction checks a bounded ready
controller for remaining movement. Completion/animation/overload/Wait/manual
checks pass; combined delivery pending. The initial scalar-only candidate was
never deployed. See the linked audit for exact native lifecycle/budgets/evidence.
Accepted route/blocker/companion architecture stays Closed and unchanged.

Status: implementation complete and offline verified. All fixes are now delivered
in the all-six final transition-route payload linked below. Earlier five-game
receipts and37ab60d9 payload facts are historical; final receipts belong there.
No game was launched,
controlled or probed. Existing dirty work and earlier installed recovery are kept.

The subsequent [terminal-route follow-up](TRANSITION_ROUTES_20261025.md) includes
these fixes and is installed in all six. The user's later Desch screenshot came
from III's old a9f44da0 DLL; Desch/base identity35 is covered by this pending name
fix and its synthetic-property regression. That old installed result is neither
gameplay confirmation nor failure of the subsequently delivered correction.

| Feature | Implementation | Offline evidence | Gameplay |
| --- | --- | --- | --- |
| Wanderer waits in every corridor | complete | full native-routine/detour/override checks pass | pending |
| FFIII companion names | complete | synthetic-property and all8 native identity checks pass | pending |
| Field magic All allies | complete, all six | native entry/return/close tests pass | pending |
| Enemy Critical disclosure | complete, default off | all-six add/fallback/remove/party tests pass | pending |
| Event key-item Autodetail/I | complete | focused source/lifetime and Event12 admission tests pass | pending |

## NPC obstruction classification

The old npc_route.h narrow_channel rule changed an autonomous patrol's terminal
message to Interact when opposite sides of its occupied cell were walled off.
Geometry cannot establish a stationary/scripted NPC. The classifier is removed:
after all complete detours fail and an NPC-free proof reaches the destination,
Actor.wanders alone selects Wait for the named NPC to move. A safe approach still
keeps its occupied cell solid. Stationary/unknown actors retain manual interaction
and native facing. The original destination, pending cross-floor identity,
beacon non-arrival, cancellation and bounded autowalk retry remain unchanged.
No route-distance cap, teleport permission or walk-through-NPC edge was added.

Native capture now copies PropertyNpc.MoveType and MoveStep into two owned scalar
facts. Area2/Route3 with positive steps are autonomous. None0/Stamp1, zero steps
and script RoutePuppet102 are stationary for this policy. Paused frames, facing
animation, position changes and proximity to a baked home are not used as proof
of a routine. Fresh native movement facts override bake flag16; immutable bake
metadata is the fallback if native properties are unavailable. This matters for
duplicate/id-zero names and condition-changed actors. Companions remain known
non-solid and are never wait/interaction obstacles. Counter links of authored or
currently wandering actors remain excluded as route-polyline metadata.

All6962 NPC/shop records match native source movement properties in the bake:
patrol/static counts I216/287, II323/531, III231/589, IV160/1098, V297/1016,
VI409/1805. npc_patrol_check.py distinguishes same-id/name actors by exact source
coordinates before checking metadata; the first coarse audit exposed that real
FFV ambiguity and was refined, not interpreted as a bake failure.
npc_gate_check.cpp replays the actual Mysidia inn corridor, detours, clearing,
unknown gatekeeper and cross-floor/story-gate cases. It now also tests native
routine capture and fresh stationary/patrol overrides. No worker calls native APIs.

## Companion identity correction

The previous PropertyNpc.AssetId lookup was wrong for generated FFIII followers.
FieldController.SetupCompanionEntity creates a synthetic PropertyNpc whose asset
ID remains zero, then ChangeCompanionEntityAsset reads the actual saved companion
base identity to choose its sprite (including condition variants). The earlier
fixture populated a property that native setup does not populate; it did not
model this producer. The corrected fixture reproduces zero and follows the saved
identity rather than merely extending an ID table.

party_entities.cpp first verifies that the current FieldMap/FieldController owns
this exact companionEntity. Only then it reads SaveMapManager.CompanionEntity's
SaveCompanionEntityData.fieldCharacterAssetId, the same source as the native
renderer. The existing8 Current.CompanionId mappings/native translations remain.
Null/unowned/missing/unknown data stays unnamed; no cached join, party slot or
nearby-NPC guess is used. The naming change is FFIII-specific; all-six companion
nonblocking classification and FFVI parked-party ownership remain unchanged.

Native provenance outside Git: ../tools/npc_blockers_20261025/ffN. Extractor:
tools/re/map_location_native.py --npc-blockers. Matching III SetupCompanionEntity
0x292260 and ChangeCompanionEntityAsset0x2794E0 establish the synthetic/base-ID
split. Dump types/fields and DLL/dump hashes are retained; no native code is copied
into repository documents. party_entities_check tests all8, ownership changes,
missing singleton/data, unknown IDs and other-game isolation.

## Field spell targeting

The screenshot's simultaneous hand cursors are native all-target mode. The
AbilityUseContentListController in each game's Serial.FFn.UI.KeyInput namespace
has distinct AllInit/SingleInit phases. IsAll/IsSingle methods describe ability
capabilities, so they must not be mistaken for the current selected range.
Native evidence: ../tools/field_targets_20261025/ffN, via --field-targets.

menus/field_magic_targets.cpp owns the range entry. Before AllInit it suppresses
nested individual cursor reads; after the original it says All allies using the
existing12-language key. Returning SingleInit suppresses outgoing group callbacks,
then reads the actual rendered focused row. Initial single focus stays with the
existing reader. FFV retains its established bounded first-ready SingleInit
producer: it clears the range prefix before its original call, and its AllInit
callback no longer schedules a single-row read. Other individual adapters and
the generic cursor yield to FieldMagicTargets.OwnsRange.

Only an active native controller in positive MainMenu context claims ownership.
Close, MenuClosed, MapChanging, BattleEntering and Title clear it. Queries verify
owner liveness/visibility. No text deduplication, key simulation, new polling or
sleep is used. Group output is constant work; single-return capture traverses at
most9 displayed party slots. Native pointers stay on the owner thread; stale
post-native callbacks cannot replace a newer owner/phase. field_targets_check
tests all six producers, nested callbacks, single return, hidden/event contexts,
FFV's existing producer and lifecycle cancellation.

## Enemy critical health

The Giant Rat screenshot has no enemy Critical label/icon/HP display. All-six
ConditionType4 masters have icon_id0. DyingConditionFunction starts AnimDying;
generic enemy animators inherit BattleAnimatorBase.Dying/UpdateDying no-ops,
with no enemy override. Party dying poses remain separate. This hidden condition
was leaking from the unconditional condition-Add producer, not the optional
renderer-proven Enemy status announcements module.

Speech now has Enemy critical status, persistent key EnemyCriticalStatus,
default false, independent of visible enemy status announcements. User explicitly
authorized this optional hidden-information disclosure. Off suppresses only
enemy type4 additions/removals; On retains the prior announcements. Party
Critical, critical-hit damage, other condition effects and Enemy HP modes are
unchanged. Master resolution returns the resolved type with its name, so an Add
before confirmed-list readiness cannot bypass the filter. No HP threshold is
invented. Existing explicit enemy visual effects continue through their own
renderer-backed path. Three menu/help keys have all12 locales and a maintained
recipe. All-six add/unconfirmed/remove/party regressions and catalog checks pass.

Native evidence is ../tools/enemy_critical_20261025/ffN (--enemy-critical), with
matching masters outside Git. This establishes generic condition presentation,
not a claim that every bespoke boss phase has identical art.

## Event key-item description

The own III log113311.328/113314.421 reads Canoe/Nepto Eye with descriptions while
EventItemSelect is active. ff3/event_item_select.cpp appended descriptions
unconditionally. It now copies the current native row's description for manual
I, appending it automatically only with saved Autodetail on. The existing local
event guard remains; no new text dedup is introduced. Blank/replaced/closed/hidden
owners clear the old description, and a new owner resets the existing guard.

The details cascade prioritizes this positive event owner over an old character
row. Event12 now admits its I controls while retaining transition/fade/loading
guards; it does not grant field movement or MainMenu context. MapChanging and
native manager Close retire the cached text. The native item list/selection
operation is unchanged. FFII's related keyword selector already gates description
with Autodetail and supplies manual details; shared item/shop/battle item readers
also retain their existing gates. The bug was in III's dedicated event adapter.
event_item_select_check tests Off/On, repeated manual I, changed/blank items,
hidden/closed/map-transition ownership, preserving native originals. The shared
game-state fixture tests Event12 admission without bypassing fades.

## Ownership, verification and remaining work

Production changes stay in cohesive field/menus/battle/input owners. The new
range module is small; existing large navigation files keep their documented
cohesive exception. Native routine capture adds a type check/two scalar fields to
the existing sliced owner capture. Computation, formatting and cancellation keep
the existing owned-worker pipeline, with no game-thread wait or pointer transfer.
No new universal latency target is asserted. Existing fixture timings exclude
real game scheduling, audio/SDK latency and device output; gameplay remains user-side.

Evidence/recovery: build_check/npc_patrol_companion_20261025/. Canonical build,
smoke, resource hashes, final Defender scan and all-six installed/protected-file
receipts are required before deployment. Intermediate fixture link/include errors
were repaired before installation; no failed build was deployed. No agents,
dependency upgrade, protection change, source upload, commit or release is involved.

## Verified payload and installation receipts

Final canonical build/all offline checks passed:7994 native-name checks, zero
missing;137 used/344 defined/zero undefined wiring. All6962 baked movement records
match their exact native data; focused NPC, companion, field range, Critical and
event-item tests pass. Isolated smoke forwards47/47 exports. All49 embedded data
and18 sound resources match source. DLL10944000 bytes SHA256
37ab60d98b07119717a883e49fd540a22d7a5b2a4d1e8511721897f34cb9e09a.

security_final_predeploy's enabled/current Defender scan reports no threats on
the exact22 staged files, zero matching detections and stable hashes. No signing
certificate is available; exclusion enumeration is unverified, and reputation/
future package coverage is not claimed. No protections changed or samples uploaded.
verification.json retains source fingerprints and checks/smoke/resource results;
native_provenance_index.json links all-six source locators/hashes outside Git.

deploy_partial.json verifies all110 installed payload hashes in I/II/IV/V/VI and
120 protected files unchanged. At that earlier deployment III's running-process guard refused replacement;
its previous DLL remained intact. Recovery and original/live-session receipts are
preserved. The user was asked to close III; a fresh deferred-game protected
baseline and final deployment were subsequently superseded by the combined
transition-route delivery, whose all132 hashes/144 protected-file checks pass.
