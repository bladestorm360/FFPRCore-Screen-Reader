# Native runtime and shared service ownership

[October25 floor/map context follow-up](../re/NPC_FLOORS_SHOP_LANDING_20261025.md)
keeps bounded native observations and owned projection workers. Guaranteed facts
from completed initializer branches survive; unsupported or divergent actor
state stays conservative. Companion collision, staged-actor bounds, real-layer
interaction goals and route-only world borders share the existing navigation
epochs. Mounted landing cues require the actual manual vehicle. No new game-thread
wait, native pointer in workers, route-distance limit or scene scan is introduced.

[October25 startup investigation](../re/STARTUP_HANG_20261025.md) distinguishes
a running Boot frame loop from successful game initialization. Narrow native
boundaries preserve each original call and exception search; no startup
workaround/repair is established yet. Approved snapshot evidence is outside Git.

Current source audit: October22. One name-resolved native hid.dll proxy serves Steam
FF1–FF6. The paused managed repositories provide historical behavior/attribution;
they are not deployment targets. The shared runtime is implemented and offline
checked; gameplay coverage varies as recorded in TESTING. The feature index links
all registered modules and their game masks, not an assertion of all-game testing.

## Boot and engine boundary

The proxy forwards47 hid exports and chains existing native hooks through the
maintained MinHook coexistence route. Boot detects game identity, loads settings,
prepares workers and installs ordered core::Install registrations filtered by game
mask. core.cpp owns action/query registries, menu_state and events. refs resolves
classes/fields/methods by name from the matching IL2CPP runtime. hooks::attach keeps
original signatures/MethodInfo and failure logs. Folded methods require type guards
and exact ABI checks, not an assumption that a shared RVA means shared semantics.
CMake embeds owned catalogs, labels, maps, guides and sound defaults.

## Native owner and asynchronous pipeline

Frame owns Unity/IL2CPP observation and approved movement/input calls. Native event
hooks detect/copy ready state, call originals in required order and return without
waiting. frame tasks are event-started continuations, not a replacement polling
architecture. CaptureBudget slices bound traversal between native accessor steps;
individual engine calls can exceed that allowance. Data, Compute, Io and Navigation
workers have separate queues, with256 waiting jobs/lane and nonblocking try-lock
admission. No full-queue fallback executes work inline. Navigation has normal
priority; other background queues use below-normal priority.

worker_job::Owner retains native publication callbacks in an owner-side registry.
Workers receive owned values and an epoch, never those callbacks or their Unity
pointers. A busy queue retains the owned job for a later admission attempt. Epoch
changes discard obsolete computation/publication; safe lifecycle cancel removes the
retained callbacks. Transition admission only retires scalar generations/epochs;
large destruction and cleanup occur at safe boundaries. A feature must also bound
its retained/pending observations; generic queue capacity alone is insufficient.

say/speech owns ordered ingress, context generations, chunking and Prism output on
its exclusive worker. A true interruption begins a new generation; same-event
follow-ups queue. Mod-menu speech remains available while game speech is muted.
Settings getters/setters are native-owner state; immutable save snapshots go to Io,
without native waits. Each game has its own INI and one-time read-only import from
legacy preferences. Localization defaults to the game's UI language; manual override
is persistent. Worker LocaleScope uses a copied locale. Data preparation never
invokes thread-affine game lookup APIs from a worker.

## Cross-cutting contracts and evidence

- [Speech and backend ordering](../re/SPEECH_ORDER.md), [Prism](../PRISM_MIGRATION.md),
  [logging](../re/LOGGING.md), [dedup inventory](../DEDUP_INVENTORY.md).
- [Snapshot pipeline and timing](../re/THREADING.md), [stall investigation](../re/STALL_2026_10_16.md).
- [Displayed menu evidence](../re/MENU_DETAILS.md), [results](../re/RESULTS.md),
  [enemy presentation](../re/ENEMY_STATUS.md), [bestiary](../re/BESTIARY.md).
- [Map state](../re/MAP_STATE.md), [exits](../re/MAP_EXITS.md), [traps](../re/TRAPS.md),
  [events](../re/EVENT_CONTEXT.md), [tracking](../re/EVENT_TRACKING.md),
  [autowalk](../re/AUTOWALK.md), [soundscape](../re/SOUNDSCAPE.md).
- [Controls and contexts](../INPUT_BINDINGS.md), [user guide](../../readme.md),
  [verification/limits](../TESTING.md), [release/recovery](../release.md).

Read source locators in the feature index alongside these contracts. Native-name,
actual-consumer, lane, cancellation, route/map and output fixtures run via build.bat;
smoketest.bat verifies the isolated proxy. Neither launches a game. These establish
offline semantics, not audible screen-reader response or modded/unmodded frame cost.
Current source-bearing historical analysis remains a repository-content blocker
recorded in memory; new evidence is factual metadata/prose with source analysis
outside Git. Do not publish or rewrite historical ownership/content incidentally.

## Extension and organization

Add features to their cohesive subsystem and register only required native hooks.
Keep central speech/input/locale/snapshot services at actual call sites. New UI
must define context owner, readiness, controls/defaults, overload, cancellation,
all-language labels and verification gaps. New navigation identifiers may use
reliable developer/game data under the standing exception; route validity and
type-only treasure rules remain. Unrelated hidden information needs approval.

Most new feature files stay below roughly1000 lines. Existing baked_nav.cpp remains
a concrete exception: private route state, transfer phases, cancellation and native
snapshot/worker publication are tightly coupled. This task changes those shared
paths without an unrelated planner split; the focused worker/route fixtures cover
its boundaries. A future extraction should preserve those private interfaces and
actual-consumer tests. No new wrapper/process is required for every responsibility.


October24 budget review: battle_message.cpp is reduced to610 lines by replacing
per-game removal tracking with native reconciliation. stat_feedback.cpp and the
shared controls reader are cohesive separate modules. entity_scanner.cpp remains
an explicit1251-line exception: its exact selection anchors/revisions are used
inside the same private native-capture, worker and publication lifecycle. This
fix keeps those ownership boundaries together; pure filtering remains in
entity_filter.cpp. baked_nav.cpp's existing private-planner exception now also
covers mounted pad phases and shared list floods (roughly3300 lines). Pure pad parsing
and sparse indices live in baked_map.cpp. No unrelated split or wrapper was added;
future extractions must retain actual-consumer cancellation/lifetime fixtures.
