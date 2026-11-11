"""Three exact discoveries; full connected actual pit/recruitment solution."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0787':('reviewed wrong-switch return',['yeti_cave'],'Manual switch15 intentionally falls to494/7 rather than the final chamber. Its complete player landing is retained; it is not disabled or presented as the solution. The guide chooses actual switch13 and has a complete recovery from this native return.'),
'sc_e_0788':('guided correct manual fall',['yeti_cave'],'Manual switch13 is the real route to496. Full continuation0788_1 places the player at58,14 then falls seven South steps. The guide covers every earlier floor/pit, this manual operation, the magicite Yes/boss397 and real Mog recruitment398; proximity cannot stand in for operation.'),
'sc_e_0790':('guided active-party recruitment',['yeti_cave'],'Actual recruitment requires active canonical Mog11; the leader check changes animation only. Ordinary conversation stays a valid destination without Mog. The Other setup instruction uses the existing bounded role pass, and full naming/joining398 plus native readiness completes the quest after the separate397 battle receipt.')}
def main():
 proof=json.loads((ROOT/'docs/re/yeti_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==87 and len(proof['actors'])==168
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==["Yeti's Cave"]:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==3 and sum(map(len,rows.values()))==3
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/YETI_CAVE_20261104.md;87 exact scripts/168 actors/seven complete fall landings/current route and owner-controller evidence.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Yeti:three exact discoveries;both actual switch outcomes,full cave/choice/battle/active-Mog recruitment398;no invented physical conversation gate.')
if __name__=='__main__':main()
