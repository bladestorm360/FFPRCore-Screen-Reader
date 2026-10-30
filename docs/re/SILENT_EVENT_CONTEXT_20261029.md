# Silent map scripts: physical mechanisms, travel and narrative scenes

The [single audit checkpoint record](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger)
owns current scope, ordered work and closure. This topic explains the implemented
contracts and evidence. No game was launched, controlled or probed; gameplay
timing/confirmation remains separate. Installed FFIII b61153fc is unchanged.

## Player behavior and contextual decisions

The finite245 silent families in the original954-family inventory now have
complete contextual dispositions:41 narrative,66 physical,123 ordinary transfers,
3 conditional transfers,6 already owned exits and6 already owned providers.
Silence alone is never a physical classification. Silent character discovery,
rescue, memory, destruction, attack and escape staging retain Scene trigger.
Named functional labels reveal neither rewards nor later story outcomes.

Reviewed mechanisms include FFV bookshelves, moving passages, gates, doors,
currents, vents, coffin/floor hazards, timer boundaries and puzzle contacts;
FFVI light-path controls, scope views, collapsing ground, wall controls, a moving
chest and entrance/barrier cues. Ordinary travel appears in Map Exits wherever
the reviewed native transfer has one usable destination. Conditional manual
events keep a functional Passage label and their native event behavior.

An inactive FFV Pyramid marker carries a copied, unrelated master script ID.
Its action is0 and it has no derived Warp; the all-six audit finds no Action0
Warps. Its physical slippery-floor name is retained through an exact identity
review. The dormant ID is not claimed as executable travel evidence. No event
was pruned for lacking a narrative source, and no phantom-route repair was added.

Some III raw actors remain Scene-labelled in the catalog because accepted
continent/vehicle providers or existing underwater exits own their publication.
Their existing suppression/ownership remains intact; raw catalog text alone is
not a missing destination. Actual native and remote producers are checked.

Controls/settings are unchanged. Functional terms use the existing Auto/manual
language system and12-language catalog. Nine new terms are localized in every
supported language:Checkpoint,Service area,Puzzle trigger,Timer start,Timer stop,
Moving passage,Coffin,Quicksand,Light path. No internal identifier is spoken.

## Authoring and category producers

`tools/review_silent_families.py` is a finite, explicitly invoked authoring tool.
It reads the original245 inventory and matching complete source/continuation
hashes, applying the reviewed contextual decisions. It is not runtime inference
and cannot admit arbitrary future families. `tools/silent_family_decisions.json`
contains factual locators, hashes and mod-owned dispositions only. Original
scripts/objects and native bodies stay outside Git under `../tools/mapdump/`
and `../tools/map_event_audit_20261028/`.

Exact occurrences join `tools/world_event_reviews.json`; existing
event_label_identities/audit_event_labels generate game-specific contextual labels.
The existing scripted_transition_identity authoring path checks complete graph
hashes and the actual first Main transfer before adding an ordinary-exit flag.
Map setup functions that contain unrelated later exits do not become additional
destinations of that entrance. The current same-target Main branches preserve
their original arrival script and native PointIn.

Native goto_map_detector uses the prepared exact per-ID/raw/cell exit index,
before generic EventTriggerDetector. RemoteBuilder/build_remote use the same
ordinary-exit identity, and all other reviewed labels use the shared safe label
reader. No new runtime reader, native hook, class lookup or script parser added.

## Three real conditional FFV doors

| Native actor | Native ScenarioFlag1 | Flag0 branch | Flag1 branch |
| --- | ---: | --- | --- |
| Map_30031_1 actor1 | 25 | map198, PointIn2 | map211, PointIn1 |
| Map_30151_1 actor10 | 64 | map250, PointIn2 | map296, PointIn2 |
| Map_30161_1 actor4 | 125 | map1, PointIn26 | map19, PointIn31 |

The old bake retained only the first Main ChangeMap. Each exact source now has
one companion arc for its second native target. Arrival continuations contain
fade/save/music/map-script maintenance, with no player-position override.
The Jacohl departure prefix first removes its teleport cache and stops two named
gimmick timers. Native Current.ResetJacholCaveGimmick at RVA2BFF50 confirms timer
removal, without changing the tested story flag; exact native evidence is in
`../tools/conditional_arrivals_20261029/ff5_door_cleanup/`.

Existing bounded entry_guard projection compiles each target's native prefix.
Six destination-specific rows are added to exactly three existing FFV state
records; every other field/program/record remains unchanged. Resolver::consumed
applies a matching destination guard only to its derived Warp, never the native
physical Event. Scoped guards take precedence over generic entry availability.
Known flags select one real arc; unavailable flags preserve both possibilities.
The live Warp match also skips a consumed companion so the second branch retains
its powered terminal boundary. No global story flag or native property is written.

## Complete FFIV Devil's Road landing

The native script chain enters a transient world scene, then Baron map29. Both
final player tracks set6,6 on Bottom; native PointIn1 is6,12. Routing through the
intermediate world failed the actual indoor planner, and merely changing to the
final ordinary PointIn would retain the wrong landing.

The exact complete source review resolves the final transfer and reuses the
existing warp_arrival_overrides contract for one virtual PointIn at6,6. Native
PointIn1 stays6,12. Locked ScenarioFlag1/29 still blocks access; both native
arrival branches (flag33) use the same proven endpoint. A fresh final-map capture
retains the selected destination. This adds no movement/input or native action.
Source authoring: `tools/review_devil_road.py`; exact native Puppet producers/
provenance: `../tools/conditional_arrivals_20261029/ff4_devil_road_native/`.
Native intermediate loading/animation timing still needs gameplay confirmation.

## Underwater entry preservation

Six exact FFV underwater scripts dismount inside the entry. When promoted to
ordinary exits they receive the existing submarine-entry flag, preserving mounted
access instead of ordering a ground landing. Their native transportation models
are8/13, type6, group7. No sea/land grid or transportation master is changed.
Actual route and exit-eligibility tests use adjacent legal native water tiles.

Player1 movement layers also exist in these bakes. An initial extra test wrongly
assumed hypothetical foot routing must fail. The archived pre-change bakes prove
the same six foot approaches were already reachable. The corrected control checks
before/after parity; it does not invent a new vehicle restriction from map names.

## Acceptance, bounds and remaining limits

Evidence is retained in `build_check/silent_context_20261029/`:

- Native detector and actual remote publication:381 exact current reviews,
  255 transfer rows/126 mechanism rows,0 failures/0 owner violations. Initial368
  rows had756 before assertions; later additions have separate source proofs.
- Three conditional doors:actual first-entrance plans3/6→6/6,10 before assertions
  →0; known/unknown flags, native door retention, chosen incoming receipt and
  both powered terminal branches pass.
- Devil's Road:0/2→2/2 actual plans,5 before assertions→0, locked controls,
  exact final receipt and fresh-map continuation pass.
- Six underwater mounted routes/exit batches and original foot parity pass.
  Current owned test total43.437ms/peak11.233ms; this includes route/batch/control
  computation, not native game-frame or audible latency.
- All245 contextual source closures and all41 narrative-label families pass;
  1362 exact functional occurrence/source/continuation/12-language checks pass.
  All-six actual planner (98 vehicle models/582 world targets) and native selector/
  pressure-switch regressions pass. Canonical build now runs the new consumers.

Strict delta validation preserves terrain, dimensions, transportation and map
masters, native objects and all unrelated arcs. Changes are134 ordinary-exit
promotions,3 conditional companions,1 explicitly resolved chained destination
and1 virtual final landing; six FFV submarine flags are exact reviewed entries.
FFIV state is byte-equivalent; FFV adds only the six targeted branch guards.

Existing graph400-state, initializer8192-instruction/64-branch and entry-prefix128
node bounds remain. Conditional metadata adds two guards per affected floor.
There is no new hook, capture loop, native invocation, worker-native pointer,
game-thread wait or eager projection. Native observation counts remain scoped
to the existing scanner. Source-to-owned-output checks do not certify actual
combined native/frame/audio cost; the cumulative25ms application-lag rule applies.

Development checks use retained exact before bakes/reports as private evidence;
none contains copied game script bodies in Git. Final combined canonical/security/
applicable delivery is separate in the single checkpoint record. Further changes
must preserve native actor/destination identity and genuine source ownership;
do not guess a conditional target or label a silent actor as physical by default.
