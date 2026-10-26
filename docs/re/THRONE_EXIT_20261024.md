# Castle Fynn throne-room transition companion

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation/offline verification complete. Latest own FFII log
26539.390 labels Exit to Castle Fynn3F as having an Event trigger on the route;
26589.531 separately lists Scene trigger (Castle Fynn Throne Room).

The extracted Map_20041_7 assets place native Event/editor104/entity96,
script1504/sc_transition_20041_7, raw editor tile(26,30) on BottomObject.
The native object anchor is one row above the editor bottom edge; the baked
route cell is(26,29). A GotoMap/editor105/
entity84 shares its exact layer/group/rectangle and leads to map40/point2.
The script name describes the transition, and this is the only sc_transition_*
placement across all six extracted map sets. Its script asset is absent from the
package/script manifest; we do not claim to have inspected an unavailable body.
Classification uses the combined exact identity/name and real exit overlap, not
a rule suppressing any scene that happens to overlap a door.

bake_maps.py validates those facts before assigning the helper baked-only type101.
The original script/action/entity/name/group/footprint fields remain intact.
The native GotoMap alone owns the exit and route edge. baked_map.cpp prepares a
small ID index on the Data worker; live scanner discovery and live route hazard
capture both consult that immutable index by exact ID/raw name/footprint. Remote
projection and static hazard classification omit the helper by type. There is no
callback map scan, script execution, grid/flag mutation or distance change.

Only FFII's bake changes; all-six source census is checked by
oct24_navigation_check.py. navigation_followup_check uses the real throne room
and a live helper snapshot, proving the exit's route has no false warning. A
separate independent live scene on the same tile must remain a hazard. The real
story events and Leviathan's reviewed Ship entry retain their prior semantics.
Player-side exit/listening confirmation remains pending. Factual source locators
only are recorded here; game assets/scripts stay outside Git in ../tools/mapdump/.
Evidence/recovery: build_check/stat_effects_20261024/.

The first new fixture incorrectly used the editor bottom edge as a route cell.
The prepared-index test exposed that mismatch; it now uses the actual baked
anchor and the logged relative approach (two west/eighteen south). No production
geometry was changed to satisfy the fixture. The failed report is retained in
the combined evidence folder.
