"""Two exact basement transfers whose world-map visit is camera-only.

Native source stays external. These reviewed facts correct the final ordinary
arrival without changing controls, native state or other scene transfers.
"""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/south_figaro_native_20261104.json').read_text(encoding='utf-8'))
ROWS={'Map_20071_20':('sc_e_1047','sc_e_1047_1',1145,57),
      'Map_20071_19':('sc_e_1048','sc_e_1048_1',1146,58)}
def changes(body):
 main=next(s for s in body['segments']if s['Label']=='Main')
 return[tuple([*m['operands']['iValues'][:2],m['operands']['sValues'][0]])for m in body['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]if m['mnemonic']=='ChangeMap']
def destination(obj,name,scripts):
 row=ROWS.get(obj['submap'])
 if not row or obj['type_name']!='Event' or obj['props'].get('entity_id')!=0:return None
 root,continuation,sid,final=row;p=obj['props']
 assert(obj['group'],p['script_id'],p['action_id'],name)==('entity_default',sid,2,root),'South Figaro passage identity changed'
 bodies=[]
 for room,label in[(obj['submap'],root),('Map_20070',continuation)]:
  body=scripts.get((room,label));assert body
  digest=hashlib.sha256(json.dumps(body['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
  assert digest==PROOF['source_script_hashes'][room+'/'+label],'South Figaro passage source changed'
  bodies.append(body)
 assert changes(bodies[0])==[(38,1,continuation)]and changes(bodies[1])==[(final,1,'')]
 return final,1,''
if __name__=='__main__':
 import author_ff6_story_guides as o
 from check_scripted_bakes import decode
 maps,_=decode(ROOT/'data/ff6/maps.bin')
 for room,(root,continuation,sid,final)in ROWS.items():
  actors=[a for a in o.OBJECTS if a['submap']==room and a['type_name']=='Event'and a['props'].get('entity_id')==0]
  assert len(actors)==1;actor=actors[0];assert destination(actor,root,o.SCRIPTS)==(final,1,'')
  arcs=[a for a in maps[room]['objects']if a[0]==100 and a[8]==0 and a[10]==actor['name']and a[11]==actor['group']]
  assert len(arcs)==1 and arcs[0][6:8]==(final,1)and arcs[0][9]&65536
 print('South Figaro: both complete camera-only chains and final native baked arrivals match.')
