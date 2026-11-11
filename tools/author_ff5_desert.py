"""Full original desert solution;navigation and encounter choice own separate toggles."""
from pathlib import Path
import json,subprocess,sys
import bake_maps
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
RIDE='Let the current carry you, then request guidance again.'
STEP='Step South, then request guidance again.'
FIGHT='Choose the first option, then win the battle.'
RETRY='Interact, then request guidance again.'
def author():
 proof=json.loads((R/'docs/re/ff5_desert_native_20261105.json').read_text(encoding='utf-8'));policy=json.loads((R/'build_check/ff5_desert_policy.json').read_text(encoding='utf-8'));assert len(policy['rows'])==2 and all(len(r)==8192 for r in policy['rows'])
 objects=[]
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']=='Map_30170':objects.append(o)
 def actor(id):
  o,=[o for o in objects if o['props'].get('entity_id')==id and o['type_name']in('NPC','GotoMap','Event')]
  return dict(map=286,entity=id,raw=o['name'],group=o['group'],layer=bake_maps.layer_index(o['layer']),x=o['tx'],y=bake_maps.tile_row(o),kind='transition'if o['type_name']=='GotoMap'else'interact',text='Take this exit, then request guidance again.'if o['type_name']=='GotoMap'else RETRY)
 data=dict(rows=policy['rows'],north=actor(63),south=actor(64),retry=actor(61),currents=[actor(a['entity'])for a in proof['actors']if a['type']=='Event'and a['entity']not in(26,65)],source_script_hashes=proof['source_script_hashes'],evidence='docs/re/FF5_DESERT_20261105.md')
 f=lambda i,op,n:['scenario',i,op,n]
 stories=[dict(id='desert_sandworm',maps=[286],select_when=[f(58,'eq',1),f(59,'eq',0)],complete_scenario=59,rules=[],evidence=data['evidence']),dict(id='desert_crossing',maps=[286],select_when=[f(59,'ge',0)],rules=[],evidence=data['evidence'])]
 return data,stories
def main():
 with(R/'build_check/desert_policy_author_report.txt').open('wb')as out:subprocess.run([str(R/'out/checks/ff5_desert_policy_check.exe')],cwd=R,stdout=out,stderr=subprocess.STDOUT,timeout=90,check=True)
 check='--check'in sys.argv;p=R/'data/ff5/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));data,stories=author();old=d['story_guides'];wanted={g['id']:g for g in stories};rows=[wanted.get(g['id'],g)for g in old]+[g for g in stories if not any(v['id']==g['id']for v in old)]
 if check:assert d.get('desert_guide')==data and old==rows
 else:
  d.update(desert_guide=data,story_guides=rows)
  from author_puzzle_types import expected
  d['guidance_types']=expected(5,d);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(d,check);print('Desert:two typed recipes,full native-ready encounter and both current layouts;earlier guides preserved.')
if __name__=='__main__':main()
