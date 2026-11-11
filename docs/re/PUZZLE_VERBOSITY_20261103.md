# Puzzle verbosity and short route notices

Owner-requested November3 scope: all-six puzzle-guide presentation, plus the
shared trap/event notices used by turn-by-turn directions. Source/focused/documentation Closed;
all six actual consumer targets and all12-language checks pass. No new DLL or installation yet. The broader
map audit remains open in the single [scope ledger](MAP_AUDIT_FOLLOWUPS_20261028.md).

Use **F8 > Navigation > Puzzle guide verbosity**. **Minimal** is the default,
including existing configurations that do not contain the new preference.
Shift+G gives the current directions and necessary puzzle action. It omits the
guide activation, numbered step caption and Step done announcement. **Verbose**
opts into those metadata announcements where the guide has them. Stateful FFVI
guides have branching objectives rather than reliable numbered steps; Verbose
adds their activation/name/introduction without inventing a step count.

November4 owner-requested addition: on entering a map with an available guide,
both modes queue **Puzzle guide active** after the map announcement. Minimal
still omits activation/step metadata on Shift+G. The entry notice advertises
availability; it does not start the guide, request directions or move the player.
Known completed/gated static recipes and unselected FFVI optional episodes stay
silent. Missing FFVI selection flags cannot advertise a shared town/world map.
The [independent puzzle-type settings](PUZZLE_TYPES_20261104.md) select which
solutions are enabled. Verbosity changes presentation only within those enabled
types; ordinary native menu reading remains independent of solution guidance.

The shared `puzzle_guide.cpp::on_map_changed` readiness continuation owns this
notice and the existing active-guide progression. `puzzle_entry_notice.h` checks
immutable metadata within the64-static/32-story/128-map/32-step limits and caches
at most64 distinct one-bit scenario observations. It does not parse files, copy
party/actor state, compute routes, add a native hook or wait for worker work.
FFIII forms and FFV typed policies reuse their existing bounded map-only query;
FFVI uses actual selection predicates rather than its broad map registration.
Map-changing, battle and title contexts retire stale receipts, including a new
entry into the same map. Notice speech queues without interrupting the map name.

Focused source-to-output qualification:27 new entry assertions in the actual
`puzzle_guide_check`, its existing89 operation controls/52 recipes, and all-six
`mapstate_check` pass. Tests include all-six readiness/notice/no-activation/stale
cases and selected/unselected/unknown FFVI shared-world episodes. One new key
has all12 translations, checked by `tools/modtext_puzzle_entry.py` and canonical
`build.bat`. Exact source/report/executable/resource identities are retained in
`build_check/broader_audit_20261104/puzzle_entry_completion.json`. This establishes
offline behavior; actual audible/map-frame timing and combined installation
remain separate. The new notification work is entry-event scoped, never a
sustained per-frame scan.

Answers to choices, spell/form instructions, party switches, safe timing cues,
failure/recovery explanations and completion/cancellation information remain
available in both modes. Existing automatic progression and explicit repeat
controls are unchanged. Opera lyrics and timed crossings retain their actual
action cues. There is no new input, automation or text deduplication.

Required-route notices are now **Trap required** or **Event required** in every
game, independently of verbosity. The existing Mod warnings setting controls
them. A required trap takes precedence when both classifications are present.
Trap avoidance, story/item gates, routing and destination identity are unchanged.

| Responsibility | Current owner and contract |
| --- | --- |
| Saved setting | `src/core/ui/preferences.h`: `PuzzleGuideVerbosity`, bounded0–1, default0/Minimal. Generic settings storage owns persistence; no new file/schema or native read. |
| Settings control | `src/core/ui/mod_menu.cpp`: shared Navigation enumeration, keyboard/controller settings behavior inherited. |
| Static recipes in all six games | `src/core/field/puzzle_guide.cpp`: optional captions at the existing speech producer; first directions still interrupt once when the activation header is omitted. Native receipts, stale-route contexts and stop/readiness ownership retained. |
| FFIII forms | `src/games/ff3/puzzle_guide.cpp`: owned raw instruction plus bounded step/count. Native-menu repeats format the current preference without reading native party/flags or requesting a route. |
| FFIII circuit | `src/games/ff3/chocobo_circuit.cpp`: same shared preference controls activation/step/completion-of-step metadata; native circuit receipt remains authoritative. |
| FFV adapters and FFVI Opera | Already instruction/action based. Essential payloads remain; no new fabricated numbering or hidden producer. |
| FFVI story guidance | `src/games/ff6/story_guidance.cpp`: first activation metadata formatted on the owner after the existing owned worker result. No preferences/native map-name access on workers; unsafe-tile immediate crossing advice is not delayed by introductions. |
| Route notices | `src/core/field/baked_nav.cpp::route_warning`, plus the selected-trap fallback reason. Short labels replace the old sentences; the obsolete chest-name/object scan is removed because the notice no longer depends on destination type. |
| Localization | `tools/modtext_puzzle_verbosity.py`, five keys in all12 supported languages in `data/common/mod_text.json`. Existing Verbose translation reused; old catalog values retained. Canonical `build.bat` checks these keys. |

The setting adds only bounded owner-side preference/formatting work to existing
input/event publications. It adds no hook, poll, native capture, worker queue or
blocking operation. FFIII caches still retire with their guide; the shared and
FFVI existing context/preview/cancellation owners remain authoritative. Finite
guide/step limits and current native thread-affinity contracts are unchanged.

Actual-consumer qualification uses `puzzle_guide_check`,
`ff3_form_guide_check`, `chocobo_circuit_check`, `ff6_story_runtime_check`,
`modal_input_check` and `route_worker_check`. These cover unset Minimal,
explicit Verbose, first speech interruption, choice/menu repeat without native
reads, setting retention through all-six settings rebuilds, worker ownership,
and actual short trap/event/empty warning formatting. Reports and finite2GB/
bounded-CPU receipts are in `build_check/puzzle_verbosity_20261103/`.

Initial qualification caught a fixture expectation that assumed only two
Verbose messages. Real resumed Deist guidance also retains its existing Step
done metadata when skipping completed prerequisites. The assertion now checks
the activation and actual current numbered step while allowing that valid
existing Verbose behavior. No production change was made for that assertion.

The broad route fixture also exposed its old exact91/71 manual-arc/directional
census. Closed Darill qualification added463/20,22,23 (three directional turtle
arcs); Closed Sealed Gate added408/69 (manual fall with unrestricted approach).
Its expectation now explicitly adds3+1 crossings and3 direction masks. Existing
approach/landing assertions stay intact; no production routing change was made.

Full combined native build, smoke/security checks, guarded delivery and natural
audible/gameplay timing remain separate from focused offline qualification.
Resource-throttled fixture duration does not establish actual game responsiveness.

Final focused outcome: all six targets pass. Total enforced time across all
attempts was313.212s, peak aggregate tree memory481026048bytes;2GB/25%CPU
limits and child cleanup verified. Exact source/report/executable identities and
failed fixture attempts are retained in completion.json. These capped fixture
measurements are not game-frame or audible latency evidence.
