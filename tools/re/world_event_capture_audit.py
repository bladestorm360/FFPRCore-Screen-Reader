"""Audit real IV/V/VI world-script identities against bake/publication pruning.

Read-only original analysis. Source-bearing facts remain outside every Git tree.
No tester logs, game execution or speculative native capture is used.
"""
from pathlib import Path
from collections import Counter
import hashlib,json,struct,subprocess,sys
root=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'tools'))
import bake_maps
from check_scripted_bakes import decode
dump=root.parent/'tools/mapdump';out=root.parent/'tools/world_event_capture_20261029'
out.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
result={}
for n in (4,5,6):
    rooms,masters=decode(root/f'data/ff{n}/maps.bin')
    groups={key:mask for key,mask in struct.iter_unpack('<iI',masters[3][0])}
    def expand(data):
        values=[]
        for at in range(0,len(data),3):
            count,value=struct.unpack_from('<HB',data,at);values.extend([value]*count)
        return values
    worlds={name for name,row in rooms.items()if row['shape'][3]&1}
    labels=json.loads((root/f'data/ff{n}/eventlabels.json').read_text())
    source=dump/f'ff{n}_objects.jsonl'
    objects=[json.loads(line)for line in source.read_text().splitlines()if line.strip()]
    submaps=json.loads((dump/f'ff{n}_submaps.json').read_text())
    by_sub={}
    for row in objects:by_sub.setdefault(row['submap'],[]).append(row)
    rows=[]
    for obj in objects:
        prop=obj['props']or{}
        if obj['submap']not in worlds or obj['type']not in (2,21,28,30)or not prop.get('script_id'):continue
        ox,oy,_,_=bake_maps.map_bounds(submaps[obj['submap']],by_sub[obj['submap']])
        x,y=obj['tx']-ox,bake_maps.tile_row(obj)-oy
        identifier=lambda row:row[10].split('|',1)[-1]if row[0]==21 else row[10]
        baked=[row for row in rooms[obj['submap']]['objects']if row[8]==prop.get('entity_id')and identifier(row)==obj['name']and row[11]==obj['group']and row[2:4]==(x,y)]
        label=labels.get(obj['name'])
        room=rooms[obj['submap']];width,height=room['shape'][1:3]
        terrain=expand(room['grids'][-1]);attribute=terrain[y*width+x]if 0<=x<width and 0<=y<height else 0
        ship_ids=[row[0] for row in masters[2]if row[3]==2 and attribute>0 and groups.get(row[1],0)&(1<<(attribute-1))and
                  not expand(room['transport'][row[1]-1])[y*width+x]and
                  (not prop.get('target_transportation_ids')or str(row[0])in prop['target_transportation_ids'].split(','))]
        rows.append(dict(submap=obj['submap'],map_id=room['shape'][0],group=obj['group'],raw=obj['name'],editor_id=obj['id'],
                         entity_id=prop['entity_id'],script=prop['script_id'],type=obj['type'],action=prop.get('action_id'),
                         label=label,suppressed=label in ('@route-only','World map transition'),baked=baked,
                         transportation_ids=prop.get('target_transportation_ids'),cell=[x,y],ship_ids=ship_ids,attribute=attribute))
    result[str(n)]={'objects_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'worlds':sorted(worlds),'rows':rows}
    print(f'FF{n}: {len(worlds)} actual world assets, {len(rows)} scripted events, {sum(row["suppressed"]for row in rows)} label-pruned')
    for row in rows:
        if row['suppressed']or not row['baked']or n==5 and row['entity_id']==63 and row['submap']=='Map_10010':
            print({key:row[key]for key in ('submap','entity_id','raw','script','label','suppressed')})
    scenes=[row for row in rows if row['type']==2 and row['ship_ids']and not any(obj[0]==100 and obj[9]&65536 for obj in row['baked'])]
    print(f'FF{n}: {len(scenes)} ordinary scripted world events stand on actual ship-reachable terrain')
(out/'inventory.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
