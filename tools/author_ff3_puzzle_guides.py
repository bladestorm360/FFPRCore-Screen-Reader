"""Author owned FFIII puzzle targets from exact native identities/receipts.

Only coordinates, identifiers, original instructions and factual hashes enter
resources. Raw native implementations remain in the external mapdump archive.
"""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
from ordinary_interaction_fixtures import author_recipe, retired_recipes
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'

CAST='Cast {0} on every party member, then repeat the guide.'
NORMAL='Cast {0} on every party member again to return to normal, then repeat the guide.'
VOICE='Follow the passage and listen to the voice.'
def target(action,step,map_id,entity,script,flag,text,arg='',receipt_script=''):
 return dict(id=action,step=step,map=map_id,entity=entity,script=script,flag=flag,text=text,arg=arg,receipt_script=receipt_script or script)
def manual(action,step,text,arg):return dict(id=action,step=step,manual_only=True,text=text,arg=arg)
FORM_GUIDES=[
 ('nepto_forms',list(range(169,174)),[
  manual('cast_mini',1,CAST,'modtext:FF3 spell Mini'),
  target('mini_opening',2,169,8,'sc_e_0032',31,'Examine the small opening.'),
  target('nepto_guardian',3,173,6,'sc_e_0033',33,'Defeat the guardian.',receipt_script='sc_e_0033_1'),
  target('nepto_eye_pickup',3,173,5,'sc_e_0033_1',33,'Collect the item.'),
  target('return_eye',4,169,7,'sc_e_0035',34,'Use {0} on the statue.','modtext:FF3 item Nepto Eye')]),
 ('owen_forms',list(range(174,185)),[
  manual('cast_frog',1,CAST,'modtext:FF3 spell Toad'),
  target('frog_waterway',2,174,10,'sc_e_0040',39,'Examine the waterway.'),
  manual('remove_frog',3,NORMAL,'modtext:FF3 spell Toad'),
  target('voice_first',4,175,7,'sc_e_0041',40,VOICE),
  target('voice_fourth',5,178,9,'sc_e_0042',41,VOICE),
  target('voice_fifth',6,179,10,'sc_e_0043',42,VOICE),
  target('owen_switch',7,179,7,'sc_sw_30071_6',225,'Examine the hidden switch'),
  target('owen_guardian',8,184,4,'sc_e_0044',43,'Defeat the guardian.',receipt_script='sc_e_0044_1'),
  target('owen_engine',9,184,9,'sc_e_0045',44,'Examine the mechanism and wait for the escape.')]),
 ('tozus_forms',list(range(45,51)),[
  manual('cast_mini',1,CAST,'modtext:FF3 spell Mini'),
  target('patient',2,46,25,'sc_e_0030',29,'Use {0} to heal the patient.','modtext:FF3 item Antidote')]),
 ('hein_forms',list(range(194,206)),[
  target('hein_soldier',1,194,9,'sc_e_0056',55,'Speak to the soldier.'),
  manual('cast_mini',2,CAST,'modtext:FF3 spell Mini'),
  target('hein_hole',3,194,18,'sc_e_0057',56,'Examine the small opening.'),
  manual('remove_mini',4,NORMAL,'modtext:FF3 spell Mini')]),
 ('lake_forms',list(range(84,91))+list(range(185,189)),[
  dict(target('lake_approach',1,90,5,'sc_e_0047',46,'Go to the waterway.'),approach_only=True),
  manual('cast_frog',2,CAST,'modtext:FF3 spell Toad'),
  target('lake_waterway',3,90,5,'sc_e_0047',46,'Examine the waterway.'),
  manual('remove_frog',4,NORMAL,'modtext:FF3 spell Toad'),
  target('lake_guardian',5,188,9,'sc_e_0048',48,'Defeat the guardian.',receipt_script='sc_e_0048_1'),
  target('dwarf_talk',6,85,21,'sc_e_0050',49,'Speak to the dwarf.'),
  target('dwarf_altar',7,85,27,'sc_e_0051',50,'Use {0} at the altar.','modtext:FF3 item Horn of Ice')]),
 ('magic_circle_forms',list(range(131,134))+list(range(238,242)),[
  target('doga_intro',1,131,10,'sc_e_0078',77,VOICE),
  target('doga_switch',2,133,11,'sc_sw_20261_3',198,'Examine the hidden switch'),
  manual('cast_mini',3,CAST,'modtext:FF3 spell Mini'),
  target('magic_hole',4,133,18,'sc_e_0317',195,'Examine the small opening.'),
  target('magic_upgrade',5,241,3,'sc_e_0079',78,'Approach the magic circle and wait for the scene to finish.')])]
STATIC_GUIDES=[
 ('sealed_cave_hidden_door',[162],target('control',1,162,7,'sc_e_0295',224,'Examine the hidden passage.')),
 ('eureka_hidden_passage',[324],target('control',1,324,15,'sc_sw_30271_10',226,'Examine the hidden passage.')),
 ('syrcus_key_door',[305],target('control',1,305,29,'sc_e_0090',94,'Use {0} on the locked door.','modtext:FF3 item Syrcus Key')),
 ('eureka_key_mirror',[306],target('control',1,306,7,'sc_e_0258',178,'Use {0} at the mirror, then wait for the scene to finish.','modtext:FF3 item Eureka Key'))]

def main():
 maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff3_master/map.txt').open(encoding='utf-8-sig'))}
 names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff3_master/script.txt').open(encoding='utf-8-sig'))}
 specs=[s for _,_,rows in FORM_GUIDES for s in rows if not s.get('manual_only')]+[s for _,_,s in STATIC_GUIDES]
 rooms={maps[s['map']]['map_name'] for s in specs};objects=[]
 for line in (DUMP/'ff3_objects.jsonl').open(encoding='utf-8'):
  row=json.loads(line)
  if row['submap'] in rooms:objects.append(row)
 infos=json.loads((DUMP/'ff3_submaps.json').read_text(encoding='utf-8'));scripts={}
 wanted={(maps[s['map']]['map_name'],s['receipt_script']) for s in specs}
 for line in (DUMP/'ff3_scripts.jsonl').open(encoding='utf-8'):
  row=json.loads(line);key=row['submap'],row['name']
  if key in wanted:scripts[key]=row['mnemonics']
 assert wanted==scripts.keys();facts=[]
 def author(spec):
  if spec.get('manual_only'):return dict(spec)
  room=maps[spec['map']]['map_name']
  actors=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==spec['entity'] and names.get(o['props'].get('script_id'))==spec['script']]
  assert len(actors)==1,(spec,len(actors));actor=actors[0]
  ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
  result=dict(id=spec['id'],step=spec['step'],map=spec['map'],entity=spec['entity'],raw=actor['name'],
   layer=bake_maps.layer_index(actor['layer']),x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,
   examine=actor['type_name'] not in ('Event','RandomEvent','SwitchEvent'),text=spec['text'])
  if spec['arg']:result['arg']=spec['arg']
  body=scripts[room,spec['receipt_script']]
  assert any(m['mnemonic']=='SetFlag' and m['operands'].get('sValues',[None])[0]=='ScenarioFlag1' and m['operands']['iValues'][0]==spec['flag'] for m in body)
  if spec.get('approach_only'):result['approach_only']=True
  else:result['done_scenario']=spec['flag']
  facts.append(dict(map=spec['map'],entity=spec['entity'],script=room+'/'+spec['script'],receipt_script=room+'/'+spec['receipt_script'],receipt=spec['flag'],
    sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
  return result
 path=ROOT/'data/ff3/puzzles.json';old=path.read_text(encoding='utf-8');data=json.loads(old)
 data['form_guides']=[dict(id=name,maps=active_maps,actions=[author(s) for s in rows]) for name,active_maps,rows in FORM_GUIDES]
 retired=retired_recipes(ROOT,'ff3')
 for name,active_maps,spec in STATIC_GUIDES:
  step=author(spec);step.pop('id');step.pop('step')
  definition=dict(id=name,maps=active_maps,done_scenario=spec['flag'],steps=[step],
   _about='One exact native control/item-selection operation; native receipt, not proximity, completes it.')
  if name=='eureka_key_mirror':definition['requires_scenario']=93
  author_recipe(data,definition,retired)
 rendered=json.dumps(data,ensure_ascii=False,indent=2)+'\n'
 if '--check' in sys.argv:assert old==rendered,'FFIII puzzle targets differ from native authoring'
 else:path.write_text(rendered,encoding='utf-8')
 (ROOT/'build_check/ff3_puzzle_receipts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFIII:',len(FORM_GUIDES),'form families/',sum(len(r) for _,_,r in FORM_GUIDES),'actions;',len(STATIC_GUIDES),'static controls;',len(facts),'native receipts.')
if __name__=='__main__':main()
