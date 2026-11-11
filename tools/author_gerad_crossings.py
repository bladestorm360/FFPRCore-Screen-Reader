"""Two exact, manually operated turtle crossings; no game code is stored."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def authored(o):
 import bake_maps
 proof=json.loads((ROOT/'docs/re/gerad_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
 actor=o.physical(448,16,'entity_default');assert actor['id']==19 and actor['type_name']=='NPC'and actor['props']['script_id']==696 and actor['w']==actor['h']==16
 patrol,=[r for r in json.loads((ROOT/'tools/npc_route_positions.json').read_text(encoding='utf-8'))if r['game']=='ff6'and r['submap']==actor['submap']and r['id']==16]
 assert patrol==proof['turtle_patrol']and patrol['loop']and patrol['route']==306411 and patrol['cells']==[[1,9,10],[1,11,10],[1,7,10]]
 assert sha(o.SCRIPTS[actor['submap'],'sc_map_30641_1'])==patrol['source_sha256']
 root=o.SCRIPTS[actor['submap'],'sc_e_0668'];child=o.SCRIPTS[actor['submap'],'sc_e_0669']
 assert root['mnemonics'][1]['mnemonic']=='BranchByPlayerDirection'and root['mnemonics'][1]['operands']['iValues'][:4]==[2,2,5,5]
 def player_land(script,start):
  segment=script['mnemonics'][start:];lands=[]
  for m in segment:
   if m['mnemonic']=='Exit':break
   if m['mnemonic']=='Lerp':lands.append(m['operands']['iValues'][:3])
  return lands
 assert [v[:2]for v in player_land(root,13)]==[[47,27],[47,25]]
 assert [v[:2]for v in player_land(child,8)]==[[47,27],[47,29]]
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[actor['submap']]
 ox,oy,*_=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']==actor['submap']])
 assert(ox,oy)==(38,17)
 rows=[]
 for landing,mask in [([47,25,1],4),([47,29,1],1)]:
  rows.append(dict(submap=actor['submap'],entity_id=16,object_id=19,same_map=True,interaction_travel=True,approach_mask=mask,destination_map=448,original_point=0,continuation='sc_e_0668',continuation_sha256=sha(root),recurring_source_cell=[47,27,1],arrival_cell=landing,arrival_baked_cell=[landing[0]-ox,landing[1]-oy,1],evidence='Exact native facing branch, child0669, existing332-gated recurring route306411 and actual counter banks. Manual interaction only; docs/re/GERAD_PASSAGE_20261104.md'))
 guard=dict(id=16,type=100,group='entity_default',raw=actor['name'],cell=[1,9,10],map_ids=[448],area_id=57,entry=0,require_known=True,warp_mask=[1<<20,1<<20],code=[[2,1,76,0,-1,1,3],[6],[0],[1,'@action'],[6]],review='Only the derived recurring turtle arcs require native332; physical NPC/counters are unchanged. docs/re/GERAD_PASSAGE_20261104.md')
 # Keep the actual area identity from the unchanged initializer record.
 state=json.loads((ROOT/'data/ff6/mapstate.json').read_text(encoding='utf-8'));guard['area_id']=state[actor['submap']]['area_id']
 return rows,guard
if __name__=='__main__':
 import author_ff6_story_guides as o
 rows,guard=authored(o);p=ROOT/'data/ff6/warp_arrival_overrides.json';old=json.loads(p.read_text(encoding='utf-8'))
 existing=[r for r in old if r['submap']=='Map_30641_1'and r.get('object_id')==19]
 if '--check'in sys.argv:assert existing==rows
 else:
  assert not existing or existing==rows
  if not existing:old.extend(rows)
  p.write_text(json.dumps(old,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/mapstate.json';state=json.loads(p.read_text(encoding='utf-8'));events=state['Map_30641_1'].setdefault('events',[]);current=[r for r in events if r.get('type')==100 and r.get('id')==16]
 if '--check'in sys.argv:assert current==[guard]
 else:
  assert not current or current==[guard]
  if not current:events.append(guard)
  p.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Gerad: two exact recurring manual turtle arcs;332-only derived guard, real counters/layers and native actor retained.')

