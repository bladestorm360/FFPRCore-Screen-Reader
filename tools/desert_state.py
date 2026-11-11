"""One exact initializer's four unchanged-layer story-NPC placements."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
def extend(script,game,region,code):
 if game!='ff5'or(script.get('submap'),script.get('name'))!=('Map_30170','sc_map_30170'):return code
 import bake_map_state as b
 proof=json.loads((R/'docs/re/ff5_desert_native_20261105.json').read_text(encoding='utf-8'));key=script['submap']+'/'+script['name'];assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][key]
 assert tuple(region[:2])==(0,0)and len(code)==133;seen=set()
 for pc,m in enumerate(script['mnemonics']):
  if m['mnemonic']!='SetPuppet':continue
  iv=m['operands']['iValues']
  if iv[1]not in(27,28,61,62):continue
  effect=b.puppet_navigation_effect(script,iv[2],region,True);assert effect==[1,55 if iv[1]in(27,61)else 51,13]
  code[pc]=[10,iv[0],iv[1],*effect];seen.add(iv[1])
 assert seen=={27,28,61,62};return code
