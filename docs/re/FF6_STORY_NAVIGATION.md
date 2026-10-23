# FF6 story sequences and baked navigation

October 23, 2026. Authoritative design, feature register and evidence for the
moogle-defense progression block and related story audit. Implementation,
offline evidence and gameplay confirmation are separate. No game was launched,
controlled or probed.

The later [October23 follow-ups](NAVIGATION_FOLLOWUPS_2026_10_23.md) supersede
the initial verbose speech policy: Shift+G now gives brief directions/actions,
and native party changes announce only the number. Defense prioritizes a live
three-tile threat, otherwise its completion commander, with one-shot Terra/Banon
danger alerts. The receipt at the end of this document is the previous delivery;
the follow-up register contains the current all-six installed artifact.

## Feature register and controls

| Feature | Implementation | Offline evidence | Gameplay |
|---|---|---|---|
| Terra defense, Narshe mines | complete | Actual contact scanner; seven encounter routes in sequence, moving/defeated actors and both native exit barriers | Original tester block; correction unconfirmed in game |
| Phoenix Cave | complete | Two-party rules, seven area switches, water/lava, held twin switches, bridge/rock states and 18 stone crossings | Unconfirmed |
| Kefka's Tower | complete | Three-party rules, outer switches/release, weights, fences/hidden doors, bosses, bridge/final switches and directional ladder/belt travel | Unconfirmed |
| Opera House | complete | Preparation/retry/letter/far-right lever; actual Upper-layer rafters, five rats/Ultros; exact prompt producer and legacy result feedback | Unconfirmed |
| Later Narshe defense, burning house, Phantom Train engine | complete | Native encounter/state objectives; train left/right on, middle off | Unconfirmed |
| Other FF6 story-map audit | complete as an inventory/shared-defect audit | All 638 submaps, 2,841 scripts and 2,297 scripted routing objects inventoried; 59 contact NPCs and 69 puzzle controls indexed | Not an exhaustive playthrough or a new solution for every puzzle |

Use existing Shift+G for guidance and Ctrl+G to stop. In controller field mod
mode, right shoulder requests guidance and left shoulder stops it. Bindings
remain independently configurable. Native controls perform movement,
interaction, party changes and combat. No timer freeze, flag write, automatic
puzzle solution or new setting/default is introduced. All 36 required messages (35 new keys) have
all 12 locales and use existing persisted Auto/manual language selection.

Multi-party guidance announces a changed active party without a location summary.
A party holding a required plate is told to hold and switch parties. Once the tower's outer door
opens, the party below is told to step off before another party drops the weight.
Old-party routes, autowalk, selections and field audio are invalidated. Each party keeps its own pending objective. For a temporary Phoenix switch, the guide first plans against geometry after release: it permits a safe departure or tells that party to stay. The still-pending objective survives the release refresh, preventing a return-to-the-same-switch loop.

## Root causes and corrections

1. Native FieldNonPlayer.SetProperty excludes contact action9 from CanAction.
   Scanner inclusion and Entity.is_alive wrongly treated that as an absent NPC.
   Both now accept action9 with a real script, preserving other disabled-talk
   guards. FF6 has59 contact NPCs, FF5 has38; I-IV have no extracted NPC occurrence.
   FF3's action definition also knows9, so applicability follows object data.
   Legacy FF6 EntityFactory did not discard these actors through CanAction.
2. Moving NPC goals used baked home coordinates. Goals now retain native ID/raw
   identity and rebind to the captured live cell. A moving actor cannot inherit
   an unrelated home counter. A missing current-floor actor cannot revive from a
   bake. Other NPCs still block routes. Paths are snapshots: repeat guidance when
   an enemy moves before acting; ordinary live selection/beacons remain available.
3. Native SetEventEntityGroup enables/adds groups. Projection kept only the last
   group. The full enabled set is now retained, verified against all six native
   implementations. Observed groups reset on map reload, including the same ID,
   and Title. Independent puzzle groups no longer replace each other.
4. Projection lacked area/map locals and moved/hidden actors. Copy8+8 locals with
   owning area/map IDs. A remote floor may use only matching area locals; foreign
   map locals stay unknown. FF6 initializer compilation records bounded,
   synchronous final Hide/Show/absolute SetPos facts. Physical actors and derived
   travel links receive those facts; PointIn/metadata ID domains do not. Live
   single-tile walls/rocks use their current cell. Entity ID0 is valid and matched
   with raw identity/position rather than discarded.
5. Scripted same-map travel was absent. There are106 reviewed links: Phoenix
   stones, Zozo jumps, cave bridge jumps and tower ladders/belts. Each has a source
   hash and exact final cell/layer; direction branches have approach masks.
   There are87 manual interactions, including69 masked approaches, and19 step-on
   links. Solid travel objects start from legal interaction sides, never their
   blocked actor tiles. Opposite branches cannot borrow another landing. Lateral
   ladder approaches closed by the actual grid are excluded. Existing trap and
   cutscene semantics remain intact.

## Native scenario contracts

Scenario indices are flat: bank1 starts0, bank2 starts256, bank3 starts512.
Area locals belong to the cave's native area; map locals belong to one floor.

| Sequence/maps | Progression and guidance |
|---|---|
| Terra318 / Map_30011_2 | Scenario13 set/14 clear adds ev_e_0018 beside the ordinary exit group. Moving NPCs18-23 use sc_e_1081-1086/action9; Guard Leader9 uses sc_e_0018/boss5. Native RTS selection precedes contact battles. Victory hides the actor; loss restores/ends the native defense. Leader victory sc_e_0018_2 restores the party and proceeds through322. Guidance prioritizes a live enemy within three tiles of Terra, otherwise the commander; simultaneous reachability of every enemy is not required. Both ID0 exit barriers remain at baked19,35 and13,7. Native SetPos14,15 becomes baked13,15, without tile-object Y subtraction. |
| Phoenix497-499 | Map498 area[1..7] controls held wall/spike/rock switches. Their explicit leave scripts sc_e_0794_1 through sc_e_0801_1 reset the corresponding local to0; these are temporary openings. Drain lever56/sc_e_0799 sets399 and reloads499. Plates41/40 hold400/401 and reset on release; sc_e_0806 latches402 when both are held. Bridge98 sets403. Floor499 switch66 sets405;58/55 select mutually exclusive406/407 rock states, not two completion tasks. The rock controls retain their selected position after leaving; the guide offers the alternate control when needed. Event50/sc_e_0828 reunites the parties toward408. Red Dragon remains in normal navigation. |
| Tower542-571,590,595,596 | On549 plates37/38 hold478/482, jointly latching483. Weights39/45 set486/487 only with the party below clear. Events565:25/26 set484/485;549:61 sets493. On565 plates40/41/44 hold488/489/490, jointly latching491. Required boss gates use474/492/494/495/496. On570 plates15/16/14 hold498/499/500; all three latch501 and start the final sequence. Native barriers and individual party access remain authoritative. |
| Opera112-118,576 | Preparation/retry actors have distinct group identities. Dressing-room Celes, stage entry and letter precede sabotage.167 starts the timed sequence; far-right lever6/sc_e_0332 sets168. Other levers are traps and are excluded from guidance. Rafters117 is Upper layer; five moving contact rats complete169-173 and Ultros5/ev_e_0344 stops the timer and proceeds to118.177 ends guidance. Displayed choices/native performance readers remain active. |
| Later Narshe19 | Scenario137 set/138 clear selects ev_e_0257. Twelve contact enemies24,38-48 and Kefka25/sc_e_0259. Live presence determines availability; native battle/result/party restoration still runs. |
| Burning house412-420 | Prefer rescue/boss Event420:1/sc_e_0539 when reachable; otherwise offer a reachable native flame encounter. Door-approach Event414:5 can create more flames and remains actionable. Strago/fire/door choreography and rescue continuation are preserved. |
| Phantom Train355-372,624 | Engine-room371 levers14/10/6 toggle120/121/122. Correct values1/0/1 precede engine356:5/sc_e_0214 and its boss. Retry guidance tests actual values rather than operating each once. Cyan's departure actor on372 is separate. |

The seven guides contain77 state rules and107 physical target references,
including held-switch identities. Other puzzle controls, including Dreamscape
and Sealed Gate, retain normal navigation and the shared corrections. This audit
does not claim new authored solutions for all69 controls or every possible
story state. The complete source-free coverage inventory is
[ff6_story_audit.json](ff6_story_audit.json).

## Opera timing and legacy parity

BlindGuyNW/ZKline FFVI_Mod/Patches/OperaPatches.cs observes rising Success, Failed
and Out of time flags. Native opera.cpp already ports their actual writers
without an Update poll. Legacy live EntityFactory/navigation exposed actors;
that parity alone did not provide the new state-based maze/lever guide.

Native PlayOperaEvent.MoveNext activates interactiveIconCanvasObject at an
input window, and tests that same active canvas before accepting confirm through
InputListener.GetKeyDown/OnClickInteractiveIcon. The detour now observes each
inactive-to-active actionable occurrence and says Opera action: confirm now.
Repeated coroutine frames do not repeat that occurrence; a new identical prompt
speaks again. StartOperaInteractiveTimer.MoveNext is a delayed failure/timeout
path, not the start of a valid window. No cue comes from that timer; no automatic
input or timer alteration is added.

## Source and native evidence

- games/ff6/story_guidance.cpp owns registration FF6.StoryGuide/order515, ready
  native observation, party hook, publication, cancellation/reset and speech.
- games/ff6/story_model.{h,cpp} owns immutable predicates, exact targets, central
  snapshot-safe names, numbering and localized instructions.
- field/puzzle_guide.cpp delegates supported FF6 maps and retires an older static
  guide; other per-game static guides continue unchanged.
- field/{entity_scanner,navigable_entity,entity_nav,entity_translator}.cpp owns
  contact inclusion, stable actor identity and worker-safe central naming.
- field/{map_state,map_state_program,baked_nav,baked_router}.cpp owns local
  lifetimes, groups/actor facts, bounded candidate routes and crossing approaches.
- tools/{author_ff6_story_guides,audit_ff6_story,bake_map_state,bake_maps}.py
  reproduces original factual recipes, coverage and compiled navigation data.

External inputs: ../tools/mapdump/ff6_objects.jsonl, ff6_scripts.jsonl and
ff6_master/{map,script}.txt; ../ff6/dump.cs and installed GameAssembly.dll.
Original extractor tools/re/ff6_story_native.py writes disassembly/provenance
only to ../tools/ff6_story_20261023 after checking it is outside every Git tree.
Arguments1-5 export bounded shared-rule evidence. Inventory data contains
counts/identities/hashes, never game implementation bodies or script streams.

| FF6 native locator | Contract |
|---|---|
| EventActionDefine..cctor2F0A20; FieldNonPlayer.SetPropertyFF1DD0; get_CanActionFF2620 | Talk availability excludes contact action9; scripts still own encounters |
| FieldController.SetEventEntityGroup3150D0 | Enable/add group. I-V matching RVAs289B10,289B20,2901B0,28BCE0,28A570 |
| Puppet.SetPos2ECBA0; Lerp2E7220 | MapRange conversion before position conversion; all-six Move/MoveTo/layer facts are in TRAPS.md |
| Current.SwitchCorps2C34B0; UserDataManager.set_CorpsSlotIndex581950 | Switch performs selection, setter, parked-party map shift and global[4] update. Unique setter ABI: instance,int32,MethodInfo*. Slots0-2; global[5] count |
| SaveMapManager.AddOtherPartyDataB345A0; SaveMapData.AddOtherPartyDataB34150 | Array indexed by corps slot; at most3 records. Defense observes mapId plus PlayerEntity.position, with bounded owned playerEntity JSON fallback; never save files |
| Opera PlayOperaEvent.MoveNextD1EF20; interactive timer MoveNextD23FC0 | Actual canvas/key window versus delayed failure timer; existing result flags preserved |

## State, threads, bounds and cancellation

Native pointers stay on their owner. Party-setter prefix marks epochs and fires
PartyChanging; ready continuations clean up and observe. Map/battle entry
interrupts worker owners. Script completion invalidates state and schedules one
coalesced ready refresh. Title clears guide/party/readiness. Native callbacks
never wait for mod work. Workers contain owned state/text and immutable bakes.
Defense party poses are copied on the native owner; bounded JSON fallback is
parsed only on the worker. Missing poses are unknown, never remembered guesses.

Bounds:16 guides,128 maps/rules each,32 predicates per rule,32 candidate goals.
Projection allows8,192 instructions/64 branches,16 call frames,32 puppet slots
and4,096 actor facts; unresolved/asynchronous final state stays conservative.
Native copies include1,024 scenario/treasure bits,16 globals and8+8 locals.
Existing entity capture has0.5ms slice admission; an indivisible native call can
exceed this, so it is not a universal callback guarantee. The bounded Navigation
lane and epochs own overload/cancellation. New requests replace old work. A
guide owns a separate preview receipt from the manual destination. No native
pointer, callback, speech or native map-name resolver enters its worker. The optional anticipated-release pointer is copied synchronously before capture; only owned flag values cross into work. Released static gates replace current captured gate geometry, while live NPC coordinates remain intact. No native state is written.

## Verification, performance and extension limits

Canonical build.bat runs scanner, map-state/native boundary, actual route worker,
story model/runtime and Opera producer checks. Terra is tested as a reachable
encounter sequence with both barriers intact. Rafters uses its actual Upper
entry. Earlier tests wrongly required all enemies reachable at once or put the
rafters player on Ground; these fixtures were corrected without loosening native
barriers or geometry. Crossings test legal approaches and every reviewed landing.

Model checks verify107 physical references, unknown state, held/released plates,
correct lever eligibility and completion, seven exact pressure-release projections, and persistent rock controls without false hold instructions. Runtime checks hold old worker/route
completions across party changes, readiness, script changes and stop, with zero
native worker operations. Opera checks distinguish new prompt occurrences,
repeated frames, timeout and inactive playback. All-six routing, obstacles,
chests, vehicles, autowalk and existing subsystem checks remain mandatory.

The contact fixture previously discarded the six mobile defense actors and now
retains all six under the same native-property conditions. Route diagnostics
report aggregate native capture, capture wall time, queue, all candidate
attempts, compute and total usable-plan delay. Guide speech carries its original
ready timestamp through both worker stages into existing Directions speech
latency diagnostics. Offline timing is fixture evidence, not measured game-thread
or audible latency. Native callback tails, capture age under game load, moving
actor response and speech-device queues remain unmeasured. No universal target
or claim of responsiveness from worker placement alone is made.

Extend only after tracing prerequisites, action, release/reset and continuation.
Use native completion evidence, not adjacency. A travel link needs exact source,
reviewed hash, final cell/layer and approach eligibility. Reject unresolved
control flow rather than guess. Update fixtures, all locales, this register,
coverage inventory and feature index. Existing source/history publication
restrictions still apply.

Luna performed the bounded catalog task and two-message follow-up. Sol retained
research/design/implementation/integration and reviewed coverage/placeholders,
corrected one Japanese entry and restored unrelated formatting while preserving
existing values. No Astra delegation occurred. Final build/smoke/deploy evidence
and prioritized gameplay checks are linked from TESTING.md and CODEX_MEMORY.md.


## Previous delivery receipt, before the brief-guidance follow-ups

Canonical build and every offline check pass after the final changes:7,132 native
name checks, zero missing;125 used/331 defined actions/queries, zero undefined.
The isolated smoke host loaded/forwarded all47 exports and exited normally.
An independent638-submap rebake is byte-identical. All four embedded resources
(maps, state, puzzles, shared catalog) match the final source data exactly.

Actual production route-worker checks pass for seven Terra encounters,87 solid
manual crossings/69 approach masks, seven Phoenix release predictions and six
Upper-layer rafters encounters. The25 model-state plans resolve107 physical
references; party/readiness/stale-completion/stop, per-party pending objectives,
release-refresh retention and Opera producer checks all pass. Event projection
checks2,454 master roots with zero failures. Model aggregate0.636ms and all-six
worst route28.567ms are this offline fixture run, not gameplay/audible measurements.

Final validation exposed a route-test stack overflow after duplicating the large
optional Flags object through many local snapshots. The released state now uses
owned immutable shared storage rather than large copies through every route
result. The canonical build gate also now rejects negative Windows crash codes;
its previous errorlevel>=1 test missed that failure. Explicit zero/positive/
negative status fixtures verify the correction. Earlier apparent success from
that intermediate run is not accepted as final evidence.

FF6 alone was deployed with deploy.bat ff6, without launching the game. Final
hid.dll SHA256:66b12fb37be13ba2ccc4b270683d1ae6fed25d357857947de020b02156c7eeb7.
The previous DLL,d1dc03fd921c5b3782a3df2dcdf509c5c6ef439c6c2ab48e2c0272398eecb9ed,
and SDL are retained under build_check/ff6_story_20261023/recovery. The delivery
receipt verifies23 protected Prism/OpenAL/settings/waypoint files unchanged in
bytes and timestamps, and installed DLL bytes equal the tested output. An initial
receipt-path error was corrected; the pre-deploy backups/manifest were retained
and verified. Receipts/reports:build_check/ff6_story_20261023/delivery.json and its
sibling reports. No commit, push or public release was made. Gameplay remains
unconfirmed; test Terra first, then the other scenes when progression permits.
