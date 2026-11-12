"""Eight exact controls plus complete independent collapse/arrival continuation."""
from pathlib import Path
import hashlib,json,sys
import pefile
from author_ff5_gohn import author
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;guide,override,proof=author();assert read(R/'docs/re/ff5_gohn_native_20261105.json')==proof
 native=proof['position_native'];dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==native['dll_sha256']and digest(R.parent/'FF5/dump.cs')==native['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in native['functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if r['submap']=='Map_30180'];assert len(selected)==8
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  reason='Complete city search follows60,then365,then either366 or367,then368 and actual collapse10. The source initializer permits the final sighting after either middle branch;all five physical middle contacts remain candidates. The complete471-operation underground callback restores actual player17,10 Ground and records61 only at its end. No selected answer,timing or special battle input occurs;Navigation owns this full journey.'
  authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=proof['source_script_hashes'][r['submap']+'/'+r['script']],disposition='guided mechanic',guides=['gohn_city_search'],reason=reason,evidence='docs/re/FF5_GOHN_20261105.md;nine complete scripts/eight original identities plus independent final collapse;actual branches,physical routes and native-ready owner.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gohn:eight identities plus final collapse/full underground callback;nine pinned bodies,four native placement helpers,real branch order.')
if __name__=='__main__':main()
