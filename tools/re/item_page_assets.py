"""Inspect only the catalog's equipment-job page prefabs and their animators.

Read-only/offline: native assets and hierarchy/curve evidence remain outside Git.
"""
from pathlib import Path
import base64,hashlib,json,struct,subprocess,sys
import UnityPy

ROOT=Path(__file__).resolve().parents[2]
number=int(sys.argv[1]) if len(sys.argv)>1 else 3
titles=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')
assets=Path('D:/Games/SteamLibrary/steamapps/common')/(titles[number-1]+' PR')/(titles[number-1]+'_Data/StreamingAssets/aa')
out=ROOT.parent/'tools/item_buffer_pages_20261028'/f'ff{number}'
out.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
catalog_path=assets/'catalog.json';catalog=json.loads(catalog_path.read_text(encoding='utf-8-sig'))
targets={p.lower()for p in catalog['m_InternalIds'] if p.endswith('.prefab') and '/Key/'in p and p.rsplit('/',1)[-1]in ('item_equipable_list.prefab','item_equipable_list_content.prefab')}
assert len(targets)==2
raw=base64.b64decode(catalog['m_EntryDataString']);count,=struct.unpack_from('<i',raw)
entries=[struct.unpack_from('<7i',raw,4+28*i)for i in range(count)]
raw=base64.b64decode(catalog['m_BucketDataString']);count,=struct.unpack_from('<i',raw);buckets=[];at=4
for _ in range(count):
    offset,n=struct.unpack_from('<ii',raw,at);at+=8
    buckets.append(struct.unpack_from('<'+'i'*n,raw,at));at+=4*n
queue=[i for i,row in enumerate(entries)if catalog['m_InternalIds'][row[0]].lower()in targets];seen=set();bundles=set()
while queue:
    i=queue.pop()
    if i in seen:continue
    seen.add(i);assert len(seen)<256
    row=entries[i];path=catalog['m_InternalIds'][row[0]]
    if path.endswith('.bundle'):bundles.add(assets/'StandaloneWindows64'/path.rsplit('/',1)[-1])
    if row[2]>=0:queue.extend(buckets[row[2]])
env=UnityPy.load(*(str(p)for p in sorted(bundles)));stack=[];nodes=[];animations=[];visited=set()
for path,ptr in env.container.items():
    if path.lower()in targets:stack.append(ptr)
while stack:
    ptr=stack.pop();obj=ptr.deref();identity=(obj.assets_file.name,obj.path_id)
    if identity in visited:continue
    visited.add(identity);assert len(visited)<1000
    go=obj.read();node={'name':go.m_Name,'active':go.m_IsActive,'components':[]}
    for pair in go.m_Component:
        component=pair.component
        if not component.m_PathID:continue
        value=component.read();kind=component.type.name
        if kind in ('Transform','RectTransform'):stack.extend(child.read().m_GameObject for child in value.m_Children)
        elif kind=='MonoBehaviour':
            script=value.m_Script.read();tree=component.read_typetree()
            node['components'].append({'class':script.m_Namespace+'.'+script.m_ClassName,'tree':tree})
        elif kind=='Animator':
            controller=value.m_Controller.read();clips=[]
            for clip in controller.m_AnimationClips:
                if clip.m_PathID:clips.append(clip.read_typetree())
            animations.append({'object':go.m_Name,'controller':controller.m_Name,'clips':clips})
    nodes.append(node)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
facts={'targets':sorted(targets),'bundles':{p.name:sha(p)for p in sorted(bundles)},'catalog_sha256':sha(catalog_path),'nodes':nodes,'animations':animations}
(out/'assets.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'FF{number}: {len(nodes)} exact prefab descendants, {len(animations)} animator(s), {len(bundles)} catalog bundle(s)')
for row in animations:print(row['object'],row['controller'],[c['m_Name']for c in row['clips']])
