# FFVI stable rentals

All seven remaining stable discoveries are ordinary paid chocobo services,
with three native stages each. Four are on the first world and three on the
later world. They require no separate puzzle guide or route geometry fix.

Each merchant introduces the service once using scenario548, then offers the
normal first-option confirmation. Native fee116 removal writes the success
local; insufficient funds end the interaction without travel. A successful
rental stages the player outside the stable, transfers to the appropriate
world PointIn and runs the actual chocobo/leader boarding callbacks. The
later-world roots additionally animate the chocobo. The introduction flag is
neither rental ownership nor consumption of the repeatable merchant.

| Stable room | Merchant entity | Native outer room | Final world/point |
| --- | --- | --- | --- |
| 189 | 5 | 188/2 | 1/104 |
| 613 | 0 | 612/2 | 1/107 |
| 615 | 0 | 614/2 | 1/108 |
| 617 | 0 | 616/2 | 1/109 |
| 316 | 5 | 315/2 | 2/36 |
| 619 | 5 | 618/2 | 2/37 |
| 621 | 5 | 620/2 | 2/38 |

The merchant remains an approachable interaction target before payment. Paid
travel is not turned into an unconditional graph shortcut, and rental548 never
pretends that the player currently owns/rides a vehicle. Existing current
vehicle snapshots and native landing rules remain authoritative after boarding.
No terrain, collision, gate, hook, game-facing string, input, worker or guide
behavior changes in this source-only capability.

Exact seven root/fourteen callback identities, flags, transfers and hashes are
retained in `ff6_stables_native_20261104.json`. Full native bodies stay in the
external `ff6_Chocobo_Stable` audit archive, outside Git. The first extraction
assumed only two stages; its failed count assertion is retained, and the
complete21-producer chain is explicitly qualified. Original script/control
facts are checked by `tools/review_ff6_stables.py`; seven exact semantic
dispositions are in the canonical review file. No new reversible-code test is
needed because no runtime or navigation data changed. Current guarded delivery
and gameplay confirmation remain separate from source qualification.
