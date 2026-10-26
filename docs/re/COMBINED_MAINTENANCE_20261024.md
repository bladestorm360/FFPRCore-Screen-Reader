# October24 combined maintenance delivery

Implementation and offline verification: complete. Deployment: verified in
all six games after the user closed FFII. Every installed payload matches the
scanned stage, with settings/dependencies preserved.
Gameplay/listening confirmation remains pending. No game was launched, controlled
or probed, no dependencies installed/upgraded, and no release, commit or push made.

## Included corrections

| Feature | Maintained contract/evidence |
|---|---|
| Roundworm room/NPC and H status summary | [Roundworm/H](ROUNDWORM_STATUS_20261024.md); HP current/max, visible MP current only |
| Imbibe and applied numerical statistic changes across six games | [Imbibe](IMBIBE_20261024.md), [stat effects](STAT_EFFECTS_20261024.md) |
| Persistent battle Poison cure and shared removal paths | [Status removals](STATUS_REMOVALS_20261024.md) |
| Exact Airship/chest selection after battle, distance re-sort and native recreation | [Target retention and interaction labels](TARGET_RETENTION_20261024.md) |
| Palamecia landing pad with mounted/parked Airship; vehicle-assisted event lists | [Mounted vehicle events](VEHICLE_EVENT_ROUTING_20261024.md) |
| Fynn throne-room native exit companion | [Transition classification](THRONE_EXIT_20261024.md) |
| Shared Shift+I current rendered control bar | [Controls recovery](KEY_HELP_20261024.md) |
| Hidden-passage Objects named by their concealing objects | [Reviewed walls/Rock/Clock](HIDDEN_PASSAGES_20261024.md) |

All earlier Leviathan/three-part Parasite, map/Bestiary/targeting/input/soundscape
corrections remain included. The source walkthrough and documented private
planner exception remain in the feature index/runtime architecture. All12 locale
catalogs and scoped regeneration recipes are retained. Luna's bounded13-key
translation handoff was reviewed and integrated by Sol; listening in those
languages is unconfirmed. No further delegation was used for these corrections.

## Final offline evidence

Canonical build.bat exits0:7,671 native-name checks/0 missing;131 used/339 defined
wiring names/0 undefined. Every required offline checker passes. The actual
scanner/filter fixture covers all six game contexts, changing distance/order,
missing/reappearing model and newer input against recreated handles. Real FFII
terrain/initializer fixtures prove Palamecia mounted/parked/missing Airship cases
and the exact throne-room exit with a separate genuine scene retained as a hazard.
Leviathan remains375 steps from the logged Ship position; distant Tropical Island
routes395/380 steps remain valid, with no distance cap. Static native ABI/phase
receipts are outside Git. Fake owners/mutators do not execute game code.

All-six direct/bulk removal fixtures cover carried Poison without Add history,
no-op, current/stacked status still applied and cleanup. Stat fixtures cover54
native base/temporary fields, actual dictionaries/direct writes/action completion,
later increments, cancellation, busy-lane retry and the64-result overload notice.
The localization check covers24 keys/288 values and16 maintained new recipes.
Controls/native-cue fixtures pass. Initial compile/fixture failures and their
corrections are retained rather than hidden; no production guard was weakened.

Final fixture timings: unchanged stat action/capture/completion median12.0us,
p9913.3us/max30.3us;26 changed modifiers through worker/main/fake output
23.2/43.4/53.9us. The first legacy report label says first fake speech even though
that unchanged scenario produces none. Scanner normal2,000 rows: four native
slices/peak44us/aggregate141us; busy8,192 rows: sixteen slices/peak243us/aggregate
3.052ms, plus scheduling. These are offline fixture measurements, excluding actual
native/game frames, Prism/audio and audible completion. Live tails remain unknown.

smoketest.bat exits0: isolated DLL load, all47 exports resolve/forward, a real
forwarded export succeeds and boot/logging complete without a game launch.
All49 embedded data resources and18 sounds match current sources.

## Exact payload, security and installation

DLL10,871,808 bytes; SHA256:
cac7266d799931e353f9a445498ee57abc4e417b5a145ee582f74804a533870b.
Enabled Defender1.461.181.0 (reported current locally) scanned the exact22-file
stage on2026-10-24T19:30:10Z: exit0/no matching detections/hashes unchanged.
Scanner signature is valid. Four DLLs remain unsigned; no signing certificate is
available. Exclusion query is administrator-protected/unverified0x80070005.
No protections were changed, uploads made or universal scanner acceptance claimed.
No outer package/download-reputation test is part of this local deployment.

Each installed game matches every file in the scanned22-file stage. After the
user closed FFII, deploy.bat ff2 succeeded; a fresh deploy_preff2.json captures
the completed live session's settings. All144 protected configuration/dependency
files across the six games remain unchanged against that post-close baseline.
The earlier five-game deployment and FFII process-guard deferral are retained
in deploy_partial.json as history, superseded by deploy_after.json and
installed_payloads_verified.json. No dependency upgrade or game launch occurred.

Evidence and recovery remain ignored under build_check/stat_effects_20261024/:
verification.json, embedded_resources.json, source hashes, build/check reports,
smoke logs, security_final_predeploy/, deploy_before/finalbefore/partial/preff2/after.json and
six previous installed DLL/SDL recovery copies. Final FFII verification binds
all22 installed files to the same scanned stage and preserves its fresh settings.
Short natural checks are first in [TESTING](../TESTING.md).
