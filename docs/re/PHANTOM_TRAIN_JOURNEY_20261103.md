# Ordinary Phantom Train entrance-to-engine journey

Source/focused Closed FFVI map/story audit slice. The accepted engine combination
remains left/right on and middle off. The actual production planner's preserved
engine-only baseline fails from platform355, first car357 and detachment car362,
while the same lever is reachable on371. Thus selecting remote engine levers
does not supply the earlier required native progression. This is a concrete
missing stage integration, not a reason to redesign the accepted engine or
shared router. Baseline and source metadata are in
build_check/phantom_train_20261103/.

Eleven native pre-engine rules are now appended using the existing story model,
ready continuation and Navigation worker. Full physical/stage/controller checks
pass; no new native DLL, installation or game/probe is claimed.
All eleven other guide records are preserved. No hook, native observation,
input injection or new timing mechanism is added.

## Native stages and user behavior

Use the existing Shift+G/controller guide command after each scene or operation.
Native events perform their own boarding, pursuit and jumps when the player
reaches the actual trigger. The mod gives the physical route and observes the
real native completion receipt. The ghost remains an actual conversation and
battle; the detachment lever is operated in each of its two native phases.

| Stage | Native receipt and next real objective |
| --- | --- |
| Platform355 |0160 sets107; then board through355 Event2/0161 |
| First car357 |0161_1 sets108;357 Event2/0163 starts the train and sets109 |
| Ghost blockade361 |1050 sets519 and enables0185 NPC6; native battle changes to624 Point11;0185_2 sets110 |
| Pursuit624 | Event1/0186 sets111, enabling the next actual roof event |
| Native roof jump624 | Event2/0187 sets112; its native choreography owns movement and landing |
| Plan detachment624 | Event3/0188 sets113 and returns to362 Point1 |
| First lever operation362 | Entity20/0189 changes to624 Point12;0189_1 scrolls the detached cars and sets114 |
| Return and second lever |624 Event22 returns to362; the same physical Entity20 now runs0190, setting115 and opening its actual barriers |
| Accepted engine371/356 | Only after115, retain the three120/121/122 objectives and boss event with the original1/0/1 solution |
| Final platform372 | After native123, talk to Cyan;0215 returns to world map1. That actual ready arrival owns guide completion |

Native point IDs are object IDs. In particular,624 uses Point11 Ground126,6 for
the pursuit and Point12 Ground96,5 for detachment. Map356 is Map_30440, whereas
the separate scrolling scene624 is Map_30440_1. Confusing these would miss the
required choreography and produce invalid intermediate routes. The native
mode flags934/935/936 describe platform/train/final-platform phases; fixture
states preserve them instead of treating these scenes as an ordinary map swap.

The engine's four objectives now require native115 and not123. Its final
conversation requires ready372/123, and arms world1 completion. Earlier stages
have mutually exclusive real receipts and exact groups; an unknown prerequisite
does not validate a later objective. Standing near a lever never records its
operation. Naming, dialogue, battle and scrolling remain native-owned.

## Source boundary and checks

The existing author calls tools/extend_phantom_train_guide.py after its original
brief engine recipe is built. The small extension preserves its controls and
adds only prerequisites, earlier stages and a final actual-exit arm. A stable
original-engine template validates the maintained author without depending on
an ignored checkpoint file. Exact source hashes for27 selected controls,
continuations and initializers are in phantom_train_native_20261103.json.
Game bodies remain in the external ../tools/mapdump archive, never this tree.

Only existing instruction keys and the train introduction are reused; all
twelve-language catalog values remain unchanged. Actual-consumer checks pass16
stage/engine/boss/departure witnesses and seven intermediate-floor continuations,
23 production route plans in total. Both lever phases, all eight engine states,
unknown final receipt and actual world1 readiness pass. There are21 new model
states and207 resolved physical recipe identities, with no failures. Eighteen
original discovery entries now have explicit dispositions, separating optional
dining and automatic marker scenes from manual operations.

## Exact bake and projection corrections

The first car's two west exits previously reused its east Point1 arrival.
Native0162 branches by facing: east uses624 Point1, west uses Point2. Four exact
physical actors now have qualified approach masks and real destinations. Ten
binary bytes change; every other native object, grid, dimension, master and
transport value is preserved. Only each derived transfer requires109; the
underlying native message event remains available beforehand. Four matching
destination guards are appended without changing any other map-state metadata.

The final boss check exposed a second exact projection failure. The outdoor
initializer branches on optional ghost characters17/18, then unconditionally
enables ev_e_0214. Treating those membership tests as opaque stopped the old
projection before that common tail, so remote directions called the boss control
unavailable. Offline inspection of External.Branch.BranchByCharacter at B5B680
proves its jump address is operand3; the native function delegates that value to
Main.JumpCommon. The matching DLL/dump hashes and external evidence locator are
in the native manifest. No native function is executed or hooked by this fix.

Only the exact hashed12-instruction initializer compiles those two branches
through the existing unknown-branch/intersection VM. Membership stays unknown:
the optional ghost group is not certified, while the unconditional boss group
is retained. Exactly code rows3/4 change. Other scripts and all runtime VM code
stay unchanged. This is a minimal correctness integration; no general character
snapshot, new opcode, framework or closed-system redesign is introduced.

The earlier prototype used the wrong absolute offset for ghost flag7 and old
pre-scene player poses. Preserved diagnostics were corrected to519 and the
actual boarding/pursuit/jump endpoints before acceptance. A diagnostic join of
map-master script_id against the event-script master was discarded: those IDs
are not an established common namespace. Binding uses the actual map library
and pinned source body. No game behavior was inferred from that join.

## Completion evidence

Native-stage/model qualification:62.866s/445837312B/exit0/drained. Complete
intermediate-floor/controller qualification:61.661s/445775872B/exit0/drained.
The final boss correction and complete16-witness replay pass in53.274s resource
time,126930944B peak; the targeted build/check itself is22.913s. Source/locale/
semantic/wiring qualification passes5.195s/495939584B/exit0/drained. These are
throttled offline measurements, not audible or game-frame results. The original
20–30min evidence slice completed its first missing-stage proof; full integration
took longer because two real transfer/projection defects were then found and
repaired. No supporting architecture branch was added. Exact identities and
preservation proofs: build_check/phantom_train_20261103/completion.json.

Current original-inventory coverage is589 logical/621 physical records;350 FFV
plus714 FFVI candidates remain,1064 total. Supplemental boarding/ghost/callback
contracts are source-checked separately. This guide/source design is Closed;
remaining work is the original audit and final combined/native delivery, with
natural gameplay confirmation distinct.

All tools/builds use the shared2GB aggregate/25%CPU/below-normal/one-worker
resource slots. Fixture times do not establish moving-actor response, audible
latency or game-frame cost. Natural confirmation is queued after the final
combined all-six audit delivery; no gameplay test is requested here.
