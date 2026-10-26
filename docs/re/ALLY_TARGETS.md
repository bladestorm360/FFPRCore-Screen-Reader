# Attack targeting and displayed ally rows

October24: implemented; canonical build and offline checks passed, gameplay pending. The FFII
screenshot shows Firion1363/1605 HP, current MP10 and Defend. Maximum MP is not
displayed and must not be invented.

Existing Battle.Command PlayerInit and player SelectContent own initial focus
and cursor moves. FFII PlayerUpdate input closures tail-call SelectContent; no
new polling/hook is needed. Attack can keep its controller inactive while
independently rooted ordinary party rows render. The old initial read could
substitute an active spell/item controller; the party reader also retained only
its last live controller even when its rows were hidden.

The exact target producer now keeps its own cursor/list. Battle.Party retains up
to16 live owners observed by Initalize, UpdateView, SetFocus and the target's
playerInfomationController. Destroyed owners retire on observation; BattleStart/
End clears the registry. No scene search occurs. Each owner has at most9 rows.
Visible view/name establish evidence, not the controller's active flag. Matching
uses native BattlePlayerData or owned-character identity, not hidden statistics.

Full ally targeting includes displayed name, HP, current MP, commandText and existing
rendered conditions. The [Roundworm/status follow-up](ROUNDWORM_STATUS_20261024.md)
separates H/party status summaries: those omit the separate command column while
full target speech retains it. Hidden fields stay absent. Counts/positions and FF6's
existing continuous-focus identity guard are unchanged. No text deduplication
is added. Native reads remain on their owner.

All-six exports/provenance are outside Git in ../tools/ally_targets_20261024/ffN/.
FFII locators: PlayerInit85B230, PlayerUpdate85B8E0, SelectContent(player)85C520,
input closure8654D0, party UpdateView635B20, ordinary row632CC0, target row8548E0.
Native party UpdateView fills both pools, with distinct renderer ownership.

party_esper_check uses actual party/renderer code with a hidden newest owner
and rendered older owner in all six adapters; it checks the screenshot's numbers/
command and hidden command exclusion. battle_focus_check covers actual entry,
delayed rendering, moves, groups and teardown. Fake views cannot establish live
owner layout/audible timing. Remaining Attack retest is in TESTING.


## October24 delivery and limits

Canonical build and every required offline check pass; isolated smoke resolves
and forwards all47 exports. Native-name audit:7,373 checks, zero missing; action
wiring:131 used/339 defined, zero undefined. All49 embedded data and18 sound
resources match current sources. DLL10793984 bytes, SHA256
 3c8562206a2fe22ca9540a4349cff82ec04c68ee645e34db8f7e5f8df73b5b82.
Installed and hash-verified all six after the user closed FFII. All144 protected
files match the fresh applicable deployment baseline. Recovery, reports,
source/resource hashes and receipts: ignored build_check/mysidia_objects_20261024/.
No game launch/control/probe, dependency install, delegation or release/push.
Gameplay/listening confirmation remains separate and pending; see TESTING.
