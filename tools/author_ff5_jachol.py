"""Exact Jachol controls,random switch poses and full native exit facts."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
OPERATE='Operate the switch.'
LIVE='Go now. Operate the skull switch.'
WAIT='Wait for the skull switch to appear.'
STAGE='Wait here for the skull switch.'
DOOR='Examine the chest. Choose the first option to open the passage.'
OPEN='Open the chest.'
CLIMB='Examine the wall to climb out.'
def author():
 p=R/'docs/re/ff5_jachol_native_20261104.json';proof=json.loads(p.read_text(encoding='utf-8'));assert proof['scope_tag']=='ff5_Jachol'and len(proof['source_script_hashes'])==14
 assert proof['switch_array']['values']==list(range(9,17))and proof['native_constants']==dict(animation_frame=1.0,tile=16.0,instant_delay=0.01666666753590107,active_seconds=3.0,hidden_seconds=6.0)
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][k]
 maps={int(r['id']):r for r in csv.DictReader((D/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[i]['map_name']for i in(284,285)};objects=[]
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects.append(o)
 infos=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'));bounds={r:bake_maps.map_bounds(infos[r],[o for o in objects if o['submap']==r])for r in rooms}
 def target(mid,e,text):
  room=maps[mid]['map_name'];o,=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==e and o['type_name']in('Entity','TreasureBox','GotoMap')];ox,oy,*_=bounds[room]
  return dict(map=mid,entity=e,raw=o['name'],group=o['group'],x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,layer=bake_maps.layer_index(o['layer']),text=text)
 targets=dict(start_left=target(284,25,OPERATE),start_right=target(284,26,OPERATE),skull=target(284,8,LIVE),chest_door=target(284,6,DOOR),treasure_a=target(285,11,OPEN),treasure_b=target(285,5,OPEN),treasure_c=target(285,6,OPEN),climb=target(285,10,CLIMB))
 room=maps[285]['map_name'];o,=[o for o in objects if o['submap']==room and o['type_name']=='PointIn'and o['props'].get('object_id')==2];ox,oy,*_=bounds[room];targets['second_floor']=dict(map=285,x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,layer=bake_maps.layer_index(o['layer']),text='Continue through the passage.')
 room=maps[284]['map_name'];ox,oy,*_=bounds[room];poses=[]
 for e in proof['switch_array']['values']:
  o,=[o for o in objects if o['submap']==room and o['type_name']=='TileAnimation'and o['props'].get('entity_id')==e];assert(o['tx'],o['ty'])==(23+2*(e-9),27)
  # Only the native random column is used to bound a short timing leg. Its
  # actual live transform,not an inferred rectangle anchor,owns the route pose.
  poses.append(o['tx']-ox)
 targets['staging']=dict(map=284,x=30-ox,y=28-oy,layer=1,text=STAGE)
 return dict(id='jachol_cave',maps=[284,285],targets=targets,skull_columns=poses,source_script_hashes=proof['source_script_hashes'],native_functions=proof['native_functions'],evidence='docs/re/FF5_JACHOL_GUIDE_20261104.md')
if __name__=='__main__':
 wanted=author();p=R/'data/ff5/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert d.get('jachol_guide')==wanted
 else:d['jachol_guide']=wanted;p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Jachol ten exact targets/eight native random columns;live pose owns routing;three-second active/six-second hidden cycle;full native wall exit.')
