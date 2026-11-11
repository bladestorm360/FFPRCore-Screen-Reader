"""Original FF2 guided operations from exact actors and native receipts.

Native script bodies stay in the external mapdump; only factual receipt hashes
and original instructions are written here. Preserve other games and recipes.
"""
import csv,hashlib,json,sys
from pathlib import Path
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
SPECS=[
 (176,32,'sc_e_0045','sc_e_0045',41,'Speak to the boy.'),
 (176,32,'sc_e_0045','sc_e_0045',42,'Follow the boy and speak to him again.'),
 (179,28,'sc_e_0047','sc_e_0047',None,'Speak to the mother and learn {0}. Repeat the guide after learning it.'),
 (184,32,'sc_e_0049','sc_e_0049',43,'Examine the pendant.'),
 (180,6,'sc_e_0050','sc_e_0050',44,'Ask the wyvern about {0} to receive its egg.'),
 (187,34,'sc_e_0051','sc_e_0051_1',45,'Follow the safe stairs past the B3 pitfall, then defeat the guardian.'),
 (187,41,'sc_e_0052','sc_e_0052',46,'Use {0} at the Life Spring.')]

def main():
    maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff2_master/map.txt').open(encoding='utf-8-sig'))}
    names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff2_master/script.txt').open(encoding='utf-8-sig'))}
    rooms={maps[m]['map_name'] for m,*_ in SPECS}
    objects=[o for line in (DUMP/'ff2_objects.jsonl').open(encoding='utf-8') if (o:=json.loads(line))['submap'] in rooms]
    infos=json.loads((DUMP/'ff2_submaps.json').read_text(encoding='utf-8'))
    wanted={(maps[m]['map_name'],receipt) for m,_,_,receipt,_,_ in SPECS}
    scripts={}
    for line in (DUMP/'ff2_scripts.jsonl').open(encoding='utf-8'):
        r=json.loads(line);key=r['submap'],r['name']
        if key in wanted:scripts[key]=r['mnemonics']
    assert scripts.keys()==wanted
    steps=[];facts=[]
    for map_id,entity,name,receipt,flag,text in SPECS:
        room=maps[map_id]['map_name'];body=scripts[room,receipt]
        actors=[o for o in objects if o['submap']==room and o['props'].get('entity_id')==entity and names.get(o['props'].get('script_id'))==name]
        assert len(actors)==(2 if map_id==180 else 1),(map_id,entity,len(actors))
        actor=actors[0]
        # Both reviewed Wyvern groups have the same logical identity and pose;
        # the native initializer/actual capture selects the active variant.
        assert all((o['name'],o['layer'],o['tx'],o['ty'])==(actor['name'],actor['layer'],actor['tx'],actor['ty']) for o in actors)
        ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in objects if o['submap']==room])
        step=dict(map=map_id,entity=entity,raw=actor['name'],layer=bake_maps.layer_index(actor['layer']),
            x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,examine=True,text=text,allow_world_transit=True)
        if flag is not None:
            assert any(m['mnemonic']=='SetFlag' and m['operands'].get('sValues',[None])[0]=='ScenarioFlag1' and m['operands']['iValues'][0]==flag for m in body)
            step['done_scenario']=flag
        else:
            assert map_id==179 and any(m['mnemonic']=='Msg' and m['operands']['sValues'][0]=='E0047_00_179_e_01' for m in body)
            step['done_keyword']=927
        if '{0}' in text:step['arg']='modtext:FF2 item Wyvern Egg' if map_id==187 else 'modtext:FF2 keyword Wyverns'
        steps.append(step)
        facts.append(dict(map=map_id,entity=entity,script=room+'/'+name,receipt=room+'/'+receipt,
            receipt_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
            progress={k:v for k,v in step.items() if k.startswith('done_')}))
    path=ROOT/'data/ff2/puzzles.json';old=path.read_text(encoding='utf-8');data=json.loads(old)
    recipe=next(p for p in data['puzzles'] if p['id']=='deist_safe_stairs')
    recipe.update(maps=list(range(176,191)),steps=steps,
        _about='Complete Deist dependency chain: boy41/42, actual learned Wyverns927, pendant43, egg44, guardian45, egg placement46. Actual Life Spring41 at B5Ground25,8 is the end; B5 PointIn1 is only an arrival. Explicit authored world transit connects castle/cave. Repeat after learning; native choices and manual operations remain game-owned.')
    data['_comment']='Original factual FF2 map/story puzzle guides. Every manual operation uses its exact native receipt; adjacency or a dungeon-floor arrival never substitutes for completion.'
    rendered=json.dumps(data,ensure_ascii=False,indent=2)+'\n'
    if '--check' in sys.argv:assert old==rendered,'FF2 guide differs from reviewed native authoring'
    else:path.write_text(rendered,encoding='utf-8')
    (ROOT/'build_check/ff2_puzzle_receipts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FF2 Deist: seven native operations including keyword learning, true spring interaction and full dependency chain.')

if __name__=='__main__':main()
