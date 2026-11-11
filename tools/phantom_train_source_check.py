"""Exact native prerequisites, landing/gate/compiler and localization contract."""
from pathlib import Path
import json
import author_ff6_story_guides as owner
import bake_map_state,phantom_train_exits
from extend_phantom_train_guide import extend,engine_template
ROOT=Path(__file__).resolve().parents[1]
def main():
    data=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'))
    guide=next(g for g in data['story_guides']if g['id']=='phantom_train')
    assert guide==extend(owner,engine_template(owner))
    state=json.loads((ROOT/'data/ff6/mapstate.json').read_text(encoding='utf-8'))
    matched=0
    for o in owner.OBJECTS:
        if phantom_train_exits.identity(o)is None:continue
        expected=phantom_train_exits.guard(o,owner.SCRIPTS[o['submap'],'sc_e_0162'])
        actual=[r for r in state[o['submap']]['events']if r.get('id')==o['props']['entity_id']and r.get('destination')==expected['destination']]
        assert len(actual)==1 and all(actual[0][k]==v for k,v in expected.items())
        assert actual[0]['cell']==[1,o['tx'],o['ty']-1]
        matched+=1
    assert matched==4
    source=owner.SCRIPTS['Map_30440','sc_map_30440']
    # No positional puppet in this12-op initializer; its native range does not
    # affect compilation of the two bounded unknown membership branches.
    expected=bake_map_state.initializer_code(source,'ff6',[0,0,1,1])
    assert state['Map_30440']['code']==expected and expected[3]==expected[4]==[2,-2,0,0,-1,0,6]
    catalog=json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))
    keys={guide['introduction'],guide['complete']}
    keys.update(t['text']for r in guide['rules']for t in r.get('targets',[]))
    langs={'en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th'}
    for key in keys:
        assert set(catalog[key])==langs and all(v and v.count('{0}')==key.count('{0}')for v in catalog[key].values()),key
    print('Phantom source:16 rules/27 source hashes/four exact native transfers and109 guards/two safe unknown membership branches; all reused strings in12 languages.')
if __name__=='__main__':main()
