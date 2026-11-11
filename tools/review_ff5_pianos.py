"""Complete piano input/proficiency review;no original game program is stored."""
from pathlib import Path
import collections,hashlib,json,sys
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
ROOTS=('sc_e_0174','sc_e_0177','sc_e_0189','sc_e_0191','sc_e_0197','sc_e_0229','sc_e_0244','sc_e_0279')
PRESENTATION={'Nop','Msg','MsgClose','Wait','PlayAnim','Move','MoveTo','Lerp','LookAt','Hide','Show','PlaySE','StopSE','PlayBGM','StopBGM','SyncBGM','PauseBGM','SetPuppet','ExecPuppet','SetEntities','Return','Exit','Call','SetFlag','Select','Branch'}
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def episode(body,mask,choice):
 pc=0;stack=[];writes=[];groups=[];work=0;choices=0
 while True:
  work+=1;assert work<=2048;row=body[pc];op=row['mnemonic'];v=row.get('operands')or{};iv=list(v.get('iValues')or[])+[0]*8;sv=list(v.get('sValues')or[])+['']*8
  if op=='Exit':assert not stack;return writes,groups,choices,work
  if op=='Call':assert len(stack)<16;stack.append(pc+1);pc=iv[0];continue
  if op=='Return':assert stack;pc=stack.pop();continue
  if op=='Select':choices+=1;assert iv[0]==2
  elif op=='Branch':
   assert sv[1:3]==['＝','imm']
   if sv[0]=='ScenarioFlag1':assert 212<=iv[0]<=219;value=(mask>>(iv[0]-212))&1
   else:assert sv[0]=='MessageSelection'and iv[0]==1;value=choice
   if value==iv[1]:pc=iv[2];continue
  elif op=='SetFlag':assert sv[0]in('ScenarioFlag1','ScenarioFlag2');writes.append(iv[0]+(256 if sv[0]=='ScenarioFlag2'else 0))
  elif op=='SetEntities':groups.append(sv[0])
  else:assert op in PRESENTATION
  pc+=1
def main():
 check='--check'in sys.argv;proof=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in proof['candidates']if r['script'].removesuffix('_1')in ROOTS or r['submap']=='Map_20101_2'and not r['script']];assert len(selected)==18
 rooms={r['submap']for r in selected};wanted={(r['submap'],r['script'])for r in selected if r['script']};wanted|={(room,'sc_map_'+room[4:])for room in rooms};wanted|={('Map_20101_2','sc_e_0198'),('Map_20101_2','sc_20101_2_to_20100_1')};bodies={};hashes={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap'],s['name']
  if key in wanted:bodies[key]=s['mnemonics'];hashes['/'.join(key)]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert set(bodies)==wanted and len(bodies)==26
 episodes=0;peak=0;facts=[]
 for (room,name),body in bodies.items():
  if name not in ROOTS and not(name.endswith('_1')and name[:-2]in ROOTS):continue
  assert all(m['mnemonic']in PRESENTATION for m in body),(name,collections.Counter(m['mnemonic']for m in body))
  assert sum(m['mnemonic']=='Select'for m in body)==1 and sum(m['mnemonic']=='Branch'for m in body)==9
  replay=name.endswith('_1');base=name.removesuffix('_1');town=220+ROOTS.index(base)
  for mask in range(256):
   for choice in range(2):
    writes,groups,count,work=episode(body,mask,choice);episodes+=1;peak=max(peak,work);assert count==1
    if choice==1:assert writes==[]
    elif replay:
     highest=max((i for i in range(8)if mask&(1<<i)),default=None);expected=[]if base!='sc_e_0197'or highest is None else[329 if highest==7 else 328];assert writes==expected
    else:
     missing=next((i for i in range(8)if not mask&(1<<i)),None);expected=[]if missing is None else[212+missing,town]+([329 if missing==7 else 328]if base=='sc_e_0197'else[]);assert writes==expected
  initializer=bodies[room,'sc_map_'+room[4:]];active={m['operands']['sValues'][0]for m in initializer if m['mnemonic']=='SetEntities'};assert 'ev_e_'+base[5:]in active and 'ev_e_'+base[5:]+'_1'in active
  facts.append(dict(room=room,script=name,operations=len(body),replay=replay,town_receipt=town))
 assert episodes==8192
 crescent=bodies['Map_20101_2','sc_map_20101_2'];assert not any(m['mnemonic']=='SetEntities'and m['operands']['sValues'][0]=='ev_e_0191'for m in crescent)
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==proof['source_sha256'][role]
 bard=bodies['Map_20101_2','sc_e_0198'];assert len(bard)==52 and {m['mnemonic']for m in bard}=={'Nop','Branch','Msg','MsgFunfare','GetItem','SetFlag','Exit'}and {m['operands']['iValues'][0]for m in bard if m['mnemonic']=='GetItem'}=={461,463,465}
 transition=bodies['Map_20101_2','sc_20101_2_to_20100_1'];assert[m['mnemonic']for m in transition]==['Nop','ResetFlag','ResetFlag','ChangeMap','Exit']and [transition[i]['operands']['iValues'][0]for i in(1,2)]==[72,73]
 manifest=dict(identities=18,locations=8,complete_scripts=26,source_script_hashes=hashes,episodes=episodes,peak_main_operations=peak,performance_facts=facts,crescent=dict(performance_receipts=[328,329],exit_resets=[328,329],late_rewards_require=125,song_items=[461,463,465]),source_sha256=proof['source_sha256'])
 p=R/'docs/re/ff5_pianos_native_20261105.json'
 if check:assert read(p)==manifest
 else:p.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  sha=hashes.get(r['submap']+'/'+r['script']);expected=proof['source_script_hashes'].get(r['submap']+'/'+r['script']);assert sha==expected
  reason='One ordinary Play/Decline confirmation is followed by a completely automatic native performance. Each fresh town piano raises the first missing proficiency212–219 and records its own220–227 receipt;replay never raises proficiency. All8192 native flag/choice cases pass. There is no manual note sequence,timed input,incorrect-answer route,map unlock or required puzzle order.'if r['script']else'Scriptless Crescent ev_e_0191 is not activated by its native initializer. Actual first/replay groups are ev_e_0197/0197_1;the placeholder is not a missing interactive puzzle control.'
  if r['script'].removesuffix('_1')=='sc_e_0197':reason+=' Crescent also records328/329 for its ordinary bard reward;leaving clears those two local performance receipts,and later songs retain native story125. The complete bard body only grants song items/conversation receipts;it has no separate puzzle input or map unlock.'
  authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=sha,disposition='ordinary optional automatic music interaction'if r['script']else'inactive scriptless music placeholder',guides=[],reason=reason,evidence='docs/re/FF5_PIANOS_20261105.md;26 complete source bodies/eight locations/8192 actual-choice/proficiency traces;immutable native identities/hashes.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert old==new
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print(f'Pianos:18 exact identities/eight locations/26 complete bodies/{episodes} proficiency-choice episodes;{peak} peak main operations;ordinary automatic interactions,no game or production change.')
if __name__=='__main__':main()
