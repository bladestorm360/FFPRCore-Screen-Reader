"""Complete original statue/switch/statue objective; game bodies stay external."""
from pathlib import Path
import hashlib,json,sys
import author_ff6_story_guides as owner
ROOT=Path(__file__).resolve().parents[1]
HASHES={
 'Map_30811_5/sc_e_0886':'759901664634c4210e63c4aef7324201774ce189a79adee1359dc8bf854ee94a',
 'Map_30811_5/sc_e_0887':'6dc4fa1526effa7da8242b835226d632b617cc226494cf180f2f53c0e95f9a1b',
 'Map_30811_5/sc_e_0887_2':'3f0a4adc51ebc95fd2686d9f088066a1f425c22005a2490f95de0a91e1f01cd4',
 'Map_30811_6/sc_e_0887_1':'a14521dbdb4dbc102efac8b02750254e7d24a637c65ad14d1b15b81a4b7a639b',
 'Map_30811_7/sc_e_0890':'11628b9da80733c3e9548d341265ba10a5b8ea7c755a9ef1e05f7e535585b4cc',
 'Map_30811_5/sc_map_30811_5':'12443da0211e3d332f3c70372f4bfafd589b276636cfc1b976df0462e9aaeae5',
 'Map_30811_6/sc_map_30811_6':'5abb97458436e4c25f8f7a67629364c504fa36730290d39bd6f30269c75e56c1',
 'Map_30811_7/sc_map_30811_7':'59c55af02641a81c4d5fa60605eff90e57949bb8df62bb692180c2053b5b1432'}
def author():
    for locator,wanted in HASHES.items():
        room,name=locator.split('/')
        assert hashlib.sha256(json.dumps(owner.SCRIPTS[room,name]['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==wanted,locator
    steps=[]
    for mid,e,cell,flag,text in((517,2,(1,14,11),428,'Examine the statue'),(517,12,(1,15,14),429,'Examine the hidden switch'),(519,10,(1,12,9),432,'Examine the statue')):
        o=owner.physical(mid,e);assert o['props']['action_id']==2
        steps.append(dict(map=mid,x=cell[1],y=cell[2],layer=cell[0],examine=True,text=text,entity=e,raw=o['name'],done_scenario=flag))
    return dict(id='ancient_castle_hidden_stairs',_about='Complete statue, hidden-switch and final-statue sequence. Native428 enables the final interaction;429 opens the actual lower passage;432 completes the objective. The old518 construction-scenery waypoint is not a physical exit.',maps=[517,518,519],done_scenario=432,steps=steps,source_script_hashes=HASHES,evidence='docs/re/ANCIENT_CASTLE_20261103.md')
if __name__=='__main__':
    path=ROOT/'data/ff6/puzzles.json';data=json.loads(path.read_text(encoding='utf-8'));expected=author()
    current=next(p for p in data['puzzles']if p['id']==expected['id'])
    if'--check'in sys.argv:assert current==expected
    else:
        data['puzzles']=[expected if p['id']==expected['id']else p for p in data['puzzles']]
        path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Ancient Castle: three exact native operations/real428 prerequisite/actual432 completion/eight source hashes.')
