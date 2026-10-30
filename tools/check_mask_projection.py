"""Verify the exact native visual-call delta without storing game source."""
from pathlib import Path
import hashlib,json

ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'build_check/mask_projection_20261029'
sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
before=json.loads((BASE/'before_hashes.json').read_text())
for n in range(1,7):
    assert sha(ROOT/f'data/ff{n}/maps.bin')==before[str(n)]['bake_sha256']
    if n!=6:assert sha(ROOT/f'data/ff{n}/mapstate.json')==before[str(n)]['state_sha256']
old=json.loads((BASE/'before/ff6_mapstate.json').read_text(encoding='utf-8'))
new=json.loads((ROOT/'data/ff6/mapstate.json').read_text(encoding='utf-8'))
assert old.keys()==new.keys()
changes=[]
for name,record in old.items():
    current=new[name]
    assert record.keys()==current.keys()
    assert all(value==current[key]for key,value in record.items()if key!='code')
    assert len(record['code'])==len(current['code'])
    for index,(a,b)in enumerate(zip(record['code'],current['code'])):
        if a!=b:
            assert name=='Map_30410'and a==[9]and b==[0]
            changes.append(index)
assert changes==[4,8]
source=None
for line in (ROOT.parent/'tools/mapdump/ff6_scripts.jsonl').open(encoding='utf-8-sig'):
    if 'sc_map_30410'not in line:continue
    item=json.loads(line)
    if (item['submap'],item['name'])==('Map_30410','sc_map_30410'):
        assert source is None;source=item
assert source
for index,expected in ((4,'帝国軍陣地マスクなし'),(8,'帝国軍陣地マスクあり')):
    instruction=source['mnemonics'][index]
    assert instruction['mnemonic']=='SysCall'and instruction['operands']['sValues'][0]==expected
result=dict(sha256=sha(ROOT/'data/ff6/mapstate.json'),records=len(new),
            changed_program='Map_30410',changed_indices=changes,all_other_metadata_and_all_six_bakes_preserved=True)
(BASE/'delta.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print('FFVI visual projection: exactly2 reviewed calls changed; every other record/field and all6 bakes preserved')
