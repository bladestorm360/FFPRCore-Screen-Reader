# Nearby walking routes and repeated interactive objects

Human scope October28: include these two small corrections with the urgent
[cross-floor NPC repair](CROSS_FLOOR_NPC_20261028.md), deploy FFIII, then resume
the [full map audit](MAP_AUDIT_FOLLOWUPS_20261028.md). Sol only; no game probe.
Initial estimate20–35minutes exceeded while correcting fixture assumptions and
rebuilding the shared offline programs; overrun reported. Source/delivery Closed:
full checks/smoke/scan pass and FFIII installed. Existing Closed
vehicle/label designs remain intact apart from these explicit follow-ups.

## Nearby walking policy

`field/baked_nav.cpp::may_shorten_with_vehicle` now declines optional vehicle
comparisons when the successful ground route is under15 tiles, or a legal final
approach is under15 wrapped Manhattan tiles away. A ground route must already
have succeeded. The proximity clause matters for the actual Saronia coordinates:
from83,62 to91,65/74,64 the legal terrain routes are17/19 tiles, despite both
entrances being11 tiles away geometrically. A strict actual-route-length-only
guard would leave the user's exact examples unchanged. Required crossings retain
the existing no-ground-route fallback; an already-boarded vehicle keeps its own
movement rules. Explicit vehicle targets keep their own boarding contract.

`find_path` also revalidates a retained optional flight near its target against
the current legal ground route. This uses the existing cancellable owned-data
search, with current gates/NPCs/terminal transitions. No geometric shortcut can
declare a blocked walking path open. Vehicle identity changes already invalidate
retained plans. No new native getter, hook, capture or game-thread search.
The rare retained-flight check may add one worker search; total native/frame/
audible cost remains unmeasured. Initial fixture assumptions omitted canoe
ownership and incorrectly required a separate landing phase from the existing
boarded-aircraft path; failed reports are preserved as test corrections.

`tools/navigation_followup_check.cpp::check_saronia_short_walks` uses actual FFIII
bakes and production full/browse planning. It checks both reported northern
entrances, walking despite an adjacent aircraft, retirement of a retained flight,
and changed guidance after boarding. Existing all-six Ship/shore and FFII
Palamecia fixtures preserve required-vehicle, missing-model and longer-shortcut
behavior. No game execution is involved.

## Repeated interactive objects

`entity_scanner.cpp` captures each interactive object's original translated
label, rather than its previously numbered output. `entity_filter.cpp::build_impl`
feeds exact duplicate labels to the existing `npc_names::renumber` service in a
separate object group. Books become Book1 through Book9; a unique Switch stays
Switch. NPCs and objects cannot consume each other's suffixes. Each map numbers
independently; source geometry provides stable order before category/distance/
reachability pruning. Native identity, route target and selection stay unchanged.
Numbers append to translated labels; no untranslated phrase or new setting.
Existing NPC developer-ordinal cleanup is unchanged; meaningful digits in an
object's label are not stripped. No new native read or callback. The existing
worker adds a bounded grouping/sort over the captured object rows.

`tools/entity_filter_check.cpp` checks nine Books, a unique Switch, two Books on
another floor, a same-named NPC, pruning, distance resorting and repeated builds.
Gameplay confirmation and actual native/audio timing remain separate.

Delivery: FFIII only,11099136 bytes, SHA256
de3e075d6bf3a089f2a8fde529ccbb8fc75c67f68c1a31a85ed1374eb0f211c9.
22 scanned installed hashes match;144 protected files and other five games'
payloads unchanged. Full build8318/0 native names,142/349/0 wiring, smoke47
exports,67 exact embedded resources,136 registrations,12 languages/six unchanged
bakes. Defender no threats/matching detections, unchanged staged hashes;
exclusions OS-denied/unknown, no signing identity. No game launched/probed.
Recovery, failed/accepted fixtures and receipts:build_check/short_walk_20261028/.

## Build memory preference

Human reported severe machine-wide lag during builds. The former MSBuild /m:4
and compiler /MP4 permitted multiplied compiler processes. `build.bat` now uses
/m:1 and CMAKE_BUILD_PARALLEL_LEVEL=1; CMake uses /MP1. Future targeted commands
must also use /m:1. This changes build scheduling only, preserving optimization,
checks and output semantics. The current final candidate was already built and
passed before this configuration change; the next necessary build will use the
serial policy. No claim of measured RAM reduction yet. Do not run parallel
build commands or a second compiler while this project's build is active.
