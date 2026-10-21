"""Audit installed native result-name producers without loading a game DLL.

Reports contain locators/hashes and conclusions only, never game implementation.
Raw exploratory disassembly/decompilation stays outside all Git trees.
"""
from pathlib import Path
import hashlib
import json
import re
import capstone
import pefile

ROOT = Path(__file__).resolve().parents[2]
STEAM = Path('D:/Games/SteamLibrary/steamapps/common')
GAMES = ['FINAL FANTASY PR', 'FINAL FANTASY II PR', 'FINAL FANTASY III PR',
         'FINAL FANTASY IV PR', 'FINAL FANTASY V PR', 'FINAL FANTASY VI PR']
records = []
for n, folder in enumerate(GAMES, 1):
    dump = ROOT.parent / f'ff{n}/dump.cs'
    source = dump.read_text(encoding='utf-8-sig')
    methods, blocks, labels = {}, {}, {}
    for block in source.split('// Namespace: ')[1:]:
        namespace = block.splitlines()[0]
        match = re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)', block, re.M)
        if not match:
            continue
        identity = f'{namespace}.{match[1]}'
        blocks[identity] = block
        for rva, method in re.findall(r'// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(', block):
            methods[f'{identity}.{method}'] = int(rva, 16)
            labels.setdefault(int(rva, 16), []).append(f'{identity}.{method}')
    binary = STEAM / folder / 'GameAssembly.dll'
    pe = pefile.PE(str(binary), fast_load=True)
    addresses = sorted(labels)
    dis = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    dis.detail = True
    def instructions(rva):
        end = next((a for a in addresses if a > rva), rva + 8192)
        return list(dis.disasm(pe.get_data(rva, min(end - rva, 16384)), pe.OPTIONAL_HEADER.ImageBase + rva))
    name_rva = methods['Last.Data.User.OwnedCharacterData.get_Name']
    safe_rva = methods['UnityEngine.SafeSetFunction.SafeTextSet']
    wrappers = set()
    for rva, symbols in labels.items():
        if 'Last.UI.UICommonProvider.SetText' in symbols and any(
                i.mnemonic in ('call', 'jmp') and i.op_str == hex(pe.OPTIONAL_HEADER.ImageBase + safe_rva)
                for i in instructions(rva)):
            wrappers.add(rva)
    producers = []
    for name, rva in methods.items():
        if '.UI.KeyInput.Result' not in name or not name.endswith('.SetData'):
            continue
        calls = [(i.address - pe.OPTIONAL_HEADER.ImageBase, int(i.op_str, 16) - pe.OPTIONAL_HEADER.ImageBase)
                 for i in instructions(rva) if i.mnemonic == 'call' and i.op_str.startswith('0x')]
        hits = [at for at, target in calls if target == name_rva]
        if not hits:
            continue
        writes = [at for at, target in calls if target == safe_rva]
        direct = any(write > hits[0] for write in writes)
        wrapped = any(at > hits[0] and target in wrappers for at, target in calls)
        if any(f'.{kind}.' in name for kind in ('ResultStatusUpController', 'ResultCharacterListContentController', 'ResultSkillLevelContentController')):
            assert direct or wrapped, f'FF{n}: name no longer flows to a verified text writer in {name}'
        producers.append({'method': name, 'rva': hex(rva), 'name_call_rvas': [hex(at) for at in hits],
                          'subsequent_safe_text_write': direct,
                          'subsequent_verified_ui_text_wrapper': wrapped,
                          'calls_after_name': [{'rva': hex(target), 'symbols': labels.get(target, [])[:4],
                                                'alias_count': len(labels.get(target, []))}
                                               for at, target in calls if at > hits[0]][:12]})
    assert any('ResultStatusUpController.' in row['method'] for row in producers), f'FF{n}: growth-name producer missing'
    assert any('ResultCharacterListContentController.' in row['method'] for row in producers), f'FF{n}: character-name producer missing'
    if n == 2:
        assert any('ResultSkillLevelContentController.' in row['method'] for row in producers), 'FF2 nested weapon-name producer missing'
    # All six build-matched getters read their saveData's Name field directly.
    owned = blocks['Last.Data.User.OwnedCharacterData']
    save_offset = int(re.search(r'OwnedCharacterSaveData saveData; // 0x([0-9A-Fa-f]+)', owned)[1], 16)
    saved = blocks['Last.Data.User.OwnedCharacterSaveData']
    name_field = re.search(r'string ([\w<>]+); // 0x([0-9A-Fa-f]+)', saved)
    assert name_field, f'FF{n}: saved-name field missing'
    name_offset = int(name_field[2], 16)
    getter = instructions(name_rva)
    def field_read(base, offset):
        # Query operand metadata, retaining no recovered instruction fragments.
        x86 = capstone.x86_const
        return any(i.mnemonic == 'mov' and len(i.operands) == 2 and
                   i.operands[0].type == x86.X86_OP_REG and i.operands[0].reg == x86.X86_REG_RAX and
                   i.operands[1].type == x86.X86_OP_MEM and i.operands[1].size == 8 and
                   i.operands[1].mem.base == base and i.operands[1].mem.index == 0 and
                   i.operands[1].mem.disp == offset for i in getter)
    assert field_read(capstone.x86_const.X86_REG_RCX, save_offset), f'FF{n}: name getter save-data access changed'
    assert field_read(capstone.x86_const.X86_REG_RAX, name_offset), f'FF{n}: saved name access changed'
    records.append({'game': n, 'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
                    'dump_sha256': hashlib.sha256(dump.read_bytes()).hexdigest(),
                    'name_getter_rva': hex(name_rva), 'saved_name_field': name_field[1],
                    'save_data_offset': hex(save_offset), 'saved_name_offset': hex(name_offset),
                    'getter_reads_saved_name_directly': True, 'producers': producers})
    print(f'FF{n}: saved-name getter and {len(producers)} native result-name producers verified')

assert 'Character {0}' not in (ROOT / 'src/core/battle/battle_result.cpp').read_text(encoding='utf-8')
(ROOT / 'docs/re/result_names_native.json').write_text(json.dumps(records, indent=2) + '\n', encoding='utf-8')
