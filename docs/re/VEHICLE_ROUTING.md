# Vehicle goals, reasons and mounted filtering, October 22

Implementation complete; actual-source offline route checks pass. Gameplay retest
is pending, including the user's exact location, which was not supplied. Shared
behavior applies across all six games; native ownership, terrain and story gates
remain authoritative. [Current combined verification](../FIXES_2026_10_22.md).

## Ship/Canoe report and correction

The user saw Ship (Requires vehicle) while explicit directions said Requires Canoe,
even with the Canoe owned. Two independent defects were found in `field/baked_nav.cpp`:

- Browse RouteSnapshot copied ownership/groups and parked vehicles but omitted
  localized vehicle_names. Both request_browse_route and request_route now call
  native-owner capture_vehicle_names. Workers receive the same owned strings and
  vehicle records with game pointers removed. Genuine requirements use the same
  specific name in both descriptions and directions.
- The general transfer planner tried to ride and land after reaching a selected
  parked boat. Its water cell has no walking area for that last leg, falsely
  rejecting a valid approach with the owned Canoe. boarding_route now recognizes
  that exact selected Ship/submarine cell. It walks to a legal adjacent approach
  with owned auto-use groups, checks the native directional edge and blockers,
  then ends at the boat via the existing WalkOn boarding phase. It does not invent
  a walkable water tile or require a fictional disembarkation afterward.

A usable indirect route reports its specific action, such as board the Ship, rather
than claiming a missing vehicle. Trap warnings retain priority. Genuinely blocked
routes retain specific missing requirements; possession alone does not defeat map,
access, landing or scenario gates.

## Current map with vehicles hides mounted destinations, October 22

User report: while riding a Chocobo, Current map with vehicles hid destinations
that Current map showed; an earlier similar Ship issue was recalled. The exact
game/map was not separately specified.

Filter queries use TargetKind::Infer. On a riding grid, native transition inference
could yield an empty goal footprint even though the mount could cross the terrain;
explicit Transition already used the walking footprint. goals_for now retries an
empty inferred world goal using that footprint, with vehicle/type/landing values
cleared, then the existing mounted/landing flood decides actual reachability.
This changes goal discovery, not collision/vehicle ownership or transition rules.

## Bounds, lifetime and evidence

EntityCursor capture stays on the native owner under existing CaptureBudget slices.
Owned immutable snapshots go to the Navigation lane; no Unity/IL2CPP pointer is
usable by the worker. Route/browse owners and cancellation epochs retire obsolete
requests; publication validates the current target/map/pose/vehicle. Existing grid-
bounded flood arrays, iteration cancellation and overload rules are unchanged.
See [threading](THREADING.md), [autowalk](AUTOWALK.md) and [map state](MAP_STATE.md).

route_worker_check now constructs a real FF2 Canoe-only river approach to a Ship-only
water destination, verifies successful WalkOn at the exact boat cell and a matching
specific browse action. All-six mounted scenarios compare inferred and explicit
transition footprints. The broader retained fixture covers103 boarded world/transport
cases (85 have no eligible representative start/landing target),582 world exits and
parked-vehicle cancellation; these counts do not certify all gameplay locations.
Final timing observations and player retest are in the fix register/TESTING. No game
was launched, controlled or probed. Extend these paths through the same snapshot and
planner for both output consumers; do not add separate string-only requirement rules.
