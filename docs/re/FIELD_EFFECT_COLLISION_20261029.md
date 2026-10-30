# Solid field effects omitted from navigation

Current checkpoint: source/offline **Closed**. The101 corrected effect records
and current/future classification were included in installed FFIII75de3594;
applicable IV/V installation remains pending later all-six delivery. No additional
production change or redeployment was needed for the final physics/route checks.
Installed DLL hash remains75de35945ed43421433ec141e6f4e7489ca817e8ad6020a28f25f85a43b33c45.
Earlier status below is retained as investigation history, not current progress.
Broader audit resumes at22 NPC geometry cases and the separate arrival/scene list.

Active capability in the human-requested all-six reachability audit. Invincible
and the reported equipment/vehicle fixes remain source/delivery Closed. Sol only;
no game launch/control/probe, new hook, dependency or publication. Estimate45–75
minutes plus combined/release checks; current status native proof complete,
implementation not started (~25%). Scope expands the8 Karnak scripted flames to
the same proven omission in101 collision-enabled effects (III4, IV1, V96), without
inventing destinations for the other4234 decorative effects. No new subsystem
architecture or rewrite of Closed navigation designs is authorized.

## Native/data evidence

Actual all-six objects are inventoried by `tools/re/field_effect_facts.py` with
full input hashes and source-bearing facts outside Git in
../tools/map_event_audit_20261028/effect_inventory/. Collision-enabled counts are
0/0/4/1/96/0; total visual effects265/585/522/1086/999/878. The previous8/6 omitted
scripted-effect triage was incomplete for collision-only objects. The4 FFIII
objects include world Map_10010/entity74 and Map_30091_1/entities11–13; FFIV's
Map_30080/entity32 is also collision-enabled. Retain actual conditional groups,
layers, rectangles and native IDs; do not infer every effect is a wall.

Exact III/IV/V `MapManager.CreateEntity` builds PropertyEffectEntity and
FieldEffectBoardEntity, whose Init disables its collider only when Collision is
false. Its collider size is the native property rectangle with the native edge
inset; the prefab enables a non-trigger BoxCollider2D. Default FieldEntity.CanEnter
returns false; FieldCharaEntity's raycast consumer invokes CanEnter and retains
non-trigger hits. Show/Hide use the ordinary native entity lifecycle. This proves
solid effects cannot be treated as decorative pass-through tiles. Source/native
and prefab evidence is in ff3_effect_native/, ff4_effect_native/ and
ff5_effect_native/ beside that inventory; original extractors are
`tools/re/map_location_native.py --field-effects` and `field_effect_assets.py`.

FFV locators: CreateEntity29FED0, Board.InitEF31B0, Board.ShowEF4AA0,
Board.HideEF3120, FieldEntity.CanEnter34CA40, Chara.GetClosestRaycastTarget570DA0.
The exact field is PropertyEffectEntity.Collision/C0, board collider138 in FFV.
No live capture has been authorized or needed for these statically settled facts.

## Minimal implementation/acceptance boundary

Retain native type11 only for collision-enabled effects in bakes, with actual
identity/group/rectangle; add its collision role to existing current/future
obstacle consumers and existing initializer visibility/position projection.
Keep the display detector's visual-effect exclusion. No generic Scene label,
new destination, event trigger or speech is implied by a collider. Preserve
transportation selectors, per-leg masks and script-owned enable/disable.

Before/after checks must use actual type11 through capture and real source rows,
not a type5 Entity stand-in. Check current/future state, hidden/moved effects,
rectangle/layer/vehicle selection, missing/disabled decorative effects, actual
Karnak route avoidance and all101 source fingerprints. Verify regenerated
metadata adds only reviewed objects, existing grids/objects/arcs unchanged and
FFI/II/VI bakes unchanged. Measure owned processing separately; no new game-thread
native observation is planned. Combined six-game checks and applicable final
build/scan/installation remain required before source/delivery closure.

## Final implementation and evidence

BAKED_TYPES/ObjType retain type11 only when its real Collision flag is true;
actual identity/group/native1x1 rectangle stay intact. Resolver active/effective
selection and geometry projection now include this type. Existing live capture
and Show/Hide presence supply current positions; matching real baked flags supply
collision role. Anticipated release uses its projected geometry and does not
resurrect an old live effect wall. Display detectors still exclude visual effects;
none becomes an NPC, Event or invented destination. No new hook, native read,
polling, thread crossing or retained game pointer is introduced by this change.

Exact bake delta receipts prove only III4/IV1/V96 additions; all original objects,
arcs, terrain/grids/master/state are preserved and I/II/VI byte-identical. The
subsequent single FFV Torna flag is independently chained in final verification;
it is not an unreviewed collision delta. All101 size checks are1x1, including25
off-grid staging positions; no larger effect anchor was generalized.

The actual type11 capture/Resolver/live-blocker host passes101/101. Real geometry
route acceptance uses actual Karnak native layers/cells with production find():
14 bare-route witnesses cross an observed solid flame before applying collision;
all14 respect it afterward,11 find a reachable detour. The remaining3 have no
route under that isolated geometry. This is not an assertion that every other
story/visibility state was present in this single-effect witness.

FFIII's one world effect is also checked in actual Enterprise ship13 and floating
Invincible11 modes. The real owner capture normalizes world layers to-1; the first
expanded fixture incorrectly overwrote that with source layer1 and also mistook
automatic canoe2 for a ship. Corrected fixture retains the production world policy
and actual master identities; both world modes and all Karnak cases pass. No
production defect or mode exception was inferred from those fixture failures.
Earlier41 aggregate visibility assertions likewise confused this effect's absence
with absence of every collocated wall; corrected contribution comparisons pass.

## Flight applicability settled from the actual native movement path

Exact FFIII FieldPlayerKeyAirshipController.MoveToAxis71A820 invokes the player's
MoveTo virtual slot37. This resolves to FieldPlayer.F17FA0, then
FieldSpriteEntity.CBE060, then FieldEntity.F0DC30. That ordinary movement calls
UpdateRaycast virtual slot38 when IsOnCollision/B4 is enabled, resolving to the
already-reviewed FieldCharaEntity.850B00/GetClosestRaycastTarget84EA80. A nontrigger
effect uses default FieldEntity.CanEnter3470B0=false. GetOnF17A00, normal/special
airship Init71A4B0/71B420, ChangeTransportation27A090 and ChangeViewBird27AE90 do
not introduce a persistent collision-disable, different effect selector or
DelegateIgnoreHit for flight; the bird-view setup changes presentation/callbacks.
Thus there is no supported reason to drop the active world effect for aircraft.
Script-owned temporary collision changes remain distinct from ordinary routing.
No guessed air-only/ground-only filter was added.

Original extractor map_location_native.py --field-effect-flight retains25 exact
producer bodies/provenance outside Git in ff3_effect_flight/. Native source/asset
proof and offline geometry are separate from target-game frame/collision/gameplay
confirmation. The production correction adds only four world/local FFIII records
and bounded existing worker classification; no new owner-thread observation.
Canonical build/smoke/8419 names/67 resources/12 locales/22-file scan and FFIII
installed-file checks passed before the final check-only extension; DLL unchanged.
Evidence: build_check/field_effect_20261029/bake_delta.json,
final_capture.txt, native_world_build.txt and native_world_acceptance.txt;
delivery build_check/ff3_oct29_followups/completion.json.
