"""Two exact music-only initializers;never ignore an arbitrary native call."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
def extend(script,game,region,code):
 if game!='ff5'or(script.get('submap'),script.get('name'))not in{('Map_30531_6','sc_map_30531_6'),('Map_30531_10','sc_map_30531_10')}:return code
 proof=json.loads((R/'docs/re/ff5_gogo_native_20261104.json').read_text(encoding='utf-8'));key=script['submap']+'/'+script['name'];assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][key]
 assert len(proof['native_functions'])==6 and code[2]==[9]and script['mnemonics'][2]['operands']['sValues'][0]=='BGMテンポアップ設定'
 # Exact Setup/ResetBgmTempoUp,its two timer callbacks and full sync-load
 # closure only register/remove the BGM timer,read the countdown/music flag
 # and load/pitch music. No actor,group,terrain or navigation flag changes.
 code[2]=[0];return code
