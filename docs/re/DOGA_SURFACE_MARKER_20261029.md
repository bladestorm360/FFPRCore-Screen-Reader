# Nautilus surfacing access to Doga's Village

Human-initiated scoped navigation follow-up, October29. Status: implementation/
offline/FFIII delivery Closed in b61153fc/11357184 bytes. Combined canonical,
isolated smoke,67 resources/12 locales and22-file Defender scan pass; installed
hashes/144 protected/other5 unchanged. Recovery0a511191 retained. Full receipt:
build_check/crystal_tower_20261029/completion.json.
Current scope/status is in the [audit record](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
The delivered provider, terrain, identity and native action design remain Closed.
A later source-proven UI integration correction is source/offline accepted:
plan_browse_snapshot previously marked its own terminal surface action indirect,
causing a reachable marker to say Not directly reachable. The exact marker’s
successful final manual action now retains the normal usable-distance annotation;
intervening actions/events, partial blockers and wrong vehicles keep their prior
restrictions. Actual provider→category→selected route→browse tests2 failures→0,
including zero-step arrival, wrong vehicle, all-five-game abstention,147-step
tunnel and post-surface landing. No new search, native work, input or speech
string. Receipt build_check/doga_browse_20261029/completion.json; final audit
canonical/security/delivery pending. Whole native-exit-category plus selected
route worker20 episodes total455.662ms/peak23.232ms; actual frame/audio unmeasured.

The user reports Doga's Village requires an underwater tunnel followed by
surfacing at its far end; no ordinary map transition/event marks the surfacing
location. Add an explicitly named synthetic surfacing destination from real
underwater/surface terrain and exact native position-transfer semantics. This
does not authorize changing ordinary village doors into aerial transitions,
inventing reachable water cells or bypassing native landing/ownership rules.

Acceptance: the actual owned snapshot lists the marker in the appropriate
category/filter modes, routes the mounted submarine through the tunnel to its
physical surfacing position, and explains the required manual surface action.
Verify the corresponding surface location admits Nautilus flight/landing/walk
to the ordinary village entrance. Preserve identity across refreshes and reject
unavailable/wrong-vehicle access; cancellation and all other games abstain.
No new native hook, whole-scene scan, timer or worker-native pointer transfer.
Keep complete game sources/native bodies outside every Git tree, with factual
locators and source hashes here. Localize all new mod-owned labels in12 languages.

## Source, geometry and selected route

Ordinary village entry is surface Map_10040/map4 GotoMap171 at150,203, leading
to village map151/Map_20320/PointIn1. Master title keyMSG_MAP_NAME_50 supplies
Doga's Village. There is no matching native Event on underwater Map_10030/map3.
The two actual256x256 terrain bakes have exactly four paired surfaceable cells
in this isolated air basin; nearest the ordinary door is150,204. Underwater
terrain30 is in submarine14's landing group9; corresponding surface terrain16
is the upgraded Nautilus9's dive terrain. The chosen point is not an invented
door, native Event, widened tunnel or bypass through a mountain.

Native submarine14/group7/type6 owns up_script360/sc_e_0346. Its map4 arrival
sc_e_0346_1 calls OnTheVehicle14, then the Nautilus9 flying-form service. Exact
OnTheVehicle9489E0 calls IMapAccessor slot87 SetPositionToTransportation, with
fromScript=true. FieldController's explicit implementation2870D0 reads that
transport's current transform position and applies the same vector to the
first party member/camera. This replaces the fallback PointIn4 location; it
does not send the player to the default33,216. ChangeMap9349A0, SetVehicle948D90
and the native landing278900/guide296900 paths are also archived with provenance
outside Git at ../tools/doga_surface_20261029/ff3/.

The actual planner routes from the prior own underwater113,118 origin to
150,204 in147 steps, including nonsurfaceable tunnel terrain29. After the
native same-position surface, Nautilus9 reaches the ordinary village door in
three physical steps with ConfirmLand before the walking approach. These are
real data/native-semantic witnesses; the sampled flag bank does not certify
an entire gameplay story trajectory or audible timing.

## Implementation and scope

nautilus_surface.h centralizes the exact map/cell and mod-owned negative/raw
identity. baked_nav.cpp::nautilus_surface_target adds one cancellable owned
A* on existing ready grids and copied native collision/applicability; no native
calls or new capture. Reachability modes require actually mounted submarine14.
Unfiltered Current map retains the physical marker with distance unknown when
not accessible. Other games/maps abstain. No bake or native vehicle/input data
changes. The fixed physical marker does not move when its selection is refreshed.

entity_filter appends the owned row to Map Exits without an Events duplicate.
entity_scanner publishes a map-lifetime-owned handle and soundscape exit with
Surface World destination. entity_nav recognizes the exact marker identity as
a mounted Spot; ordinary village Map Exits keep their required ground entry.
No new shared Entity/FilterRow field or target enum is needed. Existing bounded
capture/publication, request cancellation and selection identity stay central.
find retains the current submarine through the final tile, sets manual_after
and surface here. Native confirmation belongs to the player. Full directions
include the endpoint; shortened directions retain the existing segment limit,
and zero-step arrival explicitly says surface here. No automatic surface input.

tools/modtext_doga_surface.py supplies both new mod-owned phrases in all12
supported languages. No setting/control or polling/hook is introduced.

Initial actual category check1 failure→0. Expanded route/cancellation/ownership,
unfiltered/vehicle/other-game, actual scanner publication/selection/soundscape
checks pass (doga_surface_check, entity_scanner_check, entity_filter_check).
One temporary assertion incorrectly expected the
147-step route's endpoint inside shortened six-segment directions; full route
and zero-step instructions were already correct. That fixture assumption is
corrected without changing the established short-route formatting contract.

The actual source-to-owner test publishes the marker with no native event row,
restores the exact mod identity after recreation, retains one verified soundscape
exit and retires it on map change. Both reachable modes, unfiltered inaccessible
listing, wrong/unavailable vehicles, all other games and mid-A* cancellation pass.
No native API is called on the worker. Receipts are under
build_check/doga_surface_20261029/: before.txt/topology.txt, final_targeted.txt,
final_scanner.txt/final_filter.txt and native_extraction.txt.

Whole owned worker episode evidence includes the existing four native underwater
tiles (three grouped destinations), new marker, category building and selected
route:20 episodes577.608ms aggregate/47.463ms peak. Source-only comparison:
100 selected-route episodes797.931ms/9.193ms peak; marker+route1611.002ms/
27.027ms peak. These are cancellable asynchronous Navigation-worker computation,
not measured game-thread stalls, UI unresponsiveness or audible completion.
No new per-frame capture/hook/native work or blocking wait is introduced. Actual
frame/queue/audible tail and gameplay remain unmeasured; do not infer the user's
cumulative25ms application-lag limit is certified merely from worker placement.
No performance-driven redesign of the closed routing service is authorized.
