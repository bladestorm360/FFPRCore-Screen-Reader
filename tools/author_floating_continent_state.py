"""Four exact initializer no-ops; no other state field or map is replaced."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
def expected():
 import author_ff6_story_guides as o,bake_maps,bake_map_state
 room='Map_30600';infos=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))
 region=bake_maps.map_bounds(infos[room],[a for a in o.OBJECTS if a['submap']==room])
 return bake_map_state.initializer_code(o.SCRIPTS[room,'sc_map_30600'],'ff6',region)
if __name__=='__main__':
 p=ROOT/'data/ff6/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'));old=d['Map_30600']['code'];code=expected()
 assert len(old)==len(code);diff=[i for i,(a,b)in enumerate(zip(old,code))if a!=b]
 assert not diff or diff==[1,6,7,8],diff
 for i in diff:assert old[i]==[9]and code[i]==[0],(i,old[i],code[i])
 if'--check'in sys.argv:assert old==code
 else:d['Map_30600']['code']=code;p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Floating Continent exact initializer corrections:',diff)
