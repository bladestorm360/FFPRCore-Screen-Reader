"""Five exact player-operated Trench chains, never generic scene collapsing."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
PROOF=json.loads((ROOT/'docs/re/crescent_native_20261104.json').read_text(encoding='utf-8'))
# Only known native Right branches are graph connections. The renderer remains
# player-operated; a route stops with the explicit choice rather than driving it.
ROWS={
 ('Map_30490',1):('sc_e_0270',(379,1,''),(1<<21)|(1<<22)),
 ('Map_30490',2):('sc_e_0270',(379,1,''),(1<<21)|(1<<22)),
 ('Map_30491_1',10):('sc_e_0246',(379,1,''),1<<21),
 ('Map_30501_1',0):('sc_e_1501',(380,1,''),1<<21),
 ('Map_30501_4',0):('sc_e_1502',(96,2,'sc_e_1502_3'),0),
}
def identity(obj):
 row=ROWS.get((obj['submap'],obj['props'].get('entity_id')))
 if row is None or obj['type_name']!='Event':return None
 actors=[a for a in PROOF['actors']if (a['room'],a['entity'],a['type'])==(obj['submap'],obj['props']['entity_id'],'Event')]
 assert len(actors)==1;actor=actors[0]
 assert (obj['id'],obj['group'],obj['name'],[obj['layer'],obj['tx'],obj['ty']],[obj['w'],obj['h']])==(actor['object_id'],actor['group'],actor['raw'],actor['cell'],actor['size'])
 assert actor['script']==row[0]
 return row
def entry_flags(obj):
 row=identity(obj);return row[2]if row else 0
def destination(obj,name,scripts):
 row=identity(obj)
 if row is None:return None
 assert name==row[0]
 # Pin every connected choice, battle continuation, current-area resume and
 # final ready arrival. No first-ChangeMap heuristic certifies this connection.
 for locator,wanted in PROOF['source_script_hashes'].items():
  room,label=locator.split('/');body=scripts[room,label]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 return row[1]
