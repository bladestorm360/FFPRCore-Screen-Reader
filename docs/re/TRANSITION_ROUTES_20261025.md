# Terminal transitions in walking and vehicle routes

Status: complete, offline verified and installed/hash-verified in all six games.
FFIII had closed by deployment; the normal process guard allowed replacement.
Listening/gameplay confirmation remains pending. No game launch, control or
runtime probe was performed. No delegation, dependency upgrade or release change.

## Report and decisive evidence

The user's current FFIII log records origin (61,84), Caanan goal (87,65), and
4 North, board the Enterprise North. The exact real-bake worker fixture reproduces
45 steps/three phases, crossing the native GotoMap at (61,83), destination55,
PointIn1 (Tokkul). Moving sideways happened to avoid the entrance. This was a
search-model defect, not a bad player coordinate or a missing vehicle.

The baseline failure and own session copy are preserved in
build_check/transition_routes_20261025/. The fixture uses owned snapshots and the
actual production planner, terrain, map initializer projection and transportation
master. It does not execute game functions or infer a route from speech alone.

## Design and implementation

Native GotoMap entry and step-on script warps terminate the current map leg.
Ignoring story-event warnings must never permit using either as an intermediate
walking tile. Selected transitions and graph-planned floor changes remain legal
endpoints. A player already standing on an arrival cell can leave it normally.
Native world-town doors do not activate while flying/sailing over them; transport
terrain and native transition-entry restrictions remain authoritative.

- field/baked_map.h/.cpp prepare per-layer byte masks from existing routing
  objects. Native doors and step-on script warps have separate bits; manual
  examined warps and ordinary ToLayer stairs retain their established semantics.
- map_state.h and map_state_program.cpp project the active/effective, unconsumed
  transition objects into an immutable mask once per map in an owned Resolver.
  Unknown state stays conservative. Inactive/moved variants use the same state
  facts as the existing routing graph and obstacles, instead of blocking every
  historical placement. The binary bake format and game assets are unchanged.
- baked_router.h/.cpp Request shares that mask, falling back to the immutable
  prepared bake for standalone/offline callers. terminal_transition is a constant
  indexed lookup. edge_allowed prevents entry except for an actual open goal;
  scene-hazard fallback cannot override this. Flood goals stop expansion and
  explicit reverse seeds may leave their origin. Safe landing excludes transitions.
- baked_nav.cpp supplies projected masks to full/graph/quick worker requests,
  walking/boarding/riding legs and retained-plan validation. A boarding side is
  ordinary terrain, never a substituted town goal. Landing candidates exclude
  entrances before the final walking leg. Interaction approaches also exclude
  cells which would immediately leave the map.

Loaded-floor step warps use the already-captured powered native triggers, with
current one-cell positions and existing consumed-event guards. Remote floors use
the resolver's conservative projection. A per-worker-scope immutable mask is shared
by requests; a new scope retires it before changed captured observations are used.
This preserves the preexisting loaded-versus-remote authority distinction. The
Narshe intro regression caught future guard-guidance warps being conservatively
added to the loaded intro floor; those cannot become current obstacles while
absent from native capture. No additional capture or native enumeration is added.

No new controls, settings, localization strings, native hooks, waits, polling,
speech deduplication or worker access to native pointers are introduced. Existing
large planner/router files retain their cohesive shared search/vehicle boundaries;
this task does not split or redesign the navigation service.

## Ownership, bounds and validation limits

Masks are constructed only on the existing Data/Navigation owners from copied
state and immutable bakes. A resolver retains at most one mask per visited baked
submap; each present layer has width times height bytes. Requests/phases share
immutable storage instead of copying dense masks or scanning object lists on
every edge. Existing map/node limits, generation cancellation and result ownership
remain in force. No game-thread capture or callback work is added.

The current new test covers7422 native doors and2888 step warps across all six
games, including exact selected goals, ordinary boarding-side exclusions, mounted
flight, projected state, shared mask identity and the reported route's two sideways
origins. It also checks the selected Enterprise itself, a stale retained unsafe
route, selecting Tokkul and leaving an already-occupied arrival tile. The original
directed-transition receipt/repeat tests remain separate and unchanged. One trap
fixture now supplies its explicit selected warp goal for its entry-edge probe;
the production same-map fall/recovery/repeat assertions remain intact.

## Final verification and delivery

The exact route now takes47 steps/three phases: 1 West,4 North,1 East before
boarding North. It crosses no Tokkul entrance tile. Caanan and the selected
Enterprise both pass; the selected Tokkul goal remains exactly one step North.
All new all-six cases pass, including a previously retained invalid route and
the two sideways starts. Existing trap, cross-floor/repeat, story-gate, NPC,
vehicle and cancellation checks pass. Narshe retains Story gated with/without
the nearby scene; its absent later guard warps never become current obstacles.

Canonical build passes all checks:7994 native names/0 missing and137 used/344
defined/0 undefined wiring names. Smoke resolves/forwards all47 HID exports and
exits normally. All49 data and18 sound resources match current sources. The
retained worker regression checks98 boarded world/transport cases and582 exit
candidates, with no native access from workers. Its observed maximum route/filter
compute is40.371/30.222ms; six-category preparation maximum3.909ms. These offline
observations are not a comparable performance baseline or native-frame/audible
latency certification. Existing end-to-end diagnostics and cancellation remain.

Final DLL10948096 bytes, SHA256
0a58ff0667c5f5bcd4ec0674e7026423fa4c0d87317f430fefe8b71241905265.
Enabled Defender reports no threats on the exact22-file payload, zero matching
detections, unchanged staged hashes. Signing capability remains0 certificates;
exclusion enumeration remains unverified. No security setting or upload changed.
All132 installed payload hashes match the scanned bytes; all144 protected files
retain bytes/timestamps. Source hashes match the verified build. Existing Prism,
OpenAL, preferences, saves and waypoints were preserved.

Build/smoke/resource receipts, exact scan, recovery copies, deploy_before.json,
deploy_finalbefore.json, deploy_after.json and installed_payloads_verified.json
are in build_check/transition_routes_20261025/. Earlier intermediate failures are
explained above; baseline_failure.txt preserves the invalid45-step route.
This payload also includes every previously pending patrol/companion/field-target/
enemy-Critical/key-item fix from NPC_PATROL_TARGETS_20261025.md. Desch's identity35
is included in the all8 companion lookup regression; the user's new screenshot
was made with III's old a9f44da0 DLL, not this delivered correction.

When the final build is installed, from the reported position south of Tokkul,
request directions to the Enterprise or Caanan. Expect an east/west detour around
the town entrance before boarding. Selecting Tokkul itself must still enter it.
Cross-floor destinations must still direct the player to the required exit.
