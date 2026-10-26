# Parasite: physical hit, MP drain and MP recovery

Status: complete; implemented/offline verified and deployed in all six games.
FFII received the combined [Leviathan build](LEVIATHAN_20261024.md) after that
requested routing fix passed. Its installed hash/receipts supersede the standalone
MP candidate below. Gameplay listening for MP remains pending.
The earlier equipment strip is user-confirmed working. The earlier MP fix was
deployed/offline verified but failed gameplay confirmation; this record supersedes
its MP completion claim.

## Observed failure and native cause

The current FFII session has two examples:2084.093 Guy12damage,2084.609 Guy1damage
plus Parasite recovered1MP;2098.218 Guy13damage,2098.734 Guy1damage plus Parasite
recovered1MP. The user confirms this is one hit with three parts. The MP amount
is a separate displayed result, not another physical hit or a hit-count issue.
Installed hooks report no failures. Retained user log:
build_check/parasite_three_parts_20261024/user_ff2_latest_log.txt.

The previous code classified the absorption owner only around per-unit
CreateViewEntity/CreateAfterViewEntity wrappers. Matching native FFII
BattleBasicFunction.ViewEntity63FB10 directly calls CreateDamageView at63FCD8.
AfterViewDamage63C780 likewise calls it directly at63C8FC and63C9F8 for the two
after-result dictionaries. Those direct calls bypass the hooked wrappers, so
adding the correct absorption sibling classes still did not label these draws.
Earlier fixtures manually entered a wrapper and therefore missed this call path.

The same FFII function retains the primary physical dictionary. PhysicsAbs840690
clones that target result at840FF8 and changes the clone's amount at841005.
In MP mode it inserts this clone into the after-target dictionary at841153 and
adds typed MPRecovery to the after-caster dictionary through8411CD. It leaves
the primary physical amount intact. Thus promoting every draw owned by the
MP handler would also mislabel the first physical result. This was caught in
the deeper native audit before deployment; the fixture now uses the same owner
for all three parts with the actual split dictionaries/phases.

## Current implementation and ownership

battle_message.cpp's three actual typed CreateDamageView detours now call
special_feedback::owned_damage_type with their exact current function owner
before continuing native rendering. All-six ABIs remain distinct: FFII includes
isPlaySe, FFV includes MissType, FFI/III/IV/VI use the four-argument game method.
The three absorption classes resolve their own resource fields by name. A new
FFI/II AfterViewDamage batch hook marks the synchronous current phase by owner
identity, restoring it immediately at return. For the FFII-style absorption
class with populated after-target results, primary draws retain their native
physical type; its later batch receives MP. Pure MP draws without that split
retain their resource. Miss and explicit MP/MP-recovery types remain unchanged.
No amount, actor-name, hit-count, hidden-parameter or action-wide heuristic.

DamageContext carries copied unit identity/type through the synchronous native
utility and renderer calls. Its existing stack scope restores at return. The
renderer captures the corrected type alongside its existing owned name for the
actual rendered-digit callback. No new native pointer is retained, worker or
queue added, input changed, polling introduced or game-thread wait performed.
Only cached class/resource checks and a constant-time native after-dictionary
count distinguish the split; no dictionary enumeration is added. The phase
hook stores only stack-scoped identity. Existing wrapper scopes still cover other native
secondary-absorption paths. Existing central speech/localization remains intact;
no new user-facing string or catalog entry is needed.

Production: src/core/battle/{battle_message.cpp,special_feedback.cpp,.h}.
Native extraction logic: tools/re/battle_feedback_native.py --three-parts;
matching native bodies/provenance remain outside Git at
D:/Games/Dev/Unity/FFPR/tools/parasite_three_parts_20261024/ff1..ff6/.
The prior sibling layout/provider facts remain in
[equipment/MP investigation](BATTLE_ITEM_EQUIPMENT_20261024.md).

## Required verification

Expanded battle_feedback_check calls the actual typed damage detour directly,
with no enclosing view hook or manufactured MP scope. It follows the native
utility detour, each game's renderer ABI and the actual mod's rendered-result
callback/formatting. Game originals, Unity objects and digit-read scheduling/
values are fixtures; this does not execute the native game. Existing real
digit-sprite reconstruction checks remain part of the full suite.
Both reported physical amounts are replayed through the same native function
owner as the1MP after-drain and recovery in FFI/II. The native original batch
is a fixture that reaches the actual mod damage, utility and renderer detours;
FFIII–VI exercise their actual damage ABIs without nonexistent after methods.
Internal numeric arguments deliberately differ from rendered
digits. All-six fixtures cover context restoration, zero MP, misses, HP recovery
and a later physical attack. Delayed digit callbacks preserve copied names/types
after the native context and current source fields change. Existing wrapper-based absorption, mixed effects,
conditions and Swap checks remain.

Next natural Parasite attack should announce physical damage, then MP damage,
then Parasite recovering MP. Runtime listening remains user-side; no game launch,
control or capture probe is authorized or performed.

## Final offline evidence and delivery

Canonical build/all checks pass:7409 native names/zero missing,131 used/339 defined
actions/zero undefined. Isolated smoke resolves/forwards all47 HID exports and
exits normally. All49 embedded data and18 sound resources match source. Final
DLL10798592 bytes, SHA256
73c8332982e078689efd6e6d0c14452503cfdad00948cc1b14305171e646fafc.
Recovery, user log, direct-call locators, reports/source hashes and delivery
receipts: build_check/parasite_three_parts_20261024/. The earlier unshipped
candidate/scan remains separate; only security_final_predeploy matches this DLL.

The full suite exposed an existing fixture setup issue: crowded_address_check
reserved its one64KiB trampoline hole before MH_Initialize created a bookkeeping
heap. That ordinary heap could occupy the sole test hole, failing at hook0.
tools/coexist_check.cpp now initializes bookkeeping first and verifies the hole
is still free before creating hooks.384 crowded hooks,512 ordinary simultaneous
hooks and all hook-chain/unhook checks pass. Production MinHook is unchanged;
the failure report/build log are retained rather than silently discarded.

2000-sample fake-native callback-to-first-submission timings: physical median
0.800us/p991.200us/max16.300us; pure MP median0.800us/p991.500us/max12.700us.
The existing maximum64-target/16-function observer measured median2.600us/
p992.800us/max17.900us. These are fixture costs, not actual Unity/native, whole
three-part batch, central speech queue, frame-tail or audible latency. Correctness
also covers delayed callbacks; real queue delay/data age remains unmeasured.

Enabled Defender, current local definitions1.461.181.0, scanned the final22-file
DLL/dependency stage before deployment: no threats, exit0, no matching detection
events, unchanged hashes. The existing protected exclusion-query and unsigned/
no-signing-certificate gaps remain in [security/distribution](../SECURITY_AND_DISTRIBUTION.md).
No security setting change, external upload, game probe, delegation, commit/push
or public release change.
