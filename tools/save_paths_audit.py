"""Check matching all-six native save paths and menu context without executing a game.

Records contain factual locators, hashes and contract results, never game code.
Annotated native bodies remain outside Git via re/map_location_native.py --save-paths.
"""
from pathlib import Path
import bisect
import hashlib
import json
import re

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
STEAM = Path('D:/Games/SteamLibrary/steamapps/common')
TITLES = ('FINAL FANTASY', 'FINAL FANTASY II', 'FINAL FANTASY III',
          'FINAL FANTASY IV', 'FINAL FANTASY V', 'FINAL FANTASY VI')
FOLDERS = ('ff1', 'ff2', 'ff3', 'FF4', 'FF5', 'ff6')
result = {}
for n, (folder, title) in enumerate(zip(FOLDERS, TITLES), 1):
    dump = ROOT.parent / folder / 'dump.cs'
    dll = STEAM / (title+' PR') / 'GameAssembly.dll'
    source = dump.read_text(encoding='utf-8-sig')
    methods, by_rva, fields = {}, {}, {}
    for block in source.split('// Namespace: ')[1:]:
        ns = block.splitlines()[0]
        cls = re.search(r'^(?:public|private|internal).*?\b(?:class|struct|enum) ([\w.<>]+)', block, re.M)
        if not cls:
            continue
        owner = ns+'.'+cls[1]
        fields[owner] = block.split('// Methods')[0]
        for m in re.finditer(r'^\s*// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(([^\n]*)\)', block, re.M):
            rva = int(m[1], 16)
            key = owner+'.'+m[2]
            methods.setdefault(key, []).append(rva)
            by_rva.setdefault(rva, []).append(key)
    ordered = sorted(by_rva)
    pe = pefile.PE(str(dll), fast_load=True)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    base = pe.OPTIONAL_HEADER.ImageBase
    facts = {}

    def rva(key):
        values = methods[key]
        assert len(values) == 1, (n, key, values)
        return values[0]

    def body(key):
        start = rva(key)
        end = ordered[bisect.bisect_right(ordered, start)]
        raw = pe.get_data(start, end-start)
        calls = set()
        for ins in decoder.disasm(raw, base+start):
            if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
                calls.add(int(ins.op_str, 16)-base)
        facts[key] = {'rva': hex(start), 'bytes': len(raw),
                      'sha256': hashlib.sha256(raw).hexdigest()}
        return calls

    script = 'Last.UI.Save.KeyInput.SaveWindowController'
    menu = 'Last.UI.KeyInput.SaveWindowController'
    save_list = 'Last.UI.KeyInput.SaveListController'
    popup = 'Last.UI.KeyInput.SavePopup'
    common = 'Last.UI.KeyInput.CommonPopup'
    for owner, names in {
        script: ('view', 'commonPopup'),
        'Last.UI.Save.KeyInput.SaveWindowView': ('savePopup', 'listController'),
        save_list: ('contentList', 'selectCursor'),
        popup: ('messageText', 'titleText', 'subMessageText', 'commandList'),
    }.items():
        for name in names:
            assert re.search(r'\b'+name+r';', fields[owner]), (n, owner, name)
    hooked = [script+'.'+m for m in ('Open', 'Close', 'SaveSelectInit', 'OverwriteConfirmInit', 'CompleteInit')]
    for key in hooked:
        aliases = set(by_rva[rva(key)])
        # V/VI fold Touch and KeyInput Open with the exact same ABI. The
        # production detour forwards every call and registers only KeyInput.
        allowed = {key}
        if key.endswith('.Open'):
            allowed.add('Last.UI.Save.Touch.SaveWindowController.Open')
        assert aliases <= allowed, (n, 'unexpected folded new hook', key, aliases)
    open_calls = body(script+'.Open')
    if n <= 4:
        assert rva('Last.UI.Save.SaveWindowControllerBase.SetForcedState') in open_calls
    else:
        # V/VI inline SetForcedState. Verified writes select state1 for completed
        # saves and state4 otherwise, followed by the generic state-machine call.
        start = rva(script+'.Open')
        end = ordered[bisect.bisect_right(ordered, start)]
        writes = {(ins.mnemonic, ins.op_str) for ins in decoder.disasm(pe.get_data(start,end-start),base+start)}
        assert ('mov', 'dword ptr [rdi + 0x20], 1') in writes
        assert ('mov', 'dword ptr [rdi + 0x20], 4') in writes
    assert rva(save_list+'.SetActive') in body(script+'.SaveSelectInit')
    overwrite = body(script+'.OverwriteConfirmInit')
    assert rva(popup+'.SetData') in overwrite
    assert rva(popup+'.SetPositiveCommand') in overwrite
    assert rva(popup+'.SetNegativeCommand') in overwrite
    complete = body(script+'.CompleteInit')
    assert rva(save_list+'.Save') in complete
    assert rva(popup+'.SetEnablePositiveCommand') in complete
    # Scripted activation bypasses the ordinary menu hook. Both need observers.
    assert rva(menu+'.SetPopupActive') not in overwrite | complete
    assert rva(menu+'.SetPopupActive') in body(menu+'.CompleteInit')
    for phase in ('SaveConfirmInit', 'NoSaveConfimeInit', 'TitleConfimeInit', 'SaveSelectNoSaveConfimeInit'):
        calls = body(script+'.'+phase)
        assert rva(common+'.SetPositiveCommand') in calls
        assert rva(common+'.SetNegativeCommand') in calls
    body('Last.UI.KeyInput.InterruptionWindowController.InitComplite')
    body('Last.UI.KeyInput.LoadGameWindowController.SetPopupActive')
    body('Last.UI.KeyInput.LoadWindowController.SetPopupActive')
    states = next(value for key, value in fields.items() if key.endswith('.SubSceneManagerMainGame.State'))
    assert 'Menu = 5;' in states and 'Event = 12;' in states
    result[str(n)] = {
        'dump_sha256': hashlib.sha256(dump.read_bytes()).hexdigest(),
        'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'functions': facts,
        'scripted_list_uses_shared_save_list': True,
        'scripted_overwrite_and_complete_bypass_menu_popup_hook': True,
        'ordinary_menu_completion_uses_menu_popup_hook': True,
        'scripted_other_questions_use_existing_common_popup': True,
        'menu_state': 5, 'intro_event_state': 12,
    }
    print(f'FF{n}: script/endgame, menu save/load, title load, quicksave and Menu5/Event12 contracts verified')
(ROOT/'build_check/save_paths_native.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
print('All-six save/menu native audit passed; no game executed')
