"""Pin navigation facts from fourteen reviewed native patrols, offline only.

No game scripts or original route bodies enter Git. The manifest contains actor
fingerprints, source hashes and coordinates, checked against the original archive.
"""
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ARCHIVE = ROOT.parent / 'tools/npc_geometry_context_20261029'
PINNED = {
    (5, 'Map_20041_9'): '909015769990bcf159e9fdd820d4be150dd48cb406398b00ae8e5375a4a83e45',
    (6, 'Map_20090'): 'a41931c6feb3d1b382022284f3dd9cb10004fc203738b737af9fc257bc6f08de',
    (6, 'Map_30511_7'): '12e961707d567888a9e87c2c79b2be710f14339b7f5fafe7fb00d34e1a2ef329',
    (6, 'Map_30641_1'): 'd0e65233f20e9c96505f651eec1622f000653c83a13e4139c815eded1b145c70',
    (6, 'Map_30671_1'): '62f89dcf360b4e57843fd9ff4ce76b242307a7df2f9f365a1a7f40e708af6529',
    (6, 'Map_20051_3'): '7470f719d576f3a9fdbf41015384bd313fe789e8d5bd6a28057199427a367e33',
}


def manifest():
    result = []
    cases = json.loads((ARCHIVE / 'contexts.json').read_text(encoding='utf-8'))['cases']
    for game in (5, 6):
        scripts = {s['submap']: s for s in map(json.loads, (ROOT.parent / f'tools/mapdump/ff{game}_scripts.jsonl').read_text(encoding='utf-8').splitlines())
                   if s['name'].lower() == 'sc_' + s['submap'].lower()}
        assets = json.loads((ARCHIVE / f'ff{game}_route_assets.json').read_text(encoding='utf-8'))['assets']
        for case in cases:
            submap = case['submap']
            if case['game'] != game or (game, submap) not in PINNED:
                continue
            source = scripts[submap]
            digest = hashlib.sha256(json.dumps(source, ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
            assert digest == PINNED[game, submap]
            assignments = {(m['operands']['iValues'][1], bool(m['operands']['iValues'][2])) for m in source['mnemonics']
                           if m['mnemonic'] == 'AssignMoveRoute' and m['operands']['iValues'][0] == case['id']}
            if not assignments:
                assert game==6 and submap=='Map_20051_3' and case['id'] in (12,14,15)
                properties=case['source']['props'];region,=case['movement_regions']
                assert properties['move_type'] in (2,3) and properties['move_step']==1
                assert properties['link_object_id']==region['props']['object_id']
                assert region['layer']==case['source']['layer']=='UpperObject'
                # Native B31EF0/5E9B00 centers then bounds the rectangle; B32C10
                # expands16-unit line segments relative to their actual entity.
                # The original parserCDDB80 establishes the same cropped origin.
                if properties['move_type']==2:
                    assert region['w']==16 and region['h']==64
                    cells=[[2,region['tx']-case['bounds'][0],y-case['bounds'][1]]
                           for y in range(region['ty'],region['ty']+region['h']//16)]
                else:
                    points=region['polyline'];assert len(points)==2 and points[0]['x']==points[1]['x']==0
                    distance=points[1]['y']-points[0]['y'];assert distance==48
                    cells=[[2,math.floor(region['x']/16)-case['bounds'][0],
                            math.floor((region['y']+points[0]['y']+offset)/16)-case['bounds'][1]] for offset in range(0,distance+1,16)]
                assert cells==[[2,case['baked'][2],y]for y in range(12,16)]
                result.append(dict(game='ff6',submap=submap,id=case['id'],group=case['group'],raw=case['raw'],home=case['baked'][1:4],
                                   route=0,loop=True,cells=cells,source_sha256=digest,
                                   asset_sha256=hashlib.sha256(json.dumps(dict(actor=case['source'],region=region),ensure_ascii=False,sort_keys=True,separators=(',',':')).encode()).hexdigest()))
            for route, loop in sorted(assignments):
                asset, = [v for k, v in assets.items() if k.endswith(f'/{submap}/route_instruction_{route}.json')]
                instructions = asset['body']['instruction_list']
                # No hidden actor/conditional native trajectory is certified by
                # a waypoint list. These exact assets have no such instructions.
                assert all(m['instruction_type'] in (1, 2, 3, 4, 7, 8, 9) for m in instructions)
                cells = [[m['layer'], m['cellx'] - case['bounds'][0], m['celly'] - case['bounds'][1]]
                         for m in instructions if m['instruction_type'] in (1, 7)]
                if not loop:
                    cells = cells[-1:]  # settled one-shot endpoint, not a transient pose
                cells = [list(c) for c in dict.fromkeys(map(tuple, cells))]
                assert 0 < len(cells) <= 32
                result.append(dict(game=f'ff{game}', submap=submap, id=case['id'], group=case['group'], raw=case['raw'],
                                   home=case['baked'][1:4], route=route, loop=loop, cells=cells, source_sha256=digest,
                                   asset_sha256=asset['text_sha256']))
    assert len(result) == 14
    return result


if __name__ == '__main__':
    import sys
    target = ROOT / 'tools/npc_route_positions.json'
    data = manifest()
    if '--write' in sys.argv:
        target.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    else:
        assert json.loads(target.read_text(encoding='utf-8')) == data
    print(f'Fourteen pinned NPC patrols: {sum(len(row["cells"]) for row in data)} native positions; original bodies remain outside Git')
