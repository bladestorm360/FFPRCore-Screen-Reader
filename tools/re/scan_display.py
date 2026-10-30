"""Verify factual FFIII Scan ability/immunity contracts without running a game."""
from pathlib import Path
import csv
import hashlib
import json
import sys

root = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'tools'))
import official_text

master = root.parent/'tools/mapdump/ff3_master'
def rows(name):
    return list(csv.DictReader((master/name).open(encoding='utf-8-sig')))
abilities = {r['id']:r for r in rows('ability.txt')}
groups = {r['id']:r for r in rows('function_group.txt')}
functions = {r['id']:r for r in rows('function.txt')}
scan = []
for ability in ('15','89'):
    row=abilities[ability];group=groups[row['battle_function_group_id']]
    assert group['function1_id']=='50' and functions['50']['type_id']=='50'
    values=[int(group[f'function1_value{i}']) for i in (1,2,3)]
    assert values==[0,0,0]
    scan.append({'ability':int(ability),'group':int(group['id']),'function':50,'parameters':values})
monsters = {r['id']:r for r in rows('monster.txt')}
kraken = monsters['204']
flags = [int(r['monster_flag_id']) for r in rows('monster_flag_group.txt')
         if r['group_id']==kraken['monster_flag_group_id']]
assert flags==[14]
messages = official_text.load('ff3')['system']['en']
assert messages['MSG_MAGIC_NAME_10']=='Libra' and messages['MSG_SYSTEM_202']=='Study'
assert messages[kraken['mes_id_name']]=='Kraken'
out = root/'build_check/scan_display_20261027'
out.mkdir(exist_ok=True)
facts={'scan_abilities':scan,'kraken':{'monster':204,'message_id':kraken['mes_id_name'],
    'flag_group':int(kraken['monster_flag_group_id']),'flags':flags,
    'native_flag':'ResistLibraHideHp'},'master_hashes':{
        n:hashlib.sha256((master/n).read_bytes()).hexdigest()
        for n in ('ability.txt','function_group.txt','function.txt','monster.txt','monster_flag_group.txt')}}
(out/'native_facts.json').write_text(json.dumps(facts,indent=2)+'\n')
print('Libra/Study native Scan parameters identical; official Kraken identity and HP-hiding flag14 verified.')
