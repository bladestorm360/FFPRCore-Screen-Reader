"""Exact Pyramid discoveries,including returning-script needle controls."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
INVENTORIES=('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json','puzzle_mechanics_continuation_inventory_20261104.json')
identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((R/'docs/re/ff5_pyramid_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff5_Pyramid'and len(proof['source_script_hashes'])==244 and len(proof['actors'])==548
 assert len(proof['native_contracts']['native_gimmicks']['functions'])==16 and len(proof['native_contracts']['native_return']['functions'])==5 and len(proof['native_arrays'])==4
 variants={};facts={};sources=None
 for name in INVENTORIES:
  d=json.loads((R/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff5'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Pyramid of Moore']:variants.setdefault(identity(row),[]).append(row)
 assert len(variants)==38
 authored={}
 for key,rows in variants.items():
  row=rows[0];locator=row['submap']+'/'+row['script'];sha=facts.get(locator,{}).get('sha256')
  if sha is not None:assert sha==proof['source_script_hashes'][locator]
  else:assert row['submap']=='Map_30461_6'and row['entity']==12 and row['action']==0 and row['script']=='sc_world_3_to_20050'
  disposition='guided mechanic';guides=['pyramid_of_moore']
  if row['submap']=='Map_30461_6':
   assert row['entity']==12 and row['action']==0;disposition='inactive copied script binding';guides=[]
   reason='Action0 dormant third-floor marker carries a copied foreign world-transfer script;the actual timed native gimmick controls actors15–18 and tile animation14. It is not a travel edge or an executable puzzle control. Existing inactive review is preserved.'
  elif row['script']in('sc_e_0440','sc_e_0444','sc_e_0443'):
   disposition='optional coffin scene';guides=[];reason='Exact optional/repeat coffin animation displaces the player or exposes an optional battle. It has no separate choice/riddle solution or required onward receipt. The main guide opens the actual passage coffin436 and clears its enemy325;optional coffin437/326 remains optional.'
  else:
   reason='Complete native-room guide covers needle pattern101 and reset returns,door activation384 and reachable382/381/383 order with live snakes,coffin436/enemy325,all three quicksand controls including actual live/reloaded391 distinction,temporary side-wall388 and northern door400 open/close. Timed eighth-floor crossings use native phase witnesses and invariant holds;the eastern wrong entrance recovers to the no-slide western route. Actual falls retain trap semantics/full landing,recover through the proper earlier floor,and only full137 plus native field readiness after the seven-map tablet movie finishes the guide. Controls remain valid destinations independently of what they unlock.'
  out=dict(game='ff5',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition=disposition,guides=guides,reason=reason,evidence='docs/re/FF5_PYRAMID_20261104.md;244 exact scripts/548 actors/21 native functions/four metadata arrays;production routes and native-shaped owner.')
  if len(rows)>1:out['instances']=[{k:r[k]for k in('type','cell','action','script')}for r in rows]
  authored[key]=out
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():
  with(R.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff5'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Pyramid:38 exact discovery identities;returning needle scripts included;inactive marker and optional coffins stay distinct.')
if __name__=='__main__':main()
