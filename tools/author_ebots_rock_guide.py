"""Adaptive coral objectives from native receipts; no predicted random room."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
PARTY='Bring Strago in the active party before feeding the chest.'
KEEP='Choose the second option to keep the coral, then request guidance again.'
FEED='Choose the first option to feed at least 22 coral at once.'
WARP='Use the warp to try another room, then request guidance again.'

def author(o):
 manifest=json.loads((ROOT/'docs/re/ebots_rock_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in manifest['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c=o.condition;f=o.flag;r=o.rule;t=o.target
 rules=[]
 # Native local1 is scoped to the hungry-chest floor. Once fed, coral and
 # party membership no longer gate the passage or its boss objectives.
 fed=[c('map',0,540),c('local',1,1)]
 rules.append(r([*fed,f(460)],t(540,7,'Continue to {0}, then request guidance again.','ev_e_0949')))
 rules.append(dict(when=[*fed,f(460,1),f(461)],targets=[t(540,6,'Fight {0}.','ev_e_0950')],finish_on_scenario=461))
 rules.append(dict(when=[*fed,f(462,1),f(463)],targets=[t(540,5,'Fight {0}.','ev_e_0952')],finish_on_scenario=463))
 for map_id in (539,540,541):
  unfed=[c('map',0,map_id)]+([c('local',1)]if map_id==540 else [])
  rules.append(dict(when=[*unfed,f(462),c('strago',0)],note=PARTY))
  # Two disjoint native eligibility cases, rather than treating missing party
  # data as false or requiring Strago for the repeat encounter.
  for party in ([f(462,1)],[f(462),c('strago',0,1)]):
   base=[*unfed,*party]
   if map_id==539:
    rules.append(r([*base,f(461),c('area',2)],t(539,5,'Continue through the passage.','entity_default')))
    rules.append(r([*base,c('area',2,1)],*[t(539,i,WARP,'entity_default')for i in (4,5)]))
    rules.append(r([*base,f(461,1),c('area',2)],t(539,5,WARP,'entity_default')))
   elif map_id==540:
    rules.append(r([*base,c('global',7,22,'ge')],t(540,2,FEED,'ev_e_0943')))
    rules.append(r([*base,c('global',7,22,'lt'),f(458)],t(540,2,KEEP,'ev_e_0943')))
    rules.append(r([*base,c('global',7,22,'lt'),f(458,1)],t(540,4,WARP,'entity_default')))
   else:
    for entity,receipt in ((1,224),(2,223),(3,222),(4,225),(5,221)):
     rules.append(r([*base,c('global',7,22,'lt'),c('treasure',receipt)],t(541,entity,'Open the Treasure Chest.','entity_default')))
    rules.append(r(base,*[t(541,i,WARP,'entity_default')for i in range(11,20)]))
 for rule in rules:
  for goal in rule.get('targets',[]):goal.update(avoid_other_events=True,allow_scripted_crossings=False)
 g=o.guide('ebots_rock',[539,540,541],'',rules)
 g.update(complete_when=[f(461,1),f(462)],source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/EBOTS_ROCK_GUIDANCE_20261104.md')
 return g

if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if'--check'in sys.argv:assert[row for row in d['story_guides']if row['id']==g['id']]==[g]
 else:
  d['story_guides']=[row for row in d['story_guides']if row['id']!=g['id']]+[g]
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Ebot: current-room coral, native 22 threshold/Strago/choices/fed passage/first and repeat boss receipts.')
