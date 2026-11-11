# Cultists' Tower complete objective and return

Source/focused Closed. Shift+G now covers the actual hidden switch421, hidden
chest179, summit chest423, forced return battle424 and descent to entrance476.
The old static guide ended at its switch. The new cultists_tower story recipe
covers all twelve native floors476–487, with completion only after the actual
entrance is ready. Partial saves, already-opened chests and already-defeated
boss states resume. The optional dragon is not made a required fight, and
treasure instructions retain type-only labels. All instruction keys already
have12-language catalog coverage; no catalog entry changes.

## Ownership and minimal corrections

The existing owned-snapshot rule model, native-ready controller and Navigation
worker own the complete sequence. No new native hook, party-membership reader,
timer, input action or game-thread wait. Original author tools and17 source
hashes are in tools/author_cultists_tower_guide.py and
docs/re/cultists_tower_native_20261103.json. Game bodies remain external.

The entrance initializer had an opaque BranchByCharacter before a later392
test. This conservatively retained phantom recruitment barriers even after
recruitment completed. tools/author_cultists_tower_state.py replaces exactly
one initializer row with the existing unknown-predicate/branch-intersection
representation. Membership remains unknown. The exact native B5B680 jump
operand3 was already proved for Phantom Train against the same0029 binary/
38aa declarations; this slice reuses that evidence. Both392-complete branches
now establish barrier absence. Before392, remote membership remains unknown;
a current-floor no-Relm fixture copies the actual absent event group, rather
than treating conservative remote candidates as live instantiated actors.

The return crosses five normal playable floors. A finish_through list carries
only those reviewed floors486/484/482/480/477 to the existing exit476 completion
arm. The model allows at most16 valid baked map IDs; the controller owns only
integers and checks them once per ready map transition. The default empty list
preserves all earlier immediate-exit behavior. Unreviewed detours, battle, stop
and new plans clear or replace the arm. No extra observation or polling.

Native chest script0880 opens the actual chest and sets423; only then does the
upper-floor initializer enable0881. Its native battle continuation records424.
The guide targets that encounter explicitly before the descent. Arrival beside
a chest/switch, entering the summit and starting battle are not completion.
The eight0776 contact markers are one native joining scene, conditional on
character9 and392; they are not eight independent puzzles or a required party
composition for entering the tower. Native dialogue/choreography owns that scene.

## Evidence, preservation and remaining confirmation

build_check/cultists_tower_20261103/completion.json pins source/reports. All24
physical/resume/negative cases and63 complete floor continuations pass. These
cover every floor, both hidden-door states, the summit, forced encounter,
descent and actual pre-recruit/no-Relm current-floor capture. The real model
resolves227 physical references and checks receipts, unknowns, incomplete late
hidden-chest visits, exact return floors and recruitment projection. The real
controller retains completion across ready intermediate floors, waits for the
actual entrance, rejects stale output and preserves old wrong-exit cancellation.
Earlier Darill choice-repeat, roof and other controller scenarios still pass.
The unchanged static controller/test sources retain their89-control/52-recipe
evidence. Source authors, all-six static receipts and wiring pass.

Thirteen earlier story guides, five static recipes, the complete locale catalog,
every old semantic row, all physical bakes/masters/terrain and every other map-
state field are preserved. Nine exact original discoveries are reviewed. The
initial23-case fixture exposed one real entrance failure, recorded as two failed
assertions. The one-row repair plus added no-Relm profile passes all24 cases;
the failed report remains archived. No collision was relaxed.

Repair qualification42.589s/444583936B; actual model/controller integration
45.952s/363900928B; source/semantic/preservation/wiring7.321s/285573120B, all exit0
with children drained, shared admission,2GB aggregate memory/25%CPU/below-normal/
one worker. These are capped offline fixtures, not audible/game-frame timing.
No game/probe/installation/new native DLL was performed. Combined native output,
delivery and actual gameplay remain pending; this design is Closed.

The original audit now covers641 logical/673 physical discoveries.350 FFV and
662 FFVI candidates remain; this does not close the all-six audit.

After final delivery, request Shift+G at the entrance and after each operation.
The guide must continue beyond the hidden switch/chest to the summit, identify
the forced encounter after the summit chest, and finish only after returning
to the entrance. An already-completed summit with an unopened hidden chest must
still guide that chest and the return. No undisplayed coordinates are needed.
