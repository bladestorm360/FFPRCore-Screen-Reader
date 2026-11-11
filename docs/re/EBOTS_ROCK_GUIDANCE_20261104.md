# Ebot’s Rock coral and random warps

Shift+G follows the current room, collects reachable unopened coral chests, and
uses a reachable warp when another room is needed. It never promises which room
the native random warp will choose. Keep requesting guidance after the arrival
and after each chest. Once at least22 coral are held, collection stops and the
guide seeks the return to the hungry chest. This can require repeated warps;
there is no finite native attempt guarantee.

The first feeding requires Strago in the active party. A reserve Strago does not
qualify. At the initial chest choice with fewer than22 coral, choose the second
option to keep them; with22 or more, choose the first option and feed the whole
batch. Underfeeding loses the batch. An unavailable party/count observation
waits; it never authorizes feeding. Repeat encounters use the native462 exception.
Native map540 local1 owns the fed passage. Guidance then reaches the surprise,
the first or repeat Hidon, and the complete resulting native scene. Shift+G can
repeat the current instruction while this exact chest’s choice is open.

Minimal/Verbose and the existing control remaps remain independent of these
instructions. Four new instructions and the random-route label have all12
supported translations. No hidden reward contents are added to treasure labels.

## Source and ownership

Factual source identities and hashes are in
[ebots_rock_native_20261104.json](ebots_rock_native_20261104.json). Full recovered
source remains outside Git in the local `tools/puzzle_audit_20261030/` archive.
The source family is Map_30861_1/_2/_3 and the Hidon0950 continuation on
Map_20461_6, Map_20460 and Map_20461_5. Global7 is the coral counter; native
pickups add1/2/3/5. Treasure221–225 and actual current-floor locals own refill,
layout and opened state. The guide uses those facts without writing any of them.

The first Hidon scene travels540→311→305→310→305→310. Only its last continuation
sets461. Earlier310 is not completion. Repeat callback stays on540, sets463 and
clears462. Existing ready-state completion owns announcements; battle/map
boundaries cancel stale work without discarding the armed native scenario receipt.

The bounded current-corps role reader now copies Locke2 and Strago8 in the same
pass, using the same native getter calls and prior list bounds. It matches saved
character IDs to owned party members; absent/malformed data remains unknown.
Owned flags/signatures and the story worker receive only copied values.

`author_ebots_rock_guide.py` owns the twentieth recipe within the existing32 cap.
Its rules target only the current floor, prohibit intervening event crossings,
and keep feeding, party and fed/boss cases disjoint. The existing native-choice
instruction cache adds only exact map540/entity2, still bounded to512 characters.

`ebot_warps.py` pins the native source and records12 possible arrivals per random
pad. Derived Warp bit19 records a random attempt. The initial539/entity5 has one
deterministic edge followed by12 random alternatives; exact graph-only mask
guards select the appropriate phase from461/area2, rejecting unknown phase data.
The physical Event is unaffected. Native terrain, objects, masters, other rooms
and previous recipes remain unchanged. Existing bounded graph search and normal
route ownership remain in charge; there is no new hook, worker queue or search.

Normal directions explicitly request guidance after the random warp. Browse
labels say Random warp required instead of promising the chosen hypothetical
next map. On the actual native handoff, an observed attempt retires without
marking that pad consumed. The requested destination is retained and the next
request uses the actual new map. Unstarted attempts cannot complete. Repeated
trials remain available, including same-map random landings.

## Verification and limits

Actual role-reader, story-model/controller and production-planner fixtures are
used; no game is loaded or controlled. The initial fixture found the second
entrance pocket needed the lower pad as an alternative; that missing target was
corrected. Current-room checks include all nine native pocket arrivals and
three layout profiles, feeding/threshold/party/unknown/fed/first/repeat stages,
normal random directions and all12 actual handoff outcomes. Source/geometry
preservation and canonical author/localization/semantic checks accompany them.
Exact receipts and final pass/fail identities belong to
`build_check/broader_audit_20261104/ebot_completion.json` when closed.

These capped fixture measurements establish bounded offline behavior, not live
audible latency or gameplay completion. Final combined build/security/deployment
and natural gameplay remain separate. When progression permits, verify that
Shift+G adapts after native warps, never advises underfeeding, can repeat the
chest choice, and stops only after the complete Hidon sequence. No new gameplay
test is required to review source correctness.
