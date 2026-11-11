"""Complete real cave switch/rescue and both native Shadow/Relm returns."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
RESCUE='Reach {0}. Win the battle and wait for the rescue to finish.'
def author(o):
 proof=json.loads((R/'docs/re/veldt_cave_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert hashlib.sha256(json.dumps(o.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 def receipt(locator,bank):
  row,=[r for r in proof['facts'][locator]['flags']if r[1]==bank and r[3]==1]
  return row[2]+256*(int(bank[-1])-1)
 intro=receipt('Map_30761_1/sc_e_0741','ScenarioFlag2');rock=receipt('Map_30761_4/sc_e_0743','ScenarioFlag3')
 shadow=receipt('Map_20461_6/sc_e_0744_6','ScenarioFlag2');relm=receipt('Map_20461_6/sc_e_0745_6','ScenarioFlag2')
 assert(intro,rock,shadow,relm)==(371,533,372,373)
 f,r,t=o.flag,o.rule,o.target;result=[]
 for name,variant,done in(('veldt_cave_shadow',1,shadow),('veldt_cave_relm',0,relm)):
  rules=[r([f(intro)],t(488,1,'Continue to {0}, then request guidance again.','ev_e_0741')),
         r([f(rock)],t(491,21,'Operate {0}, then request guidance again.','ev_e_0743')),
         r([],t(492,4,RESCUE,'entity_default'))]
  for row in rules:
   for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
  rules[-1]['finish_on_scenario']=done
  g=o.guide(name,[488,489,490,491,492,311],'',rules)
  g.update(first_match=True,select_when=[f(297,1),f(298,variant)],complete_scenario=done,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/VELDT_CAVE_GUIDANCE_20261104.md');result.append(g)
 return result
def passage(o):
 import bake_maps,veldt_cave_passage
 actor=o.physical(492,4,'entity_default');guard=veldt_cave_passage.guard(actor,o.SCRIPTS[actor['submap'],'sc_e_0744']);assert guard
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info[actor['submap']],[a for a in o.OBJECTS if a['submap']==actor['submap']])
 guard.update(id=4,group=actor['group'],type=actor['type'],raw=actor['name'],cell=[1,actor['tx']-ox,bake_maps.tile_row(actor)-oy],source='sc_e_0744',map_ids=[492],area_id=57)
 return guard
if __name__=='__main__':
 import author_ff6_story_guides as o
 from ordinary_interaction_fixtures import author_recipe, retired_recipes
 wanted=author(o);p=R/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 retired=retired_recipes(R,'ff6','story_guides')
 for g in wanted:author_recipe(data,g,retired,'story_guides')
 p=R/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));guard=passage(o);old=data['Map_30761_5'].get('events',[])
 if '--check'in sys.argv:assert[r for r in old if r.get('source')=='sc_e_0744']==[guard]
 else:data['Map_30761_5']['events']=[r for r in old if r.get('source')!='sc_e_0744']+[guard];p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Cave on the Veldt:actual371 dog/533 rock switch/real492 rescue;native298 selects Shadow372 or Relm373,only full ready inn return completes.926 is not a completion receipt.')
