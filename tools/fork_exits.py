"""Fork Tower's exact scene transfers; no guessed route through party setup."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1];TAG=1<<25
def flags(game,o,script):
 if game!='ff5' or o['type_name']!='Event' or o['props'].get('entity_id')!=1:return 0
 room=o['submap'];name=script['name']
 if (room,name) not in {('Map_30481_1','sc_e_0143'),('Map_30490','sc_e_0144'),('Map_30490_1','sc_e_0144'),('Map_30490_2','sc_e_0144')}:return 0
 master=json.loads((R/'docs/re/ff5_remaining_native_20261104.json').read_text(encoding='utf-8'))
 assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==master['source_script_hashes'][room+'/'+name]
 return (1<<16)|(TAG if room=='Map_30481_1'else 0)
