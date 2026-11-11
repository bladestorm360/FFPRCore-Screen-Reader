"""Exact FFIV item-selector ABI/body identities; no native code is emitted."""
from pathlib import Path
import collections,hashlib,json,re
import pefile
import capstone
ROOT=Path(__file__).resolve().parents[2]
DLL=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY IV PR/GameAssembly.dll');DUMP=ROOT.parent/'FF4/dump.cs'
def digest(path):
 h=hashlib.sha256()
 with path.open('rb') as f:
  for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
 return h.hexdigest()
def main():
 assert digest(DLL)=='bb2f4c9db44c8ee9b065696aeafb77561a1709492130f045433e6d215abc42ed'
 assert digest(DUMP)=='011bc480d50bba3784b001844f854eae9abbe7a0050cb91c6e82146dffacc916'
 text=DUMP.read_text(encoding='utf-8');counts=collections.Counter(int(s,16) for s in re.findall(r'// RVA: 0x([0-9A-F]+) Offset:',text));starts=sorted(counts)
 pe=pefile.PE(str(DLL),fast_load=True);facts=[]
 # Touch.SelectContent8091D0 is a different controller. Runtime resolves the
 # KeyInput class by name; pin that actual body and its initial-open producer.
 methods=(('KeyInput.SelectFieldContentController.Open',0x828350),
  ('KeyInput.SelectFieldContentController.SelectContent(int,WithinRangeType)',0x828460),
  ('KeyInput.SelectFieldContentController.SelectInit()',0x828690),
  ('SelectFieldContentManager.Close()',0x704C70))
 for name,rva in methods:
  assert counts[rva]==1,(name,'folded native body')
  end=next(x for x in starts if x>rva);body=pe.get_data(rva,end-rva)
  assert body and body[0] not in (0xC3,0xE9),(name,'empty/forwarded body')
  facts.append(dict(method=name,rva=rva,bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),aliases=counts[rva]))
 # Exact dump namespace + class ownership from the preserved external archive;
 # retain raw bodies there, never in repository fixtures or generated reports.
 archive=ROOT.parent/'tools/ff4_event_initial_20261102/provenance.json'
 provenance=json.loads(archive.read_text())
 assert provenance['dll_sha256']==digest(DLL) and provenance['dump_sha256']==digest(DUMP)
 for suffix,rva in methods[:3]:
  assert any(name.startswith('Last.UI.'+suffix.split('(')[0]) and value==rva for name,value in provenance['functions'].items()),suffix
 decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);decoder.detail=True
 rva=0x828690;end=next(x for x in starts if x>rva)
 instructions=list(decoder.disasm(pe.get_data(rva,end-rva),rva))
 def reads(displacement):
  return any(any(op.type==capstone.x86.X86_OP_MEM and op.mem.disp==displacement for op in ins.operands) for ins in instructions)
 assert reads(0x30) and reads(0x18),'Initial selection must use the native cursor/default index'
 assert any(ins.mnemonic=='call' and any(op.type==capstone.x86.X86_OP_MEM and op.mem.disp==0x1c0 for op in ins.operands) for ins in instructions),'SelectInit must dispatch native SelectContent slot9'
 for name in ('contentDataList','<NameMessageId>k__BackingField','<DescriptionMessageId>k__BackingField','protected bool isEnableDescription; // 0x39'):
  assert name in text
 script_specs={('Map_30131_10','sc_e_0081'):('EncountBoss',231,'sc_e_0081_3'),
  ('Map_30131_10','sc_e_0081_3'):('ChangeMap',100,'sc_e_0081_1'),
  ('Map_20111_4','sc_e_0081_1'):('ChangeMap',201,'sc_e_0081_2')}
 chain=[];found=set();harp=False
 for line in (ROOT.parent/'tools/mapdump/ff4_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line);key=r['submap'],r['name'];body=r['mnemonics']
  if key in script_specs:
   kind,target,continuation=script_specs[key]
   assert any(m['mnemonic']==kind and m['operands']['iValues'][0]==target and m['operands']['sValues'][0]==continuation for m in body),key
   found.add(key);chain.append(dict(script='/'.join(key),operation=kind,target=target,continuation=continuation,
    sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
  if key==('Map_30131_10','sc_e_0081_2'):
   stop=next(i for i,m in enumerate(body) if m['mnemonic']=='SysCall' and m['operands']['sValues'][0]=='磁力エフェクト停止')
   receipt=next(i for i,m in enumerate(body) if m['mnemonic']=='SetFlag' and m['operands']['sValues'][0]=='ScenarioFlag1' and m['operands']['iValues'][0]==97)
   assert stop<receipt;harp=True
 assert found==script_specs.keys() and harp
 (ROOT/'build_check/ff4_event_item_contract.json').write_text(json.dumps(dict(dll_sha256=digest(DLL),dump_sha256=digest(DUMP),methods=facts,automatic_harp_chain=chain,
  initial_ready=dict(controller='Last.UI.KeyInput.SelectFieldContentController',open_rva=0x828350,select_init_rva=0x828690,select_content_rva=0x828460,cursor_field=0x30,index_field=0x18,virtual_slot=9,virtual_dispatch_offset=0x1c0,external_provenance_sha256=digest(archive)),
  ownership='Main-thread native SelectContent postfix, including initial SelectInit dispatch; ready controller list; close/map/title retirement; no native pointer enters a worker.'),indent=2)+'\n')
 print('FFIV item-selector:',len(facts),'unique exact native bodies; typed fields/dump/DLL hashes match.')
if __name__=='__main__':main()
