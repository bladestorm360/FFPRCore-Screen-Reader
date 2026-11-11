"""Source-pinned letter choices, Rust Rid access, Cyan/key/chest and optional switch."""
from pathlib import Path
import hashlib,json,sys
from ordinary_interaction_fixtures import retired_recipes
ROOT=Path(__file__).resolve().parents[1]
READ='Choose the first option to read the letter, then the first option to deliver the reply.'
BUY='Choose the first option to buy Rust Rid for 1000 gil.'
LETTER='Deliver Lola’s reply to the pigeon in Maranda, then return to Zozo.'
DRAGON='Optional: operate this switch to release the dragon, then win the battle.'

def author(o):
 proof=json.loads((ROOT/'docs/re/cyan_zozo_native_20261104.json').read_text(encoding='utf-8'))
 for locator,want in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert hashlib.sha256(json.dumps(o.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==want,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target;go='Continue to {0}, then request guidance again.'
 letters=[r([f(357)],t(278,14,go,'entity_default')),
          r([f(358),f(529)],t(278,15,READ,'ev_e_0727')),
          r([f(358)],t(278,14,'Choose the first option.','entity_default')),
          r([],t(274,69,go,'ev_e_0725'))]
 letters[-1]['finish_on_scenario']=359
 main=[dict(when=[f(361),f(359)],note=LETTER),
       r([f(359,1),f(360),f(365)],t(383,143,go,'ev_e_0730')),
       r([f(361)],t(383,100,BUY,'entity_default')),
       r([f(362)],t(383,104,'Interact to unlock the door.','ev_e_0732')),
       r([f(365)],t(474,16,go,'entity_default')),
       r([f(366)],t(475,1,go,'ev_e_0736')),
       r([],t(474,7,go,'entity_default'))]
 main[-1]['finish_on_scenario']=367
 dragon=[r([f(363)],t(472,1,DRAGON,'ev_e_0733')),
         r([],t(472,2,'Defeat {0}. Contact starts this encounter; request guidance again after battle.','ev_e_0734'))]
 dragon[-1]['finish_on_scenario']=364
 for row in letters+main+dragon:
  for goal in row.get('targets',[]):goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 guides=[]
 for name,maps,rules,selection,end in(
   ('lola_reply',[274,278],letters,[f(353,1),f(365)],359),
   ('zozo_dragon',[472],dragon,[f(353,1),f(367,1)],364),
   ('cyan_zozo',list(range(383,396))+list(range(469,476)),main,[f(353,1)],367)):
  g=o.guide(name,maps,'',rules);g.update(first_match=True,select_when=selection,complete_scenario=end,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/CYAN_ZOZO_GUIDANCE_20261104.md');guides.append(g)
 return guides

def door(o):
 import bake_maps
 actor=o.physical(383,104,'ev_e_0732');assert actor['type_name']=='Entity'and actor['props']['collision']and actor['props']['action_id']==2
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info[actor['submap']],[a for a in o.OBJECTS if a['submap']==actor['submap']])
 return dict(id=104,group=actor['group'],raw=actor['name'],cell=[bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy],ability_scenario=361,receipt=362)

def final_poses(o):
 import bake_maps
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));rows={}
 for map_id,name,pc in((274,'sc_e_0729_2',12),(474,'sc_e_0735_2',383)):
  room=o.MAPS[map_id]['map_name'];m=o.SCRIPTS[room,name]['mnemonics'][pc];assert m['mnemonic']=='SetPos'
  x,y,layer=m['operands']['iValues'][:3];assert (x,y)==((28,10)if map_id==274 else(42,56))
  ox,oy,*_=bake_maps.map_bounds(info[room],[a for a in o.OBJECTS if a['submap']==room])
  # Native Ground persists through the final reunion. The letter return
  # explicitly switches the puppet to Ground (native layer value2).
  assert layer==(2 if map_id==274 else 0)
  rows[str(map_id)]=[1,x-ox,y-oy]
 return rows

if __name__=='__main__':
 import author_ff6_story_guides as o
 check='--check'in sys.argv;guides=author(o);p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 retired=retired_recipes(ROOT,'ff6','story_guides')
 assert next(g for g in guides if g['id']=='zozo_dragon') in retired
 guides=[g for g in guides if g['id']!='zozo_dragon']
 assert all(g['id']!='zozo_dragon' for g in data['story_guides'])
 if check:
  for g in guides:assert[a for a in data['story_guides']if a['id']==g['id']]==[g]
  names=[g['id']for g in data['story_guides']];assert names.index('cyan_zozo')<names.index('zozo_clock_reward')
 else:
  # The ruined-world guide yields the exact early clock maps to their accepted
  # recipe before native353. Older recipes retain their relative order/content.
  prior=[g for g in data['story_guides']if g['id']not in{a['id']for a in guides}];at=next(i for i,g in enumerate(prior)if g['id']=='zozo_clock_reward')
  data['story_guides']=prior[:at]+guides+prior[at:];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'));want=[door(o)]
 if check:assert data['Map_30510'].get('item_gates')==want
 else:data['Map_30510']['item_gates']=want;p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 import bake_maps,cyan_passage
 actor=o.physical(474,16,'entity_default');guard=cyan_passage.guard(actor,o.SCRIPTS[actor['submap'],'sc_e_0735']);assert guard
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(info[actor['submap']],[a for a in o.OBJECTS if a['submap']==actor['submap']])
 guard.update(id=16,group=actor['group'],type=actor['type'],raw=actor['name'],cell=[1,actor['tx']-ox,bake_maps.tile_row(actor)-oy],source='sc_e_0735',map_ids=[474],area_id=31)
 existing=data['Map_30681_1'].get('events',[])
 if check:assert [r for r in existing if r.get('source')=='sc_e_0735']==[guard]
 else:data['Map_30681_1']['events']=[r for r in existing if r.get('source')!='sc_e_0735']+[guard];p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 p=ROOT/'docs/re/cyan_zozo_native_20261104.json';proof=json.loads(p.read_text(encoding='utf-8'));poses=final_poses(o)
 if check:assert proof.get('final_player_cells')==poses
 else:proof['final_player_cells']=poses;p.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Cyan/Zozo:real357/declined529/retry358/pigeon359;purchase361/manual362/Cyan365/key366/chest367;separate optional dragon363/364. One exact Rust Rid capability gate.')
