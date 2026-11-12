"""Ordinary castle controls and one corrected conditional scripted floor exit."""
from pathlib import Path
import hashlib,json,sys
import pefile
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
REASONS={
'sc_s_20051_5_1':'One ordinary wall switch opens the treasury by retiring its walls and recording405. Exterior re-entry resets405. It is Objects/navigation coverage,not a puzzle;the draft guide was withdrawn under the owner November5 definition.',
'sc_npc_20051_5_1':'Ordinary minister reward behind the opened treasury. Full native staging grants item137,then the minister leaves/hides and515 records receipt. This does not turn travel plus one wall switch into complex puzzle steps.',
'sc_e_0127':'Full automatic Krile reunion joins the party,grants item57 and records127. No player-selected solution or complex puzzle operation.',
'sc_e_0129':'Full ordinary bridge conversation records129,updates GPS401,clears Teleport cache and changes to world19 PointIn26. Native story/party state remains authoritative;no new puzzle guide.',
'sc_map_20050_gate_1':'Two automatic approach contacts open the castle gate under514.126 changes music/presentation,not a key requirement. The existing ordinary routes already work at the tested natural first-entry state;no gate bypass or new guide was added.',
'sc_map_20050_gate_2':'Three automatic approach contacts open the same native gate,retiring the source sensors and collision actors. Existing valid routes are preserved;this ordinary unlock is not a puzzle.',
'sc_npc_20050_1':'Ordinary soldier greeting opens the same gate. Story branches choose dialogue/music;no key is required. The native gate/Objects behavior is retained without puzzle guidance.',
'sc_e_0362_3':'Two native unopened/opened counterparts are one ordinary pickup. Native169 determines whether item13 is granted and433 records opening;no puzzle input or early treasure-name disclosure.',
'sc_e_0362_4':'Two native unopened/opened counterparts are one ordinary pickup. Native169 determines whether item13 is granted and434 records opening;no puzzle input or early treasure-name disclosure.',
'sc_20051_2_to_20051_1':'Conditional ordinary floor exit. Before127 or after128,the real branch changes to72/4. At127/not128,ChangeScript with ToBeReturned=false replaces the caller with0128;that full scene records128 and stays on73. Exact arc guard prevents the fictitious immediate floor change. The door contact stays reachable,event required precedes the scene,and legitimate travel no longer gives a false trap warning.',
}
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;p=R/'docs/re/ff5_tycoon_native_20261105.json';proof=read(p);assert len(proof['native_functions'])==2 and len(proof['position_native']['functions'])==4
 dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==proof['dll_sha256']and digest(R.parent/'FF5/dump.cs')==proof['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in proof['native_functions']+proof['position_native']['functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if 'Castle Tycoon'in r['area_names_en']];assert len(selected)==15 and {r['script']for r in selected}==set(REASONS)
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 rooms={r['submap']for r in selected};wanted={r['submap']+'/'+r['script']for r in selected}|{'Map_20051_2/sc_e_0128'}|{room+'/sc_map_'+room.removeprefix('Map_')for room in rooms};actual={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap']+'/'+s['name']
  if key in wanted:actual[key]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert actual.keys()==wanted and len(actual)==15
 for r in selected:assert actual[r['submap']+'/'+r['script']]==master['source_script_hashes'][r['submap']+'/'+r['script']]
 if check:assert proof.get('source_script_hashes')==actual
 else:proof['source_script_hashes']=actual;proof['identities']=15;proof['ordinary_treasury_not_puzzle']=True;p.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=actual[r['submap']+'/'+r['script']],disposition='ordinary navigation/story interaction',guides=[],reason=REASONS[r['script']],evidence='docs/re/FF5_TYCOON_20261105.md;15 complete source bodies,two exact ChangeScript functions,four retained placement helpers,actual conditional-exit and all-six route checks;no puzzle guide.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Tycoon:ordinary controls remain Objects/no guide;15 exact identities and conditional suite-exit routing qualified from15 full bodies/two native methods.')
if __name__=='__main__':main()
