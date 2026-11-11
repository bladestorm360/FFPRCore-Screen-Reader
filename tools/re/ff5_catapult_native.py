"""Two exact offline puppet owners needed for Catapult's relocated book.

Reuse the existing source-matched position/visibility archives; inspect only the
remaining entry/ready semantics. Never call a native method or emit its code in Git.
"""
from pathlib import Path
import hashlib,json,re,subprocess,sys
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[2]
DLL=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
DUMP=ROOT.parent/'FF5/dump.cs'
def sha(path):
 h=hashlib.sha256()
 with path.open('rb')as f:
  for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
 return h.hexdigest()
def main():
 reference=json.loads((ROOT/'docs/re/ff5_library_native_20261101.json').read_text())
 assert sha(DLL)==reference['dll_sha256'] and sha(DUMP)==reference['dump_sha256']
 out=ROOT.parent/'tools/ff5_catapult_20261101';out.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 ui='--entry-ui'in sys.argv;target=out/('entry_ui_native.txt'if ui else'entry_native.txt')
 manifest=ROOT/('docs/re/ff5_catapult_ui_native_20261102.json'if ui else'build_check/ff5_catapult_native.json')
 targets=(('Current.ChangeStateToEvent',0x2B5700),)if ui else(('Integrator.PuppetCoreEntry',0x78FF10),('PuppetCore.Ready',0x792E10))
 if '--check'in sys.argv:
  pe=pefile.PE(str(DLL),fast_load=True);prior=json.loads(manifest.read_text())
  assert prior['dll_sha256']==reference['dll_sha256']and prior['dump_sha256']==reference['dump_sha256']
  assert [(r['method'],r['rva'])for r in prior['methods']]==list(targets)
  for r in prior['methods']:assert hashlib.sha256(pe.get_data(r['rva'],r['bytes'])).hexdigest()==r['sha256']
  print('Exact Catapult native owners unchanged; no repeated export or execution.');return
 assert not target.exists(),'Reuse existing exact entry archive instead of exporting again'
 text=DUMP.read_text(encoding='utf-8');starts=sorted({int(m,16)for m in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text)})
 labels={}
 for m in re.finditer(r'// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|internal|protected) ([^\n]+)',text):labels.setdefault(int(m[1],16),[]).append(m[2])
 pe=pefile.PE(str(DLL),fast_load=True);dis=Cs(CS_ARCH_X86,CS_MODE_64);base=pe.OPTIONAL_HEADER.ImageBase;facts=[]
 with target.open('w',encoding='utf-8')as f:
  for name,rva in targets:
   end=next(x for x in starts if x>rva);assert 0<end-rva<=16384
   body=pe.get_data(rva,end-rva);facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest()))
   f.write('\n'+name+'\n')
   for ins in dis.disasm(body,base+rva):
    note=''
    if ins.mnemonic in('call','jmp')and ins.op_str.startswith('0x'):
     aliases=labels.get(int(ins.op_str,16)-base,[]);note=' ; '+' / '.join(aliases)if len(aliases)<=12 else ' ; folded aliases'
    f.write(f'{ins.address:x}: {ins.mnemonic} {ins.op_str}{note}\n')
 (ROOT/('build_check/ff5_catapult_ui_native.json'if ui else'build_check/ff5_catapult_native.json')).write_text(json.dumps(dict(dll_sha256=reference['dll_sha256'],dump_sha256=reference['dump_sha256'],methods=facts),indent=2)+'\n')
 print(('One exact Catapult event/UI owner'if ui else'Two exact Catapult puppet owners')+' read externally; existing source archives reused. Review pending; no game run.')
if __name__=='__main__':main()
