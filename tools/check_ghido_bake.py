"""Prove the shore correction adds one graph link/landing and preserves native data."""
from pathlib import Path
from collections import Counter
import hashlib,json,sys
from check_scripted_bakes import decode

before,after,report=map(Path,sys.argv[1:]);old,masters=decode(before);new,new_masters=decode(after)
assert masters==new_masters and old.keys()==new.keys()
changed=[]
for name,room in old.items():
    current=new[name]
    assert all(room[k]==current[k]for k in ('shape','grids','transport')),(name,'original geometry changed')
    removed=Counter(room['objects'])-Counter(current['objects']);added=Counter(current['objects'])-Counter(room['objects'])
    assert not removed,(name,'original object removed or modified')
    if not added:continue
    assert name=='Map_30381_7'and sum(added.values())==2
    objects=list(added);landing=next(o for o in objects if o[0]==0);warp=next(o for o in objects if o[0]==100)
    assert landing[1:6]==(1,12,32,1,1)and landing[6]<0 and landing[8]==-1
    assert warp[1:7]==(1,12,35,1,1,-1)and warp[7]==landing[6]and warp[8:10]==(10,1)
    original=next(o for o in room['objects']if o[0]==2 and o[8]==10 and o[10:]==warp[10:])
    assert original[1:6]==warp[1:6]
    changed.append(dict(submap=name,entity=10,landing_point=landing[6],landing=[1,12,32],trigger=[1,12,35]))
assert len(changed)==1
result=dict(sha256=hashlib.sha256(after.read_bytes()).hexdigest(),bytes=after.stat().st_size,changes=changed,
            original_objects_arcs_terrain_transport_masters_preserved=True)
report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print('FFV shore: one same-map link/landing added; every original object, arc, terrain, transport and master preserved')
