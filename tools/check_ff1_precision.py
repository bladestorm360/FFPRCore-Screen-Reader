"""Independent native-data proof of the Ice Cavern's intended floor route.

Historical filename check_ff1_precision.py remains a canonical entry point.
The center-clearance shortcut proof was invalidated by actual gameplay. This
check uses complete tile connectivity and actual scripted arrivals instead.
No game is executed; proprietary script bodies remain in the external dump.
"""
from pathlib import Path
from collections import deque
import hashlib,json,sys
try:import bake_maps as bake
except ImportError:
 sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'));import bake_maps as bake
ROOT=Path(bake.__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
import bake_map_state

def sha(path):
 digest=hashlib.sha256()
 with path.open('rb')as stream:
  for chunk in iter(lambda:stream.read(1024*1024),b''):digest.update(chunk)
 return digest.hexdigest()

def main():
 dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY PR/GameAssembly.dll')
 assert sha(dll)=='33f001b4420bea06b44740c07c4afcf92547e7196bdfd3ec27ae5feaf0229108'
 assert sha(ROOT.parent/'ff1/dump.cs')=='8313bacc58a606108db7dc0786b4fc5bcd57dc07e171cb8b051c2a68cd817740'
 assert json.loads((ROOT/'data/ff1/precision_passages.json').read_text(encoding='utf-8'))['passages']==[], 'Retired Ice shortcut must not return'
 objects=[]
 with (DUMP/'ff1_objects.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   obj=json.loads(line)
   if obj['submap'].startswith('Map_30061_'):objects.append(obj)
 room=[o for o in objects if o['submap']=='Map_30061_4']
 pits=[o for o in room if o['type_name']=='Event'and bake.trap_identifier(o.get('name')or'')]
 ids=[51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,70,71,72,73]
 assert sorted(o['props']['entity_id']for o in pits)==ids
 assert all(o['props']['action_id']==2 and o['layer']=='BottomObject'for o in pits)
 coords={(o['tx'],bake.tile_row(o))for o in pits};assert len(coords)==19
 rows=bake.collision_rows(json.loads((DUMP/'ff1_grids.json').read_text(encoding='utf-8'))['Map_30061_4']['collision'],0)
 solids={(o['tx'],bake.tile_row(o))for o in room if (o.get('props')or{}).get('collision')and o['type_name']in('TreasureBox','AnimEntity','CollisionEntity')}
 floor={(x,y)for y,row in enumerate(rows)for x,v in enumerate(row)if v=='0'}-coords-solids
 def flood(start):
  assert start in floor
  seen={start};queue=deque([start])
  while queue:
   x,y=queue.popleft()
   for p in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
    if p in floor and p not in seen:seen.add(p);queue.append(p)
  return seen
 outside=flood((23,18));island=flood((20,17))
 assert (18,18)in outside and (18,13)in outside and (19,16)not in outside
 assert (19,16)in island and not(outside&island)
 # Check every contact of the island: each open outgoing floor tile is a real
 # pit or the solid Levistone. No unexamined gap on either side is accepted.
 boundary={(x+dx,y+dy)for x,y in island for dx,dy in ((-1,0),(1,0),(0,-1),(0,1))} - island
 open_boundary={p for p in boundary if rows[p[1]][p[0]]=='0'}
 assert open_boundary and open_boundary<=coords|solids
 arrival=[o for o in room if o['type_name']=='PointIn'and o['props'].get('object_id')==2]
 assert len(arrival)==1 and(arrival[0]['tx'],bake.tile_row(arrival[0]))==(20,17)and arrival[0]['layer']=='BottomObject'
 b1holes=[o for o in objects if o['submap']=='Map_30061_1'and o['type_name']=='Event'and bake.trap_identifier(o.get('name')or'')]
 assert len(b1holes)==1 and b1holes[0]['props']['entity_id']==38
 h,rs=bake.read_csv(str(DUMP/'ff1_master/script.txt'));cols={v:i for i,v in enumerate(h)}
 names={int(r[cols['id']]):r[cols['script_name']]for r in rs}
 wanted={names[o['props']['script_id']]for o in pits+b1holes}|{names[407],'sc_enc_30061_1_2'};programs={}
 with(DUMP/'ff1_scripts.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   value=json.loads(line)
   if value['map']=='Map_30061'and value['name']in wanted:programs[value['name']]=value
 transitions={}
 for obj in pits+b1holes:
  name=names[obj['props']['script_id']];program=programs[name]
  targets={tuple((m.get('operands')or{}).get('iValues',[])[:2])for m in program.get('mnemonics',[])if m['mnemonic']=='ChangeMap'}
  expected=(91,2)if obj in b1holes else(92,99)
  assert targets=={expected},(name,targets,expected)
  transitions[str(obj['props']['entity_id'])]=list(expected)
 encounter=programs[names[407]];after=programs['sc_enc_30061_1_2']
 fights=[m['operands']for m in encounter['mnemonics']if m['mnemonic']=='Encount']
 assert len(fights)==1 and fights[0]['iValues'][0]==123 and fights[0]['sValues'][0]==after['name']
 puppets=[m['operands']['iValues']for m in after['mnemonics']if m['mnemonic']=='SetPuppet']
 assert len(puppets)==1 and puppets[0][:3]==[0,26,9]
 assert bake_map_state.puppet_navigation_effect(after,9,(0,0),False)==[0,-2147483648,-2147483648]
 assert any(m['mnemonic']=='ExecPuppet'and m['operands']['iValues'][0]==0 for m in after['mnemonics'])
 output=ROOT/'build_check/ff1_precision_bake';output.mkdir(exist_ok=True)
 bake.bake_game('ff1',str(output),check=True)
 assert(output/'ff1/maps.bin').read_bytes()==(ROOT/'data/ff1/maps.bin').read_bytes()
 facts=dict(status='independent physical floor topology/native drop arrivals/reproducible bake pass',
  outer_component=len(outside),island_component=len(island),island_boundary_contacts=len(open_boundary),pits=19,
  native_scripted_drops=transitions,native_encounter={'battle':123,'actor':26,'post_battle_hidden':True},
  game_launched_or_probed=False,bake_sha256=sha(ROOT/'data/ff1/maps.bin'))
 (ROOT/'build_check/ff1_precision_geometry.json').write_text(json.dumps(facts,indent=2)+'\n')
 print(f'PASS19 real B2 pits + B1 hole; outer{len(outside)} / island{len(island)} cardinal components disconnected, all{len(open_boundary)} island contacts checked;20 native drop scripts / real arrival20,17 / exact129-room bake. No runtime claim.')
if __name__=='__main__':main()
