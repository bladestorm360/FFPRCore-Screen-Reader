"""One exact quicksand fall,full native ready landing and trap classification."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
def main():
 proof=json.loads((R/'docs/re/ff5_pyramid_native_20261104.json').read_text(encoding='utf-8'));wanted={('Map_30461_1','sc_e_0290'),('Map_30461_2','sc_e_0290_1')};scripts={}
 for line in(R.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap'],s['name']
  if key in wanted:
   assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes']['/'.join(key)];scripts[key]=s
 assert len(scripts)==2;s=scripts['Map_30461_2','sc_e_0290_1'];m=s['mnemonics'][36]
 assert m['mnemonic']=='SetPos'and m['operands']['iValues'][:3]==[8,17,2]
 row=dict(submap='Map_30461_1',entity_id=19,destination_map=412,original_point=1,continuation=s['name'],continuation_sha256=hashlib.sha256(json.dumps(s,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest(),arrival_cell=[8,17,1],arrival_baked_cell=[8,17,1],trap_transfer=True,evidence='Full quicksand/fall callback ends at8,17 Ground after bound animation,not the native stair PointIn1;docs/re/FF5_PYRAMID_20261104.md')
 p=R/'data/ff5/warp_arrival_overrides.json';old=json.loads(p.read_text(encoding='utf-8'));new=[dict(r)for r in old];matches=[i for i,r in enumerate(new)if r['submap']==row['submap']and r['entity_id']==19]
 if matches:i,=matches;new[i]=row
 else:new.append(row)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('One source-qualified Pyramid fall:8,17 Ground,trap semantics;all prior arrival indices preserved.')
if __name__=='__main__':main()
