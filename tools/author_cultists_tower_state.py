"""One exact initializer-row repair; retain every other authored state field."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
if __name__=='__main__':
    import author_ff6_story_guides as owner
    import bake_map_state,bake_maps
    room='Map_30700';infos=json.loads((owner.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));region=bake_maps.map_bounds(infos[room],[o for o in owner.OBJECTS if o['submap']==room])
    expected=bake_map_state.initializer_code(owner.SCRIPTS[room,'sc_map_30700'],'ff6',region)
    p=ROOT/'data/ff6/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'));old=d[room]['code'];assert len(old)==len(expected)==18
    differences=[(i,a,b)for i,(a,b)in enumerate(zip(old,expected))if a!=b]
    if'--check'in sys.argv:assert not differences
    else:
        assert differences in([],[(5,[9],[2,-2,0,0,-1,0,8])]),differences
        d[room]['code']=expected;p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
    print('Cultists entrance: one exact unknown-character branch; common392 gate absence retained, no membership guessed.')
