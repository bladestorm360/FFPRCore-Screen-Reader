"""Qualify the four FFIV doors whose native item menu selects an owned key."""
import csv
import json
from pathlib import Path
import sys

import bake_maps

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
DOORS = {
    'Map_20020': ((66, 'sc_e_0156', 68, 271, 31), (67, 'sc_e_0157', 68, 272, 318)),
    'Map_30151_11': ((28, 'sc_e_0158', 75, 273, 221),),
    'Map_30201_1': ((6, 'sc_e_0062', 74, 58, 269),),
}


def lines(filename):
    with (DUMP / filename).open(encoding='utf-8') as source:
        yield from map(json.loads, source)


def table(filename):
    with (DUMP / 'ff4_master' / filename).open(encoding='utf-8-sig') as source:
        return list(csv.DictReader(line for line in source if line.strip()))


def main():
    rooms = {room: [] for room in DOORS}
    for row in lines('ff4_objects.jsonl'):
        if row['submap'] in rooms:
            rooms[row['submap']].append(row)
    scripts = {(row['submap'], row['name']): row for row in lines('ff4_scripts.jsonl') if row['submap'] in rooms}
    names = {int(row['id']): row['script_name'] for row in table('script.txt')}
    contents = {int(row['id']): row for row in table('content.txt')}
    submaps = json.loads((DUMP / 'ff4_submaps.json').read_text(encoding='utf-8'))
    path = ROOT / 'data/ff4/mapstate.json'
    original = json.loads(path.read_text(encoding='utf-8'))
    for room, doors in DOORS.items():
        objects = rooms[room]
        ox, oy, *_ = bake_maps.map_bounds(submaps[room], objects)
        gates = []
        for entity, script, content, receipt, destination in doors:
            door, = [obj for obj in objects if obj['props'].get('entity_id') == entity
                     and names.get(obj['props'].get('script_id')) == script]
            assert door['type_name'] == 'Entity' and door['props']['collision'] and door['props']['action_id'] == 2
            assert any(obj['type_name'] == 'GotoMap' and obj['props']['map_id'] == destination
                       and (obj['tx'], obj['ty'], obj['layer']) == (door['tx'], door['ty'], door['layer']) for obj in objects)
            assert contents[content]['type_id'] == '1'
            body = scripts[room, script]['mnemonics']
            assert any(op['mnemonic'] == 'SysCall' and op['operands']['sValues'][0] == 'アイテム選択肢表示' for op in body)
            assert any(op['mnemonic'] == 'Branch' and op['operands']['sValues'][:3] == ['ItemSelection', '＝', 'imm']
                       and op['operands']['iValues'][1] == content for op in body)
            assert any(op['mnemonic'] == 'SetFlag' and op['operands']['sValues'][0] in ('ScenarioFlag1', 'ScenarioFlag2')
                       and op['operands']['iValues'][0] + (256 if op['operands']['sValues'][0] == 'ScenarioFlag2' else 0) == receipt
                       for op in body)
            cell = [bake_maps.layer_index(door['layer']), door['tx'] - ox, bake_maps.tile_row(door) - oy]
            gates.append(dict(id=entity, group=door['group'], raw=door['name'], cell=cell,
                              content=content, receipt=receipt))
        if '--check' in sys.argv:
            assert original[room].get('item_gates') == gates, room
        else:
            original[room]['item_gates'] = gates
    if '--check' not in sys.argv:
        path.write_bytes((json.dumps(original, ensure_ascii=False, separators=(',', ':')) + '\n').encode('utf-8'))
    print('Verified four FFIV item-menu doors and their actual same-cell destination exits.')


if __name__ == '__main__':
    main()
