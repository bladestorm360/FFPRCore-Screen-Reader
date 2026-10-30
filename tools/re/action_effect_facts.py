"""Verify reviewed FFIII action-effect metadata/assets; never run the game.

Only this original verifier lives in Git. Native exports and derived asset
facts stay in the existing outside-repository investigation directory.
"""
from pathlib import Path
import csv
import hashlib
import json
import re
import subprocess
import UnityPy

root = Path(__file__).resolve().parents[2]
master = root.parent/'tools/mapdump/ff3_master'
dump = root.parent/'ff3/dump.cs'
out = root.parent/'tools/ability_visible_feedback_20261028/ff3'
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
def table(name):
    return list(csv.DictReader((master/name).open(encoding='utf-8-sig')))
abilities = table('ability.txt')
effects = {int(r['id']):int(r['battle_effect_asset_id']) for r in abilities}
assert {i for i,e in effects.items() if e==61} == {137,139}
assert {i for i,e in effects.items() if e==153} == {96}
assert effects[94] == 0 and effects[140] == 46
text = dump.read_text(encoding='utf-8-sig')
block = next(b for b in text.split('// Namespace: ') if b.startswith('Last.Battle\npublic class EffectEntity :'))
assert re.search(r'0x9A6EC0[^\n]*\n\s*public void Init\(EntityMeshRenderer attackEntity, EntityMeshRenderer\[\] targetEntity, EntityMeshRenderer\[\] playerEntityMeshRenderers, BattleBackgroundController backgroundController, FxQuadController fxQuadController, bool isFirst, ReactionActionInterface reactionActionInterface, bool lastAttack = False, bool isEnemySideAttack = False, Action<',block)
assert re.search(r'0x9A7D80[^\n]*\n\s*private void Update\(\)',block)
assert '<IsEnd>k__BackingField; // 0x38' in block
base = next(b for b in text.split('// Namespace: ') if b.startswith('Last.Battle\npublic class EffectEntityBase :'))
assert 'myObject; // 0x30' in base
assert 'public int get_AbilityId()' in next(b for b in text.split('// Namespace: ') if b.startswith('Last.Data.Master\npublic class Command :'))

assets = Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY III PR/FINAL FANTASY III_Data/StreamingAssets/aa/StandaloneWindows64')
facts=[]
for stem,markers,expected in [('ef_ff0_focus',{'EF_FF0_Focus_H00_0'},5),('ef_ff3_attract',{'EF_FF3_Attract_A00_0','EF_FF3_Attract_W00_3'},2)]:
    bundle, = assets.glob(stem+'_assets_all_*.bundle')
    env = UnityPy.load(str(bundle))
    prefab, = [v for k,v in env.container.items() if k.endswith('/EffectPrefab.prefab')]
    obj = prefab.read()
    transform = next(c.component.read() for c in obj.m_Component if c.component.type.name=='Transform')
    children = [c.read().m_GameObject.read().m_Name for c in transform.m_Children]
    assert obj.m_Name == 'EffectPrefab' and obj.m_IsActive and set(children)==markers
    renderers=[]
    def visit(go):
        for c in go.m_Component:
            if c.component.type.name=='ParticleSystemRenderer':
                r=c.component.read();assert r.m_Enabled
                renderers.append(go.m_Name)
            if c.component.type.name=='Transform':
                for child in c.component.read().m_Children:visit(child.read().m_GameObject.read())
    visit(obj)
    assert len(renderers)==expected
    facts.append({'bundle':bundle.name,'sha256':hashlib.sha256(bundle.read_bytes()).hexdigest(),
                  'root':obj.m_Name,'children':children,'enabled_particle_renderers':renderers})
record={'dump_sha256':hashlib.sha256(dump.read_bytes()).hexdigest(),
        'ability_table_sha256':hashlib.sha256((master/'ability.txt').read_bytes()).hexdigest(),
        'effect_61_abilities':[137,139],'effect_153_abilities':[96],
        'array_init_rva':'0x9A6EC0','array_init_parameters':10,'native_update_rva':'0x9A7D80',
        'object_field_offset':'0x30','end_field_offset':'0x38','assets':facts}
(out/'reviewed_facts.json').write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
print('FFIII exact effect ABI, unique ability/effect mappings and live prefab hierarchies verified; facts outside Git.')
