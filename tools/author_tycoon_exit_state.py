"""Grant the real floor transfer only on native branches that reach ChangeMap."""
from pathlib import Path
import json,sys
import bake_maps,tycoon_exit
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def main():
 check='--check'in sys.argv;room='Map_20051_2';objects=[];script=None
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']==room:objects.append(o)
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']==room and s['name']=='sc_20051_2_to_20051_1':script=s
 actor,=[o for o in objects if o['type_name']=='Event'and o['props'].get('entity_id')==34];assert tycoon_exit.flags('ff5',actor,script)==tycoon_exit.TAG|(1<<16)
 ox,oy,*_=bake_maps.map_bounds(read(D/'ff5_submaps.json')[room],objects)
 guard=dict(id=34,type=2,group=actor['group'],raw=actor['name'],cell=[1,actor['tx']-ox,bake_maps.tile_row(actor)-oy],map_ids=[73],area_id=17,entry=0,require_known=True,warp_mask=[tycoon_exit.TAG,tycoon_exit.TAG],destination=[72,4],code=[[2,0,127,0,-1,1,2],[3,4],[2,0,128,0,-1,1,4],[6],[1,'@action'],[6]],review='No-return ChangeScript0128 replaces the caller at127/not128. Only pre127 or completed128 reaches actual72/4. docs/re/FF5_TYCOON_20261105.md')
 p=R/'data/ff5/mapstate.json';d=read(p);rows=d[room].get('events',[]);existing=[r for r in rows if r.get('warp_mask')==[tycoon_exit.TAG,tycoon_exit.TAG]]
 if check:assert existing==[guard]
 else:
  assert not existing or existing==[guard]
  if not existing:d[room]['events']=rows+[guard];p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Tycoon suite:conditional actual72/4 exit;original scene contact remains reachable and unconsumed;no puzzle guide.')
if __name__=='__main__':main()
