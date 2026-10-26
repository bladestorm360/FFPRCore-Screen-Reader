# Battle damage resources, Swap and condition feedback

October25 [undead Cure/HP polarity correction](FF3_FEEDBACK_20261025.md):
nominal Recovery HitType must not override the actual renderer recovery flag.
All-six actual-source ABI tests retain displayed digits, MP/Miss and KO order.

October24 [Imbibe follow-up](IMBIBE_20261024.md): FFII's confirmed condition50/
type203 has no native name key. Its scoped fallback now describes Attack up,
Evasion down at the existing Add producer, preserving removal tracking and the
other games. Native mechanics, locale coverage and delivery status are separate
from gameplay listening confirmation in that record.

October23 status: **complete; offline verified and deployed to all six**.
Combined with [world exits, Ship and navigation response](WORLD_EXIT_FOLLOWUP_20261023.md).
Deployment is authorized; no game launch, gameplay control or runtime probe was used.

October24 correction: the earlier MP fixture did not cover FFII's actual sibling
class. [Current equipment/MP follow-up](BATTLE_ITEM_EQUIPMENT_20261024.md) records
the native provider, distinct layouts and expanded regression checks. The older
October23 delivery below remains historical evidence, not gameplay confirmation.

The subsequent sibling correction still failed gameplay confirmation because
the native ViewEntity loops bypass its wrappers. [Three-part direct rendering](PARASITE_THREE_PARTS_20261024.md)
records the current typed-producer binding and direct/delayed regression paths.

## Reports and established producers

The retained FFII log records Parasite, Guy:19 damage, Guy:1 damage, and
Parasite:Recovered1 MP. It also records Brain:Swap IV without a result and
Brain:Fog VI followed by misses for Firion, Guy and Leila, with no Maria condition.
The user confirms Maria received Amnesia. These are separate native result events;
no text-content deduplication or damage-total inference is appropriate.

FFII's condition master gives Amnesia id14, ConditionType.Forget205, icon12,
effect316 and MesIdName None. The shared battle fallback and a duplicate menu
fallback omitted205. Battle's by-ID resolver also returned empty immediately
for a missing message key. Consequently both Add feedback and the native icon
capture could discard the same real condition. condition_labels.h now supplies
one fallback table for battle_util::condition_name and menus::condition_name;
condition_name_by_id uses the same resolver. Native localized MesIdName remains
first choice. Ordinary harmful conditions including Mini, Toad, Pig, Imp and
Frozen are covered. Internal AI/equipment/target flags get no invented labels.
FFII's Double enum alias407 is not confused with other games' Slip/Sap.

condition_added still runs after the game's actual Add and resolves its confirmed
condition. It does not announce a requested spell/item's possible effects before
they apply. The existing removal tracking and cleanup rules remain. Player target
speech still reads the actual rendered party row, its condition text and captured
matching visible icons. Native GetIcon/UpdateView own those captures; hidden or
reused sprites never disclose a stale condition. No hidden enemy-status inventory
or new enemy-stat access is introduced. Every actual icon-bearing master condition
without a message key in all six games now has a shared fallback.

The generic damage renderer has glyphs and recovery/miss booleans, without a
resource name. The existing typed BasicFunction scope remains authoritative for
ordinary results. Absorption has additional resource owners: FunctionBase.Value1
uses2 for MP on AbsorptionFunction/MagicAbsorptionFunction; percentage/wear
BattleFunction13 (including its FFII Serial subtype) and the separate
BattleFunction13FF2/FF3 siblings own absorptionType0 HP/1 MP. Each class resolves
its own field; sibling FF2 is not assignable to the base BattleFunction13 class.
The native target result can retain an ordinary Hit/Zero type while its attacker
result is MPRecovery. The shared special_feedback producer scopes now retain that
resource across native target/attacker view calls. Physical primary hits cannot
inherit a mixed secondary absorption function's resource. Ambiguous HP/MP groups
retain their native type, misses remain misses, and scopes restore at return.
FFVI's explicit MpAbs9 is also classified as MP. The amount continues to come
only from actual rendered digit sprites; internal damage arguments and parameter
changes never supply spoken numeric damage.

## Swap mechanics and phase contract

Matching FFII Ability.process_prog WM_CHANGE uses BattleFunction23/UniqueFunction.
Serial.FF2.Calc.UniqueUpdateParameterProvider's four-argument parameter overload
exchanges caster/target current HP, caps each at its own maximum, then exchanges
current MP with the same cap. It does not exchange HP with MP. Its CreateViewEntity
emits a system message for Dispel only; WM_CHANGE has no native explanatory result.
The mod adds a brief target-and-caster line: HP and MP exchanged with the caster.

special_feedback observes the exact parameter overload performing WM_CHANGE inside
BasicFunction.UpdateParameter. Its bounded current function-target dictionary matches parameter
identity to a real unit; names are copied on that owner. A record is created only
after the actual native exchange returns. BattleFunction23.CreateViewEntity
consumes the matching operation record once, then queues its owned names/locale
for formatting and speech. A view with no actual operation cannot claim success.
Equal resource values are still a valid exchange. Other unique keys cannot create
Swap feedback. This is operation identity, without comparing speech text.

## Sources, ownership and extension

Production: battle/special_feedback.cpp and .h, battle/battle_message.cpp,
battle/battle_util.cpp, menus/magic_menu_state.cpp, core/condition_labels.h,
displayed_conditions.cpp, battle/battle_party.cpp and battle/battle_command.cpp.
Original extraction script: tools/re/battle_feedback_native.py. Matching game
bodies, hashes and locators remain outside Git in
D:/Games/Dev/Unity/FFPR/tools/battle_feedback_20261023/ff1..ff6/.
FFII's known-target direct damage-call xrefs are also retained there.

| FFII native producer | RVA | Established fact |
|---|---:|---|
| BattleBasicFunction.CreateDamageView | 63D5C0 | Typed hit kind into generic visible renderer |
| BattleBasicFunction.CreateAfterViewEntity | 63D3A0 | Separate secondary target result |
| AbsorptionFunction.Calc / UpdateParameter | 638900 / 638C30 | Value1=2 selects MP |
| BattleFunction13.PrevCalc / HitCalc | 645BC0 / 644EB0 | Outer HP/MP absorption mode and result groups |
| BattleFunction13Serial.GetAbsorptionValues | 844D90 | FFII wear absorption provider |
| UniqueFunction.UpdateParameter | 8CCAA0 | Actual unique parameter operation |
| UniqueUpdateParameterProvider.UpdateParameter(parameters) | 41C1D0 | WM_CHANGE exchange and caps |
| UniqueUpdateParameterProvider.CreateViewEntity | 41B010 | Dispel-only native explanatory text |
| BattleConditionController.Add | 6B6230 | Applied condition followed by parameter refresh |
| ConditionIconController.GetIcon | 535250 | Actual native sprite producer |

The native observation caps are64 target dictionary slots,16 functions per target
and64 Swap records. FF1/FF2 alone have the additional AfterView methods; FFIII–VI
use their actual two main view methods. By-name masks and the matching dumps own
these distinctions. No main-thread wait, whole-scene search, file/network I/O or
parameter mutation is added. Only current rooted data is read; retained pointers
are comparison identities and never dereferenced later. Install reserves Swap
record capacity. Battle end clears unmatched records. Workers receive copied
strings only. Queue rejection remains a bounded omission, never an owner wait.
Extending this code requires matching a real native resource/operation producer,
not recognizing localized monster or spell names.

## Verification and remaining player checks

Canonical build includes battle_feedback_check: actual damage/Add/special hooks,
all-six resource scopes, the ordinary-HP-plus-secondary-MP case, outer percentage
absorption without a FunctionBase list, misses, scope restoration, confirmed Fog
Amnesia and native Swap operation/view identity. battle_results_check exercises
the actual native icon producer/reader with visible, hidden and replaced Amnesia
sprites. battle_focus_check preserves supplied rendered condition names through
all-six target announcements. Existing real digit-sprite checks remain required.
The new label audit checks every icon-bearing condition and all12 catalog locales.

Offline observation timings use fake native reads and exclude real Unity/native
cost, queues, frame tails, data age and audible output. They cannot certify runtime
responsiveness. The user should next confirm Parasite's extra amount says MP damage,
Fog announces Maria:Amnesia and an affected item target includes Amnesia. A successful
Swap should briefly identify the target and caster; an unapplied attempt should not
claim an exchange. No replay is required solely for testing.

Luna/gpt-6-luna supplied seven bounded catalog keys in12 locales. Sol reviewed all
values, corrected Japanese Amnesia to the game's wording and replaced French Sap's
incorrect tree-sap translation plus three related loss-wording choices. Sol owns
implementation, native investigation and validation. No Astra was used.

## Final verification and deployment

Canonical build and all offline checks pass:7,301 native name checks/zero missing,
125 used/333 defined actions/zero undefined and47-export isolated smoke.
All six installed DLLs match:10687488 bytes, SHA256
7dc3d8623711796ea965b219ae278392f517dfe32336042f38a7f6cd52ca8e70.
All144 protected settings/waypoint/dependency files are unchanged. Every one of
43 embedded data resources matches its source. Recovery copies, complete
reports and delivery/deployment receipts are retained in ignored
build_check/world_exit_labels_20261023/. No game was launched or probed.
Gameplay confirmation remains pending. Published V0.9.5 source/tag/ZIP were not
modified; its retained public ZIP still matches the verified original hash.

The maximum MP-observation fixture (64 target slots/16 functions) measured median
2.100 microseconds, p99 3.800 and maximum32.200 over2,000 samples. This is fake-native
observation cost only, without real Unity, queue or audible response measurements.
Battle-end epochs cancel queued obsolete Swap feedback.
