# Command reset spoken before next turn

Source/offline/FFIII delivery Closed in75de3594/11340288 bytes. All-six before48→0
and synchronous actor-handoff case pass; existing focus checks0. Full serial
build142seconds/smoke/8419 names0missing/67 resources/12 locales/22-file Defender
scan pass;22 installed hashes match,144 protected/other5 installations unchanged.
Human confirmed game closed after its first running-game guard blocked. No game
launched/probed. Listening/gameplay confirmation remains pending. Exact receipt:
build_check/ff3_oct29_followups/completion.json. Broader map audit remains open.

Human-scoped FFIII follow-up: selecting an ability without manual targeting
(Boost/Guard) can announce Attack before the next character's turn. Reopens only
command focus/commit ownership; no battle architecture refactor or content-based
deduplication. Sol only; no game launch/control/probe. Estimate20–40minutes plus
combined checks. Native/source/targeted offline checks complete. Installed FFIII
ad0d80c1; combined checks/scan/delivery pending.

Own latest populated FFIII log ffpr_screen_reader.1.log shows Boost at438514.593,
an orphan Attack at438517.796, then Robyn's turn/legitimate initial Attack at
438517.812. Additional Boost/reset cases occur at438881.765/438881.921 and
439138.984/439139.468. This is separate from executing an Attack action.
The current SetCursor postfix observes an active Unity command controller with
its old turn window still open; the existing target-backout deferral is not armed
for commands which never open targeting. Identify the native commit producer and
scope its reset before speaking, rather than suppressing the word Attack.

Exact native source/provenance stays outside Git in
../tools/command_commit_20261029/. Acceptance must preserve real cursor movement,
normal initial Attack after a turn heading, cancel/backout, both immediate and
deferred native handoffs, non-targeting abilities and all applicable game paths.
Do not ask the player to review code or perform an unsafe capture.

Prior combined build22383 linked the DLL but failed a separate scan-only offline
host: its deliberate action-effects stubs lacked the two new artillery identity
functions. Those stubs now abstain explicitly; production classifier/tests are
unchanged. The canonical suite must pass after this follow-up; nothing deployed.

## Exact commit producer and implementation

NormalUpdate writes SelectedCommand before invoking the selection callback, but
that field alone does not establish cancellation lifetime. No field-based focus
filter was added. FFIII OnCommandSelect directly inlines CommandDone's forwarding
path for self-targeting commands; hooking that named helper would miss exactly
the reported cases. The actual accepted-action callback is BattleMenuController's
instance method taking List<BattleActData>, installed by CommandSelectingInit.
It forwards the accepted list and changes the parent state. Its native original
owns teardown and possible actor handoff.

All-six matching dumps/native bodies are checked, each unique (no folded alias):

| Game | Native method suffix | RVA |
| --- | --- | --- |
| I | b__71_0 | 43A260 |
| II | b__71_0 | 62F140 |
| III | b__71_0 | 6A1B90 |
| IV | b__73_0 | 39B3C0 |
| V | b__72_0 | 43DEB0 |
| VI | b__77_0 | 8BA020 |

The full method name is <CommandSelectingInit> followed by that suffix. Ref
classes have exact game masks; one method/hook is selected per game. The prefix
closes command speech ownership, clears the pending backout arm and retires its
generation before calling the native original exactly once. Native exceptions
propagate. No postfix can close a synchronously initialized new actor. The next
actual SetCommandData reopens input normally; manual target/submenu cancellation
retains its established event-owned return path.

No native metadata/data read, allocation, text comparison, new timer, frame poll,
speech output or worker wait occurs in this callback: it changes owned flags/
generation and existing command state. One callback per accepted action; startup
adds one uniquely resolved hook per running game. Runtime/native/audible timing
remains unmeasured. command_commit_check.cpp uses the actual registration, cursor/
turn and deferred-backout consumers:48 before failures→0, including all-six
inline/later reset, four-frame delayed handoff, real cancel, pending-cancel
retirement, next actor initial Attack, native once-only and exceptions. The
additional synchronous-new-actor case awaits the combined run. Existing actual
battle_focus_check (target/cancel/field/battle return) remains0. Evidence:
build_check/torna_world_20261029/command_before.txt, command_after.txt, focus_after.txt.
