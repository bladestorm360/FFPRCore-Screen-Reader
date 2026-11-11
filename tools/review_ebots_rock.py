"""Three exact audit discoveries, linked to the complete coral capability."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
 rows={};facts={};sources=None
 for filename in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for r in d['candidates']:
   if r['submap'].startswith('Map_30861_'):rows.setdefault(key(r),[]).append(r)
 assert len(rows)==3
 reasons={
 'sc_e_0943':('guided native coral choice','Current-floor feeding requires at least22 coral in one batch; early feeding discards it. Canonical Strago8 must be active for the first encounter, not reserved;462 permits repeat feeding without him. Initial second-option/keep and later first-option/feed instructions repeat through the exact native choice context. Native540 local1 owns the opened gate; neither inventory possession nor the introduction flag fakes completion.'),
 'sc_e_0949':('guided native encounter approach','After successful feeding, first reach the surprise region and observe460, then reach native first Hidon6 or repeat Hidon5. First completion requires final461 after the whole311/305/310/305/310 scene chain; repeat uses463. No earlier intermediate map or proximity finishes the guide.'),
 'sc_e_0946':('guided native first/random passage','The first entrance pad goes to540 only while461 and area2 are both zero. Later uses delegate to fresh native RNG with12 possible arrivals. Current-room guidance collects reachable unopened coral chests221–225 or selects the reachable pad, including both entrance pockets. Normal directions explicitly describe a random attempt and actual arrival retires only that attempt; the same pad remains usable. No native random result is predicted or written.')}
 authored=[]
 for identity,variants in rows.items():
  r=variants[0];kind,reason=reasons[r['script']];result=dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts[r['submap']+'/'+r['script']]['sha256'],disposition=kind,guides=['ebots_rock'],reason=reason,evidence='docs/re/EBOTS_ROCK_GUIDANCE_20261104.md; source-pinned family/actual model/current-room routes/all random outcomes/controller/native role-reader and preservation evidence.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
 for r in old['decisions']:
  if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
  else:new['decisions'].append(r)
 new['decisions'].extend(r for r in authored if key(r)not in seen)
 if'--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Ebot:3 exact discoveries;',sum(map(len,rows.values())),'physical instances; complete current-room/random/feeding/first-repeat story coverage.')
if __name__=='__main__':main()
