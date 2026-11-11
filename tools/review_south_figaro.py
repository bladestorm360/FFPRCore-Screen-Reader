"""Exact original-town and original-cave discoveries; later Gerad cave stays open."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
REASONS={
 'sc_e_0072':('guided native guard operation','Only the verified soldier disguise53 moves this guard and sets54. Merchant52 is not equivalent. The guide uses green-soldier41 Steal, not red-soldier65 or a guessed costume.'),
 'sc_e_0079':('guided native recruitment choice','After the separate sighting and costume-removal operations, first-option recruitment sets59 and clears costumes. It does not substitute for the sleeping-guard key or clock.'),
 'sc_e_0082':('guided native key choice','After59, first-option interaction obtains native key40 and sets60. Guidance repeats the current native choice and requests the clock only after the actual receipt.'),
 'sc_e_0083':('guided native clock operation','First-option clock operation requires actual60 and sets61, moving passage blocker14. Front control remains a target before opening; the route behind it retains native state.'),
 'sc_e_0084':('guided native exit conversation','Actual exit region sets62. Guidance continues through the real walking-world cave connection, both subsequent noise receipts and Tunnel Armor64; this conversation is not the final escape completion.'),
 'sc_e_0069':('guided native disguise passage','Only merchant52 permits the boy to move. Current-room local1 owns his pose. Guidance uses the real room entrances and returns to a green soldier only after the opened approach; the cider return also uses merchant Steal.'),
 'sc_e_0076':('guided native password choice','Second option opens57. Wrong options return to town13 and remove costumes but preserve55/56, so the guide resumes the password. The secret basement passage is traversed before the northern mansion approach.'),
 'sc_e_1048':('corrected guided native passage','Two-stage basement transfer uses town38 only for a hidden-player camera animation and ends on58/PointIn1. Its reverse ends on57/PointIn1. Both corrected derived graph arcs retain native terrain, physical objects, source guards and controls.'),
 'sc_e_1080':('guided native draft region','Four alternate bedroom regions start the same draft sound and current-room local1, hiding those regions. Strict guidance reaches one real draft region before the secret stair; it does not incorrectly forbid all four as intervening events.'),
 'sc_e_0044':('ordinary native conversation','Early-town merchant conversation and movement31 belong to the ordinary encounter, distinct from the cider quest old man on the same floor. No sequence/choice puzzle is introduced.'),
 'sc_e_0139':('ordinary native conversation','Duncan wife uses current-character/plot dialogue and same-room native staging100. Ordinary NPC routing/conversation owns this interaction; there is no independent puzzle input sequence.'),
 'sc_e_0041':('ordinary native conversation','Shadow first meeting/name entry and receipt28 are a normal conversation with native staging. No hidden-passage or ordered puzzle control is present.'),
 'sc_e_0042':('ordinary native conversation','Initial-town merchant29 conversation/staging is distinct from later cider merchant7. The native conversation remains available through ordinary NPC routing.'),
 'sc_e_0665':('ordinary native conversation','Later-town Jeff/Gerad conversation sets330 and performs native facing/staging. It is not a costume/password/clock counterpart; later Gerad cave discoveries remain separately pending.'),
 'sc_e_0038':('ordinary native conversation','Original cave soldier26 conversation and Edgar/vehicle staging precede this scenario. Ordinary NPC interaction retains its actual story availability; no independent puzzle is present.'),
 'sc_e_0039':('ordinary native healing region','First spring interaction restores party HP/MP/conditions and sets98. It is a native healing event, distinct from the escape sound/boss sequence.'),
 'sc_e_0085':('guided native escape region','Final cave warning63 follows582/583 on the normal route and suppresses both earlier regions. A resumed63 goes straight to Tunnel Armor rather than requesting unavailable old sounds. Native51 selects the real626..629 interior aliases.'),
 'sc_e_0085_1':('guided native escape region','Cave noise582 is reached physically before583 and the boss. Native story state owns completion; proximity does not set it.'),
 'sc_e_0085_2':('guided native escape region','Cave noise583 follows582 while63 remains zero. Guidance then reaches final warning63 before Tunnel Armor. Completion requires actual64 and field readiness after the resulting scene.')}
def main():
 rows={};facts={};sources=None
 for filename in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for r in d['candidates']:
   if 'South Figaro'in r['area_names_en']or r['submap']in('Map_30030','Map_30031_1','Map_30031_2','Map_30031_4'):rows.setdefault(key(r),[]).append(r)
 assert len(rows)==26
 authored=[]
 for variants in rows.values():
  r=variants[0]
  if not r['script']:
   if r['submap']=='Map_20071_14':kind,reason='decorative counterpart','Native clock visual20/blocker14 belongs to the real clock control1; blocker moves after61. Front operation is guided without making the passage prematurely walkable.';guides=['south_figaro_infiltration']
   else:assert r['submap']=='Map_20361_14';kind,reason='decorative map object','Later-world scriptless clock visual has no interaction producer or puzzle operation. It must not inherit the earlier infiltration clock key/door behavior.';guides=[]
  else:
   kind,reason=REASONS[r['script']];guides=['south_figaro_infiltration']if kind.startswith(('guided','corrected'))else[]
  result=dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(r['submap']+'/'+r['script'],{}).get('sha256'),disposition=kind,guides=guides,reason=reason,evidence='docs/re/SOUTH_FIGARO_INFILTRATION_20261104.md; exact source/battle receipts, native arrivals and actual model/planner/controller/recovery/preservation qualification.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
 for r in old['decisions']:
  if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
  else:new['decisions'].append(r)
 new['decisions'].extend(r for r in authored if key(r)not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('South Figaro:26 original-town/cave identities;',sum(map(len,rows.values())),'physical instances; later Gerad cave remains pending.')
if __name__=='__main__':main()
