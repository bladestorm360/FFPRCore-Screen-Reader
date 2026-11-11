"""Seven exact remaining Cyan/Zozo discoveries and their connected native contracts."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0726':('guided letter opening',['lola_reply'],'Actual Lola14 records357 and enables the physical letter. After a declined offer529, the same actor delegates1060 and only actual accepted358 advances. Later365/530 conversation is an optional epilogue,not a new access gate.'),
'sc_e_0727':('guided read and delivery choices',['lola_reply'],'First option reads,then first option accepts reply358/item46. Declining sets529 without358;guide routes back to Lola1060 rather than treating529 as completion. Actual pigeon69 delegates0729 only after358,then full native town return359 completes delivery. Active Cyan mode0/third operand0 has a separate replacement-letter420 branch after recruitment;no forged delivery receipt.'),
'sc_e_0730':('guided optional bird breadcrumb',['cyan_zozo'],'Actual359 enables the bird only before360/365. Its directional flight records360 but is not a door prerequisite. The guide may follow this delivered-letter breadcrumb before purchase;owning361 bypasses prior delivery and no route gate requires360.'),
'sc_e_0732':('guided item-enabled manual door',['cyan_zozo'],'The closed actor104 always has a physical approach. Only native purchase361 can validate a journey behind it before actual opened362;the planner stops at the real door for manual interaction. Native merchant100 delegates0731 only after359;first option and successful1000-gil payment set361/item50. Missing/unknown361 cannot certify an onward path. No inventory or Locke assumption opens this lock.'),
'sc_e_0733':('ordinary native interaction',[],'Exact switch472/1 records363 and releases native moving dragon2;victory callback sets364 and calls shared dragon accounting0975. The switch and ordinary fight do not need a separate guide. Ordinary paths remain independent of dragon victory.'),
'sc_e_0736':('guided chest key',['cyan_zozo'],'The summit key475/1 is shown only after actual Cyan365 and before366. Its native pickup hides1/2 and records366;no inventory guess. Guide follows the full reunion return at native42,56 on474,then reaches key and returns to chest7.'),
'sc_e_0737':('guided locked chest',['cyan_zozo'],'Chest7 remains physically approachable before key366. Native366 enables its actual opening and367;key possession,Cyan joining or reaching the chamber does not complete the treasure stage. Physical treasure17 is animated to42,54 and is not substituted for the real locked control. Type-only treasure labels;Cyan-party branches add a reaction,not a reward or route prerequisite.')}

def main():
 proof=json.loads((ROOT/'docs/re/cyan_zozo_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==229 and len(proof['actors'])==622
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['script']in REASONS:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==7 and sum(map(len,rows.values()))==7
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/CYAN_ZOZO_GUIDANCE_20261104.md;229 exact connected scripts/622 physical facts,actual native branch/purchase/unlock and complete production-route checks.')
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
 print('Cyan/Zozo:seven exact controls,complete native letter/purchase/reunion/key/chest/optional dragon;all earlier dispositions preserved.')
if __name__=='__main__':main()
