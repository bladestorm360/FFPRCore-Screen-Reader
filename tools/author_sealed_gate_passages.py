"""Exact interactive forced-fall landing; no recovered implementation emitted."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def author(o):
 m=json.loads((ROOT/'docs/re/sealed_gate_native_20261103.json').read_text(encoding='utf-8'));room='Map_30531_4';name='sc_e_0446';a=o.physical(408,69);s=o.SCRIPTS[room,name]
 assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==m['script_sha256'][room+'/'+name]
 assert a['type_name']=='Entity'and a['props']['action_id']==2
 return dict(submap=room,entity_id=69,object_id=a['id'],destination_map=408,original_point=0,continuation=name,
  continuation_sha256=hashlib.sha256(json.dumps(s,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
  same_map=True,interaction_travel=True,ordinary_travel=True,arrival_cell=[58,24,1],arrival_baked_cell=[45,24,1],
  evidence='Native encoded puppet layer2 converts to Ground/baked1; exact forced-fall player landing, distinct from camera puppets and control tile. docs/re/SEALED_GATE_GUIDANCE_20261103.md')
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/warp_arrival_overrides.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o);old=[r for r in d if r['submap']==g['submap']and r.get('object_id')==g['object_id']]
 if'--check'in sys.argv:assert old==[g]
 else:
  assert len(old)<=1
  if old:d[d.index(old[0])]=g
  else:d.append(g)
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Sealed Gate69: real manual operation and Ground45,24 forced-fall landing (native encoded layer2).')
