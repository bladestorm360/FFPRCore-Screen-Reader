"""Close the five exact Palace discoveries against the complete guide/source."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
 'sc_e_0370':('ordinary earlier story encounter',[], 'Pre-invitation226 palace approach sets181 in the earlier Magitek episode. It is separate from the later timed banquet tour, not a free travel edge or a new switch puzzle.'),
 'sc_e_0469':('complete timed-puzzle entry',['imperial_palace'],'Invitation226 enables the real escort region and native227. The guide follows the actual route to the Emperor before starting the240-second tour; proximity does not substitute for its scene receipt.'),
 'sc_e_0473':('complete timed-tour puzzle',['imperial_palace'],'Native229 starts240 seconds. All24 distinct soldier receipts230..253, including four battle callbacks, are guided on their actual floors. Finish-tour regions are advised only after all24 or native expiry; an unknown receipt cannot certify completion.'),
 'sc_e_0504':('complete current-choice puzzle',['imperial_palace'],'The current native Select occurrence distinguishes all banquet questions, including reused question/recall IDs, the real first-asked local4, and optional120-second break battle256. Resume uses real readiness and full257 arrival, not a remembered answer from another menu.'),
 'sc_e_0506':('complete native final reward scene',['imperial_palace'],'Native258 is the independent final reward receipt;255/256/257 do not finish the journey. Guide completion waits for actual field readiness after the whole scene. Internal score and undisplayed reward names are not narrated.')}
def main():
 proof=json.loads((ROOT/'docs/re/palace_native_20261104.json').read_text(encoding='utf-8'));rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Imperial Palace']:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==5 and sum(map(len,rows.values()))==5
 guide=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'));assert any(g['id']=='imperial_palace'for g in guide['story_guides'])
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/IMPERIAL_PALACE_GUIDE_20261104.md; palace_native_20261104.json exact90 sources/38 controls/16 choice sites; actual tour/controller/route/native contract checks.')
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
 print('Palace:five exact semantic discoveries;full24-soldier tour,current banquet/break choices and final258;early181 retained separately.')
if __name__=='__main__':main()
