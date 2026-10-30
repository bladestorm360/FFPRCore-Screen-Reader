"""Inspect FFIII's known battle UI assets offline, never internal initiative.

Only catalog-listed battle/common bundles are read. Asset hierarchies and native
field inventories remain outside Git; this never runs or controls the game.
"""
from pathlib import Path
import base64,hashlib,json,re,struct,subprocess
import UnityPy

ROOT=Path(__file__).resolve().parents[2]
ASSETS=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY III PR/FINAL FANTASY III_Data/StreamingAssets/aa')
OUT=ROOT.parent/'tools/turn_order_display_20261028/ff3'
assert subprocess.run(['git','-C',str(OUT.parent),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
OUT.mkdir(parents=True,exist_ok=True)
catalog_path=ASSETS/'catalog.json'
catalog=json.loads(catalog_path.read_text(encoding='utf-8-sig'))
paths=[p for p in catalog['m_InternalIds'] if p.lower().endswith('.prefab') and '/UI/' in p and '/Battle/' in p]
raw=base64.b64decode(catalog['m_EntryDataString']);count,=struct.unpack_from('<i',raw)
entries=[struct.unpack_from('<7i',raw,4+28*i) for i in range(count)]
raw=base64.b64decode(catalog['m_BucketDataString']);count,=struct.unpack_from('<i',raw);buckets=[];at=4
for _ in range(count):
    offset,n=struct.unpack_from('<ii',raw,at);at+=8
    buckets.append(struct.unpack_from('<'+'i'*n,raw,at));at+=4*n
targets=paths+['Assets/GameAssets/Common/UI/Key/Common/battle_window.prefab',
               'Assets/GameAssets/Common/UI/Key/Common/battle_message_window.prefab']
indices={i for i,p in enumerate(catalog['m_InternalIds']) if p in targets}
queue=[i for i,row in enumerate(entries) if row[0]in indices];seen=set();bundles=set()
while queue:
    i=queue.pop()
    if i in seen:continue
    seen.add(i);assert len(seen)<256
    row=entries[i];path=catalog['m_InternalIds'][row[0]]
    if path.endswith('.bundle'):bundles.add(ASSETS/'StandaloneWindows64'/path.rsplit('/',1)[-1])
    if row[2]>=0:queue.extend(buckets[row[2]])
assert 1<=len(bundles)<=12
env=UnityPy.load(*(str(p)for p in sorted(bundles)))
nodes=[];unresolved=[];visited=set();roots=[];stack=[]
for path,ptr in env.container.items():
    if path.lower() in {p.lower()for p in targets}:
        roots.append({'path':path,'root':ptr.read().m_Name});stack.append(ptr)
while stack:
    obj=stack.pop().deref();identity=(obj.assets_file.name,obj.path_id)
    if identity in visited:continue
    visited.add(identity);assert len(visited)<=10000
    go=obj.read();components=[];parent=None
    for pair in go.m_Component:
        component=pair.component
        if not component.m_PathID:continue
        value=component.read();kind=component.type.name
        entry={'kind':kind}
        if kind in ('Transform','RectTransform'):
            stack.extend(child.read().m_GameObject for child in value.m_Children)
            father=value.m_Father
            if father.m_PathID:parent=father.read().m_GameObject.read().m_Name
        elif kind=='MonoBehaviour':
            try:
                script=value.m_Script.read();entry['class']=script.m_Namespace+'.'+script.m_ClassName
            except Exception as error:unresolved.append({'object':go.m_Name,'script':str(error)[:160]})
            tree=component.read_typetree()
            entry['fields']=[key for key in tree if not key.startswith('m_')]
            if 'm_Text'in tree:entry['text']=tree['m_Text']
        components.append(entry)
    nodes.append({'name':go.m_Name,'active':go.m_IsActive,'parent':parent,'components':components})

dump_path=ROOT.parent/'ff3/dump.cs';ui=[];internal=[]
for block in dump_path.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
    namespace=block.splitlines()[0]
    match=re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)',block,re.M)
    if not match:continue
    name=match[1]
    if 'Battle'not in namespace+'.'+name:continue
    methods=sorted(set(re.findall(r'\b(\w*(?:Turn|Order|Initiative|Timeline)\w*)\(',block)))
    fields=[line.strip().split('//')[0].strip() for line in block.splitlines()
            if '// 0x'in line and re.search(r'\b\w*(?:turn|order|timeline|initiative)\w*',line,re.I)]
    fact={'class':namespace+'.'+name,'methods':methods,'fields':fields}
    if '.UI'in namespace:ui.append(fact)
    elif methods or fields:internal.append(fact)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
facts={'catalog_sha256':sha(catalog_path),'dump_sha256':sha(dump_path),
       'battle_ui_prefabs':paths,'bundles':{p.name:sha(p)for p in sorted(bundles)},
       'prefab_roots':roots,'nodes':nodes,'unresolved_scripts':unresolved,'battle_ui_classes':ui,'internal_order_symbols':internal}
(OUT/'asset_and_symbol_facts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'{len(paths)} battle UI prefabs, {len(bundles)} exact catalog bundles, {len(roots)} selected roots, {len(nodes)} descendant nodes; unresolved scripts {len(unresolved)}')
for row in nodes:
    if re.search(r'order|turn|timeline|initiative|command_icon|player_info|battle_info',row['name'],re.I):
        print(row['name'],[(c.get('class'),c.get('fields'))for c in row['components'] if c.get('class')])
print('Battle UI classes with turn/order symbols:',[row for row in ui if row['methods']or row['fields']])
