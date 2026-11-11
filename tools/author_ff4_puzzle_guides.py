"""Original FFIV operation recipes, pinned to exact external native owners."""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
from ordinary_interaction_fixtures import author_recipe, retired_recipes
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
def control(mid,entity,script,flag,text,arg='',**extra):
 return dict(map=mid,entity=entity,script=script,flag=flag,text=text,arg=arg,**extra)
RECIPES=[
 ('baron_key_doors',[24],[
  control(24,66,'sc_e_0156',271,'Use {0} on the locked door.','modtext:FF4 item Baron Key'),
  control(24,67,'sc_e_0157',272,'Use {0} on the locked door.','modtext:FF4 item Baron Key')]),
 ('magnetic_cave',[100]+list(range(192,202)),[
  control(100,22,'sc_e_0035',34,'Obtain {0} from your ally.','modtext:FF4 item Twin Harp',allow_world_transit=True),
  control(201,5,'sc_e_0081',97,'Remove metal equipment before exploring. Fight the first encounter, then wait for the harp scene.',receipt_script='sc_e_0081_2',allow_world_transit=True),
  control(201,4,'sc_e_0037',36,'Equip metal equipment again, then defeat the guardian.',receipt_script='sc_e_0037_1'),
  control(201,3,'sc_e_0082',98,'Collect the item.')]),
 ('agart_magma_well',[109],[
  control(109,3,'sc_e_0045',44,'Use {0} at the well, then wait for the scene to finish.','modtext:FF4 item Magma Rock',receipt_script='sc_e_0045_2')]),
 ('babel_key_door',[220],[
  control(220,28,'sc_e_0158',273,'Use {0} on the locked door.','modtext:FF4 item Dr Lugae Key')]),
 ('dwarven_castle_passage',[125],[
  control(125,17,'sc_s_20151_2',278,'Speak to the soldier.')]),
 ('sealed_cavern_door',[268],[
  control(268,6,'sc_e_0062',58,'Use {0} on the locked door.','modtext:FF4 item Luca Necklace')])]
REQUIRES={'baron_key_doors':31,'agart_magma_well':43,'babel_key_door':47,'sealed_cavern_door':57}
def main():
 maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff4_master/map.txt').open(encoding='utf-8-sig'))}
 names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff4_master/script.txt').open(encoding='utf-8-sig'))}
 rooms={maps[s['map']]['map_name'] for _,_,steps in RECIPES for s in steps};objects=[]
 for line in (DUMP/'ff4_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap'] in rooms:objects.append(r)
 infos=json.loads((DUMP/'ff4_submaps.json').read_text(encoding='utf-8'))
 wanted={(maps[s['map']]['map_name'],s.get('receipt_script',s['script'])) for _,_,steps in RECIPES for s in steps};scripts={}
 for line in (DUMP/'ff4_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line);key=r['submap'],r['name']
  if key in wanted:scripts[key]=r['mnemonics']
 assert scripts.keys()==wanted
 path=ROOT/'data/ff4/puzzles.json';old=path.read_text(encoding='utf-8');data=json.loads(old);facts=[]
 retired=retired_recipes(ROOT,'ff4')
 for name,active_maps,specs in RECIPES:
  steps=[]
  for s in specs:
   room=maps[s['map']]['map_name'];actors=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==s['entity'] and names.get(o['props'].get('script_id'))==s['script']]
   assert len(actors)==1,(name,s,len(actors));actor=actors[0]
   ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
   step=dict(map=s['map'],entity=s['entity'],raw=actor['name'],layer=bake_maps.layer_index(actor['layer']),x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,
    examine=actor['type_name'] not in ('Event','RandomEvent','SwitchEvent'),text=s['text'],done_scenario=s['flag'])
   for key in ('arg','allow_world_transit'):
    if s.get(key):step[key]=s[key]
   receipt=room,s.get('receipt_script',s['script']);body=scripts[receipt]
   # Native bank2 contributes256 to the shared scenario index, as in the
   # existing qualified FFIV switch recipes. Never conflate bank1 and bank2.
   assert any(m['mnemonic']=='SetFlag' and m['operands'].get('sValues',[None])[0] in ('ScenarioFlag1','ScenarioFlag2') and
    m['operands']['iValues'][0]+(256 if m['operands']['sValues'][0]=='ScenarioFlag2' else 0)==s['flag'] for m in body),receipt
   steps.append(step);facts.append(dict(guide=name,map=s['map'],entity=s['entity'],script=room+'/'+s['script'],receipt='/'.join(receipt),flag=s['flag'],
    sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
  definition=dict(id=name,maps=active_maps,steps=steps,_about='Exact native item/control stages; completion requires the native receipt, not adjacency. Cinematics, manual menus and battles remain native.')
  # Later native story groups retire the key controls even if the optional
  # shop was never opened. Their later receipt must outrank missing old bits.
  definition['done_scenario']=33 if name=='baron_key_doors' else 48 if name=='babel_key_door' else specs[-1]['flag']
  if name in REQUIRES:definition['requires_scenario']=REQUIRES[name]
  author_recipe(data,definition,retired)
 rendered=json.dumps(data,ensure_ascii=False,indent=2)+'\n'
 if '--check' in sys.argv:assert rendered==old,'FFIV native authoring differs'
 else:path.write_text(rendered,encoding='utf-8')
 (ROOT/'build_check/ff4_puzzle_receipts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFIV:',len(RECIPES),'additional families/',len(facts),'native operations.')
if __name__=='__main__':main()
