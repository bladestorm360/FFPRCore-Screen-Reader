"""Factual dispositions for all seven paid rentals; no automatic free travel."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
key=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((ROOT/'docs/re/ff6_stables_native_20261104.json').read_text(encoding='utf-8'));assert len(proof)==21
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Chocobo Stable']:rows.setdefault(key(row),[]).append(row)
 assert len(rows)==7 and sum(map(len,rows.values()))==7
 authored={}
 for identity,variants in rows.items():
  row=variants[0];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof[locator]['sha256']==wanted
  assert proof[locator]['flag_writes']==[['ScenarioFlag3',36,True]]
  first,=proof[locator]['changes'];assert first[2]==row['script']+'_1'
  continuation,=[v for k,v in proof.items()if k.endswith('/'+first[2])];world,=continuation['changes'];assert world[0]in(1,2)and world[2]==row['script']+'_2'
  last,=[v for k,v in proof.items()if k.endswith('/'+world[2])];assert len(last['syscalls'])==2 and not last['changes']and not last['flag_writes']
  authored[identity]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition='ordinary paid vehicle service',guides=[],reason='Ordinary repeatable paid chocobo rental. First-option confirmation and successful native fee removal are required before the staged outer-map transfer and world-mounted callback. Intro548 is not a one-time consumption/gating receipt. Merchant remains a valid interaction target without funds; no free automatic route arc, puzzle sequence or premature vehicle ownership is added.',evidence='docs/re/FF6_STABLE_RENTALS_20261104.md; all seven roots and fourteen actual arrival callbacks have exact source fingerprints in ff6_stables_native_20261104.json.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  identity=key(row)
  if row['game']=='ff6'and identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFVI stables:7 exact paid-service identities/21 native source producers; no runtime/bake/guide change.')
if __name__=='__main__':main()
