"""Native conversation/choice prerequisites, without inventing a new puzzle."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];key=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
 'sc_e_0056':('ordinary native escort conversation','Initial soldier conversation101 uses current-room local1 and native staging. Its pre42 group is distinct from the later exit soldier; a remote floor cannot borrow this current-room local.'),
 'sc_e_0058':('ordinary native story conversation','Locke conversation43 is one of the three actual party conversations before the exit soldier permits46. Repeated interaction follows the existing43 branch; no item/door/puzzle operation is invented.'),
 'sc_e_0059':('ordinary native story conversation','Sabin conversation44 is a real prerequisite alongside43/45. Its native scene owns the receipt; proximity or another party conversation cannot substitute.'),
 'sc_e_0060':('ordinary native story conversation','Edgar conversation45 supplies the third independent conversation receipt. Native availability and ordinary NPC routing remain unchanged.'),
 'sc_e_0061':('ordinary native guarded conversation','The later exit soldier requires all43/44/45 before setting46 and moving via the current-room local. Banon66/1 exists only after46 and before47; the front conversation remains approachable before satisfying its dialogue condition.'),
 'sc_e_0062':('ordinary native recruitment choice','Banon choice retains acceptance and repeated-refusal outcomes. First/second refusals544/545 return to67 without completing recruitment; the native area-local route and alternate scene continue to72 and set47 only in final0062_3. These NPC-controlled scenes are not free automatic navigation arcs.'),
 'sc_e_1036':('ordinary native optional choice','Paper choice follows its actual first/second outcomes76/77, changing optional scene details. It is not a hidden passage, required story switch, chest gate or separately guided puzzle.'),
 'sc_e_1108':('ordinary native optional conversation region','Wounded soldier sleep-talk scene records current-room local1. It is native dialogue/staging, without independent player puzzle input, permanent gate or an actionable transfer.')}
def main():
 proof=json.loads((ROOT/'docs/re/returners_native_20261104.json').read_text(encoding='utf-8'));assert len(proof)==22
 assert proof['Map_20081_6/sc_e_0062_3']['flags']==[['ScenarioFlag1',47,1]]
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==["The Returners' Hideout"]:rows.setdefault(key(row),[]).append(row)
 assert len(rows)==8 and sum(map(len,rows.values()))==8;authored={}
 for identity,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof[locator]['sha256']==wanted
  authored[identity]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=[],reason=reason,evidence='docs/re/RETURNERS_STORY_CONDITIONS_20261104.md; exact22 source producers and full choice/arrival47 chain, existing gate/current-local contracts preserved.')
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
 print('Returners:8 exact native conversation/choice identities;22 source producers; no runtime/bake/guide change.')
if __name__=='__main__':main()
