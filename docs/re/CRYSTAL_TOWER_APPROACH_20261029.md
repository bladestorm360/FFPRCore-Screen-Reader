# Crystal Tower approach after the statues are cleared

Human follow-up queued after Doga's Village surfacing marker, October29.
Status: source/offline/FFIII delivery Closed. Installed b61153fc96534b9d73b95d43adb216557faaddf555b532ececdf4bf4622d15ae,
11357184 bytes. Finite checkpoint estimate30–60 minutes for the own log,
exact statue/entrance scripts, native state and actual baked routes for both
Nautilus and Invincible. Combined checks, scan and guarded installation passed; the exact receipt is
build_check/crystal_tower_20261029/completion.json. Gameplay remains user-side.

The user reports all statue barriers disabled, yet Crystal Tower remains
unreachable with either aircraft. Preserve the own session log at
build_check/crystal_tower_20261029/reported_session.log before rollover. Verify
the actual intended approach/ordinary entrance and current cleared-state
colliders/triggers. Distinguish present native collision, stale captured state,
incorrect baked flags/geometry and an actually required walking/vehicle leg.
No invented terrain, loosened vehicle winds or automatic reopening of closed
navigation designs. Add the smallest actual regression/data integration fix
supported by source-to-output before/after evidence, then combined FFIII checks,
scan and guarded installation. No game launch/probe/delegation.

The own log at462982.156 reaches Ancients' Maze1F using the Invincible, but
Crystal Tower197,37 fails from151,74 and later from46,57 with Nautilus. Native
world4 GotoMaps173/216/175 are respectively197,41→maze299,197,39→maze304,
197,37→Tower305. Return PointIns connect different isolated world pockets.
From the real southern maze approach197,42, the existing graph already finds
the real maze route (9 visited states), yet normal local routing says No path.
The outer fallback explicitly excluded world maps, and its initial graph flood
also assumed walking while the player could still be flying.

Selected-route candidate now reuses the native legal-aircraft landing coverage
for its initial world graph state, then preserves walking on actual later-floor
and world-return arrivals. First-leg reconstruction retains flight/landing/walk
phases and the real dungeon entry. The ordinary world GotoMap fallback may use
this existing graph. Real before1 failure→0; Invincible from both logged origins
chooses map299, then returns near Tower through the maze. Nautilus alone remains
unable to cross its actual mountain approach. No statue gate, terrain, wind rule,
map arc, native input or vehicle capability is removed or invented.

Category reachability must use the same graph before pruning an inaccessible
world-local exit. It now batches at most512 immutable baked GotoMap targets in
one existing400-state search and one established later-floor-NPC retry, rather
than performing one full graph per list row. It preserves gates, fewest-trap
preference and cancellation; eligible distances come from real routes. Native
pending-transition history remains selected-goal-owned, not applied to unrelated
bulk rows. The story-gate proof also uses this same batch for both gated and
ungated coverage. The expanded all-six regression exposed two newly reachable
world targets whose old local-only gate proof was insufficient; the source
integration is corrected, without weakening the gate assertion. All-six route
worker now passes, including582 world candidates,98 boarded model/world cases
and actual captured-gate tests. Cross-floor Soldier, remote exits, all63 aircraft
landing witnesses, Invincible crossings, continent edges and Doga marker pass.

## Cleared statue projection

Exact ScenarioFlag1 bits88..91 already hide all eight native statue objects.
Their four trigger puppets202..205 are actually parked at145,59 when cleared,
but the offline generator only tracked FFVI absolute positions. The new test
found hidden8/relocated0 before this data correction, versus hidden8/relocated4
after. This is a separate real data mismatch; the original Tower failure also
required the world/dungeon route integration above.

Native FFIII Puppet.SetPos9419A0 mode0 converts the current position through
IMapAccessor slot4, normalizes absolute X/Y through slot109, then replaces the
specified cell coordinates. FieldController.ConversionMapRangeCell285040
subtracts the current submap origin, verified0,0 here. MoveEntityLayerWithWorldPosition
93D900 converts through slot3 and applies native layer request2 as GameObject
layer10. World routing normalizes this to its existing ground layer1; no indoor
layer rule is widened. Complete native/provenance archive remains outside Git
at ../tools/doga_surface_20261029/ff3/. The complete initializer canonical hash
is cb8e2a844de521a5ad58bd0422e5faa62691019d7191d98e32d16371cc674392.

bake_map_state.py authorizes only this exact root/hash, origin and four actor/
track identities. Four existing Map_10040 puppet-position effects at115/142/169/
196 change from unspecified XY to145,59. Every other field/program and every
bake is unchanged. Unreviewed non-VI position commands remain opaque. Receipts:
build_check/crystal_tower_20261029/before_mapstate.json, state_delta.json,
native_position_extraction.txt, positions_after.txt and gate_crystal_tower.txt.

## Verification, ownership and limits

The real category pipeline retains Tower under Current map with vehicles for
the Invincible; it does not invent a Nautilus mountain crossing. The original
Tower map4/cell197,37/entity175 survives the landing/maze first leg. Native
PointIns on maze299 and304 produce the next real leg; the north return197,38
then reaches Tower locally. Empty owned-vehicle assistance preserves walking;
mid-search cancellation and native-worker ownership pass. All-six immutable
world GotoMap inventories fit the512 batch bound (largest150). The existing
400-state bound and at most one remote-NPC retry apply per graph call; category
gated/ungated/eligibility and the selected request are distinct bounded passes,
not a single400-state allowance. No game-thread wait or pointer handoff is added.

Latest complete99-target world eligibility is94.014ms Nautilus and137.796ms
Invincible on the owned worker; these are computation/worker-launch measurements,
not game-frame stall or audible latency. All-six representative category peak
75.482ms and boarded route peak46.849ms remain offline timings. Read-only native
capture, existing coalescing/cancellation and owner publication are unchanged.
Do not infer actual responsiveness below the user's cumulative25ms application
lag threshold solely from worker placement. Runtime/frame/queue/audio tails and
natural gameplay remain unmeasured. Canonical regressions and final delivery are
complete; no game launch/probe occurred.

The shared graph stays in its existing private snapshot/route implementation:
its scoped initial-world coverage, reconstruction and batch result use the
same state, native applicability, traps and pending ownership. Splitting that
already large file would require a separate closed-design refactor; this fix
does not justify an unrelated source reorganization. The new marker constants
are separately owned by nautilus_surface.h. No controls/settings or native hook
are added. Existing translated exit/landing directions are reused.

## Combined delivery

Final serial full rebuild18m03.7s passes canonical checks,8290 native names/
0 missing,142 used/349 defined/0 undefined, isolated47-export startup smoke,
all67 exact embedded resources and12-language marker/endpoint checks. This
was a shared-header/full-test rebuild; ordinary small incremental builds are
not claimed to take this long. Single-project/compiler RAM settings stay.
Defender1.463.24.0 reports no threats in22 exact final payload files, no matching
detection events, hashes unchanged. The restricted status query failed first;
the preserved unrestricted scan succeeded. No protection setting was changed.
Exclusion status remains unavailable; no signing certificate is available.

The guarded FFIII-only installer found the game closed and installed directly.
All22 installed hashes match;144 protected files and other5 game payloads are
unchanged. Recovery0a511191 retained. Receipt:
build_check/crystal_tower_20261029/completion.json. Live ally statuses and Doga's
surfacing marker share this payload. No game was launched/probed or source
published. Gameplay/audio confirmation and the broader six-game audit remain
separate. This frozen verifier must not be rerun against later audit source/data.
