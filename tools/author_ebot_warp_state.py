"""Source-pinned first-entry versus fresh-random transfer availability."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
def rows(o):
 import bake_maps,ebot_warps
 actor=o.physical(539,5,'entity_default');source=o.SCRIPTS[actor['submap'],'sc_e_0946'];assert len(ebot_warps.outcomes('ff6',actor,source))==13
 body=source['mnemonics'];assert [m['operands']['iValues'][:3]for m in body[1:3]]==[[205,1,12],[2,1,12]]
 infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,_,_=bake_maps.map_bounds(infos[actor['submap']],[a for a in o.OBJECTS if a['submap']==actor['submap']])
 base=dict(id=5,type=2,group=actor['group'],raw=actor['name'],cell=[bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy],map_ids=[539],area_id=67,entry=0,require_known=True,review='Ebot first-entry/random transfer; docs/re/EBOTS_ROCK_GUIDANCE_20261104.md')
 return[dict(base,warp_mask=[ebot_warps.RANDOM,0],code=[[2,1,205,0,-1,1,4],[2,9,2,0,-1,1,4],[1,'@action'],[6],[6]]),dict(base,warp_mask=[ebot_warps.RANDOM,ebot_warps.RANDOM],code=[[2,1,205,0,-1,1,3],[2,9,2,0,-1,1,3],[6],[1,'@action'],[6]])]
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'));g=rows(o);events=d['Map_30861_1'].setdefault('events',[])
 current=[r for r in events if'warp_mask'in r]
 if'--check'in sys.argv:assert current==g
 else:
  assert not current or current==g
  if not current:events.extend(g)
  p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Ebot: first entry deterministic; later/repeat pads require exact fresh-random phase; unknown fails closed.')
