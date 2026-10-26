# Chocobo approach and Tower of Owen interaction geometry

Status: complete, offline verified and installed/hash-verified in all six games.
Gameplay/listening confirmation remains pending. Full canonical checks, smoke,
resources and exact final payload scan pass; final delivery receipts are below.
No game launch/control/probe or delegation. Existing work and prior delivery remain.

## Reports and exact evidence

The own FFIII session rides Chocobo12/type9 from(32,40) toward Owen(64,31).
It repeatedly says land at(49/50,31), then14/15 East. Native control continues
along that strip on the chocobo. After dismounting at(63,31), the same entrance
becomes Story gated and disappears from the filtered list despite native entry
working. User later clarified that the inner hole must be examinable before
Toad, and should not keep a fictional gate label after Toad. The world exit from
the tower and its deeper-floor entrance are distinct destinations.

The outside bake contains a foot-only CollisionEntity HALF_TOP2 at(64,31),
matching the native source. Live/remote blockers previously converted every
non-through collider into a full-cell wall. The native CollisionFilter retains
three sides for HALF_TOP; west/east/south entry remains legal. This explains why
the entrance appeared mounted but failed on foot. There is no missing town asset.

Map174/Map_30071_1 has both native GotoMap3 and examined entity10/script62 at
(19,19), and a derived examined Warp to175/PointIn1. The native sc_e_0040 script
calls the Frog condition check, gives ordinary-form dialogue or performs travel
when it passes. Current.CheckFrog RVA91B570 forwards condition402 to the native
party condition check. The bake correctly preserves both routes; coordinate-only
goal inference prioritized the overlapping walk-on exit instead of the examined
object, then called its permanent solid body a story gate. Party form does not
need to erase that interactible object or change its authored collision flag.

## Native riding/dismount rules

Read-only disassembly/metadata facts are outside Git at
../tools/chocobo_owen_20261025/ffN/{native.txt,provenance.json,facts.json}.
Original extractors: tools/re/map_location_native.py --chocobo-rules and
tools/re/transport_rules_facts.py. Metadata fields are decoded from matching
dump annotations and script.json FieldInfo identities, never guessed from HP,
sprite animation or a runtime probe. Exact installed DLL/dump/metadata hashes
remain with the evidence. Proprietary source/bytes do not enter this repository.

Native CanLandToWorldMap checks terrain, duplicate transports and applicable
collision; yellow chocobo dismounting is not unconditional. FFIII Chocobo12 uses
transport group2/landing group2. Black chocobos retain their separate landing
rules. Native trigger tables enable GotoMap for foot/Content and the existing
Boko/Magitek types where supported, while yellow/black chocobos use the default
table without GotoMap. Thus a yellow chocobo needs a final dismount for map entry,
but there is no reason to stop riding15 cells earlier on the same legal strip.
Boko/Magitek prohibit generic voluntary dismounts and retain mounted map entry.
Exact metadata confirms explicit entry tables1/5 in I/II/IV and1/5/11/12 in
III/V/VI; only V/VI have native Boko/Magitek transport rows. The first metadata
reader assumed the same table count/array width/instruction form everywhere;
it now reads the actual22/29/31-byte tables and both constant-key forms before
validating applicable rows. No native-rule inference survived a failed assertion.

## Implementation and ownership

- baked_router Request::Directional adds an optional native cardinal mask while
  retaining legacy axis fixtures. Shared add_collision handles None0, NoEntry1,
  partial2..5 and through6..7. Masks constrain entry and exit faces; unknown and
  triangle IDs retain conservative prior handling. Native stair connector geometry
  remains the baked parser's responsibility. Live and remote obstacles use the
  same helper, so browse, full routes, filtering and retained plans agree.
- The explicit native_ground_entry type helper preserves supported mounted
  map entry and the terminal-transition rule for Boko/Magitek. Full route goals
  may enter their selected doorway without inventing a generic landing. Other
  intermediate doorways remain terminal. Ordinary flight/ship rules remain.
  This exception is limited to map entry: existing on-foot requirements for
  examined world objects/NPCs remain unchanged. Native landing-guide speech and
  loading-time suppression in fieldnav/vehicle_landing.cpp are also unchanged.
- Chocobo landing selection prefers less walking only when total route cost ties,
  retaining legal terrain and shortest total travel. Riding continues to the
  nearest equally short usable destination approach. The localized instruction
  is dismount; the existing native confirm action is unchanged.
- Goal inference recognizes an examined Warp sharing a GotoMap's exact cell,
  destination and PointIn. Both object and map-exit selection use its legal
  adjacent interaction/facing approach. The game-owned script still owns Toad
  checks, hint dialogue and actual map travel; the mod never changes party form,
  flags, native collision or guarantees that an unmet script condition succeeds.

All substantial work stays on existing Data/Navigation owners. Partial masks add
constant work per copied collider and indexed direction checks, not native scans.
Snapshots retain their existing collection bounds, cancellation, stale-result
rejection and ownership. No new hooks/polling/waits/native pointer transfer or
speech deduplication. Existing cohesive planner/router placement is retained;
no unrelated file split or architecture change. One new mod-owned instruction is
translated in all12 supported locales; language detection/overrides are unchanged.

## Verification and player checks

tools/chocobo_owen_check.cpp uses real bakes, native transport masters and the
production owned planner. It covers exact Owen riding/foot/filter origins,
examined Hole/object and exit approaches, all-six partial collider faces,
unknown/full/None handling and real native mounted-entry cases. Full retained
route, trap, NPC, floor, event, vehicle and worker-affinity checks remain required.
Offline computation is distinct from native frame/audio latency and gameplay.

Focused production replay passes: logged Chocobo route47 steps now dismounts
at(63,31) with only one step left; the same foot origin routes/browses/filters
the entrance at distance1 with the real HALF_TOP collider active. Hole object
and exit both reach an adjacent faceable approach; facing is spoken on actual
arrival, preserving the existing phase contract. The first test incorrectly
expected facing in the distant route description and was corrected to inspect
structured arrival direction plus actual zero-step arrival speech. The exact-
exit audit now distinguishes a proved collocated examined Warp from ordinary
walk-on exits; sibling/footprint pinpoint assertions remain for all ordinary exits.

## Final validation and delivery

Final DLL10948608 bytes, SHA256
807984be33160338bef7f87e4a5fdf5fe2d818dc5a121f250208157b5fc733ad.
Canonical build passes all checks:7994 native names/0 missing,137 used/344 defined/
0 undefined wiring names. Smoke resolves and forwards all47 HID exports and exits
normally. All49 embedded data and18 sound resources match current source. The
new riding/collider/Owen/mounted-entry tests, all-six exact exits (including178
South Figaro exits), actual graph/trap/repeat/worker-affinity tests and2430 native
event routes pass. Maintained dismount translation values match all12 locales.

The retained worker fixture observes maximum route/filter compute56.357/29.297ms
over98 boarded world/transport cases and582 world exit candidates; maximum six-
category preparation3.942ms. Event-route paired compute maximum5.555ms. These
offline observations are not a comparable baseline or native callback, end-to-end
audible, load-time or gameplay certification. Existing bounded capture, generation
cancellation and current-context publication remain unchanged.

Enabled Defender reports no threats on the exact22-file final payload, zero
matching detections and stable staged hashes. Signing capability remains0
certificates; exclusion enumeration remains unverified. No security setting or
upload changed. All132 installed hashes match the scanned bytes, and all144
protected files retain bytes/timestamps. Settings, Prism/OpenAL, saves and
waypoints were preserved. Deployment used the normal process guard; all games
were closed when their copies were replaced. No release or dependency upgrade.

Evidence/recovery:build_check/chocobo_owen_20261025/. Final verification.json,
native_facts_index.json, localization_check.json, security_final_predeploy2/,
deploy_finalbefore.json, deploy_after.json and installed_payloads_verified.json
identify the delivered payload. The earlier a11c54b8 candidate was also checked
and installed, then refreshed after narrowing the doorway-only exception; its
candidate_verification.json/candidate_deploy_after.json and first scan remain
historical evidence. Final source hashes match the verified build. No game
control/runtime probe/delegation occurred.

After delivery, check these during normal play: riding toward Owen should keep
you mounted to the entrance approach, then say dismount. The adjacent entrance
must remain listed and reachable on foot. Selecting Hole should stop beside it
and face it; interacting normally should give the game's hint, while the required
Toad form should let its native script perform travel. Do not request an early
gameplay replay or remove a genuine progression condition to satisfy this check.
