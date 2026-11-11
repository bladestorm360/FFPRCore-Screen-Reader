"""One dynamic saved-position transfer; never promote its fallback PointIn."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/imperial_travel_native_20261104.json').read_text(encoding='utf-8'))

def entry_flags(obj):
 if obj['submap']!='Map_20170'or obj['props'].get('entity_id')!=9:return 0
 row,=[a for a in PROOF['actors']if a['room']=='Map_20170'and a['entity']==9 and a['type']=='Entity']
 assert(obj['id'],obj['type_name'],obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==(row['object_id'],row['type'],row['group'],row['raw'],row['cell'],row['size'])
 assert row['script']=='sc_e_BJoutboard'
 return 1<<23

def validate(scripts):
 for room,name in(('Map_20170','sc_e_BJoutboard'),('Map_10010','sc_e_BJoutboard_1')):
  body=scripts[room,name]
  assert hashlib.sha256(json.dumps(body['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PROOF['source_script_hashes'][room+'/'+name]

def destination(obj,name,scripts):
 if not entry_flags(obj):return None
 assert name=='sc_e_BJoutboard';validate(scripts)
 # Only the explicitly chosen Fly branch is represented. The dynamic flag
 # prevents generic traversal of this fallback destination/point.
 return(1,1,'sc_e_BJoutboard_1')
