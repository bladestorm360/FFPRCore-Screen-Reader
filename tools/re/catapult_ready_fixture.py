"""Real Catapult before/after compiler comparison, source-bearing output external."""
from pathlib import Path
import hashlib,json,subprocess,sys,types
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
import bake_map_state as current
def main():
 out=ROOT.parent/'tools/ff5_catapult_20261101'
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 previous=ROOT/'build_check/puppet_ready_integration_20261101/before/tools/bake_map_state.py'
 old=types.ModuleType('retained_before_ready');old.__file__=str(ROOT/'tools/bake_map_state.py')
 exec(compile(previous.read_text(encoding='utf-8'),str(previous),'exec'),old.__dict__)
 wanted={'sc_map_20231_4','sc_e_0224','sc_e_0224_1','sc_e_0224_2','sc_e_0224_3'};rows=[]
 for line in (ROOT.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8'):
  source=json.loads(line)
  if source['submap']!='Map_20231_4'or source['name']not in wanted:continue
  cases={}
  for name,module in(('before',old),('after',current)):
   code=module.initializer_code(source,'ff5',(0,0))
   for pc,m in enumerate(source['mnemonics']):
    # The actual native prompt's affirmative response is selection0. Keep
    # unrelated script branches and every original PC/call target intact.
    if m['mnemonic']=='Branch'and m['operands']['sValues'][0]=='MessageSelection':code[pc][1:3]=[-1,0]
    if m['mnemonic']=='SetPuppet'and m['operands']['iValues'][1]==23:
     effect=module.puppet_navigation_effect(source,m['operands']['iValues'][2],(0,0),True)
     if effect and effect[1:]==[10,14]:code[pc]=[10,m['operands']['iValues'][0],23,*effect]
   cases[name]=dict(entry=0,code=code,map_ids=[293])
  rows.append(dict(name=source['name'],sha256=hashlib.sha256(json.dumps(source['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),**cases))
 assert {r['name']for r in rows}==wanted
 (out/'ready_programs.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Five exact Catapult lifecycle comparison programs prepared externally, before/after; real affirmative choice and unchanged-layer book pose. No native source in Git.')
if __name__=='__main__':main()
