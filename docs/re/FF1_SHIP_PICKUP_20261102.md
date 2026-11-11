# FF1 Ship pickup — source/focused Closed, installation pending

Actual game log803580/803582 reports Ship (Story gated), foot122,40 to223,224.
The available Airship is160,36; actual game preference mode2 enables vehicle
assistance. Native masters identify Ship4/type2, Airship3/type3 and Canoe2/type5.
The first fixture incorrectly swapped Ship/Canoe IDs; its endpoint failures
are retained as inadequate reproduction, not evidence of the reported block.

Corrected actual-planner profiles reproduce six failures: walking cannot reach
the boat through the available Airship, while an already-flying Spot route
stops over its water without a boarding shore. The multi-craft adapter was
limited to FFIII and arbitrary Spot semantics admitted the hover-only result.

The same bounded owned journey now also admits an explicitly selected FF1 Ship4.
Its legs retain real walking, boarding, flying, landing and final Ship boarding;
ordinary FF1 destinations and all other games keep their existing selection
policy. A mounted request to a different Ship requires the real shore approach.
Current Airship use remains valid with optional assistance disabled, while a
walking-only origin cannot auto-board the parked Airship. No gate is removed,
terrain relaxed, native pointer moved to work or new hook introduced.

Nine real profiles pass, including both canal states, on-foot/already-flying,
missing craft, walking-only policy and post-landing goal continuation. Every
final boarding step satisfies the existing native-qualified direction contract.
Retained FFIII journeys and shared navigation/boarding regressions also pass;
zero worker-native calls. Four-profile/retained-consumer job156.049s/496984064B;
expanded continuation37.251s/433065984B, exit0/drained under hard2GB/25%CPU.
Times are resource-bounded fixture evidence, not game-frame or audible latency.

Current new check is wired into the canonical build. Exact prior/corrected/after
reports, source hashes and resource receipts: build_check/ff1_ship_airship_20261102/.
FF1 canonical/smoke/security and guarded installation Closed21:57UTC with the
Shrine candidate, DLLb62ed342de0f6896.22 installed hashes/144 protected files,
other five games unchanged; Defender22 no threats/stable hashes. No game/probe.
[Exact delivery and limits](SUNKEN_SHRINE_LOOP_20261102.md).
