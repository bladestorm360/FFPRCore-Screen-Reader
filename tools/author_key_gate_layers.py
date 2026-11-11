"""Original locked-door collision coverage from exact paired native objects.

Only navigation declarations and source hashes are emitted. Native instructions
remain external. No blanket all-layer collision policy is introduced.
"""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
def main():
 inventory=json.loads((ROOT/'docs/re/key_door_inventory_20261101.json').read_text(encoding='utf-8'))['ff1']['objects']
 doors=[o for o in inventory if o['type']=='Entity' and o['props'].get('message_key') in ('MSG_WST_CAS_03','MSG_CHS_04')]
 rooms={o['submap'] for o in doors};objects=[];scripts={}
 for line in (DUMP/'ff1_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap'] in rooms:objects.append(o)
 for line in (DUMP/'ff1_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap'] in rooms and r['name'].startswith('sc_map_'):scripts[r['submap']]=r
 info=json.loads((DUMP/'ff1_submaps.json').read_text(encoding='utf-8'));bounds={r:bake_maps.map_bounds(info[r],[o for o in objects if o['submap']==r]) for r in rooms}
 path=ROOT/'data/ff1/mapstate.json';original=path.read_text(encoding='utf-8');data=json.loads(original);facts=[]
 for room in sorted(rooms):
  source=scripts[room];body=source['mnemonics'];assert any(m['mnemonic']=='Branch' and m['operands']['sValues'][0]=='ScenarioFlag1' and m['operands']['iValues'][0]==13 for m in body),(room,'key13 predicate absent')
  rows=[];ox,oy,*_=bounds[room]
  for door in [d for d in doors if d['submap']==room]:
   p=door['props'];assert p['collision'] is True and p['action_id']==4
   receipt=13 if p['message_key']=='MSG_WST_CAS_03' else 1
   if receipt==1:assert (room,door['entity'],door['group'])==('Map_30011_1',37,'ev_e_0002')
   assert any(m['mnemonic']=='Branch' and m['operands']['sValues'][0]=='ScenarioFlag1' and m['operands']['iValues'][0]==receipt for m in body)
   counterparts=[o for o in objects if o['submap']==room and o['type_name']=='OpenTrigger' and o['tx']==door['cell'][0] and o['ty']==door['cell'][1] and o['props'].get('link_object_id',0)>0]
   layers=sorted({bake_maps.layer_index(o['layer']) for o in counterparts});assert layers and all(0<=x<=2 for x in layers),(room,door['entity'],'no native opening footprint')
   own=bake_maps.layer_index(door['layer']);assert own in layers
   raw=next(o for o in objects if o['submap']==room and o['group']==door['group'] and o['props'].get('entity_id')==door['entity'] and o['name']==door['raw'])
   row=dict(id=door['entity'],group=door['group'],raw=door['raw'],cell=[own,raw['tx']-ox,bake_maps.tile_row(raw)-oy],layers=layers)
   tested=sorted({m['operands']['iValues'][0]+256*(int(m['operands']['sValues'][0][-1])-1) for m in body if m['mnemonic']=='Branch' and m['operands']['sValues'][0].startswith('ScenarioFlag')})
   rows.append(row);facts.append(dict(submap=room,**row,open_controls=[dict(id=o['props'].get('entity_id',0),tmx_object=o['id'],group=o['group'],layer=bake_maps.layer_index(o['layer']),link=o['props']['link_object_id']) for o in counterparts],unlock_receipt=receipt,unlock_kind='mystic_key' if receipt==13 else 'story',initializer_scenario_flags=tested,initializer_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
  data[room]['collision_layers']=rows
 rendered=json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n'
 if '--check' in sys.argv:assert original==rendered,'Key-door layer declarations differ'
 else:path.write_text(rendered,encoding='utf-8')
 (ROOT/'docs/re/key_gate_layers_20261101.json').write_text(json.dumps(dict(about='Original navigation coverage of locked-door actor and exact native OpenTrigger counterpart layers; no game implementation.',doors=facts),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FF1 locked-door coverage:',len(facts),'native variants across',len(rooms),'rooms;23 Mystic Key and1 separate story seal; exact paired opening layers only.')
if __name__=='__main__':main()
