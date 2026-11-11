"""Prepare source-bearing native navigation programs outside every Git tree.

Only original tool code belongs here. The external programs are offline primary
comparison inputs, never game resources or committed fixtures.
"""
from pathlib import Path
import csv,hashlib,json,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
import bake_map_state
def main():
 target=ROOT.parent/'tools/ff5_library_20261101';target.mkdir(exist_ok=True)
 check=subprocess.run(['git','-C',str(target),'rev-parse','--show-toplevel'],capture_output=True)
 assert check.returncode!=0,'Native analysis output must be outside every Git tree'
 wanted={'sc_map_20221_4','sc_map_20221_10'}|{f'sc_e_0220{suffix}' for suffix in ('','_1','_2','_3','_4','_5','_6','_7','_8')}|{'sc_e_0223','sc_e_0223_1','sc_e_0223_2'}
 maps={r['map_name']:int(r['id']) for r in csv.DictReader((ROOT.parent/'tools/mapdump/ff5_master/map.txt').open(encoding='utf-8-sig'))}
 rows=[];found=set()
 for line in (ROOT.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if not r['submap'].startswith('Map_20221_'):continue
  found.add(r['name'])
  code=bake_map_state.initializer_code(r,'ff5',(0,0))
  # Narrow Library exception, not a change to the global non-VI projector.
  # Exact FFV SetPos2DBD20 normalizes absolute integer cells at accessor111,
  # then MoveEntityLayer2D7C80 converts them through accessor3. These three
  # unchanged-layer placements equal the baked actor's existing logical cell.
  # Native helper bodies/provenance stay external in this same analysis folder.
  # Position itself does not show an actor; the separately verified native
  # PuppetCore.Ready does so before each body. Keep the same spatial contract.
  proven={('sc_e_0220_1',22):[1,8,7],('sc_e_0220_8',24):[1,13,7],('sc_e_0220_8',25):[1,13,8]}
  for pc,m in enumerate(r['mnemonics']):
   if m['mnemonic']!='SetPuppet':continue
   iv=m['operands']['iValues'];expected=proven.get((r['name'],iv[1]))
   if expected:
    effect=bake_map_state.puppet_navigation_effect(r,iv[2],(0,0),True);assert effect==expected,(r['name'],iv[1],effect)
    code[pc]=[10,iv[0],iv[1],*effect]
  # The blocking-book comparison retains its proven pre-encounter effects;
  # the separate native continuation supplies its post-victory Hide receipt.
  prefix=[list(op) for op in code]
  for pc,m in enumerate(r['mnemonics']):
   if m['mnemonic'] in ('Encount','EncountBoss'):
    prefix[pc]=[6];break
  rows.append(dict(name=r['name'],map=maps[r['submap']],
   sha256=hashlib.sha256(json.dumps(r['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
   record=dict(code=code,map_ids=[maps[r['submap']]],entry=0),prefix_record=dict(code=prefix,map_ids=[maps[r['submap']]],entry=0),
   flag_updates=[dict(index=m['operands']['iValues'][0]+256*(int(m['operands']['sValues'][0][-1])-1),value=int(m['mnemonic']=='SetFlag')) for m in r['mnemonics']
    if m['mnemonic'] in ('SetFlag','ResetFlag') and m['operands']['sValues'][0] in ('ScenarioFlag1','ScenarioFlag2','ScenarioFlag3','ScenarioFlag4')]))
 assert wanted<=found and len(rows)<256,(len(rows),wanted-found)
 for script,receipt in (('sc_e_0049_1',48),('sc_e_0050_1',49),('sc_e_0051_2',50),('sc_e_0430',163)):
  row=next(r for r in rows if r['name']==script)
  assert any(f['index']==receipt and f['value']==1 for f in row['flag_updates']),(script,receipt)
 path=target/'native_programs.json'
 if path.exists() and not (target/'native_programs_initial14.json').exists():shutil.copy2(path,target/'native_programs_initial14.json')
 path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Prepared',len(rows),'exact native navigation comparison programs externally; no native code written inside Git.')
if __name__=='__main__':main()
