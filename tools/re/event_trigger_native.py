"""Bounded offline trigger-enable evidence. Source-bearing output stays outside Git."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import capstone
import pefile

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT.parent/'tools/event_trigger_20261023'
OUT.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
titles=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')
targets={'EventTriggerEntity':{'IsTriggerEnable','OnTriggerEnterCustom','OnTriggerExitCustom','EntityOverlap','Suspend','TriggerActivate','Hide'},
         'BattleEnemyData':{'GetMesIdName','get_MonsterMaster'},
         'FieldEntity':{'UpdateEntity'},'FieldCharaEntity':{'UpdateEntity'},'FieldNonPlayer':{'UpdateEntity'},
         'KeyboardManager':{'ShowKeyboard','Update'}}
for number,title in enumerate(titles,1):
    dump=ROOT.parent/('FF4' if number==4 else 'FF5' if number==5 else f'ff{number}')/'dump.cs'
    dll=Path('D:/Games/SteamLibrary/steamapps/common')/(title+' PR')/'GameAssembly.dll'
    names={};selected={}
    for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
        cls=re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)',block,re.M)
        if not cls:continue
        for match in re.finditer(r'^\s*// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(([^\n]*)\)',block,re.M):
            rva=int(match[1],16);label=cls[1]+'.'+match[2]
            names.setdefault(rva,[]).append(label)
            if match[2] in targets.get(cls[1],set()):selected[label]=rva
    ordered=sorted(names);pe=pefile.PE(str(dll),fast_load=True)
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);output=[]
    for label,rva in selected.items():
        end=next((n for n in ordered if n>rva),rva+4096)
        output.append(f'\n{label} RVA {rva:X}')
        for ins in md.disasm(pe.get_data(rva,min(end-rva,4096)),pe.OPTIONAL_HEADER.ImageBase+rva):
            annotation=''
            if ins.mnemonic in ('call','jmp') and ins.op_str.startswith('0x'):
                labels=names.get(int(ins.op_str,16)-pe.OPTIONAL_HEADER.ImageBase,[])
                annotation=' ; '+' / '.join(labels[:5])
            output.append(f'{ins.address:X} {ins.mnemonic} {ins.op_str}{annotation}')
    (OUT/f'ff{number}.txt').write_text('\n'.join(output),encoding='utf8')
    (OUT/f'ff{number}.json').write_text(json.dumps(dict(functions=selected,dll_sha256=hashlib.sha256(dll.read_bytes()).hexdigest()),indent=2),encoding='utf8')
    print(f'FF{number}: {len(selected)} bounded functions -> {OUT}')
