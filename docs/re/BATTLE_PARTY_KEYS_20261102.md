# Battle-only party check keys

Owner requested slots1–5 on Slash, Period, Comma, M, N, freeing native keyboard
L/R2/3. Checks must work in native battle submenus and keep existing non-active
character status behavior. A follow-up explicitly preserves these keys' outside
battle functions. Scope/status are in the single
[audit ledger](MAP_AUDIT_FOLLOWUPS_20261028.md).

Registration in battle_party.cpp defines exactly five distinct actions using
the existing read_row path. Rendered positions, hidden/absent slots, conditions,
non-active characters, H/T and controller review retain their established
semantics. No enemy turn schedule or new native hook is added. Existing typed
party rows support five; there is no four-member truncation to preserve.

BattleParty is an appended input scope; earlier context values and unrelated
binding identities stay stable. determine_context still returns the real native
information/status/menu context. A cached read-only overlay admits party checks
in ordinary battle, or Information/Status while BattleStateHelper confirms
battle. Loading/transitions, results and key help cannot activate this overlay.
Existing modal/typing/focus checks still precede ordinary dispatch. Registry
ordering gives the overlay priority for its own key while leaving other native
menu controls in their original context.

bindings.cpp prepares native key ownership from at most five catalog entries
on the input owner. Native suppression callbacks read one cached boolean and
never scan the catalog/party or call a game query for this feature. Exact plain
modifiers, physical focus/held/release and keyboard_allowed retain their existing
contracts.2/3 have no party registration/ownership; native L/R passes through.
Outside battle, slash/period/comma retain waypoint actions, M reports the map,
and other field/menu N bindings remain available. New per-slot control labels
and the scope name cover all12 UI languages; automatic/manual locale choice is
unchanged. Old numeric party binding identities are retired without modifying
other saved bindings.

Meaningful offline acceptance uses actual registration/row readers for all six
games and FFIV's fifth rendered row; actual frame dispatch/native input detours
for command and item/magic information contexts, all five outside-battle keys,
2/3 passthrough, help/loading/focus/modifier boundaries and unchanged controller/
modal handling. Other narrow native-input fixtures explicitly model no party
ownership. Source/focused qualification is Closed: five initial fixtures and
four supplemental fixtures all pass. The actual native detours make10,000
callbacks with zero extra key-state polls. Frozen source/report/resource hashes
are in build_check/battle_party_keys_20261102/completion.json.

The older isolated delivery branch receives only these binding/dispatch/cache
deltas, preserving its existing guide APIs and other behavior. Eleven focused
delivery fixtures pass, with52 matching header-cache compiler contracts and719
frozen inputs. Canonical167targets/174freshreports,8326native names/0missing,
smoke/47exports/67resources/12locales and exact22-file Defender scan all pass.
FFIV installed November2 at18:16UTC with22matching payload hashes,144protected
files and the other five games unchanged. DLL011f0dc2ce20e815fc3aa3f5a2b5fb4f3844331e3f3ee28a00f34e943d1a3a45,
11550208B. Exact delivery receipt is sibling Class-Delivery/build_check/
ff4_party_keys_20261102/completion.json. Fresh guard found IV closed; no game
launch/control/probe. Other game installations wait for the final all-six main
delivery; natural menu/input/frame/audible confirmation remains separate.

## November 6 submenu correction

The actual FFIV game log records eleven `BattleParty.Slot` dispatches during
Magic at 71185.234–71187.562 with no corresponding speech. The input overlay
already admitted the keys. The ordinary party panel was hidden, so the visible
row filter returned no party slots.

The existing `Last.UI.KeyInput.BattlePlayerInfomationController.UpdateView`
postfix now identifies the normal party owner by its non-target call. Matched
native code for all six titles binds the ordinary list in HUD slot order and
clears the row's target data for an empty member. The reader keeps the existing
visible path and reads one ordinary bound row on demand in Items, Magic, or
targeting when the requested row is hidden. Name, HP, current MP where the
row has an MP field, and current applied ally conditions use the existing
battle helpers. The native red-HP cue still requires a visible color. The
owner survives battle readiness if the panel bound earlier, and clears on
battle exit or return to title. There are no new hooks, worker reads, or
periodic work. H and the controller review keep their original scope.

The actual-source Party/Esper replay initially reproduced 50 silent menu
requests across six games plus one failed live-status assertion. It covers
all five keys, FFIV's fifth slot, a newer target-only panel, changed vitals and
KO with stale HUD text, missing slots, and closing or leaving the submenu.
Reports and capped-run receipts are under
`build_check/enemy_ko_20261106/visual_stage/party_submenu_*`. Native body and
vitals locators are in the existing external `ffN_party_display_calls` archive.
For gameplay confirmation, open Items or Magic in battle and press the key
for another party member. Expect that member's current status without moving
the menu selection.
