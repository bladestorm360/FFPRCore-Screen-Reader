"""Three manual bridge jumps; verify the three existing correct pit landings."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
 import bake_maps
 proof=json.loads((ROOT/'docs/re/esper_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
 rows=[]
 def bounds(room):
  info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[room]
  return bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']==room])[:2]
 def add(actor,body,landing,manual,point=0,continuation=None):
  ox,oy=bounds('Map_30561_1');row=dict(submap=actor['submap'],entity_id=actor['props']['entity_id'],object_id=actor['id'],destination_map=424,original_point=point,continuation=continuation or body['name'],continuation_sha256=sha(body),arrival_cell=landing,arrival_baked_cell=[landing[0]-ox,landing[1]-oy,landing[2]],evidence='Exact native player Lerp/Move or PointIn plus8 South continuation; docs/re/ESPER_CAVE_GUIDE_20261104.md')
  if manual:row.update(same_map=True,interaction_travel=True,approach_mask=actor['props'].get('accept_action_direction',15)or 15)
  rows.append(row)
 for entity,name in ((16,'sc_e_0543'),(17,'sc_e_0544'),(18,'sc_e_0545')):
  actor=o.physical(424,entity,'entity_default');body=o.SCRIPTS[actor['submap'],name];ms=body['mnemonics'];assert o.SCRIPT_NAMES[actor['props']['script_id']]==name
  assert ms[4]['mnemonic']=='SetPuppet'and ms[4]['operands']['iValues'][1:3]==[-1001,8]
  assert ms[10]['mnemonic']=='Lerp'and ms[11]['mnemonic']=='Move'and ms[11]['operands']['iValues'][:2]==[2,2]
  x,y=ms[10]['operands']['iValues'][:2];add(actor,body,[x,y+2,bake_maps.layer_index(actor['layer'])],True)
 for entity,name,point in ((10,'sc_e_0548',7),(12,'sc_e_0549',8),(11,'sc_e_0550',9)):
  actor=o.physical(426,entity,'ev_e_0548');assert o.SCRIPT_NAMES[actor['props']['script_id']]==name
  root=o.SCRIPTS[actor['submap'],name];m=root['mnemonics'][2];continuation=name+'_1'
  assert m['mnemonic']=='ChangeMap'and m['operands']['iValues'][:2]==[424,point]and m['operands']['sValues'][0]==continuation
  body=o.SCRIPTS['Map_30561_1',continuation];assert body['mnemonics'][19]['mnemonic']=='SetPuppet'and body['mnemonics'][19]['operands']['iValues'][1:3]==[-1001,22]
  assert body['mnemonics'][23]['mnemonic']=='Move'and body['mnemonics'][23]['operands']['iValues'][:2]==[2,8]
  entry,=[a for a in o.OBJECTS if a['submap']=='Map_30561_1'and a['type_name']=='PointIn'and a['props'].get('object_id')==point]
  add(actor,body,[entry['tx'],bake_maps.tile_row(entry)+8,bake_maps.layer_index(entry['layer'])],False,point,continuation)
 return rows
if __name__=='__main__':
 import author_ff6_story_guides as o
 rows=author(o);p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'))
 for row in rows:
  if row['submap']=='Map_30561_3':
   # The maintained trap review already owns these exact final landings.
   # Verify it in place; never add a second arc for the same physical pit.
   old=[r for r in data if r['submap']==row['submap']and r['entity_id']==row['entity_id']and 'object_id'not in r]
   assert len(old)==1
   for field in ('destination_map','original_point','continuation','continuation_sha256','arrival_cell','arrival_baked_cell'):assert old[0][field]==row[field],field
   duplicate=[r for r in data if r['submap']==row['submap']and r.get('object_id')==row['object_id']]
   if '--check'in sys.argv:assert not duplicate
   else:
    assert not duplicate or duplicate==[row]
    if duplicate:data.remove(duplicate[0])
   continue
  current=[r for r in data if(r['submap'],r.get('object_id'))==(row['submap'],row['object_id'])]
  if '--check'in sys.argv:assert current==[row]
  else:
   assert not current or current==[row]
   if not current:data.append(row)
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Esper:three new source-qualified manual jumps;three previous post-fall landings verified unchanged;native geometry/objects preserved.')
