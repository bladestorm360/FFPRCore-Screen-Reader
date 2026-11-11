"""Full actual manual raft landing, preserving native geometry and all old arcs."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def author(o):
 import bake_maps,solitary_passage
 solitary_passage.validate(o.SCRIPTS)
 actor=o.physical(597,3,'ev_e_0626');assert solitary_passage.identity(actor)
 body=o.SCRIPTS['Map_10020','sc_e_0626_3']
 submaps=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(submaps['Map_10020'],[a for a in o.OBJECTS if a['submap']=='Map_10020'])
 return dict(submap=actor['submap'],entity_id=3,object_id=actor['id'],destination_map=2,original_point=1,continuation='sc_e_0626_3',continuation_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_cell=[146,212,1],arrival_baked_cell=[146-ox,212-oy,1],interaction_travel=True,ordinary_travel=True,evidence='Both native0626/0630 branches share full world626_3/player SetPos146,212 and actual306;not intermediate beach192 or default World PointIn1. docs/re/SOLITARY_ISLAND_GUIDE_20261104.md')
if __name__=='__main__':
 import author_ff6_story_guides as o
 row=author(o);p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));old=[r for r in data if r['submap']==row['submap']and r.get('object_id')==row['object_id']]
 if '--check'in sys.argv:assert old==[row]
 else:
  assert not old or old==[row]
  if not old:data.append(row)
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Solitary raft:one exact manual ordinary crossing/both outcomes/full World2 arrival146,212;native geometry/actors and previous overrides preserved.')
