"""Five exact island discoveries plus the complete connected story mechanic."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0611':('complete beneficial-fish puzzle',['solitary_island'],'Actual tasty fish16 owns catch301/item41; actual spawn549 and current actor state determine availability. Complete Other guide prefers it, returns held catches to Cid7, rerolls through native0616 and covers both outcomes/hidden passage/full raft306 without exposing hidden HP or changing timers.'),
'sc_e_0612':('complete beneficial-fish fallback',['solitary_island'],'Actual ordinary fish17 owns catch302/item42 and remains a beneficial fallback when tasty fish is absent. Native feeding consumes every held type. All cabin/upper-island/beach legs and the complete recovery/death/bird/raft continuations are guided; no hidden health counter is announced.'),
'sc_e_0613':('harmful fish excluded from solution',['solitary_island'],'Actual harmful fish18 owns303/item43 and remains a normal valid physical interaction when spawned. The source-qualified solution never recommends it. Neither its existence nor a bad catch is invented as a required puzzle operation; both native story outcomes remain supported.'),
'sc_e_0614':('harmful fish excluded from solution',['solitary_island'],'Actual harmful fish19 owns304/item44. It is not a solution target or prerequisite. The guide states that Cid takes every held fish, uses native positive catches/actual reroll receipts and follows the real outcome rather than promising recovery after any input or timer history.'),
'sc_e_0963':('ordinary later animated pickup',[],'Animated magicite12 appears only after the native completed-outcome branch and353 while471 is unset. The actual acquisition sets471; it is not part of the fishing solution or a new puzzle gate. Representative off-floor routing must accept its real uncollected later state and reject early/consumed states, preserving type-only collectible naming.')}
def main():
 proof=json.loads((ROOT/'docs/re/solitary_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==44 and len(proof['actors'])==93
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Solitary Island']:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==5 and sum(map(len,rows.values()))==5
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/SOLITARY_ISLAND_GUIDE_20261104.md;44 exact scripts/93 physical facts/current guide,route/controller and previous-data preservation.')
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
 print('Solitary:five exact discoveries;complete beneficial-fish/both-outcome/raft guide;harmful fish remain optional physical controls and later magicite stays separate.')
if __name__=='__main__':main()
