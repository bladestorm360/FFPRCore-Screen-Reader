"""Fifteen exact Mobliz discoveries, with complete mail/hidden-house guides."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0223':('complete letter-quest conversation','mobliz_letters','Actual initial502 reveals the first letter; after five paid requests the same soldier owns final517. Read or paid flags alone do not finish, and the ordinary NPC remains a valid front interaction.'),
'sc_e_0224':('complete letter-quest reading','mobliz_letters','First animated letter owns503. Its actual visibility, identity and native operation precede postman504; neither proximity nor elapsed time is a receipt.'),
'sc_e_0227':('complete letter-quest reading','mobliz_letters','Second animated letter owns506 and requires the real505 shop-close/inn reply. Guide preserves its exact physical identity and the following507 payment.'),
'sc_e_0230':('complete letter-quest reading','mobliz_letters','Third letter509 follows real508 delivery and precedes510 payment; no generic current-floor-only animated-object fallback is needed.'),
'sc_e_0233':('complete letter-quest reading','mobliz_letters','Fourth letter512 follows511 delivery; source bank2/index256 aliases the same absolute bit. Native513 payment and514 delivery are independent.'),
'sc_e_0236':('complete letter-quest reading','mobliz_letters','Fifth letter515 precedes516 payment and final517 soldier reward. The guide never substitutes reading for payment or discloses future reward contents.'),
'sc_e_0225':('complete native payment choices','mobliz_letters','All five current native menus use first-option payment; refusal or insufficient funds does not supply a paid receipt. Exact88/10 can repeat its published answer without flag/party reads. Real0226 shop-close callbacks supply replies; no purchase/waiting timer is invented.'),
'sc_e_0644':('ordinary native prerequisite scene','mobliz_hidden_passage','The two equivalent child-notice regions own316. Ordinary earlier story access is retained; the later353-selected hidden-house guide includes this prerequisite if genuinely missed.'),
'sc_e_0648':('ordinary native prerequisite scene','mobliz_hidden_passage','First underground Terra scene owns317, followed by actual318 battle and319 return callbacks. The later hidden-house guide retains these missed prerequisites without granting them by approach.'),
'sc_e_0651':('native item/prerequisite scene','mobliz_hidden_passage','Magicite scene320 also sets current-area1, requiring a real village departure and re-entry before the later dog passage. The object remains a normal target; guide completion waits for final356.'),
'sc_e_0706':('complete hidden-passage navigation','mobliz_hidden_passage','Three alternative real cue regions start the dog route and own354. Source353 and320 plus actual area re-entry remain necessary; indoor strict routes are separate from the explicit non-strict world return.'),
'sc_e_0707':('complete hidden-passage discovery','mobliz_hidden_passage','Discovery355 leads to actual213/9 and both native final battle phases/full356 return. Finding the room, entering battle or temporary party changes cannot finish early.')}
def main():
 proof=json.loads((ROOT/'docs/re/mobliz_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==79
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Mobliz']:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==15 and sum(map(len,rows.values()))==15
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guide,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=[guide],reason=reason,evidence='docs/re/MOBLIZ_PUZZLES_20261104.md; exact79 connected producers; complete35-assertion/62-leg real-bake journey and630 story identities/current controller.')
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
 print('Mobliz:15 exact discoveries,two complete letter/hidden-house guides;native payments/replies/353/re-entry/full356 preserved.')
if __name__=='__main__':main()
