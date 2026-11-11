"""Author the timed escape guide and the three final native landing corrections."""
import hashlib
import json
from pathlib import Path
import sys

import bake_maps
from author_puzzle_types import expected
from ff5_rule_resources import update_header

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent/'tools/mapdump'
TOPIC = 'docs/re/FF5_KARNAK_20261105.md'
CROSS = 'Cross this trigger, then request guidance again.'
EXIT = 'Take this exit, then request guidance again.'

def read(path):
    return json.loads(path.read_text(encoding='utf-8'))

def write(path,value):
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def signature(value):
    return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def author():
    continuation = {('Map_10010','sc_e_0048_2'),('Map_40005','sc_e_0048_3'),
                    ('Map_10010','sc_e_0048_4'),('Resident','sc_e_0047_5')}
    scripts = {}
    for line in (DUMP/'ff5_scripts.jsonl').open(encoding='utf-8'):
        row = json.loads(line)
        room, name = row['submap'],row['name']
        if room == 'Map_20070' or room.startswith('Map_20071_') or (room,name) in continuation:
            scripts[room,name] = row
    assert len(scripts) == 120
    blast = scripts['Map_20071_6','sc_e_0043']['mnemonics'][15]
    assert blast['mnemonic'] == 'SetFlag' and blast['operands']['sValues'][0] == 'ScenarioFlag2'
    assert blast['operands']['iValues'][0] == 97
    body = scripts['Map_20071_7','sc_e_0047_4']['mnemonics']
    assert body[11]['mnemonic'] == 'SetFlag' and body[11]['operands']['iValues'][0] == 46
    assert body[12]['mnemonic'] == 'SetTimer' and body[12]['operands']['rValues'][0] == 600
    assert body[37]['mnemonic'] == 'SetPos' and body[37]['operands']['iValues'][:2] == [4,7]
    final = scripts['Map_10010','sc_e_0048_4']['mnemonics']
    assert len(final) == 22 and final[1]['mnemonic'] == 'SetFlag' and final[1]['operands']['iValues'][0] == 47
    assert final[11]['mnemonic'] == 'SetPuppet' and final[11]['operands']['iValues'][:3] == [0,-1001,15]
    assert final[16]['mnemonic'] == 'SetPos' and final[16]['operands']['iValues'][:3] == [83,78,0]

    objects = []
    for line in (DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
        row = json.loads(line)
        if row['submap'] == 'Map_20070':
            objects.append(row)
    left,top,*_ = bake_maps.map_bounds(read(DUMP/'ff5_submaps.json')['Map_20070'],objects)
    def target(entity,kind,text):
        actor, = (row for row in objects if row['group'] == 'ev_e_0048' and row['type_name'] == 'Event'
                  and row['props'].get('entity_id') == entity)
        return dict(map=95,entity=entity,raw=actor['name'],group=actor['group'],layer=bake_maps.layer_index(actor['layer']),
                    x=actor['tx']-left,y=bake_maps.tile_row(actor)-top,kind=kind,text=text)
    f = lambda index,value: ['scenario',index,'eq',value]
    rules = [dict(when=[f(245,0)],targets=[target(29,'spot',CROSS)]),
             dict(when=[f(245,1)],targets=[target(entity,'transition',EXIT) for entity in (54,55,56)])]
    guide = dict(id='karnak_escape',maps=list(range(95,112)),transit_maps=[1,581],
                 when=[f(42,1),f(43,1),f(46,1)],complete_scenario=47,rules=rules,evidence=TOPIC,
                 source_script_hashes={room+'/'+name:signature(row['mnemonics']) for (room,name),row in scripts.items()})
    arrivals = [dict(submap='Map_20070',entity_id=entity,destination_map=1,original_point=36,
                     continuation='sc_e_0048_2',continuation_sha256=signature(scripts['Map_10010','sc_e_0048_2']),
                     arrival_cell=[83,78,2],ordinary_travel=True,
                     evidence='Full world, explosion and final world callbacks restore the controlled player at 83,78. '+TOPIC)
                for entity in (54,55,56)]
    return guide,arrivals

def main():
    check = '--check' in sys.argv
    guide,arrivals = author()
    puzzle_path = ROOT/'data/ff5/puzzles.json'
    data = read(puzzle_path)
    rows = [guide if row['id'] == guide['id'] else row for row in data['story_guides']]
    if not any(row['id'] == guide['id'] for row in rows):
        rows.append(guide)
    arrival_path = ROOT/'data/ff5/warp_arrival_overrides.json'
    existing = read(arrival_path)
    matching = [row for row in existing if row['submap'] == 'Map_20070' and row['entity_id'] in (54,55,56)]
    if check:
        assert data['story_guides'] == rows and matching == arrivals
    else:
        assert not matching or matching == arrivals
        data['story_guides'] = rows
        data['guidance_types'] = expected(5,data)
        write(puzzle_path,data)
        if not matching:
            write(arrival_path,existing+arrivals)
    update_header(data,check)
    print('Karnak: actual timed start, exit encounter, three exit contacts, and full world return.')

if __name__ == '__main__':
    main()
