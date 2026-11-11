"""Helmet, both timed Right branches, both real chests and drained cave exit."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
BRING='Bring Gau in the active party, then request guidance again.'
def author(o):
 proof=json.loads((ROOT/'docs/re/crescent_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 f,c,t,r=o.flag,o.condition,o.target,o.rule;rules=[]
 # An already finished journey still guides a player out of an interior cave;
 # it does not send them around the trench again from the outside platform.
 end=[f(521,1),c('treasure',112,1),c('treasure',113,1),c('map',0,379,'lt'),c('map',0,378,'ne')]
 rules.append(dict(when=[f(133),c('gau',0,0)],note=BRING))
 rules.append(r([f(133),f(129)],t(378,5,'Continue to {0}, then request guidance again.','ev_e_0242')))
 rules.append(r([f(133)],t(378,10,'Continue to {0}, then request guidance again.','ev_e_0246')))
 # Native repeat events are installed only after Narshe defense138. A player
 # who skipped a cave on the first dive must progress normally before retrying.
 rules.append(dict(when=[f(521,1),f(138)],note='Return after the battle at Narshe.'))
 rules.append(r([c('map',0,378)],o.entrance(377,1)))
 rules.append(r([c('map',0,377)],*[t(377,i,'Choose the first option.','ev_e_0270')for i in(1,2)]))
 rules.append(r([c('map',0,379),c('treasure',112)],t(379,4,'Open the chest.','entity_default')))
 rules.append(r([c('map',0,379)],t(379,0,'Continue to {0}, then request guidance again.','entity_default')))
 # This is the current area's resettable water state, not permanent74. A
 # reopened visit can require draining again even when74 was set earlier.
 rules.append(r([c('map',0,380,'ge'),c('map',0,382,'lt'),c('area',1,0)],t(381,1,'Operate {0}, then request guidance again.','ev_e_1069')))
 rules.append(r([c('map',0,380,'ge'),c('map',0,383,'lt'),c('treasure',113)],t(382,6,'Open the chest.','entity_default')))
 last=t(382,0,'Continue to {0}, then request guidance again.','entity_default')
 rules.append(dict(when=[c('map',0,380,'ge'),c('map',0,383,'lt'),c('treasure',112,1),c('treasure',113,1)],targets=[last],finish_on_exit=96,finish_through=[573,1]))
 rules.append(r([c('map',0,380,'ge'),c('map',0,383,'lt')],last))
 # Missing a Right turn can skip a treasure room. The ordinary walking/owned
 # vehicle planner still validates the return; never invent early air travel.
 back=o.entrance(378,1);back.update(allow_world_transit=True,avoid_other_events=False)
 rules.append(r([c('map',0,96)],back))
 for rule in rules:
  for goal in rule.get('targets',[]):
   goal.setdefault('avoid_other_events',True);goal.setdefault('allow_world_transit',False);goal['allow_scripted_crossings']=True
 guide=o.guide('crescent_serpent_trench',[377,378,379,380,381,382,96],'',rules)
 guide.update(first_match=True,complete_when=end,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/CRESCENT_SERPENT_GUIDE_20261104.md')
 return guide
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if '--check'in sys.argv:assert [v for v in d['story_guides']if v['id']==g['id']]==[g]
 else:
  d['story_guides']=[v for v in d['story_guides']if v['id']!=g['id']]+[g];p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Crescent/Trench:Gau12/129/helmet133,two Right branches,chests112/113,current-area drain,full ready96/521.')
