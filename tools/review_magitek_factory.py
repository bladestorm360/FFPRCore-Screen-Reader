"""Exact27 factory/research identities with source-qualified dispositions."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
COHORT={'Map_30521_1':(46,62,63,57,33,60,61,31,58,59,29),'Map_30521_11':(62,63,57,33,29),'Map_30521_2':(62,31,32,33),'Map_30521_3':(16,14,11,1),'Map_30521_8':(30,7),'Map_30521_9':(1,)}
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
 rows={};facts={};sources=None
 for filename in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for r in d['candidates']:
   if r['entity']in COHORT.get(r['submap'],()):rows.setdefault(key(r),[]).append(r)
 assert len(rows)==27;authored=[]
 for identity,variants in rows.items():
  r=variants[0];name=r['script']
  if name in('sc_e_0380','sc_e_0386','sc_e_0387','sc_e_0388','sc_e_0389_1'):
   kind='native routing passage';reason='Native ladder/relative-layer/conveyor transfer, handled by exact physical passage directions rather than an ordered puzzle recipe. Eleven source-qualified arc variants retain valid approach sides and native landing cells. Manual crossings require the real interaction; the conveyor remains automatic. Normal directions now retain the exact same-floor receipt beside the control instead of losing continuation. No terrain or story state is bypassed.'
  elif name in('sc_e_0380_0','sc_e_0386_0','sc_e_0387_0'):
   kind='automatic vent presentation counterpart';reason='Exact direction/local1..3 flap frame and trigger-visibility operation around the native ladder. These change companion trigger visibility/animation/sound rather than introduce another ordered puzzle. Source initializer/local projection and native interaction remain authoritative; the corresponding physical transfer has exact source-qualified route arcs.'
  elif name=='sc_e_0393':
   kind='automatic native story counterpart';reason='Three contact regions trigger the same Kefka/esper scene and183 receipt. They are counterparts of one native scene, not three puzzle steps; no new route, reward or completion inference is added.'
  elif name=='sc_e_0397':
   kind='native battle aftermath interaction';reason='Before185 this ordinary Shiva interaction records187/message only. After the native battle receipt185 it reveals the two magicite counterparts and records186. Source stage groups/collection flags remain authoritative; route links do not expose or collect either pickup early.'
  elif name in('sc_e_0399','sc_e_0400'):
   kind='animated native pickup';reason='Single post-battle animated magicite pickup, with exact189/190 collection receipt. The shared AnimEntity collector/remote target policy already covers this type; no contained reward is added to a treasure label and collected/hidden counterparts stay unavailable.'
  elif name=='sc_e_1110':
   kind='hidden native passage';reason='Single manual passage through the hidden chest route to397/PointIn11. Its continuation finishes at17,28 Ground, matching that actual arrival. Existing transfer is preserved; no speculative geometry or extra puzzle step is added.'
  elif name=='sc_e_0402':
   kind='single native story switch';reason='One research-facility switch runs the native capsule/party scene and192 receipt. No ordered switch pattern or timed input is required. The real control7 remains the operation; its unbound visual counterpart30 is not an additional goal.'
  elif name=='sc_e_0404':
   kind='ordinary native story conversation';reason='Cid conversation advances194 and the native minecart/escape movie578 continuation. This is a scene/battle sequence rather than a direct walkable passage. Existing actor/gate/continuation ownership is retained; no early escape or vehicle requirement is invented.'
  else:
   assert not name and r['submap']=='Map_30521_8'and r['entity']==30 and r['action']==0
   kind='non-interactive switch counterpart';reason='Action0/scriptless switch appearance beside actual control7. It is not an independent player operation or a second missing puzzle step; preserve its source identity and ordinary collision/visibility.'
  result=dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(r['submap']+'/'+name,{}).get('sha256'),disposition=kind,guides=[],reason=reason,evidence='docs/re/MAGITEK_PASSAGES_20261104.md; exact six source initializers/27 identities, eleven real native arcs, production side/receipt/gating checks and all-six route regression.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
 for r in old['decisions']:
  if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
  else:new['decisions'].append(r)
 new['decisions'].extend(r for r in authored if key(r)not in seen)
 if'--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Factory/research:27 exact identities;',sum(map(len,rows.values())),'physical instances; prior dispositions preserved.')
if __name__=='__main__':main()
