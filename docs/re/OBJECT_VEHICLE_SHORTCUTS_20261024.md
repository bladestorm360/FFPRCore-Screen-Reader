# World-object vehicle route comparisons

Status: complete; full build/offline checks, smoke and payload scan pass. This is
included with the NPC story-name mask; both are now deployed/hash-verified in all six games.
The latest FFII own log31994..32037 proves a258-step walking route from(71,76)
to the cave entrance at(159,143), while Airship is five steps south at(71,81).
The user explicitly selected Current map with vehicles. A walking route existed,
so the old planner never considered a legal vehicle alternative.

## Shared routing behavior

field/baked_nav.cpp now captures the filter's vehicle-shortcut policy with each
explicit and browse snapshot. Current floor retains walking when available;
Current map with vehicles and Current map compare legal parked-vehicle routes
against that walking route. The existing unreachable-target vehicle fallback
is unchanged. No new setting, control, distance/radius cap or unlock inference.

The shared planner serves Objects, NPCs, chests and exits. It reuses the existing
boarding/riding/landing/final-walking candidate and chooses it only if its actual
phase movement count is lower. Ties retain walking. This measures movement steps,
not predicted time or native animation speed. Current selected vehicle targets
remain bound to their own model rather than borrowing another vehicle.

A wrapped Manhattan lower bound rejects only mathematically unhelpful vehicle
comparisons: distance to a boarding side plus unobstructed flight to any legal
final approach. It is deliberately optimistic; real terrain, ownership, boarding,
landing and final interaction must still be proven by the production planner.
No native pointer crosses threads. All candidate searches remain on Navigation,
with existing cancellation. Safe walking paths do not acquire an intervening
story event merely for a shortcut; native hazard/gate restrictions remain.

Kept plans record the captured policy. A walking plan is reconsidered if a newly
nearby observed model could improve it; a vehicle plan is retired if its required
model is absent from the copied current/parked models. Existing target IDs/raw
names and floor continuation remain authoritative. Ground interaction goals
also require the legal landing/foot approach when already airborne; the separate
native vehicle-action pad contract (including Palamecia) remains mounted/manual.

## All-six applicability and evidence

The original all-six world NPC/manual-object census finds no applicable world
placements in I/III; II/IV/V/VI have manual placements. Their target vehicle lists
are empty or explicitly foot/1. Ordinary ground interaction differs from type21
vehicle-action pads, whose existing allowed model IDs remain preserved. Shared
world routing and per-game transport/landing data are exercised by the full
all-six fixture suite. No game-specific movement/controller hook is changed.

navigation_followup_check uses the actual FFII cave object entity48/script143,
its real world bake and group ev_e_0087. The fixture reproduces the logged origin,
Airship position and post-Palamecia story state. It compares walking/vehicle
policies, boarding/landing/facing, an already-airborne approach, newly available
and missing cached vehicles, and a one-step town entrance where walking should
win. Existing Palamecia, Leviathan, long-distance island, world exits, native
ownership and cancellation checks remain. Fake native owners do not establish
live/audible latency or activation success. Exact timings/results will be archived
under build_check/emperor_spoiler_20261024/ with the combined delivery.

The initial new fixture omitted the party's already-owned Canoe/Snowcraft groups,
so walking was impossible and the old unreachable-target fallback already chose
the Airship. The fixture now includes real automatic groups2/5 at this progression
stage, reproducing the reported walking alternative. The failed report is retained;
no production route eligibility was loosened to satisfy it. A separate compile
failure was a fixture variable colliding with Windows' near macro; renamed only.

## Combined verification and delivery

Canonical build exits0 with7671 native-name checks/0 missing,131 used/339 defined
wiring names/0 undefined; every required checker passes. All12-language actual
NPC translator/numbering and shared live/remote/snapshot label checks pass.
Logged cave comparison:258 steps walking,153 with parked Airship,149 already
aboard; full+browse combined43.759ms offline compute. Legal boarding/landing/facing,
newly available/missing cached models, short walking route and filter/legacy mode
cases pass. These are actual production planners/real bakes with simulated native
owners; live audible/frame response and gameplay activation remain unconfirmed.

The isolated DLL smoke host resolves/forwards all47 exports and completes boot
without launching a game. All49 embedded data+18 sound resources match sources.
The exact22-file stage passed enabled/current Defender with no matching detection
and unchanged hashes. Existing unsigned-DLL/no-certificate and administrator-
protected exclusion-query limits remain; no protection change or external upload.

DLL10874880 bytes, SHA256
d75864ec918d7eaeb0300d5a342ff3e9a520983b3aadd27f8554d7117535d627.

Installed/hash-verified in all six closed games. Every installed22-file payload
matches the scanned stage;144 protected files remain unchanged against the fresh
pre-deployment baseline. Recovery, prior failures, source/resource hashes, build/
check/smoke/security and final installed manifests are retained in ignored
build_check/emperor_spoiler_20261024/. No game probe, dependency upgrade, release,
commit/push or delegation. Gameplay listening/landing confirmation remains pending.
