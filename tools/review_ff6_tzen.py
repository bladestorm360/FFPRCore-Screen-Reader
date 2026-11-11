"""Four exact controls in the distinct complete Tzen timed-rescue episode."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0634':('guided rescue opening','Two town markers play the actual quake and record309. The complete Tzen guide starts at either native marker, then reaches the real start event52; this is independent of Thamasa’s accepted burning-house guide.'),
'sc_e_0636':('guided native timed-rescue start','Event52 records310 and starts native360-second timer0 with Resident0638 timeout. Celes affects dialogue only,not eligibility. The guide warns before this operation, then covers both real floors,child311,actual exit and full rescue312. It never restarts the timer after an ordinary retreat.'),
'sc_e_0637':('guided child pickup and full return','Actual child434/9 records311 and hides after its animation. Reaching or rescuing the child is not final completion:the guide routes back to real433/0;native0639 stops the timer,records312 and subsequently joins Sabin. Final field readiness is required. This is Tzen,not the accepted Thamasa maps412–420.')}
def main():
 proof=json.loads((ROOT/'docs/re/tzen_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==34 and len(proof['actors'])==71
 assert proof['facts']['Map_20330/sc_e_0636']['timers']==[[24,'SetTimer',0,360,'sc_e_0638']]
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']in(['Tzen'],['Crumbling House']):rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==4 and sum(map(len,rows.values()))==4
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=['tzen_rescue'],reason=reason,evidence='docs/re/TZEN_RESCUE_20261104.md;34 exact scripts/71 actors/native360-second timer/complete production routes and full312 owner readiness.')
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
 print('Tzen:four exact controls;full native timed rescue,child311/return/full ready312;Thamasa guide and native timer behavior preserved.')
if __name__=='__main__':main()
