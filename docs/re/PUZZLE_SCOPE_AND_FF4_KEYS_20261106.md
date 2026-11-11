# Ordinary interactions and puzzle guide scope

The FFIV game log records a false guide activation on entry to Devil's Road
at 88020.859 and “Examine the switch. Story gated” at 88023.406. A guide also
announced itself on entry to Baron's inn at 88165.953. The first guide counted
opening a passage and walking through it as two steps. Its old operation
receipt could also finish from unrelated movement. The inn switch required
only one interaction.

The all-six catalog review removed 45 standalone guides for simple switches,
ordinary key doors, single items used at a control, uncomplicated party gates
and scene or battle follow-through. This includes the one-switch detours at
Cultists' Tower and Cave on the Veldt. Native door, event, NPC and object data
stay with the existing navigation system. Source and route checks still use
the archived control identities in
`ordinary_puzzle_interaction_fixtures_20261106.json`. The DLL embeds only the
active catalogs under `data/ffN/puzzles.json`. The independent Navigation and
Other settings apply to the remaining guides.

Ordered switches, actual warp sequences, constrained traversal, linked item
or form operations and choice sequences remain in scope. Examples include
Troia's four switches, FFIV's Magnetic Cave, FFIII's form sequences and FFVI's
Opera House, Narshe lights, Phoenix Cave and Kefka's Tower. An ordinary
interaction can still appear as one step within a genuine larger puzzle.

## FFIV key doors

The log reports obtaining the Baron Key at 88563.015. The Old Waterway save
point still reports Story gated at 88629.765 before native unlocking.
Directions to the shop and its chests work after the doors are opened.

The native item-selection scripts confirm four collision actors that share a
tile with an ordinary exit. Map24 entities66/67 require content68, the Baron
Key. Map220 entity28 requires content75, Lugae's Key. Map268 entity6 requires
content74, Luca's Necklace. Each exact control retains its own story-group
selection and opened receipt. Owning a key does not spawn an absent door,
consume an item or open the native collision.

The installed FFIV build's important-item menu reads
`UserDataManager.importantOwendItems`. Its dictionary key is selected by
`OwnedItemData.get_ContentId`. Ordinary `SearchOwnedItem` addresses the
normal dictionary. The navigation capture reads the three applicable
important items and copies only their ownership bits. The existing worker
uses those bits to qualify an onward journey, then stops the current
directions at the real door for manual interaction. Missing or unrelated
keys preserve the route gate. Only the native script owns the unlock.
`tools/author_ff4_key_gates.py` checks the native item branch, receipt and
same-cell destination. Native instructions stay outside Git under
`../tools/native_evidence/ff4_key_owner_20261106/`.

## Shop number row

The FFIV log shows ordinary shop item focus and details speech between
88831.281 and 88835.531. It contains no shop slot comparison action in that
interval. Two keyboard context paths could shadow the stock comparison.
A retained information buffer could take precedence over a visible stock
party panel, and pending guide speech forced unrelated hotkeys into Global.
The input manager now keeps Shop ownership for its visible comparison
panel and reserves the guide context override for the field. Number keys
still use the existing per-slot binding and rendered result, with ordinary
character switching outside the shop comparison panel.

## Verification and installation

The guide controller checks passed with 45 active control operations and
silent entry and no manual guide on the reviewed ordinary maps. The static
route fixture still checks 89 active and archived controls. The FFIV
production planner replay verified key absence, unrelated keys, a manual
approach with the right key and ordinary travel after opening for all four
doors. Both Baron destinations also retain their onward goal across the
manual unlock. The input fixture checks each of the nine stock comparison
slots with overlapping contexts, and the rendered comparison fixture passes
for all six games.

One canonical build compiled all 281 projects. An older FF1 test still read
the Lute slab by its former guide index. The test was corrected to use the
ordinary interaction archive and the remaining canonical checks passed
against the unchanged DLL. The smoke test and exact Defender scan passed.
All six local installations match the scanned payload, with 132 file hashes
verified and 24 protected files per game preserved. Receipts and recovery
are under `build_check/puzzle_scope_20261106/`. Gameplay is pending.
