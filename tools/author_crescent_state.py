"""One source-qualified Gau role branch; preserve all other authored state."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
HASH='2be0d7bfb883c48fbd746bc47278506908fbb5d6619cece5149045cddc6e4572'
if __name__=='__main__':
 import author_ff6_story_guides as o,bake_map_state,bake_maps
 source=o.SCRIPTS['Map_30491_1','sc_map_30491_1'];assert hashlib.sha256(json.dumps(source['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==HASH
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))['Map_30491_1'];bounds=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']=='Map_30491_1'])
 expected=bake_map_state.initializer_code(source,'ff6',bounds)
 p=ROOT/'data/ff6/mapstate.json';state=json.loads(p.read_text(encoding='utf-8'));old=state['Map_30491_1']['code'];assert len(old)==len(expected)
 changes=[i for i,(a,b)in enumerate(zip(old,expected))if a!=b];assert changes in([], [4])and expected[4]==[2,15,12,0,-1,0,23]
 if changes:assert old[4]==[9]
 if '--check'in sys.argv:assert not changes
 else:
  state['Map_30491_1']['code']=expected;p.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Crescent:exact active-Gau12 branch;all unrelated authored state preserved.')
