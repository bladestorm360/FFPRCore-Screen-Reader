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
DLL = Path('C:/Program Files (x86)/Steam/steamapps/common')/(TITLE+' PR')/'GameAssembly.dll'
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
if '--layer-contract' in sys.argv:
    OUT=ROOT.parent/'tools/puzzle_audit_20261030'/f'ff{NUMBER}_layer_contract'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'SwitchLayerEventEntity':{'Init','OnTriggerEnterCustom','OnTriggerStayCustom','PerfectCollisionHit'}}
if '--anim-visibility' in sys.argv:
    # Settle render/collider/action ownership for the exact Tower weight class.
    # The map initializer cues its animation; it does not hide the actor.
    OUT=ROOT.parent/'tools/puzzle_audit_20261030'/f'ff{NUMBER}_anim_visibility'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'FieldEntity':{'Hide','Show','FadeOutFinished'},
             'FieldSpriteEntity':{'Hide','Show'},
             'FieldMapObjectAnimDefault':{'Init','get_CanAction','TakeAction','IsAcceptActionDirection'},
             'FieldMapObjectDefault':{'Hide','get_CanAction'}}
    TARGETS['InteractiveTargetObserver']={'SearchInteractiveArea'}
    TARGETS['FieldPlayerKeyController']={'OnKeyDown'}
    TARGETS['FieldPlayerKeyController.<>c__DisplayClass4_0']={'<OnKeyDown>b__0'}
if '--encounter-position' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_encounter_position'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'External.Misc':{'EncountBoss','CallEncount'},
             'EventProcedure':{'Last.Map.IEventAccessor.EventEncountBoss'},
             'EventProcedure.<EncountTask>d__49':{'MoveNext'},
             'External.Misc.<>c__DisplayClass20_0':{'<EncountBoss>b__0'},
             'EventProcedure.<>c__DisplayClass48_0':{'<Last.Map.IEventAccessor.EventEncountBoss>b__0',
                                                  '<Last.Map.IEventAccessor.EventEncountBoss>b__1'}}
if '--area-local-boundary' in sys.argv:
    # The later Opera retry fixture must not carry an inside-only area receipt
    # out to the world and back through the front entrance without verifying it.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_area_local_boundary'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'DataStorage':{'ClearAreaLocal','ClearMapLocal'},'FieldController':{'MapInitialize'}}
if '--puzzle-control-receipt' in sys.argv:
    # A flagless ship control must use actual native actor state, rather than
    # adjacency or a guessed second switch. Inspect the exact bounded lookup.
    assert NUMBER==5
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff5_control_receipt'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'FieldController':{'GetFieldEntity'},'FieldEntity':{'Hide'}}
if '--opera-backdrop' in sys.argv:
    # Exact unresolved fact: whether these two registered Opera backdrop calls
    # can affect flags, actors, routes or collision before initializer groups.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_opera_backdrop'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'ChangeDistantViewSkyNormal','ChangeDistantViewSkyOther'},
             'FieldController':{'ShowScenery','HideScenery'},
             'MainViewMapRenderer':{'ShowScenery','HideScenery'},
             'BirdViewMapRenderer':{'ShowScenery','HideScenery'},
             'MapMultipleScroll':{'Show','Hide','FadeIn','FadeOut','SetIndexColor'}}
if '--relative-passage' in sys.argv:
    # Exact unresolved fact: operand roles/units for the native relative Move
    # used by Phoenix's two automatic passages. No game execution/capture.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_relative_passage'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Puppet':{'Move','MoveSyncCommon','GetDirection','GetSpeedID'}}
if '--shine-route' in sys.argv:
    # Exact unresolved fact: the native Narshe light-route polyline, coordinate
    # convention and failure test. No runtime capture is required or performed.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_shine_route'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'SetPolyLineAtShineRoute','StopPolyLineAtShineRoute','CheckShineRoute'}}
if '--party-placement' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_party_placement'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'.cctor','DisposePhoenixCave','DisposeKefkasTower'},'FieldController':{'PlacementPartyEntity'}}
if '--event-entry' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_event_entry'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'EventTriggerEntity':{'OnTriggerEnterCustom','OnTriggerStayCustom','EntityOverlap','CanEnter'},
             'SwitchLayerEventEntity':{'Init','ChangeLayer','CheckPerfectCollisionJudgment','PerfectCollisionHit','OnTriggerEnterCustom','OnTriggerStayCustom'},
             'FootEvent':{'Excute','PreExcute','RequestTriggerAction','RequestTriggerActionAfterCallback'},
             'FootMonitoring':{'ImmediatelyBeforeMoveingAction','MoveEndAction'},
             'FieldPlayer':{'MoveTo'},
             'FieldCharaEntity':{'CanEnter'},
             'FieldController':{'IsCanMoveToDestPosition','GetOnChangeLayerPosition'},
             'FieldPlayerKeyController':{'OnTouchPadCallback'}}
if '--opera-guide' in sys.argv:
    # Settle exact choice construction/order, native lifecycle and UI-ready
    # phase before adding read-only guidance during the performance event.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_opera'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'OperaController':{'SelectMessage','Play','Destory'},
             'OperaController.<SelectMessage>d__81':{'MoveNext'},
             'OperaMessageSelectClient':{'Play','Finish','IsSelecting','GetSelectIndex'},
             'MessageChoiceWindowManager':{'PlayOpera'},
             'MessageMultipleController':{'SetEnable','SetForcusIndex','Close','Show','SetContentValues'}}
if '--zone-eater-roof' in sys.argv:
    # Exact ceiling scroll calls invoked by the unbound0850 room-cycle script.
    # Establish its visual/phase owner before any new observation or guide.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_zone_eater_roof'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'LayerScrollDownZoneEater','LayerScrollUpZoneEater','LayerScrollInitZoneEater'}}
if '--zone-eater-timer' in sys.argv:
    # The ceiling script arms this native five-second timer only after raising.
    # Inspect exact timer replacement/removal and native update ownership.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_zone_eater_timer'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'TimerInstanceManager':{'SetGrobalTimer','RemoveGrobalTimer','Update'}}
if '--narshe-membership' in sys.argv:
    # Exact unresolved fact: a bounded, read-only source of current membership
    # for six Locke doors. Do not call the allocating native clone in the mod.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_narshe_membership_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'UserDataManager':{'get_corpsList','GetCorpsListClone','GetMemberData'},
             'OwnedCharacterData':{'get_characterStatusId'},'Corps':{'get_CharacterId'}}
if '--narshe-close-view' in sys.argv:
    # The exact opaque initializer operation blocking Narshe projection is
    # ShowCloseView1. Establish its scenery-only effect before a local no-op.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_narshe_close_view_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'ShowCloseView1'}}
if '--narshe-scenery-view' in sys.argv:
    # Exact two concrete renderer consumers of the identified scenery message.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_narshe_scenery_view_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={}
if '--narshe-scroll-show' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_narshe_scroll_show_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'MapMultipleScroll':{'Show'}}
if '--narshe-member-lookup' in sys.argv:
    # Establish the owned-ID match used by GetMemberData without calling its
    # allocating clone/closure path. Preserve the earlier six-function archive.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_narshe_member_lookup_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'OwnedCharacterData':{'get_Id'}}
if '--phantom-character-branch' in sys.argv:
    # Exact unknown: BranchByCharacter jump operand used before the train's
    # unconditional boss group. Offline native inspection only, no new hook.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_phantom_character_branch'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'External.Branch':{'BranchByCharacter'}}
if '--darills-tomb' in sys.argv:
    # Exact epitaph choice order/result ownership, read from the matching PE.
    # No runtime probe or recovered implementation inside a Git tree.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_darills_tomb'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'ShowSelectDarillsTomb','SelectedDarillsTomb','DarillsTomb'}}
if '--owzer-gimmick' in sys.argv:
    # Exact asynchronous door mechanism: which state/colliders are changed by
    # start, stop and the native cycle callback. Static inspection only.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_owzer_gimmick'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'SetGimmickTimerOwzer','RemoveGimmickTimerOwzer'},'Current.<>c':{'<SetGimmickTimerOwzer>b__742_0'}}
if '--owzer-owner' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_owzer_owner'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'TimerInstanceManager':{'UpdateTimer','AddGimmickTimer'},'Puppet':{'LookAt'},
             'PropertyColliderEntity':{'ConvertPropertyToData'},'MapUtility':{'SetBoxCollision','GetCollisionRect'}}
if '--sealed-gate-gimmick' in sys.argv:
    # Exact bridge cycle, trigger and timer ownership for maps407/408.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_sealed_gate_gimmick'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'.cctor','SetGimmickTimerEvent0443','RemoveGimmickTimerEvent0443',
                       'SetGimmickTimerEvent0444','RemoveGimmickTimerEvent0444',
                       'ExecGimmick443','ExecGimmick444','ExecFootScriptFromGimmmick'},
             'Current.<>c':{'<SetGimmickTimerEvent0443>b__734_0','<SetGimmickTimerEvent0444>b__737_0'}}
if '--collider-rect' in sys.argv:
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/collider_rect'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'MapUtility':{'GetCollisionRect','SetBoxCollision'},'FieldColliderEntity':{'Init','CanEnter'}}
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
if '--floating-continent-guide' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_floating_continent_20261103/native'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'SetupShadowTimer','ResetShadowTimer','ChangeScriptWhenTimeOut',
                       'ShowDistantView1AtFloatingContinent','ShowDistantView2AtFloatingContinent',
                       'FadeOutDistantView1AtFloatingContinent'}}
if '--floating-continent-timer' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_floating_continent_20261103/timer_callback'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current.<>c':{'<SetupShadowTimer>b__711_0'}}
if '--floating-foot-opening' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_floating_continent_20261103/foot_opening'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'FootEvent':{'SetupOpenTriggerObstacles','SetupActivateTriggerObstacles','SetupTriggerObstacles'},
             'EventTriggerEntity':{'Init','get_Gimmick','get_MapX','get_MapY'}}
if '--puppet-layer-contract' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_puppet_layers_20261103'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Puppet':{'SetPos','Lerp','MoveEntityLayerWithWorldPosition'},
             'MapUtility':{'GetLayerByInt','GetOnLayerZOrderByLayer','GetLayerByZOrder'}}
if '--puppet-layer-callbacks' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/ff6_puppet_layers_20261103/callbacks'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Puppet':{'GetChangeLayerValue'},
             'Puppet.<>c__DisplayClass39_0':{'<Lerp>b__0'}}
if '--item-target-entry' in sys.argv:
    OUT=ROOT.parent/'tools/item_target_entry_20261103'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'BattleTargetSelectController':{'PlayerInit','SideAttackPlayerInit','EnemySideAttackPlayerInit','GetPlayerTarget'}}
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
if '--keyboard-conflicts' in sys.argv:
    OUT=ROOT.parent/'tools/keyboard_bindings_20261104'/f'ff{NUMBER}'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'KeyConfig':{'InputKeyCheck','GetKeyValues','SetKey'},
             'KeyConfigSetting':{'GetKeyboardDefaultKeyConfig'},
             'InputListener':{'get_KeyConfig'},'Key':{'.ctor'}}
if '--south-figaro-disguise' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_South_Figaro/native_disguise'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'BattleFunction52FF6':{'StealItem','AddStealItem','UpdateParameter'},
             'DisguiseMerchantConditionFunction':{'Start','Remove'},
             'DisguiseSoldierConditionFunction':{'Start','Remove'}}
if '--south-figaro-disguise-receipt' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_South_Figaro/disguise_receipt'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'CheckLockDisguise'},'Current.<>c':{'<CheckLockDisguise>b__1165_0','<CheckLockDisguise>b__1165_1'}}
if '--palace-select' in sys.argv:
    # Exact native choice-key/current-instruction ownership, first-clock and
    # program-counter timing for the banquet. No game execution or broad capture.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Imperial_Palace/native_select'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'External.Message':{'Select'},'Core':{'IsFirstClock','get_op','get_pc'},
             'MainCore':{'Execute'},'MessageSelectClient':{'Play'}}
if '--palace-core' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Imperial_Palace/native_core'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Core':{'Execute','GetNextMnemonic'}}
if '--serpent-trench' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Crescent_Context/native_fork'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'SelectInSerpentTrench'}}
if '--serpent-controls' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Crescent_Context/native_controls'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'ChoiceDirectionClient':{'Play','Finish','GetDirection'},
             'ChoiceDirectionController':{'Play','Finish','SelectingInit','SelectingUpdate','UpdateDirectionArrow'},
             'ChoiceDirectionControllerBase':{'Play','Finish','GetDirection'}}
if '--dream-replacement' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Dream_Context/native_replacement'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'ReplaceEntitySlot2Leader','ReplaceEntitySlot3Leader','ReplaceEntitySlot2Slot3Leader','ResetReplaceEntity'}}
if '--dream-fury' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Dream_Context/native_fury'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'BattleActFury','CheckSoulSaver','SaveSoulSaverPos'},
             'Current.<>c':{'<CheckSoulSaver>b__1108_0'},
             'FuryConditionFunction':{'Start','Remove'},
             'FuryConditionFunction.<>c':{'<Start>b__1_1','<Start>b__1_2'}}
if '--opera-repeat-move' in sys.argv:
    # Resolve the completed lever's exact final cell/layer. In particular,
    # the third Move operand must not be guessed to change a layer.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Late_Opera_Context/native_move'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Puppet':{'Move','SetPos'}}
if '--kolts-placement' in sys.argv:
    # One essential unresolved fact: the relocated Vargas actor's actual
    # layer,not its off-map Upper spawn. No live capture or game invocation.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Mt_Kolts_Context/native_placement'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Puppet':{'MoveEntityLayerWithWorldPosition','SetPos','Lerp'}}
if '--pyramid-gimmicks' in sys.argv:
    # Source-specific FFV timed floor controls; no game/probe execution.
    assert NUMBER==5
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_Pyramid_Context/native_gimmicks'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'SetupPyramid8FGimmick','ResetPyramid8FGimmick','SwitchPyramid8FGimmick1','SwitchPyramid8FGimmick2','SetupPyramid3FGimmick','ResetPyramid3FGimmick','SwitchPyramid3FGimmick1','SwitchPyramid3FGimmick2'},'Current.<>c':{'<SwitchPyramid8FGimmick1>b__389_0','<SwitchPyramid8FGimmick1>b__389_1','<SwitchPyramid8FGimmick2>b__390_0','<SwitchPyramid8FGimmick2>b__390_1','<SwitchPyramid3FGimmick1>b__397_0','<SwitchPyramid3FGimmick1>b__397_1','<SwitchPyramid3FGimmick2>b__398_0','<SwitchPyramid3FGimmick2>b__398_1'}}
if '--pyramid-return' in sys.argv:
    # Settle the actual tbr save/restore continuation,not merely the new
    # script's entry. Also pin only this Current initializer's control arrays.
    assert NUMBER==5
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_Pyramid_Context/native_return'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'Current':{'ChangeScript','.cctor'},'Integrator':{'ChangeScript','Update'},'Integrator.HookedProcess':{'.ctor','Restore'}}
if '--jidoor-lamp' in sys.argv:
    # Exact unresolved fact: this scriptless pre-collapse painting's Talk
    # action must be distinguished from the later mansion's light puzzle.
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Zozo_Jidoor_Context/native_lamp'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'EventAction':{'Talk'},'EventProcedure':{'EventTalk','CreateCustomEventTask'},
             'EventProcedure.<EventTalkTask>d__44':{'MoveNext'}}
if '--imperial-party' in sys.argv:
    assert NUMBER==6
    OUT=ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Imperial_Travel_Context/native_party'
    OUT.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    TARGETS={'External.Branch':{'BranchByCharacter','BranchByNumOfMember'},
             'External.Branch.<>c__DisplayClass11_0':{'<BranchByNumOfMember>b__0'}}
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
        if m[2] in TARGETS.get(cls[1], set()) or ('--ally-targets' in sys.argv and cls[1].startswith('BattleTargetSelectController.') and '<PlayerUpdate>' in m[2]) or ('--narshe-member-lookup' in sys.argv and cls[1].startswith('UserDataManager.') and m[2]=='<GetMemberData>b__0') or ('--narshe-scenery-view' in sys.argv and m[2]=='ShowScenery'):
            targets[label] = rva
ordered = sorted(names)
pe = pefile.PE(str(DLL), fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
output = []
for name, rva in targets.items():
    end = next((v for v in ordered if v > rva), rva + 16384)
    output.append('\n' + name + ' RVA ' + hex(rva))
    for ins in md.disasm(pe.get_data(rva, min(end-rva, 262144 if '--party-placement' in sys.argv else 32768)), pe.OPTIONAL_HEADER.ImageBase+rva):
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
