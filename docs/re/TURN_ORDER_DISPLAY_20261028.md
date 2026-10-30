# FFIII displayed turn-order investigation

Human request: determine whether sighted players see turn order; add Shift+T
only if a turn-order list actually exists and is rendered. Investigation Closed
for the examined installed FFIII build: no upcoming-actor sequence is displayed.
No new hotkey, information disclosure, native hook or runtime feature was added.
Sol only; no game was launched, controlled or probed. Installed9146ae03 is unchanged.
The held full map audit resumes at its recorded vehicle-applicability checkpoint.

## Evidence and display ownership

tools/re/turn_order_display.py resolves37 actual battle-UI prefab addresses plus
the main battle/message roots through the installed compact Addressables catalog.
Only their six catalog-listed bundle dependencies are loaded. The final traversal
starts from39 exact prefab roots and follows4330 descendant GameObjects, with0
unresolved component scripts. All113 battle-UI class field/method inventories
were inspected alongside the serialized display bindings. No upcoming-turn list,
actor timeline, order row, turn portrait sequence or corresponding display
controller/binding was found. Shared bundle dependencies contain unrelated menus;
the final tree excludes their independent roots (including formation's Order UI).

The battle display binds party names, HP/MP, conditions, focus and command text;
enemy/target information; command/item/equipment/ability selection; and current
action/result messages. command_icon_list contains special-ability input glyphs,
with up/diagonal arrows and key icons, rather than actor portraits. Merely finding
this asset is not evidence of future combat order.

Shared player-row assets also contain ATB gauges. The exact FFIII provider
Serial.FF3.Management.SystemConfigData.GetBattleType, RVA349E40, returns1,
whose verified enum meaning is Command. SystemConfig.get_BattleType63A140
forwards to that native provider. BattlePlayerInfomationController.UpdateView
6A8480 selects UseCommandView for1 and UseATBView for0. The native command view
6A6400 enables command text and disables the gauge parent/component. These shared
gauge fields therefore do not establish a rendered FFIII readiness display.

Internal BattleProgress/BattleProgressTurn symbols concern scheduling: for example
readyActStack_, SetActData, GetStackBattleActDatas and ActClac. They are not exposed
by an upcoming-order display. No mod reader observes or announces that internal
schedule. Existing current-actor/action announcements remain unchanged.

## Reproduction and limits

tools/re/map_location_native.py 3 --turn-order-display exports the exact known UI
producers/config getter. Facts, complete selected-root hierarchy, serialized
component-field inventory, hashes and native exports are outside Git at
../tools/turn_order_display_20261028/ff3/. Initial broad dependency-node inventory
and intermediate native extracts were superseded by the root-scoped traversal;
the final facts contain no unrelated root. Matching GameAssembly SHA256:
8db9ab31a893ca4b13e5e4d2e2b1b086ac6e3fb5699b9be1d8037dafaaa3ae31;
dump SHA256cf74887b07f39807182c95701b3888ebfe171f0c9f7afbe8f33811b47c3d4b15.

This is exact installed asset/native-code inspection, not a new runtime capture
or a claim about modified/future builds. Public publisher screenshots on the
[FFIII Steam page](https://store.steampowered.com/app/1173790/FINAL_FANTASY_III/)
are consistent with the examined HUD, but the local display evidence owns this
conclusion. The user's conditional implementation requirement is not met;
Shift+T remains unassigned by this follow-up. No build/deployment is needed for
this read-only finding. Native/frame/audio performance is unchanged by the
investigation; offline tooling never attaches to the game.
