"""Five exact native Istory identities;full receipts and ordinary bard choice."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
read=lambda p:json.loads(p.read_text(encoding='utf-8'))
key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw'])
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def travel_facts():
 maps={int(r['id']):r['map_name']for r in csv.DictReader((D/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[1],maps[118]};objects={r:[]for r in rooms}
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects[o['submap']].append(o)
 infos=read(D/'ff5_submaps.json');out=[]
 selectors=[(118,'GotoMap','entity_id',11),(1,'PointIn','object_id',32),(1,'GotoMap','map_id',118),(118,'PointIn','object_id',1)]
 for mid,typ,key,value in selectors:
  room=maps[mid];matches=[o for o in objects[room]if o['type_name']==typ and o['props'].get(key)==value]
  assert len(matches)==(2 if mid==1 and typ=='GotoMap'else 1)
  for o in sorted(matches,key=lambda r:r['id']):
   p=o['props'];ox,oy,*_=bake_maps.map_bounds(infos[room],objects[room]);layer=bake_maps.layer_index(o['layer'])
   out.append(dict(map=mid,submap=room,type=typ,editor=o['id'],entity=p['entity_id'],point=p.get('object_id'),destination=p.get('map_id'),destination_point=p.get('point_id'),source_cell=[layer,o['tx']-ox,bake_maps.tile_row(o)-oy],player_layer=1 if mid==1 else layer,source_sha256=hashlib.sha256(json.dumps(o,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
 assert out[0]['destination']==1 and out[0]['destination_point']==32 and out[0]['source_cell']==[2,51,11]
 assert out[1]['source_cell'][1:]==[36,42]and {r['entity']for r in out[2:4]}=={72,73}and all(r['destination_point']==1 for r in out[2:4])and out[4]['source_cell']==[1,32,47]
 return out
def main():
 proof=read(R/'docs/re/ff5_istory_native_20261104.json');assert proof['scope_tag']=='ff5_Istory'and len(proof['source_script_hashes'])==28 and len(proof['actors'])==34
 assert proof['recovery_travel']==travel_facts()
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json','puzzle_mechanics_continuation_inventory_20261104.json'):
  inv=read(R/'docs/re'/name)['games']['ff5'];assert sources is None or sources==inv['source_sha256'];sources=inv['source_sha256'];facts.update(inv['script_facts'])
  for r in inv['candidates']:
   if r['area_names_en']==['Istory']:rows[key(dict(game='ff5',**r))]=r
 assert len(rows)==5 and {r['entity']for r in rows.values()}=={22,26,28,31,33}
 actual={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:actual[k]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert actual==proof['source_script_hashes']
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==sources[role]
 authored={}
 for identity,r in rows.items():
  sha=facts[r['submap']+'/'+r['script']]['sha256'];assert proof['source_script_hashes'][r['submap']+'/'+r['script']]==sha
  if r['entity']==22:disposition='ordinary optional song choice';reason='This native bard offers an ordinary Yes/No song gift. Yes runs0196 and records516. There is no riddle,ordered choice sequence or flower prerequisite. The already accepted sheep approach and its exact native landing remain unchanged.'
  else:
   disposition='guided mechanic';reason='The first three real flower contacts set296/297/298 in any order and independently hide. The guide resumes every saved subset,then routes to the final26 contact only after all three receipts. Full299,not the mid-scene spell grant,finishes after native readiness. All ordinary entrances and the sheep landing are covered;the latter uses native upper exit11/world1 point32 and the actual town re-entry,without changing terrain or the accepted sheep passage.'
  authored[identity]=dict(game='ff5',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition=disposition,guides=[]if r['entity']==22 else['istory_flowers'],reason=reason,evidence='docs/re/FF5_ISTORY_GUIDE_20261104.md;28 complete scripts/34 actor facts,all subset physical journeys,current native owner/entry and source preservation.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  identity=key(row)
  if identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Istory:four real flower contacts/all saved subsets/full299;ordinary bard choice remains distinct;exact sources verified.')
if __name__=='__main__':main()
