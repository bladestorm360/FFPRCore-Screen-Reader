"""Write/check only the two source-qualified Jachol initializer programs."""
from pathlib import Path
import json,sys
import bake_map_state as b
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
def main():
 wanted={};objects={room:[]for room in('Map_30161_1','Map_30161_2')}
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in objects:objects[o['submap']].append(o)
 infos=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'))
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']in objects and s['name']=='sc_map_'+s['submap'][4:]:wanted[s['submap']]=b.initializer_code(s,'ff5',b.bake_maps.map_bounds(infos[s['submap']],objects[s['submap']]))
 assert len(wanted)==2
 p=R/'data/ff5/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'))
 for room,code in wanted.items():
  old=data[room]['code']
  if '--check'in sys.argv:assert old==code
  else:
   allowed=(len(old)==445 and old[50]==[9]and old[:50]==code[:50]and old[51:]==code[51:445])if room=='Map_30161_1'else(len(old)==29 and old[1]==[9]and old[:1]==code[:1]and old[2:]==code[2:])
   assert old==code or allowed;data[room]['code']=code
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Jachol two exact initializer programs:wall/chest facts preserved;only future real skull visibility remains unknown;no geometry/VM bound change.')
if __name__=='__main__':main()
