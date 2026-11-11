"""The completed later-Opera lever's full manual exit,never its boss arena."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1];PROOF=json.loads((R/'docs/re/late_opera_native_20261104.json').read_text(encoding='utf-8'))
def validate(obj,script):
 if obj['submap']!='Map_20441_3'or obj['id']!=18:return False
 row,=[a for a in PROOF['actors']if a['room']==obj['submap']and a['object_id']==18]
 assert(obj['type_name'],obj['props'].get('entity_id'),obj['props'].get('action_id'),obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==('Entity',3,2,row['group'],row['raw'],row['cell'],row['size'])
 assert script and script['name']=='sc_e_0893'and hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes']['Map_20441_3/sc_e_0893']
 return True
def destination(obj,name,scripts):
 if obj['submap']!='Map_20441_3'or obj['id']!=18:return None
 assert name=='sc_e_0893';assert validate(obj,scripts[obj['submap'],name])
 for room,label in(('Map_20441_2','sc_e_0333_1'),('Map_20440','sc_e_0333_2')):
  assert hashlib.sha256(json.dumps(scripts[room,label]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes'][room+'/'+label]
 return(290,1,'sc_e_0333_2')
def guard(obj,script):
 if not validate(obj,script):return None
 overrides=json.loads((R/'data/ff6/warp_arrival_overrides.json').read_text(encoding='utf-8'));index,=[i for i,r in enumerate(overrides)if r['submap']==obj['submap']and r.get('object_id')==18]
 row=overrides[index];assert row['destination_map']==290 and row['arrival_cell']==[66,42,1]and row['continuation']=='sc_e_0333_2'
 return dict(entry=0,code=[[2,1,178,0,-1,1,3],[6],[0],[1,'@action'],[6]],destination=[290,-100000-index],require_known=True)
