# Zozo clock, native choices and hidden reward

The old static solution correctly identified the clock and6:10:50, but ended
at an opened-door waypoint. The new sixteenth FFVI story recipe continues to
the actual hidden chest. Native treasure115 completes the objective on both
the solution floor386 and hint floor387, including a return after map-local
state resets. Contents remain type-only. The fifteen earlier story guides and
all five static recipes are retained unchanged.

## Native controls and progress

Map_30511_3 is386. Its real solution control is Entity11 at Ground7,20;
Map_30511_4/387 Entity3 is a message-only hint clock. The guide approaches the
real386 Point1 from the hint floor, then resolves current local actor state.
It never borrows map-local1 from another floor to declare the wall open.

Native sc_e_0299 starts local1 at zero. Its three selections have6/5/5 options.
Exact branch/counter operands prove that only zero-based2/0/4, spoken positions
3/1/5, increment all three correct answers. All150 combinations are checked;
only that sequence reaches local1=3. Partial values1/2 are scores rather than
the current question, so the mod never mistakes a wrong answer for a menu phase.
The existing solution6:10:50 is preserved in all twelve supported languages.

The successful native sequence hides wall actors5–9, solution clock11 and the
inside contact event10, and moves animated wall16. The real initializer restores
their actual closed/opened state on entry. No baked wall or collision is removed
to manufacture a route. Chest14 is at Ground20,14 and owns treasure115; local3
alone, proximity, arrival at the doorway and the animation are not completion.

Native sc_e_0300 is a legitimate inside-opening contact that sets local1=3 and
opens the same wall. A native Point2 arrival is already on the reward side.
The guide retains ordered clock/reward candidates so the production worker can
choose the actually reachable operation. Closed-side chest routing fails;
open-side clock routing fails after the clock is hidden; a player already behind
the wall reaches the chest without a detour through the outside control.

## Controls, ownership and bounded work

Shift+G provides the current route and instruction. At the clock, it gives
6:10:50 and positions3/1/5. While the native choice/dialogue owns focus,
Shift+G repeats the last successfully published instruction. It does not route,
advance the menu, read flags, submit a worker or suppress repeated speech.
The existing bounded512-character owned solution cache is extended only to
exact386/Entity11, alongside the accepted466 tomb control. Other destinations,
the hint floor, the chest and changed map/load contexts cannot inherit it.
Stop and existing request-generation guards still retire stale output.

The model and actual route computations continue on the existing Navigation
worker with owned flags/positions and immutable bakes. No native hook, pointer
transfer, timer, polling, game input, shared gate/vehicle change or new flag
observer is added. The existing native treasure snapshot supplies115. Normal
native field readiness and the controller retain phase ownership.

## Evidence and verification

Primary native bodies stay external in ../tools/mapdump/ff6_scripts.jsonl.
Factual source hashes, choice positions, native local and treasure receipts are
in zozo_clock_native_20261103.json. Original source author/choice verifier and
the exact ten-discovery semantic author are in tools/author_zozo_clock_guide.py,
tools/zozo_clock_choice_check.py and tools/review_zozo_clock.py. The remaining
six Zozo story/reward candidates are explicitly outside this finite clock slice.

The real production planner passes12 entrance/local/closed/opened/inside-resume/
ordered-target cases and two complete floor continuations. The actual model
resolves282 identities without failures. Actual controller and shared input
checks pass, including repeat without native flag reads/routes/workers, changed
goal/floor/load rejection and all earlier tomb/roof/Owzer cases. All89 static
controls in52 recipes remain covered. Full source/locale/native-name/wiring,
all150 choices and earlier guide/catalog/review preservation checks pass.

The first route fixture incorrectly requested the remote clock directly from
387, whose remote local state is unknown, rather than the authored real entrance
waypoint. Its two failed assertions remain archived. Correcting the fixture to
the actual shipped recipe passes all cases without a production geometry fix.
The initial15–20min source proof completed; full focused integration extended
the slice for actual consumer/controller/input checks, without a new design branch.

Focused integration122.781s/444719104B and source/preservation18.481s/285908992B
both exit0 with children drained under shared admission, aggregate2GB/25%CPU/
below-normal/one worker. These capped fixture figures do not establish actual
game-frame, usable navigation or audible response timing. No game, probe,
new native DLL, installation or agent was used. Exact evidence is in
build_check/zozo_clock_20261103. Combined native output/security/delivery and
natural gameplay remain pending; the source/focused design is Closed.

Ten exact original discoveries bring coverage to822 logical/854 physical;
350 FFV and481 FFVI candidates remain. This does not close the broader audit.
The current16-guide bounded model capacity is full; any actual additional guide
must retain a finite validated bound and all earlier model coverage.

After final delivery, use Shift+G from either clock floor. Choose3, then1, then5
when the hour/minute/second prompts appear; Shift+G can repeat this instruction.
Once the wall opens, request guidance to the chest and collect it. Completion
must follow the native collection, and returning with the chest already opened
must not ask for the clock again. No hidden coordinates or visual inspection
are required. The separate user-requested FF3 door/key regression verification
follows this slice and does not require replaying FF3.
