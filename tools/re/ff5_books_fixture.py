"""External comparison with owned exit markers; native PCs remain unchanged."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
import bake_map_state as compiler
from ff5_stone_fixture import updates
def main():
 folder=ROOT.parent/'tools/ff5_surgate_20261102';assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 sources=json.loads((folder/'programs.json').read_text(encoding='utf-8'));pickups={'sc_e_0241','sc_e_0383','sc_e_0385'};shelves={'sc_e_0382','sc_e_0384','sc_e_0386'};wrong={f'sc_e_0387_{i}'for i in range(2,9)}
 wanted=pickups|shelves|wrong|{'sc_map_20171_6','sc_map_20171_7','sc_e_0387_1','sc_e_0387_9'};rows=[]
 for source in sources:
  if source['name']not in wanted:continue
  name=source['name'];main=next(s for s in source['segments']if s['Label']=='Main');assert main['EntryPoint']==0;count=main['Count'];commands=source['mnemonics'];assert not updates(commands[count:]),'Unreviewed helper flag write'
  exits=[pc for pc,m in enumerate(commands[:count])if m['mnemonic']=='Exit'];outcomes={str(pc):[]for pc in exits}
  if name in pickups:assert count==10 and exits==[6,9];outcomes['6']=updates(commands[:6]);assert len(outcomes['6'])==1
  elif name in shelves:assert count==26 and exits==[5,10,15,20,25];outcomes['15']=updates(commands[11:15]);outcomes['25']=updates(commands[21:25]);assert len(outcomes['15'])==len(outcomes['25'])==2
  elif name in wrong:assert count==17 and exits==[5,10,16];outcomes['16']=updates(commands[11:16]);assert len(outcomes['16'])==3
  elif name=='sc_map_20171_7':assert count==33 and exits==[26,32];outcomes['26']=updates(commands[20:26]);assert len(outcomes['26'])==6
  else:assert not updates(commands),'Unqualified flag writes'
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
 else:assert not path.exists(),'Reuse archived comparison with --check';path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Surgate:',len(rows),'external comparisons; source-qualified actual-exit flag outcomes, original PCs retained. No native execution or source output.')
if __name__=='__main__':main()
