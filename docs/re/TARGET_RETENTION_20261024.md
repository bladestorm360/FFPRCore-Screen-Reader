# Exact navigation target retention

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation/offline verification complete; gameplay unconfirmed.
The latest FFII logs repeatedly select Airship, then give Ship boarding directions
after battle. The player reports retention improving as distance decreases.
The cause fits the saved identity, not an unlock or distance limit: resident
vehicle FieldEntities can share default native entity IDs and placeholder names.
The old moving-entity restoration could bind the first matching vehicle after a
native handle rebuild. Closer list ordering could mask the substitution.

## Selection ownership

field/entity_scanner.cpp copies the Transportation model ID when saving a vehicle,
passes it through entity_filter's owned worker input, and restores that exact
model on the same map. Vehicle positions/handles/order may change. Other entity
identities retain map/native ID/raw developer name/category and their physical
anchor; ID zero is valid. The translated-name-only fallback is removed because
many type-only chests share that label. A missing objective remains unselected
with its saved identity retained, allowing it to reappear without substituting
another chest/vehicle. Explicit browsing/category changes still select normally.

Invalidated lists and out-of-range indices use no selection when an objective
is saved. A late worker publication cannot replace a newer selection with an
old restoration index: a selection revision restarts from the newer identity
when its current handle is absent. Direct directions/announcement actions also
pin their chosen row. Title retires the previous session's identity. Battle
entry cancels outstanding work; native battle completion drives fresh scanning.
No polling, native pointer transfer, distance cap, new unlock assumption or game
state mutation is introduced. Existing owner-thread snapshot budgets and
Navigation-worker cancellation remain authoritative.

## Vehicle interaction labels

field/interaction_cue.cpp now resolves a real Transportation model before the
FieldNonPlayer branch, matching the scanner's discovery priority. Resident vehicle
sprites inherit the NPC class and may have the literal placeholder NPC name.
The nearby native marker now uses the established localized vehicle display name,
including Airship. Icon/owner/visible occurrence and worker cancellation rules
remain unchanged; no extra proximity scan or speech text guard is added.

## Checks and remaining confirmation

entity_scanner_check recreates native vehicle handles at several increasing and
decreasing distances in each game context, with colliding entity ID zero and
identical NPC raw labels. It checks exact Airship retention, missing-model silence
and restoration after reappearance. entity_filter_check preserves exact chest
ID zero and refuses opened/name-only substitutions. Existing newer-input and
cross-floor exact-event fixtures remain. native_cues_check verifies Airship
interaction labels before NPC inheritance in all six adapters. These are actual
production readers/workers with fake native owners; gameplay/battle listening
and real audible/frame timings remain unmeasured.

Evidence: build_check/stat_effects_20261024/latest_followups*.log and combined
reports. No settings, controls, resource language keys or saved preferences change.

The first expanded scanner fixture omitted Vehicle/Chest in its fake category
adapter, leaving its new Vehicle category empty. The failure was preserved and
the fake now matches production category dispatch; no production selection guard
was relaxed to satisfy it. Production identity restoration is exercised through
the real scanner and filter worker, rather than an independent reimplementation.

A second fixture failure was an assumed initial ordinal: the retained synthetic
player pose made Airship the nearest row, so ordinal two selected Ship. The
fixture now selects the known Airship handle after production sorting and asserts
that precondition. It also races a newer browse choice against an older snapshot
of recreated handles, proving the selection revision rather than relying on order.
Both failed reports remain archived; these were fixture defects, not gameplay
passes or a reason to weaken identity matching.
