# Ur clerk and character MP details, October25

Follow-up: the user confirms manual I works. The [MP-only payload correction](FF3_FEEDBACK_20261025.md)
removes the repeated name/vitals from manual details and verifies that
Autodetail speaks one short row then current MP levels once. Prior build and
deployment evidence below remains historical; the combined follow-up supersedes it.

Status: complete/offline verified and installed/hash-verified in all six games.
Gameplay confirmation remains user-side. No game was launched, controlled or
probed. Sol owns this work; no new delegation or dependency installation.

## I-key character ownership

The prior short-row change correctly moved FF1/FFIII current MP charge counts
to I/Autodetail, but manual details required Cursor.IsFocus to be true. Native
StatusWindowController.SelectInit explicitly sets that flag false on both
cursors while enabling selectCursor and disabling targetSelectCursor. It is a
selection-lock/scale flag, not evidence that keyboard focus is inactive. The
old fixture incorrectly set it true. This explains the rejected manual read.

ui/character_row.cpp now validates the real overview state machine: Select2 or
SortSource3 owns selectCursor; SortTarget4 owns targetSelectCursor. None0 and
passive List1 cannot steal I from another menu. Other item/spell target rows use
their actual visible cursor/row ownership, without interpreting IsFocus as an
active flag. Positive main Menu5, live content, visible cursor image and row
anchor remain required. displayed_ui.cpp centralizes cursor_matches using the
exact template defaultCursorParent/focusCursorParent base fields and existing
generic anchors. FFIII frontParent/backParent are separate formation artwork,
not its native cursor anchors. The initially investigated Front/Back and
PositionConstraint hypotheses were rejected by native evidence.

All pointers and rendered text stay on the UI owner; only copied speech reaches
the existing worker. State checks occur on manual I, not on a new timer. No new
hook, scene scan, input interception or text deduplication is introduced.

MP presentation remains current counts for levels1..8, last in manual details
or Autodetail. Ordinary row speech remains HP/status, with displayed job/TNL
and other overview labels in their existing order. Scalar-MP games retain
their displayed layout. party_esper_check now uses native Select2 with IsFocus
false, exact cursor anchors distinct from formation artwork, fresh/hidden MP,
passive List/None rejection, different-cursor rejection and retired contexts in
both FF1 and FFIII. Visible item/spell targets with IsFocus false also remain
readable. Prior tests claiming that false alone identifies a gray cursor were
incorrect and are replaced with phase/ownership assertions.

Native FFIII locators: KeyInput StatusWindowContentController.DefaultCursorParent
6CB770, FocusCursorParent6CB780; StatusWindowController.SelectContent6DB120;
SelectInit6DB240; template KeyInput SetFocusCursor4B9F80, SetCursor4B9460;
Cursor.SetParent433D90 and SetFocus433B60. The extractor
tools/re/map_location_native.py character-rows mode archives matching all-six
functions/provenance outside Git in ../tools/character_rows_20261025/ffN.
No proprietary implementation is copied here.

## Ur's inn has an item clerk

Native map6, Map_20011_1, contains ShopNPC entity10/editor14 at12,18 Ground,
product group3, action8. Its developer label resolves to Item Shop (Elder 2).
The separate Elder is entity12 at7,8; the Woman provides the free inn dialogue.
Relay entity21/editor22 is at12,19 Ground. Both the NPC link_object_id and
relay object_id are zero. The relay tile is closed to walking; the correct
approach is12,20, facing north. From the pub doorway8,18 this is2South,4East.

The user's own FFIII log at93579..93581 skipped NPC row2 between Woman1of5
and Elder3of5. The current-floor reachability test could only find the clerk's
isolated adjacent tiles, so it filtered the clerk out. Current-map inclusion
does not prove the approach is usable; the correction repairs the route goals.

Native InteractiveTargetObserver.SearchInteractiveArea tests the tile one step
in each direction from the player against active relay ranges. A one-cell relay
adds one tile to that direction's interaction reach. No matching object ID is
required, and the player does not have to stand on the relay. Existing routing
incorrectly required a positive matching ID and selected the relay itself.

baked_router.cpp now includes walkable cardinal approaches across one-cell
relays, retaining accept_action_direction, layer and actual NPC facing. Native
overlapping relays add their reach; search bounds derive from actual relay
count/map bounds, with no route-distance cap. Normal adjacent approaches are
retained. Moved NPCs use their copied current footprint. This change is scoped
to NPC/ShopNPC, whose native relay eligibility is established; it does not
extend chest/event interaction ranges without whitelist evidence.

map_state::add_relays copies only active projected relays into Request geometry.
Full worker goals use their existing immutable RouteSnapshot Resolver. Quick
reachability copies flags once when a new flood starts, projects/collects relays
on Navigation, then uses the published immutable relay geometry for per-entity
goal checks. Group changes already invalidate the quick grid. No live Unity
pointer enters a worker, and no native thread waits for routing. The fallback
pure-data API only admits default or the selected actor's group.

The existing linked multi-cell relay rule is retained. Native rectangle-origin
mapping for the13 wider/taller relays is not certified by this fix; no claim of
new generalized multi-cell counter support is made. The one-cell audit covers
556 actual relays across all six games, including conditional groups. Active
conditional relays are state-projected; inactive/unknown variants grant no range.

FFIII native locators: SearchInteractiveArea5C07A0,
CanPlayerInteractToEntityPosition5BFB10, IsFront5C0460,
FieldRelayInteraction.IsInRange F18D60, Init F18CE0,
PropertyRelayInteraction.ConvertPropertyToData4618F0. The matching all-six
archives are ../tools/counter_interaction_20261025/ffN, outside Git. The
extractor is original mod tooling. Real baked geometry and own-log evidence
remain separate from user gameplay confirmation.

## Validation and continuation

tools/route_worker_check.cpp checks real default counter/NPC pairs in all six
games, zero-link pairs, facing, Ur's closed counter, inactive relay denial and
the actual immutable worker planner. tools/route_check.cpp corrects the older
Fynn shop expectation to the usable approach beyond the relay. Build and smoke
remain the canonical build.bat/smoketest.bat paths; deploy.bat is separate and
continues to refuse replacing a running game's DLL. Evidence is collected in
build_check/inn_counter_mp_20261025. Runtime callback/audible latency is unmeasured.

Once installed, prioritize: I on FFIII equip-character selection, I on an
item/spell target, then the Ur item clerk under the current-floor filter. Repeat
FF1 character I when convenient. Expected: current MP levels at the end, no
extra charge chatter with Autodetail off, and a route to the clerk's south-side
counter approach. Failure symptoms: silence, wrong character/description or
the clerk still omitted. No game launch is required for the offline checks.

## Final build and deployment evidence

Canonical build and all offline checks passed:7827 native names,0missing;
133 used/341 defined wiring names,0undefined. The actual character/detail
fixture, all-six routes, current-floor quick filter and immutable worker planner
pass. Real usable counter/NPC pairs checked:FF1 52,FF2 83,FF3 50,FF4 58,FF5 107,
FF6 107 (457 total,354 zero-link actors). The complete relay inventory contains
556 one-cell and13 multi-cell relays. The latter retain their prior behavior.

The isolated smoke host resolved/forwarded47 HID exports and exited normally.
All49 embedded data and18 sound resources match source. Final hid.dll is
10,915,840bytes, SHA256
e9ede9879e1bd3736d5831f8769e72e4f6994bc90451a69b6933660337900858.
Defender found no threats in the exact22-file staged payload, with hashes
unchanged. Installed hashes of all22 files match in every game;144 protected
files/settings/dependency receipts remain unchanged. No signing certificate was
available; the DLL is unsigned. The protected exclusion query remains unavailable,
so the scan does not certify exclusion configuration or universal scanner acceptance.

Evidence:build_check/inn_counter_mp_20261025/verification.json,
native_audit.json,relay_inventory.json,security_final_predeploy/,
deploy_after.json and installed_payloads_verified.json. The superseded anchor
hypothesis receipt is explicitly named; it is not the final native result.
Initial development attempts caught a test-helper namespace error and an
incorrect Touch/KeyInput field-name hypothesis. Both were corrected before the
successful final canonical build/scan/install. The original assumption that
IsFocus false means inactive was replaced by direct native SelectInit evidence.

Performance limits:the new quick job copies at most2080 logical flag values once
per new flood; projection/relay enumeration runs on Navigation. No native wait is
added. Existing whole-map vehicle/flood/cancellation checks pass, but actual
native callback cost, gameplay response and audible timing were not measured.
