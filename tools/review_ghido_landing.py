"""Pin the already reviewed FFV shore script's actual same-map landing.

Original authoring tool only. Script bodies/native proofs remain outside Git.
This adds metadata through the existing arrival-override contract; no runtime
navigation/controller implementation is changed.
"""
from pathlib import Path
import hashlib,json,shutil,subprocess
from review_travel_bookkeeping import verify_movement

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
BASE=ROOT/'build_check/ghido_landing_20261029'
sha=lambda value:hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
reviews=json.loads((ROOT/'tools/world_event_reviews.json').read_text(encoding='utf-8'))
review=next(r for r in reviews if r['game']==5 and r['submap']=='Map_30381_7'and r['entity_id']==10 and r['script_name']=='sc_e_0436')
assert review['label']=='Passage'and review['review_grammar']=='player_movement'and not review['continuations']
scripts=[]
for line in (DUMP/'ff5_scripts.jsonl').read_text(encoding='utf-8-sig').splitlines():
    if 'sc_e_0436'not in line:continue
    source=json.loads(line)
    if (source['submap'],source['name'])==('Map_30381_7','sc_e_0436'):scripts.append(source)
assert len(scripts)==1
source=scripts[0];assert sha(source)==review['script_sha256']
verify_movement(5,{'script':source['name']},[source])
code=source['mnemonics']
assert [code[i]['operands']['iValues'][0]for i in (1,2,4)]==[7,18,29]
assert code[9]['mnemonic']=='SetPuppet'and code[9]['operands']['iValues'][:3]==[1,0,15]
assert code[16]['mnemonic']=='SetPos'and code[16]['operands']['iValues'][:3]==[12,35,0]
assert code[31]['mnemonic']=='SetPuppet'and code[31]['operands']['iValues'][:3]==[1,0,37]
assert code[39]['mnemonic']=='MoveTo'and code[39]['operands']['iValues'][:4]==[1,0,32,0]
assert code[40]['mnemonic']=='Exit'
# Resolve the actual actor, not just a coincidentally named copied script.
actors=[]
for line in (DUMP/'ff5_objects.jsonl').read_text(encoding='utf-8-sig').splitlines():
    actor=json.loads(line)
    if (actor['submap'],actor['group'],actor['id'],actor['type'])==('Map_30381_7',review['group'],review['id'],2):actors.append(actor)
assert len(actors)==1
actor=actors[0];assert actor['props']['entity_id']==10 and actor['props']['script_id']==2055 and actor['name']==review['raw']
BASE.mkdir(parents=True,exist_ok=True)
for name in ('maps.bin','warp_arrival_overrides.json'):
    original=ROOT/'data/ff5'/name;backup=BASE/('before_'+name)
    if not backup.exists():shutil.copy2(original,backup)
path=ROOT/'data/ff5/warp_arrival_overrides.json';rows=json.loads(path.read_text(encoding='utf-8'))
entry=dict(submap='Map_30381_7',entity_id=10,object_id=11,same_map=True,interaction_travel=False,
    approach_mask=0,destination_map=375,original_point=0,continuation='sc_e_0436',
    continuation_sha256=sha(source),arrival_cell=[12,32,1],arrival_baked_cell=[12,32,1],
    evidence='Complete reviewed controlled-player SetPos/MoveTo sequence, native absolute Y and keep-layer semantics; docs/re/SCRIPTED_SHORE_LANDING_20261029.md')
matches=[r for r in rows if r['submap']==entry['submap']and r['entity_id']==10]
if matches:assert matches==[entry]
else:
    rows.append(entry);path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
native=ROOT.parent/'tools/npc_geometry_context_20261029/ff5_native/provenance.json'
receipt=dict(source_sha256=sha(source),body_sha256=sha(code),native_provenance=str(native),
    trigger=dict(submap=actor['submap'],editor_id=actor['id'],entity_id=10,script_id=2055),
    controlled_player_setpos_index=16,controlled_player_final_move_index=39,final_cell=[1,12,32])
(BASE/'source_contract.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
print('FFV: exact shore actor/script verified; same-map landing12,32,L1 recorded; no game run')
