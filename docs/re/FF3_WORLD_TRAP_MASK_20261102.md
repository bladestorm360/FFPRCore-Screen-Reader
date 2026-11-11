# World trap mask normalization

Source/focused integration Closed; canonical output/scan/install pending in the
custom-sound checkpoint. This is a minimal correction preserving the accepted
reader, router and native snapshot design.

The combined canonical run found two missing trap cells in FF3 map4,
Map_10040: Event IDs182/183, cells186/187,205, native Upper layer2. Primary
object identity comes from the external mapdump/ff3_objects.jsonl, not tester
logs or a game probe. These are the bottomless-bog event objects. The reader
stored their static mask on native plane2; world trap lookup uses routing
plane0, so the conservative raw-grid query missed both.

The reader now projects world trap masks to plane0, matching its existing
transition-mask rule. Indoor masks retain their exact native layers. No bake
geometry, object coordinates, script/state program, hook, pointer lifetime or
thread owner changes. Current and projected route requests already supply
authoritative live trap observations. An empty live trap set continues to clear
the conservative baseline; active observations and foot/air selector masks
retain precedence. This correction does not force an inactive story event to
remain a trap.

The old integrated fixture reported two failures. Strengthened checks reproduce
four explicit missing native/world-plane assertions before the correction.
Afterward all255 trap objects and198 reviewed arrival arcs across six games
pass. Room layer isolation, live disabled/active states, and separate foot/air
selectors at both real bog cells pass. Retained transport guard, FF1 Ice routes
and route-worker/native-ownership checks also pass. Focused group104.886s,
218763264B peak,exit0/drained under2GB/25%CPU/one worker. Baseline62.574s,
441729024B,expected exit1/drained; original failures preserved.

Exact sources/reports/resources and external primary-data hash are pinned in
build_check/custom_sounds_20261102/trap_regression/completion.json. The original
mod inputs and prior linked candidate are retained there. Canonical source
freeze v3 changes exactly the reader and strengthened fixture from v2; every
other frozen input is verified unchanged. Fresh canonical/smoke/scan session
17738 is active. No custom or new FF3 payload installed. Natural gameplay and
frame/audible response remain separate from capped offline evidence.
