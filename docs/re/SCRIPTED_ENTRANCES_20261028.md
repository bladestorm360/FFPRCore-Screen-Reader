# Scripted map entrances and Falgabard

Human continuation of the full audit: the Saronia library's hidden mountain
village is missing by name. Ordinary map transitions must appear under Map Exits,
while an actual story event such as Saronia Airspace stays in Events. This
explicitly reopens the narrow transition classification/publication integration;
no unrelated route rewrite, game probe, delegation or publication. Estimate45–75
minutes for category/source/route checks, with any newly proven vehicle mechanism
gap estimated separately. Status: in progress, native/actual-data evidence.

The actual English library record E0234_00_124_a_02 identifies Falgabard.
Map_10040's three native entrance tiles have IDs888/889/890, type2/action2,
script381/sc_e_0354 and exact transportation restriction1 (on foot).
The complete script branches only by player facing, maintains return-side
flags218/219/220 and always changes to map144/PointIn1. It contains no story
dialogue, encounter or actor staging. Source-bearing records remain outside Git
under parent tools/mapdump; hashes are already pinned in world_event_reviews.

The first audit correctly replaced Scene trigger with Map entrance, but failed
to promote this physical entrance to a named destination/Map Exits category.
`ordinary_world_exit` recognized outward world travel only, while both live and
remote adapters hardcoded World Map for that identity. This is the exact
category/publication omission the user requested the audit catch. Extend only
fully reviewed physical travel bodies and their verified continuations; a
ChangeMap opcode alone is insufficient to turn a narrative event into an exit.
Preserve destination/arrival, physical goals, native transport restrictions,
story state and grouping. Verify both Map Exits inclusion and Events exclusion.

Actual router/capture fixture: tools/world_entrance_check.cpp. An initial fixture
matched both original Event and synthetic Warp records and attempted an invalid
RouteSnapshot assignment; those failed test reports are preserved. The corrected
fixture must match the three synthetic routing arcs without duplicating native
actors. Initial reachability runs from Saronia fail for foot/Nautilus/Invincible;
this does not prove the game cannot reach the village.

Native FFIII TransportationEvent explicitly contains an Invincible mountain
crossing mechanism (IDs10/11, up to two terrain19 cells, scripts910/911/912).
The current router has no corresponding special edge. Exact native conditions,
scripted motion and source-to-output reachability still need verification before
claiming later-vehicle access is complete. Do not invent a current-Nautilus route
or remove world reachability filtering to hide this gap. This belongs to the
open vehicle-applicability checkpoint of the full audit, not a completed fix.

No new native hook/capture has been added. The new offline fixture uses owned
real map data and existing production planners; game/native/audio confirmation
remains separate. Current FFIII installed de3e075d retains the preceding urgent
fixes. Build scheduling is now serial (/m:1 plus compiler ProcessorNumber1),
confirmed in generated project metadata. No simultaneous compilation is allowed.

Classification source checkpoint now passes: the hash-pinned offline helper
`scripted_transition_identity.py` recognizes a single native destination/arrival
across complete reviewed physical travel graphs. `bake_maps.py` sets the existing
ordinary-transfer flag; the legacy world_exit API now includes these entrances.
Adapters resolve the actual target name on the native owner, and prepared
doorway grouping includes physical scripted transfers. Geometry/master/native
object/old-route comparison proves only42 added flags: FFIII3 and FFV39; no new
arcs, dimensions, grid or destination changes. FF1/2/4/6 byte hashes unchanged.
Data generation and comparison evidence:build_check/scripted_entrances_20261028/;
before/source/generated archives remain outside Git in parent
tools/map_scripted_entries_20261028/. Falgabard full native detector chain and
remote naming, actual valley-foot routing and production category publication
pass0 failures (Map Exits only, Events empty). All-six connected-floor event
tracking and navigation follow-ups also pass. Current installed DLL remains
de3e075d; these newer classification changes are not yet built/deployed.

The independent native-rule geometry witness now proves the separate crossing
gap: Nautilus8/9 cannot reach the valley from83,62; Invincible10/11 reaches36,100
in85 displacement tiles through four valid crossings, including widths1/2.
Production routing still returns No path for10/11. Witness code is original mod
code in tools/mountain_crossing_probe.py; path/metrics are outside Git. This is
geometry evidence, not a complete story/actor/vehicle-capture claim. Native
OnAirshipSpecialEvent386B10 checks type8, stationary state and ID10/11;
IsOverTheMountain386610 checks terrain19 width≤2 and far-side transport-group
terrain permission, with wrap correction. Scripts910/911(sc_up_0001/0002)
move the real player2/3 tiles;912(sc_up_0003) supplies the failed crossing visual.
No native script, timing or motion will be invoked directly by the mod.

Next bounded source checkpoint is native crossing geometry/weighted routing,
manual crossing speech/phase/beacon handoff, exact per-leg transport identities
and combined applicability/capture tests. Estimate60–90minutes in addition to
category verification, then serial full checks/security and applicable delivery.
Retain native movement ownership; autowalk must stop for the manual special
action instead of issuing a fictitious ordinary step into a mountain. Relevant
native exports/provenance:parent tools/map_scripted_entries_20261028/
ff3_mountain_native. No game probe, extra hook, pointer handoff or worker wait.
