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
    visible=-1; x=y=UNSET_POSITION; moving=False
    for pc in range(entry,min(len(rows),entry+128)):
        m=rows[pc];op=m['mnemonic'];v=m.get('operands')or{}
        ints=list(v.get('iValues')or[])+[0]*8
        if op in ('Exit','Return'):
            if moving and visible!=0:return None
            return [visible,x,y] if visible!=-1 or x!=UNSET_POSITION or y!=UNSET_POSITION else None
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
    for m in script.get('mnemonics')or[]:
        op=m['mnemonic'];v=m.get('operands')or{};ints=list(v.get('iValues')or[])+[0]*8
        if op=='SetPuppet':
            # Zero and negative IDs include resident/player aliases rather
            # than one unambiguous baked object. Keep them native-only.
            # Hide/Show and slot/actor/entry ownership are verified on all six
            # native interpreters. Absolute position units remain FFVI-only.
            effect=puppet_navigation_effect(script,ints[2],region,game=='ff6') if ints[1]>0 else None
            if statues and ints[1] in (202,203,204,205):
                assert ints[2]=={202:127,203:154,204:181,205:208}[ints[1]]
                effect=[-1,145,59] # absolute XY; the world router normalizes its layer
            # Always replace this puppet slot, even when its new body has no
            # navigation effect; ExecPuppet applies the ready final facts.
            code.append([10,ints[0],ints[1],*(effect or [-1,UNSET_POSITION,UNSET_POSITION])])
        elif op=='ExecPuppet':code.append([11,ints[0]])
        elif op=='AssignMoveRoute' and any(p['id']==ints[0] for p in patrols):
            # One opcode per native PC retains every existing branch/call target.
            # Only a reviewed exact route receives placement facts. A replacement
            # assignment to another route clears this fact rather than retaining it.
            match=next((p for p in patrols if p['id']==ints[0] and p['route']==ints[1]),None)
            if match:assert match['loop']==bool(ints[2])
            code.append([12,ints[0],ints[1] if match else -1])
        else:code.append(instruction(m,game))
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


def build(game):
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
        record['code']=initializer_code(source[name,'sc_'+name.lower()],game,bounds[name])
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
            row.setdefault('treasures',[]).append([p.get('entity_id',0),o['group'],flag])
        if o['type'] not in (2,5,6,17,21,28,30) or not p.get('script_id') or p.get('action_id')!=2 or p.get('message_key'):continue
        name=masters.get(p['script_id'],'');s=source.get((o['submap'],name))
        pending=[]
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
    output = ROOT / 'data' / game / 'mapstate.json'
    output.write_text(json.dumps(scripts, ensure_ascii=False, separators=(',', ':')) + '\n', encoding='utf-8')
    print(f'{game}: {len(scripts)} map-state records; {guards} bounded native entry guards')


if __name__ == '__main__':
    for game in ([f'ff{i}' for i in range(1, 7)] if sys.argv[1:] == ['all'] else sys.argv[1:]):
        build(game)
