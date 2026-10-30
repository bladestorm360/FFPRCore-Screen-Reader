# Inward shop arrivals and remote return exits

Human-initiated follow-up to the Closed October26 listing policy. Source complete,
Closed; combined deployment pending. Estimate25-45minutes through source/offline/docs;
Sol only. Scope: Amur
Weapons and Replito Magic return rows shown while still in town, including
before entering the shop. Graph, current-floor native exits, map identity and
selected-route semantics stay unchanged. Saronia Events checkpoint is Closed.
Boost, Draw Attacks and inter-map vehicle state remain queued separately.

## Pre-change proof

Actual-source tools/remote_exit_check.cpp reproduces both rows with real FFIII
bakes and native-init projection:97->102->97 and138->139->138. Both whole-map
states are known, PointIn IDs unique and relevant portal groups unconditional.
The earlier missing-initializer hypothesis is disproved: Selection defaults to
known for an absent initializer, correctly preserving default entities. The
earlier research statement that both endpoint distances are1 was also wrong.

Amur: Town door(18,18,L2), native return arrival(18,19,L2) one cell away;
Weapons door(7,14,L1), arrival(7,12,L1) two cells north, facing Up1 into the room.
Replito Magic has the same pattern: shop door(10,13), arrival(10,11), facing Up1.
All coordinates respect Tiled gid bottom anchoring. Raw objects and master/script
exports remain outside Git in parent tools/mapdump. No native snippets here.
before_report.txt retains the2 expected failures, plus passing all-six graph
and preservation coverage, under build_check/remote_returns_20261028/.

## Bounded correction

The listing prepass now retains each unique active PointIn's native facing.
The pure reciprocal-door policy still requires known states, exact arrivals,
same layers, decreasing discovered depth and exactly one opposite physical pair.
The existing on/adjacent case remains. A second valid case is exactly two cells
in a straight line behind the arrival, with native facing directed inward away
from that door. Unknown/wrong facing, diagonal proximity, larger gaps, ambiguous
IDs/matches, loops, one-way, world and current-floor exits remain unchanged.
This is native arrival metadata proof, not a larger unconstrained search radius.

Only copied integer metadata is added to the existing worker prepass; no native
hook/read, polling, input, setting, translation, bake or route algorithm change.
Work/state bounds stay the existing400-floor limit, portal/point index sizes,
pair comparisons and cancellation. No text deduplication. Existing all-six test
coverage is retained; both reported towns and focused facing/diagonal negative
cases are added. The Dwarven Inn's verified inward arrival is now recognized
rather than deliberately left as uncertain. Native-frame/audio/gameplay
confirmation remains unmeasured.

## Source acceptance

Real-bake before2 failures/after0: both reported returns disappear without any
visit-history requirement. The all-six largest connected graphs retain real
destinations and current-floor ownership; representative projection episodes
including worker startup/join range0.512-1.629ms offline. No speedup/native-frame
claim. Existing negative cases preserve wrong/unknown facing, diagonals, layers,
missing/duplicate arrivals, ambiguous pairs, one-way and same-depth exits.
The native Gysahl pub integration now correctly lists its onward exit while
omitting its certified town return; both physical native exits remain baked.
This changes only that test's expected presentation, not the Closed Elder code.

Final canonical build passes8291 names/0,142 used/349 defined/0 undefined wiring;
smoke passes47 exports. DLL11073536 SHA256
0e8f7886e3923dd21569bfa51c1f967e871fe8204db55b74a13c393590b39ede.
Evidence:build_check/remote_returns_20261028/; final_report.txt preserves the
all-six results, integration_before.txt preserves the obsolete pub expectation.
No new native ABI or user-facing string. Final payload security scan and install
are part of combined delivery, with gameplay/native/frame/audio limits retained.
