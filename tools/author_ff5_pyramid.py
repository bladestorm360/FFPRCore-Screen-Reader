"""Original Pyramid guide identities/receipts,not native implementations."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
SWITCH='Operate the switch.'
TRIGGER='Cross this trigger, then request guidance again.'
GO='Go now. Continue to the safe floor.'
WAIT='Wait for the floor to change.'
TABLET='Touch the tablet and wait for the scenes to finish.'
def author():
 proof=json.loads((R/'docs/re/ff5_pyramid_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff5_Pyramid'
 maps={int(r['id']):r for r in csv.DictReader((D/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[i]['map_name']for i in range(410,427)};objects=[]
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects.append(o)
 infos=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'));bounds={room:bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])for room in rooms}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap']+'/'+s['name']
  if key in proof['source_script_hashes']:assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][key]
 def target(mid,e,text,group='entity_default'):
  room=maps[mid]['map_name'];o,=[a for a in objects if a['submap']==room and a['props'].get('entity_id')==e and a['group']==group and a['type_name']in('Entity','NPC','Event','GotoMap')];ox,oy,*_=bounds[room]
  return dict(map=mid,entity=e,raw=o['name'],group=group,x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,layer=bake_maps.layer_index(o['layer']),kind='interact'if o['type_name']in('Entity','NPC')else'spot',text=text)
 def entry(mid,point=1):
  room=maps[mid]['map_name'];o,=[a for a in objects if a['submap']==room and a['type_name']=='PointIn'and a['props'].get('object_id')==point];ox,oy,*_=bounds[room]
  return dict(map=mid,x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,layer=bake_maps.layer_index(o['layer']),kind='spot',text='Continue through the passage.')
 def spot(x,y):return dict(map=424,x=x,y=y,layer=1,kind='spot',text=GO)
 targets=dict(entrance=target(410,17,'Defeat the guardians.','ev_e_0136'),first_floor=entry(411),needle_left=target(411,15,SWITCH),needle_middle_off=target(411,34,SWITCH),needle_right=target(411,17,SWITCH),snake_room=entry(415),snake_start=target(415,1,TRIGGER),snake_left=target(415,119,SWITCH),snake_middle=target(415,117,SWITCH),snake_right=target(415,120,SWITCH),snake_enemy_a=target(415,113,'Defeat the enemy.'),snake_enemy_b=target(415,114,'Defeat the enemy.'),third_floor=entry(416),fourth_floor=target(416,4,TRIGGER),coffin=target(417,23,TRIGGER,'ev_e_0300'),coffin_enemy=target(417,129,'Defeat the enemy.','ev_e_0300'),fifth_floor=entry(419),sixth_floor=entry(421),sand_first=target(421,10,SWITCH,'ev_e_0520'),sand_second=target(421,11,SWITCH,'ev_e_0520'),sand_third=target(421,12,SWITCH,'ev_e_0520'),seventh_floor=target(421,5,TRIGGER,'ev_e_0521'),wall_passage=target(422,49,SWITCH),eighth_floor=entry(424),safe_hold_left=spot(11,13),safe_hold_right=spot(13,13),safe_north_left=spot(10,10),safe_north_right=spot(14,10),upper_exit_left=entry(425),upper_exit_right=entry(425,2),tablet=target(426,3,TABLET,'ev_e_0137'))
 targets.pop('upper_exit_right')
 targets.update(return_basement=entry(411,2),return_middle_a=entry(411,3),return_coffin=entry(411,5),fifth_door=target(419,31,TRIGGER),fifth_door_close=target(419,32,TRIGGER),return_eighth_right=entry(424,3),return_sixth_east=entry(421,2),quicksand_stop=target(411,18,SWITCH,'ev_e_0289'),coffin_exit=target(411,9,TRIGGER))
 targets.update(corridor_start=target(413,1,TRIGGER,'ev_e_0293'),corridor_enemy_a=target(413,117,'Defeat the enemy.','ev_e_0293'),corridor_enemy_b=target(413,118,'Defeat the enemy.','ev_e_0293'))
 return dict(id='pyramid_of_moore',maps=list(range(410,427)),done_scenario=137,targets=targets,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/FF5_PYRAMID_20261104.md')
if __name__=='__main__':
 wanted=author();p=R/'data/ff5/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get('pyramid_guide')==wanted
 else:data['pyramid_guide']=wanted;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Pyramid:42 exact controls/arrivals/safe holds;full137 completion metadata;native script identities pinned.')
