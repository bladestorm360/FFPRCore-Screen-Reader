# Battle Items equipment strip and MP drain follow-up

October25: [Tab hand-slot panel and controls](BATTLE_GEAR_CONTROLS_20261025.md)
are separate. The equipped strip below remains gameplay-confirmed. The new slot
owner reads hand/favored-hand/name fields. Counts/descriptions in candidates
belong to the stripped battle interface, without full menu stat comparisons.

Status: complete; offline verified and deployed/hash-verified in all six games.
Gameplay/listening remains unconfirmed. No game launch, control or runtime probe.

Later October24 user confirmation: the equipment row works. Parasite's MP label
still fails in two new examples; the earlier wrapped fixture missed direct native
render calls. [Current three-part correction](PARASITE_THREE_PARTS_20261024.md)
supersedes the MP completion claim below. Earlier delivery hashes remain history.

## Equipment strip

The user's corrected screenshot shows Wing Sword and Diamond Shield above the
battle Items list, a rendered equipped marker, a sword description and native
Tab: Change Equipment. Enter's invalid-action sound belongs to the game; this
reader does not change item usability or native input.

All six KeyInput BattleItemInfomationController declarations have a separate
SelectEquipContent(Cursor,index,WithinRangeType) and UseEquipSelectInit phase.
The ordinary SelectContent hook never observes this focus. Native selection
updates the cursor and rendered description synchronously. SetEquipList populates
equipList and the corresponding equipContnetList view pool (native spelling).
Each BattleEquipContentView uses lowercase iconTextView, countText and equipedIcon.
The reader uses the explicit native index because the cursor parent can still
belong to an ordinary item. Only active root/equipment graphics, rendered text
and the visible equipped image are read. Actual list count bounds pooled rows;
invalid indices, missing names and lists above16 are rejected. Native equipment
slots fit this bound; no scene search or parameter/inventory-name fallback.

Production: src/core/battle/battle_item.cpp, equipment_announcement,
d_select_equip and d_use_equip_init. Two additional hooks cover all six games.
The entry operation counter prevents duplicate nested-entry output without text
deduplication. Ordinary item and equipment rows share menu ownership, I and
Autodetail; one short row needs no review buffer. Names/descriptions come from
localized rendered fields, Equipped uses the existing12-language catalog.
Close clears the owner. Native pointers are used only synchronously on their
owner; no worker, asynchronous retained snapshot or pointer transfer is added.
The callback reads one row, at most two bounded lists and a few known fields;
formatting has the same small-row scope as ordinary Items. Speech dispatch
remains on the shared nonblocking service. No game-thread wait or new polling.

Offline native exports/provenance are outside Git at
D:/Games/Dev/Unity/FFPR/tools/battle_item_equipment_20261024/ff1..ff6/.
Extractor: tools/re/ff6_story_native.py --battle-item-equipment.
FFII KeyInput SelectEquipContent6277C0, UseEquipSelectInit62B7B0,
SetEquipList628560 and BattleEquipContentController.UpdateView43B630 establish
selection, population, description and marker flow. Other games retain their
matching per-game declarations/exports; no fixed RVA is used in production.

## MP drain correction

The supplied log has ordinary damage to Guy, then1 damage to Guy followed by
Parasite recovering1MP. The earlier fixture tested BattleFunction13 directly
and did not represent FFII's actual sibling BattleFunction13FF2. It therefore
passed without covering the reported native producer.

FFII FunctionProvider.GetFunctionClass596A30 dispatches function13 through
596B26 and constructs BattleFunction13FF2 at596B3A. This is a sibling of
BattleFunction13, with its own absorptionType field at98 rather thanA8.
PrevCalc841330 assigns0 forHP and1 forMP; HitCalc83F800 dispatches its physical
or magic absorption. BattleFunction13FF3 is another sibling. Both exist in all
six builds, so special_feedback.cpp recognizes each class and resolves its own
field by name in every game. Base/FFII Serial inheritance remains supported.
Primary physical hits, misses, native recovery polarity and scoped restoration
are preserved. No resource guesses from amounts, monster names or hidden HP/MP.

Evidence: external tools/parasite_followup_20261024/ff1..ff6/; extractor
tools/re/battle_feedback_native.py --parasite-followup. Existing observation
bounds/thread ownership remain in [the feedback contract](BATTLE_FEEDBACK_20261023.md).

## Checks and remaining confirmation

tools/battle_focus_check.cpp exercises actual equipment detours in all six games:
entry, delayed root activation, stale item cursor parent, left/right/return,
rendered marker/quantity, I/Autodetail, hidden names, invalid index, ordinary
Items return and close. battle_feedback_check.cpp exercises each real class
with opposing sibling/base field values, HP/MP/recovery/Miss, scope restoration
and the physical-hit/MP-drain/attacker-recovery sequence. Existing ordinary
Items/FFVI Tools and condition/Swap checks remain required.

Player check when naturally available: Up from Items should announce equipped
gear; Left/Right reads the selected slot. I reads its displayed description;
Autodetail includes it automatically. Parasite's separate drain amount should
say MP damage, while the physical hit remains damage. Offline fixture speech
does not establish physical audible latency or actual game callback timing.

## Delivery and performance evidence

Canonical build/all checks pass,7403 native names/zero missing,131 used/339 defined
actions/zero undefined. Isolated smoke passes all47 HID exports with normal exit.
All49 data and18 sound resources match source. Final DLL10797056 bytes, SHA256
3fbaf55870aa9886bd1124aa8f2e3f808a12159fa67e2d598bc125c573dfe457 matches every
installed game. All144 protected preferences/waypoint/dependency files match
the final baseline after the user closed FFII. Recovery, retained rotated user
log (673.015/681.015/861.015), reports, manifests and deployment receipts are in
build_check/battle_item_equipment_20261024/. No game was launched or probed;
no delegation, commit/push or public release change.

Comparable2000-sample actual-detour/fake-native capture-to-first-speech submission:
ordinary Items median0.700us/p990.800us/max23.400us; equipment median0.900us/
p991.000us/max22.000us. Native original callback is included but stubbed; speech
submission is a fixture collector. Current row is read on the event with no
new cache/queue or snapshot age. New production work is one indexed row, with
no traversal proportional to map/inventory size. Maximum64-target/16-function
resource observation: median2.600us/p992.700us/max18.900us. These numbers exclude
actual Unity/reflection, central speech queue, frame tails and audible latency;
they establish neither runtime speed nor an audible-response guarantee.

Before deployment, enabled Defender with current definitions1.461.181.0 scanned
the exact22-file built DLL/dependency stage and reported no threats, exit0, no
matching detection events and unchanged hashes. Installed bytes match that
scanned stage. Exclusion inspection remains administrator-protected/unverified;
four DLLs remain unsigned with no available signing certificate. No security
settings changed or external artifact upload. Receipt: security_predeploy/.
See [security/distribution](../SECURITY_AND_DISTRIBUTION.md) for these limits.
