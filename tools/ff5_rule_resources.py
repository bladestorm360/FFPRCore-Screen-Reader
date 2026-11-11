"""Original shared authoring facts for the new bounded FFV rule leaf."""
from pathlib import Path
import json
R=Path(__file__).resolve().parents[1]
def map_header(data):
 from author_puzzle_types import expected
 types=expected(5,data);maps={}
 for g in data.get('story_guides',[])+([data['fire_ship_guide']]if 'fire_ship_guide'in data else[]):
  row=types[g['id']];mask=1 if row['kind']=='navigation'else 2
  if row.get('other')or'other'in row.get('additional',[]):mask|=2
  if row.get('navigation')or'navigation'in row.get('additional',[]):mask|=1
  for mid in g['maps']:maps[mid]=maps.get(mid,0)|mask
 assert 0<len(maps)<=256
 contexts=[]
 for g in data.get('story_guides',[])+([data['fire_ship_guide']]if 'fire_ship_guide'in data else[]):
  for mid in g.get('transit_maps',[]):contexts.append((mid,g['id'],maps[g['maps'][0]]))
 assert len(contexts)<=256 and all(0<mid<32768 for mid,_,_ in contexts)
 return '// Generated original map/type facts;no native calls or recipe scan.\n#pragma once\n#include <string_view>\nnamespace ff5::mechanics {\ninline unsigned types(int map){switch(map){\n'+''.join(f'case {mid}:return {mask}u;\n'for mid,mask in sorted(maps.items()))+'default:return 0;}}\ninline unsigned context_types(int map,std::wstring_view guide){\n'+''.join(f'if(map=={mid}&&guide==L"{id}")return {mask}u;\n'for mid,id,mask in contexts)+'return 0;\n}\n}\n'
def update_header(data,check):
 p=R/'src/games/ff5/mechanics_maps.h';wanted=map_header(data)
 if check:assert p.read_text(encoding='utf-8')==wanted
 else:p.write_text(wanted,encoding='utf-8')
