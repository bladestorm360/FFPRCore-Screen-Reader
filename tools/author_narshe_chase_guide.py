"""Original beginning-to-end chase instructions on exact native receipts."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
ROOM_WAIT='Wait for the treasure-room scene to finish, then use Shift+G.'
LEDGE_START='Reach {0}, then stay still until the ledge scene finishes.'
LEDGE_WAIT='Stay still until the ledge scene finishes, then use Shift+G.'
RESCUE='Talk to {0} to rescue Mog.'
def author(o):
 manifest=json.loads((ROOT/'docs/re/narshe_chase_native_20261103.json').read_text(encoding='utf-8'))
 for locator,wanted in manifest['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c=o.condition;f=o.flag;r=o.rule;t=o.target
 rules=[dict(when=[c('map',0,16),f(208)],note=ROOM_WAIT),r([f(208)],o.entrance(16,1)),
        r([f(209)],t(16,19,'Open the Treasure Chest.','ev_e_0432')),
        r([f(210)],t(3,124,'Continue to {0}, then request guidance again.','ev_e_0433')),
        r([f(211)],*[t(3,i,'Continue to {0}, then request guidance again.','ev_e_0434')for i in(120,121,122)]),
        r([f(212),c('map',0,18,'ne')],o.entrance(18,1)),
        r([f(212)],*[t(18,i,'Continue to {0}, then request guidance again.','ev_e_0435')for i in(18,19,20)]),
        r([f(213)],t(20,35,'Continue to {0}, then request guidance again.','ev_e_0436')),
        dict(when=[c('map',0,20),f(214),c('local',1,1)],note=LEDGE_WAIT),
        r([f(214)],*[t(20,i,LEDGE_START,'ev_e_0437')for i in(29,30,31)]),
        r([f(214,1)],t(20,28,RESCUE,'ev_e_0438'))]
 g=o.guide('narshe_chase',[3,16,18,19,20,323,324,325,326,327],'',rules,[f(138,1),f(207,1),f(297),f(216)])
 g.update(select_when=[f(207,1),f(297)],complete_scenario=215,inactive='No puzzle guide for this map',source_script_hashes=manifest['source_script_hashes'],evidence='docs/re/NARSHE_CHASE_GUIDANCE_20261103.md')
 for rule in rules:
  for goal in rule.get('targets',[]):goal.update(avoid_other_events=True,allow_scripted_crossings=True)
 return g
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if'--check'in sys.argv:assert[row for row in d['story_guides']if row['id']==g['id']]==[g]
 else:
  d['story_guides']=[row for row in d['story_guides']if row['id']!=g['id']]
  index=next(i for i,row in enumerate(d['story_guides'])if row['id']=='narshe_battle');d['story_guides'].insert(index,g)
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Narshe chase: treasure-room entry/empty chest/all chase stages/motionless native wait/Mog rescue; old guides preserved.')
