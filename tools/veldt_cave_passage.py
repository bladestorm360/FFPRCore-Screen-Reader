"""Only the completed rescue's repeat transfer is ordinary cross-floor travel."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
PROOF=json.loads((R/'docs/re/veldt_cave_native_20261104.json').read_text(encoding='utf-8'))
def validate(obj,script):
 if obj['submap']!='Map_30761_5'or obj['id']!=9:return False
 row,=[a for a in PROOF['actors']if a['room']==obj['submap']and a['object_id']==9]
 assert(obj['type_name'],obj['props'].get('entity_id'),obj['props'].get('action_id'),obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==('Event',4,2,row['group'],row['raw'],row['cell'],row['size'])
 assert script and script['name']=='sc_e_0744'and hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes']['Map_30761_5/sc_e_0744']
 return True
def destination(obj,name,scripts):
 if obj['submap']!='Map_30761_5'or obj['id']!=9:return None
 assert name=='sc_e_0744';assert validate(obj,scripts[obj['submap'],name])
 # Initial branches pass491 only during an uncontrollable rescue. Both full
 # battle/voyage/dream chains end at311. Only the repeat branch lands on491/3.
 return(491,3,'')
def guard(obj,script):
 if not validate(obj,script):return None
 return dict(entry=0,code=[[2,1,42,0,-1,1,4],[2,1,117,0,-1,1,7],[6],[0],[2,1,116,0,-1,1,7],[6],[0],[1,'@action'],[6]],destination=[491,3],require_known=True)
