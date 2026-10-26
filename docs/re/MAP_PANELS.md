# Basic maps and location-map menus

October24 user request: inspect every game's basic and location-map screens,
read Shift+I controls, speak highlighted location details only where rendered,
and provide a basic-map marker buffer with approximate player-relative distance
and bearing. The user's FFII screenshots show a basic dungeon map without totals,
then a location-menu map with Tropical Island Chests5/20 and Altair Chests1/1
plus service icons. Current Location/Ship selections have empty detail panels.

## Status

| Feature | Implementation | Offline verification | Gameplay |
|---|---|---|---|
| Highlighted name, services and displayed counters | Implemented | Passed | Pending |
| Screen-owned Shift+I controls | Implemented | Passed | Pending |
| Basic-map visible marker review | Implemented | Passed | Pending |
| All-six variants, cancellation, localization and bounds | Implemented | All-six native/behavior checks passed | Pending |

Raw native evidence stays outside Git in ../tools/map_panels_20261024/.
Map reading is not route planning and must never imply traversability. Basic
maps must not read location-menu counters, save flags, undisplayed treasure
contents or inactive marker data. Location selection replaces prior details,
including when the new panel is blank. Existing native selection, panning,
zooming, map-switch and cancel controls retain their ownership.

## Controls and data flow

Menus.MapPanels registration125 is in menus/map_panels.cpp. Basic Show forwards
the original first. GPS Show establishes its owner before forwarding because
initial selection can update synchronously inside Show. Renderer captures start
from Show or GpsMenuInfoController.UpdateView; Basic Hide, GPS Close, SceneLoaded
and BattleStart retire that context. Desktop KeyInput variants cover all six;
Touch classes were inspected as a separate interface, not installed as PC hooks.

The basic map provides a nearest-first Information buffer of rendered markers.
Arrow Down/Up browses; Left/Right goes first/last. Native WASD panning, zoom,
switch-map and cancel remain native. Approximate distance/bearing is straight
line, not a route. Positive native Y is north,16 units per tile; world maps wrap.
MinimapIconType0/5 player points are excluded. No chest total is read here.

On the Tab location menu, highlight speaks the rendered name. Autodetail adds
visible services/counters; with it Off, I reads them manually. Multiple detail
lines use the arrow buffer; a lone count does not. Blank Current Location/Ship
panels replace prior details. Native GPS hides contentParent but retains old
text objects: visibility, not cached text presence, determines disclosure.
Shift+I uses the owner's keyHelpController. F9 can restore native arrow use;
see [input interception](INPUT_INTERCEPTION.md).

| Owner | Captured fields |
|---|---|
| ExpandMinimapController | mapController, keyHelpController |
| Common.ExpandMinimapMapController | playerPos, minimapIconPropertyList, pointItemList |
| PropertyEntity / PropertyMinimapIcon | Pos, IconType; only when the point image renders |
| GpsMenuInfoController / selected content | view, content.view.placeText |
| GpsMenuInfoView | contentParent; chest/hidden-item label/current/maximum text; shopIconList |
| GpsIconContentView | rendered image and gpsIconType |

GpsIconType1..6 is shared; VI additionally has7..10. All-six PE/dump provenance
and23 bounded functions/game are outside Git in the investigation above. FFII
locators: GPS Info.UpdateView77C1E0, Common.SetupPointList5372D0, SetupPoint5375B0,
UpdatePointItem537A40, ExpandMinimapUIManager.SetMapResource4CD140. Those producers
establish position units and rendered point correspondence, without UI pixels.

## Ownership, bounds and extension

Only native-owner callbacks touch Unity/IL2CPP objects. Captures copy strings,
enums and coordinates. Basic parallel lists require matching counts, at most8192;
GPS services at most64. CaptureBudget checks0.5ms between operations, with512
basic/64 service defensive iteration ceilings. An accessor can overrun one slice.
These are work/corruption guards, not pathfinding distance limits.

Owned Snapshot goes through worker_job on the compute lane. map_panels_data.cpp
localizes, wraps, computes bearings, sorts and assembles Reading. Native pointers
stay in native-side callbacks; generation and visible owner gate publication.
New show/selection/scroll/scale cancels old work. Pending I survives capture for
that selection. UpdateScroll/Scaling refresh without repeated automatic speech.
ReviewBuffer owns long details, arrows and Autodetail ordering. Queue pressure
retries/cancels without inline compute or a native-thread wait. Logs distinguish
slice/native aggregate, capture age and request-to-submit; audible and unmodded
frame timing remain unmeasured.

map_panels_check runs actual detours/capture and formatting on a separate fake
worker for all six adapters: visibility, blank panels, Autodetail/I, pending I,
rapid selection, close, bearings and wrapping. Thread instrumentation checks
native view/list ownership. key_help_check tests owned controls;
oct24_navigation_check verifies required native members and12-locale placeholders.
Extend only through verified producers/rendered fields. Never reconstruct totals
from flags, expose hidden text caches or substitute treasure contents.


## October24 delivery and limits

Canonical build and every required offline check pass; isolated smoke resolves
and forwards all47 exports. Native-name audit:7,373 checks, zero missing; action
wiring:131 used/339 defined, zero undefined. All49 embedded data and18 sound
resources match current sources. DLL10793984 bytes, SHA256
 3c8562206a2fe22ca9540a4349cff82ec04c68ee645e34db8f7e5f8df73b5b82.
Installed and hash-verified all six after the user closed FFII. All144 protected
files match the fresh applicable deployment baseline. Recovery, reports,
source/resource hashes and receipts: ignored build_check/mysidia_objects_20261024/.
No game launch/control/probe, dependency install, delegation or release/push.
Gameplay/listening confirmation remains separate and pending; see TESTING.


The actual map capture/worker/review pipeline reaches a usable real buffer in
0.729ms for32 markers and148.159ms for8192 markers in the final host fixture.
These include synthetic frame scheduling/thread creation, fake native views and
fake speech submission; they exclude physical game-frame cadence and audible
screen-reader delivery. The test runs the real ReviewBuffer worker rather than
assuming its first callback is readiness. Busy/normal rows are retained in full.
