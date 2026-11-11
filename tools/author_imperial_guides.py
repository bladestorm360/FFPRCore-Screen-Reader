"""Vector's distraction/crate journey and the first-visit party prerequisite."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
PARTY='Bring Terra and at least one other character in the active party.'
def author(o):
 proof=json.loads((ROOT/'docs/re/imperial_travel_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 vector=[r([f(523),c('local',1,0)],t(143,61,'Choose the first option.','ev_e_0373')),
         r([f(523),c('local',1,1)],t(143,60,'Continue to {0}, then request guidance again.','ev_e_0374')),
         dict(when=[f(523,1)],targets=[t(143,i,'Continue through the passage.','entity_default')for i in(28,29,30)],finish_on_exit=396)]
 # These are different puzzle purposes: Vector's actual physical distraction
 # and traversal; the camp's party-setup mechanic. Native gating applies with
 # either guidance preference off, independently of solution disclosure.
 camp=[dict(when=[c('terra',0,0)],note=PARTY),dict(when=[c('members',0,2,'lt')],note=PARTY),
       r([],*[t(162,i,'Continue to {0}, then request guidance again.','ev_e_0442')for i in(18,19,20,51,52)])]
 for rules in(vector,camp):
  for row in rules:
   for target in row.get('targets',[]):target.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 first=o.guide('vector_infiltration',[143],'',vector);first.update(first_match=True,select_when=[f(194)],source_script_hashes=proof['source_script_hashes'],evidence='docs/re/IMPERIAL_TRAVEL_20261104.md')
 second=o.guide('imperial_post_party',[162],'',camp);second.update(first_match=True,select_when=[f(207,1),f(225)],complete_when=[f(217,1),c('terra',0,1),c('members',0,2,'ge')],source_script_hashes=proof['source_script_hashes'],evidence='docs/re/IMPERIAL_TRAVEL_20261104.md')
 # Native217 records the first scene, but missing Terra/solo checks precede
 # that receipt on later visits too. Completion must retain those conditions.
 camp[-1]['finish_on_scenario']=217
 return[first,second]
if __name__=='__main__':
 import author_ff6_story_guides as o
 from ordinary_interaction_fixtures import author_recipe, retired_recipes
 p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'));guides=author(o)
 old=json.loads(p.read_text(encoding='utf-8'));retired=retired_recipes(ROOT,'ff6','story_guides')
 for guide in guides:author_recipe(data,guide,retired,'story_guides')
 if '--check'in sys.argv:assert data==old
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Vector puzzle and ordinary Observation Post party gate verified. Native refusals and resets are retained.')
