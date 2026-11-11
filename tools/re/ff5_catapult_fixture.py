"""Current native Catapult comparison programs, stored outside every Git tree."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
import bake_map_state as compiler
def main():
 folder=ROOT.parent/'tools/ff5_catapult_20261101'
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 wanted={'sc_map_20231_4','sc_e_0224','sc_e_0224_1','sc_e_0224_2','sc_e_0224_3','sc_e_0224_4','sc_e_0224_5','sc_e_0432'};rows=[]
 for line in (ROOT.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8'):
  source=json.loads(line)
  if source['submap']!='Map_20231_4'or source['name']not in wanted:continue
  def record(direction=0,choice=0):
   code=compiler.initializer_code(source,'ff5',(0,0))
   if source['name']=='sc_e_0432':
    # Comparison only: exact Current.ChangeStateToEvent2B5700 changes the
    # main-game UI/input phase to Event12, not map/actor/progress state. Its
    # body is separately pinned; the shipped general projector stays opaque.
    assert source['mnemonics'][1]['mnemonic']=='SysCall'and source['mnemonics'][1]['operands']['sValues'][0]=='UIを非表示にする'
    code[1]=[0]
   for pc,m in enumerate(source['mnemonics']):
    iv=m['operands']['iValues'];sv=m['operands']['sValues']
    if m['mnemonic']=='BranchByPlayerDirection':code[pc]=[3,iv[direction]]
    if m['mnemonic']=='Branch'and sv[0]=='MessageSelection':code[pc][1:3]=[-1,choice]
    if m['mnemonic']=='SetPuppet'and iv[1]>0:
     pose=compiler.puppet_navigation_effect(source,iv[2],(0,0),True)
     if pose and any(n!=compiler.UNSET_POSITION for n in pose[1:]):code[pc]=[10,iv[0],iv[1],*pose]
   if source['name']!='sc_e_0432':assert not any(op[0]==10 and op[2]>0 and op[3]==-2 for op in code),'Unresolved positive native actor outcome'
   return dict(entry=0,code=code,map_ids=[293])
  row=dict(name=source['name'],sha256=hashlib.sha256(json.dumps(source['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),record=record(),no=record(choice=1))
  row['flag_updates']=[dict(index=m['operands']['iValues'][0]+256*(int(m['operands']['sValues'][0][-1])-1),value=int(m['mnemonic']=='SetFlag'))for m in source['mnemonics']if m['mnemonic']in('SetFlag','ResetFlag')and m['operands']['sValues'][0]in('ScenarioFlag1','ScenarioFlag2','ScenarioFlag3','ScenarioFlag4')]
  if source['name']=='sc_e_0432':
   assert source['mnemonics'][2]['mnemonic']=='BranchByPlayerDirection'and source['mnemonics'][2]['operands']['iValues'][:4]==[86,86,3,86]
   assert source['mnemonics'][6]['mnemonic']=='Branch'and source['mnemonics'][6]['operands']['sValues'][:3]==['MessageSelection','＝','imm']and source['mnemonics'][6]['operands']['iValues'][:3]==[1,0,8]
   assert source['mnemonics'][7]['mnemonic']=='Exit';row['record']=row['no']=record(2,1);assert not row['flag_updates']
  print(source['name'],'receipt assignments',row['flag_updates'],'branch producers',sorted({m['operands']['sValues'][0]for m in source['mnemonics']if m['mnemonic']=='Branch'}))
  if source['name']=='sc_e_0224_4':
   branches=[m for m in source['mnemonics']if m['mnemonic']=='BranchByPlayerDirection'];assert len(branches)==1
   row['directions']={str(d):record(d)for d in range(4)}
   successful=[]
   for d,pc in enumerate(branches[0]['operands']['iValues'][:4]):
    for m in source['mnemonics'][pc:]:
     if m['mnemonic']=='Exit':break
     if m['mnemonic']=='SetFlag'and m['operands']['sValues'][0]=='ScenarioFlag2'and m['operands']['iValues'][0]==56:successful.append(d);break
   assert successful==[0,2];row['successful_directions']=successful
  rows.append(row)
 assert {r['name']for r in rows}==wanted
 path=folder/'puzzle_programs.json'
 if '--check'in sys.argv:assert json.loads(path.read_text(encoding='utf-8'))==rows,'Catapult source comparison changed'
 else:path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Eight current native Catapult programs prepared externally; actual rest-No entry, all four book-facing paths and both final-choice paths retained. Used actor outcomes resolved; no native source in Git.')
if __name__=='__main__':main()
