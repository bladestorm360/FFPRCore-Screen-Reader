# Bestiary rendered-data capture, October 22

Implementation complete; offline lifecycle checks pass for both Config and title
Extras paths in all six adapters. Final combined build and gameplay confirmation
are recorded in [the fix register](../FIXES_2026_10_22.md). No game was launched.

## Report and root cause

FF2 Config named Leg Eater but read serialized zero/dash fields and Japanese prefab
headings. The user reports the old individual mods worked. Original supplied output:

> Name: Leg Eater
> -: 000
> -: 000
> -: 000
> -: 000
> -: 000
> -: 000
> -: 00000
> -: 00000
> -: 00000
> 獲得アイテム:
> 種族: -, -, -, -
> 弱点: -, -, -, -, -
> 耐性: -, -, -, -, -, -, -, -, -, -, -, -, -, -, -, -, -, -, -, -
> 吸収: -, -, -, -, -, -, -, -
> 出現場所: -, -, -, -, -, -, -, -, -, -

Native FF2 LibraryInfoController.SetData (RVA96EBE0) selects targetData and key
help; it does not populate the detail text. UpdateView (9701C0) later calls the
KeyInput window (9715F0), which fills the header and invokes Common.LibraryInfoContent
.UpdateView (53EB90). A fixed two-frame delay could therefore read the prefab.

## Ownership and flow

`bestiary_patches.cpp` selects the controller/data generation before native SetData,
so synchronous nested population also works. The actual LibraryInfoContent.UpdateView
postfix marks only the content owned by that controller and selected MonsterData ready.
Native LibraryInfoController.UpdateController then waits for visible content and a
nonempty rendered name. There is no scene search, arbitrary readiness timer or
master-data replacement for UI fields. Config and Extras both use this producer.

A native-owner StatCapture cursor reads five tables in UI order: monster parameters,
status, options/rewards, item drops and properties/habitat. `bestiary_reader.cpp`
copies a scalar row or one list leaf per step. CaptureBudget limits each slice to
0.5 ms between steps; one native accessor can exceed that allowance. Counts above
64 rows/table or64 list leaves/row are rejected. Retained text is limited to4096
UTF-16 units/field and65536 total (including the header). Oversized data can be
truncated/omitted instead of accumulating unbounded work. Native string decoding
retains the existing shared maximum; this is not a measured game-frame guarantee.

Only active displayed text is copied. Native unrevealed-item placeholders remain
intact. A change of selected data/controller, close or hidden/destroyed content
cancels unfinished capture. `bestiary_navigation.cpp` receives the owned entries by
move, joins list text and builds NavigationBuffer on the compute worker. Native
pointers stay in owner callbacks. Stale generations cannot publish. Initial Name
speech queues; arrows, group/top/bottom and current-stat controls retain existing
intentional review interruption. See [input bindings](../INPUT_BINDINGS.md).

## Native locations and validation

| Game | Common LibraryInfoContent.UpdateView RVA |
|---|---|
| I | 4EE7C0 |
| II | 53EB90 |
| III | 657F00 |
| IV | 4A7E40 |
| V | 58FF80 |
| VI | 4670A0 |

Names/signatures are checked against every installed dump. Exact FF2 native
analysis and all-six call metadata stay outside Git in
`D:/Games/Dev/Unity/FFPR/tools/issuefix_20261022/`; read-only Ghidra changes were
discarded. No game source/snippets were added to documentation.

`tools/bestiary_check.cpp` exercises the actual detours/reader with delayed native
population,48 updates without readiness, hidden header, both entry paths, repeated
updates, close cancellation, synchronous nested population and malformed/text bounds.
The fixture proves producer/lifecycle behavior, not the actual user's Leg Eater
values. Remaining simple player checks are in [TESTING](../TESTING.md#october-22-fixes-test-these-first).
Extend the reader through the native completed producer and bounded cursor; never
restore a time-based placeholder read or bypass native discovery/reveal gates.
