"""Twelve source-pinned discoveries in the complete connected Cyan episode."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0896':('guided first reunion approach',['dream_reunion'],'Two physical sighting markers set436, not the actual437 member join. The complete guide follows looping native doors, reaches the real55 interaction and preserves both receipts.'),
'sc_e_0897':('guided actual first member join',['dream_reunion'],'Actual member interaction55 performs the native slot-two puppet exchange/merge and records437. Sightings and proximity cannot substitute. Entry requires active Cyan3/four members; the whole journey continues through the actual second join and brothers.'),
'sc_e_0898':('guided second reunion approach',['dream_reunion'],'Two physical sighting markers set438. The guide routes through the actual same-map doors to these markers, then real second-member interaction51/439. It never treats this scene alone as a reunion.'),
'sc_e_0899':('guided actual second member join',['dream_reunion'],'Actual interaction51 merges the third-slot member and records439. Both437/439 qualify brothers461;440 victory selects the real441 train arrival. The accepted train remains unchanged and later armor/Doma guidance resumes at native449.'),
'sc_e_0936':('ordinary optional family memory',[],'Two outdoor story markers play an automatic family memory and record452. No choice or player-operated puzzle occurs, and native0939 boss entry does not test452. Ordinary availability is preserved; the guide does not invent a mandatory story gate.'),
'sc_e_0937':('ordinary optional family memory',[],'Two outdoor markers play an automatic family memory and record453. The source has no separate puzzle action or branch to solve. Native boss entry has no453 prerequisite; the full guide covers family room454, the possession mechanic, waking and final pickup without locking ordinary alternatives.'),
'sc_e_0940':('guided final animated pickup',['dream_doma'],'Animated1 and interaction5 are physical aliases for one native acquisition. Both appear after455 while456 is unset; actual0940 hides both and sets456. Full guidance continues across normal Doma floors after waking, retains type-only naming and completes only at the native acquisition receipt.')}
def main():
 proof=json.loads((ROOT/'docs/re/dream_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==95 and len(proof['actors'])==300 and 'mechanic_sources'in proof
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']in(['Dreamscape'],['Dreamscape - Doma Castle'],['Doma Castle']):rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==12 and sum(map(len,rows.values()))==12
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/DREAM_EPISODE_20261104.md;95 exact scripts/300 physical facts/seven resolved battle assets/eight native functions/current route/controller and prior-data preservation.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Dream:twelve exact discoveries;complete active-party/reunion/armor/Doma/possession/final-pickup guidance;optional memories remain ordinary,not invented gates.')
if __name__=='__main__':main()
