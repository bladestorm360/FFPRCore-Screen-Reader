# Mobliz letter quest and hidden passage

Two additional FFVI guides complete the five-letter quest and the later hidden
house/Terra journey. The later guide is selected only after actual story353,
retains any missed first-visit prerequisites, and stops only after full356.
Earlier ordinary story interactions remain available through normal navigation.
No terrain, native objects, arrival overrides or map-state programs were changed.

The letter guide starts with wounded soldier502, then follows five distinct
animated letters and paid requests. Read/payment/delivery receipts are
503/504/505,506/507/508,509/510/511,512/513/514 and515/516. Native517 from the
soldier's final scene completes the quest. Refusing or failing a payment does
not advance it. The first option pays in all five real native menus. Shift+G
can repeat the actual published instruction at exact88/10 while its choice is
open, without another route, flag or party capture. No future reward contents
are narrated.

Reply delivery is an actual native callback, not elapsed time. The weapon,
armor, item and relic shops run0226 after closing their menu; a purchase is
unnecessary. An inn rest also qualifies. The guide offers four real shopkeeper
targets for each pending reply, then returns to the next exact letter. Targets
use authored priority/first reachable selection; route directions use the
current position. The letter sequence is classified as non-navigation guidance.

After story353, the hidden-house guide can lead from World2 into the village.
If needed it first covers child notice316, Terra317, native first battle318,
full return319 and magicite320. That gift sets area-local1. The guide therefore
routes a real village exit and return; it never clears that bank or infers a
reset from distance. Dog cue354 then opens the actual hidden passage, discovery
355 leads to213/9, and the full final battle/scene owns356. Entering the room or
battle is insufficient. This sequence uses navigation guidance.

Indoor targets use the existing strict event-avoidance graph. Only the explicit
World2-to-village return opts into world transit, with strict event avoidance
off as required by that existing graph contract. Story/item/vehicle checks
remain intact. Two failed early fixtures retained in the completion record
caught an invalid world/strict combination in the new recipe; the correction
changes only its authored policy, not the common router or map geometry.

`tools/author_mobliz_guides.py` pins79 connected source producers from the
external archive, including shop-close, inn and both battle/scene continuations.
Factual `mobliz_native_20261104.json` contains hashes and physical identities;
source-bearing bodies remain outside Git. The two new instruction keys have
all12 translations under `tools/modtext_mobliz.py`. Existing common strings
and23 earlier story recipes are retained. There are now25 FFVI story guides
and88 explicitly classified recipes/native policies across all six games.

Actual-consumer checks pass35 assertions/62 real route legs for the whole letter
and hidden-house journey, including world exit/return and every animated letter.
The current story model passes630 physical identities and tests refused/failed
payment, all source receipts, selection353, real area re-entry and final356.
The current controller passes the exact new choice repeat without native reads,
plus its retained cancellation/ownership/other-guide cases. A minimal optional-
episode integration correction retains “No puzzle guide for this map” before
Palace invitation226 and before this guide's353 selection.

These fixtures use representative owned positions and source-qualified state
transitions; they do not measure actual game movement, audible latency or
native frame cost. The route fixture's displayed worker clock is a controlled
fake clock and is not performance evidence. No new hook, poll, native getter,
worker protocol, native mutation, game launch or runtime probe was introduced.
Combined DLL build/smoke/security/deployment and natural gameplay remain pending.
