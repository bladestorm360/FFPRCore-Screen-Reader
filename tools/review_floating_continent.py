"""Explicit disposition of69 original discoveries; source bodies stay external."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(row):return row['submap'],row['group'],row['entity'],row['raw']
def main():
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
  assert sources is None or sources==inventory['source_sha256'];sources=inventory['source_sha256'];facts.update(inventory['script_facts'])
  for row in inventory['candidates']:
   if row['submap']=='Map_30600':rows.setdefault(key(row),[]).append(row)
 assert len(rows)==69
 raw={}
 with(ROOT.parent/'tools/mapdump/ff6_objects.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   actor=json.loads(line)
   if actor['submap']=='Map_30600':raw[actor['submap'],actor['group'],actor['props'].get('entity_id'),actor['name']]=actor
 gates={137,138,139,84,85,86,89,90,91,93,94,95,*range(100,123),83,175,99}
 assert len(gates)==38
 controls={135:2,96:3,87:4,80:5}
 stages={151:'area1=1',149:'area1=2',147:'area1=3',145:'area6=1',74:'area1=4',
         43:'area1=6',40:'area1=7',41:'area1=7',35:'area1=8',36:'area1=8',32:'area1=9',29:'area1=10',21:'area1=11',22:'area1=11'}
 authored=[]
 for identity,variants in rows.items():
  row=variants[0];actor=raw[identity];entity=row['entity']
  if entity in gates:
   assert row['type']=='Entity'and row['action']==0 and actor['props']['collision']and not row['script']
   kind='physical gate counterpart';reason='Exact solid native stage/lift/path/wall counterpart, with no independent operation. Existing native initializer/control projections and actual route/collider checks retain its visibility and collision. Never opened by a generic routing exemption.'
  elif entity in controls:
   assert row['type']=='Entity'and row['action']==2 and not actor['props']['collision']and row['script']
   kind='guided mechanic';reason='Explicit normal-route control with native area'+str(controls[entity])+'=1 receipt. The full guide uses the real script and passage continuations; it never treats a nearby cell as proof of activation.'
  elif entity==157:
   assert row['type']=='NPC'and row['script']=='sc_e_0566'
   kind='guided party interaction';reason='Actual Shadow conversation is included before the normal switch/lift journey. Native recruitment receipts287/288 and object availability remain authoritative; no party mutation or hidden timer inference.'
  elif entity==155:
   assert row['script']=='sc_e_0567'
   kind='guided native choice';reason='Choose the second option to stay. The exact decline returns to Upper70,37, unlike departure-only70,43. Owned XY predicates retire the receipt-free fallback there, so the next request reaches path marker74 rather than repeating the prompt forever.'
  elif entity in(52,53):
   assert row['script']=='sc_e_0595'
   kind='guided native passage';reason='Both physical escape-entry triggers retain the exact automatic Upper75,29 passage and area1=5 receipt. Normal/escape aliases remain distinct; native controls and collision are preserved.'
  elif entity in stages:
   assert row['type']=='Event'and row['action']==2 and row['script']
   kind='guided native foot control';reason='Explicit native '+stages[entity]+' progression trigger, including real paired alternatives. Treasure10 is collected from the east before21/22 set stage11 and cut it off; a late missed-chest escape continues without an impossible reward target.'
  elif entity==31:
   assert row['script']=='sc_e_0600'and row['group']=='ev_e_0600'
   kind='hazardous alternate trigger';reason='The west-side collapse sets299 and spawns encounter18; its victory records area7=1. The default treasure route avoids this trigger, and an actual resumed route now explicitly clears18 before returning to29/chest/escape. No wall or terrain is bypassed.'
  elif entity==11:
   assert row['script']=='sc_e_0607'
   kind='guided native choice';reason='Choose the second option to wait for Shadow, then remain at the native edge. Initial wait296, arrival298 and native callback ownership are retained; no fixed timer estimate or automation. Completion waits through the exact catastrophe chain to ready191/native297.'
  else:
   assert entity in range(168,175)and row['script']=='sc_e_0608'and row['group']=='ev_e_0608'
   kind='automatic story continuation';reason='Seven native arrival/escape-event counterparts of the same Shadow continuation, not seven independent puzzle operations. The edge wait instruction preserves native timer/arrival ownership and the real airship/movie chain; no premature completion or fabricated timing.'
  result=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},
              script_sha256=facts.get(row['submap']+'/'+row['script'],{}).get('sha256'),
              disposition=kind,guides=['floating_continent'],reason=reason,
              evidence='docs/re/FLOATING_CONTINENT_GUIDANCE_20261103.md; source hashes/exact choices/timer ownership,8 actual guide/planner cases66 actions,real model/controller and older-guide regressions.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as stream:assert hashlib.file_digest(stream,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(row):row for row in authored};seen=set();new['decisions']=[]
 for row in old['decisions']:
  if row['game']=='ff6'and key(row)in replacement:new['decisions'].append(replacement[key(row)]);seen.add(key(row))
  else:new['decisions'].append(row)
 new['decisions'].extend(row for row in authored if key(row)not in seen)
 if'--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Floating Continent:69 exact original discoveries;',sum(map(len,rows.values())),'physical instances; all previous decisions retained.')
if __name__=='__main__':main()
