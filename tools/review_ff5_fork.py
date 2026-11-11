"""Complete paired-tower source,actual native receipts and original dispositions."""
from pathlib import Path
import hashlib,json,sys
import pefile
from author_ff5_fork import author
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;guide,guard,proof,gates=author();assert read(R/'docs/re/ff5_fork_native_20261105.json')==proof
 native=proof['position_native'];dll=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==native['dll_sha256']and digest(R.parent/'FF5/dump.cs')==native['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in native['functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if r['submap']in ('Map_30481_1','Map_30490','Map_30490_1','Map_30490_2')];assert len(selected)==11
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 for r in selected:assert proof['source_script_hashes'][r['submap']+'/'+r['script']]==master['source_script_hashes'][r['submap']+'/'+r['script']]
 E=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_ForkTower';A=dll.parent/'FINAL FANTASY V_Data/StreamingAssets/aa/StandaloneWindows64'
 for name,f in proof['battle_assets'].items():assert digest(A/f['bundle'])==f['bundle_sha256']and digest(E/(name+'.json'))==f['sha256']
 ai=read(E/'sc_ai_349_Omniscient.json')['Mnemonics'];assert len(ai)==55 and ai[32]['mnemonic']=='IsCounterTypeId'and ai[32]['operands']['iValues']==[1,50]and ai[51]['operands']['iValues']==[155]
 other=read(E/'sc_b_addcondition.json')['Mnemonics'];assert other[1]['mnemonic']=='BattleCondition'and other[1]['operands']['iValues']==[1,0,90,1]
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  barrier=r['entity']in (28,29,30,31,32)and r['submap']=='Map_30481_1'
  reason='Ordinary pre-tablet pushback contact,not a separate puzzle. The complete initializer selects these five barriers before142;each controlled-player callback returns south of the barrier. It remains selectable while onward travel is story gated.'if barrier else 'Coordinated assignment/twin-tower puzzle. Actual228..239 receipts preserve every legal saved assignment prefix;143 and144 own setup and native party swap. Paired encounters lead to the exact10,12 final-spell landing and ten-second timer;full145 and controlled world206,202 return complete the sequence. Navigation and Other preparation are independently selectable.'
  authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=proof['source_script_hashes'][r['submap']+'/'+r['script']],disposition='ordinary story barrier'if barrier else'guided complex puzzle',guides=[]if barrier else['fork_tower'],reason=reason,evidence='docs/re/FF5_FORK_TOWER_20261105.md;47 complete field programs/three exact battle assets/four retained placement functions;physical routes,native-ready owner and state guards.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  k=key(row)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Fork:11 identities,47 full field programs,three exact battle scripts and four native placement functions;simple barriers remain navigation.')
if __name__=='__main__':main()
