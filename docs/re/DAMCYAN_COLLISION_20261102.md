# Damcyan walls and Hovercraft wording

FFIV delivery Closed November2 at18:16UTC: payload011f0dc2 passed full canonical,
smoke, exact22-file scan and guarded/hash-verified installation;144protected and
otherfive unchanged. Receipt: sibling Class-Delivery/build_check/
ff4_party_keys_20261102/completion.json. Natural wall/Fly confirmation separate.

Active source/focused corrections, not installed yet. Owner reports the first
East movement toward Jail B1 is blocked. Actual game-directory FFIV log
782364.296/782365.468/782368.312 pins map42 and the complete exit leg. Native exit
5 at5,10 and the reported displacement identify start6,5 Bottom without asking
the player for hidden coordinates. Source submap Map_20041_4 is24×15.

The actual East neighbor7,5 has full native collision1. A hidden-passage cover
group also marks it. The baked data is faithful; the router wrongly promoted
any cover-marked wall to fully open floor. It also widened directional masks.
Native hidden-passage groups control tile UV animation, while movement obeys
independent raycast/collision masks. Exact10-owner traces in all six matched
GameAssembly/dump pairs are retained outside Git at
../tools/passage_native_20261102/ff1..ff6/. The initial eight FFIV owners remain
separately at ../tools/damcyan_native_20261102/ff4/. No game was run or probed.

Two existing physical-mask decisions now retain the real collision value:
field/baked_map.cpp walkable and baked_router.cpp edge_allowed. No bake, native
wall, story flag or scene pointer is modified. All six owned route/flood/landing
consumers share these decisions. The existing passage parameter/metadata remain
compatible; cover IDs never grant permission to cross a solid tile.

The all-six real-data check finds3075 genuine hidden-floor cells and955
cover-marked walls (III81,IV485,V277,VI112). All real floor remains open; all
walls stay closed. FFI/II have no cover records. Seven old failures become zero,
including teleport's former acceptance of a cover-marked wall. From the exact
reported Damcyan start the route now begins South and crosses no physical wall.
Actual planner/browse integration also passes the case. Evidence:
build_check/damcyan_20261102/geometry_before_report.txt, route_check_shared_report.txt,
browse_route_check_shared_report.txt and resource receipts. No runtime-latency
claim follows from throttled offline checks.

Hovercraft2 is native type7 in FFIV. Its list approach formerly chose the Sail
phrase; it now uses the existing fully localized Fly phrase. Ship/submarine
classification and landing/boarding semantics stay unchanged. Actual Antlion
entrance173 from hovercraft-only terrain146,42 produces Fly21.0steps then land.
The all-six existing browse and navigation-follow-up consumers pass. Other type7
game meanings retain their current behavior. No new strings or settings.

Before full candidate qualification, identical common check objects are reused
under the unchanged build contract; see BUILD_SPEED_20261102.md. This avoids
repeating thirteen compiler inputs across many checks, with exact generated
option equivalence and three actual consumers qualified. Candidate source,
canonical/smoke/security/recovery/closed-game delivery and natural gameplay
confirmation remain distinct. No replacement is installed until those checks pass.
