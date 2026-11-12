"""Six exact Jachol identities,including resets and the opened counterpart."""
from pathlib import Path
import hashlib,json,sys
import pefile
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
read=lambda p:json.loads(p.read_text(encoding='utf-8'))
key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw'])
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 proof=read(R/'docs/re/ff5_jachol_native_20261104.json');assert len(proof['source_script_hashes'])==14 and len(proof['native_functions'])==9
 dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');pe=pefile.PE(str(dll),fast_load=True)
 assert digest(dll)==proof['dll_sha256']and digest(R.parent/'FF5/dump.cs')==proof['dump_sha256']
 for f in proof['native_functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 path=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/FINAL FANTASY V_Data/il2cpp_data/Metadata/global-metadata.dat');assert digest(path)==proof['metadata_sha256']
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json','puzzle_mechanics_continuation_inventory_20261104.json'):
  g=read(R/'docs/re'/name)['games']['ff5'];assert sources is None or sources==g['source_sha256'];sources=g['source_sha256'];facts.update(g['script_facts'])
  for r in g['candidates']:
   if r['area_names_en']==['Jachol Cave']:rows[key(dict(game='ff5',**r))]=r
 assert len(rows)==6
 reasons={
  25:('guided mechanic','Actual first-side wall control sets407 and clears408 before the real recurring skull mechanic. The guide chooses a reachable real first control,then stages on safe floor.'),
  26:('guided mechanic','Actual alternate wall control has the same complete407/408 operation as25. It is an alternate physical approach,not a mandatory second activation that would restart the timer.'),
  8:('guided mechanic','Actual interactive skull relocates below one of eight native graphic columns. The new guide uses the existing live-object route pose,one exact visibility observation and original-first show/hide epochs. Fresh nearby guidance,expired/unknown refusal,manual timer-freeze behavior,flagless door and full two-world return are covered.'),
  23:('guided reset counterpart','This wall-return operation stops the native cycle and clears407 without setting408. The guide reads the actual current bits and can restart at25/26;an early return/reset cannot certify the successful skull receipt.'),
  11:('guided mechanic','Real scripted second-floor chest records435,with actual169 prisoner-dependent reward behavior preserved. The guide then requires both real treasure105/106 receipts and routes to the actual climb control;no undisplayed reward contents are spoken.'),
  12:('opened-container counterpart','After435,the initializer hides11 and moves/shows this opened counterpart. Its repeated script follows the native opened branch;it is not another reward or a second required guide operation. Existing animated-object identity/pose routing stays intact.'),
 }
 authored={}
 for identity,r in rows.items():
  sha=facts[r['submap']+'/'+r['script']]['sha256'];assert proof['source_script_hashes'][r['submap']+'/'+r['script']]==sha;kind,reason=reasons[r['entity']]
  authored[identity]=dict(game='ff5',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition=kind,guides=['jachol_cave'],reason=reason,evidence='docs/re/FF5_JACHOL_GUIDE_20261104.md;14 complete scripts/23 actor facts/nine exact native functions/eight metadata columns;actual model/physical/native-owner/entry/source-preservation checks.')
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==sources[role]
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);seen=set();new['decisions']=[]
 for r in old['decisions']:
  identity=key(r)
  if identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Jachol six exact dispositions;real first controls/random skull/reset/scripted chest/opened counterpart;full native journey retained.')
if __name__=='__main__':main()
