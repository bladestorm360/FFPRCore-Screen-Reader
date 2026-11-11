"""Verify the six exact puppet Ready producers; native bodies stay external.

Existing reviewed Hide/Show/entry archives are reused. FFV Ready was just read
for the Catapult frontier and is not exported again. No native method is called.
"""
from pathlib import Path
import hashlib,json,re,subprocess,sys
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[2]
NAMES=('FINAL FANTASY PR','FINAL FANTASY II PR','FINAL FANTASY III PR','FINAL FANTASY IV PR','FINAL FANTASY V PR','FINAL FANTASY VI PR')
def sha(p):
 h=hashlib.sha256()
 with p.open('rb')as f:
  for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
 return h.hexdigest()
def main():
 if '--check'in sys.argv:
  prior=json.loads((ROOT/'docs/re/puppet_ready_native_20261101.json').read_text())
  assert len(prior['methods'])==6
  for i,row in enumerate(prior['methods'],1):
   assert row['game']==f'ff{i}'and row['method']=='PuppetCore.Ready'
   dll=Path('D:/Games/SteamLibrary/steamapps/common')/NAMES[i-1]/'GameAssembly.dll';dump=ROOT.parent/f'ff{i}'/'dump.cs'
   assert sha(dll)==row['dll_sha256']and sha(dump)==row['dump_sha256']
   pe=pefile.PE(str(dll),fast_load=True)
   assert 0<row['bytes']<=4096 and hashlib.sha256(pe.get_data(row['rva'],row['bytes'])).hexdigest()==row['sha256']
  print('Six exact native Ready->Show producers unchanged; existing archives reused without disassembly/export or native execution.')
  return
 folder=ROOT.parent/'tools/puppet_ready_20261101';folder.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 facts=[];dis=Cs(CS_ARCH_X86,CS_MODE_64)
 for i,name in enumerate(NAMES,1):
  key=f'ff{i}';dump=ROOT.parent/key/'dump.cs';dll=Path('D:/Games/SteamLibrary/steamapps/common')/name/'GameAssembly.dll'
  old=json.loads((ROOT.parent/'tools/npc_fynn_20261023'/key/'provenance.json').read_text())
  assert sha(dll)==old['dll_sha256']and sha(dump)==old['dump_sha256'],key
  text=dump.read_text(encoding='utf-8');start=text.index('public class PuppetCore : Core');end=text.index('// Namespace:',start)
  declaration=text[start:end];ready=re.search(r'// RVA: 0x([0-9A-F]+)[^\n]*\n\s*public override void Ready\(\)',declaration);assert ready,key
  entity=text.index('public class FieldEntity ');entity_end=text.index('// Namespace:',entity)
  show=re.search(r'// RVA: 0x([0-9A-F]+)[^\n]*Slot: (\d+)\n\s*public virtual void Show\(float duration',text[entity:entity_end]);assert show,key
  slot=int(show[2]);offset=0x130+16*slot
  rva=int(ready[1],16);starts=sorted({int(s,16)for s in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text)});end=next(x for x in starts if x>rva);assert end-rva<=4096
  pe=pefile.PE(str(dll),fast_load=True);base=pe.OPTIONAL_HEADER.ImageBase;body=pe.get_data(rva,end-rva);instructions=list(dis.disasm(body,base+rva))
  calls=[ins.address-base for ins in instructions if ins.mnemonic in('call','jmp')and 'qword ptr'in ins.op_str and f'+ 0x{offset:x}]'in ins.op_str]
  assert len(calls)==1 and any(ins.mnemonic in('xorps','pxor')and ins.op_str=='xmm1, xmm1'for ins in instructions),(key,'exact zero-duration actor Show dispatch')
  row=dict(game=key,dll_sha256=old['dll_sha256'],dump_sha256=old['dump_sha256'],method='PuppetCore.Ready',rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),actor_show_rva=int(show[1],16),actor_show_slot=slot,ready_show_call_rva=calls[0])
  facts.append(row)
  if i==5:
   existing=json.loads((ROOT/'build_check/ff5_catapult_native.json').read_text())['methods'][1]
   assert existing['rva']==rva and existing['sha256']==row['sha256'];continue
  path=folder/(key+'_ready.txt')
  if path.exists():continue
  with path.open('w',encoding='utf-8')as f:
   f.write(key+' PuppetCore.Ready\n')
   for ins in instructions:f.write(f'{ins.address:x}: {ins.mnemonic} {ins.op_str}\n')
 (ROOT/'build_check/puppet_ready_contract.json').write_text(json.dumps(dict(methods=facts,scope='Static source-matched Ready calls actor Show(0) before each newly entered puppet body; native code external, no game execution.'),indent=2)+'\n')
 print('All six exact PuppetCore.Ready owners dispatch actor Show(0); FFV/current external archives reused, other five bodies external. Projection repair pending.')
if __name__=='__main__':main()
