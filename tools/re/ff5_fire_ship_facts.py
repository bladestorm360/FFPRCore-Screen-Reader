"""Bounded offline native-script semantics;recovered inputs stay outside Git.

Only deterministic flag/puppet operations used by these exact ship controls are
accepted. This tool never runs in the game or ships as a runtime interpreter.
"""
from pathlib import Path
import copy,hashlib,json,sys
R=Path(__file__).resolve().parents[2];D=R.parent/'tools/mapdump'
E=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_FireShip_Context'
ROOM='Map_30151_20'
COSMETIC={'Nop','Wait','Shake','PlaySE','StopSE','PlayBGM','StopEnvSe','CueAnim','PlayAnim','SyncSE','Blank'}
class Machine:
 def __init__(self,objects,mask,direction=0,player=(10,21)):
  self.mask=mask;self.direction=direction;self.work=0;self.slots={};self.groups={'entity_default'}
  self.actors={o['props']['entity_id']:dict(visible=o['group']=='entity_default',x=o['tx'],y=o['ty']-(1 if o.get('gid')is not None else 0),layer={'BottomObject':0,'GroundObject':1,'UpperObject':2}.get(o['layer'],1))for o in objects if o['props'].get('entity_id',0)>0}
  for id in(-1001,0):self.actors[id]=dict(visible=True,x=player[0],y=player[1],layer=1)
  self.objects=objects
 def tick(self):
  self.work+=1;assert self.work<=20000,'Native episode bound exceeded'
 def puppet(self,body,target,pc):
  a=self.actors[target];a['visible']=True # source-qualified PuppetCore.Ready
  while True:
   self.tick();m=body[pc];op=m['mnemonic'];v=m.get('operands')or{};iv=list(v.get('iValues')or[])+[0]*8
   if op in('Exit','Return'):return
   if op=='Hide':a['visible']=False
   elif op=='Show':a['visible']=True
   elif op=='SetPos':
    assert iv[3:8]==[0]*5
    if iv[0]>=0:a['x']=iv[0]
    if iv[1]>=0:a['y']=iv[1]
    if iv[2]>0:assert iv[2]<=3;a['layer']=iv[2]-1
   else:assert op in COSMETIC,('Unsupported puppet operation',op,pc)
   pc+=1
 def execute(self,body):
  self.work=0;pc=0;stack=[]
  while True:
   self.tick();assert 0<=pc<len(body);m=body[pc];op=m['mnemonic'];v=m.get('operands')or{};iv=list(v.get('iValues')or[])+[0]*8;sv=list(v.get('sValues')or[])+['']*8
   if op=='Exit':assert not stack;return self
   if op=='Return':assert stack;pc=stack.pop();continue
   if op=='Call':assert len(stack)<32;stack.append(pc+1);pc=iv[0];continue
   if op=='Jump':pc=iv[0];continue
   if op=='Branch':
    assert sv[:3]==['ScenarioFlag2','＝','imm']and 45<=iv[0]<=51
    if(self.mask>>(iv[0]-45))&1==iv[1]:pc=iv[2];continue
   elif op=='BranchByPlayerDirection':pc=iv[self.direction];continue
   elif op in('SetFlag','ResetFlag'):
    assert sv[0]=='ScenarioFlag2'and 45<=iv[0]<=51;bit=1<<(iv[0]-45);self.mask=(self.mask|bit)if op=='SetFlag'else(self.mask&~bit)
   elif op=='SetEntities':
    self.groups.add(sv[0])
    for o in self.objects:
     if o['group']==sv[0]and o['props'].get('entity_id',0)>0:self.actors[o['props']['entity_id']]['visible']=True
   elif op=='SetPuppet':assert 0<=iv[0]<32 and iv[1]in self.actors;self.slots[iv[0]]=(iv[1],iv[2])
   elif op=='ExecPuppet':
    for slot,(target,entry)in sorted(self.slots.items()):self.puppet(body,target,entry)
    self.slots.clear() # completed native cores sleep until another SetPuppet
   else:assert op in COSMETIC,('Unsupported Main operation',op,pc)
   pc+=1
def load():
 proof=json.loads((R/'docs/re/ff5_fire_ship_native_20261105.json').read_text(encoding='utf-8'));sources={};objects=[]
 for line in(E/'scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line)
  if s['submap']!=ROOM:continue
  assert hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][ROOM+'/'+s['name']];sources[s['name']]=s['mnemonics']
 for line in(D/'ff5_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']==ROOM:objects.append(o)
 return proof,sources,objects
def collect():
 proof,sources,objects=load();physical={o['props']['entity_id']for o in objects if o['props'].get('collision')and o['type_name']=='Entity'};assert set(range(19,27))<=physical
 initial=[Machine(objects,mask).execute(sources['sc_map_30151_20'])for mask in range(128)];transitions=[];mismatches=[];peak=0;player_modes=set()
 for mask,base in enumerate(initial):
  for control in range(1,9):
   for direction in range(4):
    episode=copy.deepcopy(base);episode.direction=direction;episode.execute(sources['sc_e_0218_'+str(control)]);peak=max(peak,episode.work);expected=initial[episode.mask]
    different=[id for id in physical if episode.actors[id]!=expected.actors[id]]
    if different:mismatches.append([mask,control,direction,episode.mask,different])
    moved=episode.actors[-1001]!=base.actors[-1001];player=episode.actors[-1001];leader=episode.actors[0];assert[player[k]for k in('x','y','layer')]==[leader[k]for k in('x','y','layer')]
    end=[player[k]for k in('x','y','layer')]if moved else None
    if end:player_modes.add((control,direction,mask,episode.mask,*end))
    transitions.append(dict(mask=mask,control=control,direction=direction,after=episode.mask,player=end))
 facts=dict(episodes=len(transitions),physical_actors=sorted(physical),peak_operations=peak,reload_mismatches=mismatches,transitions=transitions,layouts=[{str(id):s.actors[id]for id in physical}for s in initial])
 if '--check'in sys.argv:assert json.loads((E/'physical_facts.json').read_text(encoding='utf-8'))==facts
 else:(E/'physical_facts.json').write_text(json.dumps(facts,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Exact ship native semantics:',len(transitions),'episodes;',len(physical),'solid actors;',peak,'peak operations;',len(mismatches),'stable/reload differences.')
 print('Player move modes',sorted({(c,d,x,y,l)for c,d,m,a,x,y,l in player_modes}));print('First differences',mismatches[:12])
 return facts
if __name__=='__main__':collect()
