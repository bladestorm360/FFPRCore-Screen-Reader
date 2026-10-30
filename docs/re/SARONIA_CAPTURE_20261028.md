# Saronia missing-entry capture investigation

Human reopened the previous deployed correction: Events still has20 rows and
no Saronia Airspace in session357177250/pid4848. The Statue barrier labels prove
the revised data loaded. Earlier manual-state route fixtures and a mocked
synthetic-target owner fixture did not establish the user's actual captured
state or the full discovery-to-publication pipeline. Their passing results are
insufficient acceptance evidence, and gameplay remains failed.

Active: determine actual exclusion from read-only saved state/native code,
reproduce the responsible production path offline, correct that cause, verify
selection as well as listing, then scan and deploy FFIII. Sol only; no game
launch/control/runtime probe. Estimate45–75minutes for this checkpoint; report
an overrun rather than repeating unsupported fixes. Prior closed designs remain
closed except minimal integration repairs needed for this reported regression.

After verified FFIII deployment the latest human request explicitly resumes the
separate all-six map-event/reachability audit, including missing live capture,
publication/filtering and availability paths, rather than bake parity alone.
Existing audit records and completed work are preserved.

Raw save copies, decoded save state and proprietary native exports stay outside
every Git tree under ../tools/saronia_capture_20261028. User saves are read only.
Evidence receipts and factual results go in build_check/saronia_capture_20261028.

## Actual cause and source-to-output evidence

Read-only decoding follows exact native FileOperationUtility.FileRead RVA73C230,
RijndaelUtil.Decrypt RVA742A40 and ZipOperation.Decompress RVA7501C0. No save was
modified. Latest saved map4, player cell114/108, transportation6 and scenario73=0
match the reported position and show the scene has not completed. Format
constants and decoded content remain outside Git; the original extractor's
--save-context mode exports exact native methods there too.

The expanded actual-source fixture accepts an optional external saved flag bank,
then uses the actual initializer projection/effective objects to populate the
whole powered-trigger/collider capture. Saved state yields263 active objects,
17 hard-gate cells and91 trigger cells. The hard-gate count matches the live log.
The previous fixture omitted powered triggers and therefore never exercised the
live transition mask. With those triggers present the unchanged production code
fails16 assertions: direct route, distance, helper discovery and Events listing
all fail for6/13/15/16. Removing only triggers makes discovery succeed. An open
Saronia target has a reachable adjacent tile but its final edge is rejected.

current_transitions correctly marks the native scene's powered Warp cells as
terminal. edge_allowed previously required goals_open to enter any terminal,
but airborne scene routes intentionally disable that flag because it also opens
ground door/town terrain. The helper additionally left its goals empty and used
the default ground-goal behavior when testing the final edge. These are the
verified causes; group-cache/vehicle-variant fixes alone could not correct them.

The minimal integration correction adds Request.terminal_goal, default false.
Only a reviewed selected airborne scene/transition uses it. It permits only the
exact goal's terminal edge, independently of ground terrain opening. Discovery
keeps its flood terminal-free and permits the selected candidate only for its
final edge; flight requests retain actual aircraft identity/terrain. Distance
ranking and selected routing use that same distinction. Other terminal cells,
collision/directional gates and vehicle terrain remain enforced. No native
hook, getter, timer, capture work, language string or baked asset is added.

With actual saved flags the corrected helper and actual filter publish exactly
one Saronia Event; actual selected planning reaches the exact selected cell in
flight for all four Enterprise forms. Completed-state removal and ordinary
castle/ground-gate preservation remain checked. The deterministic canonical
fixture now also includes the full active powered-trigger/collider projection,
so builds can no longer pass by silently excluding the responsible input class.
Its different all-zero state has271 objects/18 gates/97 triggers. Raw before and
after reports are preserved. Source verification complete; delivery running.

The native snapshot reader and scanner owner-publication fixtures remain
separate component checks, not an end-to-end game probe. Offline results prove
the supplied saved-state/capture and production filter/planner behavior; actual
gameplay/frame/audible behavior cannot be guaranteed100% without observing the
running game. No such observation or game launch is authorized/performed here.

## Delivery checkpoint — Closed, gameplay pending

Canonical build8321/0 names,142/349/0 wiring and complete checks pass, including
the now-full-trigger standard Saronia fixture and actual saved-state replay
(0 failures,0 wrong-owner calls). Saved-state final aggregate2388.972ms includes
all route/filter/negative cases on offline workers during other verification;
it is not a callback/frame/audible measurement. No native workload was added.
Smoke47,67 exact resources,136 indexed registrations, six unchanged bakes and
12 languages pass. DLL11097600bytes SHA25610598fd6db714359712e9c136d2a005c9710529a37b459f48db96ae908d9120d.
Defender no threats in22 exact staged files; exclusions query OS-denied/coverage
unknown, no signing certificate or protection changes. FFIII deployed and all22
installed hashes match,144 protected files unchanged, other five payloads unchanged.
Recovery and final receipts:build_check/saronia_capture_20261028/.

Finite source/delivery checkpoint complete within45–75minute estimate. Gameplay
confirmation remains separate. The all-six map-event/reachability audit now
resumes under the human's latest explicit request. No automatic delegation.
