"""Source-qualified timer registrations; preserve every unrelated state field."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
def expected():
 import author_ff6_story_guides as o,bake_maps,bake_map_state
 infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));rows={}
 for room in('Map_30531_2','Map_30531_3','Map_30531_4'):
  region=bake_maps.map_bounds(infos[room],[a for a in o.OBJECTS if a['submap']==room])
  rows[room]=bake_map_state.initializer_code(o.SCRIPTS[room,'sc_'+room.lower()],'ff6',region)
 return rows
if __name__=='__main__':
 p=ROOT/'data/ff6/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'));changes={}
 for room,code in expected().items():
  old=d[room]['code'];assert len(old)==len(code)
  changes[room]=[i for i,(a,b)in enumerate(zip(old,code))if a!=b]
  for i in changes[room]:assert old[i]==[9] and code[i]==[0],(room,i,old[i],code[i])
  if'--check'in sys.argv:assert old==code,room
  else:d[room]['code']=code
 if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Sealed Gate exact initializer registration rows:',changes)
