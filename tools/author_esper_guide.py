"""Whole native cave puzzle: sightings, statues, Sketch, both pit branches, Espers."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
SKETCH='Fight {0}. When Relm joins, use Sketch on the enemy.'
def author(o):
 proof=json.loads((ROOT/'docs/re/esper_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 path=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Esper_Context/battle/sc_b_0547.json'
 assert hashlib.sha256(path.read_bytes()).hexdigest()==proof['battle_scripts']['sc_b_0547']['asset_sha256']
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 # The sightings are approach cues, not gates required by the later statue or
 # fight. Do not send a player backwards for them after actual statue279.
 rules=[r([f(279),f(277)],t(422,12,'Continue to {0}, then request guidance again.','ev_e_0541')),
  r([f(279),f(278)],t(424,19,'Continue to {0}, then request guidance again.','ev_e_0542')),
  r([f(279)],t(425,10,'Continue to {0}, then request guidance again.','ev_e_0546')),
  r([f(280)],t(425,2,SKETCH,'ev_e_0547')),
  r([c('treasure',148)],t(427,4,'Open the chest.','entity_default')),
  r([c('treasure',149)],t(428,4,'Open the chest.','entity_default')),
  r([],t(430,24,'Continue to {0}, then request guidance again.','ev_e_0551'))]
 for row in rules:
  for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 guide=o.guide('esper_cave',list(range(421,431)),'',rules)
 guide.update(first_match=True,select_when=[f(276,1)],complete_scenario=281,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/ESPER_CAVE_GUIDE_20261104.md')
 return guide
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'));guide=author(o)
 if '--check'in sys.argv:assert [g for g in data['story_guides']if g['id']==guide['id']]==[guide]
 else:
  data['story_guides']=[g for g in data['story_guides']if g['id']!=guide['id']]+[guide];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Esper guide:sightings/statues/native Ultros/Sketch,both pit treasures148/149,full Esper return281;no invented prerequisites.')
