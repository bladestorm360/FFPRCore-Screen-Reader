"""Complete actual Yeti pit, magicite choice/battle and Mog recruitment."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
ENTRY='Choose the first option to jump into the cave.'
FALL='Operate the switch to fall into the chamber below.'
SKULL='Choose the first option to take the magicite, then win the battle.'
MOG='Bring Mog in the active party before talking to the yeti.'
def sha(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
 proof=json.loads((ROOT/'docs/re/yeti_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 rules=[r([f(395)],t(20,1,'Defeat {0}. Contact starts this encounter; request guidance again after battle.','ev_e_0780')),
        r([c('map',0,20),f(395,1)],t(20,26,ENTRY,'ev_e_0781')),
        r([c('map',0,496),f(397)],t(496,6,SKULL,'ev_e_0789')),
        dict(when=[f(397,1),c('mog',0,0)],note=MOG),
        r([f(397,1)],t(496,1,'Continue to {0}, then request guidance again.','ev_e_0790')),
        r([],t(495,13,FALL,'entity_default'))]
 for row in rules:
  for goal in row.get('targets',[]):goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 rules[4]['finish_on_scenario']=398
 guide=o.guide('yeti_cave',[20,493,494,495,496],'',rules)
 guide.update(first_match=True,select_when=[f(353,1)],complete_scenario=398,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/YETI_CAVE_20261104.md')
 return guide
def passages(o):
 import bake_maps
 infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));rows=[];verified=[]
 for name in ('sc_e_0781','sc_e_0782','sc_e_0783','sc_e_0784','sc_e_0785','sc_e_0787','sc_e_0788'):
  sid,=[i for i,n in o.SCRIPT_NAMES.items()if n==name]
  actor,=[a for a in o.OBJECTS if a['props'].get('script_id')==sid and a['type_name']in('Entity','Event')and a['submap']in('Map_20040','Map_30771_1','Map_30771_3')]
  root=o.SCRIPTS[actor['submap'],name];transfer,=[m for m in root['mnemonics']if m['mnemonic']=='ChangeMap'];dest,point=transfer['operands']['iValues'][:2];continuation=transfer['operands']['sValues'][0];room=o.MAPS[dest]['map_name'];body=o.SCRIPTS[room,continuation]
  entries=[m['operands']['iValues'][2]for m in body['mnemonics']if m['mnemonic']=='SetPuppet'and m['operands']['iValues'][1]==-1001];ops=[]
  for entry in entries:
   for m in body['mnemonics'][entry:]:
    if m['mnemonic']=='Exit':break
    ops.append(m)
  pos,=[m['operands']['iValues'][:2]for m in ops if m['mnemonic']=='SetPos'];moves=[m['operands']['iValues'][:3]for m in ops if m['mnemonic']=='Move'];assert moves==[[2,7,4]],(name,moves)
  x,y=pos[0],pos[1]+7;ox,oy,*_=bake_maps.map_bounds(infos[room],[a for a in o.OBJECTS if a['submap']==room])
  native,=[a for a in o.OBJECTS if a['submap']==room and a['type_name']=='PointIn'and a['props'].get('object_id')==point]
  layer=bake_maps.layer_index(native['layer'])
  # Every original PointIn already describes the completed animation. The
  # second floor's arrivals are Upper, not Ground. Continuations have no
  # explicit layer change; preserve the actual native arrival and its layer.
  assert [x,y]==[native['tx'],bake_maps.tile_row(native)],(name,[x,y],native)
  verified.append(dict(source=name,destination=dest,point=point,final_cell=[x-ox,y-oy,layer],continuation_sha256=sha(body)))
  if actor['type_name']=='Entity':
   rows.append(dict(submap=actor['submap'],entity_id=actor['props']['entity_id'],object_id=actor['id'],destination_map=dest,original_point=point,continuation=continuation,continuation_sha256=sha(body),arrival_point=point,arrival_baked_cell=[x-ox,y-oy,layer],interaction_travel=True,evidence='Full native player SetPos plus seven South steps exactly matches the original PointIn;preserve its real layer and make only the switch interaction manual. docs/re/YETI_CAVE_20261104.md'))
 assert len(verified)==7 and len(rows)==2
 return rows
if __name__=='__main__':
 import author_ff6_story_guides as o
 check='--check'in sys.argv;guide=author(o);p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if check:assert[g for g in data['story_guides']if g['id']==guide['id']]==[guide]
 else:data['story_guides']=[g for g in data['story_guides']if g['id']!=guide['id']]+[guide];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'))
 wanted=passages(o)
 if not check:
  # Remove only this unfinished scope's earlier synthetic landings. No
  # previously Closed override belongs to these four rooms/native identities.
  reviewed_rooms={'Map_20040','Map_30771_1','Map_30771_3'};owned_names={name+'_1'for name in('sc_e_0781','sc_e_0782','sc_e_0783','sc_e_0784','sc_e_0785','sc_e_0787','sc_e_0788')}
  data=[r for r in data if not(r['submap']in reviewed_rooms and r.get('continuation')in owned_names)]
 for row in wanted:
  old=[r for r in data if r['submap']==row['submap']and r.get('object_id')==row['object_id']]
  if check:assert old==[row]
  else:
   assert not old or old==[row]
   if not old:data.append(row)
 if not check:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Yeti:full entrance/both switches/seven original full fall arrivals verified;only two manual companions change. Real Mog11 recruitment398;ordinary conversation remains reachable without Mog.')
