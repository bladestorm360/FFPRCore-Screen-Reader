"""Four vine exits use the full player Lerp,not the temporary PointIn1."""
from pathlib import Path
import hashlib,json,sys
import bake_maps
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
CASES=[('Map_30531_3',12,486,'sc_map_30531_2_climb_1',3,[8,6,1]),('Map_30531_3',13,486,'sc_map_30531_2_climb_2',2,[12,6,1]),('Map_30531_3',14,486,'sc_map_30531_2_climb_3',4,[10,10,1]),('Map_30531_7',6,490,'sc_map_30531_6_climb',3,[7,9,0])]
def expected():
 proof=json.loads((R/'docs/re/ff5_gogo_native_20261104.json').read_text(encoding='utf-8'));sources={};objects={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['name']in{c[3]for c in CASES}:sources[s['name']]=s
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in{'Map_30531_2','Map_30531_6'}and o['type_name']=='PointIn':objects[o['submap'],o['props']['object_id']]=o
 rows=[]
 for room,e,dest,name,point,cell in CASES:
  s=sources[name];body=s['mnemonics'];assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][s['submap']+'/'+name]
  shift=1 if dest==490 else 0;assert body[21+shift]['mnemonic']=='SetPuppet'and body[21+shift]['operands']['iValues'][:3]==[0,-1001,25+shift]
  assert body[27+shift]['mnemonic']=='Lerp'and body[27+shift]['operands']['iValues'][:3]==cell[:2]+[0]
  assert body[31+shift]['mnemonic']=='Lerp'and body[31+shift]['operands']['iValues'][:3]==cell[:2]+[0]
  o=objects[s['submap'],point];assert[o['tx'],bake_maps.tile_row(o),{'BottomObject':0,'GroundObject':1,'UpperObject':2}[o['layer']]]==cell
  rows.append(dict(submap=room,entity_id=e,destination_map=dest,original_point=1,continuation=name,continuation_sha256=hashlib.sha256(json.dumps(s,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_point=point,arrival_baked_cell=cell,evidence='Full controlled-player vine continuation ends at this existing native PointIn;Lerp layer0 preserves the destination layer. docs/re/FF5_GOGO_20261104.md;accepted FFV Lerp ownership/layer proof in ISTORY_SHEEP_ENTRY_20261029.md.'))
 return rows
if __name__=='__main__':
 p=R/'data/ff5/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));wanted=expected();keys={(r['submap'],r['entity_id'])for r in wanted};old=[r for r in data if(r['submap'],r['entity_id'])in keys];assert not old or old==wanted
 if '--check'in sys.argv:assert old==wanted
 elif not old:data.extend(wanted);p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gogo four exact vine continuations:existing native destination points3/2/4/3;all old arrival indices retained.')
