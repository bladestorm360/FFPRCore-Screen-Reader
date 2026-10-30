# FFIII vehicle and connected-world journeys

User-triggered continuation of the single [scope ledger](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
Source/offline/all-six delivery is Closed: a6d9b4ce19dfb6ad/11460096 bytes.
Completion/recovery: build_check/navigation_delivery_20261030/completion.json.
Gameplay/frame/audible confirmation remains separate. Sol only; no game launch
or native probe.

## Same-world approaches and parked craft

The actual planner reproduced a34-tile walk after an Invincible landing for
both151,74 and46,57 origins to the south Maze entrance. Landing197,42 leaves
one walking tile at unchanged total81/265 steps. The existing preference for
fewer mountain actions is preserved; equal-cost non-sailing landing ties now
prefer the shorter final walk, as the previous chocobo policy already did.

On-foot approach and a captured parked Invincible already worked. A current
Nautilus discarded that parked craft in selected routes and list coverage.
world_vehicle_journey.inl adds a FFIII-only bounded graph over at most32 copied
parked models, reusing the existing single-craft leg builder. One physical craft
node, deterministic family identities, no cycles, cancellation on each search/
candidate and real standable boarding contacts preserve manual deck/landing/
mountain actions. No native hook/read, input, extra game pointer or persistent
cache is added. Other games retain their previous independent-craft behavior.

The same native wind actors896/898/899 at97,212 retain transportation selectors
1/10/6. A first incomplete test omitted these actors and incorrectly allowed
Invincible flight through the winds. No production terrain or gate was changed
to accommodate that incomplete fixture. With the real captured selectors,
Invincible-to-Nautilus and foot-to-Invincible-to-Nautilus reach Doga's Manor.
Missing Nautilus, repeated directions and moved parked positions are checked.

Evidence: build_check/navigation_journeys_20261030/ contains Maze before/after,
vehicle_chain_before/after, wind_native/after and all-six route-worker results.
The qualified selected-plus-list wind episode took377.043ms on the owned worker;
native-owner calls0. This is compute evidence, not game-frame or audible latency.
All-six route-worker checks pass. Targeted development builds preserve canonical
CMake inputs, one compiler and RAM scheduling; final build/smoke/scan/deployment
remain required.

## Surface and underwater connection

Finite scope: FFIII native world3/4 plus qualified continent travel; fourth
World map filter, limited to objects physically on connected world bakes.
Preserve Current map's literal inventory and actual native entrances. Estimate
90–150minutes for source/production-consumer integration and affected controls,
then final combined delivery. Report overruns and concrete remaining work.

Native dive/surface evidence remains in the externally archived sources indexed
by [Nautilus access](NAUTILUS_ACCESS_20261028.md) and
[Doga's Village surfacing](DOGA_SURFACE_MARKER_20261029.md). Nautilus9 changes to
submarine14 on world3; surfacing returns to9 on world4 at the same position.
Surface terrain16 and the submarine's real surfaceable landing terrain govern
the paired action. Village150,203 requires the real underwater tunnel and basin.
Source-bearing scripts/disassembly remain outside every Git tree.

world_map_journey.inl is an owned worker graph. It first proves access to an
actually captured upgraded Nautilus through the same-world journey service.
It then uses the production router's exact successor guards, separate native
mode requests and copied map-state projection. It returns only the first map
leg and explicit manual dive/surface action; the existing pending destination
recaptures on native entry. It never sends a submerged craft through a walking
door or uses an off-map Unity transform as a saved parked position.

The fixed graph allows at most two map changes across three65536-cell stages.
Preference is fewer underwater tiles, then fewer manual changes and physical
steps, not an invented estimate of animation duration. Ordinary same-underwater
selection retains the existing valid slower route. Native owner does no search
or wait. Cancellation, queue/work/memory bounds, selected/list/browse/native
continuation, unavailable upgrades and exact final manual endpoints must pass
before this checkpoint is marked complete.

First production-consumer run had four failing expectations, retained as
world_before.txt. Two exact integration corrections resolve them: the selected
script entrance retains its terminal/trap-goal permission; reverse walking
seeds clear forward goals, matching the existing boarding planner. Village
surfacing and the actual subsequent landing/walk now pass, including a surface
origin requiring the underwater tunnel. Saronia Catacombs dive95,78 leaves one
underwater step after104 flying steps. Original Nautilus8 without an upgrade,
cancellation and native-owner0 controls pass. First owned scenario set796.481ms;
no game-frame/audio claim. World inventory/scanner targeted checks pass.

## Continent connections and off-map positions

world_air_journey.inl reuses the seven reviewed native air-event families.
Actual continent arrivals are SetVehicle cells32,216 on surface/old flooded
world and2,97 on the Floating Continent, rather than fallback PointIn locations.
Carried models retain the script's native conversions:10/11 Invincible,6/15
Enterprise, and Nautilus8/9 selected by copied ScenarioFlag78 when leaving the
continent. Unknown flags do not grant a mode. Existing ScenarioFlag66 selects
the actual restored/flooded world. Fixed arrival/native position qualification:
External.Vehicle.SetVehicle948d90; FieldController.TransportationEnabled294390;
TransportationController.SetData468990 invokes IMapAccessor slot3 conversion;
FieldController.ConvertCellPositionToWorldPosition27c650 and278b30 use the
matching16-unit centered-cell arithmetic. Sources/provenance remain external
at ../tools/world_vehicle_20261030/ff3/, with original extractor in tools/re/.

Future-world scopes contain copied values only, project native gates/selectors
from the existing resolver and discard later-floor NPC occupancy as first-leg
proof only. The loaded map always recaptures real actors. Each request returns
only its current leg, keeps the exact destination and recaptures on native map
entry. Depth is at most3; no world detour back to its caller is introduced.
An integration test caught a lost terminal-goal bit in boarding_route's aerial
leg: the copied riding request now retains that exact typed endpoint. Powered
native border actors plus surface/continent/underwater return/continuation cases
pass0;1.404s total owned compute for the expanded scenario set.

The fourth World map mode retains a literal active inventory on connected world
bakes, including entrances into interiors but never their contents. Existing
Current map remains literal. The old0/1/2 preference numbers/default2 remain;
World map is3. The existing nearest air-marker provider retains ownership of
continent-border copies. Projection bounds32 world bakes; observed FFIII
surface projection55 remote rows/4 underwater entrances. All text is in12
languages. Scanner projection cache records the filter mode as well as map.
Literal inventories now defer unused unique-exit distance queries in both
Current map and World map; repeated exit/event groups retain route evidence.
The human-scoped [Floating Continent lag fix](FLOATING_NAVIGATION_LAG_20261030.md)
also preserves fresh browse/directions while removing redundant stationary
refresh work. Its native frame-local pointer rules are separate from saved
off-map value capture.

Remaining off-map vehicle gap is addressed through the native departure save,
not unloaded Unity transforms. TransportationController.OverrideData467f00
copies each real model's StayMapId and SaveMapManager.ConvertEntityPosition
into OwnedTransportationData. saved_vehicles.cpp reads the ready user's bounded
owned list and saved map/cell; get_Enable66bbf0 reads native release flags.
Both UserDataManager and DataStorage static instances must already exist;
neither singleton getter is called to create state. No collection clone, scene
scan, serialization, native write, new hook or retained game pointer. The helper
abstains outside FFIII/off the owner/unready, validates names/shape/finite cells,
and rejects collections over32. Current-map craft continue to use live positions.
Future scopes exclude the carried physical craft and use enabled saved positions
for other craft on that map, enabling a later switch to Nautilus after Invincible
continent travel. Native reader and actual planner integration pass targeted
checks, including disabled/missing craft, updated positions, native-owner-only
access and continent Invincible -> surface saved Nautilus -> underwater/wind
destination. One fixture pointer-conversion compile error is retained with its
explicit fix. Capture is requested only for World map mode or a selected
different-world destination, not for every ordinary current-map action.
Internal Symbol/Content/player records remain excluded consistently with the
existing live-vehicle enumerator; enabled metadata cannot invent a parked craft.

## Reported list and label consistency

Group exits now keys the actual destination together with its source floor.
Maze1F and5F each retain their real World Map door; duplicate exits on either
individual floor still collapse using existing usable-nearest/selection/pending
priority. Actual Maze bake/provider/category checks cover both floors and a
same-floor duplicate. An initial provider fixture omitted prepared state and
was corrected explicitly; production filtering was not relaxed for that error.

The own-log Invincible197,42 -> parked Nautilus46,57 reproduces browse's false
Not directly reachable, although the actual selected route succeeds. Browse
previously only explained boarding actions before a phase and ignored a first
manual mountain action after flight. It now describes that first boundary:
Fly62.0 steps, then fly over the mountains West. Genuine missing/locked Nautilus
still fails. This is the same planned route, not a second reachability shortcut.

NPC labels strip developer ordinals from inside a final descriptive parenthesis
before applying the actual duplicate suffix. Magic Shop (Black Mage1)2 becomes
Magic Shop (Black Mage)2. Descriptive job text, explicit outer ordinal, fullwidth
parentheses/digits and nonterminal digits such as Guard (2nd shift) are retained.
No translation strings or vehicle capabilities are guessed from names.

Final combined verification/all-six delivery is complete at
build_check/navigation_delivery_20261030/. Constructor-policy/Village-list/
completed-transition controls are in the normal canonical runner alongside all
five new journey/reader/capture/performance checks. Earlier development receipts
and the explicitly interrupted first build are retained separately.
Actual game-frame lag, audible response and natural gameplay remain unmeasured.

Final normal canonical3m23s and154 fresh reports pass. The initial FFV parked-ship
parity failure was corrected by limiting the new ExitSnapshot vehicle policy to
FFIII. The subsequent build-script fallthrough was corrected with an explicit
normal successful return; C++/data stayed unchanged. Earlier compile/test/exit
receipts are retained.67 exact embedded resources/12 languages/683 inputs,
47-export smoke,22-file Defender stable scan and132 installed hashes pass;
144 protected files unchanged. No further gameplay or latency claim is made.
