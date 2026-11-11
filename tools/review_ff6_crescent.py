"""Eight exact Crescent/Trench discoveries, one complete connected guide."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0270':('complete native repeat-dive choice','Owned helmet133 permits first-option repeat diving. Both Right choices visit the two real cave arrivals; guide includes both treasure receipts112/113, current-area drain and full ready96/521. Neither owned airship nor Gau is invented as a repeat requirement.'),
'sc_e_0242':('complete helmet-search prerequisite','Actual active Gau12 gates the physical search regions. Information129 leads to real helmet133, then both timed underwater branches and treasure caves. Reserve/unknown Gau cannot validate the behind-gate path.'),
'sc_e_0243':('optional wrong-search scene','Native130 is an optional unsuccessful search, not a helmet prerequisite or completion receipt. The guide avoids this region while reaching the real helmet and both subsequent cave treasures.'),
'sc_e_0244':('optional search reward scene','Native131 is optional and independent of helmet133. It remains a normal physical destination; the complete main journey neither requires it nor reveals future reward contents.'),
'sc_e_0245':('optional search prank scene','Native132 is an optional prank, not a necessary puzzle step. Strict routes avoid it; active-Gau event state and ordinary target access remain valid.'),
'sc_e_0246':('complete helmet and timed-navigation journey','Helmet133 starts the actual automatic377/573 continuation. It does not complete guidance. Native rendered Right0 choices at both forks visit379/1 and380/1; both real chests, drain and final ready96/521 remain required.'),
'sc_e_1069':('complete resettable water-drain control','Native1069 hides the real water blocker and writes current-area1 plus persistent74. Initializer checks current-area1, so an earlier74 cannot reopen a reset visit. Guide routes the real control, second chest and actual final exit.')}
def main():
 proof=json.loads((ROOT/'docs/re/crescent_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==29
 rows={};facts={};sources=None
 for name in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if set(row['area_names_en'])&{'Crescent Mountain Cave','Serpent Trench'}:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==8 and sum(map(len,rows.values()))==8
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in ('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=['crescent_serpent_trench'],reason=reason,evidence='docs/re/CRESCENT_SERPENT_GUIDE_20261104.md;29 connected native scripts/current fork and rendered-arrow producers;actual role/model/controller/route checks.')
 for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Crescent/Trench:8 exact discoveries;active-Gau search,optional scenes,two timed Right branches,both treasures,current-area drain/full final521.')
if __name__=='__main__':main()
