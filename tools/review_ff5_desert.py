"""Exact native/field sources for19 controls and274 permanent current areas."""
from pathlib import Path
import hashlib,json,sys
import pefile
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;path=R/'docs/re/ff5_desert_native_20261105.json';proof=read(path);assert proof['scope_tag']=='ff5_Desert'and len(proof['source_script_hashes'])==10 and len(proof['actors'])==30 and len(proof['sliding_areas'])==274 and len(proof['native_functions'])==28
 position=read(R.parent/'tools/ff5_library_20261101/puppet_position_provenance.json');assert len(position['functions'])==4
 dll=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==proof['dll_sha256']==position['dll_sha256']and digest(R.parent/'FF5/dump.cs')==proof['dump_sha256']==position['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in proof['native_functions']+position['functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 if check:assert proof['position_native']==position
 else:assert 'position_native'not in proof or proof['position_native']==position;proof['position_native']=position;path.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 actual={};bodies={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap']+'/'+s['name']
  if key in proof['source_script_hashes']:actual[key]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest();bodies[s['name']]=s['mnemonics']
 assert actual==proof['source_script_hashes']and len(bodies['sc_e_0060_1'])==175 and len(bodies['sc_map_30170'])==133
 master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if r['submap']=='Map_30170'];assert len(selected)==19
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  assert r['script']in bodies;reason='Full controlled-player currents land at52,14 or53,14 only after full native59;before then the encounter/pushback remains active. Complete guide selects the first automatic entry and observes actual landing/current position;274 permanent areas/957 cells are independently included in routing. No painted-floor path,manual counter,current cancellation or early story arc.'
  if r['entity']==65:reason='Root performs real northward pushback and prerequisite58/intro287 checks. Safe53,13 staging steps south;choice0 starts native448. Only full175-operation callback sets59. Declined/reloaded intro retries actual moved NPC61. Other and Navigation guidance are independently controlled.'
  if r['entity']==26:reason='Pre-battle encounter-return current is hidden after59 or63 and never receives a post-battle crossing companion. Complete guide covers native intro/pushback,encounter and current recovery.'
  authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=actual[r['submap']+'/'+r['script']],disposition='guided mechanic',guides=['desert_sandworm','desert_crossing'],reason=reason,evidence='docs/re/FF5_DESERT_20261105.md;10 complete scripts/30 actors/274 areas/28 exact native functions/four placement helpers;independent macro graph,physical routes and native owner.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Desert:19 identities plus274 permanent current areas;complete native choice,carries,story receipts and safe exit.')
if __name__=='__main__':main()
