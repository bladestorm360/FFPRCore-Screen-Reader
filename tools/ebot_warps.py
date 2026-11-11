"""Exact Ebot random-transfer outcomes; metadata never predicts fresh RNG."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
RANDOM=1<<19
def outcomes(game,obj,script):
 if game!='ff6'or obj['submap']not in('Map_30861_1','Map_30861_2','Map_30861_3')or not script or script['name']not in('sc_e_0946','sc_e_0947'):return []
 manifest=json.loads((ROOT/'docs/re/ebots_rock_native_20261104.json').read_text(encoding='utf-8'))
 locator=obj['submap']+'/'+script['name'];body=script['mnemonics']
 assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==manifest['source_script_hashes'][locator],locator
 assert obj['type_name']=='Event'and obj['group']=='entity_default'
 arrivals=[(int(m),int(p),'sc_e_0946_0')for m,p in manifest['random_arrivals']]
 if script['name']=='sc_e_0946':
  assert obj['submap']=='Map_30861_1'and obj['props']['entity_id']==5
  return [(arrivals[0],False)]+[(v,True)for v in arrivals]
 assert any(m['mnemonic']=='SetRandomValue'and m['operands']['iValues'][:2]==[3,15]for m in body)
 actual={(m['operands']['iValues'][0],m['operands']['iValues'][1],m['operands']['sValues'][0])for m in body if m['mnemonic']=='ChangeMap'}
 assert actual==set(arrivals)and len(arrivals)==12
 return [(v,True)for v in arrivals]
