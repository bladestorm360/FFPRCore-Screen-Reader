"""Exact two-room initializer projection contract; preserve other state fields."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
if __name__=='__main__':
    import author_ff6_story_guides as o,bake_maps,bake_map_state
    infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));d=json.loads((ROOT/'data/ff6/mapstate.json').read_text(encoding='utf-8'))
    for room in('Map_30631_3','Map_30631_4'):
        region=bake_maps.map_bounds(infos[room],[a for a in o.OBJECTS if a['submap']==room])
        expected=bake_map_state.initializer_code(o.SCRIPTS[room,'sc_'+room.lower()],'ff6',region)
        assert d[room]['code']==expected,room
    print('Owzer exact timer registration/facing projections match both pinned native initializers.')
