"""Source-qualified surroundings of the accepted Dream Train guide.

Entry uses native active-party requirements and the south bed approach. Real
join receipts, rather than sightings, unlock the brothers. The existing train
owns522..527 unchanged. Doma guidance ends at the actual post-waking pickup.
"""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
PARTY='Bring Cyan in a full four-character active party.'
REST='Move one step north into the bed, then choose the first option to rest.'
POSSESSION='Fight the boss. When it vanishes, knock out and revive allies one at a time until it returns. Keep one ally alive. Attack the boss when it returns; repeat if needed.'
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def author(o):
 import bake_maps
 proof=json.loads((ROOT/'docs/re/dream_native_20261104.json').read_text(encoding='utf-8'))
 for locator,wanted in proof['source_script_hashes'].items():
  room,name=locator.split('/');assert sha(o.SCRIPTS[room,name]['mnemonics'])==wanted,locator
 c,f,r,t=o.condition,o.flag,o.rule,o.target
 def named(map_id,entity,text,group,script):
  number,=[i for i,name in o.SCRIPT_NAMES.items()if name==script]
  return t(map_id,entity,text,group,number)
 actor=o.physical(300,8,'ev_e_0895');room=actor['submap']
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[room]
 ox,oy,*_=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']==room])
 bed=dict(map=300,x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy+1,layer=bake_maps.layer_index(actor['layer']),kind='spot',label='Passage',text=REST)
 reunion=[dict(when=[c('map',0,300),f(435),c('cyan',0,0)],note=PARTY),
          dict(when=[c('map',0,300),f(435),c('members',0,4,'ne')],note=PARTY),
          r([c('map',0,300),f(435)],bed),
          r([f(436)],*[t(520,i,'Continue to {0}, then request guidance again.','ev_e_0896')for i in(56,57)]),
          r([f(437)],t(520,55,'Continue to {0}, then request guidance again.','ev_e_0897')),
          r([f(438)],*[t(520,i,'Continue to {0}, then request guidance again.','ev_e_0898')for i in(52,53)]),
          r([f(439)],t(520,51,'Continue to {0}, then request guidance again.','ev_e_0899')),
          r([f(440)],t(521,8,'Defeat {0}. Contact starts this encounter; request guidance again after battle.','ev_e_0901')),
          r([],t(521,7,'Continue through the passage.','ev_e_0902'))]
 #529 is also part of the initial looping route to the real0934 marker.
 #Merely entering it through a normal door does not set450 or reach point3.
 armor=[r([f(450)],named(528,0,'Continue through the passage.','entity_default','sc_e_0934')),
        r([f(450,1)],t(529,9,'Continue through the passage.','ev_e_0935'))]
 #452/453 are optional memories. They never gate the real boss in native code.
 #454 is the family room scene, not an invented lock on537. The guide visits
 #it for the complete story, while ordinary routes may take the direct path.
 doma=[r([f(455),f(454)],named(532,0,'Continue through the passage.','entity_default','sc_e_0938')),
       r([f(455)],*[t(537,i,POSSESSION,'ev_e_0939')for i in range(2,9)]),
       r([f(455,1)],*[t(304,i,'Continue to {0}, then request guidance again.','ev_e_0940')for i in(1,5)])]
 for rules in(reunion,armor,doma):
  for row in rules:
   for goal in row.get('targets',[]):goal.update(avoid_other_events=True,allow_scripted_crossings=True,allow_world_transit=False)
 reunion[-1]['finish_on_scenario']=441;armor[-1]['finish_on_scenario']=451;doma[-1]['finish_on_scenario']=456
 guides=[]
 for name,maps,rules,select,complete in (
 ('dream_reunion',[300,520,521],reunion,[f(365,1),f(455)],441),
 ('dream_armor',[528,529],armor,[f(449,1),f(451)],451),
 ('dream_doma',[*range(530,539),*range(296,305)],doma,[f(451,1)],456)):
  guide=o.guide(name,maps,'',rules);guide.update(first_match=True,select_when=select,complete_scenario=complete,finish_on_completion=True,source_script_hashes=proof['source_script_hashes'],evidence='docs/re/DREAM_EPISODE_20261104.md');guides.append(guide)
 return guides
def state(o,room='Map_20451_4'):
 import bake_map_state,bake_maps
 source=o.SCRIPTS[room,'sc_map_'+room.removeprefix('Map_')];info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[source['submap']]
 bounds=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']==source['submap']])
 return bake_map_state.initializer_code(source,'ff6',bounds)
def arrival(o):
 import bake_maps
 actor=o.physical(300,8,'ev_e_0895');body=o.SCRIPTS['Map_30821_1','sc_e_0895_1'];ms=body['mnemonics']
 assert ms[15]['mnemonic']=='SetPuppet'and ms[15]['operands']['iValues'][1:3]==[-1001,19]
 assert ms[20]['mnemonic']=='SetPos'and ms[20]['operands']['iValues'][:2]==[22,54]
 assert o.SCRIPTS['Map_20451_4','sc_e_0895']['mnemonics'][1]['operands']['iValues'][:4]==[2,60,60,60]
 info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))['Map_30821_1'];ox,oy,*_=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']=='Map_30821_1'])
 return dict(submap=actor['submap'],entity_id=8,object_id=actor['id'],destination_map=520,original_point=1,continuation='sc_e_0895_1',continuation_sha256=sha(body),arrival_cell=[22,54,1],arrival_baked_cell=[22-ox,54-oy,1],approach_mask=4,evidence='Native northward bed entry/first rest option and full0895_1 player SetPos22,54;not ordinary door PointIn1. docs/re/DREAM_EPISODE_20261104.md')
if __name__=='__main__':
 import author_ff6_story_guides as o
 check='--check'in sys.argv;guides=author(o);p=ROOT/'data/ff6/puzzles.json';data=json.loads(p.read_text(encoding='utf-8'))
 for guide in guides:
  if check:assert[g for g in data['story_guides']if g['id']==guide['id']]==[guide]
  else:data['story_guides']=[g for g in data['story_guides']if g['id']!=guide['id']]+[guide]
 if not check:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/mapstate.json';data=json.loads(p.read_text(encoding='utf-8'))
 for room in ('Map_20451_4','Map_30821_1','Map_30821_2'):
  wanted=state(o,room);old=data[room]['code']
  if check:assert old==wanted,room
  else:
   assert len(old)==len(wanted)
   for a,b in zip(old,wanted):assert a==b or a==[9]and b[0]in(0,2,10), (room,a,b)
   data[room]['code']=wanted
 # A qualified destination cannot be certified from unknown party data.
 # This guard projects the exact native entrance initializer, retaining only
 # its ev0895 branch. The ordinary-rest branch still owns its separate actor1.
 import bake_maps
 actor=o.physical(300,8,'ev_e_0895');room=actor['submap'];info=json.loads((o.DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[room]
 ox,oy,*_=bake_maps.map_bounds(info,[a for a in o.OBJECTS if a['submap']==room])
 guard=dict(entry=0,code=[[0]if row==[1,'ev_e_0894']else row for row in state(o)],require_known=True,id=8,type=actor['type'],group=actor['group'],raw=actor['name'],cell=[bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy],source='sc_map_20451_4',map_ids=[300],area_id=data[room]['area_id'])
 old=[g for g in data[room].get('events',[])if g.get('source')=='sc_map_20451_4']
 if check:assert old==[guard]
 else:
  assert not old or old==[guard]
  if not old:data[room].setdefault('events',[]).append(guard)
  p.write_text(json.dumps(data,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 p=ROOT/'data/ff6/warp_arrival_overrides.json';data=json.loads(p.read_text(encoding='utf-8'));wanted=arrival(o);old=[row for row in data if row['submap']==wanted['submap']and row.get('object_id')==wanted['object_id']]
 if check:assert old==[wanted]
 else:
  if old:
   assert old==[wanted]or old==[{**wanted,'approach_mask':1}]
   data[data.index(old[0])]=wanted
  else:data.append(wanted)
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Dream:exact active Cyan/four-member entry,real437/439 joins,brothers/train handoff,armor passage,family/boss possession and actual456 pickup. Prior train unchanged.')
