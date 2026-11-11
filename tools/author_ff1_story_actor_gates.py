"""Exact unremovable story actors; factual identities and source hashes only.

Native programs remain in the external extraction. Giant11 loads before18;
its own script refuses to open the passage without Ruby receipt17. A future
NPC approach retry cannot omit that story prerequisite.
"""
from pathlib import Path
import hashlib,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
def main():
 room='Map_30041_1';objects=[];programs={}
 with (DUMP/'ff1_objects.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   o=json.loads(line)
   if o['submap']==room:objects.append(o)
 wanted={(room,'sc_map_30041_1'),(room,'sc_e_0018'),('Map_30031_3','sc_e_0017')}
 with (DUMP/'ff1_scripts.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   row=json.loads(line);key=row['submap'],row['name']
   if key in wanted:programs[key]=row['mnemonics']
 assert programs.keys()==wanted
 actors={o['props']['entity_id']:o for o in objects if o['type_name']=='NPC'}
 assert set(actors)=={11,13}
 early=actors[13];ready=actors[11]
 assert early['group']=='ev_e_0016' and ready['group']=='ev_e_0018'
 assert early['props']['action_id']==4 and early['props']['script_id']==0
 assert ready['props']['action_id']==2 and ready['props']['script_id']==36
 assert early['name']==ready['name']=='巨人'
 assert (early['tx'],bake_maps.tile_row(early),bake_maps.layer_index(early['layer']))==(15,12,2)
 def digest(body):return hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert digest(programs[room,'sc_e_0018'])=='e3818893bc7fb1ba0f31e9e2b99830b1a72919eb52d2ac79e3f3e111b8292aba'
 assert digest(programs['Map_30031_3','sc_e_0017'])=='9a4d914131ada5b97b9b1cb7284650ffb994acb3b9203366f3a1f50f8325c4e6'
 assert digest(programs[room,'sc_map_30041_1'])=='1594da2de60c93b587b8bc66297f84d291e004add72f1eeae3a9fc2a917ddfc9'
 branch=programs[room,'sc_e_0018'][1]
 assert branch['mnemonic']=='Branch' and branch['operands']['iValues'][:2]==[17,1] and branch['operands']['sValues'][0]=='ScenarioFlag1'
 gate={'type':6,'id':11,'group':ready['group'],'raw':ready['name'],'cell':[2,15,12],
       'initializer_sha256':digest(programs[room,'sc_map_30041_1']),
       'prerequisite_receipt':17,'opening_receipt':18}
 path=ROOT/'data/ff1/mapstate.json';original=path.read_text(encoding='utf-8');data=json.loads(original)
 data[room]['story_actor_gates']=[gate]
 rendered=json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n'
 if '--check' in sys.argv:assert original==rendered,'FF1 story-actor gates differ from reviewed authoring'
 else:path.write_text(rendered,encoding='utf-8')
 (ROOT/'docs/re/ff1_story_actor_gates_20261101.json').write_text(json.dumps({'map':81,'gate':gate,'ready_variant':11,'receipt_source':'FF1_SLAB_GUIDES_20261101.md'},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FF1 Giant11: native branch17 remains a story gate without Ruby; owned Ruby permits manual opening18.')
if __name__=='__main__':main()
