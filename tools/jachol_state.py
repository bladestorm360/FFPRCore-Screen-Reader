"""Two exact Jachol initializers;preserve wall facts around native timers."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
def extend(script,game,region,code):
 if game!='ff5'or(script.get('submap'),script.get('name'))not in{('Map_30161_1','sc_map_30161_1'),('Map_30161_2','sc_map_30161_2')}:return code
 proof=json.loads((R/'docs/re/ff5_jachol_native_20261104.json').read_text(encoding='utf-8'));key=script['submap']+'/'+script['name'];assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][key]
 if script['name']=='sc_map_30161_2':
  assert script['mnemonics'][1]['operands']['sValues'][0]=='ジャコールのギミック解除'and code[1]==[9]
  # Exact2BFF50 only unregisters the two prior-room timer keys. It cannot
  # suppress this room's subsequent groups or435 chest/counterpart placement.
  code[1]=[0]
 else:
  assert len(code)==445 and script['mnemonics'][50]['operands']['sValues'][0]=='ジャコールのギミック設定'and code[50]==[9]
  # Exact2C4760 calls the reset phase,then schedules the native random cycle.
  # Only real Entity8 has uncertain future visibility/position. Do not discard
  # the preceding native wall Hides/Shows or invent an unloaded timed target.
  # Current-floor routing always consumes its actual live object transform.
  base=len(code);code[50]=[3,base];code.extend([[10,0,8,-2,-2147483648,-2147483648],[11,0],[6]])
 return code
