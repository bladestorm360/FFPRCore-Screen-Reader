"""Pin FFIII consumable-key/Thief owners; native implementations stay external.

This is an offline investigation, never a game attach or a call to Current's
script wrappers. Those wrappers may write interpreter scratch state.
"""
from pathlib import Path
import collections,hashlib,json,re,subprocess,sys
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_64

ROOT=Path(__file__).resolve().parents[2]
DLL=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY III PR/GameAssembly.dll')
DUMP=ROOT.parent/'ff3/dump.cs'
TARGETS=(('Current.CheckThiefIsInCorps',0x91B5C0),
         ('Current.ThiefCorpsVisitor',0x9294F0),
         ('Current.ThiefMemberIdentity',0x389260),
         ('OwnedCharacterData.get_JobId',0x668EC0),
         ('OwnedCharacterData.get_IsEnableCorps',0x668EA0),
         ('OwnedItemData.get_Count',0x66A990),
         ('OwnedItemData.get_ItemId',0x3472B0),
         ('OwnedItemData.get_ContentId',0x66A970),
         ('UserDataManager.GetOwnedCharactersClone',0x644540),
         ('UserDataManager.GetMemberData',0x642F20),
         ('UserDataManager.SearchOwnedItem',0x6454B0))

def digest(path):
 h=hashlib.sha256()
 with path.open('rb') as stream:
  for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
 return h.hexdigest()

def main():
 dll_sha=digest(DLL);dump_sha=digest(DUMP)
 assert dll_sha=='8db9ab31a893ca4b13e5e4d2e2b1b086ac6e3fb5699b9be1d8037dafaaa3ae31'
 assert dump_sha=='cf74887b07f39807182c95701b3888ebfe171f0c9f7afbe8f33811b47c3d4b15'
 if '--check' in sys.argv:
  # Preserve the already-reviewed native archive. A current fingerprint/body
  # comparison qualifies its reuse without repeating disassembly or exports.
  prior=json.loads((ROOT/'docs/re/ff3_key_capability_20261101.json').read_text())
  assert prior['dll_sha256']==dll_sha and prior['dump_sha256']==dump_sha
  methods=prior['methods'];assert [(row['method'],row['rva'])for row in methods]==list(TARGETS)
  pe=pefile.PE(str(DLL),fast_load=True)
  for row in methods:
   assert 0<row['bytes']<=16384
   assert hashlib.sha256(pe.get_data(row['rva'],row['bytes'])).hexdigest()==row['sha256'],row['method']
  archive=ROOT.parent/'tools/ff3_key_capability_20261101/native_bodies.txt'
  (ROOT/'build_check/ff3_key_capability_recheck.json').write_text(json.dumps(dict(
   dll_sha256=dll_sha,dump_sha256=dump_sha,methods=len(methods),archive_sha256=digest(archive)if archive.is_file()else None,
   status='Existing exact native evidence reused; no native execution, disassembly or export.'),indent=2)+'\n')
  print('FFIII key capability:11 existing pinned native owners unchanged; archive preserved, no repeated export or game execution.')
  return
 output=ROOT.parent/'tools/ff3_key_capability_20261101';output.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(output),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 text=DUMP.read_text(encoding='utf-8');counts=collections.Counter(int(x,16) for x in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text));starts=sorted(counts)
 labels={}
 for match in re.finditer(r'// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|internal|protected) ([^\n]+)',text):
  labels.setdefault(int(match[1],16),[]).append(match[2])
 pe=pefile.PE(str(DLL),fast_load=True);base=pe.OPTIONAL_HEADER.ImageBase;cs=Cs(CS_ARCH_X86,CS_MODE_64);facts=[];raw=[]
 for name,rva in TARGETS:
  assert rva in counts,name
  end=next(x for x in starts if x>rva);assert end-rva<=16384,name
  body=pe.get_data(rva,end-rva);instructions=list(cs.disasm(body,base+rva));calls=[];lines=[name]
  for ins in instructions:
   annotation=''
   if ins.mnemonic in ('call','jmp') and ins.op_str.startswith('0x'):
    destination=int(ins.op_str,16)-base;aliases=labels.get(destination,[])
    calls.append(dict(rva=destination,metadata_aliases=aliases if len(aliases)<=12 else [],alias_count=len(aliases)))
    annotation=' ; '+(' / '.join(aliases) if len(aliases)<=12 else str(len(aliases))+' folded aliases')
   lines.append(f'{ins.address:x}: {ins.mnemonic} {ins.op_str}{annotation}')
  raw.append('\n'.join(lines));facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),metadata_aliases=counts[rva],calls=calls))
 (output/'native_bodies.txt').write_text('\n\n'.join(raw)+'\n',encoding='utf-8')
 (ROOT/'build_check/ff3_key_capability_contract.json').write_text(json.dumps(dict(dll_sha256=dll_sha,dump_sha256=dump_sha,methods=facts,scope='Static owners only; interpreter wrappers must never be called for navigation.'),indent=2)+'\n')
 print('Pinned FFIII key capability:',len(facts),'native owners; implementations external. Predicate review still required.')

if __name__=='__main__':main()
