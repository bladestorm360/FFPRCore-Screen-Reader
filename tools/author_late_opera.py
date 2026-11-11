"""Separate complete optional late-Opera dragon,operator and verified repeat exit."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
LEVER='Operate {0}, then win the battle.'
def author(o):
 proof=json.loads((R/'docs/re/late_opera_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert hashlib.sha256(json.dumps(o.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 done,=[r for r in proof['facts']['Map_20440/sc_e_0893_3']['flags']if r[1]=='ScenarioFlag2'];done=done[2]+256;assert done==434
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 rules=[r([c('map',0,293,'ne')],o.entrance(293,1)),r([c('local',2,0)],t(293,9,'Talk to {0}.','entity_default')),r([],t(293,3,LEVER,'entity_default'))]
 for row in rules:
  for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 rules[-1]['finish_on_scenario']=done
 g=o.guide('late_opera_dragon',list(range(290,296)),'',rules);g.update(first_match=True,select_when=[f(297,1)],complete_scenario=done,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/LATE_OPERA_DRAGON_20261104.md');return g
def passage(o):
 import bake_maps,late_opera_passage
 actor=o.physical(293,3,'entity_default');guard=late_opera_passage.guard(actor,o.SCRIPTS[actor['submap'],'sc_e_0893']);assert guard
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info[actor['submap']],[a for a in o.OBJECTS if a['submap']==actor['submap']])
 guard.update(id=3,group=actor['group'],type=actor['type'],raw=actor['name'],cell=[1,actor['tx']-ox,bake_maps.tile_row(actor)-oy],source='sc_e_0893',map_ids=[293],area_id=int(o.MAPS[293]['area_id']))
 # A synthetic arrival ID is assigned during packing. Destination-specific
 # guards must name that same stable append index,not the obsolete PointIn1.
 overrides=json.loads((R/'data/ff6/warp_arrival_overrides.json').read_text(encoding='utf-8'));index,=[i for i,r in enumerate(overrides)if r['submap']==actor['submap']and r.get('object_id')==18];guard['destination'][1]=-100000-index
 return guard
def arrival(o):
 import bake_maps
 room='Map_20440';body=o.SCRIPTS[room,'sc_e_0333_2'];rows=body['mnemonics'];assert rows[17]['mnemonic']=='SetPos'and rows[17]['operands']['iValues'][:3]==[66,39,2]
 moves=[m['operands']['iValues'][:3]for m in rows[35:44]if m['mnemonic']=='Move'];assert moves==[[2,2,0],[2,1,1]]
 # Current exact Puppet.Move2E98E0 sends operand2 to GetSpeedID,not a layer
 # setter. Ground from SetPos persists through both South moves. No guess.
 native=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Late_Opera_Context/native_move';proof=json.loads((native/'provenance.json').read_text(encoding='utf-8'));assert proof['functions']['Last.Interpreter.Instructions.Puppet.Move(PuppetCore pc)']==0x2e98e0 and proof['functions']['Last.Interpreter.Instructions.Puppet.SetPos(PuppetCore pc)']==0x2ecba0
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info[room],[a for a in o.OBJECTS if a['submap']==room])
 return dict(submap='Map_20441_3',entity_id=3,object_id=18,destination_map=290,original_point=1,continuation='sc_e_0333_2',continuation_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_cell=[66,42,1],arrival_baked_cell=[66-ox,42-oy,1],interaction_travel=True,ordinary_travel=True,evidence='Only after native434:full repeat landing SetPos66,39 Ground plus three South steps;Move operand2 is speed. No access through the first boss arena. docs/re/LATE_OPERA_DRAGON_20261104.md')
if __name__=='__main__':
 import author_ff6_story_guides as o
 check='--check'in sys.argv;g=author(o);p=R/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if check:assert[a for a in data['story_guides']if a['id']==g['id']]==[g]
 else:data['story_guides']=[a for a in data['story_guides']if a['id']!=g['id']]+[g];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=R/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));wanted=arrival(o);old=[r for r in data if r['submap']==wanted['submap']and r.get('object_id')==18]
 if check:assert old==[wanted]
 else:
  assert not old or old==[wanted]
  if not old:data.append(wanted)
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=R/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));wanted=passage(o);old=data['Map_20441_3'].get('events',[])
 if check:assert[r for r in old if r.get('source')=='sc_e_0893']==[wanted]
 else:data['Map_20441_3']['events']=[r for r in old if r.get('source')!='sc_e_0893']+[wanted];p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Late Opera:actual local2 operator/lever3/boss/full434;one strict manual repeat exit at full native66,42 Ground. Earlier performance unchanged.')
