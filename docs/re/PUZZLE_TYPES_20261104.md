# Independent puzzle guidance controls

Owner addition, November4: keep building solutions for navigation, choice and
other map/story puzzles during the all-six audit. F8 → Navigation now offers
**Navigation puzzle guidance** and **Non-navigation puzzle guidance**. Both
default on and save independently. Minimal remains the verbosity default.
This addition does not add optional minigames to the agreed audit scope.

Navigation covers mazes, passages, spatial switches, doors and transformations
needed for traversal. Non-navigation covers answers, word choices, equipment
rules and other puzzle mechanics. Classification follows the puzzle operation,
even when reaching that operation also requires ordinary route directions.
For example the FFV clue sequence and alphabetical library-book arrangement
are non-navigation; the moving library passages and stone-chest layout are
navigation. FFIV's magnetic equipment sequence is non-navigation.

Mixed FFVI episodes retain their native order. Opera letters/catwalks and the
timed crossings use navigation guidance; lyrics/performance advice uses the
other toggle. Darill's password, Ebot's coral/party instructions and South
Figaro's disguise/password answers are non-navigation steps within a larger
navigation journey. A disabled unsolved stage is withheld, never treated as
completed or skipped. Solve it independently, then press Shift+G to resume the
next enabled stage after its real native receipt. The Palace guide will use
the same policy for its walking tour versus banquet answers.

Changing either preference quietly retires the current guide session, held
worker/route publications, guide previews and cached choice repeats. It does
not cancel an unrelated selected-object route. Ctrl+G retains its ordinary stop
behavior. With both types off, Shift+G says “Puzzle guidance is off” without
capturing party/state or requesting a route. Native menu text, visible prompts
and results remain accessible independently of solution guidance.

Map entry still queues “Puzzle guide active” after actual field readiness when
an eligible guide has an enabled type. Entry never activates or routes a guide.
If no enabled type remains on that map, the notice is withheld. A mixed guide
can advertise an enabled type even if its current stage belongs to the disabled
type; Shift+G then explains that the current type is off.

| Responsibility | Implementation boundary |
| --- | --- |
| Saved controls | `src/core/ui/preferences.h` and `mod_menu.cpp`; existing generic settings persistence and change notifications. No new settings file, hook or polling. |
| Exact classification | Root `guidance_types` in each game's `puzzles.json`, independent of source-qualified recipe rows. `tools/author_puzzle_types.py` explicitly checks85 current recipes/native policies, with exact raw instruction overrides for mixed steps. New guides require a classification. |
| Shared policy | `src/core/field/puzzle_policy.h`; bounded metadata selection, unknown data allowed only while both types are on. Disabling a type never discloses an unqualified custom solution. |
| Static all-six guides | `puzzle_guide.cpp`; type check at activation, progression and actual route publication. Existing native operation receipts remain authoritative. One settings-change action retires only guide-owned state and dispatches the game-specific retirement. |
| FFIII forms/circuit | Existing owners check navigation policy before capture/request and at route publication; the combined native owner quietly retires both. |
| FFV typed guides | Existing single guide owner checks the current policy before native actor/flag capture, worker publication and directions; menu repeat uses the same type. |
| FFVI story guides | Owned policy booleans travel in `story::Snapshot`. The worker selects only enabled operations and marks the required output-type mask. The owner rechecks it before publication; native choice repeats and timed cues retain their cancellation epochs. No worker reads settings or native pointers. |
| FFVI lyric choices | `opera.cpp`; disabled answers do not prepend a solution to the rendered focused lyric. Ordinary lyric/cursor reading and native success/failure/action prompts stay available. |
| Localization | `tools/modtext_puzzle_types.py`: seven keys in all12 supported languages, checked by the canonical build. |

The existing guide limits remain: static64 recipes/32 steps, story32 guides/
128 rules/32 offered targets, metadata128 definitions/64 instruction overrides.
Entry uses at most64 distinct single-bit observations. Type policy adds bounded
owned comparisons to existing event/input publications, with no new native
call, scan, queue, wait or per-frame observation. Native game responsiveness
and actual audible latency are not established by these offline fixtures.

Source/focused/documentation Closed. Seven actual-consumer targets pass:
`puzzle_guide_check`, `modal_input_check`, `ff3_form_guide_check`,
`ff5_library_owner_check`, `ff6_story_check`, `ff6_story_runtime_check` and
`ff6_opera_check`. They cover all-six saved controls/defaults/independence,
entry availability, disabled-stage ordering, manual-solve resumption, held
route cancellation, zero capture when both types are off, cached answers,
and ordinary rendered Opera choices without solution hints. Metadata/all12
locales and repository source-boundary checks pass. Exact source/executable/
report hashes and retained stale-fixture compile failure are in
`build_check/broader_audit_20261104/puzzle_types_completion.json`.
The complete initial check tree peaked at396255232 bytes under its enforced
2GB/25%-CPU slot; the corrected Opera rerun passed under the same limits.
These are fixture measurements, not actual game responsiveness.
Combined DLL build, smoke/security checks,
deployment and natural gameplay confirmation remain separate and pending.
