"""Verify the collision-only effect delta without changing native map geometry."""
from pathlib import Path
from collections import Counter
import hashlib,json,sys
from check_scripted_bakes import decode

before=Path(sys.argv[1]);after=Path(sys.argv[2]);report=Path(sys.argv[3])
cases=json.loads((Path(__file__).resolve().parents[2]/'tools/map_event_audit_20261028/effect_inventory/collision_cases.json').read_text())
result={}
for n in range(1,7):
    old_path=before/f'ff{n}/maps.bin';new_path=after/f'ff{n}/maps.bin'
    old,old_masters=decode(old_path);new,new_masters=decode(new_path)
    assert old_masters==new_masters and old.keys()==new.keys(),('Changed masters/submaps',n)
    assert (before/f'ff{n}/mapstate.json').read_bytes()==(after/f'ff{n}/mapstate.json').read_bytes(),('Unreviewed state metadata change',n)
    added=[]
    for name,room in old.items():
        updated=new[name]
        assert all(room[key]==updated[key]for key in ('shape','grids','transport')),('Changed native terrain/collision/layers',n,name)
        delta=Counter(updated['objects'])-Counter(room['objects'])
        assert not Counter(room['objects'])-Counter(updated['objects']),('Existing object/arc changed',n,name)
        for obj,count in delta.items():
            assert count==1 and obj[0]==11 and obj[6]==obj[7]==0 and obj[9]==1,('Unexpected new object',n,name,obj)
            source=[row for row in cases if row['game']==n and row['submap']==name and row['id']==obj[8]and row['raw']==obj[10]and row['group']==obj[11]and
                    (row['layer'],row['x'],row['y'],row['width'],row['height'])==obj[1:6]]
            assert len(source)==1,('Effect lacks exact collision-enabled source',n,name,obj)
            assert obj[4:6]==(1,1),('New size needs a separate native anchor review',n,name,obj)
            added.append([name,obj[8],obj[10],obj[11],obj[1:6]])
    expected={3:4,4:1,5:96}.get(n,0)
    assert len(added)==expected,(n,len(added),expected)
    if not expected:assert old_path.read_bytes()==new_path.read_bytes(),('Unchanged game bake differs',n)
    result[str(n)]={'added':added,'sha256':hashlib.sha256(new_path.read_bytes()).hexdigest(),'bytes':new_path.stat().st_size}
    print(f'FF{n}: {len(added)} exact solid effects added; all original objects/arcs/geometry/masters/state unchanged')
report.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
