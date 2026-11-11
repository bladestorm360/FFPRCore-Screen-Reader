"""Complete infiltration objectives from native costumes, choices and receipts."""
from pathlib import Path
import json,hashlib,sys
ROOT=Path(__file__).resolve().parents[1]
MERCHANT='Use Steal against {0} to take the merchant disguise, then request guidance after battle.'
SOLDIER='Use Steal against {0} to take the soldier disguise, then request guidance after battle.'
FIRST='Choose the first option.'
SECOND='Choose the second option.'
RECOVERY=SECOND+' '+MERCHANT
def author(o):
 manifest=json.loads((ROOT/'docs/re/south_figaro_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in manifest['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 for name,row in manifest['battle_scripts'].items():
  p=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_South_Figaro/battle_scripts'/(name+'.json')
  assert hashlib.sha256(p.read_bytes()).hexdigest()==row['asset_sha256'],name
 c=o.condition;f=o.flag;r=o.rule;t=o.target;rules=[]
 opening=[f(57),f(56),f(55),f(54)]
 # The merchant's fallen pose is current-room local1. Observe the real room
 # before selecting him; leaving via its native return resets that local and
 # permits another Steal attempt if the first battle ended without a costume.
 rules.append(r([*opening,f(52),f(53),c('map',0,62,'ne')],o.entrance(62,1)))
 rules.append(r([*opening,f(52),f(53),c('map',0,62),c('local',1,1)],o.entrance(38,12)))
 rules.append(r([*opening,f(52),f(53),c('map',0,62),c('local',1)],t(62,8,MERCHANT,'ev_e_0065')))
 rules.append(r([*opening,f(52,1),f(53),c('map',0,60),c('local',1)],t(60,8,'Talk to {0}.','ev_e_0069')))
 # Soldier defeat locals belong to the loaded town, while the boy's blocking
 # pose belongs to his loaded room. Observe each through its real entrance.
 # From the town prefer a reachable green soldier; otherwise approach the boy
 # through the house's southern entrance, rather than his blocked northern side.
 rules.append(r([*opening,f(52,1),f(53),c('map',0,38)],*[t(38,i,SOLDIER,'ev_e_0065')for i in(103,105)],o.entrance(60,3)))
 rules.append(r([*opening,f(52,1),f(53),c('map',0,60,'ne'),c('map',0,38,'ne')],o.entrance(60,3)))
 rules.append(r([*opening,f(52,1),f(53),c('map',0,60),c('local',1,1)],o.entrance(38,11)))
 rules.append(r([*opening,f(53,1)],t(38,108,'Talk to {0}.','ev_e_0072')))
 # The cider battle is the same costume-producing merchant44 battle. Steal
 # his clothes as well; the boy refuses the soldier outfit on the return.
 rules.append(r([f(57),f(56),f(55),f(54,1)],t(43,7,MERCHANT,'ev_e_0073')))
 # A saved game may have won the cider battle without taking that outfit.
 # The alternative merchant accepts a soldier challenge (second option).
 # His fallen local resets only after the real room exit, just like shop62.
 recovery=[f(57),f(56),f(55,1),f(52)]
 rules.append(r([*recovery,c('map',0,41,'ne')],o.entrance(41,1)))
 assert any(x['submap']==o.MAPS[41]['map_name']and x['type_name']=='GotoMap'and x['props'].get('map_id')==42 and x['props'].get('point_id')==3 for x in o.OBJECTS)
 rules.append(r([*recovery,c('map',0,41),c('local',1,1)],o.entrance(42,3)))
 rules.append(r([*recovery,c('map',0,41),c('local',1),f(53,1)],t(41,12,RECOVERY,'ev_e_0074')))
 rules.append(r([*recovery,c('map',0,41),c('local',1),f(53)],t(41,12,MERCHANT,'ev_e_0074')))
 rules.append(r([f(57),f(56),f(55,1),f(52,1),c('map',0,60),c('local',1)],t(60,8,'Talk to {0}.','ev_e_0069')))
 rules.append(r([f(57),f(56),f(55,1),c('map',0,61,'ne')],o.entrance(61,1)))
 rules.append(r([f(57),f(56),f(55,1),c('map',0,61)],t(61,1,'Talk to {0}.','ev_e_0075')))
 rules.append(r([f(57),f(56,1)],t(60,1,SECOND,'ev_e_0076')))
 # The password's purpose is the basement passage into the northern town
 # pocket. Use its actual arrivals before approaching the mansion; a shorter
 # hypothetical town route can stop at an imperial guard instead.
 rules.append(r([f(57,1),f(58),c('map',0,38),c('player_y',0,20,'ge')],o.entrance(58,2)))
 for map_id in [60,61,*range(39,44),*range(53,57),*range(59,66)]:
  rules.append(r([f(57,1),f(58),c('map',0,map_id)],o.entrance(58,2)))
 for map_id in (58,57):rules.append(r([f(57,1),f(58),c('map',0,map_id)],o.entrance(38,7)))
 for map_id in (38,44,45):rules.append(r([f(57,1),f(58),c('map',0,map_id)],o.entrance(46,1)))
 rules.append(r([f(57,1),f(58),c('map',0,46),c('local',1)],*[t(46,i,'Continue to {0}, then request guidance again.','ev_e_0038_1')for i in(7,8,9,10)]))
 # The mansion corridor's costume-removal choice is a real blocking operation
 # before the Celes sighting, distinct from freeing Celes after that scene.
 for disguise in (52,53):rules.append(r([f(57,1),f(58),f(disguise,1)],t(48,19,FIRST,'ev_e_0077')))
 rules.append(r([f(57,1),f(58)],*[t(48,i,'Continue to {0}, then request guidance again.','ev_e_0078')for i in(1,21)]))
 rules.append(r([f(58,1),f(59)],t(50,11,FIRST,'ev_e_0079')))
 rules.append(r([f(59,1),f(60)],t(50,6,FIRST,'ev_e_0082')))
 rules.append(r([f(60,1),f(61)],t(52,1,FIRST,'entity_default')))
 rules.append(r([f(61,1),f(62)],t(55,1,'Continue to {0}, then request guidance again.','ev_e_0084')))
 rules.append(r([f(62,1),c('map',0,55)],o.entrance(38,14)))
 rules.append(r([f(62,1),c('map',0,38)],o.entrance(1,5)))
 # Native51 switches interior links to aliases626..629. The world initializer
 # selects the physical333/630 entry variant; the planner proves the active
 # native edge. Observe the world before asking for that cave continuation.
 # Warning582 enables583, then the final warning63
 # suppresses both earlier regions; a saved63 goes straight to Tunnel Armor.
 for map_id in range(38,66):rules.append(r([f(62,1),c('map',0,map_id)],o.entrance(1,5)))
 rules.append(r([f(62,1),c('map',0,1)],o.entrance(630,2),o.entrance(333,2)))
 rules.append(r([f(62,1),c('map',0,328)],o.entrance(626,1)))
 rules.append(dict(when=[f(62,1),f(63,1)],targets=[t(626,3,'Fight {0}.','ev_e_0086')],finish_on_scenario=64))
 rules.append(r([f(62,1),f(63),c('map',0,626)],t(626,2,'Continue to {0}, then request guidance again.','ev_e_0085')))
 rules.append(r([f(62,1),f(63),f(582)],t(629,7,'Continue to {0}, then request guidance again.','ev_e_0085_1')))
 rules.append(r([f(62,1),f(63),f(582,1),f(583)],t(627,21,'Continue to {0}, then request guidance again.','ev_e_0085_2')))
 rules.append(r([f(62,1),f(63),f(582,1),f(583,1)],t(626,2,'Continue to {0}, then request guidance again.','ev_e_0085')))
 for rule in rules:
  for goal in rule.get('targets',[]):
   world=goal['map']in(1,333,630) and goal.get('entity',-1)<0
   # The existing strict event-avoidance planner is indoor-only. Opt into the
   # ordinary walking-world policy only for the reviewed town-to-cave leg.
   goal.update(avoid_other_events=not world,allow_scripted_crossings=True,allow_world_transit=world)
 g=o.guide('south_figaro_infiltration',[1,*range(38,66),328,333,*range(626,631)],'',rules,[f(50,1),f(51,1),f(64),f(297)])
 g.update(first_match=True,select_when=[f(50,1),f(51,1),f(64),f(297)],complete_scenario=64,inactive='No puzzle guide for this map',source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/SOUTH_FIGARO_INFILTRATION_20261104.md')
 return g
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if'--check'in sys.argv:assert[row for row in d['story_guides']if row['id']==g['id']]==[g]
 else:
  d['story_guides']=[row for row in d['story_guides']if row['id']!=g['id']]+[g]
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('South Figaro: full costume/cider/password/Celes/key/clock/escape/cave/boss sequence; old guides preserved.')
