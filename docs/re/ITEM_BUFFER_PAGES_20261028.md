# Equipment information page readiness

Human follow-up: Protect Ring initially lists eligible jobs, but after a native
page change only Page 2/2 remains, including when returning to Page 1/2.
This is a scoped correctness follow-up to the closed equipment buffer, preserving
its design and controls. Sol only; no game execution, control or runtime probe.

Status: reported follow-ups source/delivery Closed. FFIII installed/hash-verified
d3053479822a982e454e10e243bfb665a96d87dcbe6d4c545d2f8bd78b686200,
11311104 bytes. Canonical serial build, smoke,8406 native names/0 missing,
142 used/349 defined wiring/0 undefined and all applicable integration checks pass.
Defender reports no threats for22 exact staged files; installed hashes match,
144 protected files and the other5 installations remain unchanged. Evidence/
recovery: build_check/item_buffer_pages_20261028/completion.json, deploy_after.json,
security_predeploy and recovery/. No game launch/probe/delegation/publication.
Actual listening confirmation remains pending. The broader map audit resumes.
The45–90minute focused repair estimate included the intervening vehicle reports;
the full serial build adds elapsed time while keeping one compiler at a time.

## Source-to-output evidence

Own log retained at build_check/item_buffer_pages_20261028/latest.log:
generation383 has12 rendered job names/Can equip and Page1/2 at390389.109;
generations385/387/389 at390397.859/390402.343/390405.828 have only the pager.

The shared KeyInput ItemEquipmentController.SetPage activates an incoming page
before the native page coroutine runs its fade-in. The shared displayed reader
correctly excludes text with zero inherited CanvasRenderer alpha. A one-shot
SetPage snapshot therefore completes with no labels during this animation.
Both page coroutines request the appropriate StartIdle iterator only after the
incoming animation completes. The mod now observes that producer and replaces
the copied current-page buffer without another item-entry announcement.

Production: src/core/menus/item_equipment.cpp, capture,
refresh_after_animation, d_equipable_idle, d_detail_idle and install.
Both original native iterator objects/returns are preserved. The callback checks
current window/state7, exact equipment ownership and current detail/page identity;
late completion after closure or from an unrelated page cannot publish.
Reset the shared row index when a hidden detail section is skipped, so an
interrupted detail capture cannot start partway through the subsequent job list.

Existing controls remain: arrows review, I full current buffer, Q description/
parameter footer, native numbered-page controls change the displayed page.
Read only actual rendered fields and native observed eligibility colors. Hidden
pages remain excluded. No hidden data, guessed delay, new polling or text dedup.

## Exact native/asset applicability

tools/re/map_location_native.py --item-pages and tools/re/item_page_assets.py
read matching installed binaries/dumps and only the two catalog-listed equipment
job prefabs plus their dependencies. Proprietary bodies/hierarchies/curves stay
outside Git at ../tools/item_buffer_pages_20261028/ffN/ with hashes.
All six have5 prefab descendants, the same6 switching/idle clip references and
two separately unique StartIdle native producers (no folded hook bodies).

| Game | Job-page StartIdle RVA | Detail-page StartIdle RVA |
| --- | --- | --- |
| I | 49D5B0 | 616630 |
| II | 9561A0 | 95AFA0 |
| III | 7B3D60 | 7B8B60 |
| IV | 4E4EC0 | 64E5E0 |
| V | 9E5670 | 9EA470 |
| VI | 9520D0 | 956F80 |

## Bounds and verification

Two producer hooks occur once per native page transition, rather than per
animation frame. At most8 page references are checked; existing generation-
cancelled capture owns at most4x64 parameter rows plus8x32 eligibility rows,
16 icons per attribute row, with the existing CaptureBudget. One latest capture
label retires obsolete work. Native pointers stay on their owner; only copied
text enters the existing review formatter/worker. Hook/game/audio latency is
not measured by the fake-owner host; the cumulative25ms rule still applies.

Actual registered-hook fixture tools/equipment_party_check.cpp models the
incoming page's invisible text and native ready event across all six games:
36 failures before,0 after. Repeated page2/1 changes, visible eligibility,
returning parameters, hidden-page exclusion, original call/iterator identity,
unrelated producer and late closed-owner cases pass. Existing item quantity,
single entry, Q refresh, Autodetail, slot and menu integration checks also pass.
Evidence: build_check/item_buffer_pages_20261028/{before,after}.txt and
all_six_native_producers.json. Final combined build/security/delivery and actual
listening confirmation remain distinct. Final combined build/security/delivery
now pass as recorded above; listening remains unconfirmed.

## Adjacent human reports, separate checkpoints

- Underwater guide should say Can surface. Source/focused checks pass4→0;
  native Submarine6 models are FFIII14 and FFV8/13. Existing guide availability
  and occurrence edge own speech;12 locales, no extra native terrain queries.
  Source: fieldnav/vehicle_landing.cpp and tools/modtext_dive.py. Installed in FFIII.
- A parked vehicle directly outside an exited map can say Interact: NPC before
  discovery has populated its model identity. Source correction passes the
  actual prompt/model host across all six games. Native CreateProperty and
  SetMapObject place transportation ID in PropertyEntity.ObjectId. The resolver
  uses one bounded native model dictionary lookup on a cache miss, checking
  exact MapObject identity, enabled state, stay map and model ID. Reused NPC IDs,
  other maps, disabled/mismatched models and oversized dictionaries abstain.
  No nearest-model inference, new hook or polling. Source:
  field/vehicle_identity.cpp and navigation.cpp/vehicle_type_of. All-six native
  facts: ../tools/vehicle_map_identity_20261028/ffN/ and
  build_check/item_buffer_pages_20261028/identity_native_fields.json.
  Native_cues_check integration0 failures; native game latency unmeasured.
  Included in the checked FFIII installation.
- Duster to Ancient Ruins may lose a vehicle leg while technically walkable.
  Investigate full approach/ride/landing cost and retained-route/capture behavior.
  The available own log later shows(115,110)→(46,56),146 steps/3 phases including
 5 walking steps to Nautilus; that successful later route does not disprove the
  reported earlier failure. Evidence routes_and_vehicle_prompts.log. Actual
  real-bake Duster witness(114,111), Nautilus(114,113), Ancient Ruins(46,56)
  reproduces2 corrected-host failures:144-step full route becomes141 steps and
  loses boarding on repeat/overlapping approach. FFIII/VI manual deck boarding
  intentionally uses VehicleAction.None; the first fixture's ConfirmBoard
  assumption was corrected before counting those failures. The route suffix
  locator formerly chose a later airborne occurrence of the same coordinate
  without checking current movement mode. remaining_from now rejects that
  incompatible suffix and uses the normal complete planner, preserving real
  pending boarding/landing boundaries. Follow-up check2→0; existing all-six
  route-worker/cancellation/98 boarded-vehicle cases pass. No heading-only
  pruning, map/goal substitution or shortest-route redesign. Installed in FFIII.
  Final corrected log:144 initial/144 repeated/141 already-boarded steps.
  All-six movement-mode/boarding/manual-deck/landing suffix boundaries pass.

The canonical check initially stopped on1582 legacy fixture assumptions that a
projected transition must apply to every vehicle. vehicle_transition_check now
prepares actual per-leg rules and verifies each matching native model plus the
current walking mode; all-six transition checks pass. Production was unchanged
for that fixture correction. Preserve legacy_fixture/build failure evidence.
