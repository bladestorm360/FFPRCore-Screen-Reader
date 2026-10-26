"""Match all-six baked patrol flags to exact native object movement properties.

Source JSON remains outside Git. Coordinates distinguish unnamed/id-zero actors
with the same developer name; do not pretend one such identity is unique.
"""
from pathlib import Path
import collections
import json
import struct
import bake_maps

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
for number in range(1,7):
    game=f'ff{number}'
    objects=[json.loads(line) for line in (DUMP/f'{game}_objects.jsonl').read_text(encoding='utf8').splitlines()]
    by_sub=collections.defaultdict(list)
    for obj in objects:by_sub[obj['submap']].append(obj)
    submaps=json.loads((DUMP/f'{game}_submaps.json').read_text(encoding='utf8'))
    bounds={name:bake_maps.map_bounds(info,by_sub[name]) for name,info in submaps.items()}
    source=collections.defaultdict(set)
    for obj in objects:
        if obj['type'] not in (6,17):continue
        p=obj.get('props')or{}
        ox,oy,*_=bounds[obj['submap']]
        key=(obj['submap'],obj['type'],obj['name'],obj['group'],p.get('entity_id',0),
             bake_maps.layer_index(obj['layer']),obj['tx']-ox,bake_maps.tile_row(obj)-oy)
        # Native None/Stamp can face or move through scripts without acquiring
        # an autonomous Area/Route routine. A travelling routine needs steps.
        source[key].add(p.get('move_type') in (2,3) and bool(p.get('move_step')))
    raw=(ROOT/f'data/{game}/maps.bin').read_bytes()
    header=struct.unpack_from('<8sIB3s12I',raw)
    assert header[:3]==(b'FFPRMAP1',2,number)
    strings,strings_count,submaps_offset,submap_count=header[4:8]
    def text(index):
        assert 0<=index<strings_count
        at=struct.unpack_from('<I',raw,strings+4*index)[0]
        return raw[at:raw.index(0,at)].decode('utf8')
    counts=collections.Counter()
    for index in range(submap_count):
        sm=struct.unpack_from('<IiHH4B10I',raw,submaps_offset+56*index)
        for i in range(sm[-1]):
            obj=struct.unpack_from('<BBhhHHiiiiII',raw,sm[-2]+34*i)
            if obj[0] not in (6,17):continue
            key=(text(sm[0]),obj[0],text(obj[10]),text(obj[11]),obj[8],obj[1],obj[2],obj[3])
            expected=source[key]
            assert len(expected)==1 and bool(obj[9]&16)==next(iter(expected)),(game,key,expected,obj[9])
            counts['patrol' if obj[9]&16 else 'stationary']+=1
    assert counts['patrol']>0 and counts['stationary']>0
    print(f'{game}: {counts["patrol"]} authored patrols, {counts["stationary"]} static/facing actors; exact native source/bake match')
print('All-six NPC movement metadata: PASS')
