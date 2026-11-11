"""Six exact original Narshe locks; no game implementation is stored here."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(row):return row['submap'],row['group'],row['entity'],row['raw']
def main():
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
  assert sources is None or sources==inventory['source_sha256'];sources=inventory['source_sha256'];facts.update(inventory['script_facts'])
  for row in inventory['candidates']:
   if row['submap']=='Map_20010'and row['group']=='ev_e_0703'and row['entity']in(115,116,117,118,119,114):rows.setdefault(key(row),[]).append(row)
 assert len(rows)==6
 proof=json.loads((ROOT/'docs/re/narshe_party_gates_native_20261103.json').read_text(encoding='utf-8'))
 authored=[]
 for identity,variants in rows.items():
  row=variants[0];assert row['type']=='Entity'and row['action']==2
  digest=facts[row['submap']+'/'+row['script']]['sha256'];assert digest==proof['script_sha256'][row['submap']+'/'+row['script']]
  result=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=digest,
   disposition='native active-party door capability',guides=[],
   reason='Native active-corps canonical Locke2, rather than an owned reserve member, can unlock this exact solid door. Front remains reachable. Behind-door goals require that ability or the native411..416 opened receipt. Confirmed ability gives a manual interaction prefix, never a native write, consumable mutation or passage through a still-closed door. Flag353 enables the World of Ruin lock group. Exact initializer21 is visual-only; opaque character branch86 remains conservative.',
   evidence='docs/re/NARSHE_PARTY_LOCKS_20261103.md; actual reader18 cases, six-door route/browse93 assertions, unchanged FFIII246859 key/Thief assertions and FFI all-map/Cornelia277 checks.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as stream:assert hashlib.file_digest(stream,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(row):row for row in authored};seen=set();new['decisions']=[]
 for row in old['decisions']:
  if row['game']=='ff6'and key(row)in replacement:new['decisions'].append(replacement[key(row)]);seen.add(key(row))
  else:new['decisions'].append(row)
 new['decisions'].extend(row for row in authored if key(row)not in seen)
 if'--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Narshe six exact native locks;',sum(map(len,rows.values())),'physical instances; previous decisions preserved.')
if __name__=='__main__':main()
