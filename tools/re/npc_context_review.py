"""Exact finite22 NPC geometry contexts; source-bearing output stays outside Git."""
from pathlib import Path
from collections import Counter,defaultdict
import csv,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
import bake_maps
from check_scripted_bakes import decode
DUMP=ROOT.parent/'tools/mapdump';OUT=ROOT.parent/'tools/npc_geometry_context_20261029'
OUT.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
def rows(path):
 with path.open(encoding='utf-8-sig')as file:return list(csv.DictReader(file))
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def body_hash(script):return hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
with (ROOT/'build_check/map_event_audit_20261028/state_geometry.tsv').open(encoding='utf-8')as file:
 pending=[r for r in csv.DictReader(file,delimiter='\t')if r['classification']=='needs_native_or_dynamic_context']
assert len(pending)==22
result=[];inputs={}
for n in (2,5,6):
 selected=[r for r in pending if int(r['game'])==n];rooms={r['submap']for r in selected}
 obj_file=DUMP/f'ff{n}_objects.jsonl';script_file=DUMP/f'ff{n}_scripts.jsonl'
 objects=[]
 with obj_file.open(encoding='utf-8-sig')as file:
  for line in file:
   r=json.loads(line)
   if r['submap']in rooms:objects.append(r)
 master={int(r['id']):r['script_name']for r in rows(DUMP/f'ff{n}_master/script.txt')if r['id']}
 maps=rows(DUMP/f'ff{n}_master/map.txt');baked,_=decode(ROOT/f'data/ff{n}/maps.bin')
 by_name=defaultdict(list);local=defaultdict(list)
 with script_file.open(encoding='utf-8-sig')as file:
  for line in file:
   s=json.loads(line);by_name[s['name']].append(s)
   if s['submap']in rooms:local[s['submap']].append(s)
 for row in selected:
  sm=row['submap'];native_id=int(row['id'])
  actual=[o for o in objects if o['submap']==sm and o['group']==row['group']and o['props'].get('entity_id')==native_id and o['name']==row['raw']]
  assert len(actual)==1,(n,row,len(actual));npc=actual[0]
  source=master.get(npc['props'].get('script_id'));choices=by_name.get(source,[])
  candidates=[s for s in choices if s['submap']==sm]
  if not candidates and choices and len({body_hash(s)for s in choices})==1:candidates=choices[:1]
  aliases=[r for r in maps if r['map_name']==sm]
  cells=[o for o in baked[sm]['objects']if o[0]in (bake_maps.BAKED_TYPES['NPC'],bake_maps.BAKED_TYPES['ShopNPC'])and o[8]==native_id and o[10]==row['raw']and o[11]==row['group']]
  assert len(cells)==1,(n,row,'bake identity')
  # Use the same native crop as the production baker. Route JSON positions are
  # map cells, whereas owned navigation uses cells relative to this rectangle.
  room_objects=[o for o in objects if o['submap']==sm]
  _,width,height,*_=baked[sm]['shape']
  bounds=bake_maps.map_bounds(dict(width=width,height=height),room_objects)
  region_type={2:'MoveArea',3:'Polyline'}.get(npc['props'].get('move_type'))
  movement_regions=[o for o in room_objects if region_type and o['type_name']==region_type and
     o['props'].get('object_id')==npc['props'].get('link_object_id') and o['layer']==npc['layer'] and o['group']==npc['group']]
  # These are authored region/line candidates, not a simulation of native
  # collision or timing. Keep that distinction in the owned consumer report.
  region_cells=[]
  for area in movement_regions:
   layer=bake_maps.layer_index(area['layer']);assert layer>=0
   if area['type_name']=='MoveArea':
    for y in range(int(area['ty']),int(area['ty'])+int(area['h'])//16):
     for x in range(int(area['tx']),int(area['tx'])+int(area['w'])//16):region_cells.append([layer,x-bounds[0],y-bounds[1]])
   else:
    for point in area.get('polyline',[]):
     region_cells.append([layer,int((area['x']+point['x'])//16)-bounds[0],int((area['y']+point['y'])//16)-bounds[1]])
  detail=dict(game=n,submap=sm,id=native_id,group=row['group'],raw=row['raw'],source=npc,baked=cells[0],bounds=list(bounds),
    map_aliases=aliases,script=source,body_candidates=[dict(submap=s['submap'],name=s['name'],sha256=body_hash(s),operations=dict(Counter(m['mnemonic']for m in s['mnemonics'])))for s in candidates],
    pointins=[o for o in baked[sm]['objects']if o[0]==bake_maps.BAKED_TYPES['PointIn']],
    relays=[o for o in room_objects if o['type_name']=='RelayInteraction'],movement_regions=movement_regions,region_candidates=region_cells,
    local_motion=[])
  if n==5 and sm=='Map_20090':
   # Complete named sheep-kick continuation: track1 is the controlled entity0;
   # its Lerp endpoint, followed by the visible party clone at the same point,
   # supplies a real prerequisite entry rather than a fabricated walking edge.
   s=next(s for s in by_name['sc_e_0195']if s['submap']==sm);ms=s['mnemonics']
   assert ms[46]['mnemonic']=='SetPuppet'and ms[46]['operands']['iValues'][:3]==[1,0,57]
   assert ms[58]['mnemonic']=='Lerp'and ms[58]['operands']['iValues'][:3]==[33,19,0]
   parents=[o for o in room_objects if master.get(o['props'].get('script_id'))==s['name']]
   assert len(parents)==1
   detail['manual_entry']=dict(cell=[1,33-bounds[0],19-bounds[1]],name=s['name'],sha256=body_hash(s),
       track_index=46,position_index=58,prerequisite=parents[0])
  if (n==2 and sm=='Map_30131_7')or(n==5 and sm=='Map_30381_7'):
   scripts={'sc_e_0082'}if n==2 else {'sc_e_0117','sc_e_0258','sc_e_0436'}
   detail['scene_controls']=[o for o in room_objects if o['type_name']=='Event'and master.get(o['props'].get('script_id'))in scripts]
  if n==5 and sm=='Map_30381_7':
   s=next(s for s in by_name['sc_e_0436']if s['submap']==sm);ms=s['mnemonics']
   assert ms[9]['operands']['iValues'][:3]==[1,0,15]and ms[16]['mnemonic']=='SetPos'and ms[16]['operands']['iValues'][:3]==[12,35,0]
   assert ms[31]['operands']['iValues'][:3]==[1,0,37]and ms[39]['mnemonic']=='MoveTo'and ms[39]['operands']['iValues'][:3]==[1,0,32]
   detail['manual_entry']=dict(cell=[1,12-bounds[0],32-bounds[1]],name=s['name'],sha256=body_hash(s),track_index=31,position_index=39)
  for s in local[sm]:
   motion=[dict(index=i,**m)for i,m in enumerate(s['mnemonics'])if any(k in m['mnemonic'].lower()for k in ('pos','move','route','jump','collision','entity','puppet'))or m['mnemonic']=='Call']
   if motion:detail['local_motion'].append(dict(name=s['name'],sha256=body_hash(s),instructions=motion))
  result.append(detail)
  print(f"FF{n} {sm}/{native_id} {row['raw']}: layer/cell={cells[0][1:4]}, action={npc['props'].get('action_id')}, collision={npc['props'].get('collision')}, script={source}, aliases={[r['id']for r in aliases]}, entries={len(detail['pointins'])}, relays={len(detail['relays'])}")
 inputs[str(n)]={str(p.relative_to(ROOT.parent)):digest(p)for p in (obj_file,script_file,DUMP/f'ff{n}_master/map.txt',DUMP/f'ff{n}_master/script.txt',ROOT/f'data/ff{n}/maps.bin')}
(OUT/'contexts.json').write_text(json.dumps(dict(inputs=inputs,cases=result),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
assert len(result)==22
print('22 exact source/bake contexts retained outside Git; no classification or shipped change yet')
