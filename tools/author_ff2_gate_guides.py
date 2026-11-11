"""Author additional FF2 physical gates without changing the Closed Deist recipe."""
import csv,hashlib,json,sys
from pathlib import Path
import bake_maps
from ff2_gate_specs import RECIPES,REQUIRES
from ordinary_interaction_fixtures import author_recipe, retired_recipes
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'

def main():
    maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff2_master/map.txt').open(encoding='utf-8-sig'))}
    names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff2_master/script.txt').open(encoding='utf-8-sig'))}
    rooms={maps[s['map']]['map_name'] for _,_,specs in RECIPES for s in specs}
    objects=[]
    for line in (DUMP/'ff2_objects.jsonl').open(encoding='utf-8'):
        row=json.loads(line)
        if row['submap'] in rooms:objects.append(row)
    infos=json.loads((DUMP/'ff2_submaps.json').read_text(encoding='utf-8'))
    wanted={(maps[s.get('receipt_map',s['map'])]['map_name'],s.get('receipt_script',s['script'])) for _,_,specs in RECIPES for s in specs}
    scripts={}
    for line in (DUMP/'ff2_scripts.jsonl').open(encoding='utf-8'):
        row=json.loads(line);key=row['submap'],row['name']
        if key in wanted:scripts[key]=row['mnemonics']
    assert scripts.keys()==wanted
    path=ROOT/'data/ff2/puzzles.json';old=path.read_text(encoding='utf-8');data=json.loads(old);facts=[]
    retired=retired_recipes(ROOT,'ff2')
    for name,active_maps,specs in RECIPES:
        steps=[]
        for spec in specs:
            room=maps[spec['map']]['map_name']
            actors=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==spec['entity'] and names.get(o['props'].get('script_id'))==spec['script']]
            assert len(actors)==1,(name,spec['map'],spec['entity'],len(actors));actor=actors[0]
            ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
            step=dict(map=spec['map'],entity=spec['entity'],raw=actor['name'],layer=bake_maps.layer_index(actor['layer']),
                x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,examine=actor['type_name'] not in ('Event','RandomEvent','SwitchEvent'),text=spec['text'])
            for key in ('arg','arg2','allow_world_transit','before_scenario'):
                if spec.get(key):step[key]=spec[key]
            receipt=(maps[spec.get('receipt_map',spec['map'])]['map_name'],spec.get('receipt_script',spec['script']));body=scripts[receipt]
            if spec['flag'] is not None:
                assert any(m['mnemonic']=='SetFlag' and m['operands'].get('sValues',[None])[0]=='ScenarioFlag1' and m['operands']['iValues'][0]==spec['flag'] for m in body),receipt
                step['done_scenario']=spec['flag']
            else:
                assert spec['map']==43 and spec['entity']==23 and spec['keyword'] in (924,930,931)
                step['done_keyword']=spec['keyword']
            steps.append(step)
            facts.append(dict(guide=name,map=spec['map'],entity=spec['entity'],script=room+'/'+spec['script'],receipt='/'.join(receipt),
                receipt_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
                progress={k:v for k,v in step.items() if k.startswith('done_')}))
        definition=dict(id=name,maps=active_maps,steps=steps,
            _about='Original native operation sequence. Learned keywords and exact scenario receipts support resumes; proximity is never completion. Named quest items are prerequisites to their native item-use controls. Existing manual menus and game-owned cinematics remain native.')
        definition['done_scenario']=specs[-1]['flag']
        if name in REQUIRES:definition['requires_scenario']=REQUIRES[name]
        author_recipe(data,definition,retired)
    rendered=json.dumps(data,ensure_ascii=False,indent=2)+'\n'
    if '--check' in sys.argv:assert old==rendered,'FF2 gate recipes differ from native authoring'
    else:path.write_text(rendered,encoding='utf-8')
    (ROOT/'build_check/ff2_gate_receipts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FF2 additional gates:',len(RECIPES),'families,',len(facts),'exact native operations; Deist unchanged.')

if __name__=='__main__':main()
