"""Original factual navigation identities for FFIII's thirteen consumable locks.

Only IDs, footprints, receipts and source hashes are emitted. Ordered native
programs remain in the external extraction archive.
"""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'

def main():
 inventory=json.loads((ROOT/'docs/re/key_door_inventory_20261101.json').read_text(encoding='utf-8'))['ff3']
 doors=[o for o in inventory['objects'] if o['type']=='Entity' and o['script'].startswith('sc_key_')]
 assert len(doors)==13
 rooms={o['submap'] for o in doors};objects=[];programs={};writes={}
 for line in (DUMP/'ff3_objects.jsonl').open(encoding='utf-8'):
  row=json.loads(line)
  if row['submap'] in rooms:objects.append(row)
 for line in (DUMP/'ff3_scripts.jsonl').open(encoding='utf-8'):
  row=json.loads(line)
  if row['submap'] in rooms:programs[row['submap'],row['name']]=row
  for m in row['mnemonics'] or []:
   if m['mnemonic'] not in ('SetFlag','ResetFlag'):continue
   iv=m['operands'].get('iValues',[]);sv=m['operands'].get('sValues',[])
   if not iv or not sv or sv[0] not in ('ScenarioFlag1','ScenarioFlag2','ScenarioFlag3','ScenarioFlag4'):continue
   bit=iv[0]+256*(int(sv[0][-1])-1)
   if 205<=bit<=217:
    assert m['mnemonic']=='SetFlag',(row['submap'],row['name'],bit,'unlock receipt can be reset')
    writes.setdefault(bit,set()).add((row['submap'],row['name']))
 info=json.loads((DUMP/'ff3_submaps.json').read_text(encoding='utf-8'))
 bounds={r:bake_maps.map_bounds(info[r],[o for o in objects if o['submap']==r]) for r in rooms}
 maps={}
 for row in csv.DictReader((DUMP/'ff3_master/map.txt').open(encoding='utf-8-sig')):maps.setdefault(row['map_name'],[]).append(int(row['id']))
 content=next(r for r in csv.DictReader((DUMP/'ff3_master/content.txt').open(encoding='utf-8-sig')) if int(r['id'])==15)
 assert int(content['type_id'])==1 and int(content['type_value'])==14,content
 facts=[]
 for door in doors:
  room=door['submap'];body=programs[room,door['script']]['mnemonics'];calls={m.get('operands',{}).get('sValues',[''])[0] for m in body if m['mnemonic']=='SysCall'}
  assert {'シーフがいるか','消費アイテム選択肢表示'}<=calls
  receipts=[m['operands']['iValues'][0] for m in body if m['mnemonic']=='SetFlag' and m['operands']['sValues'][0]=='ScenarioFlag1']
  uses=[m['operands']['iValues'] for m in body if m['mnemonic']=='UseItem']
  assert len(receipts)==1 and 205<=receipts[0]<=217 and len(uses)==1 and uses[0][0]==14,(door,receipts,uses)
  ox,oy,*_=bounds[room];raw=next(o for o in objects if o['submap']==room and o['group']==door['group'] and o['props'].get('entity_id')==door['entity'] and o['name']==door['raw'])
  assert raw['props']['collision'] is True
  controls=[o for o in objects if o['submap']==room and o['type_name']=='OpenTrigger' and o['tx']==raw['tx'] and bake_maps.tile_row(o)==bake_maps.tile_row(raw)]
  own=bake_maps.layer_index(raw['layer']);layers=sorted({own}|{bake_maps.layer_index(o['layer']) for o in controls})
  facts.append(dict(submap=room,maps=maps[room],id=door['entity'],group=door['group'],raw=door['raw'],cell=[own,raw['tx']-ox,bake_maps.tile_row(raw)-oy],layers=layers,open_controls=[dict(id=o['props'].get('entity_id',0),tmx_object=o['id'],group=o['group'],layer=bake_maps.layer_index(o['layer']),link=o['props'].get('link_object_id',0)) for o in controls],script=door['script'],receipt=receipts[0],item=14,content=15,thief_job=3,sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
 assert sorted(f['receipt'] for f in facts)==list(range(205,218))
 assert set(writes)==set(range(205,218))
 for row in facts:assert writes[row['receipt']]=={(row['submap'],row['script'])},(row['receipt'],writes[row['receipt']])
 # Factual prerequisites are independently established by the real-floor
 # topology check, including every passive floor and native stair arrival.
 proof=json.loads((ROOT/'build_check/ff3_key_dependencies.json').read_text())
 parents={r['receipt']:r['parents'] for r in proof['dependencies']}
 assert parents=={i:([214] if i in (216,217) else []) for i in range(205,218)}
 # The reviewed native collision correction excludes eight cover-marked walls:
 # roots67/219 now503/599, root242 remains751. The13-door prerequisites and
 # complete two-key union still pass. Keep this guard tied to that exact proof.
 assert proof['max_keys']==2 and proof['all_open_cells']==1853
 delta=proof['cover_wall_delta']
 expected={(71,0,x,y)for x in(9,26)for y in(6,7)}|{(223,1,x,y)for x in(9,16)for y in(5,6)}
 assert proof['legacy_all_open_cells']==1861 and len(delta)==8
 assert {(r['map'],r['layer'],r['x'],r['y'])for r in delta}==expected
 assert all(r['collision']==0 and r['cover']==1 for r in delta)
 path=ROOT/'data/ff3/mapstate.json';original=path.read_text(encoding='utf-8');data=json.loads(original)
 for room in rooms:data[room]['key_gates']=[]
 for row in facts:
  row['prerequisites']=parents[row['receipt']]
  data[row['submap']]['key_gates'].append({k:row[k] for k in ('id','group','raw','cell','receipt','prerequisites')})
 rendered=json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n'
 if '--check' in sys.argv:assert original==rendered,'Magic Key declarations differ'
 else:path.write_text(rendered,encoding='utf-8')
 factual=ROOT/'docs/re/ff3_magic_key_gates_20261101.json'
 rendered_facts=json.dumps(dict(doors=facts,scope='Factual lock identities and source predicates; no native implementation or runtime solver.'),ensure_ascii=False,indent=2)+'\n'
 if '--check' in sys.argv:assert factual.read_text(encoding='utf-8')==rendered_facts,'Magic Key factual identities differ'
 else:factual.write_text(rendered_facts,encoding='utf-8')
 print('FFIII consumable locks:',len(facts),'identities;',len(rooms),'submaps; item14/content15, active enabled Thief job3; receipts205..217, no resets/other writers in complete script corpus.')
 for row in facts:
  print(row['maps'],row['id'],row['cell'],'receipt',row['receipt'],'opening layers',row['layers'],'controls',len(row['open_controls']))
  if not row['open_controls']:
   ox,oy,*_=bounds[row['submap']]
   nearby=[dict(type=o['type_name'],id=o['props'].get('entity_id',0),layer=bake_maps.layer_index(o['layer']),cell=[o['tx']-ox,bake_maps.tile_row(o)-oy],link=o['props'].get('link_object_id',0)) for o in objects if o['submap']==row['submap'] and o['type_name'] in ('OpenTrigger','ToLayer','CollisionEntity') and abs(o['tx']-ox-row['cell'][1])<=1 and abs(bake_maps.tile_row(o)-oy-row['cell'][2])<=1]
   print('Nearby native controls:',nearby)

if __name__=='__main__':main()
