"""Actual final Nikeah pose, rather than the earlier native entry point."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def author(o):
 import bake_maps,serpent_passages
 actor=o.physical(382,0,'entity_default');assert serpent_passages.destination(actor,'sc_e_1502',o.SCRIPTS)==(96,2,'sc_e_1502_3')
 body=o.SCRIPTS['Map_20121_4','sc_e_1502_3'];assert body['mnemonics'][16]['mnemonic']=='SetPos'and body['mnemonics'][16]['operands']['iValues'][:2]==[24,11]
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))['Map_20121_4'];ox,oy,*_=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']=='Map_20121_4'])
 entry,=[a for a in o.OBJECTS if a['submap']=='Map_20121_4'and a['type_name']=='PointIn'and a['props'].get('object_id')==2]
 layer=bake_maps.layer_index(entry['layer']);assert layer==2
 # The final absolute SetPos changes x/y only. Preserve the actual PointIn2
 # Upper layer rather than assuming the ground layer used by the cave rooms.
 return dict(submap=actor['submap'],entity_id=0,object_id=actor['id'],destination_map=96,original_point=2,continuation='sc_e_1502_3',continuation_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_cell=[24,11,layer],arrival_baked_cell=[24-ox,11-oy,layer],evidence='Exact last3D/world/city continuation, final player SetPos24,11/native PointIn2 Upper and full521; docs/re/CRESCENT_SERPENT_GUIDE_20261104.md')
if __name__=='__main__':
 import author_ff6_story_guides as o
 row=author(o);p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));existing=[v for v in data if v['submap']==row['submap']and v.get('object_id')==row['object_id']]
 if '--check'in sys.argv:assert existing==[row]
 else:
  if existing and existing!=[row]:
   assert len(existing)==1 and existing[0]['arrival_cell']==[24,11,1]and existing[0]['arrival_baked_cell']==[24,11,1]
   old=dict(existing[0]);old.update(arrival_cell=row['arrival_cell'],arrival_baked_cell=row['arrival_baked_cell'],evidence=row['evidence']);assert old==row
   data[data.index(existing[0])]=row
  elif not existing:data.append(row)
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Trench:one exact final ready Nikeah landing;all previous arrivals preserved.')
