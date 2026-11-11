"""Finite all-six map/story mechanic inventory, not a solution certification.

Only factual identities, source hashes and effect indices are written here.
Native script bodies/operands remain in the external mapdump. Label candidates
and named interpreter calls are independent discovery passes; each still needs
human-owned semantic triage against its exact source.
"""
import csv
import hashlib
import json
from pathlib import Path
import re
import sys
import bake_maps

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
PATTERNS={
 'control':r'スイッチ|スウィッチ|レバー|ボタン|switch|lever|button',
 'riddle':r'石版|石板|石碑|墓石|墓標|銘文|時計|暗号|パズル|謎|riddle|puzzle|clock',
 'movement':r'飛び石|ジャンプ|小人|こびと|ミニマム|トード|カエル|床が|天井|落ちる床|動く床|回転|滑る|mini|toad',
 'passage':r'隠し扉|隠し階段|隠し部屋|隠し通路|本棚|絵画|壁画|絵を|hidden door|secret passage',
 'memory':r'順番|光の道|光る道|光を|ライト|ランプ|演奏|楽器|ピアノ|オペラ|箱の配置',
}
MATCHERS={k:re.compile(v,re.I) for k,v in PATTERNS.items()}
ACTORS={'Entity','AnimEntity','NPC','ShopNPC','Event','RandomEvent','SwitchEvent','GotoMap','TreasureBox'}

def digest(path):
 h=hashlib.sha256()
 with path.open('rb') as f:
  for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
 return h.hexdigest()

def main():
 animation_pass='--animation-pass' in sys.argv
 baseline=ROOT/'docs/re/puzzle_mechanics_inventory_20261031.json'
 result={'about':'Candidate inventory only. Existing target references and possible flag receipts do not certify a complete puzzle solution. Native source bodies remain external. Optional minigames are outside the current additional solution scope.', 'games':{}}
 if animation_pass:result.update(discovery='Independent animation-plus-local-state supplement; baseline1634 retained unchanged',baseline_sha256=digest(baseline))
 for number in range(1,7):
  game=f'ff{number}'
  paths={key:DUMP/name for key,name in {
   'objects':f'{game}_objects.jsonl','scripts':f'{game}_scripts.jsonl',
   'maps':f'{game}_master/map.txt','script_master':f'{game}_master/script.txt',
   'areas':f'{game}_master/area.txt','english_names':f'{game}_master/system_en.txt'}.items()}
  maps={int(r['id']):r for r in csv.DictReader(paths['maps'].open(encoding='utf-8-sig'))}
  areas={int(r['id']):r for r in csv.DictReader(paths['areas'].open(encoding='utf-8-sig'))}
  english={}
  for line in paths['english_names'].open(encoding='utf-8-sig'):
   if '\t' in line:
    key,value=line.rstrip('\r\n').split('\t',1);english[key]=value
  names={int(r['id']):r['script_name'] for r in csv.DictReader(paths['script_master'].open(encoding='utf-8-sig'))}
  aliases={}
  for map_id,row in maps.items():aliases.setdefault(row['map_name'],[]).append(map_id)
  rooms={}
  for line in paths['objects'].open(encoding='utf-8'):
   o=json.loads(line);rooms.setdefault(o['submap'],[]).append(o)
  infos=json.loads((DUMP/f'{game}_submaps.json').read_text(encoding='utf-8'))
  puzzles=json.loads((ROOT/f'data/{game}/puzzles.json').read_text(encoding='utf-8'))
  references={}
  def reference(map_id,entity,raw,guide):
   if entity is not None:references.setdefault((map_id,entity,raw),set()).add(guide)
  for p in puzzles['puzzles']:
   for s in p['steps']:reference(s['map'],s.get('entity'),s.get('raw',''),p['id'])
  for p in puzzles.get('story_guides',[]):
   for key in ('maze_targets','pattern_targets'):
    for s in p.get(key,[]):reference(s['map'],s.get('entity'),s.get('raw',''),p['id'])
   for rule in p.get('rules',[]):
    for s in rule.get('targets',[]):reference(s['map'],s.get('entity'),s.get('raw',''),p['id'])
    s=rule.get('at',{})
    if 'map' in s:reference(s['map'],s.get('entity'),s.get('raw',''),p['id'])
  candidates=[];script_owners={};candidate_keys=set();bounds_by_room={}
  def identity(o):return (o['submap'],o['group'],o['props'].get('entity_id'),o['name'])
  def candidate(o,roles):
    room=o['submap']
    if room not in bounds_by_room:bounds_by_room[room]=bake_maps.map_bounds(infos[room],rooms[room])
    ox,oy,*_=bounds_by_room[room];entity=o['props'].get('entity_id')
    area_ids=sorted({int(maps[m].get('area_id') or 0) for m in aliases[room]})
    guides=set()
    for mid in aliases[room]:guides.update(references.get((mid,entity,o['name']),()))
    return dict(submap=room,maps=aliases[room],area_ids=area_ids,
       area_names_en=[english.get(areas.get(n,{}).get('area_name',''),areas.get(n,{}).get('area_name','')) for n in area_ids],
       entity=entity,raw=o['name'],group=o['group'],type=o['type_name'],roles=roles,
       cell=[bake_maps.layer_index(o['layer']),o['tx']-ox,bake_maps.tile_row(o)-oy],
       action=o['props'].get('action_id'),script=names.get(o['props'].get('script_id',0),''),guide_references=sorted(guides),
       status='referenced; semantic triage required' if guides else 'needs semantic triage')
  for room,pool in rooms.items():
   if room not in aliases:continue
   for o in pool:
    if o['type_name'] not in ACTORS:continue
    key=room,names.get(o['props'].get('script_id',0),'')
    script_owners.setdefault(key,[]).append(o)
    roles=[k for k,p in MATCHERS.items() if p.search(o['name'])]
    if o['type_name']=='SwitchEvent' and 'control' not in roles:roles.append('control')
    if not roles:continue
    candidates.append(candidate(o,roles));candidate_keys.add(identity(o))
  wanted={(r['submap'],r['script']) for r in candidates if r['script']}
  facts={};calls={};special=[];unavailable=[];semantic=0;animated=0;animation_keys=set()
  for line in paths['scripts'].open(encoding='utf-8'):
   s=json.loads(line);key=s['submap'],s['name'];body=s['mnemonics']
   if not isinstance(body,list):
    owners=script_owners.get(key,[])
    unavailable.append(dict(script='/'.join(key),physical_owners=[dict(maps=aliases.get(o['submap'],[]),entity=o['props'].get('entity_id'),type=o['type_name'],action=o['props'].get('action_id')) for o in owners]))
    for o in owners:
     if identity(o) in candidate_keys:continue
     candidates.append(candidate(o,['unavailable script; explicit triage']));candidate_keys.add(identity(o));wanted.add(key)
    if key in wanted:facts['/'.join(key)]={'available':False}
    continue
   named=set();flags=set();resets=set();locals_=set();destinations=set();selects=0;geometry=False;animation=False
   for m in body:
    op=m['mnemonic'];v=m.get('operands',{});iv=v.get('iValues',[]);sv=v.get('sValues',[])
    if op=='SysCall' and sv and sv[0]:
     name=sv[0];named.add(name);calls.setdefault(name,[]).append('/'.join(key))
    if op=='SetFlag' and sv and iv and sv[0].startswith('ScenarioFlag'):
     flags.add(iv[0]+256*(int(sv[0][-1])-1))
    if op=='ResetFlag' and sv and iv and sv[0].startswith('ScenarioFlag'):
     resets.add(iv[0]+256*(int(sv[0][-1])-1))
    if op=='Calc' and sv and iv and sv[0] in ('MapLocalVariable','AreaLocalVariable'):locals_.add((sv[0],iv[0]))
    if op=='ChangeMap' and iv:destinations.add(iv[0])
    if op=='Select':selects+=1
    geometry|=op in ('Hide','Show','SetPos','Move','MoveTo','SetEntities')
    animation|=op in ('CueAnim','PlayAnim')
   # Independent of developer labels: stateful physical scene changes can be
   # switches/bookshelves/doors with generic names. This is a candidate pass,
   # not an inference that every short story/cutscene is a puzzle.
   owners=script_owners.get(key,[])
   semantic_owners=[o for o in owners if o['props'].get('action_id') in (2,4,9)] if geometry and (flags or locals_) else []
   for o in semantic_owners:
    if identity(o) in candidate_keys:continue
    candidates.append(candidate(o,['stateful physical script']));candidate_keys.add(identity(o));semantic+=1
   animation_owners=[o for o in owners if o['props'].get('action_id') in (2,4,9)] if animation_pass and animation and (locals_ or flags or resets) else []
   for o in animation_owners:
    if identity(o) in candidate_keys:continue
    candidates.append(candidate(o,['animation and local state; explicit triage']));candidate_keys.add(identity(o));animation_keys.add(identity(o));animated+=1
   if key in wanted or semantic_owners or animation_owners:
    facts['/'.join(key)]=dict(sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
     scenario_bits_set=sorted(flags),scenario_bits_reset=sorted(resets),local_indices_written=sorted(locals_),map_destinations=sorted(destinations),choice_count=selects,named_calls=sorted(named))
   if any(p.search(name) for name in named for p in MATCHERS.values()):
    special.append(dict(script='/'.join(key),maps=aliases.get(key[0],[]),
      entities=sorted({o['props'].get('entity_id',-1) for o in owners}),calls=sorted(named)))
  if animation_pass:
   # Only newly discovered identities belong to this supplement. A fresh
   # target-reference pass can recognize completed guides without rewriting
   # the baseline's earlier source-matched counts or triage record.
   candidates=[row for row in candidates if (row['submap'],row['group'],row['entity'],row['raw']) in animation_keys]
   selected={r['submap']+'/'+r['script'] for r in candidates}
   facts={key:value for key,value in facts.items() if key in selected}
  counts=dict(candidates=len(candidates),rooms=len({r['submap'] for r in candidates}),
    referenced=sum(bool(r['guide_references']) for r in candidates),named_calls=len(calls),special_call_scripts=len(special),
    unavailable_scripts=len(unavailable),unavailable_candidate_scripts=sum(f.get('available') is False for f in facts.values()),
    additional_semantic_candidates=semantic,additional_animation_candidates=animated)
  result['games'][game]=dict(source_sha256={k:digest(p) for k,p in paths.items()},counts=counts,candidates=candidates,script_facts=facts,
    named_calls={k:dict(uses=len(v),example_locators=v[:4]) for k,v in sorted(calls.items())},special_call_scripts=special,unavailable_scripts=unavailable)
  print(game,json.dumps(counts),flush=True)
 out=ROOT/('docs/re/puzzle_mechanics_animation_inventory_20261031.json' if animation_pass else 'docs/re/puzzle_mechanics_inventory_20261031.json')
 out.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Factual inventory written; no native bodies/operand streams, no completion claim')

if __name__=='__main__':main()
