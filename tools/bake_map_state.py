"""Extract navigation-only map-initializer decisions, never dialogue or outcome text.

Input: matching offline mapdump scripts/objects. Output: per-game mapstate.json.
The small program retains flag tests, local shadow writes and entity-group selection.
It cannot call native scripts or change a save. Unknown control flow fails closed.
"""
import csv
import json
import pathlib
import sys
import collections
import hashlib
import bake_maps
import bake_walk_open_doors
import phantom_train_exits

ROOT = pathlib.Path(__file__).absolute().parents[1]
DUMP = ROOT.parent / 'tools' / 'mapdump'
CATEGORIES = {**{f'ScenarioFlag{i+1}': i for i in range(4)},
              **{f'TreasureFlag{i+1}': i+4 for i in range(4)},
              'ScenarioVariable': 8, 'AreaLocalVariable': 9,
              'MapLocalVariable': 10, 'ScriptLocalVariable': 11,
              'MessageSelection': 12, 'ItemSelection': 13, 'imm': -1}
OPERATORS = {'＝': 0, '≠': 1, '＜': 2, '＞': 3, '≦': 4, '≧': 5, '←': 6}
UNSET_POSITION = -2147483648
PATROLS = json.loads((ROOT / 'tools/npc_route_positions.json').read_text(encoding='utf-8'))


def puppet_navigation_effect(script, entry, region, positions=True):
    """Recover final visibility/absolute placement, not animation or story text.

    FF6 Puppet.SetPos uses IMapAccessor.ConversionMapRangeCell before assigning
    cell coordinates (native 2ECBA0). Only its absolute, unchanged-layer form is
    accepted. Branching/moving puppets remain outside this bounded projection.
    """
    rows=script.get('mnemonics') or []
    # All six exact PuppetCore.Ready producers first call actor.Show(0).
    # A new puppet body therefore supersedes an earlier Hide even if it only
    # positions/animates the actor. Native fingerprints: puppet_ready contract.
    visible=1; x=y=UNSET_POSITION; moving=False
    for pc in range(entry,min(len(rows),entry+128)):
        m=rows[pc];op=m['mnemonic'];v=m.get('operands')or{}
        ints=list(v.get('iValues')or[])+[0]*8
        if op in ('Exit','Return'):
            if moving:return [visible,UNSET_POSITION,UNSET_POSITION]
            return [visible,x,y]
        if op=='Hide':visible=0
        elif op=='Show':visible=1
        elif op=='SetPos':
            if not positions:moving=True;continue
            if ints[2]!=0 or ints[3]!=0:moving=True;continue
            if ints[0]>=0:x=ints[0]-region[0]
            if ints[1]>=0:y=ints[1]-region[1]
        elif op in ('Move','Lerp','MoveTo','Route'):moving=True
        elif op not in ('Nop','Wait','CueAnim','PlayAnim','PlaySE','StopSE','SetDirection','SyncSE'):
            return None
    return None


def initializer_code(script,game,region):
    code=[]
    patrols=[p for p in PATROLS if p['game']==game and p['submap']==script.get('submap')]
    if patrols:
        digest=hashlib.sha256(json.dumps(script,ensure_ascii=False,sort_keys=True,separators=(',',':')).encode()).hexdigest()
        assert all(p['source_sha256']==digest for p in patrols)
    # Exact reviewed FFIII exception: cleared statue triggers are parked away
    # from their old corridor. Other non-VI absolute placement remains opaque.
    # Matching SetPos9419A0, MoveEntityLayer93D900 and range conversion285040:
    # docs/re/CRYSTAL_TOWER_APPROACH_20261029.md. No broad layer/position inference.
    statues=game=='ff3' and script.get('submap')=='Map_10040' and script.get('name')=='sc_map_10040'
    if statues:
        digest=hashlib.sha256(json.dumps(script,ensure_ascii=False,sort_keys=True,separators=(',',':')).encode()).hexdigest()
        assert digest=='cb8e2a844de521a5ad58bd0422e5faa62691019d7191d98e32d16371cc674392'
        assert list(region[:2])==[0,0]
    # Exact train initializer: two optional ghost-membership branches converge
    # before its unconditional boss group. Native B5B680 reads jump address
    # iValues[3] and calls Main.JumpCommon. Keep membership unknown and use the
    # existing branch intersection; no party query or guessed group is added.
    train=game=='ff6'and script.get('submap')=='Map_30440'and script.get('name')=='sc_map_30440'
    if train:
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='2d84a8398ab471ad976672f8c41511562c43748a920c6e0979549ae5ad60a83e'
    zozo_magicite=game=='ff6'and(script.get('submap'),script.get('name'))==('Map_30511_12','sc_map_30511_12')
    if zozo_magicite:
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='00b9688aafee3a32b79437df50d7f9a376c93c3889b5b9c45ad60db71f4fdf00'
    kolts=game=='ff6'and(script.get('submap'),script.get('name'))==('Map_30360','sc_map_30360')
    if kolts:
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='87ea43f9eb0b5dfa72d29ac812c8f75505007d22b68c020eca8922fe3e0b84f3'
    tower=game=='ff6'and script.get('submap')=='Map_30700'and script.get('name')=='sc_map_30700'
    if tower:
        # Same verified B5B680 jump operand3. Once392 is set, both unknown
        # membership branches skip the recruitment barrier. Keep membership
        # unknown; do not create a new party reader or enable an absent group.
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='414ca3635e27ea439392931a59cdc12d8a3b628c22984c0d45d1eb70d4814edd'
    owzer_timer = game=='ff6' and (script.get('submap'),script.get('name')) in {
        ('Map_30631_3','sc_map_30631_3'),('Map_30631_4','sc_map_30631_4')}
    if owzer_timer:
        # These exact start/stop registrations only add/remove GimmickOwzer.
        # Its later callback owns the cycling collider changes. Do not let a
        # registration hide the initializer's subsequent groups/puppet facts.
        # Matching native methods 2BD110/2B2A60, callback53F760; Owzer evidence.
        expected={'Map_30631_3':'d5b70dc53b03ac894f30b621f8d423bb845ec7a59293f0b7b6bbd35e0dafe7c0',
                  'Map_30631_4':'8ed56662b207815ce39f1ab5c4a1102315322ba1c620e112978269406111b15c'}
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==expected[script['submap']]
    sealed_timers = game=='ff6' and (script.get('submap'),script.get('name')) in {
        ('Map_30531_2','sc_map_30531_2'),('Map_30531_3','sc_map_30531_3'),
        ('Map_30531_4','sc_map_30531_4')}
    if sealed_timers:
        # Exact registrations only add/remove the native three-second loop.
        # ExecGimmick443/444 owns later phase/actor writes. Keep initializer
        # groups and puppet facts after these calls; never guess a timer phase.
        expected={'Map_30531_2':'f45b5b8dd0b00d4c9072bdff0f98734ef7e0c723331b67047855dded798be338',
                  'Map_30531_3':'1be931bbe6004867e64639bddac40624562199dd9523929a54bab0a648989eff',
                  'Map_30531_4':'567a6da0029a32706186d2937a52b24a7dfdc88b8a8941cb87d18a97d795d5a0'}
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==expected[script['submap']]
    narshe_close = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_20010','sc_map_20010')
    if narshe_close:
        # Exact CloseView1 display: message82 -> FieldController/MainViewMapRenderer
        # -> MapMultipleScroll.Show (scenery activation/particle/color only).
        # It does not make the following six lock visibility branches unknown.
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='f52d6466569c28a5c73cd86a6c761112f3f19ff3dd205202dc342797f8cfcc1d'
    floating = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_30600','sc_map_30600')
    if floating:
        # Three exact backdrop calls only show/hide scenery1/2. SetupShadowTimer
        # registers a later callback; it does not execute its position-dependent
        # rescue script here. Preserve later initializer groups/puppet facts,
        # without inventing an elapsed-time or Shadow-arrival receipt.
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='1b7b0dc3a0a30b2161ae854b501a5fd2378dbb9e9416894f4be6d760e7013239'
    duncan = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_20400','sc_map_20400')
    if duncan:
        # Same source-qualified BranchByCharacter ownership/jump-operand3 as
        # the accepted party-role observer. Read Sabin6 from the active corps;
        # do not replace this with roster ownership or an unconditional group.
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='2cc1f33ee991e80f58073d75930944005f3c11eefb8ef0855efaa990728ed961'
    crescent = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_30491_1','sc_map_30491_1')
    if crescent:
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='2be0d7bfb883c48fbd746bc47278506908fbb5d6619cece5149045cddc6e4572'
    post = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_20240','sc_map_20240')
    if post:
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()=='6c720f180bfc9b6293775d329e0c6568f048ca7b7645aaf1a89edf4e0ff13e6b'
    dream = game=='ff6' and (script.get('submap'),script.get('name'))==('Map_20451_4','sc_map_20451_4')
    if dream:
        proof=json.loads((ROOT/'docs/re/dream_native_20261104.json').read_text(encoding='utf-8'))
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes']['Map_20451_4/sc_map_20451_4']
    dream_alias = game=='ff6' and (script.get('submap'),script.get('name'))in(
        ('Map_30821_1','sc_map_30821_1'),('Map_30821_2','sc_map_30821_2'))
    if dream_alias:
        proof=json.loads((ROOT/'docs/re/dream_native_20261104.json').read_text(encoding='utf-8'))
        assert hashlib.sha256(json.dumps(script['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==proof['source_script_hashes'][script['submap']+'/'+script['name']]
    for m in script.get('mnemonics')or[]:
        op=m['mnemonic'];v=m.get('operands')or{};ints=list(v.get('iValues')or[])+[0]*8
        if narshe_close and op=='SysCall' and (v.get('sValues')or[''])[0]=='近景１：表示':
            code.append([0])
        elif dream_alias and op=='SysCall'and(v.get('sValues')or[''])[0]in(
                'パペット入れ替え：スロット２先頭','パペット入れ替え：スロット３先頭',
                'パペット入れ替え：スロット２・３先頭','パペット入れ替え：解除'):
            # Exact four native exports only install/reset temporary resident
            # puppet aliases. These bodies act on -17/-18, never a baked actor
            # or story/local bank. Preserve the following real group branches;
            # do not infer the off-floor resident character's name or position.
            code.append([0])
        elif owzer_timer and op=='SysCall' and (v.get('sValues')or[''])[0] in ('ギミック開始：アウザーの館','ギミック解除：アウザーの館'):
            code.append([0])
        elif sealed_timers and op=='SysCall' and (v.get('sValues')or[''])[0] in (
                'ギミック開始：sc_e_0443','ギミック解除：sc_e_0443',
                'ギミック開始：sc_e_0444','ギミック解除：sc_e_0444'):
            code.append([0])
        elif floating and op=='SysCall' and (v.get('sValues')or[''])[0] in (
                '魔大陸遠景１のみ表示','魔大陸遠景１フェードアウト','シャドウタイマー設定'):
            code.append([0])
        elif duncan and op=='BranchByCharacter':
            assert ints[:4]==[6,1,0,6]
            code.append([2,15,6,0,-1,0,6])
        elif crescent and op=='BranchByCharacter':
            assert ints[:4]==[12,1,0,23]
            code.append([2,15,12,0,-1,0,23])
        elif post and op=='BranchByCharacter':
            assert ints[:4]==[1,1,0,12]
            code.append([2,15,1,0,-1,0,12])
        elif post and op=='BranchByNumOfMember':
            assert ints[:3]==[0,1,15]and(v.get('sValues')or[''])[0]=='＝'
            code.append([2,15,0,0,-1,1,15])
        elif dream and op=='BranchByCharacter':
            assert ints[:4]==[3,1,0,8]
            code.append([2,15,3,0,-1,0,8])
        elif dream and op=='BranchByNumOfMember':
            assert ints[:3]==[0,4,8]and(v.get('sValues')or[''])[0]=='≠'
            code.append([2,15,0,1,-1,4,8])
        elif train and op=='BranchByCharacter':
            assert ints[:4]in([17,0,0,6],[18,0,0,6])
            code.append([2,-2,0,0,-1,0,6])
        elif tower and op=='BranchByCharacter':
            assert ints[:4]==[9,1,0,8]
            code.append([2,-2,0,0,-1,0,8])
        elif op=='SetPuppet':
            # Zero and negative IDs include resident/player aliases rather
            # than one unambiguous baked object. Keep them native-only.
            # Hide/Show and slot/actor/entry ownership are verified on all six
            # native interpreters. Absolute position units remain FFVI-only.
            effect=puppet_navigation_effect(script,ints[2],region,game=='ff6') if ints[1]>0 else None
            if owzer_timer and script['submap']=='Map_30631_4' and ints[1]==51:
                # These four exact bodies only LookAt0/1/2/3 after Ready.
                # Ready shows the decorative NPC; facing cannot hide or move it.
                assert ints[2] in (260,267,274,281)
                body=script['mnemonics'][ints[2]:ints[2]+3]
                assert [r['mnemonic']for r in body]==['Nop','LookAt','Exit']
                effect=[1,UNSET_POSITION,UNSET_POSITION]
            if statues and ints[1] in (202,203,204,205):
                assert ints[2]=={202:127,203:154,204:181,205:208}[ints[1]]
                effect=[1,145,59] # Ready shows it; the reviewed pose clears the old corridor
            if kolts and ints[1]==2:
                # Only the actual40-and-not41 initializer branch reaches this
                # exact SetPos23,32,2. Native2E8850 maps request2 to Unity10
                # (Ground),not the actor's off-map Upper home. No general
                # moving-puppet/layer inference is enabled for other actors.
                assert ints[:3]==[0,2,25]and script['mnemonics'][26]['mnemonic']=='SetPos'
                assert script['mnemonics'][26]['operands']['iValues'][:4]==[23,32,2,0]
                code.append([10,0,2,1,23-region[0],32-region[1],1]);continue
            # Always replace this puppet slot, even when its new body has no
            # navigation effect; ExecPuppet applies the ready final facts.
            # An opaque positive-actor body can hide again after Ready. Its
            # final visibility is unknown, rather than an old hidden receipt.
            fallback=-2 if ints[1]>0 else -1
            code.append([10,ints[0],ints[1],*(effect or [fallback,UNSET_POSITION,UNSET_POSITION])])
        elif op=='ExecPuppet':code.append([11,ints[0]])
        elif op=='AssignMoveRoute' and any(p['id']==ints[0] for p in patrols):
            # One opcode per native PC retains every existing branch/call target.
            # Only a reviewed exact route receives placement facts. A replacement
            # assignment to another route clears this fact rather than retaining it.
            match=next((p for p in patrols if p['id']==ints[0] and p['route']==ints[1]),None)
            if match:assert match['loop']==bool(ints[2])
            code.append([12,ints[0],ints[1] if match else -1])
        else:code.append(instruction(m,game))
    if zozo_magicite:
        # The complete native tail only conditionally hides six party NPCs.
        # Its seven owned/active-member tests cannot change the earlier four
        # pickup groups. Preserve those guaranteed receipts without inventing
        # membership or enumerating96 equivalent party branches. The existing
        # unknown-visibility opcode keeps only these six NPCs conservative.
        assert code[28]==[9]and len(code)==126
        code[28]=[3,len(code)]
        code.extend([[10,slot,actor,-2,UNSET_POSITION,UNSET_POSITION]for slot,actor in enumerate((44,30,29,28,27,26))])
        code.extend([[11,0],[6]])
    if game=='ff5':
        import pyramid_state
        code=pyramid_state.extend(script,game,region,code)
        import jachol_state
        code=jachol_state.extend(script,game,region,code)
        import gogo_state
        code=gogo_state.extend(script,game,region,code)
        import karnak_state
        code=karnak_state.extend(script,game,region,code)
        import walse_jail_state
        code=walse_jail_state.extend(script,game,region,code)
        import death_valley_journey
        code=death_valley_journey.extend(script,game,region,code)
        import fire_ship_state
        code=fire_ship_state.extend(script,game,region,code)
        import desert_state
        code=desert_state.extend(script,game,region,code)
        if script['submap']=='Map_40001' and script['name']=='sc_map_40001':
            import western_valley_state
            code=western_valley_state.extend(script,code)
    return code


def instruction(m, game):
    op = m['mnemonic']
    values = m.get('operands') or {}
    ints = list(values.get('iValues') or []) + [0] * 8
    strings = list(values.get('sValues') or []) + [''] * 8
    if op == 'SetEntities': return [1, strings[0]]
    if op == 'Branch':
        return [2, CATEGORIES.get(strings[0], -2), ints[0],
                OPERATORS.get(strings[1], -1), CATEGORIES.get(strings[2], -2), ints[1], ints[2]]
    if op == 'Jump': return [3, ints[0]]
    if op == 'Call': return [4, ints[0]]
    if op == 'Return': return [5]
    if op == 'Exit': return [6]
    if op in ('SetFlag', 'ResetFlag'):
        return [7, CATEGORIES.get(strings[0], -2), ints[0], int(op == 'SetFlag')]
    if op == 'Calc' and strings[1] == '←':
        return [8, CATEGORIES.get(strings[0], -2), ints[0], CATEGORIES.get(strings[2], -2), ints[1]]
    if op == 'SetRandomValue': return [8, CATEGORIES.get(strings[0], -2), ints[0], -2, 0]
    # FF1 Current.MapEntryRoofControl only calls IMapAccessor slots 10/20:
    # map-layer rendering and minimap-icon visibility. It never selects an entity
    # group or writes DataStorage. Matching native manifest: docs/re/ff1_roof*.
    if game == 'ff1' and op == 'SysCall' and strings[0] == 'MapEntryRoofControl': return [0]
    # Exact FFVI registrations resolve to IMapAccessor.ScalingAlphaMask and
    # ForegroundAlphaMask's material/scroll animation, without navigation-state
    # effects. Skipping only these visual commands lets later native Hide facts
    # be projected; opaque calls remain conservative. Native proof and actual
    # initializer regression: docs/re/NPC_MASK_PROJECTION_20261029.md.
    if game == 'ff6' and op == 'SysCall' and strings[0] in {
            '帝国軍陣地マスクあり', '帝国軍陣地マスクなし',
            '帝国軍陣地マスク拡大', '帝国軍陣地マスク縮小'}: return [0]
    # Exact registrations25E8E10/25E8DE0 dispatch to28A5E0/28A710:
    # IMapAccessor82/83 show/hide distant scenery1/2/3, through the two map
    # renderers and MapMultipleScroll. No flags, field actors or collision
    # state changes. Preserve unknown semantics for every other backdrop call.
    # Native provenance: docs/re/SHIFTG_PUZZLE_AUDIT_20261030.md, Opera section.
    if game == 'ff6' and op == 'SysCall' and strings[0] in {
            '背景変更：飛空艇（通常）','背景変更：飛空艇（特殊）'}: return [0]
    # FFIII Current.HideMist -> FieldController.HideScenery -> both map
    # renderers' MapMultipleScroll.Hide. No flags, actors or collision writes.
    # CheckWithCompanion conditionally writes map-local0; model that output as
    # unknown, retaining only facts guaranteed on both resulting branches.
    # Matching native provenance: docs/re/NPC_FLOORS_SHOP_LANDING_20261025.md.
    if game == 'ff3' and op == 'SysCall':
        if strings[0] == '霧を非表示に': return [0]
        if strings[0] == '同行者がいるか': return [8, 10, 0, -2, 0]
        # Current.CheckChild delegates to CheckConditionCommon(401), testing all
        # active corps members and writing only map-local0. Category14 is the
        # copied navigation result; never call that state-writing native SysCall.
        if strings[0] == '状態異常：小人かどうか': return [8, 10, 0, 14, 401]
    # These can change control flow/state beyond this read-only model.
    if op.startswith('Branch') or op in ('Calc', 'ChangeScript', 'SysCall', 'ChangeMap', 'Encount'):
        return [9]
    return [0]


def entry_guard(script, game, destination=None):
    """Only the native control-flow prefix before the first real action.

    Calls and every unmodelled command remain actionable. No dialogue, outcome,
    script writes or native execution enters this availability predicate.
    """
    ms=script.get('mnemonics') or []
    entry=next((p['EntryPoint'] for p in script['segments'] if p['Label']=='Main'),0)
    todo=[entry];nodes={};empty_exit=False
    while todo:
        pc=todo.pop()
        if pc in nodes:continue
        if pc<0 or pc>=len(ms) or len(nodes)>=128:return None
        m=ms[pc];op=m['mnemonic']
        if op in ('Nop','Wait'):
            nodes[pc]=[0];todo.append(pc+1)
        elif destination is not None and (op=='RemoveTelepoCache' or
                op=='SysCall'and (m.get('operands')or{}).get('sValues',[''])[0]=='ジャコールのギミック解除'):
            # Exact reviewed Jacohl departure cleanup changes the local gimmick/
            # teleport cache, not the ScenarioFlag1/125 tested immediately next.
            nodes[pc]=[0];todo.append(pc+1)
        elif destination is not None and op=='ChangeMap':
            target=(m.get('operands')or{}).get('iValues',[])[:2]
            nodes[pc]=[1,'@action']if target==list(destination)else[6]
            empty_exit|=nodes[pc]==[6]
        elif op in ('Exit','Return'):
            nodes[pc]=[6];empty_exit=True
        elif game=='ff3' and op=='SysCall' and (m.get('operands') or {}).get('sValues',[''])[0]=='状態異常：小人かどうか':
            nodes[pc]=instruction(m,game);todo.append(pc+1)
        elif op=='Jump':
            nodes[pc]=instruction(m,game);todo.append(nodes[pc][1])
        elif op=='Branch':
            nodes[pc]=instruction(m,game);todo.extend([pc+1,nodes[pc][6]])
        else:nodes[pc]=[1,'@action']
    if not empty_exit or not any(row[0]==1 for row in nodes.values()) or not any(
            row[0]==2 and (0<=row[1]<=8 or row[1]==10 and any(r==[8,10,0,14,401] for r in nodes.values())) for row in nodes.values()):return None
    offsets={};code=[]
    for pc,row in sorted(nodes.items()):
        offsets[pc]=len(code);code.append(list(row))
        if row[0]==1:code.append([6])
    for pc,row in nodes.items():
        target=code[offsets[pc]]
        if row[0]==3:target[1]=offsets[row[1]]
        elif row[0]==2:target[6]=offsets[row[6]]
    return {'entry':offsets[entry],'code':code}


def dwarven_guards(objects, bounds):
    # All three guards occupy a one-tile passage in the pre-battle group.
    # The native scenario-46 initializers remove that group together.
    contacts = {
        'Map_20151_1': {18: (24, 10, 'N112_C01_121_04_01')},
        'Map_20151_3': {18: (17, 6, 'N112_C01_121_06_01'),
                        19: (8, 14, 'N112_C01_121_04_01')},
    }
    rows = {}
    for submap, guards in contacts.items():
        found = []
        for entity, (x, y, message) in guards.items():
            guard, = [o for o in objects[submap] if o['type'] == 6
                      and o['group'] == 'ev_e_0047' and o['props']['entity_id'] == entity]
            assert (guard['tx'], guard['ty']) == (x, y)
            assert guard['props']['move_type'] == 1 and guard['props']['message_key'] == message
            assert (guard['w'], guard['h']) == (16, 16)
            left, top = bounds[submap][:2]
            found.append(dict(id=entity, type=6, group=guard['group'], raw=guard['name'],
                              cell=[bake_maps.layer_index(guard['layer']), x-left,
                                    bake_maps.tile_row(guard)-top], size=[1, 1]))
        rows[submap] = found
    return rows


def build(game, initializers_only=False):
    scripts = {}
    source={}
    for line in (DUMP / f'{game}_scripts.jsonl').read_text(encoding='utf-8').splitlines():
        s = json.loads(line)
        source[s['submap'],s['name']]=s
        if s['name'].lower() == 'sc_' + s['submap'].lower():
            main = next((p['EntryPoint'] for p in s['segments'] if p['Label'] == 'Main'), 0)
            scripts[s['submap']] = {'entry': main, 'code': [instruction(m, game) for m in s.get('mnemonics') or []]}
    objects=list(map(json.loads,(DUMP / f'{game}_objects.jsonl').read_text(encoding='utf-8').splitlines()))
    by_sub=collections.defaultdict(list)
    for o in objects:by_sub[o['submap']].append(o)
    submaps=json.loads((DUMP/f'{game}_submaps.json').read_text(encoding='utf-8'))
    bounds={sm:bake_maps.map_bounds(info,by_sub[sm]) for sm,info in submaps.items()}
    for name,record in scripts.items():
        initializer=source[name,'sc_'+name.lower()]
        record['code']=initializer_code(initializer,game,bounds[name])
        if game == 'ff4' and name == 'Map_20010_1':
            # ClearMapLocal runs before loading this floor. The persistent
            # gate receipts are still evaluated. Only the already loaded
            # courtyard can have the two transient trigger-deletion locals.
            code = record['code']
            assert code[14] == [2, 10, 1, 0, -1, 0, 16]
            assert code[17] == [2, 10, 2, 0, -1, 0, 19]
            record['current_code'] = code
            record['code'] = code.copy()
            record['code'][14] = [3, 16]
            record['code'][17] = [3, 19]
        if game=='ff5' and name=='Map_30611_12':
            import trench_state
            record['current_code']=trench_state.live_code(initializer,record['code'])
        patrols=[p for p in PATROLS if p['game']==game and p['submap']==name]
        if patrols:
            # Bind the native assignment to its original actor, never a coincident
            # ID in a different group. Facts only; no original route/script body.
            record['patrols']=[{k:p[k] for k in ('id','group','raw','home','route','cells')} for p in patrols]
    with (DUMP/f'{game}_master/script.txt').open(encoding='utf-8-sig') as f:
        masters={int(r['id']):r['script_name'] for r in csv.DictReader(line for line in f if line.strip()) if r['id']}
    guards=0
    with (DUMP/f'{game}_master/map.txt').open(encoding='utf-8-sig') as f:
        maps=list(csv.DictReader(line for line in f if line.strip()))
    metadata={}
    for row in maps:
        if not row['id']: continue
        key=row['map_name']
        info=metadata.setdefault(key,dict(map_ids=[],area_id=int(row['area_id'])))
        # A shared asset can have aliases in different native areas (FF3).
        # Its area locals are ambiguous remotely, rather than interchangeable.
        if info['area_id']!=int(row['area_id']): info['area_id']=0
        info['map_ids'].append(int(row['id']))
    transport_selectors=collections.defaultdict(dict)
    for o in objects:
        p=o.get('props') or {}
        selectors=p.get('target_transportation_ids')
        if selectors:
            # Preserve exact native selectors independently from presentation
            # names. Warp aliases match the original native actor fingerprint.
            ids=[int(v.strip()) for v in str(selectors).split(',') if v.strip()]
            assert ids and len(ids)<=64 and all(0<value<64 for value in ids),(game,o['submap'],o['id'])
            region=bounds[o['submap']]
            row=transport_selectors[o['submap']]
            layer=bake_maps.layer_index(o['layer']);layer=1 if layer<0 else layer
            key=f"{o['type']}:{p.get('entity_id',0)}:{layer}:{o['tx']-region[0]}:{bake_maps.tile_row(o)-region[1]}"
            bucket=row.setdefault('transport',{}).setdefault(key,{}).setdefault(o['group'],{})
            raw=str(selectors) if o['type']==19 else o['name']
            assert raw not in bucket or bucket[raw]==ids,(game,o['submap'],key,raw)
            bucket[raw]=ids
        # Flag zero is the first real saved treasure bit, not a missing-ID
        # sentinel. Require an explicit bounded native property so a missing
        # extraction cannot silently become that first chest's state.
        flag=p.get('flag_id')
        if o['type']==7 and type(flag) is int and 0<=flag<1024:
            row=scripts.setdefault(o['submap'],{'entry':0,'code':[]})
            key=[p.get('entity_id',0),o['group'],flag]
            # The two money chests on Karnak 2F share ID zero and the same
            # group. The same baked raw name identifies each native chest.
            if game=='ff5' and o['submap']=='Map_20071_10' and key[0]==0:
                key.append(o['name'])
            row.setdefault('treasures',[]).append(key)
        if o['type'] not in (2,5,6,17,21,28,30) or not p.get('script_id') or p.get('action_id')!=2 or p.get('message_key'):continue
        name=masters.get(p['script_id'],'');s=source.get((o['submap'],name))
        pending=[]
        train_guard=phantom_train_exits.guard(o,s)if game=='ff6'and s else None
        if train_guard is not None:pending.append(train_guard)
        if game=='ff6':
            import cyan_passage
            if cyan_guard:=cyan_passage.guard(o,s):pending.append(cyan_guard)
            import veldt_cave_passage
            if veldt_guard:=veldt_cave_passage.guard(o,s):pending.append(veldt_guard)
            import late_opera_passage
            if opera_guard:=late_opera_passage.guard(o,s):pending.append(opera_guard)
        guard=entry_guard(s,game) if s else None
        if guard is not None:pending.append(guard)
        for destination in bake_maps.reviewed_conditional_destinations(game,o,s):
            branch=entry_guard(s,game,destination[:2]);assert branch,'Reviewed conditional prefix is not supported'
            branch['destination']=list(destination[:2]);pending.append(branch)
        if not pending:continue
        region=bounds[o['submap']]
        for guard in pending:
            guard.update(id=p.get('entity_id',0),group=o['group'],type=o['type'],raw=o['name'],
                         cell=[bake_maps.layer_index(o['layer']),o['tx']-region[0],bake_maps.tile_row(o)-region[1]],source=name)
            guard.update(metadata.get(o['submap'],{}))
            row=scripts.setdefault(o['submap'],{'entry':0,'code':[]})
            row.setdefault('events',[]).append(guard);guards+=1
    for key,selectors in transport_selectors.items():
        if key not in scripts:scripts[key]={'entry':0,'code':[],'initializer_unknown':True}
        scripts[key]['transport']=selectors['transport']
    for key,record in scripts.items(): record.update(metadata.get(key,{}))
    if game == 'ff4':
        for key, guards in dwarven_guards(by_sub, bounds).items():
            scripts[key]['story_actor_gates'] = guards
    if not initializers_only:
        for key, contacts in bake_walk_open_doors.generate(game, DUMP):
            scripts.setdefault(key, {'entry': 0, 'code': [], 'initializer_unknown': True})['walk_open'] = contacts
    output = ROOT / 'data' / game / 'mapstate.json'
    if initializers_only:
        # A focused compiler correction must preserve authored locks, actor
        # gates, event predicates and every unrelated source-qualified field.
        prior=json.loads(output.read_text(encoding='utf-8'));changed=0
        for key,row in prior.items():
            generated=scripts.get(key)
            if not generated or 'code'not in row:continue
            old=row['code'];new=generated['code']
            assert row.get('entry',0)==generated.get('entry',0)and len(old)==len(new),(game,key,'initializer layout changed')
            for a,b in zip(old,new):
                assert a==b or a[0]==b[0]==10 and a[:3]==b[:3],(game,key,'unrelated initializer change')
            changed+=old!=new;row['code']=new
        scripts=prior
        print(f'{game}: focused puppet correction in {changed} initializer records; all authored metadata preserved')
    output.write_text(json.dumps(scripts, ensure_ascii=False, separators=(',', ':')) + '\n', encoding='utf-8')
    print(f'{game}: {len(scripts)} map-state records; {guards} bounded native entry guards')


if __name__ == '__main__':
    args=[s for s in sys.argv[1:]if s!='--initializers-only']
    for game in ([f'ff{i}' for i in range(1, 7)] if args == ['all'] else args):
        build(game,'--initializers-only'in sys.argv)
