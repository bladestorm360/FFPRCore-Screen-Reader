# Mounted world-to-interior first hops

Source/focused checkpoint complete; combined build and deployment pending.
This is part of the owner's resumed all-six audit in the
[single ledger](MAP_AUDIT_FOLLOWUPS_20261028.md). No game was run.

The existing98 boarded tests mainly targeted entrances on the current world.
The new production-consumer check compares a successful local entrance plan
with a remote goal at that entrance's actual interior PointIn. It uses the
same carrier, position, native master rows, map-state policy and shipped bake.
All six games supply witnesses;50 model/world profiles qualify. Ordinary ship
models without an eligible entrance in the bounded sample, and FFIII's
submarine under the initial zero-flag sample, are explicitly unqualified by
this check. Previously qualified actual Nautilus/underwater contexts remain
separate evidence; this check never grants upgrades or ignores entry rules.

Two baseline failures reproduce: FFV Boko/type11 and FFVI Magitek/type12.
The local planner correctly enters a ground door while riding. First-hop
reconstruction instead demanded the generic landing planner, which correctly
has no voluntary landing action for these two modes. A legal local route was
therefore discarded. The graph now uses their riding terrain, and reconstruction
uses a legal riding route for native ground-entry modes, retaining its carrier
identity. Ordinary aircraft continue to require legal landing. Native Player's
walking correction is preserved.

After correction all50 mounted witnesses pass, plus all-six native Player
first hops, Mist's48 route/browse pairs and two cave continuations, and the
shared route-worker's98 boarded/582 exit cases. Native-owner violations0.
Before source/executable/report, final source identities and reports are in
build_check/navigation_resume_20261031/mounted_checkpoint/. The serial enforced
resource receipts are mounted_before_budget.json and mounted_fixed_budget.json
in the parent directory. These fixture costs are not native gameplay timing.

## Longer approaches and underwater interior continuation

The resumed distance check starts on real riding terrain without a nearby foot
landing. Five additional local-entrance/remote-interior mismatches reproduce:
FFI and FFII ships, FFIV black chocobo, FFV ship and black chocobo. Their root
graph used a foot flood, despite the successful local route already supporting
their actual riding and landing. Extending that existing reach policy passes
all eleven qualified distance profiles. The five-check integration chain also
passes; receipt mounted_distance_fixed_budget.json records255.783s and
491757568B peak, with no cap increase.

FFIII submarine entrances are native Event2/scripted world exits rather than
ordinary GotoMap records. Including their real aliases and powered trigger
observations reproduces a sixth mismatch:44,142 to Temple of Time's44,138
entrance succeeds locally, but requesting interior242 fails. A submarine root
must flood in its riding mode, retaining its entry selectors rather than seeking
a voluntary foot landing. This correction passes all twelve longer profiles.
Before evidence is in submarine_interior_checkpoint/; the qualified targeted
receipt submarine_interior_fixed_budget.json records40.761s/429334528B.

The separate underwater_interior_check then tests all four actual arcs from
the logged113,118 submarine origin, Surface Nautilus9 and Continent Invincible11
with a real saved Surface Nautilus9. All four submarine-to-room requests pass,
while all eight requests from the other worlds reproduce No path. The world
journey adapter previously accepted only world-map targets, so entrance-marker
success did not imply final-interior success. Baseline source/executable/report
and capped receipt are frozen in underwater_interior_checkpoint/. The minimal
continuation adapter and its projected indoor gate proof are being qualified;
this additional slice remains Active. No new DLL or deployment has occurred.

Extending the actual-consumer fixture to Doga Manor's native131/7,7 L1 arrival
and Village's151/16,8 L2 arrival reproduces five more failures: both from
underwater/Continent and Village from Surface. The adapter covers those two
reviewed doors alongside the four underwater arcs, proves their indoor leg
with the existing cancellable cross-floor graph, and publishes the actual
first world transition while preserving the original final goal. Current-room
proofs use projected native gates/chests/selectors rather than an empty live
inventory for an unloaded room. The existing connected-floor inventory rejects
unrelated indoor families before flooding. No Player reset or parked craft is
invented; the existing final Nautilus selector policy is preserved.

All eighteen route/browse pairs pass, including the Ground-layer0 Saronia and
Upper-layer2 Village receipts. Four moving dungeon requests retain the manual
dive receipt. Full arrival replay adds four submarine-to-dungeon continuations
and Village's dive→surface→ordinary entry. It reproduced a new post-surface
loop in the provisional adapter: forcing another same-world tunnel after the
Village landing pocket was already reachable. The adapter now checks the
ordinary landing/foot approach first and defers to that existing entry path.
Missing upgraded craft, immediate cancellation and native-worker ownership
guards also pass. The separate owner-capture check reproduced losing saved
craft when a continued interior goal was used outside World map filter; capture
now includes remote interiors, retaining the same-map zero-extra-read policy.
The final five-check shared integration run passes,272.363s/486039552B, exit0
and children drained. Exact native ConvertEntityPosition4628b0 dispatches
Last.Map.IMapAccessor slot4 (ConvertWorldPositionToCellPosition), then
OwnedTransportationData.set_Position66bd60 writes the inherited saved field.
The existing reader's X/Y grid-cell units are correct; no conversion fix is
required. Native world Z is retained by the producer but unused by layer1 world
navigation. Three exact native bodies/hashes remain external in
../tools/saved_world_positions_20261031/ff3/; factual contract receipt and26
frozen current inputs are in underwater_interior_checkpoint/final/. This
additional regression slice is source/focused Closed, with combined delivery
pending; both new checks are in the canonical build. These are offline checks, not native
game-frame, speech-response or natural gameplay confirmation.
