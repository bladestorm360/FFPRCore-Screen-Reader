"""Exact native Select ABI/nested bindings and source-pinned Palace facts."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'tools'))
from name_check import Dump
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 dump_path=R.parent/'ff6/dump.cs';d=Dump(dump_path)
 fields={('Last.Interpreter','Core'):['<programCounter>k__BackingField','mnemonics','currentInstruction'],
  ('Last.Interpreter','Core.ProgramCounter'):['<clockCount>k__BackingField','<currentAddress>k__BackingField'],
  ('Last.Interpreter.Instructions','Format.Mnemonic'):['mnemonic','operands'],
  ('Last.Interpreter.Instructions','Format.Operands'):['sValues']}
 for(ns,name),wanted in fields.items():
  assert (ns,name.split('.')[0])in d.classes,(ns,name)
  row=d.classes.get((ns,name))or d.classes.get(('',name));assert row,(ns,name)
  assert set(wanted)<=row['fields'],(name,wanted)
 message=d.classes['','External.Message'];assert message['methods']['Select']=={1}
 base=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Imperial_Palace'
 provenance=json.loads((base/'native_select/provenance.json').read_text(encoding='utf-8'))
 assert provenance['dump_sha256']==digest(dump_path)
 assert provenance['dll_sha256']==digest(Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY VI PR/GameAssembly.dll'))
 assert any('.External.Message.Select('in name and rva==0xB63E60 for name,rva in provenance['functions'].items())
 proof=json.loads((R/'docs/re/palace_native_20261104.json').read_text(encoding='utf-8'))
 assert len(proof['source_script_hashes'])==90 and len(proof['choices'])==16
 print('Palace native:correct outer namespaces/eight fields, unique one-argument static Select/result ABI and current exact native provenance;90 scripts/16 choice sites.')
if __name__=='__main__':main()
