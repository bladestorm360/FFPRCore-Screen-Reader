"""Source-qualified current transfers,exact native completion and four NPC poses."""
from pathlib import Path
import hashlib,json,sys
import bake_map_state as b
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';ROOM='Map_30170';TAG=1<<24
read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def main():
 check='--check'in sys.argv;proof=read(R/'docs/re/ff5_desert_native_20261105.json');scripts={};objects=[]
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']==ROOM:scripts[s['name']]=s
 actual={ROOM+'/'+k:hashlib.sha256(json.dumps(v['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()for k,v in scripts.items()}
 assert actual==proof['source_script_hashes']
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']==ROOM:objects.append(o)
 region=b.bake_maps.map_bounds(read(D/'ff5_submaps.json')[ROOM],objects);assert tuple(region)==(0,0,64,64)
 path=R/'data/ff5/warp_arrival_overrides.json';arrivals=read(path);new=list(arrivals);guards=[]
 # Arrival indices are baked into the graph. Keep previously reviewed rows
 # in their original positions when other maps have appended corrections.
 existing={}
 for index,row in enumerate(arrivals):
  if row['submap']==ROOM and row.get('automatic_current'):
   assert row['object_id'] not in existing
   existing[row['object_id']]=index
 left={13,15,17,20,21,829,502,504};right={14,16,22,23,24,25,828,503,505}
 for o in objects:
  id=o['props'].get('entity_id')
  if o['type_name']!='Event'or id not in left|right:continue
  fact,=[a for a in proof['actors']if a['editor']==o['id']];script=scripts[fact['script']]
  index=existing.pop(o['id'],len(new))
  arrival=dict(submap=ROOM,entity_id=id,object_id=o['id'],destination_map=286,original_point=0,continuation=script['name'],continuation_sha256=hashlib.sha256(json.dumps(script,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_cell=[52 if id in left else 53,14,2],same_map=True,ordinary_travel=True,automatic_current=True,evidence='Full controlled-player current chains after complete Sandworm callback59. docs/re/FF5_DESERT_20261105.md')
  new[index:index+1]=[arrival]
  guards.append(dict(id=id,type=2,group=o['group'],raw=o['name'],cell=[2,o['tx'],b.bake_maps.tile_row(o)],map_ids=[286],area_id=35,entry=0,require_known=True,warp_mask=[TAG,TAG],destination=[-1,-100000-index],code=[[2,0,59,1,-1,1,3],[1,'@action'],[6],[6]],review='Only the full native battle callback sets59;the original scene actor is unchanged. docs/re/FF5_DESERT_20261105.md'))
 assert len(guards)==17 and not existing
 statepath=R/'data/ff5/mapstate.json';state=read(statepath);record=state[ROOM];assert record['map_ids']==[286];assert record['area_id']==35
 gates=[]
 for o in objects:
  if o['type_name']=='Event':gates.append(dict(id=o['props']['entity_id'],type=2,group=o['group'],raw=o['name'],cell=[2,o['tx'],b.bake_maps.tile_row(o)],size=[1,1],review='Native pushback before prerequisite58 or completed59;approach remains selectable. docs/re/FF5_DESERT_20261105.md'))
 assert len(gates)==19
 code=b.initializer_code(scripts['sc_map_30170'],'ff5',region);assert len(code)==len(record['code'])==133
 for x,y in zip(record['code'],code):
  if x!=y:assert x[0]==y[0]==10 and x[:4]==y[:4]and x[2]in(27,28,61,62)and x[4:]==[-2147483648]*2
 if check:assert arrivals==new and record.get('events')==guards and record.get('story_actor_gates')==gates and record['code']==code
 else:
  path.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');record.update(code=code,events=guards,story_actor_gates=gates);statepath.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Desert:17 guarded automatic transfers,19 exact prerequisite barriers,four initializer NPC placements;all earlier arrivals preserved.')
if __name__=='__main__':main()
