"""Prove Istory adds one exact manual graph edge/landing, with no native rewrite."""
from pathlib import Path
from collections import Counter
import hashlib,json,sys
from check_scripted_bakes import decode

before,after,report=map(Path,sys.argv[1:])
old,masters=decode(before);new,new_masters=decode(after)
assert masters==new_masters and old.keys()==new.keys()
changes=[]
for name,room in old.items():
    current=new[name]
    assert all(room[key]==current[key]for key in ('shape','grids','transport'))
    removed=Counter(room['objects'])-Counter(current['objects']);added=Counter(current['objects'])-Counter(room['objects'])
    assert not removed,(name,'original actor/arc changed')
    if not added:continue
    assert name=='Map_20090'and sum(added.values())==2
    objects=list(added);landing=next(o for o in objects if o[0]==0);warp=next(o for o in objects if o[0]==100)
    assert landing[1:6]==(1,33,19,1,1)and landing[6]<0 and landing[8]==-1
    assert warp[1:7]==(1,36,19,1,1,-1)and warp[7]==landing[6]and warp[8:10]==(21,2|(8<<8))
    original=next(o for o in room['objects']if o[0]==6 and o[8]==21 and o[10:]==warp[10:])
    assert original[1:6]==warp[1:6]and original[7]==14
    changes.append(dict(submap=name,entity=21,landing_point=landing[6],landing=[1,33,19],trigger=[1,36,19],approach_mask=8))
assert len(changes)==1
result=dict(sha256=hashlib.sha256(after.read_bytes()).hexdigest(),bytes=after.stat().st_size,changes=changes,
            original_npcs_objects_arcs_terrain_transport_masters_preserved=True)
report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print('Istory: one exact manual edge/landing; every original NPC, object, arc, terrain, transport and master preserved')
