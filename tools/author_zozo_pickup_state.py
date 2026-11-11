"""Only the exact Zozo initializer's irrelevant party-visibility tail changes."""
from pathlib import Path
import json,sys
import bake_map_state as b
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
def main():
 proof=json.loads((R/'docs/re/zozo_jidoor_native_20261104.json').read_text(encoding='utf-8'));source=None;objects=[]
 with(D/'ff6_scripts.jsonl').open(encoding='utf-8')as f:
  for line in f:
   s=json.loads(line)
   if(s['submap'],s['name'])==('Map_30511_12','sc_map_30511_12'):source=s;break
 assert source and b.hashlib.sha256(json.dumps(source['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes']['Map_30511_12/sc_map_30511_12']
 with(D/'ff6_objects.jsonl').open(encoding='utf-8')as f:
  for line in f:
   o=json.loads(line)
   if o['submap']=='Map_30511_12':objects.append(o)
 info=json.loads((D/'ff6_submaps.json').read_text(encoding='utf-8'));region=b.bake_maps.map_bounds(info['Map_30511_12'],objects);wanted=b.initializer_code(source,'ff6',region)
 p=R/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));old=data['Map_30511_12']['code'];assert len(wanted)==134
 if '--check'in sys.argv:assert old==wanted
 else:
  assert old==wanted or len(old)==126 and old[28]==[9]and old[:28]==wanted[:28]and old[29:]==wanted[29:126]
  data['Map_30511_12']['code']=wanted;p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Zozo:four independent saved pickup groups preserved across the unrelated six-NPC party tail;no new query,VM opcode or runtime bound.')
if __name__=='__main__':main()
