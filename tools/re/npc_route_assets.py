"""Read only the exact NPC-route JSON assets from the installed catalog offline."""
from pathlib import Path
import base64,hashlib,json,struct,subprocess,sys
import UnityPy
ROOT=Path(__file__).resolve().parents[2];n=int(sys.argv[1])if len(sys.argv)>1 else 6
titles=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')
OUT=ROOT.parent/'tools/npc_geometry_context_20261029';OUT.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
cases=json.loads((OUT/'contexts.json').read_text())['cases'];needed=set()
for c in cases:
 if c['game']!=n:continue
 for script in c['local_motion']:
  for m in script['instructions']:
   ii=m.get('operands',{}).get('iValues')or[]
   if m['mnemonic']=='AssignMoveRoute'and ii and ii[0]==c['id']:needed.add((c['submap'],ii[1]))
assert 0<len(needed)<=32
assets=Path('D:/Games/SteamLibrary/steamapps/common')/(titles[n-1]+' PR')/(titles[n-1]+'_Data/StreamingAssets/aa')
catalog_path=assets/'catalog.json';catalog=json.loads(catalog_path.read_text(encoding='utf-8-sig'))
targets={p.lower()for p in catalog['m_InternalIds']if any(p.endswith('/'+sm+'/route_instruction_'+str(route)+'.json')for sm,route in needed)}
assert len(targets)==len(needed),(needed,targets)
data=base64.b64decode(catalog['m_EntryDataString']);count,=struct.unpack_from('<i',data)
entries=[struct.unpack_from('<7i',data,4+28*i)for i in range(count)]
data=base64.b64decode(catalog['m_BucketDataString']);count,=struct.unpack_from('<i',data);buckets=[];at=4
for _ in range(count):
 offset,size=struct.unpack_from('<ii',data,at);at+=8;buckets.append(struct.unpack_from('<'+'i'*size,data,at));at+=4*size
pending=[i for i,row in enumerate(entries)if catalog['m_InternalIds'][row[0]].lower()in targets]
seen=set();bundles=set()
while pending:
 i=pending.pop()
 if i in seen:continue
 seen.add(i);assert len(seen)<=512
 row=entries[i];path=catalog['m_InternalIds'][row[0]]
 if path.endswith('.bundle'):bundles.add(assets/'StandaloneWindows64'/path.rsplit('/',1)[-1])
 if row[2]>=0:pending.extend(buckets[row[2]])
env=UnityPy.load(*(str(p)for p in sorted(bundles)));result={}
for path,ptr in env.container.items():
 if path.lower()not in targets:continue
 assert ptr.type.name=='TextAsset',ptr.type.name
 value=ptr.read();text=value.m_Script
 if isinstance(text,bytes):text=text.decode('utf-8-sig')
 result[path]=dict(text_sha256=hashlib.sha256(text.encode()).hexdigest(),body=json.loads(text))
assert len(result)==len(targets)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
output=dict(game=n,catalog_sha256=sha(catalog_path),bundles={p.name:sha(p)for p in sorted(bundles)},assets=result)
(OUT/f'ff{n}_route_assets.json').write_text(json.dumps(output,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
for name,row in result.items():
 instructions=row['body'].get('instruction_list');assert isinstance(instructions,list)
 print(name.rsplit('/',1)[-1],len(instructions),'instructions',[(m['instruction_type'],m.get('cellx'),m.get('celly'),m.get('layer'))for m in instructions[:4]])
print(f'FF{n}: {len(result)} exact route assets/{len(bundles)} catalog bundles retained outside Git; no game run')
