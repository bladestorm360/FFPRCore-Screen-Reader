# Narshe chase and remaining interactions, November 3

Source/focused/documentation Closed; actual guide/planner/model/controller checks pass. The
single milestone record is MAP_AUDIT_FOLLOWUPS_20261028.md. This is separate
from the Closed six Locke doors and the already accepted light-path guide.
Combined Main output, security, delivery and natural gameplay remain pending.

## Complete native sequence

The new narshe_chase recipe covers treasure-room entry, its native timer/scene,
the empty chest, town pursuit, northern approach, cliff pursuit, the motionless
ledge wait and Mog rescue. Prerequisites are native138/207 before collapse297;
receipts208–215 select actual progress. The mutually exclusive alternative216
retires rescue guidance. Rewards are never added to treasure labels. The guide
uses existing Shift+G/controller controls and the shared Minimal default;
essential wait/action instructions remain in both verbosity modes.

The room's first native scene runs to208. On the ledge, sc_e_1055 sets local1
and starts the native five-second sc_e_0437 callback. Moving into any of five
relative movement detectors cancels it and clears local1. Guidance first routes
to the real trigger, then gives a motionless wait instruction while the native
timer runs. After214 it routes to Mog's actual interaction28, rather than the
alternate27. Native rescue215 finishes the guide. The mod neither estimates a
timer nor operates controls, moves characters, recruits them or writes flags.

Ten maps3/16/18/19/20 and northern mine323–327 are included. Map19 is the intermediate floor used by the
earlier defense. A narrowly opt-in select_when predicate gives the later chase
ownership only after207 and before297; false selection retains the earlier
defense, while unknown selection waits safely. All previous recipe values and
their relative order are retained. The guide count is19 within the existing32
bound. There is no unrelated guide-selection refactor.

## Physical routes and native ownership

The first actual route fixture exposed a remote-local dependency at the north
approach. Its trigger positions depend on that floor's local1. A source-matched
entrance waypoint reaches map18/PointIn1 before requesting its trigger. This
keeps unknown remote state conservative and refreshes from the current floor;
it does not force the actor active or relax collision. Treasure-room entry uses
the real Event197 transfer/PointIn1. Native terrain, passage data, objects,
arrival metadata and initializer programs are unchanged by this guide slice.

The model uses the existing owned flag/cell snapshot and production route
worker. No native observation, hook, polling, clone, write or worker-side native
API is added. Native pointer ownership, input/context cancellation, finite guide
and condition bounds, and existing speech interruption policy remain unchanged.
Four original wait/rescue instructions have all12 supported translations.

Source hashes and factual timer/receipt locators are in
[the manifest](narshe_chase_native_20261103.json). Exact recovered scripts remain
outside Git in the parent tools/puzzle_audit_20261030/ff6_narshe_interactions_20261103
archive. The production author, localization author, actual model/planner
fixture and explicit semantic reviewer are separate owned tools. Their comments
explain the stage dependencies; source-bearing fixtures are prohibited.

## Other exact Narshe discoveries

The remaining43 identities consist of20 chase operations/counterparts and23
other interactions. These include four action0/scriptless mysterious-esper
movie counterparts, one secret-passage control, guard/elder/mine/Shadow story
scenes, the native World of Ruin choice/reward NPCs and Mog's later recruitment.
Each has an explicit source-qualified disposition; earlier decisions are retained. A descriptive riddle keyword
or blinking animation does not establish a player-operated puzzle. Ordinary
single interactions and native choices keep their existing destination/focus
and story-state ownership. Native key/party locks are not inferred from labels.
The Closed light320 guide retains its15 stages, trap/restart handling and final
ready317. It remains distinct from later Mog recruitment on the same room.

## Verification and limits

Actual fixture and resource receipts are retained under
build_check/narshe_interactions_20261103. The initial fixture reader compile
error and failing remote-local journey are preserved. Assertions cover full
physical progress and continuations, both waits, native rescue completion,
alternate outcome, unknown state and earlier defense selection. Existing model,
actual controller and prior guide regressions pass for this source
checkpoint. Canonical build wiring retains authors/locale/semantic
and actual route checks. Capped fixture wall time is not a game or audible
response measurement. No game/probe/delegation is authorized or performed.

After combined delivery, natural play should confirm each chase scene, waiting
without movement on the ledge, and guidance toward Mog after the scene changes.
An existing partial chase should resume from its native receipts. No gameplay
test is requested before delivery; do not claim this behavior gameplay-confirmed.

All43 identities/43 physical instances are explicitly reviewed; audit totals
are1009logical/1041physical,350 FFV+294 FFVI=644 pending. Actual full journey16
assertions/11 native continuations and all current/older model/controller cases
pass. Final three-consumer batch237.877s, peak aggregate467341312bytes,
2GB/25%CPU/one worker,exit0/drained. Source and output identities, preservation,
failures and receipts are in completion.json. No Main release/install is claimed.
