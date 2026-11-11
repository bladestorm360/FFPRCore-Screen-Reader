"""Exact native factory transfers, including direction-dependent ladder ends.

Only reviewed scripts/physical identities are accepted. The existing private
manual/automatic arc consumer owns interaction, collision and native resume.
"""
from pathlib import Path
import hashlib,json,struct,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
 import bake_maps
 from check_scripted_bakes import decode
 proof=json.loads((ROOT/'docs/re/magitek_passages_native_20261104.json').read_text(encoding='utf-8'));maps,_=decode(ROOT/'data/ff6/maps.bin');infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));rows=[]
 for map_id,ids in((396,(46,57,33,31,29)),(643,(57,33,29)),(397,(62,))):
  room=o.MAPS[map_id]['map_name'];data=maps[room];width,height=data['shape'][1:3];grid=[]
  for packed in data['grids'][:3]:
   cells=[]
   for at in range(0,len(packed),3):run,value=struct.unpack_from('<HB',packed,at);cells.extend([value]*run)
   grid.append(cells)
  def walk(layer,x,y):return 0<=layer<3 and 0<=x<width and 0<=y<height and bool(grid[layer])and bool(grid[layer][y*width+x])
  ox,oy,*_=bake_maps.map_bounds(infos[room],[a for a in o.OBJECTS if a['submap']==room])
  for entity in ids:
   a=o.physical(map_id,entity);script=o.SCRIPTS[room,o.SCRIPT_NAMES[a['props']['script_id']]];assert sha(script['mnemonics'])==proof['source_script_hashes'][room+'/'+script['name']]
   variants={};manual=a['type_name']=='Entity';assert manual or(a['type_name']=='Event'and entity==62 and map_id==397)
   for facing in range(4):
    side=(facing+2)%4;accept=a['props'].get('accept_action_direction',15) or 15
    if manual and not(accept&(1<<side)):continue
    dx,dy=((0,-1),(1,0),(0,1),(-1,0))[facing];start=[a['tx']-(dx if manual else 0),bake_maps.tile_row(a)-(dy if manual else 0),bake_maps.layer_index(a['layer'])]
    if not walk(start[2],start[0]-ox,start[1]-oy):continue
    ms=script['mnemonics'];pc=0;stack=[];puppets={};executed=[]
    for _ in range(256):
     assert 0<=pc<len(ms);m=ms[pc];pc+=1;op=m['mnemonic'];v=m.get('operands')or{};iv=list(v.get('iValues')or[])+[0]*8;sv=list(v.get('sValues')or[])+['']*8
     if op=='BranchByPlayerDirection':pc=iv[facing]
     elif op=='Call':stack.append(pc);pc=iv[0]
     elif op=='Return':
      assert stack;pc=stack.pop()
     elif op=='Exit':break
     elif op=='SetPuppet':puppets[iv[0]]=(iv[1],iv[2])
     elif op=='ExecPuppet':assert iv[0]==0;executed.extend(entry for actor,entry in puppets.values()if actor==-1001);puppets.clear()
     elif op=='Calc':assert sv[:3]==['MapLocalVariable','←','imm']and iv[0]in(1,2,3)and iv[1]in(0,1)
     else:assert op in('Nop','PlaySE','Wait'),(script['name'],op)
    else:raise AssertionError('Unbounded native transfer')
    assert len(executed)==1;landing=start[:]
    for m in ms[executed[0]:executed[0]+128]:
     op=m['mnemonic'];iv=list((m.get('operands')or{}).get('iValues')or[])+[0]*8
     if op=='Exit':break
     if op in('Lerp','SetPos')and iv[3]==0:
      if iv[0]!=-1:landing[0]=iv[0]
      if iv[1]!=-1:landing[1]=iv[1]
      if iv[2]>0:landing[2]=iv[2]-1
     elif op=='SetPos'and iv[3]==1:assert iv[:2]==[0,0]and iv[4]==-1001 and iv[2]in(2,3);landing[2]=iv[2]-1
     elif op=='Move':assert 0<=iv[0]<4;x,y=((0,-1),(1,0),(0,1),(-1,0))[iv[0]];landing[0]+=x*iv[1];landing[1]+=y*iv[1]
     else:assert op in('Nop','Wait','PlaySE','CueAnim','LookAt'),(script['name'],op)
    else:raise AssertionError('Unbounded player segment')
    assert walk(landing[2],landing[0]-ox,landing[1]-oy),(map_id,entity,facing,'invalid native landing',landing)
    variants[tuple(landing)]=variants.get(tuple(landing),0)|(1<<side)
   assert variants,(map_id,entity,'no physical operation side')
   for landing,mask in variants.items():
    rows.append(dict(submap=room,entity_id=entity,object_id=a['id'],same_map=True,interaction_travel=manual,ordinary_travel=not manual,approach_mask=mask if manual else 0,destination_map=map_id,original_point=0,continuation=script['name'],continuation_sha256=sha(script),arrival_cell=list(landing),arrival_baked_cell=[landing[0]-ox,landing[1]-oy,landing[2]],evidence='Exact factory player movement/relative layer and physical approach side; native local writes are visual flap receipts and remain native-owned. docs/re/MAGITEK_PASSAGES_20261104.md'))
 return rows
if __name__=='__main__':
 import author_ff6_story_guides as o
 rows=author(o);p=ROOT/'data/ff6/warp_arrival_overrides.json';old=json.loads(p.read_text(encoding='utf-8'))
 for row in rows:
  matches=[r for r in old if(r['submap'],r.get('object_id'),r.get('approach_mask',0))==(row['submap'],row['object_id'],row['approach_mask'])]
  if'--check'in sys.argv:assert matches==[row]
  elif matches:assert len(matches)==1;old[old.index(matches[0])]=row
  else:old.append(row)
 if'--check'not in sys.argv:p.write_text(json.dumps(old,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Factory:',len(rows),'source-qualified directional ladder/conveyor arcs; native geometry/controls unchanged.')
