"""The post-reunion summit edge; the initial native round trip stays a scene."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/cyan_zozo_native_20261104.json').read_text(encoding='utf-8'))
def validate(obj,script):
 if obj['submap']!='Map_30681_1'or obj['id']!=24:return False
 row,=[a for a in PROOF['actors']if a['room']==obj['submap']and a['object_id']==24]
 assert (obj['type_name'],obj['props'].get('entity_id'),obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==('Event',16,row['group'],row['raw'],row['cell'],row['size'])
 assert script and script['name']=='sc_e_0735'and hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes']['Map_30681_1/sc_e_0735']
 return True
def destination(obj,name,scripts):
 if obj['submap']!='Map_30681_1'or obj['id']!=24:return None
 assert name=='sc_e_0735';assert validate(obj,scripts[obj['submap'],name])
 # Only the repeat branch is physical travel. The separate exact365 guard
 # must qualify this graph companion; the first reunion remains an event.
 return (475,1,'')
def guard(obj,script):
 if not validate(obj,script):return None
 return dict(entry=0,code=[[2,1,109,0,-1,1,3],[6],[0],[1,'@action'],[6]],destination=[475,1],require_known=True)
