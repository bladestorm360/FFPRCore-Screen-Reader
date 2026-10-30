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
if '--doga-surface' in sys.argv:
    out = ROOT.parent / 'tools/doga_surface_20261029' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'External.Misc': {'ChangeMap'}, 'External.Vehicle': {'SetVehicle','OnTheVehicle'}, 'Puppet': {'SetPos','MoveEntityLayerWithWorldPosition'},
               'FieldController': {'CanLandToWorldMap','UpdateStateSwitchLandable','Last.Map.IMapAccessor.SetPositionToTransportation','Last.Map.IMapAccessor.ConversionMapRangeCell'}}
if '--dive-guidance' in sys.argv:
    out = ROOT.parent / 'tools/dive_guidance_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldController': {'CanLandToWorldMap','UpdateStateSwitchLandable','GetCellAttribute','InterruptStartLandingRunScript','TransportationPlaneLanding'},
               'TransportationEvent': {'.ctor','OnStartLanding','IsLandingEvent','IsDiving','OnChange'},
               'TransportationInfo': {'CheckLandingList'},
               'MapAccessor': {'GetAttribute','GetMapData'},
               'MapUtility': {'GetAttribute'}}
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
# Separate archive for the user-reported confirm/empty/return command episode.
# Keep the original October26 exports intact; all game-bearing output stays outside Git.
if '--warehouse-command' in sys.argv:
    out = ROOT.parent / 'tools/warehouse_command_20261027' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'WarehouseController': {'SelectCommandFunc','SelectCommandInit','SelectingCommandInit',
               'SelectEntrustTargetInit','SelectWithdrawTargetInit','<SelectingCommandInit>b__29_0'},
               'WarehouseView': {'SetNoContentsMessage','SetNoContensMessage'}}
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
# Exact native landing predicates for the user-reported early Enterprise landing.
# Separate source-bearing archive preserves the prior flight research.
if '--enterprise-landing' in sys.argv:
    out = ROOT.parent / 'tools/enterprise_landing_20261027' / (f'ff{number}' + ('-detail' if '--landing-detail' in sys.argv else ''))
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldController': {'CanLandToWorldMap','TransportationPlaneLanding','TransportationLanding','GetCombineLandingGroupMappingData'},
               'TransportationEvent': {'CanLand'},
               'Transportation': {'CanLand'}}
    if '--landing-detail' in sys.argv:
        targets = {'FieldController': {'IsDuplicateTransportationAll','IsDuplicateCollisionEntityForLanding'},
                   'TransportationController': {'CheckLandingList'}}
if '--temple-vehicle' in sys.argv:
    out = ROOT.parent / 'tools/temple_vehicle_20261027' / (f'ff{number}' + ('-table' if '--temple-table' in sys.argv else '-init' if '--temple-init' in sys.argv else ''))
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'TransportationController': {'get_CurrentTransportation','IsPlayer','GetPlayerId','ClearCurrentTransport','SetStayMapId','OverrideData','ClearTemporary','Last.Map.ITransportation.GetCurrentTransportationId'},
               'FieldController': {'CreateFieldPlayer','SetControlPlayer','InitPlayerStatePlay'},
               'FieldPlayer': {'GetOn','GetOff','get_MoveState'},
               'FieldController.<LastSettings>d__174': {'MoveNext'}}
    if '--temple-init' in sys.argv:
        targets = {'FieldPlayer': {'Init','ChangeMoveState'},
                   'FieldController': {'ChangeAssetOfPlayer','MapSetup','InitPlayerStatePlay'}}
    if '--temple-table' in sys.argv:
        targets = {'FieldPlayer': {'.ctor'}, 'TransportationController': {'AutomationSelectId'}}
if '--scan-display' in sys.argv:
    out = ROOT.parent / 'tools/scan_display_20261027' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'ScanBattleFunction': {'HitCalc','IsFunctionEnd','AddDisplayMessages','DefaultAddMessage','AddHpMessageId','AddMpMessageId','AddLevelMessageId','IsInvalidScan','AddWeakPointMessageId'},
               'BattleCommandMessageController': {'SetMessage','SetSystemMessage'},
               'BattleMenuController': {'SetCommadnMessage'},
               'BattleUIManager': {'SetCommadnMessage','SetSystemMessage'}}
if '--results-counter' in sys.argv:
    out = ROOT.parent / 'tools/results_counter_20261027' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'ResultPointController': {'PointsUpdate','UpdateController'},
               'ResultCharacterListController': {'ForcedEndPerformance','PlayPerformance','<PlayPerformance>b__6_0'},
               'ResultCharacterListContentController': {'ForcedEndPerformance'},
               'ResultMenuController': {'ShowPointsUpdate'}}
if '--npc-area' in sys.argv:
    out = ROOT.parent / 'tools/npc_area_20261027' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldNonPlayerAreaMoveController': {'WithinRange','SearchDestPositionListInRange','.ctor'},
               'FieldNonPlayerControllerManager': {'SetupNonPlayerControllerList','FactoryFieldNonPlayerController'}}
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
if '--ability-feedback' in sys.argv:
    out = ROOT.parent / 'tools/ability_visible_feedback_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'BattleBasicFunction': {'ViewEntity','CreateViewEntity','CreateHitMessage'},
               'EffectEntityController': {'Init','Play','PlayMagic','PlayTargets','PlayEffect','PlayFixedEffect','PlayBattleEffect'},
               'EffectController': {'Play','CreateEffectAsync','ExecuteDoNotEffect'},
               'EffectController.<CreateEffectAsync>d__26': {'MoveNext'},
               'EffectEntity': {'Init','InitEffect'},
               'AddConditionFunctionFF3': {'Calc','UpdateParameter'},
               'AttractConditionFunction': {'Start'},
               'BattleFunction46': {'SetRandomAbility','CreateAbility'},
               'FF3AccumulateConditionFunction': {'Start','Update'},
               'AccumulateConditionFunction': {'Start'}}
if '--vehicle-state' in sys.argv:
    out = ROOT.parent / 'tools/vehicle_state_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldController': {'ChangeTransportation','TransportationDownloadData','CreateFieldPlayer'},
               'FieldCharaEntity': {'get_IsRiging','RideOn','RideOut'},
               'EventActionScript': {'IsRidingVehicle','Start','End'},
               'FieldPlayer': {'GetOn'},
               'TransportationController': {'ChangeTransportation','Last.Map.ITransportation.SetupCharaAsset'},
               'Integrator': {'IsRidingVehicle','InitializePlayerPuppetEntities'}}
if '--save-context' in sys.argv:
    out = ROOT.parent / 'tools/saronia_capture_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'RijndaelUtil': {'Encrypt','Decrypt'},
               'FileOperationUtility': {'FileRead','Encrypt'},
               'SaveSlotManager': {'CloudConvertSlotData','CloudConvertEncryptData'},
               'ZipOperation': {'Decompress','DecompressToString'}}
if '--relay-origin' in sys.argv:
    out = ROOT.parent / 'tools/map_event_audit_20261028' / f'ff{number}_relay_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'MapUtility': {'ParseMapObjectGroupData'}, 'MapObjectGroupData': {'.ctor'},
               'FieldEntity': {'SetProperty'}, 'EventTriggerEntity': {'IsTriggerEnable','OnTriggerEnterCustom'},
               'FieldMapObjectDefault': {'OnTriggerEnterCustom'},
               'PropertyEntity': {'CheckIfNeedTargetTransportation','CheckTargetTransportationIds'}}
if '--transport-guards' in sys.argv:
    out = ROOT.parent / 'tools/map_transport_audit_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'PropertyEntity': {'.ctor','ConvertPropertyToData','CheckIfNeedTargetTransportation','CheckTargetTransportationIds'},
               'FieldColliderEntity': {'Init','SetProperty','CanEnter','OnTriggerEnterCustom'},
               'FieldEntity': {'OnTriggerEnterCustom','CheckHit','CanEnter','SetProperty','CacheActive','RestoreCacheActive','Show','Hide'},
               'FieldMapObjectDefault': {'OnTriggerEnterCustom','CanEnter'},
               'EventTriggerEntity': {'IsTriggerEnable','OnTriggerEnterCustom','Show','Hide','TriggerActivate'},
               'FieldPlayer': {'ChangeTransportation','GetOn','GetOff'}}
if '--auto-transport-order' in sys.argv:
    # Resolve the exact collision/seamless-mode sequence before applying native
    # transportation-ID guards to future walking/canoe legs. Offline PE only.
    out = ROOT.parent / 'tools/map_transport_audit_20261028' / f'ff{number}_auto_order'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'TransportationController': {'AutomationSelectId','SeamlessHitCheck','CheckLandingAutoMove','ChangeTransportation'},
               'FieldController': {'OnPlayerHitCollider','OnImmediatelyBeforeMoveing','OnStepTransportationEventTrigger','ChangeTransportation'},
               'EventTriggerEntity': {'EntityOverlap'},
               'EventProcedure': {'EventTransportation','EventTransportationLanding'},
               'Transportation': {'EventTransportation','EventTransportationLanding','EventTransportationMove','EventTransportationChange'},
               'Transportation.<EventTransportationMove>d__36': {'MoveNext'},
               'Transportation.<EventTransportationChange>d__37': {'MoveNext'},
               'FieldEntity': {'CheckHit'}, 'FieldPlayer': {'SquareMoveFinished'}}
if '--trigger-lifetime' in sys.argv:
    out = ROOT.parent / 'tools/map_transport_audit_20261028' / f'ff{number}_trigger_lifetime'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'EventTriggerEntity': {'set_TriggerPower','EntityOverlap','Reset','Suspend','Hide','OnTriggerExitCustom','SetProperty'},
               'EventSwitchTriggerEntity': {'EntityOverlap','Suspend'},
               'FootEvent': {'Excute','RequestTriggerAction','SetupActivateTriggerObstacles'},
               'FieldController': {'MapSetup','PauseMap','WorldActionCommand'}}
    # FFVI dispatches trigger lifetime methods virtually, so direct-call xrefs
    # alone miss its producers. Inspect their same exact known native owners.
    for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: '):
        match=re.search(r'\bclass (Transportation\.<EventTransportationMove>d__\d+)\b',block)
        if match:targets[match[1]]={'MoveNext'}
if '--switch-power' in sys.argv:
    assert number==6
    out = ROOT.parent / 'tools/conditional_arrivals_20261029/ff6_switch_power'
    out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    targets={'EventSwitchTriggerEntity':{'OnTriggerEnterCustom','OnTriggerExitCustom','EntityOverlap','SetProperty','Suspend','LeaveTriggerFinished','IsLeaveScript'},
             'FootEvent':{'RequestTriggerAction'},'PropertyEventSwitch':{'ConvertPropertyToData'}}
if '--conditional-door' in sys.argv:
    assert number==5
    out=ROOT.parent/'tools/conditional_arrivals_20261029/ff5_door_cleanup'
    out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    targets={'Current':{'ResetJacholCaveGimmick','ResetSwitchJacholCave'}}
if '--teleport-landing' in sys.argv:
    assert number==4
    out=ROOT.parent/'tools/conditional_arrivals_20261029/ff4_devil_road_native'
    out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    targets={'Puppet':{'SetPos','GetDirectTargetPosition','GetTargetEntityPosition'}}
if '--script-ownership' in sys.argv:
    assert number in(2,4,5)
    out=ROOT.parent/'tools/final_script_cases_20261029'/f'ff{number}_ownership'
    out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    targets={'FieldController':{'GetScriptData'},'MapManager':{'GetScriptData'},
             'MapModel':{'GetScript'},'MapAssetData':{'GetScript'}}
if '--trigger-closure' in sys.argv:
    assert number==6
    out=ROOT.parent/'tools/final_script_cases_20261029/ff6_trigger'
    out.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    targets={'FieldController':{'Last.Map.IMapAccessor.IsDuplicateEntityOnTrigger'}}
if '--boot-dependencies' in sys.argv:
    out = ROOT.parent / 'tools/startup_hang_20261028' / f'ff{number}_dependencies'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FadeManager': {'FadeOut'},'GameStateTracker': {'Initialize'},'GameStateUpdater': {'Initialize'},
               'SerialTypeManager': {'Initialize'},'SteamSDKManager': {'Initialize'},
               'ProviderManager': {'Initialize'},'SystemConfig': {'Initialize'}}
if '--mountain-crossing' in sys.argv:
    out = ROOT.parent / 'tools/map_scripted_entries_20261028' / f'ff{number}_mountain_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'TransportationEvent': {'IsOverTheMountain','IsFrontMountain','OnAirshipSpecialEvent','OnHitCollider','ResetHitEventParam'}}
if '--script-change' in sys.argv:
    out = ROOT.parent / 'tools/map_event_audit_20261028' / f'ff{number}_script_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'Integrator': {'ChangeScript'}, 'External.Misc': {'ChangeScript','ChangeScriptAtOnTrigger'},
               'FieldController': {'GetFieldEntity'}}
if '--ability-targets' in sys.argv:
    out = ROOT.parent / 'tools/ability_targets_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'BattleBaseFunction': {'Init'}, 'BattleBasicFunction': {'CreateDamageView','CreateViewEntity','ViewEntity'},
               'BattleFunction46': {'CreateAbility','CreateViewEntity'},
               'BattleFunctionController': {'Update','Init','FunctionEnd'},
               'EffectEntityController': {'Play','PlayTargets','PlayFixedEffect','IsEffectEnd'},
               'EffectController': {'Play'}}
if '--turn-order-display' in sys.argv:
    out = ROOT.parent / 'tools/turn_order_display_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'BattleInfomationController': {'Initalize','Show'},
               'BattlePlayerInfomationController': {'Initalize','UseATB','UpdateView','CreateContents'},
               'BattlePlayerInfomationContentView': {'UseATBView','UseCommandView'},
               'SystemConfig': {'get_BattleType'},'SystemConfigData': {'GetBattleType'}}
if '--vehicle-map-identity' in sys.argv:
    out = ROOT.parent / 'tools/vehicle_map_identity_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'TransportationController': {'SetMapObject','SetData','CreateProperty','SwitchActiveToAllExistingTransportation'},
               'Transportation': {'GetTransportation'},
               'FieldController': {'CreateTransportationEntity'},
               'PropertyTransportation': {'ConvertPropertyToData'}}
if '--item-pages' in sys.argv:
    out = ROOT.parent / 'tools/item_buffer_pages_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'ItemEquipmentController': {'SetPage','CreateViews','UpdateView','SetDescriptionText'},
               'ItemEquipableController': {'Initalize','UpdateView','StartIdle'},
               'ItemEquipmentDetailController': {'StartIdle'},
               'ItemEquipableContentController': {'Initalize','UpdateView'},
               'ItemEquipableContentView': {'SetName','UseNameText','UseCharaImage'},
               'ItemEquipmentController.<StartPrevPage>d__22': {'MoveNext'},
               'ItemEquipmentController.<StarNextPage>d__23': {'MoveNext'},
               'ItemEquipableController.<StartOutLeft>d__10': {'MoveNext'},
               'ItemEquipableController.<StartOutRight>d__11': {'MoveNext'},
               'ItemEquipableController.<StartInLeft>d__12': {'MoveNext'},
               'ItemEquipableController.<StartInRight>d__13': {'MoveNext'}}
if '--field-effects' in sys.argv:
    out = ROOT.parent / 'tools/map_event_audit_20261028' / f'ff{number}_effect_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldEffectBoardEntity': {'Init','Show','Hide'},
               'FieldEffectParticleEntity': {'Init','Show','Hide'},
               'FieldEntity': {'SetProperty','Show','Hide','SetBoxCollider','CanEnter'},
               'FieldCharaEntity': {'CanEnter','UpdateRaycast','GetClosestRaycastTarget'},
               'FieldPlayer': {'CanEnter','UpdateRaycast'},
               'PropertyEffectEntity': {'ConvertPropertyToData','.ctor'},
               'MapManager': {'CreateEntity'}}
if '--scripted-sea-entry' in sys.argv:
    out = ROOT.parent / 'tools/world_event_capture_20261029' / f'ff{number}_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'External.Vehicle': {'SetVehicle'},
               'EventTriggerEntity': {'Init','CanAction','IsTriggerEnable','OnTriggerEnterCustom'},
               'PropertyTransportationEventAction': {'.ctor'},
               'MapManager': {'CreateEntity'}}
if '--invincible-artillery' in sys.argv:
    out = ROOT.parent / 'tools/invincible_artillery_20261029' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'ActFunctionProvider': {'ViewMessage','HitViewMessage'},
               'ContentUtitlity': {'GetMesIdMessageConclusion','GetMesIdAbilityName'},
               'BattleActExection': {'InitActionState','EndFinishingState','HitViewMessage'}}
if '--field-effect-flight' in sys.argv:
    # Exact remaining effect-collision applicability question: who changes the
    # player's native raycast/layer when a flight model is boarded? PE only.
    out = ROOT.parent / 'tools/map_event_audit_20261028' / f'ff{number}_effect_flight'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldController': {'ChangeTransportation','ChangeTransportationCollisionEntity','ChangeTransportationSwitchEntity','ChangeViewBird'},
               'FieldPlayer': {'Init','GetOn','GetOff','MoveTo','UpdateEntity'}, 'FieldCharaEntity': {'Init','UpdateRaycast'},
               'TransportationController': {'ChangeTransportation','CollisionEnable'},
               'FieldEntity': {'SetCollisionEnabled','SwitchCollisionEnable','MoveTo','UpdateEntity'},
               'FieldPlayerController': {'Update'},
               'FieldPlayerKeyAirshipController': {'MoveToAxis','Init'},
               'FieldPlayerKeySpecialAirshipController': {'Init'},
               'FieldSpriteEntity': {'MoveTo'},
               'BirdViewController': {'SetCharacterAsset','SetMoveCallback'}}
if '--command-commit' in sys.argv:
    out = ROOT.parent / 'tools/command_commit_20261029' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'BattleCommandSelectController': {'SetCursor','CursorClear','FocusClear','NormalUpdate','SetCommandData','CreateState','Initalize'},
               'BattleInfomationController': {'OnCommandSelect','CommandDone','CommandSelectInit','ClearCommandSelected'},
               'BattleMenuController': {'CommandSelectedExit','SetActiveCommandInputWindow','CommandSelectingInit','<CommandSelectingInit>b__71_0','<CommandSelectingInit>b__73_0','<CommandSelectingInit>b__72_0','<CommandSelectingInit>b__77_0'},
               'BattleCommandInputController': {'CommandSelectedInit','CommandSelectInit','CommandSelectUpdate','CommandSelectedUpdate'}}
if '--npc-route-context' in sys.argv:
    out = ROOT.parent / 'tools/npc_geometry_context_20261029' / f'ff{number}_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'External.Misc': {'AssignMoveRoute'},
               'FieldController': {'SetRouteMoveEntityInstruction'},
               'FieldNonPlayerRouteMoveInstructionController': {'.ctor','OnDestroy','get_IsMovingType','UpdateController','NextInstruction'},
               'MoveToInstruction': {'MoveTo'},
               'MoveLerpInstruction': {'Execute','ChangeLayer'},
               'External.Puppet': {'SetPuppet'},
               'Puppet': {'MoveTo','Route','GetEntity','GetDirectTargetPosition','GetTargetEntityPosition'},
               'PropertyPolyline': {'SetUpPolyLineRoute'},
               'FieldNonPlayerControllerManager': {'FactoryFieldNonPlayerController','SetUpPolyLineRoute','CreateRouteMoveInstructionController','GetTargetController'}}
if '--npc-entry-context' in sys.argv:
    out = ROOT.parent / 'tools/npc_geometry_context_20261029' / f'ff{number}_entry_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'Current': {'ShowNearMaskImperialCamp','HideNearMaskImperialCamp',
                           'ExpandNearMaskImperialCamp','ReduceNearMaskImperialCamp','.cctor'},
               'FieldController': {'ScalingAlphaMask'},
               'MainViewMapRenderer': {'ScalingAlphaMask'},
               'ForegroundAlphaMask': {'ScalingAlphaMask','ChangeMaskScale','UpdateScroll'}}
if '--npc-region-context' in sys.argv:
    out = ROOT.parent / 'tools/npc_geometry_context_20261029' / f'ff{number}_region_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'FieldNonPlayerAreaMoveController': {'SearchDestPositionListInRange','UpdateController','WithinRange','.ctor'},
               'FieldNonPlayerRouteMoveController': {'ExtractMoveDestPositionList','UpdateController','WithinRange','.ctor'},
               'FieldNonPlayerController': {'Setup','SearchMovableDirList','WithinRange'},
               'PropertyAreaObject': {'ConvertPropertyToData'},
               'PropertyMoveArea': {'ConvertPropertyToData'},
               'PropertyPolyline': {'SetUpPolyLineRoute','ConvertPropertyToData'},
               'FieldNonPlayerControllerManager': {'FactoryFieldNonPlayerController','GetPropertyMapArea','GetPropertyPolyline'}}
if '--arrival-selection' in sys.argv:
    out = ROOT.parent / 'tools/conditional_arrivals_20261029' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'MapManager': {'SearchPointInEntity','SearchEntity','CreateFieldPlayer'},
               'MapModel': {'GetEntitysDefault'},
               'PropertyPointIn': {'ConvertPropertyToData'},
               'FieldController': {'CreateFieldPlayer'}}
if '--sheep-entry' in sys.argv:
    out = ROOT.parent / 'tools/npc_geometry_context_20261029' / f'ff{number}_sheep_entry_native'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'Puppet': {'Lerp','SetPos','GetDirectTargetPosition','GetTargetEntityPosition'}}
if '--boost-use' in sys.argv:
    out = ROOT.parent / 'tools/boost_use_20261028' / f'ff{number}'
    out.mkdir(parents=True, exist_ok=True)
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode != 0
    targets = {'BattleActExection': {'InitActionState'}, 'BattleFunction46': {'CreateAbility'}, 'ActFunctionProvider': {'ViewMessage','HitViewMessage'},
               'ContentUtitlity': {'GetMesIdBattle','GetMesIdAbilityName'},
               'BattleUIManager': {'SetCommandText'}, 'BattleMenuController': {'SetCommandText'}}
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
        allowed_namespace = '--results-counter' not in sys.argv or ns in ('Last.UI.KeyInput', f'Serial.FF{0 if number==4 else number}.UI.KeyInput')
        allowed_namespace &= '--ability-feedback' not in sys.argv or ns.startswith('Last.Battle') or cls[1]=='EffectController.<CreateEffectAsync>d__26'
        allowed_namespace &= '--turn-order-display' not in sys.argv or ns.startswith('Last.UI') or ns=='Last.Management' and cls[1]=='SystemConfig' or ns==f'Serial.FF{number}.Management' and cls[1]=='SystemConfigData'
        allowed_namespace &= '--item-pages' not in sys.argv or ns == 'Last.UI.KeyInput' or not ns and cls[1].startswith(('ItemEquipmentController.<','ItemEquipableController.<'))
        allowed_namespace &= '--arrival-selection' not in sys.argv or ns=='Last.Map' or not ns and cls[1].startswith('MapManager.')
        if '--arrival-selection' in sys.argv and allowed_namespace and cls[1].startswith('MapManager.') and match[2] in ('<SearchPointInEntity>b__0','<SearchEntity>b__0'):
            wanted[label]=rva
        if allowed_namespace and match[2] in targets.get(cls[1], set()) and ('--warehouse' not in sys.argv or ns in ('Last.UI.KeyInput','Last.UI')):
            wanted[label] = rva
literal_file = dump.parent / 'stringliteral.json'
if '--arrival-selection' in sys.argv:
    # Exact terminal helpers reached by SearchPointInEntity in the matching
    # binaries. Archive their bodies outside Git; no guessed generic symbol.
    assert number in (3,5)
    wanted['arrival list selection helper']={3:0x9bcf60,5:0x95ccf0}[number]
literals = {int(row['address'], 16): row['value'] for row in
            json.loads(literal_file.read_text(encoding='utf-8-sig'))}
ordered = sorted(names)
pe = pefile.PE(str(dll), fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
if '--transport-guards' in sys.argv or '--trigger-lifetime' in sys.argv or '--command-commit' in sys.argv or '--arrival-selection' in sys.argv:
    # Exact known guard xrefs, verified at decoded instruction boundaries. This
    # reads PE bytes only and keeps all source-bearing output outside Git.
    import struct
    guards={rva for label,rva in wanted.items() if
            '.PropertyEntity.CheckTargetTransportationIds(' in label or
            '--arrival-selection'in sys.argv and '.MapManager.SearchPointInEntity('in label or
            '--command-commit'in sys.argv and label.startswith('Last.UI.KeyInput.BattleCommandSelectController.CursorClear(') or
            '--trigger-lifetime'in sys.argv and any(token in label for token in ('.EventTriggerEntity.set_TriggerPower(','.EventTriggerEntity.EntityOverlap(','.EventTriggerEntity.Suspend('))}
    candidates={}
    for section in pe.sections:
        if not section.Characteristics & 0x20000000:continue
        data=section.get_data();start=section.VirtualAddress
        for opcode in (b'\xe8',b'\xe9'):
            at=data.find(opcode)
            while at>=0:
                if at+5<=len(data) and start+at+5+struct.unpack_from('<i',data,at+1)[0] in guards:
                    owner=bisect.bisect_right(ordered,start+at)-1
                    if owner>=0:candidates.setdefault(ordered[owner],set()).add(start+at)
                at=data.find(opcode,at+1)
    callers=[]
    for owner,addresses in candidates.items():
        index=bisect.bisect_right(ordered,owner);end=ordered[index] if index<len(ordered) else owner+16384
        for instruction in decoder.disasm(pe.get_data(owner,min(end-owner,32768)),base+owner):
            if instruction.address-base in addresses and instruction.mnemonic in ('call','jmp') and instruction.op_str.startswith('0x') and int(instruction.op_str,16)-base in guards:
                callers.append(dict(owner_rva=owner,owner=names[owner],instruction_rva=instruction.address-base))
                wanted['guard caller: '+' / '.join(names[owner][:3])]=owner
    (out/'callers.json').write_text(json.dumps(callers,indent=2)+'\n',encoding='utf8')
lines = []
for label, rva in wanted.items():
    at = bisect.bisect_right(ordered, rva)
    end = ordered[at] if at < len(ordered) else rva+16384
    if label=='arrival list selection helper':
        # Generic bodies are absent from dump.cs's ordered symbols. Use their
        # exact PE x64 runtime-function span, not the next unrelated dump entry.
        import struct
        directory=pe.OPTIONAL_HEADER.DATA_DIRECTORY[pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_EXCEPTION']]
        spans=pe.get_data(directory.VirtualAddress,directory.Size)
        begin,end,_=next(row for row in struct.iter_unpack('<III',spans)if row[0]<=rva<row[1])
        assert begin==rva
    lines.append('\n'+label+' RVA '+hex(rva))
    for ins in decoder.disasm(pe.get_data(rva, min(end-rva, 32768)), base+rva):
        note = ''
        if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
            aliases = names.get(int(ins.op_str, 16)-base, [])
            if aliases:
                note = ' ; '+(' / '.join(aliases) if len(aliases) < 5 else str(len(aliases))+' folded aliases')
        rip = re.search(r'\[rip ([+-]) (0x[0-9a-f]+)\]', ins.op_str)
        if rip:
            target=ins.address+ins.size+(1 if rip[1]=='+'else -1)*int(rip[2],16)-base
            literal = literals.get(target)
            if literal is not None:
                note += ' ; literal '+repr(literal)
            if ins.mnemonic=='lea' and target in names:
                aliases=names[target]
                note+=' ; callback '+(' / '.join(aliases) if len(aliases)<5 else str(len(aliases))+' folded aliases')
        lines.append(f'{ins.address:X} {ins.mnemonic} {ins.op_str}{note}')
(out/'native.txt').write_text('\n'.join(lines), encoding='utf-8')
(out/'provenance.json').write_text(json.dumps({
    'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
    'dump_sha256': hashlib.sha256(dump.read_bytes()).hexdigest(),
    'functions': wanted}, indent=2)+'\n')
print(f'FF{number}: {len(wanted)} exact location producers exported outside Git: {out}')
