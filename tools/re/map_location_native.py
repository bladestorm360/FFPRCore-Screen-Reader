"""Extract exact native location-label producers for offline inspection.

Original extractor only in Git; proprietary disassembly/provenance is written
outside Git. This never executes the game or modifies its assets.
"""
from pathlib import Path
import bisect
import hashlib
import json
import re
import subprocess
import sys

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[2]
number = int(sys.argv[1]) if len(sys.argv) > 1 else 2
assert 1 <= number <= 6
folder = ('ff1', 'ff2', 'ff3', 'FF4', 'FF5', 'ff6')[number-1]
title = ('FINAL FANTASY', 'FINAL FANTASY II', 'FINAL FANTASY III',
         'FINAL FANTASY IV', 'FINAL FANTASY V', 'FINAL FANTASY VI')[number-1]
dump = ROOT.parent / folder / 'dump.cs'
dll = Path('D:/Games/SteamLibrary/steamapps/common') / (title+' PR') / 'GameAssembly.dll'
out = ROOT.parent / 'tools/map_floor_20261025' / f'ff{number}'
out.mkdir(parents=True, exist_ok=True)
assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'],
                      capture_output=True).returncode != 0
targets = {
    'SaveContentController': {'GetLocationTextBySaveData', 'SetData', 'SetInterruptionData'},
    'UserDataManager': {'GetLocationText'},
    'FieldController': {'GetMapTitleID', 'GetMapTitleType', 'MapSetup', 'MapInitialize'},
    'External': {'ShowMapTitle'},
}
if '--battle-gear' in sys.argv:
    out = ROOT.parent / 'tools/battle_gear_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'],
                          capture_output=True).returncode != 0
    targets = {'BattleEquipInfomationConroller': {'SelectContent', 'UseSelectInit', 'ShowSelect', 'Close', 'SetDescription', 'DecisionItem'},
               'BattleEquipSelectContentController': {'UpdateView'},
               'BattleEquipSelectContentView': {'SetPartsText', 'SetIconText', 'SetDominantText'},
               'BattleItemInfomationController': {'EquipmentSelectInit', 'EquipmentSelectExit', 'EquipmentSelectUpdate'},
               'KeyHelpIconManager': {'Initialize', 'AddController', 'RemoveController'},
               'KeyIconController': {'Initialize', 'UpdateView', 'SetIconData', 'OnDestroy'}}
if '--save-paths' in sys.argv:
    out = ROOT.parent / 'tools/save_paths_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'],
                          capture_output=True).returncode != 0
    targets = {
        'SaveWindowController': {'Open', 'Close', 'SaveSelectInit', 'SaveConfirmInit',
            'NoSaveConfimeInit', 'TitleConfimeInit', 'SaveSelectNoSaveConfimeInit',
            'OverwriteConfirmInit', 'CompleteInit', 'SetPopupActive'},
        'SaveWindowManager': {'SaveInit', 'CleanSavePopups'},
        'SaveListController': {'SetActive', 'GetSelectedContent'},
        'InterruptionWindowController': {'InitComplite', 'InitConfirm'},
        'LoadWindowController': {'SetPopupActive'},
        'LoadGameWindowController': {'SetPopupActive'},
        'SavePopup': {'SetActive', 'SetData', 'SetTitle', 'SetMessage', 'SetCursor'},
    }
if '--dialogue-opening' in sys.argv:
    out = ROOT.parent / 'tools/dialogue_opening_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {
        'MessageWindowManager': {'SetContent', 'PlayingInit', 'SetSpeker', 'SetSpeakerAsset'},
        'MessageWindowController': {'SetSpeker', 'SetEnableSpeker', 'SetSpeakerBaseImage'},
        'MessageWindowView': {'SetSpeker', 'UseSpekerText'},
        'MessageWindowClient': {'Play'},
        'BattleController': {'StartPreeMptiveMes'},
        'BattleMenuController': {'SetCommandMessageAtKey', 'SetCommadnMessage'},
        'BattleCommandMessageController': {'SetMessage', 'SetSystemMessage'},
        'BattleUtility': {'SetCommandMessageAtKey'},
        'BattleUIManager': {'SetCommadnMessage', 'SetCommandMessage', 'SetSystemMessage'},
        'DyingConditionFunction': {'Start', 'End', 'UpdateParameter', 'GetParameterGroupDic'},
        'BattleStatusControl': {'UpdateTempParamter'},
        'BattleTempParameter': {'UpdateParameter'},
    }
if '--job-icons' in sys.argv:
    out = ROOT.parent / 'tools/job_icons_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'JobChangeWindowController': {'UpdateJobInfo'},
               'JobChangeWindowBaseController': {'UpdateJobEquipView', 'UpdateJobEquipIconView'},
               'JobInfomationData': {'GetEquipIconList'},
               'IconTextUtility': {'GetIcon', 'GetIconSprite', 'SetIconSprite', 'GetSprite'}}
if '--character-rows' in sys.argv:
    out = ROOT.parent / 'tools/character_rows_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'StatusWindowContentController': {'DefaultCursorParent', 'FocusCursorParent'}, 'StatusWindowControllerBase': {'SetCursor', 'SetFocusCursor'}, 'StatusWindowController': {'SelectContent', 'SetFocus', 'ListInit', 'SelectInit', 'SelectUpdate'}, 'Cursor': {'SetSource', 'SetParent', 'SetFocus'}, 'ItemWindowController': {'TargetSelectInit', 'InterChangeSelectInit'},
               'ItemUseController': {'SelectContent', 'SetCursor', 'SingleInit'},
               'StatusWindowContentViewBase': {'SetMp', 'SetParameter', 'UpdateParameter'},
               'StatusWindowContentControllerBase': {'UpdateView'}}
if '--counter-interaction' in sys.argv:
    out = ROOT.parent / 'tools/counter_interaction_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'InteractiveTargetObserver': {'CanPlayerInteractToEntityPosition', 'SearchInteractiveArea', 'IsFront', 'IsAcceptActionDirection', '.ctor'},
               'FieldRelayInteraction': {'Init', 'IsInRange', 'get_HorizontalCellLimit', 'get_VerticalCellLimit'},
               'PropertyRelayInteraction': {'.ctor', 'ConvertPropertyToData'}, 'PropertyEntity': {'ConvertPropertyToData'}, 'MapUtility': {'CanRelayInteraction'}}
if '--landing-guidance' in sys.argv:
    out = ROOT.parent / 'tools/landing_guidance_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'MapUIManager': {'ShowLandingGuide', 'HideLandingGuide', 'SwitchLandable'},
               'FieldController': {'UpdateStateSwitchLandable', 'ChangeTransportation'},
               'FieldMap': {'UpdateStateSwitchLandable'},
               'TransportationController': {'get_CurrentTransportation', 'ChangeTransportation'}}
if '--treasure-flags' in sys.argv:
    out = ROOT.parent / 'tools/treasure_flags_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'FieldTresureBox': {'Init', 'Open', 'SetProperty', 'UpdateEntity'},
               'PropertyTresureBox': {'get_FlagId', 'ConvertPropertyToData'},
               'DataStorage': {'GetFlag', 'SetFlag'}, 'Flags': {'Get', 'Set'},
               'MapUIManager': {'GetFlag', 'GetTreasureFlag'}}
if '--npc-blockers' in sys.argv:
    out = ROOT.parent / 'tools/npc_blockers_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'FieldController': {'SetupCompanionEntity', 'ChangeCompanionEntityAsset'},
               'MapManager': {'CreateCompanion'},
               'PropertyNpc': {'get_AssetId'},
               'FieldNonPlayer': {'SetProperty', 'Init', 'SetMoveType', 'UpdateMove'},
               'FieldCharaEntity': {'ChangeAsset', 'SetAsset', 'get_AssetId'},
               'MapAccessor': {'SetCompanionEntity'}}
if '--field-targets' in sys.argv:
    out = ROOT.parent / 'tools/field_targets_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'AbilityUseContentListController': {'AllInit','SingleInit','IsAll','IsSingle','SetCursor','Show','Close','SetAllActive'},
               'AbilityWindowController': {'UseTargetInit','UseTargetExit'},
               'AbilityUseContentListView': {'SetAllCursor','SetSingleCursor'}}
if '--enemy-critical' in sys.argv:
    out = ROOT.parent / 'tools/enemy_critical_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'DyingConditionFunction': {'Start','End','Remove'},
               'AnimStateController': {'AnimDying','UpdateEntity','SetAnimState','GetAnimState','SetIdleAnimState','SetStandingState'},
               'BattleSpriteEntity': {'SetAnimState','SetAnimation'},
               'BattleEnemyEntity': {'SetAnimState','SetAnimation'},
               'BattleAnimatorBase': {'Dying','UpdateDying'},
               'BattleEnemyAnimatorBase': {'Dying','UpdateDying'},
               'BattleEnemyAnimator': {'Dying','UpdateDying'}}
if '--npc-floors' in sys.argv:
    out = ROOT.parent / 'tools/npc_floors_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'Current': {'HideMist', 'CheckWithCompanion', 'MapEntryRoofControl'},
               'FieldController': {'SetEventEntityGroup', 'HideScenery'},
               'FieldMap': {'HideScenery'},
               'FieldNonPlayer': {'Init', 'Setup', 'Show', 'Hide', 'get_CanAction', 'get_AllLayerTarget'},
               'FieldMapObjectDefault': {'get_AllLayerTarget'},
               'FieldCompanionEntity': {'CanEnter', 'UpdateRaycast'},
               'PropertyTresureBox': {'get_PickUp', 'ConvertPropertyToData'},
               'FieldCharaEntity': {'Hide', 'Show'},
               'FieldEntity': {'Hide', 'Show', 'get_IsActive', 'SetPosition'},
               'ShopCharaStatusContentController': {'Comparison', 'SetCanEquipAnime', 'SetDefaultAnime', 'SetMoveAnime', 'SetData'},
               'CharacterImageController': {'SetAnimation', 'SetData', 'PlayAnimation', 'SetCanEquipAnime', 'SetDefaultAnime', 'SetMoveAnime'},
               'MainViewMapRenderer': {'HideScenery'}}
if '--chocobo-rules' in sys.argv:
    out = ROOT.parent / 'tools/chocobo_owen_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'FieldController': {'CanLand', 'CanLandToWorldMap', 'CanLandToSubMap',
                    'WorldActionCommand', 'BoardingTransportationAction', 'TransportationLanding'},
               'TransportationEvent': {'OnChange', 'OnStartLanding', 'IsLandingEvent',
                    'CanIncludeLandingToMappingData', 'CheckLandingList'},
               'TransportationController': {'CheckLandingList', 'CheckLandingAutoMove'},
               'EventTriggerEntity': {'IsTriggerEnable'},
               'FieldEntity': {'CanEnter', 'SetCollisionEnabled', 'SwitchCollisionEnable'},
               'FieldMapObjectDefault': {'Init', 'SetProperty', 'OnTriggerEnterCustom'},
               'Current': {'CheckFrog', 'CheckConditionCommon'},
               'MapUtility': {'GetCollisionRect'},
               'GotoMapEventEntity': {'OnTriggerStayCustom'},
               'MapConstants': {'.cctor'}}
if '--boot-scenes' in sys.argv:
    out = ROOT.parent / 'tools/startup_hang_20261025' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'SceneBoot': {'Start', 'CreateInstance', 'Load', 'LoadAssetBundlePathMatch', 'LoadSceneWait', 'RequestLoadScene', 'IsInfomationConnect', 'CheckInfomationConfig', 'SetUp'},
               'ResourceManager': {'Initialize'}, 'LogManager': {'Instance', 'LogSetting'},
               'SceneBoot.<Load>d__9': {'MoveNext'},
               'SceneBoot.<LoadAssetBundlePathMatch>d__10': {'MoveNext'},
               'SceneBoot.<LoadSceneWait>d__12': {'MoveNext'}}
if '--ally-status' in sys.argv:
    out = ROOT.parent / 'tools/ally_status_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'BattlePlayerInfomationContentController': {'UpdateView','SettingCondition','UseCondition','Reset'},
               'BattlePlayerInfomationContentView': {'SetHPText','SetCurrentHpTextColor','SetConditionView','UseCommandView'},
               'BattleConditionController': {'UpdateConditionView','SetDefaultState','SetConditionText','IconInit','TextInit','BuffInit','DeuffInit','Reset'},
               'ConditionIconController': {'UpdateView','GetIcon','ResetView'},
               'BattlePlayerAnimatorBase': {'Dying','UpdateDying'},
               'FF6BattlePlayerAnimatorBase': {'Dying','UpdateDying'},
               'BattleEnemyAnimatorBase': {'Dying','UpdateDying'},
               'BattleEnemyAnimator': {'Dying','UpdateDying'},
               'TextColors': {'.cctor'},
               'BattleAnimatorBase': {'Dying','UpdateDying'},
               'DyingConditionFunction': {'Start','End'},
               'CharacterParameterBase': {'IsDying','IsDead','ConfirmedConditionList'},
               'BattleUtility': {'GetParameterColor','GetHpColor','GetHPColor'},
               'ParameterColorUtility': {'GetHpColor','GetHPColor','GetColor'}}
if '--ally-status' in sys.argv:
    blocks = dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]
    for block in blocks:
        if 'class BattlePlayerInfomationContentController.<>c ' in block:
            methods = set(re.findall(r'internal bool (<?[\w]+>b__\d+_\d+)\(', block))
            targets.setdefault('BattlePlayerInfomationContentController.<>c', set()).update(methods)
if '--ally-bodies' in sys.argv:
    out = ROOT.parent / 'tools/ally_body_status_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'BattlePlayerEntity': {'ChangeMotionListFromCondition','SetAnimSprite','SpriteChangeInit'},
               'SpriteAnimBattle': {'SetupSpriteAnimation','GetCurrentSpriteName','SetSprite'},
               'BattlePlayerAnimatorBase': {'Dying','UpdateDying'},
               'AnimStateController': {'AnimDying','AnimCondition','UpdateEntity'},
               'InstantiateManager': {'InitPlayerEntityFromCondition','InitPlayerEntity','InitPlayerSpriteFromCondition'},
               'ConditionEffectEntityController': {'Play'}}
if '--party-visuals' in sys.argv:
    out = ROOT.parent / 'tools/party_status_visuals_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'BattleUtility': {'ReselectChangableCondition'},
               'InstantiateManager': {'ChangePlayerMotions'},
               'PlayerConditionEffectEntityController': {'SelectedCharaSize'},
               'ConditionEffectEntityController': {'Play','EntityUpdate'},
               'AnimStateController': {'AnimDying','AnimStop','AnimConfusion'},
               'BattlePlayerEntity': {'ChangeMotionListFromCondition'},
               'SpriteAnimBattle': {'SetupSpriteAnimation'},
               'BattlePlayerAnimatorBase': {'Dying','Down','Sleep','Paralysis','Stop'},
               'BattleEnemyAnimatorBase': {'Dying','UpdateDying'}}
    for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
        cls = re.search(r'^public (?:abstract )?class ([\w.]+)', block, re.M)
        if cls and 'ConditionFunction' in cls[1] and block.startswith('Last.Battle\n'):
            targets[cls[1]] = {'Start','StartSub','EntityUpdate','GetSkinColorGroup','Remove','RemoveSub'}
if '--world-conditions' in sys.argv:
    out = ROOT.parent / 'tools/world_event_conditions_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'Current': {'CheckChild','CheckConditionCommon'}, 'External.Corps': {'CorpsController'}}
    for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
        cls = re.search(r'private sealed class ([\w.<>]+)', block)
        if cls and '<CheckConditionCommon>' in block:
            targets[cls[1]] = set(re.findall(r'internal (?:void|bool) (<?[\w]+>b__\d+)\(', block))
if '--effect-readers' in sys.argv:
    out = ROOT.parent / 'tools/party_effect_readers_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'EffectEntityBase': {'PurseEffectPrefab'}, 'ConditionEffectEntity': {'Init','EntityUpdate'},
               'ConditionEffectEntityController': {'Play'},
               'PlayShader': {'SetMaterial','SetMaterialColorSpeed','SetMaterialColor'}}
if '--reaction-readers' in sys.argv:
    out = ROOT.parent / 'tools/party_reaction_readers_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'EffectReactionInspector': {'Init','NonInverseUpdate','EffectUpdate','End'},
               'EffectReactionOverrideColor': {'StartReaction','UpdateReaction','EndReaction'},
               'EffectReactionHollowColor': {'StartReaction'},
               'EffectReaction': {'Init'}}
if '--warehouse' in sys.argv:
    out = ROOT.parent / 'tools/warehouse_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git', '-C', str(out), 'rev-parse', '--show-toplevel'], capture_output=True).returncode != 0
    targets = {'WarehouseController': {'Initialize','Open','Close','SelectCommand','SelectContent','ContentUpdate',
               'SelectingCommandInit','SelectingEntrustTargetInit','SelectingWithdrawTargetInit',
               'SetSelectCommandKeyHelp','SetSelectListKeyHelp','SetPopupKeyHelp'},
               'WarehousePopupController': {'Show','Close','UpdateCotroller','UpdateArrowImage'},
               'WarehousePopupControllerBase': {'SetInitData','AddSelectedCount','TakeSelectedCount'},
               'WarehouseControlleBase': {'CreateState'}}
if '--enterprise' in sys.argv:
    out = ROOT.parent / 'tools/enterprise_flight_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'MapUtility': {'BranchTransportationAction'},
               'FieldController': {'GetCombineLandingGroupMappingData','AirshipSpecialAction','TransportationPlaneLanding','TransportationLanding','TransportationEnabled','ChangeTransportation'},
               'TransportationController': {'SetData','OverrideData','ChangeTransportation'},
               'Transportation': {'SetupInfoData','GetInfoData','LandingGroupValueTable','TransportationGroupValueTable'},
               'TransportationInfo': {'get_Enable','set_Enable'},
               'TransportationEvent': {'OnAirshipSpecialEvent'},
               'Current': {'RemodelEnterprise','RemodelTranportationCommon','TransfarShip','TransfarAirShip','TransfarShipInSubmerged','TransfarAirShipInSubmerged','TransformAnimByShipAndAirShip','TransferTranportationCommon'},
               'MapUIManager': {'OnRiseAirshipButton','OnFallAirshipButton'},
               'FieldPlayerKeyTwoStepController': {'OnKeyDown','SetAction','SwitchSelectAction'}}
if '--gysahl-talk' in sys.argv:
    out = ROOT.parent / 'tools/gysahl_elder_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'EventAction': {'Talk'},
               'EventProcedure': {'EventTalkTask'},
               'EventProcedure.<EventTalkTask>d__44': {'MoveNext'},
               'FieldNonPlayer': {'get_CanAction'},
               'MessageManager': {'GetMessage'},
               'InputTypeMessageProvider': {'GetMessageId'}}
if '--chocobo-circuit' in sys.argv:
    out = ROOT.parent / 'tools/chocobo_circuit_20261026' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'PropertyEntity': {'CheckIfNeedTargetTransportation','CheckTargetTransportationIds'},
               'DataStorage.Flags': {'Get','IsValidIndex'},
               'DataStorage': {'Get','GetFlag','get_Instance'}}
names, wanted = {}, {}
for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
    ns = block.splitlines()[0]
    cls = re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)', block, re.M)
    if not cls:
        continue
    for match in re.finditer(r'^\s*// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(([^\n]*)\)', block, re.M):
        rva = int(match[1], 16)
        label = ns+'.'+cls[1]+'.'+match[2]+'('+match[3]+')'
        names.setdefault(rva, []).append(label)
        if match[2] in targets.get(cls[1], set()) and ('--warehouse' not in sys.argv or ns in ('Last.UI.KeyInput','Last.UI')):
            wanted[label] = rva
literal_file = dump.parent / 'stringliteral.json'
literals = {int(row['address'], 16): row['value'] for row in
            json.loads(literal_file.read_text(encoding='utf-8-sig'))}
ordered = sorted(names)
pe = pefile.PE(str(dll), fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
lines = []
for label, rva in wanted.items():
    at = bisect.bisect_right(ordered, rva)
    end = ordered[at] if at < len(ordered) else rva+16384
    lines.append('\n'+label+' RVA '+hex(rva))
    for ins in decoder.disasm(pe.get_data(rva, min(end-rva, 32768)), base+rva):
        note = ''
        if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
            aliases = names.get(int(ins.op_str, 16)-base, [])
            if aliases:
                note = ' ; '+(' / '.join(aliases) if len(aliases) < 5 else str(len(aliases))+' folded aliases')
        rip = re.search(r'\[rip \+ (0x[0-9a-f]+)\]', ins.op_str)
        if rip:
            literal = literals.get(ins.address+ins.size+int(rip[1], 16)-base)
            if literal is not None:
                note += ' ; literal '+repr(literal)
        lines.append(f'{ins.address:X} {ins.mnemonic} {ins.op_str}{note}')
(out/'native.txt').write_text('\n'.join(lines), encoding='utf-8')
(out/'provenance.json').write_text(json.dumps({
    'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
    'dump_sha256': hashlib.sha256(dump.read_bytes()).hexdigest(),
    'functions': wanted}, indent=2)+'\n')
print(f'FF{number}: {len(wanted)} exact location producers exported outside Git: {out}')
