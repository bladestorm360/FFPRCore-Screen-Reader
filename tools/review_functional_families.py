"""Record the exact physical families reviewed during the October28 audit.

This explicitly invoked offline authoring tool pins complete source hashes;
it is not runtime inference or an automatic build-time classification rule.
"""
from pathlib import Path
import csv,hashlib,json
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
OUT=ROOT.parent/'tools/map_event_audit_20261028'
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
def label_for(n,f):
    name=f['script'];ops=set(f['operations']);raw=f['occurrences'][0]['raw']
    if n==2 and name=='sc_e_0081_2':
        assert ops<={'Nop','Call','Exit','SetPuppet','ExecPuppet','Return','Move'}
        return 'Passage barrier'
    if n==3:
        if name in {'sc_e_0068','sc_e_0068_1','sc_e_0068_2','sc_e_0068_3'}:return 'Chain'
        if name=='sc_e_0250':return 'Passage barrier'
    if n==4:
        if name.startswith('sc_s_') or name in {'sc_barondoor_1','sc_barondoor_2'}:
            assert not ops&{'Msg','ChangeMap','Encount','EncountBoss','SysCall','Move','MoveTo'}
            return 'Switch' if 'スイッチ' in raw else 'Gate' if '門' in raw else 'Door'
        if name.startswith('sc_rope_'):
            assert not ops&{'Msg','ChangeMap','Encount','EncountBoss','SysCall'}
            return 'Rope crossing'
        return {'sc_savepoint_1':'Save Point','sc_e_0103':'Fountain','sc_e_0104':'Fountain',
                'sc_e_0161':'Telescope','sc_ship_0008':'World map transition'}.get(name)
    if n==5:
        if name in {'sc_map_20030_warterfall_1','sc_map_20040_carry_1','sc_map_20040_carry_2',
                    'sc_30151_11_carry_1','sc_30151_11_carry_2','sc_30151_18_carry',
                    'sc_map_30571_2_carry_1','sc_map_30571_2_carry_2'}:return 'Water current'
        if name in {'sc_s_20041_6_1','sc_s_20041_8_1','sc_e_0387_9','sc_30471_1_door','sc_30471_16_door'}:return 'Door'
        if name.startswith('sc_30481_1_barrier') or name in {'sc_map_10020_barrier','sc_map_10020_barrier_2'}:return 'Passage barrier'
        if name=='sc_e_0239_2':return 'Switch'
        if name=='sc_e_0209':return 'Save Point'
        if name.startswith('sc_e_0249_'):return 'Hidden item'
    if n==6:
        if name in {'sc_e_0437_0','sc_e_0385','sc_e_0900','sc_e_0900_1','sc_e_0900_2','sc_e_0969','sc_e_0969_1'}:return 'Passage barrier'
        if name in {f'sc_e_{i:04}' for i in range(291,299)}:return 'Jump point'
        if name in {'sc_e_0387_0','sc_e_0386_0','sc_e_0380_0'}:return 'Vent'
        if name=='sc_e_0389_1':return 'Conveyor belt'
        if name in {'sc_e_0443','sc_e_0444'}:return 'Bridge'
        if name in {'sc_e_0454','sc_e_0455','sc_e_0456','sc_e_1069','sc_e_0733','sc_e_0808',
                    'sc_e_0817','sc_e_0816','sc_e_0811','sc_e_0849','sc_e_0857','sc_e_0980','sc_e_0993'}:return 'Switch'
        if name in {'sc_e_0572','sc_e_0573','sc_e_0574','sc_e_0575','sc_e_0590'}:return 'Passage'
        if name in {'sc_e_1064','sc_e_1063','sc_e_0984','sc_e_0985'}:return 'Door'
        if name in {'sc_e_0850_in1','sc_e_0850_in2','sc_e_1104','sc_e_0904_1'}:return 'Passage'
        if name in {'sc_e_0850_1','sc_e_0850_2'}:return 'Safe spot'
        if name.startswith('sc_e_0968'):return 'Escalator'
        return {'sc_e_1078_0':'Hatch','sc_e_0810':'Trap','sc_e_0039':'Fountain',
                'sc_e_0906_1':'Treasure Chest','sc_e_0906_2':'Treasure Chest'}.get(name)
    return None

def build():
    path=ROOT/'tools/world_event_reviews.json';reviews=json.loads(path.read_text(encoding='utf8'));before=len(reviews)
    seen={(r['game'],r['submap'],r['group'],r['id']) for r in reviews}
    for n in (2,3,4,5,6):
        source=[json.loads(l) for l in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf8')]
        scripts={(s['submap'],s['name']):s for s in source}
        objects=[json.loads(l) for l in (DUMP/f'ff{n}_objects.jsonl').open(encoding='utf8')]
        by_object={(o['submap'],o['group'],o['id']):o for o in objects}
        def append(obj,name,script,label,reason,children,variants=None):
            key=(n,obj['submap'],obj['group'],obj['id'])
            if key in seen:return
            if obj['props'].get('action_id')!=0 and script['submap']not in(obj['submap'],'Resident'):
                # The old triage allowed unique names from unrelated maps.
                # Those bodies are not present in the native lookup banks;
                # exact native-description reviews own such markers instead.
                return
            if n==4 and obj['submap']=='Map_20041_3' and name in {'sc_e_0103','sc_e_0104'}:
                label='Pot';reason='reviewed healing-pot effect helper shares its physical pot identity; action0 is not a narrative scene'
            assert obj['type'] in (2,5,10,28) and obj['props']['action_id'] in (0,2),key
            row=dict(game=n,submap=obj['submap'],group=obj['group'],id=obj['id'],object_type=obj['type'],
                action_id=obj['props']['action_id'],entity_id=obj['props']['entity_id'],raw=obj['name'],script_id=obj['props']['script_id'],
                script_name=name,script_sha256=sha(script),label=label,reason=reason,continuations=children)
            if variants:row['source_variants']=variants
            reviews.append(row);seen.add(key)
        for family in json.loads((OUT/f'ff{n}_scene_review.json').read_text(encoding='utf8')):
            label=label_for(n,family)
            if not label:continue
            assert not family['unresolved'] and not set(family['operations'])&{'Encount','EncountBoss','ChangeScript'},(n,family['script'])
            if 'GetItem' in family['operations']:
                assert n==5 and family['script'].startswith('sc_e_0249_') and label=='Hidden item'
            for loc in family['occurrences']:
                obj=by_object[(loc['submap'],loc['group'],loc['editor_id'])]
                script=scripts.get((obj['submap'],family['script']))
                if not script:
                    script=next(s for s in source if s['name']==family['script'] and sha(s['mnemonics'])==family['body_sha256'])
                append(obj,family['script'],script,label,'reviewed complete physical animation/interaction body and continuations',family['continuations'])
        if n==5:
            variants=[s for s in source if s['name']=='sc_e_0209'];resident=next(s for s in variants if s['submap']=='Resident')
            hashes=sorted({sha(s['mnemonics']) for s in variants})
            for s in variants:assert {m['mnemonic'] for m in s['mnemonics']}<={'Nop','Branch','ColorFade','PlaySE','Exit','Msg','SetFlag'}
            ids={int(r['id']) for r in csv.DictReader((DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig')) if r['id'] and r['script_name']=='sc_e_0209'}
            for obj in objects:
                p=obj.get('props')or{}
                if obj['type']!=2 or p.get('script_id') not in ids or p.get('action_id')!=2 or obj['submap'].startswith('Map_00000'):continue
                append(obj,'sc_e_0209',scripts.get((obj['submap'],'sc_e_0209'),resident),'Save Point',
                       'both complete global save-point variants only animate/save tutorial and set tutorial-seen flag',[],hashes)
    path.write_text(json.dumps(reviews,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(f'Added {len(reviews)-before} exact reviewed physical occurrences')
if __name__=='__main__':build()
