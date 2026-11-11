"""Original complete letter and hidden-house guides, native receipts only."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
SHOP='Talk to {0}, then close the shop menu.'
RETURN='Leave the village, then return and request guidance again.'
def author(o):
 proof=json.loads((ROOT/'docs/re/mobliz_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');body=o.SCRIPTS[room,name]['mnemonics']
  assert hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 f,c,t,r=o.flag,o.condition,o.target,o.rule
 rules=[r([f(502)],t(90,9,'Talk to {0}.','entity_default'))]
 for read,paid,reply,letter in ((503,504,505,8),(506,507,508,7),(509,510,511,6),(512,513,514,5),(515,516,None,4)):
  rules.append(r([f(read)],t(90,letter,'Read {0}.')))
  rules.append(r([f(paid)],t(88,10,'Choose the first option.','entity_default')))
  if reply is not None:
   # Any real shop-close callback supplies this one next-letter receipt. A
   # menu visit is sufficient; neither a purchase nor elapsed time is required.
   rules.append(r([f(reply)],*[t(map,entity,SHOP,'entity_default')for map,entity in((86,25),(86,26),(87,14),(91,16))]))
 rules.append(r([f(517)],t(90,9,'Talk to {0}.','entity_default')))
 letters=o.guide('mobliz_letters',list(range(84,92)),'',rules,[f(517)])
 letters.update(first_match=True,complete_scenario=517,finish_on_completion=True)
 rules=[r([c('map',0,2)],o.entrance(207,1)),
  r([f(316)],*[t(207,i,'Continue to {0}, then request guidance again.','ev_e_0644')for i in(57,58)]),
  r([f(317)],t(212,46,'Continue to {0}, then request guidance again.','ev_e_0648')),
  r([f(318)],t(212,66,'Talk to {0}.','ev_e_0649')),
  r([f(319)],t(212,44,'Continue to {0}, then request guidance again.','ev_e_0650')),
  r([f(320)],t(212,43,'Continue to {0}, then request guidance again.','ev_e_0651'))]
 # The gift sets area-local1, so this first visit must really leave the area.
 # The world-map return then observes the actual reset, never manufactures it.
 exit=t(207,6,RETURN,'entity_default');exit['kind']='transition'
 rules.append(r([c('map',0,207,'ge'),c('map',0,214,'lt'),c('area',1,1)],exit))
 rules.append(r([f(354)],*[t(210,i,'Continue to {0}, then request guidance again.','ev_e_0706')for i in(7,8,9)]))
 rules.append(r([f(355)],t(213,1,'Continue to {0}, then request guidance again.','ev_e_0707')))
 rules.append(r([f(356)],t(213,9,'Continue to {0}, then request guidance again.','ev_e_0709')))
 hidden=o.guide('mobliz_hidden_passage',[2,*range(207,214)],'',rules,[f(356)])
 hidden.update(first_match=True,select_when=[f(353,1)],complete_scenario=356,finish_on_completion=True,inactive='No puzzle guide for this map')
 for guide in(letters,hidden):
  guide.update(source_script_hashes=proof['source_script_hashes'],evidence='docs/re/MOBLIZ_PUZZLES_20261104.md')
  for row in guide['rules']:
   for goal in row['targets']:
    world=guide['id']=='mobliz_hidden_passage'and row['when']==[c('map',0,2)]
    # The accepted strict event-avoidance graph is indoor-only. Use the same
    # explicit world/indoor pairing as the other qualified fetch/return guides.
    goal.update(avoid_other_events=not world,allow_scripted_crossings=True,allow_world_transit=world)
 return [letters,hidden]
if __name__=='__main__':
 import author_ff6_story_guides as o
 p=ROOT/'data/ff6/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));guides=author(o);ids={g['id']for g in guides}
 if '--check'in sys.argv:assert [g for g in d['story_guides']if g['id']in ids]==guides
 else:
  d['story_guides']=[g for g in d['story_guides']if g['id']not in ids]+guides
  p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Mobliz:five letters/payments/shop-close replies/final517;native353 gate,first-visit receipts,real area reset,dog354/discovery355/full356.')
