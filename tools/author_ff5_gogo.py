"""Full sunken-tower air/Gogo/return solution from exact native sources."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
AIR='Take the air supply to reset the timer.'
GOGO='Disable Auto-Battle. Examine the crystal and choose the second option. In battle, keep the main command menu open and take no actions until the enemy leaves.'
LEAVE='Leave the tower before the timer expires.'
def author():
 proof=json.loads((R/'docs/re/ff5_gogo_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff5_Gogo'and len(proof['source_script_hashes'])==47 and len(proof['actors'])==59
 actual={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:actual[k]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert actual==proof['source_script_hashes']
 maps={int(r['id']):r['map_name']for r in csv.DictReader((D/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[490],maps[494],maps[485]};objects={r:[]for r in rooms}
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects[o['submap']].append(o)
 infos=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'))
 def target(mid,e,group,typ,text):
  room=maps[mid];o,=[o for o in objects[room]if o['props'].get('entity_id')==e and o['group']==group and o['type_name']==typ];ox,oy,*_=bake_maps.map_bounds(infos[room],objects[room])
  return dict(map=mid,entity=e,raw=o['name'],group=group,layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,kind='transition'if typ=='GotoMap'else'interact',text=text)
 f=lambda bit,n:['scenario',bit,'eq',n]
 return dict(id='gogo_sunken_tower',maps=list(range(485,495)),select_when=[['scenario',166,'ge',0]],complete_scenario=166,return_map=622,return_target=target(485,2,'entity_default','GotoMap',LEAVE),rules=[dict(when=[f(347,0)],targets=[target(490,3,'ev_e_0333','Entity',AIR)]),dict(when=[f(347,1)],targets=[target(494,8,'ev_e_0166','Entity',GOGO)])],source_script_hashes=proof['source_script_hashes'],evidence='docs/re/FF5_GOGO_20261104.md')
if __name__=='__main__':
 p=R/'data/ff5/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'));old=data['story_guides'];wanted=author();new=[wanted if g['id']=='gogo_sunken_tower'else g for g in old];check='--check'in sys.argv
 if not any(g['id']=='gogo_sunken_tower'for g in old):new.append(wanted)
 if check:assert old==new
 else:data['story_guides']=new;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(data,check);print('Gogo:full tower air/reset,correct second choice,inaction battle and actual native exit;earlier guides preserved.')
