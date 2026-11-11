"""Eleven exact Esper Cave/Thamasa discoveries; keep ordinary story separate."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0543':('complete manual bridge navigation',['esper_cave'],'Actual player Lerp37,53 then2 South owns this manual landing. One new same-floor arc retains the real control and approach mask; no automatic interaction or terrain rewrite.'),
'sc_e_0544':('complete manual bridge navigation',['esper_cave'],'Actual player Lerp38,54 then2 South owns this manual landing. Full guide includes both treasure branches and final Espers; physical entry and manual receipt use existing worker navigation.'),
'sc_e_0545':('complete manual bridge navigation',['esper_cave'],'Actual player Lerp48,53 then2 South owns this third bridge landing. Existing correct pit7/8/9 plus8 South arrivals are verified in place, without duplicate replacement arcs.'),
'sc_e_0541':('optional approach scene',['esper_cave'],'Sight receipt277 is not a gate required by the statue or battle. The initial approach includes it, but actual statue279 prevents forcing a missed sighting backwards.'),
'sc_e_0542':('optional approach scene',['esper_cave'],'Second sight278 is another navigation cue, not a required later story gate. Native group visibility is unchanged; the guide skips it after actual statue279.'),
'sc_e_0546':('complete story-puzzle navigation',['esper_cave'],'Actual statue information279 enables real battle region0547. Guide covers native Relm/Sketch solution as Other, battle280, both pit-branch treasures148/149 and actual430/24/full Thamasa return281.'),
'sc_e_0519':('ordinary optional information',[],'Boy magic scene269 contains no choice or required unlock. It remains a normal physical event; it is not promoted to a puzzle prerequisite.'),
'sc_e_0520':('ordinary optional information',[],'Woman magic scene270 is another optional information scene. No choice/order or story-gate dependency is invented from its state receipt.'),
'sc_e_0521':('ordinary native story introduction',[],'Actual introduction/naming scene owns271 and real return180/8. Naming Strago/Relm is ordinary UI; it does not stand in for later273 rescue or any puzzle answer.'),
'sc_e_0523':('ordinary native rescue introduction',['burning_house'],'Native273, Strago join/recovery and actual412/1 precede the existing complete burning-house guide. The physical NPC and current native group remain unchanged; no duplicate rescue guide or free travel edge.'),
'sc_e_0557':('ordinary party-sensitive conversation',[],'Active canonical Strago8 enables native Gung-Ho conversation receipt283; the alternate dialogue leaves283 unset. This changes the conversation, not access to the NPC. No path gate or Ebot design change is introduced.')}
def main():
 proof=json.loads((ROOT/'docs/re/esper_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==119 and set(proof['battle_scripts'])=={'sc_b_0547'}
 rows={};facts={};sources=None
 for name in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if set(row['area_names_en'])&{'Esper Cave','Thamasa'}:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==11 and sum(map(len,rows.values()))==11
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in ('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/ESPER_CAVE_GUIDE_20261104.md;119 episode scripts/exact battle asset/current actual guide and route consumers.')
 for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Esper/Thamasa:11 exact discoveries;complete cave/Sketch/treasures/full281;ordinary introductions/optional scenes/Gung-Ho dialogue remain separate.')
if __name__=='__main__':main()
