"""Exact physical metadata for a narrow map/script selection; no native bodies."""
import csv,json,sys
from pathlib import Path
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
game=sys.argv[1];map_id=int(sys.argv[2]);prefixes=sys.argv[3:]
maps={int(r['id']):r for r in csv.DictReader((DUMP/f'{game}_master/map.txt').open(encoding='utf-8-sig'))}
names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/f'{game}_master/script.txt').open(encoding='utf-8-sig'))}
room=maps[map_id]['map_name']
objects=[o for line in (DUMP/f'{game}_objects.jsonl').open(encoding='utf-8') if (o:=json.loads(line))['submap']==room]
info=json.loads((DUMP/f'{game}_submaps.json').read_text(encoding='utf-8'))[room]
ox,oy,*_=bake_maps.map_bounds(info,objects)
for o in objects:
    p=o['props'];script=names.get(p.get('script_id',0),'')
    if prefixes and not any(script.startswith(s) or s=='type:'+o['type_name'] or s=='entity:'+str(p.get('entity_id')) for s in prefixes):continue
    if o['type_name'] not in {'Entity','AnimEntity','Event','SwitchEvent','RandomEvent','GotoMap','PointIn','TreasureBox','NPC'}:continue
    print(json.dumps(dict(map=map_id,entity=p.get('entity_id'),raw=o['name'],type=o['type_name'],group=o['group'],script=script,
        cell=[bake_maps.layer_index(o['layer']),o['tx']-ox,bake_maps.tile_row(o)-oy],
        size=[max(1,bake_maps.to_int(o.get('w'))//16),max(1,bake_maps.to_int(o.get('h'))//16)],
        properties={k:v for k,v in p.items() if k in {'entity_id','object_id','script_id','leave_script_id','action_id','collision','destination_map_id','point_id','treasure_id','flag_id','treasure_flag','accept_action_direction'}}),ensure_ascii=False))
