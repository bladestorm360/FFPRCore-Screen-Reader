# Floating Continent complete map and story guide

Source/focused/documentation Closed. Actual guide/planner/model/controller,
semantic, canonical wiring and preservation checks pass. No Main DLL,
installation, game execution or gameplay confirmation yet. See the single
[audit ledger](MAP_AUDIT_FOLLOWUPS_20261028.md) for the wider assignment.

Shift+G follows the normal switch/lift/path journey, four treasures, Ultima
Weapon, confrontation/collapse, timed escape, Nerapa, native airship choice and
catastrophe continuation. Ctrl+G stops. Shared Minimal verbosity defaults to
directions and essential actions; Verbose adds metadata. No input automation or
timer manipulation is added. Treasure labels remain type-only.

The normal airship prompt must be declined to continue toward the summit.
Choose its second option. Native staging moves the player and then restores
Upper70,37 on decline. Upper70,43 belongs to departure, not the stay branch.
The guide first tries path marker74, then offers the blocking prompt if needed.
An owned XY predicate removes that receipt-free fallback at its actual restored
tile; the controller cannot keep prioritizing the finished prompt indefinitely.

During escape, collect chest10 from the east before either21/22 changes the
stage to11. The latter closes native walls24–27 and cuts off the approaches.
The guide requires the actual treasure152 receipt before recommending those
collapse triggers. A resumed stage11 with the chest missed continues the escape
and explains the lost approach. No collision/terrain relaxation is used.

The alternate west collapse31 sets299 and spawns encounter18. Its native victory
records area7=1. A real resume check initially failed because that branch was
missing; the guide now clears18 before returning through29 and the chest/exit
sequence. The normal safe route never invents encounter18 when it is absent.

At the final airship prompt choose the second option to wait for Shadow and
remain at the edge. The first wait sets296; actual arrival sets298. The native
timer/callback retains ownership; the guide does not guess how many seconds to
wait. Shadow's arrival and the temporary airship119 are not final completion.
The exact sc_e_0607_1 through _13 continuation passes119,431,605,608,334,582,1,
584,2,192,190 and reaches ready191/native297. Existing completion ownership
preserves those reviewed intermediate maps and waits for native field readiness.

The save-room432 guide returns through the actual431 PointIn3 at Upper90,50.
Its factual entry is Default9,11. An earlier working note called the return
PointIn2; the source has no such marker. Exact source lookup corrected the note
and recipe before qualification. The existing bake already has the correct
PointIn3/continuation; no new save-room geometry or arrival override is needed.

| Responsibility | Source and contract |
| --- | --- |
| Authored objectives | `tools/author_floating_continent_guide.py`,18th story guide on431/631/432. Both shared-geometry aliases select phase by actual receipts. All17 older story/five static recipes and prior catalog values preserved. |
| Rule evaluation | `src/games/ff6/story_model.cpp`; existing bounded32-guide model. New player_x predicate reads the existing owned cell, with no new native capture. Native flags/area/treasure receipts own advancement. |
| Live guide and choices | `src/games/ff6/story_guidance.cpp`; existing Navigation worker and native owner. Exact431/631 controls155/11 reuse the512-character owned instruction cache during native choices. No snapshots/routes/native reads on those repeats. Map/battle/stop/title ownership remains. |
| Initializer projection | `tools/author_floating_continent_state.py`, exact hash guard in `tools/bake_map_state.py`: four native scenery/timer-registration initializer operations are projection no-ops. Real game timer and scenery still execute. Other state code and rooms unchanged. |
| Automatic passages | `tools/author_floating_continent_passages.py`: twelve exact native transfers/arrivals. Same-map encoding preserves431/631 identity, current native layer and all collision. Native geometry/objects/masters remain; only12 Warp/12 graph-only PointIn rows added. |
| Native evidence | `floating_continent_native_20261103.json`: exact script identities, two choices, save return, irreversible chest and final continuation. Raw recovered bodies/disassembly remain outside every Git tree. Existing FFVI build-matched timer/scenery evidence is in the external `ff6_floating_continent_20261103` archive. |
| Language | `tools/modtext_floating_continent.py`: three new stay/wait/missed-chest instructions in all12 languages; no unrelated catalog edits. |
| Physical qualification | `tools/floating_continent_route_check.cpp`, `tools/floating_continent_cases.json`: actual authored model plus production planner, native-state blockers and real bakes. Four complete alias journeys, save return, missed chest, alternate collapse and waiting resume. `--diagnostic` retains the earlier investigative census. |

Eight actual guide/planner cases cover66 actions, including full16-action normal
and12-action escape journeys on both aliases. The save-room case validates its
real transition arrival; alternate-collapse and native-wait cases retain actual
state and collision. Actual `ff6_story_check` validates all prior guides/current
target identities, every new state, unknown receipts and completion. Actual
`ff6_story_runtime_check` covers choice-repeat ownership on both aliases and
all11 intermediate maps through ready191; stop/map/modal guards remain.

The guide adds no hook, timer, poll, native pointer on workers or input action.
Existing copied cells/flag snapshots and context/preview cancellation are reused.
The new XY guard and finite rules are evaluated on owned worker data. All build,
extraction and fixture trees retain shared2GB aggregate/25%CPU/one-worker limits.
Resource-throttled fixture times do not establish audible/game-frame responsiveness.
The cumulative25ms application-lag rule remains, with natural gameplay separate.

The initial20–30min source/physical estimate was exceeded while native ordering
and return/escape branches were settled. Original failed orderings, wrong landing
hypotheses and the real alternate-encounter failure are preserved under
`build_check/floating_continent_guide_20261103/`. No terrain adjustment was used
to conceal those counterexamples. Older Closed designs are retained.

All69 original discoveries/69 physical instances have explicit source-qualified
dispositions; earlier decisions remain unchanged. Current audit coverage is
960logical/992physical, with350 FFV/343 FFVI candidates pending. Canonical build
retains the guide/state/passage/locale/semantic checks and actual route fixture.
Final integration41.612s, peak aggregate tree288149504bytes,
2GB/25%CPU/one worker,exit0/children drained. Exact identities and failed attempts
are retained in completion.json. No combined Main release/install is claimed.
