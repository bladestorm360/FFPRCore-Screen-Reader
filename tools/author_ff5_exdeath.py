"""Author the exact contacts used for Castle Exdeath's complete crossing."""
from pathlib import Path
import hashlib
import json
import sys

import bake_maps
from author_puzzle_types import expected
from ff5_rule_resources import update_header

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
TOPIC = 'docs/re/FF5_EXDEATH_20261105.md'
STOP = 'Stop the floor. Request guidance after it stops or after the battle.'
RETRY = 'Restart the floor, then request guidance again.'
REARM = 'Step South once, then North once to restart the floor.'
RESTORE = 'Step on this tile to restore the bridge.'
CROSS = 'Cross this trigger, then request guidance again.'
EXIT = 'Take this exit, then request guidance again.'

def read(path):
    return json.loads(path.read_text(encoding='utf-8'))

def author():
    rooms = {'Map_30441_13': 397, 'Map_30441_17': 401}
    objects = {room: [] for room in rooms}
    source_hashes = {}
    passage = None
    for line in (DUMP / 'ff5_objects.jsonl').open(encoding='utf-8'):
        obj = json.loads(line)
        if obj['submap'] in objects:
            objects[obj['submap']].append(obj)
    for line in (DUMP / 'ff5_scripts.jsonl').open(encoding='utf-8'):
        script = json.loads(line)
        if script['submap'] in {*rooms, 'Map_30441_16'}:
            body = json.dumps(script['mnemonics'], ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode()
            source_hashes[script['submap'] + '/' + script['name']] = hashlib.sha256(body).hexdigest()
        if (script['submap'], script['name']) == ('Map_30441_16', 'sc_e_0438_1'):
            passage = script

    info = read(DUMP / 'ff5_submaps.json')
    bounds = {room: bake_maps.map_bounds(info[room], actors) for room, actors in objects.items()}
    def target(room, entity, typ, text):
        obj, = (actor for actor in objects[room] if actor['props'].get('entity_id') == entity
                and actor['type_name'] == typ and actor['group'] == 'entity_default')
        left, top, *_ = bounds[room]
        return dict(map=rooms[room], entity=entity, raw=obj['name'], group=obj['group'],
                    layer=bake_maps.layer_index(obj['layer']), x=obj['tx'] - left,
                    y=bake_maps.tile_row(obj) - top, kind='transition' if typ == 'GotoMap' else 'spot', text=text)

    roulette, bridge = rooms
    targets = dict(start=target(roulette,24,'Event',CROSS), stop=target(roulette,27,'Event',STOP),
                   retry=target(roulette,24,'Event',RETRY), north=target(roulette,4,'GotoMap',EXIT),
                   north_reset=target(roulette,25,'Event',CROSS),
                   collapse=target(bridge,14,'Event',CROSS), restore=target(bridge,1,'Event',RESTORE),
                   exit=target(bridge,13,'GotoMap',EXIT), west_return=target(bridge,8,'GotoMap',EXIT),
                   west_return_alt=target(bridge,9,'GotoMap',EXIT), middle_return=target(bridge,12,'GotoMap',EXIT),
                   east_return=target(bridge,10,'GotoMap',EXIT))
    guide = dict(id='exdeath_castle_crossing', maps=list(range(397,402)), targets=targets,
                 texts=[REARM], evidence=TOPIC, source_script_hashes=source_hashes)

    # The full scene ends with the controlled player at the existing PointIn 7.
    # The cinematic starts elsewhere and never calls ChangeMap.
    assert passage is not None
    program = passage['mnemonics']
    assert len(program) == 55 and not any(row['mnemonic'] == 'ChangeMap' for row in program)
    assert program[36]['mnemonic'] == 'SetPuppet' and program[36]['operands']['iValues'][:3] == [0,-1001,41]
    assert program[44]['mnemonic'] == 'SetPos' and program[44]['operands']['iValues'][:3] == [15,4,0]
    assert program[45]['mnemonic'] == 'Show'
    signature = hashlib.sha256(json.dumps(passage, ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    arc = dict(submap='Map_30441_16', entity_id=20, destination_map=400, original_point=0,
               continuation='sc_e_0438_1', continuation_sha256=signature, same_map=True,
               arrival_point=7, ordinary_travel=True,
               evidence='Full 55-operation passage restores the controlled player at native PointIn 7, Ground 15,4. ' + TOPIC)
    return guide, arc

def main():
    check = '--check' in sys.argv
    path = ROOT / 'data/ff5/puzzles.json'
    data = read(path)
    guide, arc = author()
    rows = [guide if row['id'] == guide['id'] else row for row in data['story_guides']]
    if not any(row['id'] == guide['id'] for row in rows):
        rows.append(guide)
    arrival_path = ROOT / 'data/ff5/warp_arrival_overrides.json'
    arrivals = read(arrival_path)
    matching = [row for row in arrivals if (row['submap'], row['entity_id']) == (arc['submap'], arc['entity_id'])]
    if check:
        assert data['story_guides'] == rows and matching == [arc]
    else:
        data['story_guides'] = rows
        data['guidance_types'] = expected(5, data)
        path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        assert not matching or matching == [arc]
        if not matching:
            arrivals.append(arc)
            arrival_path.write_text(json.dumps(arrivals, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    update_header(data, check)
    print('Castle Exdeath: actual moving-floor stop and retry, bridge restoration, and safe exit.')

if __name__ == '__main__':
    main()
