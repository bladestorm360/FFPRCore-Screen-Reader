"""Complete native raft forks/landings/boss/full scenario-selection return."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
BOARD='Board the raft. Choose the first option at the fork.'
LOOP='Board the raft. At the next fork, choose the second option.'
BOSS='Board the raft, then win the battle. Keep Banon alive.'
def author(o):
 proof=json.loads((R/'docs/re/lethe_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff6_Lethe'
 for name,wanted in proof.get('mechanic_sources',{}).items():assert hashlib.sha256((R.parent/'tools/mapdump/ff6_master'/name).read_bytes()).hexdigest()==wanted
 keys=[(room,name)for room,name in o.SCRIPTS if name.startswith(('sc_e_0063','sc_e_0064','sc_e_0065','sc_e_0143'))and room in('Map_30380','Map_30381_1','Map_30390','Map_30391_1','Map_40012')];pins={}
 for key in keys:
  sha=hashlib.sha256(json.dumps(o.SCRIPTS[key]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest();assert proof['source_script_hashes']['/'.join(key)]==sha;pins['/'.join(key)]=sha
 assert len(pins)==11,pins
 s=o.SCRIPTS['Map_30390','sc_e_0064_1']['mnemonics'];assert s[19]['mnemonic']=='Branch'and s[19]['operands']['iValues'][:3]==[0,0,31]and s[51]['mnemonic']=='Jump'and s[51]['operands']['iValues'][0]==16
 assert s[29]['mnemonic']=='ChangeMap'and s[29]['operands']['iValues'][:2]==[348,1]
 final=o.SCRIPTS['Map_40012','sc_e_0065_2']['mnemonics'];assert final[17]['mnemonic']=='SetFlag'and final[17]['operands']['iValues'][0]==50
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 rules=[r([c('map',0,mid)],t(mid,e,text,'entity_default'))for mid,e,text in((345,12,BOARD),(346,1,LOOP),(348,1,BOSS))]
 for row in rules:
  row['finish_on_scenario']=50
  for goal in row['targets']:goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 g=o.guide('lethe_river',[345,346,348],'',rules);g.update(first_match=True,select_when=[f(47,1),f(297)],complete_scenario=50,finish_on_completion=True,source_script_hashes=pins,evidence='docs/re/LETHE_RIVER_20261104.md');return g
if __name__=='__main__':
 import author_ff6_story_guides as o
 g=author(o);p=R/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert[a for a in data['story_guides']if a['id']==g['id']]==[g]
 else:data['story_guides']=[a for a in data['story_guides']if a['id']!=g['id']]+[g];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Lethe:three actual raft controls,first/Straight and second/Left fork answers,full50 scenario-selection completion;no bake/native owner hook change.')
