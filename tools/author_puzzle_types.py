"""Explicit purpose classifications; never infer a solution's type from wording.

Metadata is independent of the source-qualified guide recipes. New recipes must
be classified here before the canonical check can pass. Mixed episodes keep
their native receipt order; disabling a type never skips its unsolved operation.
"""
from pathlib import Path
import json,sys
from ordinary_interaction_fixtures import retired_recipes
ROOT=Path(__file__).resolve().parents[1]
NAV={
1:'citadel_of_trials earth_rod_slab',
2:'deist_safe_stairs kashuan_sunfire mysidia_masks dreadnought_escape coliseum_escape',
3:'',
4:'troia_castle_treasury',
5:'library_policy ghido_stone_chests gohn_city_search fork_tower exdeath_castle_crossing karnak_escape ship_graveyard_jumps trench_chest_switches',
6:'ancient_castle_hidden_stairs terra_defense phoenix_cave kefkas_tower opera narshe_chase narshe_battle burning_house phantom_train narshe_light_path dream_train zone_eater_roof zone_eater_bridges zone_eater_stepping_stones darills_tomb owzers_mansion sealed_gate floating_continent ebots_rock south_figaro_infiltration gerad_passage imperial_palace',
}
OTHER={1:'',2:'fynn_incantation',3:'',4:'magnetic_cave',5:'catapult_clues surgate_books',6:'zozo_clock zozo_clock_reward mobliz_letters'}
OVERRIDES={
'mt_kolts_vargas':['Fight {0}. When Sabin fights alone, use Raging Fist from Blitz: Left, Right, Left, then Confirm.'],
'lola_reply':['Choose the first option to read the letter, then the first option to deliver the reply.','Choose the first option.'],
'cyan_zozo':['Choose the first option to buy Rust Rid for 1000 gil.','Deliver Lola’s reply to the pigeon in Maranda, then return to Zozo.'],
'yeti_cave':['Choose the first option to take the magicite, then win the battle.','Bring Mog in the active party before talking to the yeti.'],
'dream_reunion':['Bring Cyan in a full four-character active party.','Move one step north into the bed, then choose the first option to rest.'],
'dream_doma':['Fight the boss. When it vanishes, knock out and revive allies one at a time until it returns. Keep one ally alive. Attack the boss when it returns; repeat if needed.'],
'solitary_island':['Catch this fish, then return to Cid.','Give Cid every fish you are carrying.','Talk to Cid to refresh the fish.'],
'esper_cave':['Fight {0}. When Relm joins, use Sketch on the enemy.'],
'imperial_palace':['Talk to {0}. Request guidance at each choice.','Continue to {0}. Request guidance at each choice.'],
'opera':['Follow performance cues.'],
'darills_tomb':['Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.'],
'ebots_rock':['Bring Strago in the active party before feeding the chest.','Choose the first option to feed at least 22 coral at once.','Choose the second option to keep the coral, then request guidance again.'],
'south_figaro_infiltration':['Choose the first option.','Choose the second option.','Choose the second option. Use Steal against {0} to take the merchant disguise, then request guidance after battle.','Use Steal against {0} to take the merchant disguise, then request guidance after battle.','Use Steal against {0} to take the soldier disguise, then request guidance after battle.'],
}
def recipe_rows(data):
 for section in ('puzzles','form_guides','story_guides'):
  for row in data.get(section,[]):yield row['id'],row
 for section in ('library_policy','catapult_guide','stone_guide','book_guide','pyramid_guide','jachol_guide','fire_ship_guide'):
  if section in data:yield data[section].get('id',section),data[section]
def expected(n,data):
 rows=dict(recipe_rows(data));nav=set(NAV[n].split());other=set(OTHER[n].split())
 if n==6:nav.update(('mt_kolts_vargas','lethe_river'))
 if n==5:nav.update(('pyramid_of_moore','jachol_cave','istory_flowers','fire_ship_passages','desert_crossing'));other.update(('gogo_sunken_tower','desert_sandworm'))
 if n==6:nav.update(('mobliz_hidden_passage','crescent_serpent_trench','esper_cave','vector_infiltration','solitary_island','dream_reunion','dream_armor','dream_doma','yeti_cave','tzen_rescue','lola_reply','cyan_zozo','late_opera_dragon'))
 if n==3:nav.update(row['id']for row in data.get('form_guides',[]))
 assert not nav&other
 assert set(rows)==nav|other,dict(game=n,unclassified=sorted(set(rows)-nav-other),stale=sorted((nav|other)-set(rows)))
 result={}
 for name,row in rows.items():
  value=dict(kind='navigation'if name in nav else'other')
  if name in OVERRIDES:
   encoded=json.dumps(row,ensure_ascii=False)
   for key in OVERRIDES[name]:assert key in encoded,(name,key)
   value['other']=OVERRIDES[name]
  if name in('opera','imperial_palace','fork_tower'):value['additional']=['other'] # native choices and constrained party preparation
  maps=row.get('maps')
  if name=='library_policy':maps=list(range(272,284))
  if maps:value['maps']=maps
  result[name]=value
 if n==3:result['chocobo_circuit']=dict(kind='navigation',maps=[1])
 return result
def main():
 check='--check'in sys.argv;total=0
 for n in range(1,7):
  p=ROOT/f'data/ff{n}/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'));want=expected(n,data)
  retired={row['id'] for section in ('puzzles','story_guides') for row in retired_recipes(ROOT,f'ff{n}',section)}
  assert not retired.intersection(want),(n,'ordinary interaction republished')
  assert all(len(row['steps'])>=2 for row in data['puzzles']),(n,'standalone puzzle needs multiple steps')
  if check:assert data.get('guidance_types')==want,p
  else:data['guidance_types']=want;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
  total+=len(want)
 print(f'Puzzle purpose classifications: {total} explicit recipes/native policies, all six games.')
if __name__=='__main__':main()
