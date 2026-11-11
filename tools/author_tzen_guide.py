"""Separate complete six-minute Tzen rescue; preserve Thamasa's accepted guide."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
START='Start the rescue. Find the child and leave the house within six minutes.'
EXIT='Leave the house with the child.'
def author(o):
 proof=json.loads((ROOT/'docs/re/tzen_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert hashlib.sha256(json.dumps(o.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 exit_script,=[i for i,name in o.SCRIPT_NAMES.items()if name=='sc_e_0639_0']
 rules=[r([f(309)],*[t(201,i,'Continue to {0}, then request guidance again.','ev_e_0634')for i in(54,55)]),
        r([f(310)],t(201,52,START,'ev_e_0636')),
        r([f(311)],t(434,9,'Continue to {0}, then request guidance again.','entity_default')),
        r([],t(433,0,EXIT,'entity_default',exit_script))]
 for row in rules:
  for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 rules[-1]['finish_on_scenario']=312
 guide=o.guide('tzen_rescue',[201,433,434],'',rules)
 guide.update(first_match=True,select_when=[f(297,1)],complete_scenario=312,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/TZEN_RESCUE_20261104.md')
 return guide
if __name__=='__main__':
 import author_ff6_story_guides as o
 guide=author(o);p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert[g for g in data['story_guides']if g['id']==guide['id']]==[guide]
 else:data['story_guides']=[g for g in data['story_guides']if g['id']!=guide['id']]+[guide];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Tzen:actual309 quake/310 six-minute timer/child311/real house return/full ready312 after Sabin joining;Thamasa guide untouched.')
