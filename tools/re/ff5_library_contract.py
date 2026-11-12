"""Exact Library transfer owners; source-bearing analysis stays outside Git."""
from pathlib import Path
import collections,csv,hashlib,json,re,subprocess,sys
import pefile
ROOT=Path(__file__).resolve().parents[2]
DLL=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
DUMP=ROOT.parent/'FF5/dump.cs'
TARGETS=(('Current.ChangeScript',0xA3E3A0),('MainCore.ChangeScript',0x78DC40))
def sha(path):
 h=hashlib.sha256()
 with path.open('rb')as stream:
  for part in iter(lambda:stream.read(1024*1024),b''):h.update(part)
 return h.hexdigest()
def main():
 dll_sha=sha(DLL);dump_sha=sha(DUMP)
 assert dll_sha=='355bb0eccd3cf6a2cb70d48f2329379eb98d4b5085971d46985154a882dcfb17'
 assert dump_sha=='99239a856351c4cc7310bae2679af73085efebc765a00459b4d64cf088ee7aa3'
 folder=ROOT.parent/'tools/ff5_library_20261101';source=ROOT.parent/'tools/mapdump'
 archive=json.loads((folder/'native_programs.json').read_text(encoding='utf-8'))
 expected={r['name']:r for r in archive};assert len(expected)==len(archive)==53
 actual={}
 for line in (source/'ff5_scripts.jsonl').open(encoding='utf-8'):
  row=json.loads(line)
  if row['submap'].startswith('Map_20221_'):actual[row['name']]=row
 assert actual.keys()==expected.keys()
 for name,row in actual.items():
  assert hashlib.sha256(json.dumps(row['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==expected[name]['sha256'],name
  assert next(s['EntryPoint']for s in row['segments']if s['Label']=='Main')==0,name
 scripts={int(r['id']):r['script_name']for r in csv.DictReader((source/'ff5_master/script.txt').open(encoding='utf-8-sig'))}
 doors={('Map_20221_5',2):(2634,'sc_e_0220_9',395,275,2),('Map_20221_7',2):(1242,'sc_20221_7_to_20221_6',371,277,3)};found=set()
 for line in (source/'ff5_objects.jsonl').open(encoding='utf-8'):
  row=json.loads(line);key=row['submap'],row['props'].get('entity_id')
  if key not in doors or row['type_name']!='Event':continue
  script_id,name,bit,destination,point=doors[key]
  assert row['props']['script_id']==script_id and scripts[script_id]==name
  body=actual[name]['mnemonics'];assert [m['mnemonic']for m in body]==['Nop','ResetFlag','ChangeMap','Exit']
  assert expected[name]['flag_updates']==[dict(index=bit,value=0)]
  change=next(m for m in body if m['mnemonic']=='ChangeMap');assert change['operands']['iValues'][:2]==[destination,point]
  found.add(key)
 assert found==doors.keys()
 census=dict(programs=53,external_archive_sha256=sha(folder/'native_programs.json'),exact_scripted_returns=2,
  source_program_hashes={name:row['sha256']for name,row in expected.items()})
 (ROOT/'build_check/ff5_library_primary_contract.json').write_text(json.dumps(census,indent=2)+'\n')
 pe=pefile.PE(str(DLL),fast_load=True)
 if '--check'in sys.argv:
  prior=json.loads((ROOT/'docs/re/ff5_library_native_20261101.json').read_text())
  assert prior['dll_sha256']==dll_sha and prior['dump_sha256']==dump_sha
  assert [(r['method'],r['rva'])for r in prior['methods']]==list(TARGETS)
  for r in prior['methods']:assert 0<r['bytes']<=16384 and hashlib.sha256(pe.get_data(r['rva'],r['bytes'])).hexdigest()==r['sha256']
  print('FFV Library: exact native transfer owners unchanged; no repeated export or native execution.')
  return
 from capstone import Cs,CS_ARCH_X86,CS_MODE_64
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 text=DUMP.read_text(encoding='utf-8');starts=sorted({int(s,16)for s in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text)})
 labels={}
 for m in re.finditer(r'// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|internal|protected) ([^\n]+)',text):labels.setdefault(int(m[1],16),[]).append(m[2])
 dis=Cs(CS_ARCH_X86,CS_MODE_64);base=pe.OPTIONAL_HEADER.ImageBase;facts=[]
 with (folder/'transfer_native.txt').open('w',encoding='utf-8')as output:
  for name,rva in TARGETS:
   end=next(x for x in starts if x>rva);assert end-rva<=16384
   body=pe.get_data(rva,end-rva);facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest()))
   output.write('\n'+name+'\n')
   for ins in dis.disasm(body,base+rva):
    note=''
    if ins.mnemonic in ('call','jmp')and ins.op_str.startswith('0x'):
     aliases=labels.get(int(ins.op_str,16)-base,[]);note=' ; '+' / '.join(aliases)if len(aliases)<=12 else ' ; folded aliases'
    output.write(f'{ins.address:x}: {ins.mnemonic} {ins.op_str}{note}\n')
 (ROOT/'build_check/ff5_library_transfer_native.json').write_text(json.dumps(dict(dll_sha256=dll_sha,dump_sha256=dump_sha,methods=facts),indent=2)+'\n')
 print('Two exact Library script-transfer owners read externally; review pending, no game execution.')
if __name__=='__main__':main()
