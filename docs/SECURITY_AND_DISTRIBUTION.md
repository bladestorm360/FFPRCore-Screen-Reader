# Build and distribution security

## November 21 FFIII Storyline clues

Source `e5243511b334977fa13b058c3f375e187a7ecc64`. The focused
`build_local.bat storyline_check story_filter_check` passed in 115.2
seconds under the enforced 2 GB, one-worker and 25 percent CPU limits.
Peak aggregate memory was 516,616,192 bytes. The campaign replay passed
86 FFIII objectives and five clue routes. Story-filter and live-trigger
fixtures passed for all six games. The largest gate batch took 2.329 ms
in the offline fixture. The build verified all 73 embedded resources
and the isolated smoke test verified all 47 forwarded exports.

The unsigned DLL SHA-256 is
`d24853b083d40ced2b5cf693e19c9fc85b3a4c463db5302f5b9298185534378b`.
Defender definitions `1.463.397.0` reported no threats in the 22 exact
files and no matching detection events. Staged hashes were stable. The
exclusion query was denied. The guarded install ran `deploy.bat ff3`.
It verified 22 FFIII payload hashes, 138 protected states and the existing
installations for the other five games. Gameplay confirmation remains pending.

Recovery uses the previous build source `5efb0d3`. Compact evidence is
under `../tools/ff3_storyline_clues_20261121`. Cleanup reclaimed 306,545,596
bytes from test artifacts and staging and retained the production cache.

## November 20 visual descriptions and Storyline

Built source `5efb0d37099f633c9306a78f11a51b9c624f3134` on master.
The 21,318,144-byte unsigned DLL SHA-256 is
`bb8b5356300e117231568a354174cc0b0e2cdfdd86e40fe7ac74d6700b1c9b9b`.
The canonical focused build used `build_local.bat` with 16 check targets.
The resource receipt records 287.0 seconds, peak aggregate memory of
525,459,456 bytes, a 2 GB cap, one worker and a 25 percent CPU cap.
It verified 73 embedded resources and 9,113 native-name bindings. The
isolated `smoketest.bat` loaded the same DLL and forwarded all 47 exports.

Defender definitions `1.463.387.0` reported no threats in the 22 exact
staged files and no matching detection events. Every staged hash was
unchanged. The exclusion query was denied. The guarded `deploy.bat`
installation verified all 22 files in each of the six Steam game folders
and 138 protected file states. The installer verified Prism as read-only.
The game processes remained closed.

The full procedure, maintained check names, installed hashes, scan
result, frozen corpus and compact check results are in the local
`../tools/visual_audit_20261119/final-delivery` evidence folder.
The reserved resource job and tested build inputs are reproducible
from the source checkpoint, the existing pinned dependencies and the
verified installed game inputs. Recovery sources for the preceding
payload are `270661f` for FFIV and `8f061ac` for the other five games.
Recovery of this build uses the exact source checkpoint above.

The offline scheduler fixture ran 300 synthetic callbacks across 132
native updates and measured a maximum dispatch of 0.528 ms. The movement
fixture ran 2,400 warm owner reads in 0.070 ms total with no queued work.
The world benchmark passed 24 worlds and 16 routes, with a worst worker
flood of 17.399 ms and route of 49.391 ms under the same resource caps.
The route-worker fixture observed 148.902 ms for its longest full
vehicle route, 25.436 ms for its longest filter and 0.829 ms for its
longest parked cancellation. Scene observation tests cover the loading
gate, delayed and cancelled speech, exact draw state and the bounded
scan of up to eight emitters per frame for 16 frames. These measurements
are from offline fixtures. In-game combined lag, audible timing and
end-to-end gameplay confirmation remain pending.

The cleanup receipt records 1,848,933,018 bytes reclaimed from generated
builds, test output, serial project files and payload staging. The
source, frozen input evidence and installed files remain available.

## November 19 FFIV hovercraft journeys

The focused browse, landing and route worker checks passed with 67 verified
resources and 47 forwarded exports. The unsigned DLL SHA-256 is
`10462480d1dac8ce81f0c2ec3effcf59d2b9e8cc3c0b8e305f126b39e9437e87`.
Defender found no threats on the 22 exact payload files. The exclusion query
was denied. The installer verified all FFIV payload hashes, the 138 protected
file states, and the other five games. Recovery uses `8f061ac`. Receipts are
under `../tools/karnak_routing_20261119/ffiv-hovercraft-journey`.
Gameplay confirmation remains pending.

## November 19 vehicle destination focus and FFIV markers

The focused scanner, filter and browse checks passed with 67 verified
resources and 47 forwarded exports. The unsigned DLL SHA-256 is
`f39aed2e86ce0f50d0619453684e044d49883da055954d00115f8f0df7e72c2b`.
Defender found no threats on the 22 exact payload files. The exclusion query
was denied. The installer verified all 22 payload files in each of the six
games and 138 protected file states. Recovery uses `aa7d358` for FFIV and
`10ace35` for the other five. Receipts are under
`../tools/karnak_routing_20261119/vehicle-marker-focus`.
Gameplay confirmation remains pending.

## November 19 FFIV Sealed Cavern rope crossings

The focused browse, trap and route worker checks passed. All 67 resources and
47 forwarded exports verified. The unsigned DLL SHA-256 is
`2a6aa8a941543f14a195955f8cfb6039c58380531ccd65b4061e16fa5d8a28f8`.
Defender found no threats on the 22 exact payload files. The exclusion query
was denied. All 22 FFIV payloads matched the scanned files, and the installer
verified 138 protected file states and the other five games. Recovery source
is `10850d9`. Receipts are under `../tools/karnak_routing_20261119/ffiv-sealed-cavern`.
Gameplay confirmation remains pending.

## November 19 FFIV Moon entrances

The focused browse and vehicle landing checks passed. All 67 resources and
47 forwarded exports verified. The unsigned DLL SHA-256 is
`135850aca7858b90cff6d17dff5d7ed0f96004376a5ad0056c4d35f48a7c88fd`.
Defender found no threats and reported zero matching events in the 22 exact
payload files. The exclusion query was denied. The installer verified all
22 files in FFIV, all 138 protected file states and the other five DLLs.
Recovery source is `10ace35`. Receipts are under
`../tools/karnak_routing_20261119/ffiv-moon`. Gameplay confirmation remains
pending.

## November 19 field pause and spell targeting

The focused build passed pause opening, all-six field magic range ownership,
command context and native name checks. It verified all 67 resources and
47 forwarded exports. The unsigned DLL SHA-256 is
`9701e3afd36abd0ffa2998c4ed0c4c733368c3bdd18738f41dd176be6d89f9cc`.
Defender found no threats and reported zero matching events on the 22
payload files. The separate exclusion query was denied. All six installed
copies match the scanned payload, and 138 protected file states match the
initial inventory. Recovery source is `6ecf964`. Receipts are under
`../tools/karnak_routing_20261119/field-menu`. Gameplay confirmation remains
pending. Remote `master` was verified at `10ace35a583208e5e9d79bef846d91c7fe2babe6`.

## November 19 shared save-point and FFV navigation fixes

The focused build verified 67 embedded resources and completed in 159.2
seconds with one worker, a 25% CPU cap and a 513,232,896-byte peak job.
FFV story contacts passed 49 assertions and 31 route legs. The browse,
scanner, filter and separate trap checks passed. The smoke host verified
47 exports. The 13,475,328-byte unsigned DLL SHA-256 is
`38e01de3caab8c27f27f3583f083e0b5a6ffcf8a3a274d1279f1906ef64b0586`.

Defender found no threats in 22 exact payload files and reported no matching
detection events. The separate exclusion query was denied. The installer
hash-verified the 22 files in all six Steam targets and preserved 138
protected file states. For recovery use `f7d0043` for FFV and `a6008be` for
the other five games. Installation receipts are under
`../tools/karnak_routing_20261119/ffv-navigation`. Gameplay confirmation
remains pending. Remote `master` was verified at
`6ecf964424e71fb9953e2bbeb4012650a89d58e8`.

## November 19 FFV Karnak world routing

The focused build passed FFV story contacts, browse routing and route
worker, verified 67 embedded resources and completed in 192.1 seconds.
It used one worker with a 25% CPU cap and a 514,588,672-byte peak job.
The isolated smoke host resolved all 47 exports. The 13,474,304-byte
unsigned DLL SHA-256 is
`b2c9ed50cdf980c99e83d48317e0346c83750723001500adb963c36fa94cf638`.

Defender definitions `1.463.368.0` found no threats in the 22 exact
payload files. All file hashes stayed stable, and there were zero matching
detection events. The separate exclusion query was denied. FFV's installed
hashes matched all scanned files. The other five DLLs and 138 protected
file states matched the pre-install inventory. The previous FFV DLL was
`7c4580631476988bd9fcf0c3755142e55e5848686e549dea72924fdc62c79192`
from `a6008be`. Rebuild that commit for recovery. Evidence is under
`../tools/karnak_routing_20261119`. Gameplay confirmation remains pending.

## November 13 combined door, route and boss update

The focused build ran five offline checks, matched 67 embedded resources,
and verified 8,450 native names. The isolated smoke host resolved all
47 exports. Build time was 203.3 seconds with one worker, a 25% CPU cap
and peak job memory of 523,952,128 bytes. The unsigned 13,474,304-byte
DLL SHA-256 is
`7c4580631476988bd9fcf0c3755142e55e5848686e549dea72924fdc62c79192`.
Defender definitions `1.463.284.0` found no threats in the 22 exact
payload files. Hashes stayed stable and no matching detection event was
recorded. The OS denied the separate exclusion query.

The established installer updated all six Steam copies after the games
closed. All 22 installed files matched the scanned bytes in each game,
and all 138 protected file states matched the pre-install inventory.
Receipts and prior per-game hashes are under
`../tools/boss_visuals_20261112/walk-open-delivery`. Source recovery starts
from `8d7143c7642a70e237a8648b6f6626902c98d2ef` for the previous battle
fix, `993e06e` for the previous FFIV build, and the per-game delivery
records for other previous builds. No game was launched for verification.

## November 13 FFIV and FFV boss sprite correction

The focused build passed the production boss sprite, battle entry and
battle feedback checks in 51.7 seconds. It verified 67 resources and
8,450 native names. The 47-export smoke test passed. Defender definitions
`1.463.284.0` found no threats across all 22 files, with stable hashes and
zero matching detection events. The DLL is unsigned and the exclusion
query was denied. SHA256 is
`caaccaad05f650eaeb14267aa37c5c7f57acbd16672c45c727d45c9fa28933a4`.

FFV matched all 22 hashes and the installer verified 138 protected file
states across all six games. FFIV received the fix with the later combined
door and navigation update. Recovery starts from `993e06e`
for source and the previous per-game delivery records for installed hashes.
Receipts are in `../tools/boss_visuals_20261112/sprite-fix-delivery`.

## November 13 FFIV Castle Baron and hovercraft

The focused build took 67.7 seconds and passed the browse-route, vehicle
landing, entity scanner, fade readiness and equipment/party fixtures.
The smoke test passed all 47 exports and the native-name audit checked
8,445 names. The candidate is 13,453,312 bytes with SHA256
`820eae2cc028ea9aebf81d153bd9d4a787232d13831c4b9c59f3ad5d026b71f9`.
Defender definitions `1.463.281.0` reported no threats on the 22 exact
files. Hashes stayed stable and there were zero matching detection events.
The OS denied the exclusion query. The DLL is unsigned.

The FFIV Steam installation matched all 22 scanned files after the game
closed. The installer verified 138 protected file states across all six
games and verified the prior DLL hashes in the other five installations.
Delivery receipts and source locators are under
`../tools/ff4_underworld_20261112/doors-hovercraft-20261113` relative to
the checkout. Recovery uses prior source checkpoint
`311a9f5` and the verified previously installed DLL hash
`b99cc005befc5c759a66ff4d88f5a5cdf01866a0320d7626aaf474bf95823e11`.

## November 13 map close and field input

The focused build passed three context fixtures, 8,439 native-name checks
and the 47-export smoke test. The DLL is 13,439,488 bytes with SHA256
`b99cc005befc5c759a66ff4d88f5a5cdf01866a0320d7626aaf474bf95823e11`.
Defender definitions `1.463.281.0` reported no threats on the 22 exact files,
with zero matching events and stable hashes. The OS denied the exclusion
query. The DLL is unsigned.

All 22 installed hashes matched in each of the six games. The installer
verified Prism read-only and preserved 138 protected file states. FFIV
was installed after the process exited. Recovery starts from the prior source checkpoint
`87d6c46002361ff442001e83f8e0673e5bba68c1`. Per-game prior DLL hashes and
delivery receipts are in the external map-panels archive.

## November 12 FFIV–VI boss animation cues

The focused boss-visual, battle-entry and battle-feedback checks
passed in 46.0 seconds with the linked production DLL. The job
peaked at 303,001,600 bytes with one worker and the 25% CPU cap.
All 67 embedded resources matched their inputs and all 47 exports
resolved in the isolated smoke host. Defender found no threats
in the 22 exact payload files. The exclusion query was unavailable
and no signing certificate was present.

The FFIV, FFV and FFVI Steam installations each matched all 22
scanned files. All 69 protected file checks passed, and the FF1–III
DLL and SDL hashes matched their pre-installation values. No game
was launched. The installed SHA-256 is
`c9f1072d36f1f1c3b43c2b0f3833632c8bef3483c28eef68babf35f4d1d17acb`.
Concise receipts are in the workspace's
`tools/boss_visuals_20261112/delivery` directory.

## November 12 FFIV Dwarven Castle chests

The warm focused build and three route and map-state checks completed in
103.0 seconds. Peak job memory was 494,252,032 bytes with one worker and the
25% CPU cap. All 67 embedded resources matched their source bytes. The
isolated host resolved 47 exports and booted successfully. Defender found
no threats in the 22 exact payload files and their hashes remained stable.
The exclusion query was unavailable and no signing certificate was present.
The DLL SHA-256 is
`5fb04b743a0fc2f8e4817b3cfbfe979aa821f2797355271359f1c5a7229fcf98`.
After the user closed FFIV, the established installer applied the
DLL and verified all 22 scanned files and 138 protected file checks.
The later boss-cue build includes this route fix and supersedes this
DLL in FFIV. The separate route delivery receipt is in the workspace's
`tools/dwarven_castle_20261112` directory.

## November 12 FFIV Castle Baron gate

Production source 27572a4 plus the focused delivery entry e3b5082.
The user approved stopping the earlier full 281-target job. The focused
production DLL, smoke host and four named route/state/filter checks
completed in 595.7 seconds. The enforced job peaked at 926,883,840 bytes,
with 25% CPU and one worker. All 67 embedded resources matched their
source files and all 47 exports resolved in the isolated smoke host.

Defender found no threats in 22 exact staged files with stable hashes.
Exclusion status was unavailable. No signing certificate was present.
Guarded deploy.bat ff4 installed SHA-256 d33c195037659787dd9ed35641fb09869724590d4564a817f09813624f7a08ef.
All 22 installed files, 138 protected file checks and the other five
installations verified. Concise receipts are in the workspace's
tools/baron_gate_20261112 directory. Gameplay confirmation is pending.


## November 11 FFIV Tower of Zot chest

FFIV now uses the 13,390,336-byte DLL with SHA-256
`8e6cf822bdc5eaf2f73d8610fc2de4270f193eba552785ba7f7d902e84ed7c77`.
The route source is in checkpoint `1016b7d08d9c20e8a958215237926dd96b58d19e`.
The exact source fingerprints and receipts are in
`build_check/ff4_zot_chest_20261111`.

Reproduce with the enforced runner and `build.bat`, then
`smoketest.bat`. The corrected canonical build completed 281 serial
targets and 8,366 native-name checks, with zero missing or undefined wiring
names. The full Zot, Dream and shared route checks passed. It took
7,059.075 seconds, peaked at 794,390,528 bytes and exited 0 with the
process tree drained. The built DLL retained 67 byte-matched resources,
47 forwarded exports and the verified FFIV labels in all 12 locales.

The isolated smoke host loaded the same DLL. The signed Defender scanner
used definitions 1.463.246.0 and reported zero matching detections on all
22 exact payload files, with stable hashes. Its exclusion query was
unavailable and no signing certificate was present. The guarded
`deploy.bat ff4` installed the verified payload. All 22 files matched
afterward, and all 144 protected fingerprints and the other five
installations matched their preinstall checks.

Cleanup removed 35,548,493 bytes of temporary build groups, smoke and
scan staging, and the one-off verification helper. The installed hash
was checked again afterward. Keep the canonical incremental build and
the source, deployment and Git checkpoint receipts.

## November 11 FFIV Kain's Airship

The locally installed FFIV DLL is 13,389,312 bytes with SHA-256
`1bb6cf58617ea34f2cea242bdd5c5cae94ff13d5b5aba18a11d35f1cc523bbcc`.
All 281 canonical build targets compiled. The tool session ended after
the final NPC label check. The same frozen candidate passed the remaining
canonical name, wiring and soundscape asset checks in the enforced runner.
The PE check matched all 67 embedded resources and 47 forwarded exports.
Only the two FFIV label and language resources differ from the previous
verified installation. Both contacts and all 12 locales pass.

The canonical smoke test loaded that DLL. The signed Defender scanner
reported zero detections for the exact 22 staged files, with stable hashes.
The exclusion query was unavailable and no signing certificate was present.
The guarded `deploy.bat ff4` installed the exact scanned payload. All 22
installed files matched, and the protected files and five other installations
matched their preinstall fingerprints. Rebuild inputs, scan and installation
receipts are in `build_check/ff4_kain_airship_20261111`.

## November 10 FFIV navigation and battle

The local FFIV Steam installation has the 13,388,800-byte DLL with SHA-256
`863953f692f48fe4a2f2a03d45f612dd8e02ce3aa99016b1277f549712a01d7e`.
The canonical build completed 281 serial projects, 8,366 native-name
checks, and all offline checks. It had zero missing names and zero undefined
wiring names. The built PE matched 67 embedded resources and 47 forwarded
exports. The new guide and bearing strings cover all 12 locales.

The source base is `5f1cc7c3dbd2251e572c3dc0f42f557f45191d9d` plus the
preserved working-tree sources. The 1,301 build-input hashes are in the
delivery receipt. Visual Studio 18 Community, MSVC 19.50.35721.0 and CMake
4.1.1-msvc1 built x64 Release. Run `build.bat` and `smoketest.bat` under
`scripts/run-build.py`. The final build took 5,482.012 seconds and peaked at
792,875,008 bytes with exit 0 and the tree drained. Smoke loaded the same
DLL in 5.286 seconds and checked the exports, forwarding and startup log.
The first combined candidate was withheld when the raw travel audit failed.
The final suite retains 190 tagged scripted returns in the raw inventory
and passes across 2,454 master roots.

Fingerprint the established game roots, payload, preferences and waypoints.
Use `tools/security_payload_check.ps1 -BuiltPayload` with that preinstall
receipt. The signed Defender scanner checked all 22 staged files with
definitions 1.463.227.0, exit 0, zero matching detections and stable hashes.
The exclusion query was unavailable and no signing certificate was present.
After rechecking the exact source hashes, run the guarded `deploy.bat ff4`
under the same runner while FFIV is closed. The installed 22 files match
the scanned payload. All 144 protected files and the other five game
installations match their preinstall fingerprints. The canonical,
smoke, scan, deployment and recovery receipts are in
`build_check/ff4_navigation_20261110`.

Cleanup removed 57,129,667 bytes of scratch project groups, staging,
isolated smoke output and one-off verification helpers. The canonical
incremental build and active output are retained with the source and
verification receipts. The installed DLL hash was checked again afterward.

## November 10 FFV and shared controls

All six installations matched the 13,354,496-byte DLL at that deployment,
with SHA-256
`4b4d23779b2444a0819891e97b50f5f72ab1367d789ba3acd462131e5501530c`.
The canonical build completed 281 projects and 8,362 native-name checks with
zero missing and zero undefined wiring names. The built PE has all 47
forwarded exports and 67 byte-matched embedded resources. The three changed
audio and bearing entries cover 12 locales.

The source base is `5f1cc7c3dbd2251e572c3dc0f42f557f45191d9d` plus the active
working-tree inputs, fingerprinted in 1,307 entries. Visual Studio 18
Community, MSVC 19.50.35721.0 and CMake 4.1.1-msvc1 built x64 Release.
Run `build.bat` from the repository through the enforced runner, then
`smoketest.bat` in its own bounded job. The canonical job took 7,807.648 seconds,
peaked at 963,600,384 bytes and drained with exit 0 at the 2 GB and 25% CPU
limits. The isolated smoke job passed in 8.490 seconds.

Capture the established game roots and protected-file fingerprints, then
run `tools/security_payload_check.ps1` with BuiltPayload and the preinstall
receipt. The exact 22-file stage passed the signed Defender scanner using
definitions 1.463.227.0. It reported zero matching detections and stable
hashes. The system denied the exclusion query and exposed no signing
certificate. The guarded `deploy.bat` installed the scanned payload after
the closed-game and dependency checks. All 132 installed file hashes match
and 144 protected files retained their fingerprints.

The build inputs, limits, concise test results, resource and scan manifests,
and preinstall and installed receipts live in `build_check/bug_reports_20261110/`.
Use the verified v1.0 commit `2fcf0f6f9b6d890abaa5e4886836df6670ab19a5` and the
same build, smoke, scan and deployment procedure for release recovery.
The current local build also includes later working-tree changes. The shared
canonical build cache remains the reusable build tree. Temporary rollback
copies, scanned payload copies, isolated smoke copies, generated serial
project files and one-off verification tooling are disposable after the
installed hashes and protected-file checks pass.

November3UTC custom/shared candidate canonical/offline/smoke/scan Closed,
not installed:12146688B SHA300ff3dfbebb647ae814baa13a171545533a2ae915236b33b1daeada9cc9d484,
834 frozen inputs/205targets/242freshreports/67resources/12languages/8340names
0missing/0undefined/47exports. Canonical passed1272.039s. The first scan setup
lacked the required dependency snapshot, so no scan ran; failed evidence kept.
Read-only installed-dependency inventory plus exact22-file Defender scan/output
verification then passed38.070s/101339136B/exit0/drained under2GB/25%CPU. No threats,
0 matching detection events, stable bytes, valid/current enabled scanner;
exclusions unverified/signing0. No game/install/upload/security change.
Exact build_check/custom_sounds_20261102/canonical_completion.json and security_final.

November2 isolated FF1 Shrine/Ship/service/EXP delivery Closed21:57UTC:
DLL11625984B SHA b62ed342de0f689673ce7a5a629e588e994477fa1aacde48f13ca3529f180a96.
724 frozen inputs/169 compiler targets/178 fresh reports/47exports/67resources/
12languages/8326names0missing/0undefined. Canonical/smoke qualification passes;
Defender22 exact files no threats/0 matching events/stable hashes, valid scanner
signature, exclusions unverified/signing0.22 installed hashes match,144 protected
files and other five games unchanged. No game/probe/upload/security change.
Full resumed qualification710.831s/801751040B; install12.235s/115920896B,
both2GB/25%CPU/one worker/exit0/drained. Original2400s compiler timeout preserved.
Exact Stage ff1_sunken_shrine_20261102/completion.json; Main delivery receipt in
sunken_shrine_20261102/delivery_completion.json. Natural performance unclaimed.

November1 isolated FF1 class/gates/animated-object delivery:11502080-byte DLL,
SHA2562b7c5f50f0a1c30ba0367d36d67c0fcdeedc3eb33a553b4c8f6eb6d17e929284.
699 frozen inputs/165 canonical reports,47-export smoke,67 resources/12 locales
pass. Enabled/current Defender1.463.72.0 reports no threats/0 matching detections
and stable hashes for22 exact staged files. Scanner signature Valid; exclusions
OS-denied/unverified, signing identity0. Guarded FF1 installation matches all22
hashes;144 protected files and the other5 installations unchanged. No game
launch/probe, external upload, dependency/security change or public release.
Build4075.394s/930578432B peak and verification/deploy38.671s/124035072B, both
serial enforced1 GB/exit0/drained. Isolated source/evidence/recovery retained in
../FFPRCore-Class-Delivery/build_check/ff1_class_description_20261101/.
[Scope and qualified cases](re/FF1_GATE_FOLLOWUPS_20261101.md).

October29 final six-game map audit delivery:11374080 bytes, SHA256
cf805c2e8e05ff0235f51f5228a916bb23c80e1e6251e31c95bc86cf34a5e86b.
Canonical/offline/isolated smoke,67 resources and12 locales pass. Defender
reported no threats in22 exact staged files, stable hashes/0 matching detection
events. All132 installed payload hashes match;144 protected files unchanged.
Evidence/recovery: build_check/map_audit_final_20261029/. First full compile
21m42s; two legacy test assumptions corrected, production candidate unchanged;
final canonical2m37s pass. One compiler /m:1 /MP1 retained. Exclusion inspection
remains OS-denied/unknown, signing certificates0, no outer-package/reputation
claim. No game launch, protection change, prerequisite repair or upload.
[Current content cleanup and deferred historical-content issue](re/SOURCE_CONTENT_REVIEW_20261029.md):
200 raw files preserved externally;30 source-free tracked references, old bodies
still in existing history. No destructive history action was performed.

## Earlier security receipts

October29 Invincible FFIII delivery:11327488 bytes, SHA256
ad0d80c154698e08a954ac820341c8e522211c0f5c538ad2ece19e8bdf0d0555.
Serial build/smoke,67 exact resources and12 locales pass. Defender reports no
threats for22 staged final files; hashes unchanged,0 matching detections.
All22 installed hashes match,144 protected files/other5 games unchanged.
Evidence/recovery: build_check/invincible_20261029/. Exclusion status remains
OS-denied/unknown and no signing certificate is available. No outer package,
external upload, dependency installation, security change, game launch or probe.

October28 equipment/vehicle FFIII delivery:11311104 bytes, SHA256
d3053479822a982e454e10e243bfb665a96d87dcbe6d4c545d2f8bd78b686200.
Full serial build and smoke pass; Defender reports no threats for22 exact staged
files, hashes unchanged/0 matching detections. Installed22 hashes match;
144 protected files and other5 installations unchanged. Evidence/recovery:
build_check/item_buffer_pages_20261028/. Authorized local scan used protected
status access; exclusion status remains OS-denied/unknown, signing identity0.
No security setting, dependency, outer package, upload, game launch or probe.
Deployment's process guard passed without an additional close request.

October28 Boost/Guard/category FFIII delivery:11107328 bytes, SHA256
9146ae033c305a4564f9f26bc5df2b003d210ad1c2f0bc4b2823ca2ed9f7ba6a.
Full serial/offline/smoke/resource/localization checks pass; Defender reports
no threats for22 exact staged files with unchanged hashes/no matching detections.
Evidence/recovery: build_check/boost_use_20261028/. The first restricted status
query was denied; its partial evidence is retained in security_predeploy_restricted.
The normal authorized scan then completed with protected status access. Exclusion
visibility remains OS-denied/unknown; no signing identity, outer-package or
reputation certification. No security setting, dependency, upload or game launch
changed. Installation followed human save/close and verified process closure;
all22 installed payload hashes match.144 protected files and the other five
game payloads are unchanged. Source/delivery Closed; gameplay/audio unconfirmed.

October28 Nautilus/startup FFIII delivery:11103232 bytes, SHA256
b2cacd6fab1b71d4d7cab5ff52567c151d8c0e24b2e2ea1d549a49e71b1a257b.
Full serial build/offline checks, isolated47-export smoke and67-resource/locales
verification pass. Defender reports no threats for22 exact staged files; hashes
remain stable. All22 installed FFIII hashes match;144 protected files and the
other five game payloads are unchanged. Evidence/recovery:
build_check/nautilus_access_20261028/. No signing identity, outer-package or
reputation certification; protected-exclusion inspection remains unavailable.
No protection setting, dependency prerequisite, game launch/control, upload or
publication changed. [Scope and native limits](re/NAUTILUS_ACCESS_20261028.md).

October28 cross-floor NPC candidate:11098112 bytes, SHA256
c23a8fa9bca219e5c41b104eb6a9984822d661a67391c2ab775a7c7e63a78b59.
Full canonical/smoke/resources/locales pass. Fresh Defender scan reports no
threats for22 exact staged files; stable hashes/no matching detection events.
Evidence/recovery:build_check/cross_floor_npc_20261028/. Exclusions remain
OS-denied/unknown; no signing identity, outer package/reputation check, upload
or game probe. Installation waits for game closure; the running-game guard
blocked the first copy without installing the update.

October28 Boost sentence update supersedes the preceding FFIII payload:
11097600 bytes, SHA256a7a97a14d23101e044fbb1825c9222741e33ad873a0464924aa103981c50a6b1.
Full build/smoke/resources/12 locales pass; Defender reports no threats for22
exact staged files, unchanged hashes and no matching detections.22 installed
FFIII hashes match;144 protected unchanged, other five payloads unchanged.
Evidence/recovery:build_check/boost_wording_20261028/. Exclusion visibility
OS-denied/unknown; no signing certificate, outer package/reputation test, upload
or game probe. The actual action/result producer is unchanged.

Latest October28 FFIII delivery:11096576-byte DLL, SHA256
9aff428068b6a6f37d795f91a73f67dfc74ed5db7a989d6cdc6c27c46e16aa62.
Full canonical/smoke/resources/locale checks pass. Fresh Defender scan reports
no threats for22 exact staged files; hashes unchanged, no matching detections.
All22 FFIII installed payload hashes match the scanned manifest;144 protected
files unchanged and other five games' payloads unchanged. Recovery, scanner/
definition/time records, signatures and coverage limits are preserved under
build_check/map_event_audit_20261028/. Exclusions remain OS-denied/unknown,
no signing certificate; no outer package/reputation test or external upload.
No game launch/control/probe. Earlier delivery records below remain historical.

October26 navigation maintenance: final DLL11032064 bytes, SHA256
73115338cf025dc45b46673fbaabcf69dca62730756c623fcd42530359154013. Canonical8287/0 names/142/349/0 wiring,
47-export smoke and67 resources pass. Exact22-file built payload scan with enabled,
current Defender1.461.216.0 reports no threats/zero matching detections/stable hashes.
Installed132 hashes match all6 games;144 protected settings/dependency files
unchanged. Evidence/recovery:build_check/ff3_navigation_20261026/. Exclusion query
remains denied/unverified, signing certificates0; DLL unsigned. No outer archive,
download reputation or other scanner result is claimed. No game launch/probe,
security/dependency installation or public-release replacement. Detailed source
scope and remaining native/gameplay timing limits are linked from memory.

Release1.0 candidate, October26: x64 DLL10993152 SHA256
35a59650e5d86cfded076263d5f2b58c7352eaf218ca3a861ee83af1f03565e4;
6199100-byte ZIP SHA256
a3b3c385bb446f0746cdcd14f9ee0ddb8e037604bc950b75f042f98eb795419f.
Full canonical checks/smoke, exact28-file package/CRCs/manifest/locked runtimes/
notices and PE1.0.0.0/ASLR/NX/high-entropy VA pass. Exact29-file Defender scan
includes the final outer ZIP and all contained payload: no threats/zero matching
detections/stable hashes, enabled/current definitions1.461.211.0. Evidence:
build_check/v10_20261026/security_release/ and package_verification.json.
No signing identity available; DLLs unsigned, exclusions unverified after denied
query. Download reputation/other scanner acceptance remain untested. No protection
or dependency change. User-authorized [public V1.0](https://github.com/bladestorm360/FFPRCore-Screen-Reader/releases/tag/v1.0)
is published and Latest, with draft and anonymous public downloaded bytes matching
the scanned archive. Six prior releases preserved; no game launch/probe.
Microsoft [signing guidance](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/code-signing-options)
and [MpCmdRun guidance](https://learn.microsoft.com/en-us/defender-endpoint/command-line-arguments-microsoft-defender-antivirus)
rechecked October26. No paid signing service/tool install is introduced.

October26 [beacon movement/spatial payload](re/BEACON_MOVEMENT_20261026.md):
DLL10993152 SHA256
ea355feca25bf1c7bad59630a2c994ec0fb711a1c3cdd8d75fab111742c64079.
Full checks/47-export smoke pass; exact22-file enabled/current Defender scan
reports no threats/zero matching detections/unchanged hashes. After the user
closed III, all six verify132 stage hashes and144 protected entries unchanged.
Evidence/recovery:build_check/beacon_movement_20261026/. Exclusions unverified,
certificates0; package/reputation/other scanner acceptance untested. No protection,
dependency, upload, release or game launch/control change.

October26 [locked-door approach payload](re/LOCKED_DOORS_20261026.md): DLL10983424,
SHA256574da7fc17ed8c8a0b2dcc7697566b0833370263f8b31f1bf61cd322da77aa2b.
Full canonical checks,47-export smoke and49data/18sound resources pass. Enabled,
current Defender1.461.211.0 reports no threats/zero matching detections on22 exact
staged files; hashes unchanged. All132 installed hashes match/144 protected files
unchanged after guarded deployment. Includes Warehouse/remote-return and prior
fixes below. Evidence/recovery:build_check/locked_doors_20261026/. Exclusion query
denied/unverified, signing certificates0; outer package/download reputation and
other scanner acceptance untested. No protection/dependency change, external
upload, release or game launch/control/probe.

October26 [Warehouse/remote-return combined payload](re/WAREHOUSE_20261026.md):
DLL10983424 SHA256
bc93b87173047349cf8d6e74ca3a7b1eb654445c72d5812f7fbafa740db3b8e4.
Canonical8275/0 names,141/348/0 wiring,47-export smoke and49data/18sound resource
checks pass. Exact22-file staged Defender scan reports no threats/zero matching
detections/hashes unchanged. All132 installed stage hashes match/144 protected
files unchanged after the user closed FFIII. Evidence/recovery and exact scanner
readiness/signatures:build_check/remote_returns_20261026/. No signing certificate;
protected exclusion queries and outer-package/reputation tests remain explicit
limits. No protection/dependency change, external upload, release or game probe.

October26 [combined ally status/world-event payload](re/ALLY_STATUS_20261026.md):
canonical full checks/47-export smoke pass;49 data/18 sound resources match source.
DLL10959872 bytes, SHA256
b88ebc08a531a1bb841aa99942da0ba54d85750fb0c49a5fbf7d8bfcd669cf74.
Enabled/current Defender1.461.211.0 reports no threats, exit0/no matching detection
events/all22 staged hashes unchanged. Installed all six after the user's close
confirmation;132 stage-to-installed hashes match/144 protected files unchanged.
Recovery, exact scan and final receipts:build_check/ally_status_20261026/.
Existing unsigned/no signing certificate, protected exclusion-query denial and
outer-package/download-reputation limits remain; no settings/protection/dependency
change, external upload, publication, release or game control/probe.

October26 [shop Content-type correction](re/SHOP_COMPARISON_20261026.md):
full build/checks/47-export smoke and49data/18sound resource checks pass. Exact
22-file predeploy stage scanned with enabled/current Defender1.461.211.0,
exit0/no threats/no matching detections/unchanged hashes. DLL10950144 bytes,
SHA2561b5f454bc8d4f3e0c7e9f482c5a634f9ee855c8336fb2e8d1d349299dad637b7.
Five installed games I/II/IV/V/VI match all110 staged payload hashes; running FFIII
was refused by the deployment guard and retains its prior DLL.144 protected files
unchanged against deploy_finalbefore.json. Evidence/recovery/final scan/partial
receipt:build_check/shop_comparison_20261026/. Existing signing, protected
exclusion-query and outer-package/reputation limits remain. No protection change,
external upload, dependency update or game launch/control/probe.

October25 [battle title/party shortcut repair](re/BATTLE_TITLE_PARTY_20261025.md):
10949632-byte DLL SHA256
ca11df243fda6fb1b90ae5405974080e6ed7facfcc7cf0783f7d4d9495ed56bd.
Canonical checks/47-export smoke/49data+18sounds match current sources. Enabled
Defender:22 exact staged files, no threats/zero matching detections/stable hashes.
Guarded all-six delivery:132 installed hashes match/144 protected files unchanged.
Evidence/recovery:build_check/battle_title_party_20261025/verification.json,
security_final_predeploy/, deploy_after.json, installed_payloads_verified.json.
No game launch/probe, dependency/protection change, upload or release. Signing0
and exclusion-query verification false remain limits; no universal scanner claim.

October25 [chocobo/Owen final build](re/CHOCOBO_OWEN_20261025.md):10948608-byte
DLL SHA256807984be33160338bef7f87e4a5fdf5fe2d818dc5a121f250208157b5fc733ad.
Full canonical checks/47-export smoke/49data+18sound resource checks pass.
Enabled Defender found no threats on22 exact staged files, zero matching
detections and stable hashes. All132 installed payload hashes match;144 protected
files retain bytes/timestamps. Final evidence:build_check/chocobo_owen_20261025/
security_final_predeploy2/ and final deploy_after/installed_payloads_verified
receipts. Prior candidate scan/install remains historical. No launch/probe,
dependency upgrade, security change, upload or release. Signing0/exclusion-query
limits remain explicit; no universal scanner/reputation claim is made.

October25 [terminal-transition combined build](re/TRANSITION_ROUTES_20261025.md):
10948096-byte DLL SHA256
0a58ff0667c5f5bcd4ec0674e7026423fa4c0d87317f430fefe8b71241905265.
Full canonical checks/47-export smoke pass;49data+18sounds match. Enabled Defender
found no threats on22 exact staged files, zero matching detections, stable hashes.
All132 installed hashes match and144 protected files retain bytes/timestamps.
III was closed when deployment ran; normal guard preserved. No dependency upgrade,
game launch/probe, security change, upload or release. Signing certificates0 and
exclusion enumeration unverified remain explicit limits. Evidence/recovery and
final receipts:build_check/transition_routes_20261025/.

October25 [patrol/companion/field-menu follow-up](re/NPC_PATROL_TARGETS_20261025.md):
full canonical checks/47-export smoke pass;49data+18sound resources match source.
DLL10944000 bytes SHA256
37ab60d98b07119717a883e49fd540a22d7a5b2a4d1e8511721897f34cb9e09a.
Exact22-file enabled/current Defender scan: no threats/zero detections/stable
hashes. I/II/IV/V/VI installed/hash-verified (110 payload hashes,120 protected
files unchanged); III remains deferred by running-process protection. Evidence:
build_check/npc_patrol_companion_20261025/. No protection/dependency/upload/game
control change; signing certificate unavailable, exclusion enumeration unverified.

October25 [connected floors/treasure/map/shop follow-up](re/NPC_FLOORS_SHOP_LANDING_20261025.md):
canonical build/all offline checks/47-export smoke pass;49 data and18 sounds match
source. Exact22-file Defender scan reports no threats/zero detections/stable
hashes. DLL10932224 bytes SHA256
a9f44da03d374418d02e031f17256ee0f34190915c0d33d4c8516a6e42aaefee.
After the user closed III, all132 installed hashes match;144 protected files are
unchanged. Receipts/recovery/source hashes/native provenance and final scan:
build_check/npc_floors_shop_landing_20261025/. Prior partial III deferral retained.
No game launch/control/probe, dependency upgrade, upload or security-setting change.
No signing certificate; exclusions remain unverified and reputation untested.

October25 [startup diagnostics and silent landing initialization](re/STARTUP_HANG_20261025.md):
full checks/47-export smoke pass. Exact22-file Defender stage/no threats/stable
hashes. DLL10918912 bytes SHA256
07d54d2c8069fa309cd739e8c558e95d47ccbac1949c047fd3a4121fc407ece3.
All132 installed payload hashes match;144 protected files unchanged. Evidence:
build_check/startup_hang_20261025/. The user-approved read-only FFIII snapshot
is outside Git; no game launch/input, dependency install, upload or protection
change. Existing signing/exclusion/reputation limits remain. Startup cause is
unresolved despite the subsequent successful user run.

October25 [floor labels](re/MAP_FLOORS_20261025.md) and
[battle hands/controls](re/BATTLE_GEAR_CONTROLS_20261025.md): full build/checks and
47-export smoke pass. Final DLL10902016 bytes SHA256
8e47ebf353835ccb715241973848462bc2e06f885ae099b73debf516d32a1155
matches the exact22-file Defender stage (enabled/current definitions1.461.186.0,
exit0/no threats/no matching detection events/hashes stable). All six games match
every staged file, including FFII after the user's deploy reply.144 protected
files unchanged against deploy_preff2.json. Evidence/recovery/final receipts:
build_check/map_floor_20261025/. Four unsigned DLLs/no signing certificate,
protected exclusion-query/unverified coverage and outer package/reputation limits
remain. No protections changed, upload, game probe or dependency update.

October24 [Object vehicle routes](re/OBJECT_VEHICLE_SHORTCUTS_20261024.md) and
[NPC name masks](re/NPC_SPOILERS_20261024.md): full checks/smoke pass, exact22-file
Defender stage/no threats/hashes unchanged; every installed file matches in all
six games.144 protected files unchanged. DLL10874880 bytes SHA256
 d75864ec918d7eaeb0300d5a342ff3e9a520983b3aadd27f8554d7117535d627.
Evidence: build_check/emperor_spoiler_20261024/security_final_predeploy/ and
installed_payloads_verified.json. Prior signing/exclusion limits remain; no
security changes, upload, dependency upgrade or game probe.


October24 [combined maintenance delivery](re/COMBINED_MAINTENANCE_20261024.md):
exact22-file predeploy stage scanned with current enabled Defender1.461.181.0,
exit0/no matching detections/unchanged hashes. DLL10871808 bytes SHA256
cac7266d799931e353f9a445498ee57abc4e417b5a145ee582f74804a533870b.
All22 installed files match in all six games;144 protected files unchanged against
the fresh post-close baseline. FFII was deployed after the user closed it.
Receipts: build_check/stat_effects_20261024/deploy_after.json and
installed_payloads_verified.json.
Four unsigned DLLs/no signing certificate and exclusion-query0x80070005 limits
remain; no security changes, upload or outer archive/reputation test.


October24 [Roundworm/H delivery](re/ROUNDWORM_STATUS_20261024.md): exact22-file
stage scanned before deployment with enabled/current Defender1.461.181.0,
scan exit0, no matching detection events and unchanged hashes. DLL10805760 bytes,
SHA2560c5721b60b27935db3a5912ac68c54302f9c1c421b00a6196f54b83db822fe9f.
Receipts: build_check/roundworm_room_20261024/security_predeploy/.
Five games updated; FFII process guard deferred installation until closing.
Installed manifests/protected-file verification are recorded separately.
Existing signing/exclusion limits below remain; no protection changes or upload.

October24 combined [Leviathan/MP delivery](re/LEVIATHAN_20261024.md):
final22-file stage scanned before installation with enabled/current Defender,
no threats or matching detection events, unchanged hashes. All six installed
payloads match that stage;144 protected files unchanged. DLL SHA256
c3a7160030c86c010aad513c139f3a8b9b78d01a3a68c350724657773b3aede7.
Receipts: build_check/leviathan_20261024/security_predeploy/ and deploy_after.json.
Existing signing/exclusion-query limits below remain; no protection changes,
external upload or outer package/reputation test.


October24 [three-part MP correction](re/PARASITE_THREE_PARTS_20261024.md):
the final phase-specific DLL/dependencies were scanned before installation;
security_final_predeploy under build_check/parasite_three_parts_20261024/ binds
the22-file stage to DLL73c8332982e078689efd6e6d0c14452503cfdad00948cc1b14305171e646fafc.
Enabled/current Defender1.461.181.0 reports no threats, unchanged bytes and no
matching detection events. security_predeploy in that task is an earlier
unshipped candidate, not the final payload. Existing signing/exclusion limits
remain. No protection changes or external upload.

October24 subsequent [equipment/MP delivery](re/BATTLE_ITEM_EQUIPMENT_20261024.md)
was scanned before installation using security_payload_check.ps1 -BuiltPayload.
This mode takes the fresh canonical out/hid.dll plus verified identical installed
dependencies, using deploy_before.json; it does not claim all games are already
updated.22-file stage, current definitions1.461.181.0, no threats, unchanged bytes
and no matching detection events. Final deployed hashes match the scanned stage.
Evidence: build_check/battle_item_equipment_20261024/security_predeploy/ and
deploy_after.json. Existing unsigned/no certificate and protected exclusion-query
limits below remain; no new external upload or security-setting change.

October24 user requirement applies to subsequent development and deliveries.
The current follow-up was already installed when the requirement arrived; its
exact final installed payload was then checked before this handoff. This record
does not certify previous releases or universal scanner acceptance.

## Loading, privileges and build inputs

The mod is an ordinary native x64 DLL using the game's established hid.dll
proxy-loading route. All47 system HID exports forward unchanged. MinHook hooks
the verified local game's methods for accessibility. Company/file description
identify the FFPR Screen Reader Project and accessibility mod; required proxy
filename does not claim Microsoft or Square Enix authorship. No executable
packer, obfuscated downloader or additional launcher is introduced.

build.bat uses the installed Visual Studio18/CMake toolchain, compiles mod-owned
source, embeds checked data/sounds and runs the offline suite. deploy.bat is
separate, refuses running games and writes only approved mod/runtime payloads.
Preference/dependency receipts verify preservation. Deployment never launches
games, installs/repairs Prism, changes security settings or requests Windows UAC.
Existing filesystem access remains subject to the host's permissions.

SDL3 has its established runtime hash. OpenAL Soft's eight-file pinned payload
and Prism0.18.3 upstream release/commit/hash are verified by the existing
dependency tools and prism.lock.json. Native game source remains outside Git.
The PE evidence records x64, ASLR, high-entropy VA, DEP/NX and a security cookie.
Control Flow Guard is not enabled in this established build; no hardening was
weakened for this task, and no new CFG compatibility claim is made.

## Exact local delivery check, October24

Final DLL10793984 bytes, SHA256
3c8562206a2fe22ca9540a4349cff82ec04c68ee645e34db8f7e5f8df73b5b82.
All six installed copies match. The local scan staging manifest contains22
exact installed files: hid.dll, SDL3.dll, private OpenAL Soft and Prism runtime/
license payload. Each staged hash matches every installed game's corresponding
file. Game executables, game code, saves, settings and waypoints were excluded.

Windows Defender is the registered antivirus, with antivirus and real-time
protection enabled. Engine1.1.26090.9/product4.18.26090.9 used signatures1.461.179.0,
updated October23 at19:46 Pacific; reported age0 and not out of date. Microsoft's
installed MpCmdRun utility has a valid Microsoft Windows signature.

Its custom directory scan returned0 and explicitly reported no threats. All22
staged hashes were unchanged afterward. Matching detection/remediation events
were not found. Scan times, version/definition state, output/events, manifest,
signatures and PE hardening are retained in ignored
build_check/mysidia_objects_20261024/security_verified/. The initial exclusion
preflight and first successful scan are separately preserved in security/.

Coverage limit: Windows permits this user's custom scan but requires admin rights
to inspect exclusions. The exclusion query returned0x80070005; the exclusion list
is unverified. This is a permissions limitation, not a malware detection. No
protection/exclusion setting was changed and remediation was not disabled.
The record says Defender reported no threats; it does not claim full visibility
into administrator policy or acceptance by other products.

## Signatures and remaining distribution gaps

The mod DLL, SDL3, OpenAL Soft and Prism report NotSigned. Third-party bytes and
provenance were preserved; no dependency signature was stripped or replaced.
CurrentUser/My and LocalMachine/My contain no available code-signing certificate.
No trusted signature/timestamp can be produced with the available identity.
Do not substitute a self-signed public-trust claim or obtain signing credentials/
services/tools without authorization. Unsigned status is distinct from a malware
detection or downloaded-file reputation verdict.

No installer/archive was created in this follow-up. Outer-package scanning,
download-origin/SmartScreen reputation and installation/update/uninstall under
downloaded-file trust prompts remain untested. No private binary/source was
uploaded to a public scanner or vendor, and no game was launched or probed.

## Maintained workflow and safe extension

October28 per-target Draw/Boost candidate:11095040 bytes, SHA256
3e3be68a07fe9b2cb6ffa506724c8f2f0582c6b06134ef2fda62422457280d72.
Fresh Defender scan reports no threats across22 exact staged payload files;
all hashes remain unchanged, with no matching detection/remediation events.
Evidence:build_check/ability_targets_20261028/security_predeploy/ contains the
manifest, scanner/version/definition/time records, signatures and coverage limits.
Exclusion query remains OS-denied/unknown; no signing certificate, outer package,
reputation test or external upload. After player closure, FFIII-only installation
passes22 scanned installed hashes,144 protected unchanged and other five games'
payloads unchanged. The running-game guard blocked the first copy. No game
launched/probed. Receipts/recovery are adjacent to the scan evidence.

tools/security_payload_check.ps1 accepts a fresh EvidenceDirectory under the
workspace, adjacent to the verified deployment receipt. It stages only intended
payload files, compares all-six hashes, records Authenticode/certificate capability
and Defender readiness, scans locally, records coverage limits, and verifies bytes
after scanning. It neither signs nor changes protection. Preserve every prior
evidence folder and investigate errors/detections before interpreting results.

For future releases, validate source/dependency provenance and hardening; use the
established trusted signing identity if one becomes available; verify signatures
and timestamps, then scan the exact final contained payload and outer package.
Any changed artifact needs new hashes and applicable checks. Record scanner/
definitions/time/results and test installation with protections enabled within
existing permissions. The current extracted-file scan does not cover a future ZIP.

Retain any detection's exact name/hash and investigate code, dependency and
installer behavior. Do not rename/repack/rebuild merely to change a verdict,
disable protections, add blanket exclusions or silently submit samples. Vendor
false-positive submission and artifact uploads require existing authorization.
An unresolved detection is distinct from the unsigned/reputation gaps above.

## November 12 FFIV Underworld routing

Focused build, three navigation fixtures, 8,427 native-name checks and the
47-export smoke test passed. The installed FFIV `hid.dll` is 13,438,976 bytes
with SHA256 `6db637319016d3dc187e8725e717664b130f1d6ad6f2cad76546f4f187fe076c`.
Defender definitions `1.463.267.0` reported no threats on the 22 exact files,
with zero matching detection events and stable hashes. The OS denied the
exclusion query. The build has no signing certificate or outer package.
All 22 FFIV installed hashes matched. The installer preserved 138 protected
file states and the other five game payloads. It verified Prism read-only.

Evidence is in `../tools/ff4_underworld_20261112/delivery` relative to the
checkout. Recovery uses source checkpoint `cc1ce8d0cb76ab48738199a53203e95879521277`
with the documented focused build, smoke, scan and FFIV deploy sequence.
Its prior installed DLL hash is
`c9f1072d36f1f1c3b43c2b0f3833632c8bef3483c28eef68babf35f4d1d17acb`.
