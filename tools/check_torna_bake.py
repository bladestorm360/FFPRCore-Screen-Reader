"""Prove the Torna correction changes one reviewed entry-policy bit only."""
from pathlib import Path
from collections import Counter
import hashlib,json,sys
from check_scripted_bakes import decode

before,after,report=map(Path,sys.argv[1:])
old,old_masters=decode(before);new,new_masters=decode(after)
assert old_masters==new_masters and old.keys()==new.keys()
changes=[]
for name,room in old.items():
    updated=new[name]
    assert all(room[key]==updated[key]for key in ('shape','grids','transport'))
    removed=Counter(room['objects'])-Counter(updated['objects'])
    added=Counter(updated['objects'])-Counter(room['objects'])
    if not removed and not added:continue
    assert name=='Map_10010' and sum(removed.values())==sum(added.values())==1
    a=next(iter(removed));b=next(iter(added))
    assert a[0]==b[0]==100 and a[6:9]==b[6:9]==(210,1,63)
    assert b==(*a[:9],a[9]|(1<<17),*a[10:]) and b[9]!=a[9]
    changes.append(dict(submap=name,entity=63,old_flags=a[9],new_flags=b[9]))
assert len(changes)==1
result=dict(changes=changes,sha256=hashlib.sha256(after.read_bytes()).hexdigest(),bytes=after.stat().st_size,
            original_terrain_objects_arcs_masters_preserved=True)
report.write_text(json.dumps(result,indent=2)+'\n')
print('Torna: one reviewed Ship-entry bit changed; every original object/arc/terrain/master preserved')
