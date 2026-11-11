"""One exact raft departure: both native story branches share the full arrival."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/solitary_native_20261104.json').read_text(encoding='utf-8'))
CHAIN=(('Map_20291_2','sc_e_0626'),('Map_20291_2','sc_e_0630'),
       ('Map_20300','sc_e_0626_1'),('Map_20300','sc_e_0630_1'),
       ('Map_10020','sc_e_0626_2'),('Map_10020','sc_e_0626_3'))
def identity(obj):
 if obj['submap']!='Map_20291_2'or obj['props'].get('entity_id')!=3:return False
 row,=[a for a in PROOF['actors']if a['room']==obj['submap']and a['entity']==3 and a['type']=='AnimEntity']
 assert(obj['id'],obj['type_name'],obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==(row['object_id'],row['type'],row['group'],row['raw'],row['cell'],row['size'])
 assert row['script']=='sc_e_0626'and(obj['props'].get('accept_action_direction')or 15)==15
 return True
def validate(scripts):
 for room,name in CHAIN:
  body=scripts[room,name]
  assert hashlib.sha256(json.dumps(body['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes'][room+'/'+name]
 final=scripts['Map_10020','sc_e_0626_3']['mnemonics']
 assert final[13]['mnemonic']=='SetPuppet'and final[13]['operands']['iValues'][1:3]==[-1001,17]
 assert final[18]['mnemonic']=='SetPos'and final[18]['operands']['iValues'][:2]==[146,212]
def destination(obj,name,scripts):
 if not identity(obj):return None
 assert name=='sc_e_0626';validate(scripts)
 return(2,1,'sc_e_0626_3')
