"""Explicit review of three repeated FFVI cinematic lighting identities.

Every physical instance is retained. No game body or automatic label-based
classifier is emitted; these exact non-interactive source identities are fixed.
"""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
SPECS={('Map_40006','ev_e_0404_4',0,'ライト反転'):21,
       ('Map_40006','ev_e_0404_4',0,'ライト'):11,
       ('Map_40006_1','ev_e_0404_7',0,'天井照明'):3}
def identity(r):return r['submap'],r['group'],r['entity'],r['raw']
def order(r):return r['type'],tuple(r['cell']),r['action'],r['script']
def main():
 candidates={k:[] for k in SPECS};source_hashes=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
  if source_hashes is not None:assert source_hashes==inventory['source_sha256']
  source_hashes=inventory['source_sha256']
  for r in inventory['candidates']:
   if identity(r)in candidates:candidates[identity(r)].append(r)
 # Bind absence of an action/script to the exact object and script-name inputs.
 for role,name in(('objects','ff6_objects.jsonl'),('script_master','ff6_master/script.txt')):
  digest=hashlib.sha256()
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
   for chunk in iter(lambda:f.read(1024*1024),b''):digest.update(chunk)
  assert digest.hexdigest()==source_hashes[role],(role,'primary source changed')
 path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';review=json.loads(path.read_text(encoding='utf-8'))
 expected=dict(review);authored=[]
 for key,count in SPECS.items():
  rows=candidates[key];assert len(rows)==count and all(r['type']=='AnimEntity'and r['action']==0 and r['script']==''for r in rows)
  instances=sorted((dict(type=r['type'],cell=r['cell'],action=r['action'],script=r['script'])for r in rows),key=order)
  assert len({order(r)for r in instances})==count
  authored.append(dict(game='ff6',submap=key[0],group=key[1],entity=key[2],raw=key[3],script='',script_sha256=None,
   instances=instances,disposition='cinematic scenery',guides=[],
   reason='Non-interactive lighting props: AnimEntity, action0, no bound script and zero logical ID. Every listed physical position is preserved; none is a player-operated puzzle or story gate.',
   evidence='Exact immutable discovery identities and current external object/script-master hashes; physical-instance review tools/review_puzzle_scenery_instances.py.'))
 # Preserve existing decision positions as other finite families are added.
 replacements={identity(r):r for r in authored};seen=set()
 expected['decisions']=[]
 for old in review['decisions']:
  if old['game']=='ff6'and identity(old)in replacements:
   assert identity(old)not in seen;seen.add(identity(old));expected['decisions'].append(replacements[identity(old)])
  else:expected['decisions'].append(old)
 expected['decisions'].extend(r for r in authored if identity(r)not in seen)
 if '--check'in sys.argv:assert review==expected,'Scenery instance decisions differ'
 else:path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFVI repeated cinematic lighting:35 physical instances/3 explicit identities; no instance omitted.')
if __name__=='__main__':main()
