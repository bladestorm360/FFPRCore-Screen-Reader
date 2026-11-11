# Owzer's Mansion: complete map and story guidance

The former flower-painting-only recipe did not cover the mansion. The new
fifteenth FFVI story guide covers the native warning, diary, light switch,
flower/woman/corridor paintings, timed left door, painting transfer, all four
hanging-chest encounters, final door painting, Owzer's boss scene and bookshelf.
Boss390 is not completion. Bookshelf391 and independent385–388 encounter
receipts together finish the entire objective. A later visit with391 already
set still routes to unfinished hanging-chest encounters without repeating the
boss. The fourteenth and earlier guides and five static recipes are retained.

## Native state and actual destinations

Eligibility is scenario298 OR378. Two disjoint conjunctions retain the existing
bounded rule model, without adding a native party query or guessing unknowns.
Native379 enables diary577;577 enables the reversible switch380. Paintings
record381,382 and384. These actual receipts, not proximity, select each stage.
The final painting writes389; the boss writes390; the bookshelf writes391,
joins Relm and returns to town. Leaving the boss chamber after390 first reaches
the real Relm contact event before the bookshelf route is requested again.

| Mechanism | Native source | Actual ready destination |
| --- | --- | --- |
| Timed left door |445 Event55, sc_e_1064 |445 Ground25,32; local2 becomes2 |
| Painting transfer |445 Event56, sc_e_0765 |445 Ground6,24; local1/area1 become1 |
| Final painting doorway |445 Event57, sc_e_0766 |445 Ground20,18; local1 becomes2 |
| Lower return |445 Event59, sc_e_0766_1/2 |445 native Point3; local1 resets and timer restarts |

Three new automatic travel arcs, three synthetic arrivals and the existing
return's ordinary-travel classification express those exact movements. Terrain,
layers, native objects, masters and every other room remain unchanged. A stage
check must start at the final player pose, rather than an old entrance point.
The initial diagnostic failures using old points were not evidence of missing
terrain or permission to relax collision.

Two exact initializer SysCalls only register/remove GimmickOwzer. Projecting
them as opaque stopped subsequent group/puppet facts, hiding the corridor
painting. The source-qualified compiler skips those registrations only in the
two pinned room initializers. Four exact decorative LookAt bodies retain the
native Ready/show effect. Collider actors participate in the existing physical
puppet visibility/placement projection; their ID domain is separate from PointIn.
No VM opcode, native script execution or flag write was added.

## Timed door and ownership

The matching callback Current.<>c.<SetGimmickTimerOwzer>b__742_0 at53F760 cycles
map-local3 through0/1/2 once per second. Native tables identify colliders85/86/87
and animations21/22/23 as left/middle/right. The callback shows the previous
collider, hides the next one, writes the next phase and plays the native sound.
An initial local3 value alone is insufficient evidence of a fresh opening.

Shift+G first routes to safe Ground23,23. At that cell it previews exactly one
cardinal step north. A second explicit request arms the cue. The exact native
callback, after the original, reads only the existing storage and two elements
of its verified eight-element map-local array. The hook checks native-owner
thread, actual map445, active cycle1 and true left phase0. It queues one owned
notification; no pointer survives, no route/speech/I/O runs inside that hook,
and no polling, timer change, extra worker or automatic movement is introduced.

Deferred publication rechecks request generation, field readiness, focus,
modals and the exact staging cell. Only the first quarter of the one-second
opening is eligible for Go now. A delayed task waits for the next native opening;
movement, map/battle changes, stop or focus/modal loss cancel readiness. There
is at most one posted notification. Normal field-ready story completion remains
owned by the existing controller, with391 armed only at the actual bookshelf
objective. An already-set391 never terminates an unfinished late chest visit.

## Zero-value collider correction

The first strict door check exposed a real shared error: an enabled instantiated
FieldColliderEntity with collision0 was discarded as if it were terrain None0.
Matching primary GetCollisionRect/SetBoxCollision/Init/CanEnter evidence was
checked in all six installed binaries. Each zero-valued object retains the full
unit rectangle and rejects entry while enabled. Its terrain-grid namesake has
different semantics. The shared object decoder, current capture consumer and
transport-specific gates now retain that wall; disabled objects remain absent.
Partial2–5, axis6–7, native terrain and existing vehicle selectors are unchanged.
This is the minimal integration correction demonstrated by the actual closed
left-door counterexample. It does not reopen the accepted routing design.

Native bodies remain outside every Git tree under
../tools/puzzle_audit_20261030/ff6_owzer_gimmick, ff6_owzer_owner and
collider_rect/ff1 through ff6. Factual manifests are
owzers_mansion_native_20261103.json and collider_zero_native_20261103.json.
The FFVI binary0029a22e and declarations38aa9c54 match the earlier evidence.

## Verification and delivery limits

The actual production planner passes33 route/resume/negative cases and eight
complete floor continuations. All hanging-chest encounters, every authored
operation, true-open/closed door phases, real post-animation arrivals and late
completed-story returns are covered. The actual model resolves279 physical
identities with no failures. Actual controller checks cover preview/readiness,
wrong door, stale notification, stop, movement, focus/modal loss, map/battle,
missing storage, foreign map and off-owner rejection; all older controller and
roof cases pass. Chocobo/Owen and shared transportation-selector checks pass.
Three concise instructions are supplied in all twelve supported languages.

The initial closed-door acceptance report and first controller compilation
failure remain archived. The former demonstrated the zero-collider bug; the
latter was an ambiguous fixture speech identifier, corrected without changing
production behavior. All source hashes and failures remain in the checkpoint.

The initial20–30min proof slice overran after the demonstrated native timer,
landing and zero-collider dependencies. The recorded scope review retained the
single complete capability and its necessary shared correction. Sol performed
all work; no agent, game launch, attach or live probe was used. Resource runners
enforce shared admission, aggregate2GB/25%CPU/below-normal/one worker. Fixture
observer10000 mean0.377us/peak18.6us measures added bounded observation only;
it is not game-frame, navigation-episode or audible latency evidence.

Focused integration229.795s/465936384B, source/reference/preservation14.985s/
286130176B and final exact-pose closure55.612s/452501504B all exit0 with children
drained. The six-game primary proof pins24 native functions; the mansion pins45
complete script hashes. Exact source/report identities and failed receipts are
in build_check/owzers_mansion_20261103/completion.json and its sibling evidence.

Source/focused evidence remains separate from the combined native build,
security checks, installation and natural gameplay confirmation. Earlier FF1,
FF4 and custom-sound payloads are unchanged. Eight exact original discoveries
bring the audit to649 logical/681 physical reviewed, with350 FFV and654 FFVI
logical candidates still pending. This does not complete the all-six audit.

After final delivery, request Shift+G at the mansion entrance and after each
operation. At the timed door, memorize one step north, request the guide again,
then take that step only on Go now. Continue after the boss to the bookshelf.
On a later visit, unfinished hanging-chest encounters must still be guided.
The expected result is full progression without hidden coordinates or visual
door timing. Actual speech-to-movement timing remains a gameplay check.
