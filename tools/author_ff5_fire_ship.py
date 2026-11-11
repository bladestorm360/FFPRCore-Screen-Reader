"""Original offline passage policy;no native programs enter shipped resources."""
from pathlib import Path
import hashlib,json,subprocess,sys
import bake_maps
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
FACE='Face {0}. Operate the switch.'
def author():
 proof=read(R/'docs/re/ff5_fire_ship_native_20261105.json');actual={};objects={r:[]for r in('Map_30151_19','Map_30151_20')}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap']+'/'+s['name']
  if key in proof['source_script_hashes']:actual[key]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert actual==proof['source_script_hashes']
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in objects:objects[o['submap']].append(o)
 info=read(D/'ff5_submaps.json')
 def target(room,id,typ):
  o,=[o for o in objects[room]if o['type_name']==typ and o['props']['entity_id']==id];ox,oy,*_=bake_maps.map_bounds(info[room],objects[room])
  return dict(entity=id,raw=o['name'],layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy)
 controls=[target('Map_30151_20',id,'Entity')for id in(21,19,20,23,24,26,25,22)]
 for t in controls:
  o,=[o for o in objects['Map_30151_20']if o['props'].get('entity_id')==t['entity']and o['type_name']=='Entity'];assert o['props']['collision']and o['props']['accept_action_direction']==15 and t['layer']==1
 policy=read(R/'build_check/ff5_fire_ship_policy.json');rows=policy['rows'];assert len(rows)==128 and all(len(s)==675 and set(s)<=set('.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh')for s in rows)
 assert policy['states']<=4096 and policy['edges']<=65536 and policy['comparisons']==5376
 return dict(id='fire_ship_passages',maps=[270],transit_maps=[269],controls=controls,exit=target('Map_30151_20',16,'GotoMap'),reset_exit=target('Map_30151_20',15,'GotoMap'),return_exit=target('Map_30151_19',6,'GotoMap'),rows=rows,source_script_hashes=actual,evidence='docs/re/FF5_FIRE_SHIP_20261105.md')
def main():
 # Canonical builds compile checks before producers. Regenerate the original
 # policy from the current source-qualified comparison data;never certify a
 # stale build_check table merely because an older investigation left it here.
 with(R/'build_check/fire_ship_policy_author_report.txt').open('wb')as out:subprocess.run([str(R/'out/checks/ff5_fire_ship_policy_check.exe')],cwd=R,stdout=out,stderr=subprocess.STDOUT,timeout=90,check=True)
 p=R/'data/ff5/puzzles.json';data=read(p);want=author();check='--check'in sys.argv
 if check:assert data.get('fire_ship_guide')==want
 else:data['fire_ship_guide']=want;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(data,check);print('Ship:complete eight-control policy,exact facing and actual exit/reset/return;earlier guides preserved.')
if __name__=='__main__':main()
