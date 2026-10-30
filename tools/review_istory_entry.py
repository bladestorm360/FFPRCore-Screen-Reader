"""Pin the complete native sheep entry and its manual same-map landing.

Original authoring tool only; game scripts/native source remain outside Git.
"""
from pathlib import Path
import csv,hashlib,json,shutil
import bake_maps

ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
BASE=ROOT/'build_check/istory_entry_20261029';BASE.mkdir(exist_ok=True)
sha=lambda row:hashlib.sha256(json.dumps(row,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
sources=[]
for line in (DUMP/'ff5_scripts.jsonl').open(encoding='utf-8-sig'):
    if 'sc_e_0195'not in line:continue
    row=json.loads(line)
    if (row['submap'],row['name'])==('Map_20090','sc_e_0195'):sources.append(row)
assert len(sources)==1
source=sources[0];digest=sha(source)
assert digest=='f067ebffac1dbcd8dec7f1f74645eca72ce6baad0276b413ce5d74b632f3b212'
code=source['mnemonics']
allowed={'Nop','BranchByPlayerDirection','Call','Msg','Wait','Exit','SetPuppet','ExecPuppet','Return',
         'Hide','SetPos','LookAt','Show','PlayAnim','Lerp','CueAnim','PlaySE'}
assert {m['mnemonic']for m in code}<=allowed
assert code[1]['mnemonic']=='BranchByPlayerDirection'and code[1]['operands']['iValues'][:4]==[12,2,12,12]
assert code[46]['mnemonic']=='SetPuppet'and code[46]['operands']['iValues'][:3]==[1,0,57]
assert code[58]['mnemonic']=='Lerp'and code[58]['operands']['iValues'][:3]==[33,19,0]
assert code[65]['mnemonic']=='SetPuppet'and code[65]['operands']['iValues'][:3]==[1,-1001,71]
assert code[72]['mnemonic']=='SetPos'and code[72]['operands']['iValues'][:3]==[33,19,0]
objects=[]
for line in (DUMP/'ff5_objects.jsonl').open(encoding='utf-8-sig'):
    if 'Map_20090'not in line:continue
    row=json.loads(line)
    if row['submap']=='Map_20090':objects.append(row)
actor=next(o for o in objects if o['id']==79)
p=actor['props'];assert actor['type_name']=='NPC'and p['entity_id']==21 and p['script_id']==354
assert p['move_type']==1 and p['move_step']==0 and p['accept_action_direction']==14
with (DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig')as stream:
    master={int(r['id']):r['script_name']for r in csv.DictReader(line for line in stream if line.strip())if r['id']}
assert master[354]=='sc_e_0195'
info=json.loads((DUMP/'ff5_submaps.json').read_text(encoding='utf-8'))['Map_20090']
bounds=bake_maps.map_bounds(info,objects);assert bounds[:2]==(0,0)
assert (actor['tx'],bake_maps.tile_row(actor))==(36,19)
for name in ('maps.bin','warp_arrival_overrides.json'):
    target=BASE/('before_'+name)
    if not target.exists():shutil.copy2(ROOT/'data/ff5'/name,target)
entry=dict(submap='Map_20090',entity_id=21,object_id=79,same_map=True,interaction_travel=True,
    approach_mask=8,destination_map=118,original_point=0,continuation='sc_e_0195',continuation_sha256=digest,
    arrival_cell=[33,19,1],arrival_baked_cell=[33,19,1],
    evidence='Complete east-facing sheep interaction; controlled-player Lerp preserves current layer at33,19; docs/re/ISTORY_SHEEP_ENTRY_20261029.md')
path=ROOT/'data/ff5/warp_arrival_overrides.json';rows=json.loads(path.read_text(encoding='utf-8'))
matches=[r for r in rows if r['submap']=='Map_20090'and r['entity_id']==21]
if matches:assert matches==[entry]
else:rows.append(entry);path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
receipt=dict(source_sha256=digest,actor_sha256=sha(actor),entity=21,editor=79,script=354,
             native_provenance='../tools/npc_geometry_context_20261029/ff5_sheep_entry_native/provenance.json',
             trigger_cell=[1,36,19],approach_cell=[1,35,19],facing='East',approach_mask=8,landing=[1,33,19])
(BASE/'source_contract.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
print('Istory native sheep/script verified; west approach/East facing and33,19,L1 landing recorded')
