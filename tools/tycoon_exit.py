"""One conditional suite exit;the no-return story replacement is not travel."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1];TAG=1<<25
def flags(game,o,script):
 if game!='ff5'or o['submap']!='Map_20051_2'or o['props'].get('entity_id')!=34:return 0
 assert o['type_name']=='Event'and o['name']=='屋外へ'and o['group']=='entity_default'and script['name']=='sc_20051_2_to_20051_1'
 body=script['mnemonics'];assert len(body)==9 and body[5]['mnemonic']=='ChangeScript'and body[5]['operands']['sValues'][0]=='sc_e_0128'and body[5]['operands']['iValues'][0]==0
 assert body[2]['mnemonic']==body[7]['mnemonic']=='ChangeMap'and all(body[i]['operands']['iValues'][:2]==[72,4]for i in(2,7))
 master=json.loads((R/'docs/re/ff5_remaining_native_20261104.json').read_text(encoding='utf-8'));assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==master['source_script_hashes'][o['submap']+'/'+script['name']]
 return TAG|(1<<16)
