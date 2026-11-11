"""Original complete cave objectives and verified cardinal bridge recipes."""
from pathlib import Path
import json,hashlib,sys
ROOT=Path(__file__).resolve().parents[1]
CHOICE='Operate {0}. Choose the first option, then request guidance again.'
def author(o):
 m=json.loads((ROOT/'docs/re/sealed_gate_native_20261103.json').read_text(encoding='utf-8'))
 for locator,wanted in m['script_sha256'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c=o.condition;f=o.flag;r=o.rule;t=o.target;rules=[]
 for mid in(405,406):rules.append(r([c('map',0,mid)],o.entrance(407,1)))
 # Local4 is the forced bridge fall. Persistent door/stair receipts prevent
 # sending a party entering by the right-hand return back through that fall.
 rules.append(r([c('map',0,408),c('local',4),f(218)],t(408,69,'Operate {0}, then request guidance again.')))
 rules.append(r([c('map',0,408),c('local',4),f(218,1),f(219)],t(408,69,'Operate {0}, then request guidance again.')))
 for flag,entity in((218,65),(219,60)):
  rules.append(r([c('map',0,408),f(flag)],t(408,entity,'Operate {0}, then request guidance again.')))
 for flag,entity,group in((220,58,'ev_e_0450'),(221,57,'ev_e_0451'),(222,56,'ev_e_0452'),(223,55,'ev_e_0453')):
  rules.append(r([c('map',0,408),f(219,1),f(flag)],t(408,entity,'Examine {0}, then request guidance again.',group)))
 rules.append(r([c('map',0,408),f(224)],t(408,40,'Step on {0} to open the passage, then request guidance again.','ev_e_0456',spot=True)))
 # The first bridge must appear before the toggle approach can be reached.
 rules.append(r([c('map',0,408),c('local',5)],t(408,48,'Step on {0} to open the passage, then request guidance again.',spot=True)))
 # Native option0 restores bridge3=2 from closed3=1. Default0 is already
 # open; pressing a default-open switch would close it and is not an objective.
 rules.append(r([c('map',0,408),c('treasure',140),c('local',3,1),c('local',6)],t(408,34,CHOICE)))
 # Once the second bridge collapses, its toggle approach is cut off. The
 # real side-room round trip clears map locals and restores the default bridge.
 rules.append(r([c('map',0,408),c('treasure',140),c('local',3,1),c('local',6,1),f(224,1)],o.entrance(410,1)))
 rules.append(r([c('map',0,408),c('local',6),c('local',3,1,'ne')],t(408,46,'Step on {0} to open the passage, then request guidance again.',spot=True)))
 rules.append(r([c('map',0,408),c('treasure',140),c('local',1)],t(408,38,'Operate {0}, then request guidance again.')))
 rules.append(r([c('map',0,408),c('treasure',140),c('local',1,1),c('local',3,1,'ne')],t(408,17,'Open the Treasure Chest.')))
 # A previously changed right island is native-valid; the real planner still
 # owns which approach is available. Do not toggle it merely to match a recipe.
 rules.append(r([c('map',0,408),c('treasure',140),c('local',1,1),c('local',2,1)],t(408,36,'Operate {0}, then request guidance again.')))
 rules.append(r([c('map',0,408),f(225)],o.entrance(411,1)))
 rules.append(r([c('map',0,409)],o.entrance(408,3)))
 rules.append(r([c('map',0,410)],o.entrance(408,4)))
 receipts=[f(i,1)for i in(218,219,220,221,222,223,224)]+[c('treasure',140,1)]
 final=r([c('map',0,411),f(225),*receipts],t(411,5,'Continue through the passage.','entity_default',spot=True));final['finish_on_scenario']=225;rules.append(final)
 prefix=[]
 for receipt in receipts:
  missing=[receipt[0],receipt[1],receipt[2],0]
  rules.append(r([c('map',0,411),f(225),*prefix,missing],t(411,5,'Continue through the passage.','entity_default',spot=True)))
  prefix.append(receipt)
 rules.append(r([c('map',0,411),f(225,1)],o.entrance(408,2)))
 g=o.guide('sealed_gate',list(range(405,412)),'',rules)
 g.update(complete_when=[f(225,1),*receipts],bridges=m['bridge_paths'],source_script_hashes=m['script_sha256'],evidence='docs/re/SEALED_GATE_GUIDANCE_20261103.md')
 for rule in rules:
  for target in rule.get('targets',[]):target.update(avoid_other_events=True,allow_scripted_crossings=True)
 return g
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));g=author(o)
 if'--check'in sys.argv:assert[g0 for g0 in d['story_guides']if g0['id']==g['id']]==[g]
 else:d['story_guides']=[g0 for g0 in d['story_guides']if g0['id']!=g['id']]+[g];p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Sealed Gate: seven floors/eight timed cardinal crossings/door/stairs/floor caches/bridges/islands/chest/story225.')
