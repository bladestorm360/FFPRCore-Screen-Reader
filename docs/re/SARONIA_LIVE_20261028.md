# Saronia live Events failure and remaining barrier labels

Superseded acceptance: the user reports the deployed result still fails.
The actual powered-trigger cause and complete-capture reproduction are in
[the reopened investigation](SARONIA_CAPTURE_20261028.md). Passing the earlier
manual-state/mocked-target fixtures did not prove live appearance. The latest
human request also resumes the all-six audit after verified FFIII redeployment.

Human reopened the deployed Saronia fix: still absent in Events with Current map
with vehicles, StoryGateFilter=false, EventGroupFilter/MapExitFilter=true. Rename
the event Saronia Airspace. Three Scene labels remain among airship barriers.
Source/delivery complete, Closed; estimate30-45minutes, completed within estimate.
Sol only. All-six audit explicitly deferred;
no game launch/control/probe. Fix this narrow regression and redeploy FFIII.

Own log session352167578/pid14000 confirms the new build/data are loaded, Events
has20 rows and no Saronia entry; ordinary castle browse still proposes landing.
Source of the missed test: earlier Saronia fixtures directly constructed a
Resolver with all-zero flags and no native observed-group cache, and tested only
Enterprise6/13. The real default Resolver replaces projected selection with
observed additive groups marked complete. A partial observation therefore can
exclude native initializer group ev_e_0078 even while copied ScenarioFlag1/73
is false. Native world initializer activates that group solely on flag73=false;
that exact prerequisite is available independently of the partial group cache.
The observed-group generalization is a separate audit finding; this fix will
use the exact reviewed Saronia prerequisite without refactoring shared state.

The native Saronia Event has no target_transportation_ids restriction. Enterprise
also has the native15/16 flight/ship pair, omitted by the prior6/13-only rule.
Use the actual Enterprise mode and the existing two-pair conversion planner;
do not guess that every Enterprise uses the same master ID. Native travel/ground
gates and ordinary castle/district endpoints remain unchanged.

Remaining co-located Scene labels are statue-barrier controls (scripts230–233,
sc_e_0088/sc_e_0328/sc_e_0329/sc_e_0330), distinct from aircraft repel scripts.
Full scripts check a Fang flag, remove the barrier when satisfied or repel the
player with a generic failure line. No narrative dialogue/plot scene. Classify
Statue barrier without naming rewards or conditions/outcomes in the event label.

## Implemented correction and before evidence

Before production changes, the new actual-source Saronia fixture fails16 checks:
all four Enterprise forms lose the event/route when the selected group cache
contains only a later observed group. It uses the user's(114,108) position,
Current-map-with-vehicles behavior and StoryGateFilter=false. The separate
actual native default-Resolver fixture confirms copied flag73=0 while generic
group-active=false after observe_current(4,ev_e_0185). Evidence:
build_check/saronia_live_20261028/{before_build,before_report,native_cache_report}.txt.
No game/control/native probe was involved.

planning_object_active now uses the reviewed copied scenario73 prerequisite
only for exact FFIII/world4/script120/raw identity. Known completed stays absent;
unknown retains prior conservative state behavior. Discovery, route classification
and distance filtering share this decision. Other events/gates keep their
existing state resolver. The general partial-group-cache issue remains recorded
for the deferred audit rather than silently refactoring shared state here.

The Saronia rule accepts flight6/15, using existing13→6 and16→15 takeoff. Candidate
selection no longer applies the continent-specific aircraft whitelist to this
distinct story entry. The event name is Saronia Airspace in all12 languages,
independent of an ordinary castle exit/name surviving another filter. Owner
publication/selection/audio still uses the existing owned Event lifetime.

Four exact statue-control occurrences are pinned by full-script hashes in the
existing review file. Their only Msg is the common barrier failure line, with
no narrative conversation/encounter. All21 reviewed functional occurrences and
12 locales pass the updated checker. No flag, vehicle, collision or bake write;
no new hook, native getter, timer, polling or queue. Availability adds one copied
optional-bit read on the worker only; both flight variants reuse existing bounded
map searches/cancellation. Actual native/frame/audible timing remains unmeasured.

The first correction fixed discovery but the expanded fixture still caught eight
selection-route failures: route_to_goal's independent lifetime guard used the
generic partial group cache. That guard now uses the same exact Saronia decision
for this current-map scene. first_fix_build/report preserve that failure; no
intermediate candidate was deployed. The final fixture passes0 failures, both
discovery and selected airborne routes for6/13/15/16, completed exclusion and
ordinary doorway identity/ground gate preservation.697.752ms aggregate includes
all positive/negative route/category cases on offline workers; it is not a native
callback/frame or audible-response measurement. Wrong-owner calls0.

Final canonical8321/0 names,142/349/0 wiring, owner-publication/selection/audio
fixture,21 functional-label cases, smoke47,67 exact embedded resources,
136 indexed registrations, six unchanged bakes and12 languages pass. Final DLL
11096576bytes SHA2566785a04e77ff2903898c9960eac3fcd1abf369bbcb9efbc2e31a09f9310ce9b4.
Defender reports no threats in22 exact payload files; exclusions query OS-denied,
coverage unknown; no signing certificate or protection change. FFIII only was
redeployed/hash-verified:22 installed hashes match,144 protected files unchanged,
other five games' payloads unchanged. Recovery/complete receipts:
build_check/saronia_live_20261028/. No game was launched/controlled/probed.

Gameplay confirmation remains user-side: while flying Enterprise, select Events
> Saronia Airspace. It should appear with Current map with vehicles and story
filter off, then guide into the native scene without landing. The three remaining
statue controls should read Statue barrier. Broader audit remains explicitly
deferred; do not resume it automatically after this deployment.
