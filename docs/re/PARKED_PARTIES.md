# FF6 parked parties and route occupancy

October23. Implementation complete; canonical build, all offline checks and isolated smoke pass.
Gameplay confirmation applies to the preceding Terra guide/progression build,
not this new occupancy correction. Current request also authorizes V0.9.5 through
the established [release procedure](../release.md).

## Evidence and mechanics

The user's latest FF6 log is1,209,004 bytes, modified October23 at07:41 local on
the initial read. Lines13144–14397 repeatedly route from14,33 to Guard Leader
at14,40 with two fixed NPCs and zero moving NPCs. Party3 switches at14526;
15710 again offers seven steps South. The user confirms another party occupies
the corridor and is a hard collision. The same session reaches Guard Leader
battle and Locke's post-defense dialogue, confirming the preceding progression
work while exposing this missing obstacle class.

FF6 stores parked corps in FieldController.partyEntityList, a separate native
FieldPartyEntity array. Existing EntityCursor enumerated entityList,
colliderEntityList, transport models and foot triggers; it never included that
array. Saved party poses used by danger advice did not make them physical route
obstacles. The correction uses active native actors rather than remembered poses.

| Native FF6 locator | Verified contract |
|---|---|
| Last.Map.FieldController.partyEntityList, offsetB8 | FieldPartyEntity array, at most three corps slots |
| GetPartyEntity301B60 | Bounds-checks and returns the array slot without reordering |
| PlacementPartyEntity3116A0 | Native negative party identifier maps to the array slot; placement uses native transforms |
| Last.Entity.Field.FieldPartyEntity.CanEnterFF28E0 | Inherits ordinary NPC collision; special same-position allowance permits leaving an overlapping start |
| Current.SwitchCorps2C34B0 | Native party replacement/placement owns the active and parked identities |

Source-bearing disassembly and matching PE/dump hashes stay outside Git at
../tools/ff6_story_20261023/native.txt and provenance.json. The bounded exporter
is tools/re/ff6_story_native.py. No game was launched, controlled or probed.

## Implementation and ownership

field/party_entities.{h,cpp} owns FF6-only class/field references, bounded native
array iteration and pointer-to-slot identity. Other games return before resolving
those names. Missing arrays and counts outside1–3 are unknown; no unbounded scan,
scene search, remembered position or save-file read is introduced.

EntityCursor's final stage yields one party slot per call. Existing cross-list
native identity guards prevent duplicate captures. Owner-side active/lifetime
checks omit hidden or removed markers. The destination scanner skips their display
rows; they are physical obstacles rather than invented NPC destinations.

baked_nav.capture_obstacle copies party identity, native cell/layer and corps slot.
No pointer enters workers. Temporarily unknown party ownership remains solid;
a named boundary can never create a prefix through an earlier unknown marker.
selected_live_npc excludes synthetic party properties,
so their property IDs cannot impersonate a baked boss or story actor. live_blockers
keeps parked parties in their own hard-occupancy collection, on the actual layer.
They survive gate-free searches and never become story barriers. Existing manual,
browse, guidance, follower, filter and landing snapshots share EntityCursor.
Native collision and approved native movement/input remain unchanged.

The user refined the contract during implementation: do not replace distant
directions with a standalone blocked/switch message. Prefer a completely legal
detour. When all approaches are occupied, an owned search removing only parties
proves the otherwise legal route and truncates it exactly one walking step before
the first party. Directions retain that usable prefix and append “Blocked by
Party1.” Only an already-adjacent player receives that phrase alone. No direction
step enters an occupied tile. A scripted nonadjacent landing is not an adjacent
party warning. Gates, NPCs, chests, traps and geometry remain unchanged.

The partial phase has a manual terminal boundary; autowalk stops there without
facing/interacting with the boss or claiming arrival. Beacon arrival stays false.
The original destination remains selected; partial routes are freshly replanned
when requested, so a party moving clear cannot leave a stale blocked endpoint.
Defense keeps the commander/urgent objective and omits its fight instruction
until an actual approach is clear. Existing held-switch policy remains separate.

Danger advice compares the world after each hypothetical native party switch:
remove only that party's own parked marker, add the formerly active party at its
current cell, and retain all other blockers. Snapshot obstacles are restored
before normal planning. Partial blocker approaches do not count as successful
interception when comparing the closest party. All three candidates use the actual shared route worker;
missing poses still suppress a closest-party claim. No switch or move is automated.

Native capture remains inside existing0.5ms admission slices; one indivisible
engine call can exceed that. At most three array slots/identity comparisons are
added. Searches use the existing bounded Navigation lane, snapshots, cancellation,
request epochs and ready-owner publication. PartyChanging cancels old routes,
selections/audio and cached reachability; fresh requests observe the current actors.
No worker waits on native work and the game thread never waits for the worker.

## Controls, localization and verification

Existing Shift+G and ordinary pathfinding controls are unchanged. There is no new
setting. One prompt has all12 game-supported locales, using existing Auto/manual
selection. Luna/gpt-6-luna supplied the bounded catalog task (two initial prompts, replaced by12 final values after
the user's refinement); Sol reviewed
translations/placeholders and idempotence and owns all mechanics/code/testing.

party_entities_check exercises the actual native collection adapter with three
slots, holes, changed ownership, rejected counts and no FF6 reads in FF1–5.
ff6_defense_check exercises actual FF6 bakes, the logged corridor, manual/browse/
guide prefixes, adjacent-only standalone blocks, safe autowalk stopping,
another-floor exclusion, moving clear and
changed active party. Existing all-six routing, state, input, name-entry, defense
warning, autowalk, vehicle and speech checks remain mandatory. Final results and
performance limits are recorded here after verification.

The user's report confirms the preceding defense guidance works, including the
commander goal, and documents this remaining occupancy defect. This fix still
requires a gameplay check when convenient: leave Party1 at the narrow approach,
switch to Party2 and request guidance. Expect a legal detour or usable directions ending with “Blocked by Party1”,
then a valid route after moving Party1 clear. No replay is required
solely to delay the requested release; offline evidence and gameplay limits are
kept separate.


## Final offline and installation evidence

Canonical build passes7,182 native names with zero missing,125 used/333 defined
wiring names with zero undefined, and every included offline regression. The
isolated smoke host loads/forwards all47 HID exports and exits normally.
The actual defense fixture passes native corps capture, legal detours, occupied
corridor prefixes, adjacent-only standalone suffixes, a clear northern approach,
long brief/full suffix retention, unknown-slot solidity, autowalk terminal Action,
layer exclusion, movement clear and active-party replacement. Existing all-six
story/filter/map-state/vehicle/name-entry/speech/interaction checks also pass.

This run's danger-advice fixture worst0.985ms is owned offline computation with
fake native boundaries; it is not live callback/audible timing. Earlier failures
assumed a full block where the real map has a37-step detour, or treated a northern
approach as the same isolated corridor. Those expectations were corrected from
actual geometry without removing barriers. Native capture cost under game load,
queue tails and audible timing remain unmeasured. Existing route diagnostics
retain capture/native/queue/compute/usable-response/snapshot-age accounting.

All six local games now have the tested V0.9.5 DLL; all144 protected dependency,
settings and waypoint files retain bytes/timestamps. All19 embedded map/state/
puzzle/catalog resources match source. Recovery, build/check/smoke reports,
protected-file manifests and delivery.json are under build_check/v095/.
DLL SHA256: ea1cd296802a9f2510b06759b2077d57481ca6711c02d931c45549a9b0230419.
No game launch/control/runtime probe occurred. The new party correction remains
gameplay-unconfirmed; release publication is recorded in docs/release.md.
