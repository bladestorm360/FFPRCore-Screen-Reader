"""Verify exact master/message/native provenance, never execute game code.

Only factual IDs and hashes enter the repository. Source messages, AI bodies
and native disassembly remain in the external evidence archive.
"""
from pathlib import Path
import csv,hashlib,json,sys
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump/ff6_master'
EXTERNAL=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Dream_Context'
digest=lambda p:hashlib.file_digest(p.open('rb'),'sha256').hexdigest()
def row(name,identity):
 result,=[r for r in csv.DictReader((DUMP/(name+'.txt')).open(encoding='utf-8-sig'))if int(r['id'])==identity]
 return result
ability=row('ability',355);group=row('function_group',559);condition=row('condition',73)
assert ability['battle_function_group_id']=='559'and ability['battle_renge_id']=='5'
assert(group['function1_id'],group['function1_value1'])==('9','73')
assert(condition['condition_type'],condition['until_type'],condition['target_type'])==('661','18','1')
proof_path=ROOT/'docs/re/dream_native_20261104.json';proof=json.loads(proof_path.read_text(encoding='utf-8'))
assert len(proof['source_script_hashes'])==95 and len(proof['actors'])==300 and len(proof['battle_scripts'])==7
assert proof['facts']['Map_30821_2/sc_e_0901']['encounters']==[[12,461,46,57,'sc_e_0901_1']]
assert proof['facts']['Map_30851_8/sc_e_0939']['encounters']==[[7,463,25,11,'sc_e_0939_1']]
provenance=json.loads((EXTERNAL/'native_fury/provenance.json').read_text(encoding='utf-8'))
assert len(provenance['functions'])==8
assert provenance['dll_sha256']=='0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd'
assert provenance['dump_sha256']=='38aa9c54abe1c712ffd3c6e88a893ea6428f6df7001718b754b21afba7f4a61b'
message_key='E0939_01_537_b_01';messages={}
for language in ('en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th'):
 p=DUMP/('story_mes_'+language+'.txt');lines=p.read_text(encoding='utf-8-sig').splitlines()
 found,=[line for line in lines if line.partition('\t')[0]==message_key]
 assert found.partition('\t')[2]
 messages[language]=dict(source_sha256=digest(p),message_sha256=hashlib.sha256(found.encode()).hexdigest())
want=dict(master_sha256={name:digest(DUMP/(name+'.txt'))for name in ('ability','function_group','condition','monster_party','monster','script','monster_ai_script')},
 possession=dict(ability=355,range=5,function_group=559,function=9,condition=73,condition_type=661,until_type=18,target_type=1,message_key=message_key,messages=messages),
 native_provenance_sha256=digest(EXTERNAL/'native_fury/provenance.json'),native_export_sha256=digest(EXTERNAL/'native_fury/native.txt'))
if '--record'in sys.argv:
 assert 'mechanic_sources'not in proof or proof['mechanic_sources']==want
 proof['mechanic_sources']=want;proof_path.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
else:assert proof.get('mechanic_sources')==want
replacement=json.loads((EXTERNAL/'native_replacement/provenance.json').read_text(encoding='utf-8'))
assert len(replacement['functions'])==4 and replacement['dll_sha256']==provenance['dll_sha256']and replacement['dump_sha256']==provenance['dump_sha256']
alias_proof=dict(provenance_sha256=digest(EXTERNAL/'native_replacement/provenance.json'),native_export_sha256=digest(EXTERNAL/'native_replacement/native.txt'),functions=replacement['functions'])
if '--record'in sys.argv:
 assert 'replacement_sources'not in proof or proof['replacement_sources']==alias_proof
 proof['replacement_sources']=alias_proof;proof_path.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
else:assert proof.get('replacement_sources')==alias_proof
print('Dream native:95 scripts/300 actors/7 resolved battle assets/8 exact native functions;actual355/559/73/661/18 possession and all12 native message sources. No game execution.')
