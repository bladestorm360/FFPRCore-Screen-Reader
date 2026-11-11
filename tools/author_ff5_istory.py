"""Four actual flower contacts/full299;the bard remains an ordinary gift."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
def author():
 proof=json.loads((R/'docs/re/ff5_istory_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff5_Istory'and len(proof['source_script_hashes'])==28
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][k]
 objects=[]
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']=='Map_20090':objects.append(o)
 info=json.loads((D/'ff5_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info['Map_20090'],objects)
 def target(e,exit=False):
  o,=[o for o in objects if o['group']==('entity_default'if exit else'ev_e_0193')and o['props'].get('entity_id')==e and o['type_name']==('GotoMap'if exit else'Event')]
  if exit:assert o['props']['map_id']==1 and o['props']['point_id']==32
  return dict(map=118,entity=e,raw=o['name'],group=o['group'],layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,kind='transition'if exit else'spot',text='Take this exit, then request guidance again.'if exit else'Cross this trigger, then request guidance again.')
 condition=lambda bit,value:['scenario',bit,'eq',value]
 rules=[]
 for bit,e in((296,31),(297,28),(298,33)):
  rules.append(dict(when=[condition(i,1)for i in range(296,bit)]+[condition(bit,0)],targets=[target(e)]))
 rules.append(dict(when=[condition(i,1)for i in(296,297,298)],targets=[target(26)]))
 return dict(id='istory_flowers',maps=[118],transit_maps=[1],complete_scenario=299,when=[condition(299,0)],recovery=dict(anchor=target(31),target=target(11,True)),rules=rules,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/FF5_ISTORY_GUIDE_20261104.md')
if __name__=='__main__':
 wanted=author();p=R/'data/ff5/puzzles.json';d=json.loads(p.read_text(encoding='utf-8'));old=d.get('story_guides',[]);rows=[wanted if g['id']=='istory_flowers'else g for g in old]
 if not any(g['id']=='istory_flowers'for g in old):rows.append(wanted)
 d['story_guides']=rows;check='--check'in sys.argv
 if check:assert old==rows
 else:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(d,check);print('Istory four real flower contacts/all saved subsets/full299;constant map/type/context availability;one recovery instruction plus existing12-language text.')
