# Ancient Castle complete statue objective

Source/focused Closed FFVI map/story audit slice. Source, physical routes,
controller, all-six references, locale and semantic checks pass. The original static
recipe ended at518 Ground14,32, a construction-scenery position with no physical
route after the stairs opened. It also omitted the enabling statue and the real
final interaction. The actual production planner reproduces both problems:
the old waypoint has no path, and the final statue is unavailable without428.

The corrected existing guide has three native operations: examine517 NPC2 at
Ground14,11 for428; examine517 hidden-switch Entity12 at Ground15,14 for429;
then examine519 Entity10 at Ground12,9 for432. The guide covers517/518/519 and
uses root432 completion. The first statue is the native prerequisite for the
last interaction; opening the stairs alone is insufficient. Diary430 is a clue,
not a required receipt. The side trigger517 Entity30 is another operation point
for the same first statue, not a fourth step. No reward name is added to guide
instructions, and treasure labels keep their existing type-only policy.

## Ownership, source and extension

The maintained static guide/controller/Navigation service owns the sequence.
No runtime source, hook, snapshot field, input injection or timer changes.
Existing reactive movement/readiness/explicit-repeat observation reads the
current operation receipt and one final receipt; it never polls a full flag bank
or waits on the native thread. Native script/dialogue/animation owns the action.
The core accepts completion only when field control is ready. Starting after432
does not replay earlier interactions or require checking an old prerequisite.

tools/author_ancient_castle_guide.py pins eight exact source fingerprints and
the actual actor identities/cells. tools/author_static_puzzle_receipts.py now
also validates both added native operations and their flags. One concise
“Examine the statue” key has all twelve supported languages, while the existing
switch instruction is reused. Twelve story guides and four other static recipes
are preserved. No map bake, geometry or map-state program changes.

Native bodies remain outside Git in ../tools/mapdump. Original factual actor/
script evidence, failing old-waypoint result, source recovery and resource
receipts are in build_check/ancient_castle_20261103. Source flags428/429/432 are
the actual completion receipts of0886/0887/0890. The switch's native temporary
map518 visit and517 return use0887_1/0887_2; the guide waits for the existing
ready continuation and resumes from the real current floor.

## Physical and controller evidence

Thirteen physical/resume/negative cases pass, including all four real throne-room
arrivals, the actual518→519 route and full517→518→519 continuation. Starting in
the final chamber without428 returns through the real Bottom/Ground stairs to
the first statue. Six intermediate-floor continuations retain actual native
arrival layers. The final chamber's optional dragon does not become a compulsory
battle or trap shortcut. The old construction waypoint and inactive final actor
remain negative witnesses; no collision or gate is relaxed to make them pass.

The actual generic controller checks first-stage selection, native movie waits,
map-transition readiness, real final actor, unknown final receipt, adjacency not
completion, late-route rejection, completed-state resume on all three floors
and return to the missing prerequisite. All89 controls across52 recipes and
all-six binary references pass, preserving18 other entrance approaches,12 maze
resumes,48 reward routes and24 guardian handoffs. The first controller and
reference runs exposed their old87-control census assertions: adding two real
operations correctly makes89. No production controller/route failure occurred;
both old counts and failed reports are preserved.26 exact original discoveries
have explicit mechanic, gate and optional-clue dispositions.

Initial routes75.778s/444956672B; the full route/controller checkpoint66.615s/
442920960B ended only at the old controller count. Its corrected controller pass
was followed by the old reference count;28.497s/277921792B/exit1/drained retains
that distinction. Corrected all-six references42.101s/442626048B/exit0/drained.
The final source/locale/semantic/wiring receipt and exact source/report identities
are in build_check/ancient_castle_20261103/completion.json. No more internal
refactoring is authorized by this closure. Native output/install/gameplay remain
separate. Coverage615logical/647physical original records,350V+688VI=1038 pending.

Tools/builds retain shared admission,2GB aggregate memory/25%CPU/below-normal/
one compiler/worker. Offline fixture results do not certify audible or game-frame
latency. Combined native output/installation and natural gameplay confirmation
remain pending. After final delivery, use Shift+G after each statue/switch scene;
it must continue beyond the opened stairs to the last statue and finish only
after its native result, without requiring undisplayed coordinates.
