# Key-door integration regression verification

The owner reaffirmed that a reachable unlocking door must retain its valid
approach. The destinations behind it, rather than the door itself, own a story
gate until the player can actually unlock it. Owned usable keys/Thief or an
already-open receipt can validate the continuation; other prerequisites and
consumable quantities must remain effective. FF3 is already gameplay-complete;
no replay was requested or used to verify this integration.

The actual current shared collider0 correction was checked with the existing
real FF3 bakes, native capability reader,13 exact doors and complete route
consumer. All seven regression targets pass:

- Native key/Thief reader:37 cases; one content15 lookup, no inventory scan,
  native script invocation/write or worker native read.
- Consumable topology:13 doors,1853 real all-open cells, eight retained cover
  walls and native prerequisite chains with up to two required keys.
- Actual route/browse/filter and complete journey consumers:246859 assertions,
  215 chest profiles,65 door profiles,203 complete journeys,61 manual unlocks
  and490 native floor transitions. Missing/unknown capabilities, quantities,
  Thief, already-open receipts, nested prerequisites and final interaction
  positions are retained. No path walks through a still-closed lock; key
  consumption is simulated only after the explicit native unlock instruction.
- FF1 all-map key doors: six rooms/24 variants/14 layouts,24 control approaches,
  118 chest cases and143 routes.
- Cornelia door-versus-content consumers:277 cases and24 exact declarations.
- Locked-door interaction approaches retain actual native masks/activation;
  inactive or no-cardinal-approach cases remain explicit verification limits.
- Melmond trench/owned-ship consumers:126 cases retain the story/vehicle split.

Every target has zero failures and zero worker-native violations. The exact
FF3 maps/state/capability/gate records and shared routing inputs are unchanged
before/after these checks. No FF3 implementation repair was necessary. This
does not replace the remaining all-six map/story/vehicle audit or claim actual
gameplay responsiveness from fixture timings.

Evidence: build_check/ff3_key_regression_20261103/checks_complete.json, inputs.json,
progress.json and resource.json. Full tree529.036s, peak485986304B, exit0 with
children drained under shared admission, aggregate2GB/25%CPU/below-normal/one
worker. No game launch, attach, probe, new native DLL, installation or agent was
used. Source/offline integration verification is Closed; combined output and
natural gameplay remain separate. The original FF3 accepted design is preserved.
