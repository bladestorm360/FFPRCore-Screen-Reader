"""Exact factory variants, crate landing and five party-sensitive camp regions."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
 import bake_maps,bake_map_state,vector_passages,blackjack_passage
 blackjack_passage.validate(o.SCRIPTS)
 proof=json.loads((ROOT/'docs/re/imperial_travel_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
 submaps=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))
 def cell(actor):
  room=actor['submap'];ox,oy,*_=bake_maps.map_bounds(submaps[room],[a for a in o.OBJECTS if a['submap']==room])
  return[bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy],(ox,oy)
 state=json.loads((ROOT/'data/ff6/mapstate.json').read_text(encoding='utf-8'));guards=[]
 for entity in(28,29,30):
  actor=o.physical(143,entity,'entity_default');source=o.SCRIPTS['Map_20210','sc_e_1109'];at,_=cell(actor)
  assert o.SCRIPT_NAMES[actor['props']['script_id']]=='sc_e_1109'
  for target in vector_passages.variants(actor,source):
   guard=bake_map_state.entry_guard(source,'ff6',target[:2]);assert guard,target
   guard.update(destination=list(target[:2]),require_known=True,id=entity,type=actor['type'],group=actor['group'],raw=actor['name'],cell=at,source='sc_e_1109',map_ids=[143],area_id=state['Map_20210']['area_id']);guards.append(guard)
 barriers=[]
 for actor in o.OBJECTS:
  if actor['submap']!='Map_20240'or actor['type_name']not in('NPC','Event'):continue
  name=o.SCRIPT_NAMES.get(actor['props'].get('script_id'))
  if name not in('sc_e_0350','sc_e_0352','sc_e_0440','sc_e_0441'):continue
  assert actor['group']in('ev_e_0350','ev_e_0440','ev_e_0441');at,_=cell(actor)
  #0350/0352 victories still return outside;0440/0441 directly push back.
  end=name+'_1'if name in('sc_e_0350','sc_e_0352')else name
  transfers=proof['facts']['Map_20240/'+end]['transfers'];assert any(t[1:3]==[1,28]for t in transfers)
  barriers.append(dict(id=actor['props']['entity_id'],type=actor['type'],group=actor['group'],raw=actor['name'],cell=at,size=[max(1,(actor['w']+15)//16),max(1,(actor['h']+15)//16)],source=name,source_script_sha256=proof['source_script_hashes']['Map_20240/'+name]))
 assert barriers and len(barriers)<=32
 post=o.SCRIPTS['Map_20240','sc_map_20240'];ox,oy,*_=bake_maps.map_bounds(submaps['Map_20240'],[a for a in o.OBJECTS if a['submap']=='Map_20240'])
 post_code=bake_map_state.initializer_code(post,'ff6',(ox,oy,submaps['Map_20240'].get('width',0),submaps['Map_20240'].get('height',0)))
 actor=o.physical(143,60,'ev_e_0374');body=o.SCRIPTS['Map_20210','sc_e_0374'];ms=body['mnemonics'];at,(ox,oy)=cell(actor)
 assert ms[60]['mnemonic']=='SetPuppet'and ms[60]['operands']['iValues'][1:3]==[-1001,64]
 assert ms[72]['mnemonic']=='Lerp'and ms[72]['operands']['iValues'][:3]==[57,34,2]
 landing=dict(submap='Map_20210',entity_id=60,object_id=actor['id'],same_map=True,ordinary_travel=True,destination_map=143,original_point=0,continuation='sc_e_0374',continuation_sha256=sha(body),arrival_cell=[57,34,1],arrival_baked_cell=[57-ox,34-oy,1],evidence='Exact final player Lerp57,34/native layer2 after distraction/crate; native local1/523 group guards preserved. docs/re/IMPERIAL_TRAVEL_20261104.md')
 actor=o.physical(125,74,'ev_e_0426');at,_=cell(actor);body=o.SCRIPTS['Map_20180','sc_e_0426']['mnemonics']
 assert body[53]['mnemonic']==body[64]['mnemonic']=='Lerp'and body[53]['operands']['iValues'][:2]==[45,23]and body[64]['operands']['iValues'][:2]==[43,23]
 albrook=[dict(id=74,type=actor['type'],group=actor['group'],raw=actor['name'],cell=at,size=[max(1,(actor['w']+15)//16),max(1,(actor['h']+15)//16)],source='sc_e_0426',source_script_sha256=proof['source_script_hashes']['Map_20180/sc_e_0426'])]
 vector=[];body=o.SCRIPTS['Map_20210','sc_e_0375_1']['mnemonics'];assert body[20]['mnemonic']=='SetPos'and body[20]['operands']['iValues'][:2]==[38,47]
 for entity in(57,58,59):
  actor=o.physical(143,entity,'ev_e_0375');at,_=cell(actor)
  assert o.SCRIPT_NAMES[actor['props']['script_id']]=='sc_e_0375'
  vector.append(dict(id=entity,type=actor['type'],group=actor['group'],raw=actor['name'],cell=at,size=[1,1],source='sc_e_0375',source_script_sha256=proof['source_script_hashes']['Map_20210/sc_e_0375']))
 puppets={m['operands']['iValues'][1]for m in o.SCRIPTS['Map_20210','sc_e_0375']['mnemonics']if m['mnemonic']=='SetPuppet'};assert puppets=={70,71,72}
 for entity in(70,71,72):
  actor=o.physical(143,entity,'ev_e_0347');at,_=cell(actor)
  assert actor['type_name']=='NPC'and not actor['props'].get('script_id')
  vector.append(dict(id=entity,type=actor['type'],group=actor['group'],raw=actor['name'],cell=at,size=[max(1,(actor['w']+15)//16),max(1,(actor['h']+15)//16)],source='sc_e_0375',source_role='scripted_guard_puppet',source_script_sha256=proof['source_script_hashes']['Map_20210/sc_e_0375']))
 return guards,barriers,landing,post_code,albrook,vector
if __name__=='__main__':
 import author_ff6_story_guides as o
 guards,barriers,landing,post_code,albrook,vector=author(o);p=ROOT/'data/ff6/mapstate.json';state=json.loads(p.read_text(encoding='utf-8'))
 events=state['Map_20210'].setdefault('events',[]);old=[e for e in events if e.get('source')=='sc_e_1109']
 if '--check'in sys.argv:assert old==guards and state['Map_20240'].get('story_actor_gates')==barriers and state['Map_20240']['code']==post_code and 'party_event_gates'not in state['Map_20240']and state['Map_20180'].get('story_actor_gates')==albrook and state['Map_20210'].get('story_actor_gates')==vector
 else:
  if old and old!=guards:
   prior=[{k:v for k,v in r.items()if k!='require_known'}for r in guards];assert old==prior
   for a,b in zip(old,guards):events[events.index(a)]=b
  elif not old:events.extend(guards)
  post=state['Map_20240'];discarded=post.pop('party_event_gates',None)
  if discarded:assert len(discarded)==5 and all(r['group']=='ev_e_0442'for r in discarded)
  assert 'story_actor_gates'not in post or post['story_actor_gates']==barriers
  changes=[i for i,(a,b)in enumerate(zip(post['code'],post_code))if a!=b];assert len(post['code'])==len(post_code)and changes in([],[8,9]),changes
  if changes:assert post['code'][8]==post['code'][9]==[9]
  post['code']=post_code;post['story_actor_gates']=barriers
  assert 'story_actor_gates'not in state['Map_20180']or state['Map_20180']['story_actor_gates']==albrook
  state['Map_20180']['story_actor_gates']=albrook;p.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
  assert 'story_actor_gates'not in state['Map_20210']or state['Map_20210']['story_actor_gates']in(vector,vector[:3])
  state['Map_20210']['story_actor_gates']=vector;p.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));old=[r for r in data if r['submap']==landing['submap']and r.get('object_id')==landing['object_id']]
 if '--check'in sys.argv:assert old==[landing]
 else:
  assert not old or old==[landing]
  if not old:data.append(landing)
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Imperial travel:six exact194-selected factory guards,exact current-party initializer/two role branches/20 camp pushback actors,one Albrook port barrier,one actual crate landing;other authored state preserved.')
