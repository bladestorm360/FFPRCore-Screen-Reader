# Molten Cave progression and local door review, October26

Status: investigation complete; actual-source offline replay passes. No production
behavior change or new installation is required. The installed DLL remains
b88ebc08a531a1bb841aa99942da0ba54d85750fb0c49a5fbf7d8bfcd669cf74.
No game was launched, controlled or probed; real activation remains user-side.

## Report and result

The user reports Gutsco stealing both Horns of Ice in Dwarven Hollows, then a
dwarf directing them to northern mountains. Molten Cave is the next destination;
another world-map scene trigger is not required. The game's local scripts,
objects and actual route replay establish that the reported obstruction is
B3's rock-operated hidden door, rather than insufficient horn-story progression.

The current own log, preserved as build_check/molten_cave_20261026/ff3_report.log,
shows map191/Map_30091_3, player(8,25), layer1. Directions to the local passage
(20,6), and the onward map192 entry(11,16), both say Story gated. The log reports
two chests and two gate colliders, no blocking NPCs. Thus this report concerns
progress inside the cave, not a sealed world-map entrance.

## Native object and state evidence

| Object | Native source locator | Function |
| --- | --- | --- |
| Rock | Map_30091_3, editor16/entity9, native type5, action2/script761 sc_sw_30091_3, raw ほのおの洞窟：隠し扉を開ける | Manual examination opens the local door |
| Door collider | editor46/entity11, native type5, collision true, no action/script | Blocks the passage at baked(20,6) |
| Rock location | Tiled(22,6), baked(22,5), layer1 | Reachable before the door opens |
| Opening state | ScenarioFlag1/199 | Local switch completion, independent of Gutsco's horn dialogue |
| Onward exit | map192/Map_30091_4 entry(11,16) | Becomes reachable through the opened B3 passage |

The native switch script controls the door's hide/animation sequence, retires
the Rock control and sets the opening flag. The map initializer sc_map_30091_3,
master script191, checks199; its completed-open group ev_sw_30091_3 hides entities
9 and11 on reload. The existing map_state projection already reproduces these
effects. Exact source-bearing scripts remain outside Git in parent tools/mapdump/
ff3_scripts.jsonl, object facts in ff3_objects.jsonl and map master/map.txt.
No game source or snippets are copied into this document or repository.

The existing reviewed event identity maps the Rock's exact raw identifier to Rock
in data/ff3/eventlabels.json, with occurrence evidence in
docs/re/ff3_event_label_audit.json. It reuses the common12-language Rock label.
entity_scanner's manual interactive detector retains it; entity_filter places
manual examinations in Objects. Events holds automatic triggers. The old
translation string describing Open Hidden Door is not the current physical label.

## Actual production path and verification

tools/npc_gate_check.cpp::molten_cave extends the existing known-pattern gate
fixture, including the actual route_worker implementation. It loads the real
FFIII bake and state program, copies native-shaped current chest/gate obstacles,
and uses the exact logged player position. It checks:

* Before199: both native colliders remain, the Rock approach succeeds in37 steps,
  the local passage and onward route are blocked with the reported generic phrase.
* After199: Rock and door colliders retire, and both routes succeed without an
  additional story prerequisite.
* Existing FFII soldier, Mysidia and Castle Fynn gate regressions remain intact;
  canonical all-six map/state/route checks pass, with zero worker native access.

The existing shared no_route_reason uses Story gated for a demonstrated route
that becomes reachable only with copied gate colliders removed. It is a generic
obstruction label; it does not identify which plot event is missing. That wording
can also describe a manual puzzle door, as here. No flag bypass, movement change,
new unlock inference, native callback, timer or pointer ownership is introduced.

The fixture receives only immutable bakes, copied flags and obstacle values.
There are no game-thread operations or additional production costs. Full canonical
build passes8191 native names/0 missing,137 used/344 defined/0 undefined wiring.
The DLL hash matches the already scanned/installed all-six payload, so no game
closure, repeat deployment, dependency change or security scan is needed for this
test/documentation-only investigation. Exact reports, hashes and initial source
recovery are under build_check/molten_cave_20261026/. Offline paths do not measure
real frame/audio latency or confirm the user has activated this particular Rock.

## Accessible next step and extension limits

On Molten Cave B3 select Objects, then Rock. Request directions, approach/follow
the facing instruction, and press native Confirm to examine it. Then request
the route onward again. A manual opening action is needed; an unrelated world
scene and another trip to the dwarves are not prerequisites for this door.

The user's physical switch activation and subsequent route are unconfirmed.
If the Rock is missing or its actual interaction fails, retain that new log and
investigate the corresponding live list/script producer. Do not weaken this
verified collider rule or add a destination-specific bypass based on the
ambiguous generic wording. Other games retain their existing gate contracts;
the new exact Molten Cave regression is FFIII-specific.
