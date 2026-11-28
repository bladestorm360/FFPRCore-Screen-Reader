"""Original authored operation receipts from exact external game identities.

Never copies game implementation into resources. --check verifies current data
without writes. Native temporary state is map-owned; proximity is a waypoint
receipt only. See docs/re/SHIFTG_PUZZLE_AUDIT_20261030.md.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import bake_maps
from ordinary_interaction_fixtures import retired_recipes

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
# map, entity, exact native script, scenario bit (None = reviewed temporary state)
CONTROLS={
 'ff3':[(336,9,'sc_e_0290',221),(153,10,'sc_e_0003',2),(159,8,'sc_e_0274',180),
        (179,7,'sc_sw_30071_6',225),(191,9,'sc_sw_30091_3',199)],
 # A contact opens the next gate. Its receipt belongs to the reachable
 # contact on this side, beginning with the southern entrance switch.
 'ff4':[(102,41,'sc_s_20111_2',274),(102,43,'sc_s_20111_2',275),
        (102,45,'sc_s_20111_2',276),(102,44,'sc_s_20111_2',279),
        (4,42,'sc_s_20010_1',280),(4,45,'sc_s_20010_2',281),
        (5,16,'sc_s_20011_1',283),(16,7,'sc_s_20011_2',283),(29,7,'sc_s_20021_2',283)],
 'ff5':[(256,13,'sc_e_0216',None)],
 'ff6':[(386,11,'sc_e_0299',None),(458,1,'sc_e_0683',341),(463,24,'sc_e_0685',343),
        (517,2,'sc_e_0886',428),(517,12,'sc_e_0887',429),(519,10,'sc_e_0890',432),
        (478,5,'sc_e_0878',421),(442,15,'sc_e_0761',381)]}

# Additional single-operation mechanisms discovered by the independent all-six
# inventory. Each native script sets exactly one completion bit. These are
# independently verified ordinary interactions, kept out of the guide catalog.
ADDITIONAL={
 'ff3': [('caanan_hidden_door',43,10,'sc_sw_20071_7',222),
         ('argus_hidden_door_lower',69,700,'sc_sw_20131_2',202),
         ('argus_hidden_door_upper',73,700,'sc_sw_20131_6',223),
         ('saronia_hidden_door',110,9,'sc_sw_20211_6',201),
         ('doga_manor_hidden_door',133,11,'sc_sw_20261_3',198),
         ('replito_hidden_door',141,3,'sc_sw_20281_3',204)],
 'ff4': [('baron_inn_switch',25,15,'sc_s_20021_1',283),
         ('damcyan_switch',42,11,'sc_s_20041_1',283),
         ('fabul_switch',47,11,'sc_s_20051_1',283),
         ('eblan_switch',84,10,'sc_s_20091_1',283),
         ('troia_town_passage',91,26,'sc_s_20101_1',283),
         ('troia_town_switch',94,10,'sc_s_20101_2',283),
         ('troia_town_rear_passage',95,20,'sc_s_20101_3',280),
         ('troia_castle_passage',104,9,'sc_s_20111_1',279)]}
for game,rows in ADDITIONAL.items():
 CONTROLS[game].extend((m,e,s,f) for _,m,e,s,f in rows)

def main():
 check=argparse.ArgumentParser();check.add_argument('--check',action='store_true');args=check.parse_args()
 evidence=[]
 # The old maze target was the automatic return warp, not the maze's end.
 # Native PointIn1 on3F is stable; entry proves the maze leg ended. The final
 # reward leg and recipe end receipt are owned by author_ff1_slab_guides.py.
 path=ROOT/'data/ff1/puzzles.json';old=path.read_text(encoding='utf-8');root=json.loads(old)
 objects=[o for line in (DUMP/'ff1_objects.jsonl').open(encoding='utf-8') if (o:=json.loads(line))['submap']=='Map_30071_3']
 points=[o for o in objects if o['type_name']=='PointIn' and o['props'].get('object_id')==1]
 assert len(points)==1
 info=json.loads((DUMP/'ff1_submaps.json').read_text(encoding='utf-8'))['Map_30071_3']
 ox,oy,*_=bake_maps.map_bounds(info,objects);point=points[0]
 cell=(bake_maps.layer_index(point['layer']),point['tx']-ox,bake_maps.tile_row(point)-oy)
 assert cell==(2,35,32),('Citadel endpoint changed',cell)
 next(s for s in root['puzzles'][0]['steps'] if s.get('reach_map')).update(x=cell[1],y=cell[2],layer=cell[0],reach_map=True)
 rendered=json.dumps(root,ensure_ascii=False,indent=2)+'\n'
 if args.check:assert old==rendered,('ff1','maze endpoint differs')
 else:path.write_text(rendered,encoding='utf-8')
 for game,controls in CONTROLS.items():
  path=ROOT/f'data/{game}/puzzles.json';old=path.read_text(encoding='utf-8');root=json.loads(old)
  retired=retired_recipes(ROOT,game)
  original_retired=json.dumps(retired,ensure_ascii=False)
  recipes=root['puzzles']+retired
  maps={int(r['id']):r for r in csv.DictReader((DUMP/f'{game}_master/map.txt').open(encoding='utf-8-sig'))}
  names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/f'{game}_master/script.txt').open(encoding='utf-8-sig'))}
  wanted={(maps[m]['map_name'],name) for m,_,name,_ in controls}
  if game=='ff6':wanted.add((maps[442]['map_name'],'sc_e_0761_1'))
  scripts={}
  for line in (DUMP/f'{game}_scripts.jsonl').open(encoding='utf-8'):
   s=json.loads(line);key=s['submap'],s['name']
   if key in wanted:scripts[key]=s
  assert scripts.keys()==wanted,(game,'missing exact source')
  rooms={r for r,_ in wanted}
  objects=[o for line in (DUMP/f'{game}_objects.jsonl').open(encoding='utf-8') if (o:=json.loads(line))['submap'] in rooms]
  infos=json.loads((DUMP/f'{game}_submaps.json').read_text(encoding='utf-8'))
  for name,map_id,entity,script,flag in ADDITIONAL.get(game,[]):
   assert any(p['id']==name for p in retired),(game,name,'missing archived interaction')
  if game=='ff6':
   clock=root['puzzles'][0];clock['_about']='Zozo inn: actual solution clock is map386/entity11, at7,20 Ground. Map387/entity3 only gives a clue. Correct choices are6,10,50 (positions3,1,5). Native local1=3 and hidden actors certify the opened wall; no persistent scenario flag is written.'
   clock['steps'][0].update(map=386,x=7,y=20,entity=11)
  for map_id,entity,name,flag in controls:
   room=maps[map_id]['map_name']
   found=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==entity and names.get(o['props'].get('script_id'))==name]
   assert len(found)==1,(game,map_id,entity,name,len(found));actor=found[0]
   ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
   cell=(bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy)
   targets=[s for p in recipes for s in p['steps'] if s['map']==map_id and (s.get('entity')==entity or (s['layer'],s['x'],s['y'])==cell)]
   assert len(targets)==1,(game,map_id,entity,len(targets));step=targets[0]
   step.update(entity=entity,raw=actor['name'],layer=cell[0],x=cell[1],y=cell[2])
   body=scripts[room,name]['mnemonics']
   if flag is not None:
    receipt=body if name!='sc_e_0761' else scripts[room,'sc_e_0761_1']['mnemonics']
    assert any(m['mnemonic']=='SetFlag' and m['operands']['iValues'][0]+256*(int(m['operands']['sValues'][0][-1])-1)==flag for m in receipt if m['operands'].get('sValues') and m['operands']['sValues'][0].startswith('ScenarioFlag'))
    step['done_scenario']=flag
   elif game=='ff6':
    assert sum(m['mnemonic']=='Calc' and m['operands']['sValues'][:3]==['MapLocalVariable','＋','imm'] and m['operands']['iValues'][:2]==[1,1] for m in body)==3
    step.update(done_local=1,done_value=3)
   else:
    # Exact native script binds Hide(0) to actors13/15. No second control or
    # persistent flag exists; an explicit repeat checks the live retained actor.
    bindings={m['operands']['iValues'][1]:m['operands']['iValues'][2] for m in body if m['mnemonic']=='SetPuppet'}
    assert set(bindings)>={13,15}
    assert all(body[bindings[e]+1]['mnemonic']=='Hide' and body[bindings[e]+1]['operands']['iValues'][0]==0 for e in (13,15))
    step['done_hidden']=True
   evidence.append(dict(game=game,map=map_id,entity=entity,script=room+'/'+name,
       source_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),cell=cell,receipt={k:v for k,v in step.items() if k.startswith('done_')}))
  assert original_retired==json.dumps(retired,ensure_ascii=False),(game,'retired interaction differs from native authoring')
  root['_comment']='Original factual puzzle recipes. Examined controls require native operation receipts and exact target identities; waypoint arrival alone completes only navigation steps. Source/evidence/limits: docs/re/SHIFTG_PUZZLE_AUDIT_20261030.md.'
  rendered=json.dumps(root,ensure_ascii=False,indent=2)+'\n'
  # The Karnak route's distance table deliberately keeps each row compact.
  # Verify the authored values without reformatting unrelated recipes.
  changed=json.loads(old)!=root
  if args.check:assert not changed,(game,'recipe differs; regenerate reviewed receipts')
  elif changed:path.write_text(rendered,encoding='utf-8')
 (ROOT/'build_check/static_puzzle_receipts.json').write_text(json.dumps(evidence,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print(f'Static puzzle receipts: {len(evidence)} exact controls in4 games; corrected Citadel maze endpoint, Zozo clock and flagless ship; no native code output')

if __name__=='__main__':main()
