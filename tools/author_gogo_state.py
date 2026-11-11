"""Write/check only the two source-qualified air/Gogo initializer programs."""
from pathlib import Path
import json,sys
import bake_map_state as b
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
def main():
 objects={room:[]for room in('Map_30531_6','Map_30531_10')};wanted={}
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in objects:objects[o['submap']].append(o)
 infos=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'))
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']in objects and s['name']=='sc_map_'+s['submap'][4:]:wanted[s['submap']]=b.initializer_code(s,'ff5',b.bake_maps.map_bounds(infos[s['submap']],objects[s['submap']]))
 assert len(wanted)==2;p=R/'data/ff5/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'))
 for room,code in wanted.items():
  old=d[room]['code'];assert old==code or len(old)==len(code)and old[2]==[9]and code[2]==[0]and old[:2]==code[:2]and old[3:]==code[3:]
  if '--check'in sys.argv:assert old==code
  else:d[room]['code']=code
 if '--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Gogo:two exact BGM-only initializer corrections;air347 and encounter166 groups retain actual native eligibility;no geometry change.')
if __name__=='__main__':main()
