"""One exact missing active-Sabin initializer predicate; preserve authored state."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
HASH='2cc1f33ee991e80f58073d75930944005f3c11eefb8ef0855efaa990728ed961'
def main():
 import author_ff6_story_guides as o,bake_map_state,bake_maps
 source=o.SCRIPTS['Map_20400','sc_map_20400'];assert hashlib.sha256(json.dumps(source['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==HASH
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))['Map_20400'];bounds=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']=='Map_20400'])
 expected=bake_map_state.initializer_code(source,'ff6',bounds)
 p=ROOT/'data/ff6/mapstate.json';state=json.loads(p.read_text(encoding='utf-8'));old=state['Map_20400']['code'];assert len(old)==len(expected)
 changes=[i for i,(a,b)in enumerate(zip(old,expected))if a!=b]
 assert changes in([], [4])and expected[4]==[2,15,6,0,-1,0,6]
 if changes:assert old[4]==[9]
 if '--check'in sys.argv:assert not changes
 else:
  state['Map_20400']['code']=expected;p.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Duncan: exact active-Sabin6 branch; reserve/unknown do not grant training; all unrelated authored state retained.')
if __name__=='__main__':main()
