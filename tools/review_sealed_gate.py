"""Explicit disposition of the69 original bridge/switch discoveries, no game code."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
  assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for r in d['candidates']:
   if r['submap']in('Map_30531_3','Map_30531_4'):rows.setdefault(key(r),[]).append(r)
 assert len(rows)==69
 raw={}
 for line in(ROOT.parent/'tools/mapdump/ff6_objects.jsonl').open(encoding='utf-8'):
  a=json.loads(line)
  if a['submap']in('Map_30531_3','Map_30531_4'):
   raw[a['submap'],a['group'],a['props'].get('entity_id'),a['name']]=a
 reasons={48:'Native first-bridge activation/local5 is guided before the second crossing.',
 46:'Native second-bridge foot trigger/local6 collapses the approach; the closed-toggle recovery uses the actual410 round trip.',
 38:'Native left-island/local1 control is retained before the treasure route.',
 36:'Native right-island/local2 control is reversible; both native arrangements retain real exit routes.',
 69:'Actual forced-fall interaction is a manual same-floor arc to Ground45,24. Encoded native layer2 is not baked Upper2.',
 63:'Opposite-side native door control writes the same218 door receipt; the initial guide uses65, preserving this independent return-side control.',
 65:'Native door218 is an explicit guide stage after the forced fall.',
 60:'Native stairs219 are explicitly operated before the four hidden floor caches.',
 40:'Native foot trigger224 opens the side passage and is an explicit required stage.',
 58:'Native hidden floor-cache220 is examined before full completion.',
 57:'Native hidden floor-cache221 is examined before full completion.',
 56:'Native hidden floor-cache222 is examined before full completion.',
 55:'Native hidden floor-cache223 is examined before full completion.',
 34:'Native first-option toggle cycles bridge3. A closed state is restored before collapse, or reset through410 afterward; default-open is not needlessly closed.'}
 authored=[]
 for identity,variants in rows.items():
  r=variants[0];a=raw[identity];id=r['entity']
  if r['submap']=='Map_30531_3':
   assert r['type']=='Event'
   if r['script']=='sc_e_0445':
    kind='timed hazard';reason='Exact phase-dependent fall trigger retained. Native callback tables override the initializer; eight short cardinal crossings use both phases, with stable waiting and bounded recovery. Falling resets locals1–4 at Ground7,8.'
   else:
    assert id in(9,10,12,13)and r['script']in('sc_e_0443','sc_e_0444')
    kind='guided mechanic';reason='Real native bridge-mode/timer activation, including both entry sides. The owned guide starts at the actual trigger, previews cardinal segments and requires explicit readiness plus a fresh matching native phase.'
  elif r['script']:
   assert id in reasons;kind='guided mechanic';reason=reasons[id]
  elif r['type']=='AnimEntity':
   assert id in(83,84)and not a['props']['collision']and r['action']==0
   kind='decorative counterpart';reason='Native island-animation counterpart with no operation or collision; actual island controls36/38 and solid geometry retain ownership.'
  else:
   assert id in(49,50,76,77,78,71,72,73,59,62,81,43,44,74)
   assert a['props']['collision']and r['action']==0 and not a['props']['script_id']
   kind='physical gate counterpart';reason='Exact solid native bridge/island/door obstruction, with no independent player interaction. Initializer/control projections and actual route checks retain its visibility/collision; it is never replaced by a walk-through shortcut.'
  out=dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},
   script_sha256=facts.get(r['submap']+'/'+r['script'],{}).get('sha256'),
   disposition=kind,guides=['sealed_gate'],reason=reason,
   evidence='docs/re/SEALED_GATE_GUIDANCE_20261103.md; exact native callbacks/tables/layer conversion,37 physical journeys/six continuations,actual model/controller and prior-guide regressions.')
  if len(variants)>1:out['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(out)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old)
 replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
 for r in old['decisions']:
  if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
  else:new['decisions'].append(r)
 new['decisions'].extend(r for r in authored if key(r)not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Sealed Gate:69 exact original discoveries;',sum(map(len,rows.values())),'physical instances; all previous decisions retained.')
if __name__=='__main__':main()
