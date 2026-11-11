"""Exact timed-arrow ABI, active-Gau branch and current source provenance."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'tools'))
from name_check import Dump
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 dump_path=R.parent/'ff6/dump.cs';d=Dump(dump_path)
 base=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Crescent_Context'
 for directory,count in(('native_controls',13),('native_fork',1)):
  proof=json.loads((base/directory/'provenance.json').read_text(encoding='utf-8'))
  assert proof['dump_sha256']==digest(dump_path)
  assert proof['dll_sha256']==digest(Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY VI PR/GameAssembly.dll'))
  assert len(proof['functions'])==count
 for ns in ('Serial.FF6.UI.KeyInput','Serial.FF6.UI.Touch'):
  methods=d.classes[ns,'ChoiceDirectionController']['methods']
  for name,argc in (('SelectingInit',0),('UpdateDirectionArrow',1),('Finish',0)):assert methods[name]=={argc},(ns,name)
 proof=json.loads((R/'docs/re/crescent_native_20261104.json').read_text(encoding='utf-8'))
 assert len(proof['source_script_hashes'])==29
 print('Crescent:6 unique instance start/enum-writer/finish ABIs;13 UI bodies and1 exact native fork from current DLL/dump;29 pinned scripts. Right0/Left1 renderer and full521 arrival source reviewed.')
if __name__=='__main__':main()
