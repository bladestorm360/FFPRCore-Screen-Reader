"""Bounded offline FF6 story/party/entity evidence; never runs game code.

Disassembly is written outside every Git tree. Only original extraction logic
belongs here; the matching local PE and declarations are external prerequisites.
"""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys
import capstone
import pefile

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT.parent / 'tools/ff6_story_20261023'
NUMBER=int(sys.argv[1]) if sys.argv[1:] else 6
assert 1<=NUMBER<=6
if NUMBER!=6:OUT=OUT/f'ff{NUMBER}'
OUT.mkdir(parents=True, exist_ok=True)
assert subprocess.run(['git', '-C', str(OUT), 'rev-parse', '--show-toplevel'],
                      capture_output=True).returncode != 0, 'Evidence must be outside Git'
DUMP = ROOT.parent / ('FF4' if NUMBER==4 else 'FF5' if NUMBER==5 else f'ff{NUMBER}') / 'dump.cs'
TITLE=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')[NUMBER-1]
DLL = Path('D:/Games/SteamLibrary/steamapps/common')/(TITLE+' PR')/'GameAssembly.dll'
TARGETS = {
    'FieldNonPlayer': {'SetProperty', 'CanEnter', 'Setup', 'get_CanAction'},
    'FieldPartyEntity': {'CanEnter'},
    'FieldNonPlayerControllerManager': {'Create', 'Setup', 'Add', 'CreateController'},
    'FieldPlayerController': {'SwitchCorps', 'ChangePartyEntityAsset'},
    'UserDataManager': {'get_CorpsSlotIndex', 'set_CorpsSlotIndex'},
    'EventActionDefine': {'.cctor'},
    'Current': {'SwitchCorps', 'ShiftManipulateSlot', 'ShiftManipulateSlotAfterChangeMap'},
    'FieldController': {'SetEventEntityGroup','PlacementPartyEntity','CreateFieldPartyEntities','GetPartyEntity','IsCanMoveToDestPosition'},
    'Entity': {'SetEntities'},
    'External.Entity': {'SetEntities'},
    'Puppet': {'SetPos','Lerp'},
    'SaveMapManager': {'AddOtherPartyData','get_SaveData'},
    'SaveMapData': {'AddOtherPartyData'},
    'External.Puppet': {'SetPuppet','ExecPuppet'},
    'Integrator': {'ExecutePuppetCores','AddPuppetCore'},
    'OperaController.<StartOperaInteractiveTimer>d__79': {'MoveNext'},
    'OperaController.<PlayOperaEvent>d__75': {'MoveNext'},
}
if NUMBER!=6:
    TARGETS={key:TARGETS[key] for key in ('FieldNonPlayer','EventActionDefine','FieldController')}
    TARGETS['FieldNonPlayer']={'SetProperty','get_CanAction'}
if '--visibility' in sys.argv:
    # Shared visibility/collision maintenance uses its own evidence directory;
    # never replace the earlier story/party investigation's native outputs.
    OUT=ROOT.parent/'tools/npc_fynn_20261023'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'External.Puppet':{'SetPuppet','ExecPuppet'},
             'Integrator':{'AddPuppetCore','ExecutePuppetCores'},
             'Puppet':{'Hide','Show','ShowHideCommon'},'FieldNonPlayer':{'CanEnter'},
             'FieldCharaEntity':{'CanEnter'}}
if '--npc-detours' in sys.argv:
    # Keep doorway/movement evidence separate from completed visibility exports.
    OUT=ROOT.parent/'tools/npc_detours_20261023'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'FieldPlayerKeyController':{'OnTouchPadCallback'},
             'FieldPlayer':{'MoveTo'},'FieldSpriteEntity':{'MoveTo'},'FieldEntity':{'MoveTo'},
             'FieldController':{'IsCanMoveToDestPosition'}}
if '--npc-actions' in sys.argv:
    # Exact unanswered question: can action 2 with no script/message interact?
    # Inspect dispatch and the script runner, retaining native text outside Git.
    OUT=ROOT.parent/'tools/mysidia_objects_20261024'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'FieldNonPlayer':{'TakeAction','IsAcceptActionDirection'},
             'EventAction':{'Talk','CraeteTask'},'EventActionDefine':{'.cctor'},
             'EventActionScript':{'RequestEventRunScript','RunScript'},
             'EventActionScript.<EventRunScript>d__9':{'MoveNext'},
             'EventProcedure':{'EventTalk','EventTalkTask'},'EventActionScript.<EventRunScript>d__8':{'MoveNext'}}
if '--map-panels' in sys.argv:
    # Known displayed map producers: settle positions/units, visible counts,
    # service-icon visibility and exact Show/Close ownership before hooking.
    OUT=ROOT.parent/'tools/map_panels_20261024'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'ExpandMinimapMapController':{'Show','SetupPointList','SetupPoint','UpdatePointItem'},
             'ExpandMinimapUIManager':{'SetMapResource'},
             'ExpandMinimapController':{'Show','Hide'},
             'GpsMenuInfoController':{'UpdateView'},
             'GpsMenuController':{'Close','SetupKeyHelp','SetupKeyHelpFromExpandMinimap'},
             'GpsIconContentView':{'SetIcon'},
             'GpsMenuInfoView':{'SetEnableContentParent','SetNumChestValue','SetMaxChestValue','SetEnableShopSign'}}
if '--ally-targets' in sys.argv:
    OUT=ROOT.parent/'tools/ally_targets_20261024'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'BattleTargetSelectController':{'PlayerInit','PlayerUpdate','SelectContent','ShowWindow'},
             'BattlePlayerInfomationController':{'UpdateView','SetTargetFocus','SetFocus','UseTargetSelectContent','UsePlayerInfoContent'},
             'BattlePlayerInfomationTargetSelectContentController':{'UpdateView','SetFocus'},
             'BattlePlayerInfomationContentController':{'UpdateView'}}
if '--poison-status' in sys.argv:
    OUT=ROOT.parent/'tools/poison_status_20261024'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'BattleConditionController':{'BattleEndRecoveryCondition'},
             'ConditionUtility':{'IsOutBattle'},'ConditionDamage':{'Last.Map.IFootMonitor.Excute'},
             'PoisonConditionFunction':{'Start','UpdateParameter'}}
if '--battle-item-equipment' in sys.argv:
    OUT=ROOT.parent/'tools/battle_item_equipment_20261024'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'BattleItemInfomationController':{'SelectEquipContent','UseEquipSelectInit','UseEquipSelectUpdate','UseSelectUpdate','SetEquipDescription','UpdateEquipFocus','EquipmentSelectInit','SelectContent','SetEquipList'},
             'BattleEquipContentController':{'UpdateView'}}
names, targets = {}, {}
for block in DUMP.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
    ns = block.splitlines()[0]
    cls = re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)', block, re.M)
    if not cls:
        continue
    for m in re.finditer(r'^\s*// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(([^\n]*)\)', block, re.M):
        rva = int(m[1], 16)
        label = ns + '.' + cls[1] + '.' + m[2] + '(' + m[3] + ')'
        names.setdefault(rva, []).append(label)
        if m[2] in TARGETS.get(cls[1], set()) or ('--ally-targets' in sys.argv and cls[1].startswith('BattleTargetSelectController.') and '<PlayerUpdate>' in m[2]):
            targets[label] = rva
ordered = sorted(names)
pe = pefile.PE(str(DLL), fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
output = []
for name, rva in targets.items():
    end = next((v for v in ordered if v > rva), rva + 16384)
    output.append('\n' + name + ' RVA ' + hex(rva))
    for ins in md.disasm(pe.get_data(rva, min(end-rva, 32768)), pe.OPTIONAL_HEADER.ImageBase+rva):
        annotation = ''
        if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
            labels = names.get(int(ins.op_str, 16)-pe.OPTIONAL_HEADER.ImageBase, [])
            annotation = ' ; ' + (' / '.join(labels) if len(labels) < 5 else str(len(labels)) + ' folded aliases')
        output.append(f'{ins.address:X} {ins.mnemonic} {ins.op_str}{annotation}')
(OUT/'native.txt').write_text('\n'.join(output), encoding='utf-8')
receipt = dict(dll_sha256=hashlib.sha256(DLL.read_bytes()).hexdigest(),
               dump_sha256=hashlib.sha256(DUMP.read_bytes()).hexdigest(), functions=targets)
(OUT/'provenance.json').write_text(json.dumps(receipt, indent=2), encoding='utf-8')
print(f'{len(targets)} bounded functions exported to {OUT}')
