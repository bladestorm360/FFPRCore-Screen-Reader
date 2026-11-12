"""Read three exact native position helpers, writing raw analysis outside Git."""
from pathlib import Path
import hashlib,json,re,subprocess
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[2];BASE=ROOT.parent/'tools'
def sha(path):
 h=hashlib.sha256()
 with path.open('rb') as f:
  for data in iter(lambda:f.read(1024*1024),b''):h.update(data)
 return h.hexdigest()
def main():
 out=BASE/'ff5_library_20261101';out.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');dump=ROOT.parent/'FF5/dump.cs'
 old=json.loads((BASE/'chocobo_owen_20261025/ff5/provenance.json').read_text(encoding='utf-8'))
 assert sha(dll)==old['dll_sha256'] and sha(dump)==old['dump_sha256']
 text=dump.read_text(encoding='utf-8');starts=sorted({int(x,16) for x in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text)})
 pe=pefile.PE(str(dll),fast_load=True);dis=Cs(CS_ARCH_X86,CS_MODE_64);facts=[]
 with (out/'puppet_position_native.txt').open('w',encoding='utf-8') as output:
  for name,rva in (('Puppet.SetPos',0x2DBD20),('Puppet.GetDirectTargetPosition',0x2D57C0),('Puppet.GetChangeLayerValue',0x2D5510),('Puppet.MoveEntityLayerWithWorldPosition',0x2D7C80)):
   end=next(x for x in starts if x>rva);body=pe.get_data(rva,end-rva);facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest()))
   output.write(f'\n{name} RVA {rva:X}\n')
   for instruction in dis.disasm(body,rva):output.write(f'{instruction.address:X} {instruction.mnemonic} {instruction.op_str}\n')
 (out/'puppet_position_provenance.json').write_text(json.dumps(dict(dll_sha256=old['dll_sha256'],dump_sha256=old['dump_sha256'],functions=facts),indent=2)+'\n')
 print('Read four exact FFV position helpers externally; DLL/dump hashes match; no raw native code inside Git.')
if __name__=='__main__':main()
