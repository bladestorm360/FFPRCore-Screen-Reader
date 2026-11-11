"""Read the exact FF1 defense producer offline; native bodies stay external."""
from pathlib import Path
import collections,hashlib,json,re,subprocess,struct
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
from capstone.x86_const import X86_REG_RIP,X86_OP_MEM
ROOT=Path(__file__).resolve().parents[2]
DLL=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY PR/GameAssembly.dll');DUMP=ROOT.parent/'ff1/dump.cs'
def digest(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
 return h.hexdigest()
def main():
 assert digest(DLL)=='33f001b4420bea06b44740c07c4afcf92547e7196bdfd3ec27ae5feaf0229108'
 assert digest(DUMP)=='8313bacc58a606108db7dc0786b4fc5bcd57dc07e171cb8b051c2a68cd817740'
 out=ROOT.parent/'tools/ff1_monk_comparison_20261101';out.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 text=DUMP.read_text(encoding='utf-8');counts=collections.Counter(int(s,16) for s in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text));starts=sorted(counts);pe=pefile.PE(str(DLL),fast_load=True);cs=Cs(CS_ARCH_X86,CS_MODE_64);cs.detail=True;facts=[];raw=[]
 for name,rva in (('ParameterProvider.GetDefensePower',0x37F080),('ParameterProvider.GetMonkDefensePower',0x37F410),('ParameterProvider.GetSuperMonkDefensePower',0x37F8C0)):
  assert counts[rva]==1;end=next(x for x in starts if x>rva);body=pe.get_data(rva,end-rva);instructions=list(cs.disasm(body,pe.OPTIONAL_HEADER.ImageBase+rva));constants=[]
  for i in instructions:
   if i.mnemonic!='mulss':continue
   for op in i.operands:
    if op.type==X86_OP_MEM and op.mem.base==X86_REG_RIP:
     location=i.address+i.size+op.mem.disp-pe.OPTIONAL_HEADER.ImageBase;constants.append(dict(rva=location,value=struct.unpack('<f',pe.get_data(location,4))[0]))
  facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),float_constants=constants));raw.append(name+'\n'+'\n'.join(f'{i.address:x}: {i.mnemonic} {i.op_str}' for i in instructions))
 (out/'native_bodies.txt').write_text('\n\n'.join(raw)+'\n',encoding='utf-8')
 (ROOT/'build_check/ff1_monk_comparison_contract.json').write_text(json.dumps(dict(methods=facts,scope='Offline primary producer review; no runtime/game parameter read, write or launch.'),indent=2)+'\n',encoding='utf-8')
 print('Verified FF1 defense producer:',len(facts),'unique exact native bodies; analysis external.')
if __name__=='__main__':main()
