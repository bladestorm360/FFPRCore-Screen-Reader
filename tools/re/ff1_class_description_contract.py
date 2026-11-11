"""Exact FF1 class-panel identities; recovered bodies stay outside Git."""
from pathlib import Path
import collections,hashlib,json,re,subprocess
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[2]
DLL=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY PR/GameAssembly.dll')
DUMP=ROOT.parent/'ff1/dump.cs'
def digest(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
 return h.hexdigest()
def main():
 dll_sha=digest(DLL);dump_sha=digest(DUMP)
 assert dll_sha=='33f001b4420bea06b44740c07c4afcf92547e7196bdfd3ec27ae5feaf0229108'
 assert dump_sha=='8313bacc58a606108db7dc0786b4fc5bcd57dc07e171cb8b051c2a68cd817740'
 target=ROOT.parent/'tools/ff1_class_description_20261101';target.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(target),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 text=DUMP.read_text(encoding='utf-8');counts=collections.Counter(int(s,16) for s in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text));starts=sorted(counts)
 start=text.index('// Namespace: Last.UI.KeyInput\npublic class JobContentListController :');block=text[start:text.index('private sealed class JobContentListController.<>c',start)]
 for declaration in ('private List<JobContentController> contentList; // 0x38','private GameObject discriptionParent; // 0x40','private Text discriptionText; // 0x48'):assert declaration in block
 pe=pefile.PE(str(DLL),fast_load=True);cs=Cs(CS_ARCH_X86,CS_MODE_64);facts=[];raw=[]
 for name,rva in (('JobContentListController.SelectContent',0x830FA0),('JobContentListController.SetDiscriptionText',0x831450),('JobContentController.SetData',0x830580)):
  assert counts[rva]==1,(name,'folded body');end=next(x for x in starts if x>rva);body=pe.get_data(rva,end-rva);assert body and body[0] not in (0xc3,0xe9)
  facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),aliases=counts[rva]))
  raw.append(name+'\n'+'\n'.join(f'{i.address:x}: {i.mnemonic} {i.op_str}' for i in cs.disasm(body,pe.OPTIONAL_HEADER.ImageBase+rva)))
 (target/'native_bodies.txt').write_text('\n\n'.join(raw)+'\n',encoding='utf-8')
 report=dict(dll_sha256=dll_sha,dump_sha256=dump_sha,methods=facts,fields={'contentList':56,'discriptionParent':64,'discriptionText':72},raw_analysis='External ../tools/ff1_class_description_20261101/native_bodies.txt')
 (ROOT/'build_check/ff1_class_description_contract.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(report))
if __name__=='__main__':main()
