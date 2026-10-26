# Battle hand slots and standalone controls, October25

Implementation/offline verification complete. Build, all checks, smoke and exact
payload Defender scan pass. All six installed/hash-verified, including FFII after
the user's deploy reply.144 protected files unchanged. Gameplay listening remains
pending. [Shared delivery receipt](MAP_FLOORS_20261025.md).

Screenshots show a stripped battle equipment interface: R.Hand/Favored Hand with
Sun Blade, L.Hand with Aegis Shield, Tab:Items and a displayed description. Opening
a slot shows Remove and inventory quantities (Ice Brand1, Flame Sword2, Wing Sword3)
plus a description. It has no main-menu stat comparison panel. FFII log79607..79737
repeats only R.Hand; candidates already speak Ice Brand,1. Shift+I79618 reports
No controls displayed despite Tab:Change Equipment. The October24 equipped-strip
confirmation remains valid: this Tab hand-slot screen is a different native owner.

## Native producers and battle state

KeyInput BattleEquipInfomationConroller (native spelling) owns slotList and current
contentList. FFII SelectContent43c390 assigns index/cursor source, focus and
description. UseSelectInit43d7c0 initializes/selects its slot pool. Row
BattleEquipSelectContentView has rootObject, partsText, dominantText and
iconTextView; no count/stat-comparison fields. UpdateView43e550 populates the
actual name, localized hand and favored-hand note. Candidates reuse
BattleItemInfomationController: EquipmentSelectInit626400 calls SelectContent6275e0.
Its displayed row countText and description are appropriate to speak.

`battle/battle_item.cpp` adds SelectContent/UseSelectInit/Close hooks for the hand
owner in all six games. It reads one visible row, list bound16, and uses the existing
BattleItem state, command-submenu notification, I and Autodetail. Common g_item_owner
switches to the focused panel; closing an old owner cannot clear a newer candidate.
Entry-operation sequence guards nested initial focus without text deduplication.
Candidates retain visible counts; slots never infer an inventory count or absent
stats. Pointers stay in their owner callback; no polling or worker transfer added.

## Displayed controls ownership

Footers do not own all controls. `system/key_help.cpp` now also observes standalone
KeyIconController.Initialize(bool), final UpdateView(string,string,Key[],bool),
and OnDestroy. Native SetIconData/device refresh call that renderer; matching
all-six ABI/name evidence is exported offline. Originals retain phase order;
publication registers identity and invalidates stale work, without extracting UI.

`key_help_owned.cpp` retains256 observed footers/1024 widgets. Generic Shift+I also
reads exact KeyHelpIconManager.helpControllers/iconControllers on request, covering
initialization before hook attachment. No scene or descendant search. Pointers stay
on the native-owner coroutine. Slices consider at most32 registry/candidate entries
under CaptureBudget's500us cooperative budget; footers allow64 rows, widgets16 key
texts plus3 caption/separator fields, each capped256 units. Manager rosters above
1024 footers/4096 widgets are excluded; observed owners remain available. Recency
replacement bounds observed registries. One current footer precedes visible
standalone hints. The same physical child in both lists is copied once by identity;
different widgets with identical text remain. Explicit map/Bestiary controls retain
their own footer, and the config controls popup retains its navigation buffer.

Actual view/text visibility gates disclosure. Workers only receive copied strings
and locale, join them and return for owner/generation validation. New input/request,
scene retirement, republishing a captured widget or native destruction cancels stale
work. No blocking wait, native mutation, input change or new translation string.
Budgeting cannot preempt one slow native operation. Registry-size/cold-reflection,
Unity, speech queue and audible/frame timing remain runtime limits, not guaranteed
by the fake-native tests.

## Evidence, verification and extension

Original extractor: tools/re/map_location_native.py --battle-gear. Proprietary
exports and binary/dump hashes stay outside Git at
../tools/battle_gear_20261025/ff1..ff6/. RVAs above are evidence only; production
uses name-resolved refs. Extend only from a real lifecycle/registered owner,
visible fields and verified ABI; never infer a shortcut or undisplayed statistics.

battle_focus_check exercises actual all-six detours: initial/delayed entry, both
hands/return, favored-hand field, I/Autodetail, hidden row, bad index, absent slot
quantity, candidate quantity/description and old-owner closure. key_help_check
exercises actual capture/worker formatting: footer, standalone, combined physical
identity, hidden/pool views, explicit map ownership, retirement/cancellation and
native-registry recovery after80 hidden preloads. Native worker reads are rejected.
Evidence/delivery is shared with the floor fix: build_check/map_floor_20261025/.
Final canonical tests, including native-registry recovery and all-six hand paths,
pass. Gameplay listening remains separate.

Comparable actual request-to-first-submission fixtures (50 requests each): footer
median145.7us/p95167.4us/max195.1us; footer plus standalone140.9/163.0/193.5us;
80 hidden native-registry preloads plus a visible widget1750.8/2130.7/2162.9us.
Maximum fixture ticks2/2/29 respectively. The fake clock charges100us per timing
query and drain runs ticks without real16ms frame spacing: these are offline
completion costs, not real elapsed game-frame/audible latency. Worker string
formatting and request/cancellation/capture paths are real; native/UI/speech sink
are fake. All native worker-read checks pass.2000 actual hand detours: median1.2us,
p991.5us/max36.4us; ordinary Items0.6/0.7/30.5us, equipped strip0.8/1.2/189.4us.
No worker queues/snapshot age are added for the immediate single-row hand read.
Maximum registry count, cold/native/device/frame/audio latency remain unmeasured.

Natural check after deployment: Shift+I in battle Items should identify
Tab:Change Equipment. Tab: read hand/favored-hand/item, I description, Autodetail
when enabled. Open a slot: visible quantity and I description; return restores
the hand label. Check ordinary menu controls too; hidden hints stay silent.
No game launch/control/probe, delegation or release action authorized/performed.
