"""Exact permanent FFV current rectangles as original navigation metadata."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
CURRENT_TAG=1<<24
def areas(game,objects):
 if game!='ff5':return []
 proof=json.loads((R/'docs/re/ff5_desert_native_20261105.json').read_text(encoding='utf-8'));wanted={a['editor']:a for a in proof['sliding_areas']};result=[]
 for o in objects:
  if o['submap']!='Map_30170'or o['type_name']!='SlidingFloorGimmickArea':continue
  a=wanted.pop(o['id']);p=o['props'];assert a['cell']==[o['tx'],o['ty']]and a['size']==[o['w'],o['h']]and a['entity']==p['entity_id']and a['direction']==p['sliding_direction']
  assert o['gid']is None and o['layer']=='UpperObject'and o['group']=='entity_default'and p['gimmick_enabled']and not p['switch_seconds']and not p['all_layers']and p['sliding_direction']in(1,2,4,8)
  assert o['w']%16==o['h']%16==0 and 0<=o['tx']<o['tx']+o['w']//16<=64 and 0<=o['ty']<o['ty']+o['h']//16<=64
  # This metadata is not a native actor,entity ID,destination or label. The
  # source ID in d is retained only for offline attribution/conformance.
  result.append(dict(o,type_name='CurrentArea',name='',group='',props=dict(direction=p['sliding_direction'],source_entity=p['entity_id'])))
 assert not wanted and len(result)==274
 return result
