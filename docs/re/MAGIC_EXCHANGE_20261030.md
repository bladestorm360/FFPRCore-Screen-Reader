# FFIII Magic Exchange character selection

Human screenshot shows Magic -> Exchange, source Ryco (Summoner, HP1818/1818)
and three recipients Corrad, Rae and Robyn with HP and eight displayed MP
charge counts. This is a character selection screen. The previous reader
misclassified Exchange5 as a spell grid and rejected its character SetCursor
producer because only UseTarget2 was accepted. Six production-consumer failures
reproduce entry, navigation, source re-entry and cancellation consequences.

## Producer, ownership and behavior

Matching FFIII GameAssembly SHA256
8db9ab31a893ca4b13e5e4d2e2b1b086ac6e3fb5699b9be1d8037dafaaa3ae31:
AbilityWindowController.ExchangeInit68cfb0 hides the spell grid, rebuilds the
useController's other-character rows, publishes initial/restored cursor through
AbilityUseContentListController.SetCursor68a0f0 and registers native callbacks.
SetCursor uses the cursor Index to reparent it to the selected character row.
The source-change callback50b6a0 returns the state machine to Exchange5 and
re-enters Init; cancel50b570 returns to Command7. Native ExchangeUpdate68d770
delegates recipient input to the useController. Fields are name-resolved, not
hard-coded offsets. Original extractor tools/re/map_location_native.py supports
--magic-exchange; all raw game bodies and provenance are outside every Git tree
at ../tools/magic_exchange_20261030/ff3/.

The existing FFIII12 hooks and bounded initial-focus event are retained.
Exchange is removed from the spell-row phase. Character cursor speech accepts
UseTarget2 and Exchange5 only with the active cached native window's exact
useController/selectCursor, active hierarchy, main-menu ownership and ready
rendered character row. Entry reads Exchange, the displayed source name/job/HP,
then the focused recipient name/HP and list position. Native source switching
reads its updated header. Subsequent arrows read only the selected recipient.
An early native arrow fulfils the pending entry; the following update does not
repeat it. Deliberate later arrows remain audible, with no new text dedup.

ui::character_row retains the actual focused row/cursor on the native owner.
The existing I key reads its eight visible current MP charge counts; Autodetail
adds that tail after the focus line. No hidden maximum MP, spell inventory or
unrendered status is inferred. Closure, hidden cursor or focus reparenting
releases details through the existing shared ownership checks. No game pointer
is sent to a worker. At most9 target slots and8 MP levels are inspected; idle
UpdateController does no new work after pending entry is consumed, and unready
entry retains the existing10-update limit. No new timer, hook, native write or
input is introduced. Strings reuse localized Exchange/HP/MP/position/level keys
and the game's rendered source/recipient text.

## Evidence and limits

tools/battle_focus_check.cpp exercises the actual FFIII detours with native
objects and speech boundaries replaced by fixtures: two native Init focus
events before phase publication, one ready initial read, three recipients,
backtracking/repeated arrows, source re-entry, early-arrow completion, hidden/
foreign/inactive/unready rows, cancellation and prior Use/Learn/Remove/Forget
behavior. tools/party_esper_check.cpp independently exercises the actual shared
displayed reader/character-row service, visible HP and eight MP charge levels,
I freshness, Autodetail order and hidden/reparented-cursor lifetime. Existing
FFI/FFII character target readers already accept their own native target
producers; no matching FFIII Exchange state exclusion is shared with them.

Development reports: build_check/navigation_journeys_20261030/magic_exchange_*.txt.
Combined canonical build, smoke, exact payload scan and guarded all-six deployment
are complete at a6d9b4ce19dfb6ad; receipt navigation_delivery_20261030/completion.json. No game launch/probe. Natural gameplay, actual frame and audible
latency are not certified by the fake-native checks. At the next natural use,
open Magic -> Exchange, move among recipients, use I for charges, change source
with native2/3, then cancel. Each step should speak the current displayed row;
the source header should repeat only on entry/source change.

Status and delivery are tracked in the single
[scope ledger](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
