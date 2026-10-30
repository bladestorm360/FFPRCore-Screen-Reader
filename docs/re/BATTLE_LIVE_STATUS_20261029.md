# Current ally battle conditions

Human-initiated scoped follow-up to the closed ally-condition reader, October29.
Status: source/offline/FFIII delivery Closed in b61153fc/11357184 bytes; canonical
8290 native names/0 missing,142 used/349 defined/0 undefined, isolated smoke,
67 resources/12 locales and exact22-file Defender scan pass. Installed hashes,
144 protected/other5 games unchanged; recovery0a511191. Combined with the Doga
marker and Crystal Tower follow-ups. Full receipt:
build_check/crystal_tower_20261029/completion.json. Preserve the six-game audit.

Scope: ally targeting, H and number-row battle review must read current native
ally conditions. The user explicitly requests live state, including current
Reflect even between its animations. Names, rendered HP/MP, command ownership,
Autodetail, enemy disclosure and input controls retain their existing contracts.
No new poll, delay, text deduplication, native hook or retained native pointer.

The own FFIII log reports KO removed at460953.937 and Reflect removed
at460961.984, then reads Rae with Reflect at460976.718 and revived Robyn with
HP643/1430 and KO at460980.343/461003.625. The shared read_row currently trusts
the rotating rendered condition column for targeting and transient visible
effects for party review. These are not the current condition inventory.

Reuse the existing removal producer's current-condition list plus live battle
function conditions. Those sources already identify successful removals.
Read only the visible row's rooted BattlePlayerData on the native input owner,
bound both lists before traversal, and coalesce native condition IDs across
stacked functions. Distinct IDs retain independent entries even for equal text.
Preserve Critical's current displayed HP-color equivalent without inventing an
HP threshold. Remove obsolete presentation observers if no consumer remains.

Acceptance: actual-source all-six H/number/target regression for current Reflect,
removed Reflect and revived KO; both sources independently, overlapping IDs,
stacked/permanent functions, empty/malformed bounds, ownership and repeated
requests. Measure complete fake-native request episodes and sustained requests;
actual game-frame/audible cost remains unmeasured. Canonical checks, smoke test,
exact embedded data, final-payload scan and guarded FFIII installation follow.

## Implementation and verification

battle_party.cpp::read_row calls party_status.cpp::current_lines for all three
readers. CurrentConditionList is read through info_parameter/current_condition_list;
the same BattleUnitDataInfo provides its current BattleConditionFunction list.
These exact shared accessors already power removal_after in battle_message.cpp.
The immutable/rooted masters are consumed synchronously, never retained. Row
visibility, player/owned-character identity and existing native focus guards
remain authoritative; the FFVI repeated-target guard precedes status reading.

Both source counts must be0..128 before any entry is read. A fixed256-ID table
coalesces native identities, including overlap and stacked permanent functions.
Distinct IDs with equal localized text remain distinct. Null/invalid IDs and
unnamed bookkeeping conditions contribute no label. No native allocating union,
renderer hierarchy walk, worker pointer, cross-request status cache, timer or
wait is introduced. Critical4 remains owned by the current rendered red HP;
its persistent function alone is insufficient. KO comes from the current lists,
so a stale gray color or rotating text cannot preserve it after revival.

The two obsolete presentation observers (ChangePlayerMotions and SetConditionText)
and their16/32-entry caches are removed, along with effect/render traversal. This
removal follows the explicit human-requested source-correctness change, without
an efficiency-driven architectural refactor or changes to automatic add/removal
speech. Enemies and other menus retain their existing disclosure contracts.
All condition names use existing game localization/fallbacks; no new UI string,
setting or control. Existing twelve-language Critical remains.

Actual-source party_esper_check:36 before failures become0. Additional all-six
checks cover source-only conditions, current/function overlap, stacking, removal,
revival, native-ID versus text identity, null/oversized storage, hidden rows,
Corrad/Ryko independently carrying Reflect, row reordering and actor ownership.
Existing Esper buffers, formation, rendered vitals, charges and repeated requests
also pass. battle_focus_check passes unchanged, including existing native focus
guards and command/target ownership. No game was launched or probed.

One fake-native episode includes H, a number request and matching ally formatting:
1000 no-condition episodes15.379ms aggregate/43.7us peak; current Reflect15.727ms/
41.2us; the deliberately extreme128+128 distinct source case140.113ms/351.5us.
Boundary calls per episode349/355/2269. This includes the complete shared reader
and fake speech submission, not real Unity getters or audible completion. Each
explicit request reads once; no per-frame observer/backlog is added. These bounds
and timings do not certify real application lag below the user's cumulative25ms
threshold. Native/audio tails and natural gameplay confirmation remain separate.

Evidence: build_check/ally_live_status_20261029/ before_game.log, before.txt,
targeted_build.txt, after.txt and focus.txt. Full release receipts follow there.
The first canonical attempt caught equipment_party_check's obsolete isolated
helper signature; only that test stub needed updating. The rerun passes. No
failed payload was installed. Candidate11341312 bytes is frozen pending the
combined follow-up candidate; it was superseded by installedb61153fc. Recovery
is0a511191. Other games' installation remains pending after the full audit.
