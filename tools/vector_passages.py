"""Three exact story-selected factory entrances, with both native variants."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/imperial_travel_native_20261104.json').read_text(encoding='utf-8'))
ROWS={115:28,116:29,117:30}
def identity(obj):
 if obj['submap']!='Map_20210'or obj['id']not in ROWS:return False
 row,=[a for a in PROOF['actors']if a['room']=='Map_20210'and a['object_id']==obj['id']and a['type']=='Event']
 assert (obj['type_name'],obj['props'].get('entity_id'),obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==('Event',ROWS[obj['id']],row['group'],row['raw'],row['cell'],row['size'])
 assert row['script']=='sc_e_1109'
 return True
def variants(obj,script):
 if not identity(obj):return []
 assert script and script['name']=='sc_e_1109'
 wanted=PROOF['source_script_hashes']['Map_20210/sc_e_1109']
 assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted
 return[(396,1,''),(643,1,'')]
def destination(obj,name,scripts):
 if not identity(obj):return None
 assert name=='sc_e_1109'
 # The baker emits both qualified branches. Destination-specific state guards
 # authored with the existing compiler retain native scenario194 selection.
 return variants(obj,scripts['Map_20210',name])[0]
