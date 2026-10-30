"""Bounded all-six source inventory for the active collision-effect audit."""
from pathlib import Path
from collections import Counter
import hashlib,json,subprocess
sys_path=__import__('sys').path
sys_path.insert(0,str(Path(__file__).resolve().parents[1]))
import bake_maps

root=Path(__file__).resolve().parents[2]
dump=root.parent/'tools/mapdump'
out=root.parent/'tools/map_event_audit_20261028/effect_inventory'
out.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
result={};cases=[]
for n in range(1,7):
    source=dump/f'ff{n}_objects.jsonl'
    objects=[json.loads(line)for line in source.read_text(encoding='utf-8-sig').splitlines()if line.strip()]
    effects=[row for row in objects if row['type_name']=='EffectEntity']
    by_sub={}
    for row in objects:by_sub.setdefault(row['submap'],[]).append(row)
    submaps=json.loads((dump/f'ff{n}_submaps.json').read_text())
    for row in effects:
        if row['props'].get('collision')is not True:continue
        ox,oy,_,_=bake_maps.map_bounds(submaps[row['submap']],by_sub[row['submap']])
        cases.append(dict(game=n,submap=row['submap'],group=row['group'],id=row['props']['entity_id'],
                          raw=row['name'],layer=bake_maps.layer_index(row['layer']),
                          x=row['tx']-ox,y=bake_maps.tile_row(row)-oy,
                          width=max(1,(row['w']+15)//16),height=max(1,(row['h']+15)//16)))
    result[str(n)]={'objects_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'effects':effects}
    print(f'FF{n}: {len(effects)} effects, {sum(row["props"].get("collision")is True for row in effects)} collision-enabled')
    for row in effects:
        if row['props'].get('collision')is True and row['submap']=='Map_20060':
            print(row['submap'],row.get('group'),row.get('tx'),row.get('ty'),row.get('tw'),row.get('th'),
                  {key:row['props'].get(key)for key in ('entity_id','script_id','action_id','collision','size','asset_name','layer')})
(out/'facts.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(out/'collision_cases.json').write_text(json.dumps(cases,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
