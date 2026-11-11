"""Independent finite discovery of delegated mechanisms and choice/state callers.

Original inventories are immutable. Store factual source identities/effect kinds,
never mnemonic bodies. Conservative cross-room references are discovery leads,
not an assertion that the engine executes a particular provider in that room.
"""
from pathlib import Path
import csv,hashlib,json,sys
import bake_maps
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
OUT=R/'docs/re/puzzle_mechanics_continuation_inventory_20261104.json'
BASELINES=('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json')
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def key(o):return o['submap'],o['group'],o['props'].get('entity_id'),o['name']
def main():
 old={};sources={};base={n:json.loads((R/'docs/re'/n).read_text(encoding='utf-8'))for n in BASELINES}
 for n,d in base.items():
  for game,g in d['games'].items():
   sources.setdefault(game,g['source_sha256']);assert sources[game]==g['source_sha256']
   old.setdefault(game,set()).update((r['submap'],r['group'],r['entity'],r['raw'])for r in g['candidates'])
 result=dict(discovery='Conservative delegated physical/state and direct choice/state supplement;explicit semantic review required.',baseline_sha256={n:digest(R/'docs/re'/n)for n in BASELINES},games={})
 for game in sources:
  paths={role:D/name for role,name in {'objects':game+'_objects.jsonl','scripts':game+'_scripts.jsonl','maps':game+'_master/map.txt','script_master':game+'_master/script.txt','areas':game+'_master/area.txt','english_names':game+'_master/system_en.txt'}.items()}
  assert {k:digest(p)for k,p in paths.items()}==sources[game]
  maps={int(r['id']):r for r in csv.DictReader(paths['maps'].open(encoding='utf-8-sig'))};aliases={}
  for mid,row in maps.items():aliases.setdefault(row['map_name'],[]).append(mid)
  names={int(r['id']):r['script_name']for r in csv.DictReader(paths['script_master'].open(encoding='utf-8-sig'))}
  areas={int(r['id']):r for r in csv.DictReader(paths['areas'].open(encoding='utf-8-sig'))};english={}
  for line in paths['english_names'].open(encoding='utf-8-sig'):
   if '\t'in line:k,v=line.rstrip('\r\n').split('\t',1);english[k]=v
  rooms={}
  for line in paths['objects'].open(encoding='utf-8'):
   o=json.loads(line)
   if o['submap']in aliases:rooms.setdefault(o['submap'],[]).append(o)
  infos=json.loads((D/(game+'_submaps.json')).read_text(encoding='utf-8'));facts={};by_name={}
  for line in paths['scripts'].open(encoding='utf-8'):
   s=json.loads(line);body=s['mnemonics']
   if not isinstance(body,list):continue
   locator=s['submap']+'/'+s['name'];calls=set();physical=False;state=False;choices=0
   for m in body:
    op=m['mnemonic'];v=m.get('operands')or{};sv=v.get('sValues')or[]
    if op=='ChangeScript'and sv and sv[0]:calls.add(sv[0])
    physical|=op in('Hide','Show','SetPos','Move','MoveTo','SetEntities','CueAnim','PlayAnim')
    state|=op in('SetFlag','ResetFlag')or(op=='Calc'and sv and sv[0]in('MapLocalVariable','AreaLocalVariable'))
    choices+=op=='Select'
   facts[locator]=dict(sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),calls=sorted(calls),physical=physical,state=state,choice_count=choices)
   by_name.setdefault(s['name'],[]).append(locator)
  def providers(room,name):
   here=room+'/'+name
   return [here]if here in facts else by_name.get(name,[])
  candidates=[];selected={};bounds={}
  for room,pool in rooms.items():
   for o in pool:
    if o['type_name']not in('Entity','AnimEntity','NPC','ShopNPC','Event','RandomEvent','SwitchEvent','GotoMap','TreasureBox')or o['props'].get('action_id')not in(2,4,9)or key(o)in old[game]:continue
    name=names.get(o['props'].get('script_id',0),'');locator=room+'/'+name;root=facts.get(locator)
    if not root:continue
    visited=set();todo=[locator];physical=False;state=False
    while todo:
     node=todo.pop()
     if node in visited:continue
     visited.add(node);f=facts[node];physical|=f['physical'];state|=f['state']
     assert len(visited)<=4096,('Unbounded script discovery graph',game,locator)
     for callee in f['calls']:todo.extend(p for p in providers(node.rsplit('/',1)[0],callee)if p not in visited)
    roles=[]
    if root['calls']and physical and state:roles.append('delegated physical/state mechanism;explicit triage')
    if root['choice_count']and state:roles.append('choice and state;explicit triage')
    if not roles:continue
    if room not in bounds:bounds[room]=bake_maps.map_bounds(infos[room],pool)
    ox,oy,*_=bounds[room];area_ids=sorted({int(maps[mid].get('area_id')or 0)for mid in aliases[room]})
    candidates.append(dict(submap=room,maps=aliases[room],area_ids=area_ids,area_names_en=[english.get(areas.get(n,{}).get('area_name',''),areas.get(n,{}).get('area_name',''))for n in area_ids],entity=o['props'].get('entity_id'),raw=o['name'],group=o['group'],type=o['type_name'],roles=roles,cell=[bake_maps.layer_index(o['layer']),o['tx']-ox,bake_maps.tile_row(o)-oy],action=o['props'].get('action_id'),script=name,guide_references=[],status='needs exact semantic triage'))
    selected.update((k,facts[k])for k in visited)
  result['games'][game]=dict(source_sha256=sources[game],counts=dict(candidates=len(candidates),identities=len({(r['submap'],r['group'],r['entity'],r['raw'])for r in candidates}),rooms=len({r['submap']for r in candidates})),candidates=candidates,script_facts=selected)
  print(game,result['games'][game]['counts'],flush=True)
 if '--check'in sys.argv:assert json.loads(OUT.read_text(encoding='utf-8'))==result
 else:OUT.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('All-six bounded continuation/choice discovery;no solution or routing certification.')
if __name__=='__main__':main()
