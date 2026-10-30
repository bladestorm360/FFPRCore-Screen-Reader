"""Inspect exact field-effect prefab collider defaults; never load the game.

Only this original extractor is kept in Git; asset trees remain outside Git.
"""
from pathlib import Path
import base64,hashlib,json,struct,subprocess,sys
import UnityPy

root=Path(__file__).resolve().parents[2]
number=int(sys.argv[1]) if len(sys.argv)>1 else 5
titles=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')
assets=Path('D:/Games/SteamLibrary/steamapps/common')/(titles[number-1]+' PR')/(titles[number-1]+'_Data/StreamingAssets/aa')
out=root.parent/'tools/map_event_audit_20261028'/f'ff{number}_effect_native'
out.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
catalog_path=assets/'catalog.json';catalog=json.loads(catalog_path.read_text(encoding='utf-8-sig'))
targets={path.lower() for path in catalog['m_InternalIds'] if path.endswith('/FieldEffectBoardEntity.prefab') or path.endswith('/FieldEffectParticleEntity.prefab')}
assert len(targets)==2
raw=base64.b64decode(catalog['m_EntryDataString']);count,=struct.unpack_from('<i',raw)
entries=[struct.unpack_from('<7i',raw,4+28*i)for i in range(count)]
raw=base64.b64decode(catalog['m_BucketDataString']);count,=struct.unpack_from('<i',raw);buckets=[];at=4
for _ in range(count):
    offset,n=struct.unpack_from('<ii',raw,at);at+=8
    buckets.append(struct.unpack_from('<'+'i'*n,raw,at));at+=4*n
pending=[i for i,row in enumerate(entries)if catalog['m_InternalIds'][row[0]].lower()in targets]
seen=set();bundles=set()
while pending:
    index=pending.pop()
    if index in seen:continue
    seen.add(index);assert len(seen)<256
    row=entries[index];path=catalog['m_InternalIds'][row[0]]
    if path.endswith('.bundle'):bundles.add(assets/'StandaloneWindows64'/path.rsplit('/',1)[-1])
    if row[2]>=0:pending.extend(buckets[row[2]])
env=UnityPy.load(*(str(path)for path in sorted(bundles)))
pending=[ptr for path,ptr in env.container.items()if path.lower()in targets]
seen=set();nodes=[]
while pending:
    obj=pending.pop().deref();identity=(obj.assets_file.name,obj.path_id)
    if identity in seen:continue
    seen.add(identity);assert len(seen)<128
    value=obj.read();node={'name':value.m_Name,'active':value.m_IsActive,'layer':value.m_Layer,'components':[]}
    for pair in value.m_Component:
        ptr=pair.component
        if not ptr.m_PathID:continue
        kind=ptr.type.name;component=ptr.read()
        if kind in ('Transform','RectTransform'):pending.extend(child.read().m_GameObject for child in component.m_Children)
        elif kind in ('BoxCollider2D','Rigidbody2D','MonoBehaviour'):
            row={'type':kind,'tree':ptr.read_typetree()}
            if kind=='MonoBehaviour':
                script=component.m_Script.read();row['class']=script.m_Namespace+'.'+script.m_ClassName
            node['components'].append(row)
    nodes.append(node)
sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
facts={'targets':sorted(targets),'catalog_sha256':sha(catalog_path),'bundles':{path.name:sha(path)for path in sorted(bundles)},'nodes':nodes}
(out/'assets.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'FF{number}: {len(nodes)} exact prefab nodes, {len(bundles)} catalog bundles')
for node in nodes:
    for component in node['components']:
        if component['type']=='BoxCollider2D':
            tree=component['tree'];print(node['name'],{key:tree.get(key)for key in ('m_Enabled','m_IsTrigger','m_Size','m_Offset')})
