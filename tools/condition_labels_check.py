"""Audit unnamed rendered conditions against the shared fallback and locale catalog.

Only original validation logic belongs here. Native masters remain outside Git.
"""
from pathlib import Path
import csv,json,re
ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'src/core/condition_labels.h').read_text(encoding='utf-8')
fallback={int(value) for value in re.findall(r'case (\d+):',source)}
missing=[]
for number in range(1,7):
    path=ROOT.parent/'tools/mapdump'/f'ff{number}_master/condition.txt'
    rows=list(csv.DictReader(path.read_text(encoding='utf-8-sig').splitlines()))
    icons=[row for row in rows if int(row['icon_id'])>0]
    unnamed=[row for row in icons if row['mes_id_name'] in ('','None')]
    absent=[row['id'] for row in unnamed if int(row['condition_type']) not in fallback]
    missing.extend((number,value) for value in absent)
    print(f'FF{number}: {len(icons)} icon-bearing records, {len(unnamed)} unnamed, {len(absent)} missing fallback')
keys=['Amnesia','Pig','Imp','Frozen','Sap','Invisible','{0}: HP and MP exchanged with {1}','World Map','Attack up, Evasion down',
      '{0} up','{0} down','Magic interference','Critical rate','Damage multiplier','Attack count','Attack speed',
      'Defense count','Magic defense count','Accuracy count','Evasion count','Fear','Magic count',
      'Statistic feedback is busy; some changes could not be read.','Maximum {0}']
from modtext_enemy_critical import ROWS,LOCALES
keys+=list(ROWS)
catalog=json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))
for key,values in ROWS.items():assert catalog[key]==dict(zip(LOCALES,values)),key
locales={'en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt'}
for key in keys:
    values=catalog[key]
    assert set(values)==locales,key
    expected=sorted(re.findall(r'\{\d+\}',key))
    for locale,value in values.items():
        assert value.strip(),(key,locale)
        assert sorted(re.findall(r'\{\d+\}',value))==expected,(key,locale)
assert not missing,missing
ff2=json.loads((ROOT/'data/ff2/mod_text.json').read_text(encoding='utf-8-sig'))
assert set(ff2['Venom'])==locales and ff2['Venom']['en']=='Poison'
assert ff2['Poison']['en']=='Venom' and ff2['Venom']['ja']=='猛毒'
rows={int(r['id']):r for r in csv.DictReader((ROOT.parent/'tools/mapdump/ff2_master/condition.txt').open(encoding='utf-8-sig'))}
assert rows[10]['condition_type']=='10' and rows[10]['mes_id_name']=='MSG_SYSTEM_042' and rows[10]['until_type']=='5'
assert rows[13]['condition_type']=='204' and rows[13]['mes_id_name']=='None' and rows[13]['until_type']=='3' and int(rows[13]['field_se_asset_id'])>0
# Bind the new label to the actual source chain and signed parameter semantics.
# This is factual data only; native implementations remain outside every Git tree.
def master(table):
    return list(csv.DictReader((ROOT.parent/f'tools/mapdump/ff2_master/{table}.txt').open(encoding='utf-8-sig')))
content=next(r for r in master('content') if r['mes_id_name']=='MSG_MON_ATK_80')
assert content['type_value']=='714',content
ability=next(r for r in master('ability') if r['id']=='714')
assert ability['battle_function_group_id']=='178' and ability['process_prog']=='None'
group=next(r for r in master('function_group') if r['id']=='178')
assert group['function1_id']=='9' and group['function1_value1']=='50'
assert rows[50]['condition_type']=='203' and rows[50]['parameter_group_id']=='11' and rows[50]['mes_id_name']=='None'
effects={(r['parameter_id'],r['vertical_id'],r['value']) for r in master('parameter_group') if r['group_id']=='11'}
assert effects=={('10','1','14'),('17','2','20')},effects
for number in (1,3,4,5,6):
    other=csv.DictReader((ROOT.parent/f'tools/mapdump/ff{number}_master/condition.txt').open(encoding='utf-8-sig'))
    assert all(r['condition_type']!='203' for r in other),number
print('FFII temporary Venom/type10 and persistent Poison/type204 retain separate labels in all12 locales')
print('FFII Imbibe content940/ability714/function178/condition50 names Attack up and Evasion down; unused in the other five masters')
stat_source=(ROOT/'src/core/battle/stat_feedback.cpp').read_text(encoding='utf-8')
field_names=set(re.findall(r'\b\w+\{kParameter,"([^"]+)"\}',stat_source))
assert len(field_names)>=50,field_names
for number,folder in enumerate(('ff1','ff2','ff3','FF4','FF5','ff6'),1):
    native=(ROOT.parent/folder/'dump.cs').read_text(encoding='utf-8-sig')
    parameter=next(block for block in native.split('// Namespace: ')[1:]
                   if re.search(r'^public (?:abstract )?class CharacterParameterBase\b',block,re.M))
    for name in field_names:assert name in parameter,(number,name)
    parameters={int(r['id']):int(r['parameter_type_id']) for r in
                csv.DictReader((ROOT.parent/f'tools/mapdump/ff{number}_master/parameter.txt').open(encoding='utf-8-sig'))}
    expected={i:i for i in range(1,25)}|{25:201}
    if number==2:expected|={26:202,27:203,28:204}
    assert parameters==expected,(number,parameters)
    groups=list(csv.DictReader((ROOT.parent/f'tools/mapdump/ff{number}_master/parameter_group.txt').open(encoding='utf-8-sig')))
    assert all(r['parameter_id']!='15' for r in groups),(number,'new Luck effect needs a visible-source review')
    assert all(int(r['parameter_id']) in parameters for r in groups),(number,'unsupported modifier parameter')
    assert all(r['vertical_id'] in ('1','2','3') for r in groups),(number,'unsupported signed group semantics')
    print(f'FF{number}: {len(field_names)} native base/temporary fields and {len(parameters)} modifier IDs cover {len(groups)} actual group rows')
print(f'PASS: all unnamed rendered conditions covered; {len(keys)} catalog keys / {len(keys)*12} localized values verified')

from modtext_stat_feedback import RECORDS
for scope,key,values,_ in RECORDS:
    assert scope=="common" and catalog[key]==values,(key,"statistic recipe drift")
print(f"Maintained statistic recipes: {len(RECORDS)} keys in all12 locales match the current catalog")
