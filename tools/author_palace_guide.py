"""Full timed tour, native current-question advice and post-banquet reward."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
TOUR='Talk to every soldier before time runs out.'
TALK='Talk to {0}. Request guidance at each choice.'
RESUME='Continue to {0}. Request guidance at each choice.'
# Persistent talked-to receipts, including the four native battle conversations.
SOLDIERS=[(230,165,24),(231,165,25),(232,166,44),(233,166,45),(234,166,46),(235,166,47),
 (236,168,15),(237,168,14),(238,168,13),(239,168,11),(240,168,10),(241,170,1),
 (242,173,9),(243,173,7),(244,174,8),(245,178,20),(246,178,19),(247,178,18),
 (248,178,17),(249,178,15),(250,168,12),(251,169,4),(252,179,9),(253,165,26)]
def author(o):
 proof=json.loads((ROOT/'docs/re/palace_native_20261104.json').read_text(encoding='utf-8'))
 assert len(SOLDIERS)==24 and {x[0]for x in SOLDIERS}==set(range(230,254))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 rules=[r([f(227)],t(165,27,'Continue to {0}, then request guidance again.','ev_e_0469')),
        r([f(229)],t(176,15,'Talk to {0}.','ev_e_0473'))]
 unvisited=[]
 for flag,map,entity in SOLDIERS:
  row=t(map,entity,'Talk to {0}.');row['when']=[f(flag)];unvisited.append(row)
 finish=t(176,17,'Continue to {0}, then request guidance again.','ev_e_0471')
 finish['when']=[f(flag,1)for flag,_,_ in SOLDIERS];unvisited.append(finish)
 rules.append(r([f(255)],*unvisited))
 # The native break question precedes this playable field stage. Explicit
 # requests in that question are answered by the narrow Select observer.
 rules.append(r([f(257),f(256)],*[t(177,entity,TALK,'ev_e_0502')for entity in(3,4,6,7)]))
 rules.append(r([f(257),f(256,1)],t(177,13,RESUME,'ev_e_0504')))
 rules.append(r([f(257,1)],t(166,41,'Continue to {0}, then request guidance again.','ev_e_0506')))
 for rule in rules:
  for row in rule['targets']:row.update(avoid_other_events=True,allow_scripted_crossings=True)
 guide=o.guide('imperial_palace',list(range(165,180))+[584],TOUR,rules,[f(258)])
 guide.update(first_match=True,select_when=[f(226,1)],complete_scenario=258,finish_on_completion=True,
  inactive='No puzzle guide for this map',source_script_hashes=proof['source_script_hashes'],
  evidence='docs/re/IMPERIAL_PALACE_GUIDE_20261104.md')
 return guide
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if '--check'in sys.argv:assert [v for v in d['story_guides']if v['id']==g['id']]==[g]
 else:
  d['story_guides']=[v for v in d['story_guides']if v['id']!=g['id']]+[g]
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Palace:24 native tour receipts, current banquet/break choices, full257 arrival and final258 reward.')
