"""External native comparison, instrumented only with owned exit receipts.

Appending trace exits keeps every original PC/branch/puppet target unchanged.
Actual native control flow chooses which source-qualified flag writes completed.
No native body or comparison bytecode is written into the repository.
"""
from pathlib import Path
import csv,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
import bake_map_state as compiler
def updates(commands):
 return[dict(index=m['operands']['iValues'][0]+256*(int(m['operands']['sValues'][0][-1])-1),value=int(m['mnemonic']=='SetFlag'))for m in commands if m['mnemonic']in('SetFlag','ResetFlag')and m['operands']['sValues'][0]in('ScenarioFlag1','ScenarioFlag2','ScenarioFlag3','ScenarioFlag4')]
def main():
 folder=ROOT.parent/'tools/ff5_stone_20261102';assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 sources=json.loads((folder/'programs.json').read_text(encoding='utf-8'));wanted={'sc_map_30381_2','sc_map_30381_3','sc_map_30381_4','sc_e_0254','sc_e_0255','sc_e_0252_7','sc_e_0252_9'}|{f'sc_e_0252_{i}'for i in range(1,6)};rows=[]
 for source in sources:
  if source['name']not in wanted:continue
  name=source['name'];main=next(s for s in source['segments']if s['Label']=='Main');assert main['EntryPoint']==0;count=main['Count'];commands=source['mnemonics'];assert not updates(commands[count:]),'Unreviewed helper flag write'
  exits=[pc for pc,m in enumerate(commands[:count])if m['mnemonic']=='Exit'];outcomes={str(pc):[]for pc in exits}
  if name in{f'sc_e_0252_{i}'for i in range(1,6)}:
   assert count==37 and exits==[6,17,21,26,36];assert commands[1]['mnemonic']=='Branch'and commands[1]['operands']['iValues'][2]==18
   assert not any(m['mnemonic']in('Branch','Jump','ChangeScript','ChangeMap')for m in commands[7:17]+commands[27:36])
   outcomes['17']=updates(commands[7:17]);outcomes['36']=updates(commands[27:36]);assert len(outcomes['17'])==len(outcomes['36'])==2
  elif updates(commands[:count]):
   assert len(exits)==1 and not any(m['mnemonic']in('Branch','Jump','ChangeScript','ChangeMap')for m in commands[:count]);outcomes[str(exits[0])]=updates(commands[:count])
  def record(choice):
   code=compiler.initializer_code(source,'ff5',(0,0))
   for pc,m in enumerate(commands):
    if m['mnemonic']=='Branch'and m['operands']['sValues'][0]=='MessageSelection':code[pc][1:3]=[-1,choice]
   for pc in exits:
    end=len(code);code.extend([[1,'mod_fixture_exit_'+str(pc)],[6]]);code[pc]=[3,end]
   return dict(entry=0,code=code)
  rows.append(dict(name=name,sha256=hashlib.sha256(json.dumps(commands,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),record=record(0),no=record(1),outcomes=outcomes))
 assert{r['name']for r in rows}==wanted
 path=folder/'comparison_programs.json'
 if '--check'in sys.argv:assert json.loads(path.read_text(encoding='utf-8'))==rows,'Native comparison changed'
 else:
  assert not path.exists(),'Reuse existing native comparison; use --check'
  path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Ghido:',len(rows),'external source-qualified comparison programs; original PCs retained, actual exits select native flag outcomes. No game execution/source output.')
if __name__=='__main__':main()
