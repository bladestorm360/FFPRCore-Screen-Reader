"""Write/check the one exact passage initializer;all other rooms stay intact."""
from pathlib import Path
import json,sys
import bake_map_state as b
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
def main():
 room='Map_30151_20';objects=[];source=None
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']==room:objects.append(o)
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']==room and s['name']=='sc_map_30151_20':source=s
 assert source
 region=b.bake_maps.map_bounds(json.loads((D/'ff5_submaps.json').read_text())[room],objects)
 code=b.initializer_code(source,'ff5',region);p=R/'data/ff5/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));old=data[room]['code']
 assert len(old)==len(code)==436
 for a,c in zip(old,code):
  if a!=c:assert a[:4]==c[:4]and a[0]==10 and a[2]in(52,53,56,57,58,824)and a[4:]==[-2147483648]*2
 if '--check'in sys.argv:assert old==code
 else:data[room]['code']=code;p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Ship:one initializer/six physical positions;all existing PCs,visibility,branches and other rooms preserved.')
if __name__=='__main__':main()
