# Imperial Palace complete guide

The twenty-third FFVI guide covers the invitation/escort, timed soldier tour,
every banquet question, optional break battle, resumed dinner and final reward
scene. The earlier pre-invitation encounter181 is a separate ordinary story
event. Canonical scope remains the five frozen Palace discoveries; the connected
guide additionally qualifies all24 actual soldier controls across165–179.

| Native stage | Guidance and actual completion |
| --- | --- |
| Invitation/escort | Selected by226; approach165/27, wait for its real227 scene, then approach Emperor176/15. Native229 starts the240-second timer. |
| Soldier tour | Offer only still-unvisited230–253 targets on their actual floors. The ordinary production planner chooses the next reachable soldier. Four encounters award the talked-to receipt even on a native loss; the mod never fabricates victory or repeats a consumed conversation. |
| Finish the tour | After all24 are known complete, route to176/17. The current native Yes/No recommends returning to the tour until all24 are met. The native timer may also expire; missed soldiers are not silently marked complete. |
| Banquet | Shift+G reads the exact current native question. Toast/war/Doma/Kefka use their actual best option IDs. Ask all three distinct questions, then end that menu; recall the question actually asked first using native local4. Reordered options retain their current index. |
| Break | Choose the break, then approach one of the four real177 soldiers. The current native Yes/No starts the120-second battle. Native256 owns its result/timeout receipt; it is not inferred from the mod route. |
| Resume | Approach177/13 after256. Current questions include resuming, final words, cooperation and its confirmation. Full257 and the actual177/3 return precede the final route. |
| Final scene | Route to166/41. Only258 can finish, and the controller waits for field readiness after the complete native dialogue. Earlier255/256/257 never substitute. |

Navigation guidance owns the tour and its finish-tour choice. Non-navigation
guidance owns banquet/break answers and those associated objectives. Both use
the [independent type controls](PUZZLE_TYPES_20261104.md); Minimal still gives
the necessary action/directions without a step caption. A disabled stage is
withheld rather than skipped. Native rendered dialogue/cursor reading remains
available. Internal scores and future reward contents are not narrated.

`tools/author_palace_guide.py` pins90 connected native scripts and38 physical
controls in `palace_native_20261104.json`;16 native choice sites are qualified.
Source-bearing archives and disassembly remain outside Git under the existing
`tools/puzzle_audit_20261030/broader_audit_20261104/` research directory. The new
recipe does not alter terrain, native objects, arrival overrides or map state.
Its opt-in target predicates use the existing32-condition/32-target bounds;
all older targets retain their previous semantics.

The narrow observer in `src/games/ff6/palace/choices.cpp` attaches only to the
source-matched native External.Message.Select. Its real outer namespace is
Last.Interpreter.Instructions; nested Core/Format fields resolve through their
actual declaring types. `tools/re/palace_contract.py` explicitly checks these
nested bindings and matching DLL/dump/native provenance. The static native
one-argument Select returns the existing integer interpreter result unchanged.

Only Palace176/177 are watched. On the native owner and actual first clock
(counter.clockCount1), the observer checks the current-instruction/PC identity,
at most4096 instructions and an option-array bound of8, then copies at most
six32-character keys. A lookback of at most3 rows permits only intervening
Wait/Nop before the actual Msg. This distinguishes asking from remembering
despite identical option IDs, and identifies generic empty-ID Yes/No questions.
Unknown/mixed/duplicate/truncated keys withhold advice. Waiting ticks copy no
strings/arrays. No native pointer is retained, native choice/timer/state is
never written, and the hook submits no speech, worker task or timer.

Explicit Shift+G reads only the necessary current locals (at most3), one break
bit, or at most24 tour bits. Field routing still uses owned worker snapshots.
Map/battle/title boundaries retire the copied question; menu-open/readiness
guards prevent advice outside the current native UI. Type changes quietly
retire guide work/cache; enabling mid-question can use the still-current native
observation. Full scene completion uses the existing readiness continuation.

Focused evidence:159 actual native-observer/controller assertions pass,
including option permutations, eight question histories, real first-question
recall, generic Yes/No, unknown states, PC/string bounds, lifetime and both
type controls. Ten thousand waiting detours take1.970ms aggregate in the
throttled fake-native fixture, with zero string copies; this is not actual
game-frame or audible latency. The real-bake guide journey passes36 assertions,
66 route legs and883 cardinal cells, reaching all24 distinct soldiers and the
final reward. This proves route validity from the representative owned fixture
poses, not native240-second completion or actual speech/movement timing.
Current story model589 identities and retained Opera/controller checks pass.

Source/focused/documentation Closed. Native/locale/source-preservation/semantic
checks pass; exact source/report/executable/resource hashes and the retained
fixture compile failure are in broader_audit_20261104/palace_completion.json.
The successful route/model/Opera tree peaked at472219648 bytes under its enforced
2GB/25%-CPU slot. All22 older story recipes and the actual terrain/native
objects/state/arrival bytes are preserved. Combined DLL/build/smoke/security/deployment and natural
gameplay remain pending. When progression permits, remaining user confirmation
is to request Shift+G at each banquet question, then after the break and reward;
no new game launch or runtime probe was performed by the agent.
