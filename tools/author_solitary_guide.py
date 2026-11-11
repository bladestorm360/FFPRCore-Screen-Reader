"""Complete native Cid/fishing/both-outcome/raft guide, without hidden HP output."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
CATCH='Catch this fish, then return to Cid.'
FEED='Give Cid every fish you are carrying.'
REFRESH='Talk to Cid to refresh the fish.'
RAFT='Board the raft.'

def author(o):
 proof=json.loads((ROOT/'docs/re/solitary_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 fishing=[f(305),f(525)]
 # Native feeding consumes every held fish and rerolls actual549..552 spawn
 # receipts. Only16/17 are beneficial; never invent a hidden HP announcement.
 rules=[r([*fishing,f(301,1)],t(191,7,FEED,'ev_e_0615')),
        r([*fishing,f(302,1)],t(191,7,FEED,'ev_e_0615')),
        r([*fishing,f(549,1)],t(192,16,CATCH,'ev_e_0611')),
        r([*fishing,f(550,1)],t(192,17,CATCH,'ev_e_0612')),
        r(fishing,t(191,7,REFRESH,'ev_e_0615')),
        r([f(525,1),f(307)],t(191,8,'Continue to {0}, then request guidance again.','ev_e_0627')),
        r([f(525,1),f(307,1),f(308)],t(193,1,'Continue to {0}, then request guidance again.','ev_e_0628'),t(191,8,'Continue to {0}, then request guidance again.','ev_e_0627')),
        r([f(305,1)],t(597,3,RAFT,'ev_e_0626')),
        r([f(308,1)],t(597,3,RAFT,'ev_e_0626'))]
 for row in rules:
  # The real cliff episode passes through World2. Keep this ordinary travel
  # policy rather than combining strict indoor routing with world transit.
  for goal in row['targets']:goal.update(avoid_other_events=False,allow_scripted_crossings=True,allow_world_transit=True)
 for row in rules[-2:]:row['finish_on_scenario']=306
 guide=o.guide('solitary_island',[190,191,192,193,597,2],'',rules)
 guide.update(first_match=True,select_when=[f(297,1),f(306)],complete_scenario=306,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/SOLITARY_ISLAND_GUIDE_20261104.md')
 return guide
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'));guide=author(o)
 if '--check'in sys.argv:
  assert[g for g in data['story_guides']if g['id']==guide['id']]==[guide]
  ids=[g['id']for g in data['story_guides']];assert ids.index(guide['id'])<ids.index('gerad_passage')
 else:
  prior=[g for g in data['story_guides']if g['id']!=guide['id']]
  # Gerad intentionally owns the broad297..336 World2 episode. The earlier,
  # narrower island sequence takes precedence only until its actual306 end;
  # preserve every previous guide's relative order and accepted behavior.
  at=next(i for i,g in enumerate(prior)if g['id']=='gerad_passage');prior.insert(at,guide);data['story_guides']=prior
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Solitary guide:actual beneficial fish/held inventory/reroll;both Cid outcomes/bird/hidden passage/raft/full ready306. No hidden HP or timer modification.')
