"""Gerad's entire native journey, using persistent receipts and actual controls."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
WAIT='Wait for Gerad to stop, then request guidance again.'
CROSS='Face {0} and keep interacting until you cross.'

def author(o):
 proof=json.loads((ROOT/'docs/re/gerad_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 assets=json.loads((ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Gerad_Context/ff6_route_assets.json').read_text(encoding='utf-8'))
 for name,row in proof['follow_routes']['assets'].items():assert assets['assets'][name]['text_sha256']==row['sha256']
 c=o.condition;f=o.flag;r=o.rule;t=o.target;rules=[]
 # All four conversations may be taken in any order. The fourth native script
 # itself dispatches the bandit's departure and326; there is no fifth actor.
 for receipt,entity in ((322,3),(323,2),(324,1),(325,4)):
  rules.append(r([f(326),f(receipt)],t(217,entity,'Talk to {0}.','ev_e_0653')))
 rules.append(dict(when=[f(326)],note='Story guidance is waiting for current game state.'))
 rules.append(r([f(327)],o.entrance(214,3)))
 # Local1 belongs only to the currently loaded town. Native route201203 writes
 #575 on arrival, route201204 writes576. Never tell the player to catch a moving
 #actor or manufacture these receipts from proximity.
 rules.append(r([f(328),c('map',0,214,'ne')],o.entrance(214,3)))
 rules.append(dict(when=[f(328),f(575),c('local',1,1)],note=WAIT))
 rules.append(dict(when=[f(328),f(576),c('local',1,2)],note=WAIT))
 rules.append(r([f(328),f(576,1)],t(214,72,'Talk to {0}.','ev_e_0659')))
 rules.append(r([f(328),f(575,1)],*[t(214,i,'Talk to {0}.','ev_e_0659')for i in(1,72)]))
 rules.append(r([f(328)],t(214,1,'Talk to {0}.','ev_e_0659')))
 rules.append(r([f(329)],t(218,6,'Continue to {0}, then request guidance again.','ev_e_0662')))
 # A player can leave South Figaro before speaking to Gerad. At Nikeah's port
 # the repeat captain requires the second option, unlike the first boarding.
 rules.append(r([f(330),c('map',0,214,'ge'),c('map',0,219,'lt')],t(218,5,'Choose the second option.','entity_default')))
 rules.append(r([f(330)],t(222,11,'Talk to {0}.','ev_e_0665')))
 rules.append(r([f(331),c('map',0,447,'ge'),c('map',0,455,'lt')],t(452,6,'Talk to {0}.','ev_e_0666')))
 world_target=t(452,6,'Talk to {0}.','ev_e_0666');world_target['allow_world_transit']=True
 rules.append(r([f(331)],world_target))
 # Enter from the actual preceding room to start the332 turtle scene. A saved
 #game already inside448 must leave and return; arrival alone grants no flag.
 rules.append(r([f(332),c('map',0,448)],o.entrance(449,2)))
 rules.append(r([f(332)],o.entrance(448,1)))
 rules.append(r([f(333)],t(454,1,'Continue to {0}, then request guidance again.','ev_e_0670')))
 rules.append(r([f(334)],t(250,14,'Continue to {0}, then request guidance again.','ev_e_0671')))
 rules.append(r([f(335)],t(440,1,'Fight {0}.','ev_e_0672')))
 rules.append(dict(when=[f(335,1)],targets=[t(249,6,'Continue to {0}, then request guidance again.','ev_e_0673')],finish_on_scenario=336))
 for row in rules:
  for goal in row.get('targets',[]):goal.update(avoid_other_events=not goal.get('allow_world_transit',False),allow_scripted_crossings=True)
 maps=[2,*range(214,219),*range(219,252),*range(435,442),*range(447,455)]
 g=o.guide('gerad_passage',maps,'',rules,[f(297,1),f(336)])
 g.update(first_match=True,select_when=[f(297,1),f(336)],complete_scenario=336,inactive='No puzzle guide for this map',source_script_hashes=proof['source_script_hashes'],evidence='docs/re/GERAD_PASSAGE_20261104.md')
 return g

if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if '--check'in sys.argv:assert [v for v in d['story_guides']if v['id']==g['id']]==[g]
 else:
  d['story_guides']=[v for v in d['story_guides']if v['id']!=g['id']]+[g]
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gerad: pub/follow/ferry/inn/turtle/jail/engine/surfacing; native575/576 and final336.')
