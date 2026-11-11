"""Exact automatic painting/door landings; native bodies remain external."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(owner):
    import bake_maps
    manifest=json.loads((ROOT/'docs/re/owzers_mansion_native_20261103.json').read_text(encoding='utf-8'))
    rows=[];room='Map_30631_4'
    infos=json.loads((owner.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))
    ox,oy,*_=bake_maps.map_bounds(infos[room],[o for o in owner.OBJECTS if o['submap']==room])
    for entity,name,landing in ((56,'sc_e_0765',[71,52,1]),(57,'sc_e_0766',[85,46,1]),(55,'sc_e_1064',[90,60,1])):
        actor=owner.physical(445,entity);body=owner.SCRIPTS[room,name]
        assert sha(body['mnemonics'])==manifest['source_script_hashes'][room+'/'+name]
        assert actor['type_name']=='Event' and actor['props']['action_id']==2
        rows.append(dict(submap=room,entity_id=entity,object_id=actor['id'],destination_map=445,original_point=0,continuation=name,continuation_sha256=sha(body),same_map=True,ordinary_travel=True,arrival_cell=landing,arrival_baked_cell=[landing[0]-ox,landing[1]-oy,1],evidence='Exact automatic native painting/door transfer and final player pose. docs/re/OWZERS_MANSION_20261103.md'))
    actor=owner.physical(445,59);body=owner.SCRIPTS[room,'sc_e_0766_2']
    assert sha(body['mnemonics'])==manifest['source_script_hashes'][room+'/sc_e_0766_2']
    rows.append(dict(submap=room,entity_id=59,object_id=actor['id'],destination_map=445,original_point=3,continuation='sc_e_0766_2',continuation_sha256=sha(body),arrival_point=3,ordinary_travel=True,evidence='Native return transfer resets local1 and restarts the door cycle. docs/re/OWZERS_MANSION_20261103.md'))
    return rows
if __name__=='__main__':
    import author_ff6_story_guides as owner
    p=ROOT/'data/ff6/warp_arrival_overrides.json';old=json.loads(p.read_text(encoding='utf-8'));expected=author(owner)
    for row in expected:
        matches=[r for r in old if(r['submap'],r.get('object_id'))==(row['submap'],row['object_id'])]
        if'--check'in sys.argv:assert matches==[row]
        elif matches:assert len(matches)==1;old[old.index(matches[0])]=row
        else:old.append(row)
    if'--check'not in sys.argv:p.write_text(json.dumps(old,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Owzer: three exact automatic same-floor landings and native return point3.')
