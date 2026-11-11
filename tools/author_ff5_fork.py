"""Original copied-state twin-tower guide and source-qualified scene arc."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps,fork_exits
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
read=lambda p:json.loads(p.read_text(encoding='utf-8'))
PREP='Prepare magic users for the left tower and physical attackers for the right tower.'
MAGIC='Before crossing, prepare this character to use magic.'
PHYSICAL='Before crossing, prepare this character for physical attacks.'
RIGHT='Win this battle with physical attacks. Do not use magic.'
LEFT='Take the spell immediately, then win the battle with magic.'
CROSS='Cross this trigger, then request guidance again.'
def author():
 masters=list(csv.DictReader((D/'ff5_master/map.txt').open(encoding='utf-8-sig')))
 selected=[r for r in masters if r['area_id']in ('66','67','68')];rooms={r['map_name']for r in selected};scripts={};objects=[]
 for line in (D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']in rooms or (s['submap'],s['name'])in {('Map_10040','sc_e_0444_3'),('Resident','sc_e_0445')}:scripts[s['submap'],s['name']]=s
 for line in (D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects.append(o)
 hashes={room+'/'+name:hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()for(room,name),s in scripts.items()}
 assert len(hashes)==47 and 'Map_10040/sc_e_0444_3'in hashes and 'Resident/sc_e_0445'in hashes
 infos=read(D/'ff5_submaps.json');bounds={room:bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])for room in rooms}
 def target(room,id,group,kind,text):
  o,=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==id and o['group']==group and o['type_name']in ('Event','Entity')]
  ox,oy,*_=bounds[room];mid=next(int(r['id'])for r in selected if r['map_name']==room)
  return dict(map=mid,entity=id,raw=o['name'],group=group,layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,kind=kind,text=text)
 targets=dict(start=target('Map_30481_1',1,'ev_e_0143','spot',CROSS),left=target('Map_30481_1',10,'entity_default','spot',CROSS),right=target('Map_30481_1',11,'entity_default','spot',CROSS),swap=target('Map_30490',1,'ev_e_0144','spot',CROSS),white=target('Map_30501_8',1,'entity_default','interact','Interact, then request guidance again.'),black=target('Map_30491_8',1,'entity_default','interact','Interact, then request guidance again.'))
 targets.update(swap648=target('Map_30490_1',1,'ev_e_0144','spot',CROSS),swap649=target('Map_30490_2',1,'ev_e_0144','spot',CROSS))
 guide=dict(id='fork_tower',maps=sorted(int(r['id'])for r in selected),select_when=[['scenario',145,'ge',0]],complete_scenario=145,targets=targets,texts=[PREP,MAGIC,PHYSICAL,RIGHT,LEFT],evidence='docs/re/FF5_FORK_TOWER_20261105.md',source_script_hashes=hashes)
 # These are exact native receipts, not an invented puzzle step counter.
 for name,count in [('sc_e_0143',176),('sc_e_0143_1',188),('sc_e_0143_2',188),('sc_e_0143_6',26)]:assert len(scripts['Map_30481_1',name]['mnemonics'])==count
 assert scripts['Map_30491_8','sc_e_0444_1']['mnemonics'][47]['operands']['iValues'][0]==145
 assert scripts['Map_30491_8','sc_e_0145_4']['mnemonics'][29]['operands']['rValues'][0]==10
 actor,=[o for o in objects if o['submap']=='Map_30481_1'and o['group']=='ev_e_0143'and o['props'].get('entity_id')==1 and o['type_name']=='Event']
 assert fork_exits.flags('ff5',actor,scripts['Map_30481_1','sc_e_0143'])==fork_exits.TAG|(1<<16)
 guard=dict(id=1,type=2,group=actor['group'],raw=actor['name'],cell=[1,13,18],map_ids=[443],area_id=66,entry=0,require_known=True,warp_mask=[fork_exits.TAG,fork_exits.TAG],destination=[19,33],code=[[2,0,143,0,-1,1,2],[6],[1,'@action'],[6]],review='Initial split stays on443. Only the already-split regroup confirmation can reach19/33. Original interaction remains reachable. docs/re/FF5_FORK_TOWER_20261105.md')
 gates=[dict(id=o['props']['entity_id'],type=2,group=o['group'],raw=o['name'],cell=[1,o['tx'],bake_maps.tile_row(o)],size=[1,1],review='Native pre142 initializer selects pushback;full controlled-player Lerp returns south to row20. Contact remains selectable;onward travel is gated. docs/re/FF5_FORK_TOWER_20261105.md')for o in sorted(objects,key=lambda o:o['props'].get('entity_id',-1))if o['submap']=='Map_30481_1'and o['group']=='ev_e_0142'and o['type_name']=='Event']
 assert [r['id']for r in gates]==[28,29,30,31,32]
 existing=R/'docs/re/ff5_fork_native_20261105.json'
 battle=read(existing)['battle_assets']if existing.exists()else read(R/'build_check/broader_audit_20261104/ff5_ForkTower/battle_facts.json')
 facts=dict(scope_tag='ff5_ForkTower',source_script_hashes=hashes,maps=[dict(id=int(r['id']),room=r['map_name'],area=int(r['area_id']))for r in selected],battle_assets=battle,position_native=read(R.parent/'tools/ff5_library_20261101/puppet_position_provenance.json'))
 return guide,guard,facts,gates
def main():
 check='--check'in sys.argv;guide,guard,proof,gates=author();p=R/'data/ff5/puzzles.json';d=read(p);old=d['story_guides'];rows=[guide if g['id']==guide['id']else g for g in old]
 if not any(g['id']==guide['id']for g in old):rows.append(guide)
 state=read(R/'data/ff5/mapstate.json');events=state['Map_30481_1'].get('events',[]);same=[r for r in events if r.get('warp_mask')==guard['warp_mask']]
 proof_path=R/'docs/re/ff5_fork_native_20261105.json'
 if check:assert old==rows and same==[guard]and read(proof_path)==proof and state['Map_30481_1'].get('story_actor_gates')==gates
 else:
  d['story_guides']=rows
  from author_puzzle_types import expected
  d['guidance_types']=expected(5,d);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
  assert not same or same==[guard]
  if not same:state['Map_30481_1']['events']=events+[guard]
  state['Map_30481_1']['story_actor_gates']=gates;(R/'data/ff5/mapstate.json').write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
  proof_path.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(d,check);print('Fork:source-qualified assignments,paired tower encounters,timed final spell and full native return;ordinary setup arc guarded.')
if __name__=='__main__':main()
