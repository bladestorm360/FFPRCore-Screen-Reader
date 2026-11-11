"""Exact final player landings for ten normal transfers/two escape triggers."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def author(o):
 import bake_maps
 room='Map_30600';rows=[]
 # Source identities and final actual player positions; all native requests
 # retain Upper. map_id=-1 in the compiled arc preserves431/631 identity.
 for entity,script,xy in(
  (143,'sc_e_0576',(32,26)),(141,'sc_e_0577',(40,16)),
  (133,'sc_e_0579',(42,27)),(131,'sc_e_0580',(67,49)),
  (129,'sc_e_0581',(63,41)),(127,'sc_e_0582',(40,34)),
  (125,'sc_e_0583',(77,41)),(123,'sc_e_0584',(48,32)),
  (78,'sc_e_0588',(70,33)),(76,'sc_e_0589',(89,35)),
  (52,'sc_e_0595',(75,29)),(53,'sc_e_0595',(75,29))):
  a=o.physical(431,entity);s=o.SCRIPTS[room,script]
  assert o.SCRIPT_NAMES[a['props']['script_id']]==script and bake_maps.layer_index(a['layer'])==2
  entries=[m['operands']['iValues'][2]for m in s['mnemonics']if m['mnemonic']=='SetPuppet'and m['operands']['iValues'][1]==-1001]
  landings=[]
  for entry in entries:
   segment=[]
   for m in s['mnemonics'][entry:]:
    if m['mnemonic']=='Exit':break
    if m['mnemonic']=='Lerp':segment.append(m['operands']['iValues'])
   if segment:landings.append(segment[-1][:3])
  assert landings==[[*xy,0]],(entity,landings)
  rows.append(dict(submap=room,entity_id=entity,object_id=a['id'],destination_map=431,original_point=0,
   continuation=script,continuation_sha256=hashlib.sha256(json.dumps(s,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
   same_map=True,ordinary_travel=True,arrival_cell=[*xy,2],arrival_baked_cell=[*xy,2],
   evidence='Exact player-only final landing; retained Upper layer and original normal/escape event activation. docs/re/FLOATING_CONTINENT_GUIDANCE_20261103.md'))
 return rows
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/warp_arrival_overrides.json';old=json.loads(p.read_text(encoding='utf-8'));rows=author(o)
 for row in rows:
  previous=[r for r in old if r['submap']==row['submap']and r.get('object_id')==row['object_id']]
  assert len(previous)<=1
  if'--check'in sys.argv:assert previous==[row]
  elif previous:old[old.index(previous[0])]=row
  else:old.append(row)
 if'--check'not in sys.argv:p.write_text(json.dumps(old,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Floating Continent:twelve exact source-qualified same-map transfers; normal/escape map identity retained.')
