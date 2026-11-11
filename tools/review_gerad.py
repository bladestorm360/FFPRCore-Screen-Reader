"""Exact eighteen remaining Figaro/Nikeah/cave discovery dispositions."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
REASONS={
 'sc_e_0024':('ordinary native arrival region','Three alternative entrance regions share the native greeting and area-local1/580 receipt. The actual script hides the regions after the greeting. It is arrival dialogue, with no player puzzle sequence or terrain operation.'),
 'sc_e_0030':('ordinary native story conversation','First soldier conversation sets18; it is one of the two actual prerequisites for the later Kefka conversation, not a key item, switch or hidden floor transfer.'),
 'sc_e_0031':('ordinary native story conversation','Second soldier conversation independently sets19. Native Kefka checks both18/19; neither this ordinary target nor its conversation is treated as a locked door.'),
 'sc_e_0032':('ordinary native story conversation','Kefka checks both soldier conversations18/19, then the native scene sets20. Earlier attempts retain their alternate conversation. No independent puzzle inputs or invented movement connection are added.'),
 'sc_e_0033':('ordinary native story conversation','After actual20, Locke initiates the normal staged departure/party21 sequence. Earlier dialogue remains distinct. Native staging/route assignment is not player-controlled puzzle movement.'),
 'sc_e_0026':('ordinary native recruitment conversation','Throne-room Edgar conversation owns name entry, native staging and15. It is an ordinary character introduction, not the later Gerad/turtle/engine quest or a puzzle control.'),
 'sc_e_0671':('guided native jail entry','Actual jail region advances334 after fleeing Siegfried333. The Gerad guide continues through the real engine entrance and Tentacles335, then requires the final surfacing336 scene before completion.'),
 'sc_e_0666':('guided native cave conversation','Cave Siegfried sets331. Guide then uses the real449-to448 entry continuation to start332, retaining its actual post-scene player pose before the turtle bank. Standing near the turtle cannot substitute for332.'),
 'sc_e_0670':('guided native cave escape region','Fleeing Siegfried region sets333 after the recurring turtle crossing. Both banks use the actual counter approach/facing and native0668/0669 landing; guidance proceeds through jail334/engine335/surfacing336.'),
 'sc_e_0247':('ordinary native conversation','Original Nikeah dancer stages dialogue/actor animation and first134. The complete call/puppet closure contains no independent navigation puzzle input or gated passage operation.'),
 'sc_e_0652':('ordinary native information region','Four alternative regions stage the magicite information conversation and321, then hide their own triggers. This informational NPC scene is independent of the four pub conversations322..325 and Gerad departure326.'),
 'sc_e_0659':('guided native following sequence','Two actual Gerad actor identities share the follow root. Four pub conversations cause326, town initialization327, then exact native routes201203/201204 write575/576 at their real endpoints. Guide waits for those receipts, handles the visible actor variant, reaches328 and first boarding329, then continues the complete cave/castle journey.')}

def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
 proof=json.loads((ROOT/'docs/re/gerad_native_20261104.json').read_text(encoding='utf-8'));selected={tuple(v)for v in proof['discovery_keys']}
 assert len(selected)==18
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if key(row)in selected:rows.setdefault(key(row),[]).append(row)
 assert rows.keys()==selected
 authored={}
 for identity,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  result=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=['gerad_passage']if kind.startswith('guided')else[],reason=reason,evidence='docs/re/GERAD_PASSAGE_20261104.md; exact connected script/follow-asset fingerprints and actual model/planner/controller/bake preservation checks.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored[identity]=result
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  identity=key(row)
  if row['game']=='ff6'and identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(row)
 new['decisions'].extend(row for identity,row in authored.items()if identity not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gerad:18 exact logical identities/',sum(map(len,rows.values())),'physical instances reviewed; ordinary early scenes remain distinct.')
if __name__=='__main__':main()
