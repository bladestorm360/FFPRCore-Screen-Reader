"""Verify displayed Boost/Guard sources offline; game assets stay outside Git."""
from pathlib import Path
import base64,csv,hashlib,json,struct
import UnityPy

ROOT=Path(__file__).resolve().parents[2]
MASTER=ROOT.parent/'tools/mapdump/ff3_master'
OUT=ROOT.parent/'tools/guard_feedback_20261028/ff3'
ASSETS=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY III PR/FINAL FANTASY III_Data/StreamingAssets/aa')
def table(name):return list(csv.DictReader((MASTER/name).open(encoding='utf-8-sig')))
def sha(path):
    with path.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()

content={int(r['type_value']):r for r in table('content.txt') if r['type_id']=='4'}
assert content[137]['mes_id_name']==content[139]['mes_id_name']==content[94]['mes_id_name']=='MSG_SYSTEM_207'
assert all(content[i]['mes_id_battle']=='None' for i in (94,137,138,139,140,87))
assert content[138]['mes_id_name']=='None' and content[140]['mes_id_name']=='MSG_MON_ATK_03'
assert next(r for r in table('message_conclusion.txt') if r['key_name']=='BATTLE_HIT_ABILITY_140')['message_id']=='MSG_SYSTEM_068'
command=next(r for r in table('command.txt') if r['id']=='15');assert command['ability_id']=='87'
ability=next(r for r in table('ability.txt') if r['id']=='87')
assert ability['battle_effect_asset_id']=='154' and ability['battle_function_group_id']=='139' and ability['renge_id']=='5'
group=next(r for r in table('function_group.txt') if r['id']=='139')
assert (group['function1_id'],group['function1_value1'],group['function2_id'],group['function3_id'])==('9','28','0','0')
condition=next(r for r in table('condition.txt') if r['id']=='28')
assert (condition['mes_id_name'],condition['mes_id_description'],condition['icon_id'])==('None','None','0')
assert not any(r['key_name']in('BATTLE_HIT_ABILITY_87','BATTLE_HIT_ABILITY_87_ENEMY') for r in table('message_conclusion.txt'))

# Resolve the one known prefab's actual compact-catalog dependency graph. The
# resulting bundle CABs are verified by resolving their native material/mesh
# pointers below; no directory-wide asset loading or speculative file scan.
catalog_path=ASSETS/'catalog.json';catalog=json.loads(catalog_path.read_text(encoding='utf-8-sig'))
raw=base64.b64decode(catalog['m_EntryDataString']);count,=struct.unpack_from('<i',raw)
assert len(raw)==4+count*28
entries=[struct.unpack_from('<7i',raw,4+28*i) for i in range(count)]
raw=base64.b64decode(catalog['m_BucketDataString']);count,=struct.unpack_from('<i',raw);buckets=[];at=4
for _ in range(count):
    offset,n=struct.unpack_from('<ii',raw,at);at+=8;assert 0<=n<=len(entries)
    buckets.append(struct.unpack_from('<'+'i'*n,raw,at));at+=4*n
assert at==len(raw)
internal=next(i for i,s in enumerate(catalog['m_InternalIds']) if s.endswith('/EF_FF3_Guard/Prefabs/EffectPrefab.prefab'))
queue=[i for i,row in enumerate(entries) if row[0]==internal];seen=set();bundle_paths=[]
while queue:
    i=queue.pop()
    if i in seen:continue
    seen.add(i);assert len(seen)<=64
    row=entries[i];path=catalog['m_InternalIds'][row[0]]
    if path.endswith('.bundle'):bundle_paths.append(ASSETS/'StandaloneWindows64'/path.rsplit('/',1)[-1])
    if row[2]>=0:queue.extend(buckets[row[2]])
assert len(bundle_paths)==5
guard=next(p for p in bundle_paths if p.name.startswith('ef_ff3_guard_assets_all_'))
assert sha(guard)=='4fcabba557f21252438e7edb9d3701c2fb1baa3debd4bfc5382d599840e30623'
env=UnityPy.load(*(str(p)for p in bundle_paths));renderers=[];textures=set();blue=False;meshes=[]
guard_files={f.name for f in env.assets if any(o.type.name=='GameObject' and o.read().m_Name=='EF_FF3_Guard_A00_0' for o in f.objects.values())}
for obj in env.objects:
    if obj.assets_file.name not in guard_files:continue
    if obj.type.name=='ParticleSystemRenderer':
        r=obj.read();assert r.m_Enabled;renderers.append(r.m_GameObject.read().m_Name)
        mesh=obj.read_typetree().get('m_Mesh')
        if mesh and mesh['m_PathID']:
            m=r.m_Mesh.read();meshes.append(m.m_Name);assert m.m_Name=='EF_Dome04_NY'
    elif obj.type.name=='Material':
        m=obj.read()
        for key,value in m.m_SavedProperties.m_TexEnvs:
            if value.m_Texture.m_PathID:textures.add(value.m_Texture.read().m_Name)
        for key,color in m.m_SavedProperties.m_Colors:
            if key=='_GradientWhiteColor' and color.b>.99 and color.r<.7 and color.a>.99:blue=True
assert len(renderers)==12 and len(meshes)==2 and blue
assert {'EF_Sphere08_SA','EF_Sphere19_SA','EF_Line10_MS'}<=textures
OUT.mkdir(parents=True,exist_ok=True)
facts={'master_sha256':{n:sha(MASTER/n)for n in ('command.txt','content.txt','ability.txt','condition.txt','function_group.txt','message_conclusion.txt')},
       'catalog_sha256':sha(catalog_path),'bundles':{p.name:sha(p)for p in bundle_paths},
       'renderers':renderers,'meshes':meshes,'textures':sorted(textures),'blue_white_barrier':True,
       'boost_redundant_children':[137,138,139],'self_destruct_and_hit_message_preserved':140}
(OUT/'reviewed_facts.json').write_text(json.dumps(facts,indent=2)+'\n',encoding='utf-8')
print('Boost native name/blank/outcome identities and Guard self-barrier: exact masters,5 catalog bundles,12 renderers,2 domes verified')
