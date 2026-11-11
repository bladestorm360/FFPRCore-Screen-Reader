"""Full mountain approach and the source-qualified mandatory Blitz solution."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
FIGHT='Fight {0}. When Sabin fights alone, use Raging Fist from Blitz: Left, Right, Left, then Confirm.'
def author(o):
 proof=json.loads((R/'docs/re/mt_kolts_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff6_Mt_Kolts'
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert hashlib.sha256(json.dumps(o.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
 assert proof['facts']['Map_30360/sc_e_0054']['flags']==[[4,'ScenarioFlag1',40,1]]
 assert proof['facts']['Map_30360/sc_e_0055']['encounters']==[[23,436,22,32,'sc_e_0055_1']]
 assert proof['facts']['Map_30360/sc_e_0055_1']['flags']==[[43,'ScenarioFlag1',41,1]]
 assert proof['battle_references']['436']['script']=='sc_b_0055'
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 rules=[r([f(40)],*[t(343,i,'Continue to {0}, then request guidance again.','ev_e_0054')for i in(1,10)]),r([],t(343,2,FIGHT,'ev_e_0054'))]
 for row in rules:
  for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 rules[-1]['finish_on_scenario']=41
 g=o.guide('mt_kolts_vargas',list(range(334,344)),'',rules);g.update(first_match=True,select_when=[f(297)],complete_scenario=41,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/MT_KOLTS_VARGAS_20261104.md');return g
if __name__=='__main__':
 import author_ff6_story_guides as o
 g=author(o);p=R/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert[a for a in data['story_guides']if a['id']==g['id']]==[g]
 else:data['story_guides']=[a for a in data['story_guides']if a['id']!=g['id']]+[g];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 # Only the known native branch gets the final boss pose;all other state
 # records and every original native actor/geometry byte are retained.
 import bake_map_state as b
 source=o.SCRIPTS['Map_30360','sc_map_30360'];objects=[a for a in o.OBJECTS if a['submap']=='Map_30360'];info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));region=b.bake_maps.map_bounds(info['Map_30360'],objects)
 wanted=b.initializer_code(source,'ff6',region);p=R/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));old=data['Map_30360']['code'];assert len(old)==len(wanted)==34
 if '--check'in sys.argv:assert old==wanted
 else:
  assert old==wanted or old[20]==[10,0,2,1,-2147483648,-2147483648]and old[:20]==wanted[:20]and old[21:]==wanted[21:]
  data['Map_30360']['code']=wanted;p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Mt. Kolts:actual40 sightings,required Vargas/Blitz solution/full41;exact native Ground boss pose only after40/before41.')
