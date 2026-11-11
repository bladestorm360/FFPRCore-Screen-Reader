"""Original source-qualified city search,alternative sightings and full underground arrival."""
from pathlib import Path
import hashlib,json,sys
import bake_maps
from ff5_rule_resources import update_header
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
KEY='Cross this trigger, then request guidance again.'
def author():
 scripts={};objects=[];rooms={'Map_30180','Map_30181_1'}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']in rooms and s['name']!='sc_e_0162_43':scripts[s['submap'],s['name']]=s
 assert len(scripts)==9
 hashes={room+'/'+name:hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()for(room,name),s in scripts.items()}
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects.append(o)
 infos=read(D/'ff5_submaps.json');bounds={room:bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])for room in rooms}
 def target(id):
  o,=[o for o in objects if o['submap']=='Map_30180'and o['type_name']=='Event'and o['props']['entity_id']==id];ox,oy,*_=bounds[o['submap']]
  return dict(map=287,entity=id,raw=o['name'],group=o['group'],layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,kind='spot',text=KEY)
 f=lambda bit,value:['scenario',bit,'eq',value]
 rules=[dict(when=[f(60,0)],targets=[target(5)]),dict(when=[f(60,1),f(365,0)],targets=[target(6)]),dict(when=[f(60,1),f(365,1),f(368,1)],targets=[target(10)])]
 for prior in(366,367):rules.append(dict(when=[f(60,1),f(365,1),f(368,0),f(prior,1)],targets=[target(17)]))
 rules.append(dict(when=[f(60,1),f(365,1),f(368,0),f(366,0),f(367,0)],targets=[target(id)for id in(7,8,9,18,19)]))
 guide=dict(id='gohn_city_search',maps=[287,288],select_when=[['scenario',61,'ge',0]],complete_scenario=61,rules=rules,source_script_hashes=hashes,evidence='docs/re/FF5_GOHN_20261105.md')
 # The native callback's final controlled player SetPos,not its cinematic
 # starting PointIn,owns continuation routing on the underground floor.
 root=scripts['Map_30180','sc_e_0062'];arrival=scripts['Map_30181_1','sc_e_0062_1'];body=arrival['mnemonics'];assert len(body)==471 and body[76]['mnemonic']=='SetFlag'and body[76]['operands']['iValues'][0]==61 and body[457]['mnemonic']=='SetPuppet'and body[457]['operands']['iValues'][:3]==[2,-1001,466]and body[467]['mnemonic']=='SetPos'and body[467]['operands']['iValues'][:3]==[17,10,2]
 override=dict(submap='Map_30180',entity_id=10,destination_map=288,original_point=1,continuation='sc_e_0062_1',continuation_sha256=hashlib.sha256(json.dumps(arrival,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),arrival_cell=[17,10,1],trap_transfer=True,evidence='Full471-operation native callback ends controlled player17,10,native layer2/Ground. Existing exact FFV SetPos/layer qualification. docs/re/FF5_GOHN_20261105.md')
 facts=dict(scope_tag='ff5_Gohn',source_script_hashes=hashes,actors=[dict(room=o['submap'],type=o['type_name'],editor=o['id'],entity=o['props'].get('entity_id'),raw=o['name'],group=o['group'],cell=[bake_maps.layer_index(o['layer']),o['tx'],bake_maps.tile_row(o)],size=[o['w'],o['h']])for o in objects if o['type_name']in('Event','NPC','Entity','AnimEntity','GotoMap','PointIn')],position_native=read(R.parent/'tools/ff5_library_20261101/puppet_position_provenance.json'))
 return guide,override,facts
def main():
 check='--check'in sys.argv;guide,override,facts=author();p=R/'data/ff5/puzzles.json';d=read(p);old=d['story_guides'];rows=[guide if g['id']==guide['id']else g for g in old]
 if not any(g['id']==guide['id']for g in old):rows.append(guide)
 proof=R/'docs/re/ff5_gohn_native_20261105.json';arr=R/'data/ff5/warp_arrival_overrides.json';arrivals=read(arr);same=[r for r in arrivals if r['submap']=='Map_30180'];assert not same or same==[override]
 if check:assert old==rows and read(proof)==facts and same==[override]
 else:
  d['story_guides']=rows
  from author_puzzle_types import expected
  d['guidance_types']=expected(5,d);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');proof.write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
  if not same:arrivals.append(override);arr.write_text(json.dumps(arrivals,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 update_header(d,check);print('Gohn:real ordered sightings,either middle branch,final collapse and full61/17,10 Ground arrival;earlier guides preserved.')
if __name__=='__main__':main()
