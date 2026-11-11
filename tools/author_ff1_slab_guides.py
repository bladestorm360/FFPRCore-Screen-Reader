"""Original slab guides from exact FF1 identities and native operation receipts.

Raw bodies remain in the external mapdump. A fetch/return leg explicitly opts
into bounded world transit; ordinary indoor routing keeps its existing policy.
"""
import csv,hashlib,json,sys
from pathlib import Path
import bake_maps
from ordinary_interaction_fixtures import author_recipe, retired_recipes
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
SPECS=[(78,'sc_e_0016','sc_e_0016_2',16,'Defeat the guardian.'),
       (78,'sc_e_0017','sc_e_0017',17,'Open the chest after the battle.'),
       (81,'sc_e_0018','sc_e_0018',18,'Speak to the giant.'),
       (40,'sc_e_0019','sc_e_0019',19,'Speak to the sage to obtain the rod.'),
       (78,'sc_e_0020','sc_e_0020',20,'Examine the stone slab to use the rod.'),
       (115,'sc_e_0039','sc_e_0039',37,'Examine the stone slab to play the Lute.'),
       (93,'sc_e_0046','sc_e_0046',44,'Speak to the elder to begin the trial.'),
       (95,'sc_e_0047','sc_e_0047',45,'Open the chest after the battle.')]
def main():
    maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff1_master/map.txt').open(encoding='utf-8-sig'))}
    names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff1_master/script.txt').open(encoding='utf-8-sig'))}
    rooms={maps[m]['map_name'] for m,*_ in SPECS}
    objects=[o for line in (DUMP/'ff1_objects.jsonl').open(encoding='utf-8') if (o:=json.loads(line))['submap'] in rooms]
    infos=json.loads((DUMP/'ff1_submaps.json').read_text(encoding='utf-8'))
    wanted={(maps[m]['map_name'],receipt) for m,_,receipt,_,_ in SPECS}
    scripts={}
    for line in (DUMP/'ff1_scripts.jsonl').open(encoding='utf-8'):
        row=json.loads(line);key=row['submap'],row['name']
        if key in wanted:scripts[key]=row['mnemonics']
    assert scripts.keys()==wanted
    steps=[];facts=[]
    for map_id,name,receipt,flag,text in SPECS:
        room=maps[map_id]['map_name'];body=scripts[room,receipt]
        assert any(m['mnemonic']=='SetFlag' and m['operands']['sValues'][0]=='ScenarioFlag1' and m['operands']['iValues'][0]==flag for m in body)
        owners=[o for o in objects if o['submap']==room and names.get(o['props'].get('script_id'))==name]
        assert len(owners)==1,(map_id,name,len(owners));actor=owners[0]
        ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
        step=dict(map=map_id,entity=actor['props']['entity_id'],raw=actor['name'],
            layer=bake_maps.layer_index(actor['layer']),x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,
            examine=actor['type_name'] not in ('Event','RandomEvent','SwitchEvent'),text=text,done_scenario=flag)
        if map_id in (78,81,40):step['allow_world_transit']=True
        steps.append(step)
        facts.append(dict(map=map_id,entity=step['entity'],script=room+'/'+name,receipt=room+'/'+receipt,
            receipt_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),done_scenario=flag))
    path=ROOT/'data/ff1/puzzles.json';old=path.read_text(encoding='utf-8');data=json.loads(old)
    definitions=[dict(id='earth_rod_slab',maps=[78,81,40],
        _about='Vampire victory16, actual Ruby acquisition17, Giant passage18, Sarda Rod19 and slab20. Native receipts skip completed prerequisites; authored world transit only. No treasure-content label disclosure.',steps=steps[:5]),
        dict(id='lute_slab',maps=[115],_about='Actual interactive slab9, not decoration16; native Lute sequence37 is completion, not proximity.',steps=steps[5:6])]
    citadel=next(p for p in data['puzzles'] if p['id']=='citadel_of_trials')
    # Keep the owner-confirmed maze unchanged;3F arrival is an intermediate
    # navigation receipt. Only the real reward interaction completes the trial.
    reward=steps[7]
    assert (reward['entity'],reward['layer'],reward['x'],reward['y'])==(9,0,27,29)
    chest=next(o for o in objects if o['submap']==maps[95]['map_name'] and o['props'].get('entity_id')==9)
    assert chest['type_name']=='TreasureBox' and chest['props']['flag_id']==512 and chest['props']['content_id']==57
    receipt=scripts[maps[95]['map_name'],'sc_e_0047']
    assert next(i for i,m in enumerate(receipt) if m['mnemonic']=='TreasureBox')<next(i for i,m in enumerate(receipt) if m['mnemonic']=='SetFlag')
    citadel['steps']=[steps[6]]+[s for s in citadel['steps'] if s.get('reach_map')]+[reward]
    citadel['done_scenario']=45
    citadel['_about']='Speak to the first-floor elder (native44), follow the unchanged teleport maze to3F native PointIn1, Upper35,32, then open the trial reward chest9 at Ground27,29. Native reward45 completes the entire guide and persists after the item is handed over;3F arrival alone only completes the maze step. The automatic return warp at Bottom29,28 is never a guide target.'
    retired=retired_recipes(ROOT,'ff1')
    for recipe in definitions:author_recipe(data,recipe,retired)
    rendered=json.dumps(data,ensure_ascii=False,indent=2)+'\n'
    if '--check' in sys.argv:assert old==rendered,'FF1 slab guide differs from reviewed authoring'
    else:path.write_text(rendered,encoding='utf-8')
    (ROOT/'build_check/ff1_slab_receipts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FF1: eight native receipts for the Rod chain, ordinary Lute slab and Citadel elder/maze/reward.')
if __name__=='__main__':main()
