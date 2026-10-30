# Inter-map vehicle state follow-up

Human reopened the Temple/canoe state checkpoint with the Shrine/parrot report.
Source complete, Closed October28; combined FFIII deployment pending. Estimate45-75minutes,
Sol only. Boost/Draw and reciprocal-return
source checkpoints are Closed. Only current-state observation, map reconciliation
and boarding speech are in scope; route search and accepted vehicle mechanics
stay Closed. Finish this and specific FFIII event labels before FFIII deployment.

The earlier rule incorrectly used TransportationInfo.StayMapId as evidence that
the current FieldPlayer is riding a transport. It is the saved craft location,
not the current player's rendered asset. Automatic canoe can remain selected
across maps; native Content/Canoe and Submarine both use Walk0. Conversely, a
constructed walking player can coexist with remembered Enterprise state.

Exact native ITransportation.SetupCharaAsset (FFIII467A90) uses the selected
transport's FrontStructure when present, otherwise Structure, as the player's
AssetStructure. The same current map owner supplies these references; identity
comparison therefore distinguishes applied native transport from a remembered
craft without guessing from a world map ID. FieldController.ChangeTransportation
(27A090) applies this asset before GetOn (F17A00). Neither native movement nor
transportation will be modified. Proprietary native exports/hashes stay outside
Git in parent tools/vehicle_state_20261028/ff3/; original extractor uses
tools/re/map_location_native.py --vehicle-state.

Unei's Shrine is map142 Map_20291_1, not a water world. Its five scripts, including
the map entry and parrot, declare RidingVehicle0. That controls script puppet
presentation, and does not by itself prove the remembered current transport was
cleared. The applied native player asset is the stronger observation for V and
the map-ready cache; do not assume every script flag is a dismount.

Current own-log copies:build_check/vehicle_maps_20261028/user_ff3_recent.{1,2}.log.
Recent rotations no longer include the originally reported Shrine dialogue;
the human's exact report remains the reproduction constraint. No runtime probe
or game control was performed. Native/frame/audible measurements and final
Shrine listening remain separate from static/offline acceptance.

## Implemented state flow and bounds

navigation.cpp::player_uses_vehicle_asset is a read-only owner observation. It
checks the current native player/controller/info, resolves the reviewed fields
and returns an optional identity comparison. Missing owner/assets/metadata stay
unknown. No managed/Unity pointer escapes the call. FFIII/V/VI have the native
FrontStructure branch; I/II/IV have Structure only, explicitly game-masked.
tools/re/vehicle_asset_facts.py verifies those exact layouts and native setup
branches for all six; derived facts/hashes remain outside Git. The first audit
caught the absent front field on I/II/IV before build; the implementation now
models that difference rather than treating it as missing native readiness.

move_state::native_foot still requires actual Walk/Dash. An actually applied
vehicle asset proves riding regardless of StayMapId; a different initialized
asset proves foot even on the same map. Unknown assets preserve current transport
context and cannot invent a dismount. V and the existing single map-ready
deferral share this decision. Map/battle generations cancel obsolete work.
Walking into Temple and retaining automatic canoe across maps are both covered.
Ship/airship/nonfoot modes retain their native identity; Invincible receives
no canoe-specific exception or guessed progression rule.

The existing GetOn postfix also declines a remembered vehicle restoration when
the current Walk/Dash player demonstrably has a different initialized asset.
It corrects the cache to foot and announces only a real cache edge in that case.
An applied canoe keeps normal boarding speech. Native travel/GetOff, routes,
settings and existing12-language strings are unchanged.

No new hooks, timers, searches, worker queues or retained native state. Added
work is a fixed owner player/transport lookup and at most three asset field reads
and comparison, only on relevant Walk/Dash status/map/boarding events. No new
per-frame work. Real Unity/getter/frame and audible costs remain unmeasured;
these bounds do not certify cumulative25ms responsiveness.

Actual-source vehicle_state_check covers foreign-map applied canoe, same-map
remembered craft with a foot asset, unknown assets, foot/dash, ship/submarine,
map seeding and transition cancellation. vehicle_landing_check exercises actual
GetOn suppression and retained applied-canoe acknowledgement alongside all-six
landing-guide cases. Native sprite application/Shrine listening remain gameplay
confirmation limits.

Unrelated timing observation retained in the copied recent Duster log: a47.9ms
owner span has0ms reported owner CPU and a conflicting21.5ms frame interval;
the child lacks CPU attribution. This unresolved wall-time sample does not
establish47.9ms mod computation. Record for a scoped timing investigation;
do not refactor the Closed scanner within vehicle state. A proven cumulative
25ms mod-caused application stall remains an immediate blocker.

Final canonical build8321 native names/0 missing,142/349/0 wiring, both vehicle
fixtures0 failures and smoke47 pass. Candidate11090944bytes SHA256
68e2648ee8fad0629ceab0888579741d51473591a59aba3ba39e0a0d75cc8368.
Receipts:build_check/vehicle_maps_20261028/. No source overrun or new subsystem
was opened. Next source checkpoint is the requested FFIII event labels.
