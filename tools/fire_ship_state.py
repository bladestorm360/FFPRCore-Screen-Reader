"""Exact six unchanged-layer placements in the native passage initializer."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
def extend(script,game,region,code):
 if game!='ff5'or(script.get('submap'),script.get('name'))!=('Map_30151_20','sc_map_30151_20'):return code
 import bake_map_state as b
 proof=json.loads((R/'docs/re/ff5_fire_ship_native_20261105.json').read_text(encoding='utf-8'))
 key=script['submap']+'/'+script['name'];assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][key]
 assert tuple(region[:2])==(0,0)and len(code)==436
 seen=set()
 for pc,m in enumerate(script['mnemonics']):
  if m['mnemonic']!='SetPuppet':continue
  iv=m['operands']['iValues']
  if iv[1]not in(52,53,56,57,58,824):continue
  effect=b.puppet_navigation_effect(script,iv[2],region,True)
  assert effect and effect[0]==1 and 0<=effect[1]<25 and 0<=effect[2]<27
  code[pc]=[10,iv[0],iv[1],*effect];seen.add(iv[1])
 assert seen=={52,53,56,57,58,824}
 return code
