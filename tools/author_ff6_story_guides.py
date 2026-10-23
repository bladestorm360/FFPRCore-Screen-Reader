"""Rebuild original FF6 guide recipes from reviewed identities, never game code.

Inputs are local, read-only extracted map/script records outside Git. Recipes
contain factual target identities/flags and original instructions. Landing
records pin the reviewed source hash so an updated game cannot silently reuse
an obsolete scripted crossing. See docs/re/FF6_STORY_NAVIGATION.md.
"""
import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
OBJECTS = [json.loads(s) for s in (DUMP/'ff6_objects.jsonl').read_text(encoding='utf-8').splitlines()]
SCRIPTS = {(s['submap'], s['name']): s for s in
           map(json.loads, (DUMP/'ff6_scripts.jsonl').read_text(encoding='utf-8').splitlines())}
MAPS = {int(r['id']): r for r in csv.DictReader((DUMP/'ff6_master/map.txt').open(encoding='utf-8-sig'))}
SCRIPT_NAMES = {int(r['id']): r['script_name'] for r in
                csv.DictReader((DUMP/'ff6_master/script.txt').open(encoding='utf-8-sig'))}


def physical(map_id, entity, group=None, script=None):
    found = [o for o in OBJECTS if o['submap'] == MAPS[map_id]['map_name']
             and o['props'].get('entity_id') == entity
             and o['type_name'] in ('Entity', 'AnimEntity', 'NPC', 'ShopNPC', 'Event', 'RandomEvent', 'SwitchEvent')
             and (group is None or o['group'] == group)
             and (script is None or o['props'].get('script_id') == script)]
    assert len(found) == 1, (map_id, entity, group, script, len(found))
    return found[0]


def target(map_id, entity, text, group=None, script=None, spot=False):
    o = physical(map_id, entity, group, script)
    return dict(map=map_id, entity=entity, raw=o['name'], group=o['group'],
                kind='spot' if spot or o['type_name'] in ('Event', 'RandomEvent', 'SwitchEvent') else 'interact', text=text)


def condition(bank, index, value=0, op='eq'):
    return [bank, index, op, value]


def flag(index, value=0):
    return condition('scenario', index, value)


def rule(conditions, *targets):
    return dict(when=conditions, targets=list(targets))


def hold(map_id, entity, latch, conditions=(), group=None):
    return dict(when=[flag(latch), *conditions], at=target(map_id, entity, '', group),
                note='Stay on this switch. Change parties and guide another party to the remaining switch. Leave only after the shared passage opens.')


def guide(name, maps, introduction, rules, when=()):
    return dict(id=name, maps=maps, introduction=introduction, when=list(when), rules=rules,
                complete='No pending objective is available here. Continue through the opened passage and request guidance again.')


def author_guides():
    encounter = 'Defeat {0}. Contact starts this encounter; request guidance again after battle.'
    switch = 'Operate {0}, then request guidance again.'
    plate = 'Stand on {0}. Keep this party here and change parties.'
    crossing = 'Use {0} to cross, then request guidance again.'
    guides = [guide('terra_defense', [318],
        'Protect Terra. Reach an approaching enemy or the commander. You can change between three parties.',
        [rule([], *[target(318, i, encounter, 'ev_e_0018') for i in range(18, 24)],
              target(318, 9, 'Defeat {0} to finish the defense.', 'ev_e_0018'))],
        [flag(13, 1), flag(14)])]

    # Area switches have explicit leave scripts that reset their locals. Route
    # a departing party against that release, retaining the other party's state.
    # The later two rock controls are persistent toggles, not held plates.
    phoenix = [hold(498, i, 402) for i in (40, 41)]
    for local,entity in ((1,108),(2,94),(3,86),(4,77),(5,61),(6,49),(7,44)):
        actor=physical(498,entity)
        leave=SCRIPT_NAMES[actor['props']['leave_script_id']]
        body=SCRIPTS[actor['submap'],leave]['mnemonics']
        assert body[1]['mnemonic']=='Calc' and body[1]['operands']['iValues'][:2]==[local,0] and \
            body[1]['operands']['sValues'][:3]==['AreaLocalVariable','←','imm'], (entity,leave)
        phoenix.append(dict(when=[condition('area',local,1)],at=target(498,entity,''),
                            release=['area',local,0],leave_script=leave,note='Stay on this switch. Change parties and guide the other party across the rocks.' if local>=6 else
                            'Stay on this switch. Change parties and continue through the opened passage.'))
    # Prefer the actual story exit when reachable, instead of cycling temporary
    # switches which reset after a party has already advanced past them.
    phoenix.append(rule([flag(408)],target(499,50,'Reach {0} to reunite the parties and advance the Phoenix Cave story.',spot=True)))
    for local, entity in ((1, 108), (2, 94), (3, 86), (4, 77), (5, 61), (6, 49), (7, 44)):
        phoenix.append(rule([condition('area', local)], target(498, entity, plate, spot=True)))
    phoenix += [rule([flag(399)], target(498, 56, 'Operate {0} to drain the water and cool the lava.')),
                rule([flag(402), flag(400)], target(498, 41, plate)),
                rule([flag(402), flag(401)], target(498, 40, plate)),
                rule([flag(403)], target(498, 98, 'Step on {0} to extend the bridge.', spot=True)),
                rule([flag(405)], target(499, 66, 'Step on {0} to raise the stepping stones.', spot=True)),
                rule([flag(406), flag(407)], target(499, 58, 'Stand on {0} to move the rocks. Keep one party on the switch while the other crosses.', spot=True)),
                rule([flag(406,1)],target(499,55,'Operate {0}, then request guidance again.',spot=True)),
                rule([flag(407,1)],target(499,58,'Operate {0}, then request guidance again.',spot=True))]
    guides.append(guide('phoenix_cave', [497, 498, 499],
        'Phoenix Cave uses two parties. Keep a party on pressure switches while the other advances. The guide follows current switch and rock state.',
        phoenix, [flag(408)]))

    tower_maps = list(range(542, 572)) + [590, 595, 596]
    tower = [hold(549, i, 483) for i in (37, 38)]
    for entity, completed in ((37,486),(38,487)):
        tower.append(dict(when=[flag(483,1),flag(completed)],at=target(549,entity,''),
                          note='The door is open. Move off this switch so another party can push the weight.'))
    tower += [hold(565, i, 491) for i in (40, 41, 44)]
    tower += [hold(570, i, 501) for i in (14, 15, 16)]
    # Moving onto a plate is an objective only until the corresponding latch.
    tower += [rule([flag(483), flag(f)], target(549, i, plate)) for i, f in ((37, 478), (38, 482))]
    # A weight cannot be dropped onto a party occupying the plate below it.
    for weight, completed, occupied in ((39, 486, 478), (45, 487, 482)):
        tower.append(rule([flag(483,1),flag(completed)], target(549, weight,
            'Push {0}. First move the party below off its switch; the weight cannot fall onto that party.')))
    for map_id, entity, completed in ((565, 25, 484), (565, 26, 485), (549, 61, 493)):
        tower.append(rule([flag(completed)], target(map_id, entity, switch, spot=True)))
    tower += [rule([flag(491), flag(f)], target(565, i, plate)) for i, f in ((40, 488), (41, 489), (44, 490))]
    # Required bosses and actual event cells stay available to any reachable
    # party. Optional dragons remain in ordinary navigation, not this checklist.
    for map_id, entity, completed, group in ((544, 14, 474, None), (566, 47, 492, None),
                                           (542, 93, 494, None), (567, 2, 495, None), (568, 2, 496, 'ev_e_0997')):
        tower.append(rule([flag(completed)], target(map_id, entity, 'Reach {0} and defeat the boss to open this route.', group)))
    tower += [rule([flag(501), flag(f)], target(570, i, plate)) for i, f in ((15, 498), (16, 499), (14, 500))]
    guides.append(guide('kefkas_tower', tower_maps,
        'Kefka’s Tower uses three parties. Hold the outer switches to open the central route, move clear before dropping weights, then place all three parties on the final switches.',
        tower, [flag(501)]))

    opera = []
    # Native retry phases have distinct actors; identity includes the group.
    for entity, before, after, group in ((21, 158, 159, 'ev_e_0315'), (20, 160, 161, 'ev_e_0322'),
                                        (19, 162, 164, 'ev_e_0325'), (18, 163, 164, 'ev_e_0327')):
        conditions = [flag(after)] + ([] if before == 158 else [flag(before, 1)])
        opera.append(rule(conditions, target(112, entity, 'Speak to {0} to prepare or retry the performance.', group)))
    opera += [rule([flag(159)], target(113, 6, 'Speak to {0} in the dressing room.', 'ev_e_0316')),
              rule([flag(159, 1), flag(160)], target(113, 7, 'Enter the stage through {0}. Follow the spoken lyric choices and native performance prompts.', 'ev_e_0318', spot=True)),
              rule([flag(164), flag(162, 1)], target(113, 12, 'Speak to {0} to prepare the retry.', 'ev_e_0323')),
              rule([flag(160, 1), flag(166)], target(113, 13, 'Read {0} to discover the sabotage.', 'ev_e_0329')),
              rule([flag(167, 1), flag(168)], target(115, 6, 'Pull {0}, the far-right lever. The other three levers are traps.')),
              rule([flag(167, 1)], target(117, 5, 'Reach {0} across the rafters before the timer expires.', 'ev_e_0344'))]
    for entity, done in zip((13, 12, 11, 10, 9), range(169, 174)):
        opera.append(rule([flag(167, 1), flag(done)], target(117, entity, encounter)))
    opera.append(dict(when=[condition('map', 0, 576)],
                      note='Follow the spoken lyric choices and native performance prompts. Use the movement prompts on stage; the game controls the performance timer.'))
    guides.append(guide('opera', list(range(112, 119)) + [576],
        'Opera guidance follows the current performance and sabotage phase. The timer continues normally.', opera, [flag(177)]))

    # The later defense has twelve contact encounters and Kefka himself.
    # Native actor presence, not the baked starting locations, owns each fight.
    guides.append(guide('narshe_battle',[19],'',[
        rule([], *[target(19,i,encounter,'ev_e_0257') for i in (24,*range(38,49))],
             target(19,25,'Defeat {0} to finish the defense.','ev_e_0257'))],
        [flag(137,1),flag(138)]))

    # Prefer the rescue objective when reachable. If live flames obstruct it,
    # offer the next reachable contact encounter without inventing opened doors.
    fire=[target(420,1,'Reach {0} and defeat the boss to open this route.','ev_e_0539',spot=True)]
    for map_id,entities in ((413,range(1,4)),(414,range(1,5)),(415,range(1,4)),(417,range(1,3))):
        fire += [target(map_id,i,encounter) for i in entities]
    fire += [target(414,5,switch,spot=True)]
    guides.append(guide('burning_house',list(range(412,421)),'',[rule([], *fire)]))

    # Engine levers are toggles. Correct native state is left/right enabled,
    # middle disabled; operating each lever once would fail on a retry.
    train=[rule([flag(120)],target(371,14,switch)),
           rule([flag(121,1)],target(371,10,switch)),
           rule([flag(122)],target(371,6,switch)),
           rule([flag(120,1),flag(121),flag(122,1)],
                target(356,5,'Reach {0} and defeat the boss to open this route.','ev_e_0214')),
           rule([condition('map',0,372)],target(372,1,'Speak to {0} to continue.','ev_e_0215'))]
    guides.append(guide('phantom_train',list(range(355,373))+[624],'',train))

    # The requested guide is brief: route first, then the action/required switch.
    # Threat interception is ranked from captured moving actors by the worker;
    # otherwise the actual event-completion actor is the first objective.
    for identity,map_id,actor,group,warning in (
            ('terra_defense',318,32,'ev_e_0018','Terra in danger.'),
            ('narshe_battle',19,64,'ev_e_0257','Banon in danger.')):
        entry=next(g for g in guides if g['id']==identity)
        entry['defend']=target(map_id,actor,'',group)
        entry['danger']=warning
        targets=entry['rules'][0]['targets']
        targets.insert(0,targets.pop())
    short={
        encounter:'Fight {0}.',switch:'Operate {0}.',plate:'Hold switch. Switch parties.',
        'Defeat {0} to finish the defense.':'Fight {0}.',
        'Stay on this switch. Change parties and guide another party to the remaining switch. Leave only after the shared passage opens.':'Hold switch. Switch parties.',
        'Stay on this switch. Change parties and continue through the opened passage.':'Hold switch. Switch parties.',
        'Stay on this switch. Change parties and guide the other party across the rocks.':'Hold switch. Switch parties.',
        'The door is open. Move off this switch so another party can push the weight.':'Leave the switch.',
        'Operate {0} to drain the water and cool the lava.':'Operate {0}.',
        'Reach {0} to reunite the parties and advance the Phoenix Cave story.':'Join {0}.',
        'Stand on {0} to move the rocks. Keep one party on the switch while the other crosses.':'Hold switch. Switch parties.',
        'Step on {0} to extend the bridge.':'Step on {0}.',
        'Step on {0} to raise the stepping stones.':'Step on {0}.',
        'Push {0}. First move the party below off its switch; the weight cannot fall onto that party.':'Push {0}. Clear the party below first.',
        'Reach {0} and defeat the boss to open this route.':'Fight {0}.',
        'Follow the spoken lyric choices and native performance prompts. Use the movement prompts on stage; the game controls the performance timer.':'Follow performance cues.',
        'Enter the stage through {0}. Follow the spoken lyric choices and native performance prompts.':'Enter the stage.',
        'Pull {0}, the far-right lever. The other three levers are traps.':'Pull the far-right lever.',
        'Reach {0} across the rafters before the timer expires.':'Reach {0}.',
        'Read {0} to discover the sabotage.':'Read {0}.',
        'Speak to {0} in the dressing room.':'Talk to {0}.',
        'Speak to {0} to prepare or retry the performance.':'Talk to {0}.',
        'Speak to {0} to prepare the retry.':'Talk to {0}.',
        'Speak to {0} to continue.':'Talk to {0}.',
    }
    for entry in guides:
        entry['introduction']=''
        entry['complete']='Continue through the passage.'
        for row in entry['rules']:
            if 'note' in row:row['note']=short.get(row['note'],row['note'])
            for candidate in row.get('targets',[]):candidate['text']=short.get(candidate['text'],candidate['text'])
    path = ROOT/'data/ff6/puzzles.json'
    data = json.loads(path.read_text(encoding='utf-8'))
    data['_comment'] = ('Original factual navigation recipes. Static puzzles retain their original authorship. '
                        'Story guides use native completion flags and reachable live targets, not proximity completion. '
                        'See docs/re/FF6_STORY_NAVIGATION.md for source locators, controls, limits and pending gameplay confirmation.')
    data['story_guides'] = guides
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Authored', len(guides), 'story guides;', sum(len(g['rules']) for g in guides), 'state rules')


def author_crossings():
    path = ROOT/'data/ff6/warp_arrival_overrides.json'
    rows = json.loads(path.read_text(encoding='utf-8'))
    # Each reviewed crossing selects the player puppet and performs an
    # unconditional linear sequence of Lerp operations before Exit. Values are
    # map asset cells; native Lerp converts through the same MapRange accessor.
    for map_id, script_ids in ((498, (830, 831)), (499, (*range(840, 844), *range(846,856), 857, 858))):
        for script_id in script_ids:
            objects = [o for o in OBJECTS if o['submap']==MAPS[map_id]['map_name']
                       and o['props'].get('script_id')==script_id]
            assert len(objects)==1, (map_id, script_id)
            o=objects[0]
            source=SCRIPTS[o['submap'], SCRIPT_NAMES[script_id]]
            puppet=next(m['operands']['iValues'][2] for m in source['mnemonics']
                        if m['mnemonic']=='SetPuppet' and m['operands']['iValues'][1]==-1001)
            moves=[]
            for m in source['mnemonics'][puppet:]:
                if m['mnemonic']=='Exit':break
                assert m['mnemonic'] in ('Nop','PlaySE','Lerp','Wait','CueAnim'), (source['name'],m['mnemonic'])
                if m['mnemonic']=='Lerp':moves.append(m['operands']['iValues'][:2])
            assert moves
            x,y=moves[-1]
            bounds=next(b for b in OBJECTS if b['submap']==o['submap'] and b['type_name']=='MapRange')
            ox,oy=int(bounds['x']/16),int(bounds['y']/16)
            row=dict(submap=o['submap'],entity_id=o['props']['entity_id'],object_id=o['id'],same_map=True,
                     interaction_travel=True,destination_map=map_id,original_point=0,continuation=source['name'],
                     continuation_sha256=hashlib.sha256(json.dumps(source,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest(),
                     arrival_cell=[x,y,1],arrival_baked_cell=[x-ox,y-oy,1],
                     evidence='Reviewed player-puppet stepping-stone crossing; final Lerp, native MapRange conversion. docs/re/FF6_STORY_NAVIGATION.md')
            old=next((r for r in rows if r['submap']==o['submap'] and r.get('object_id')==o['id']),None)
            if old:old.update(row)
            else:rows.append(row)
    path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Reviewed',sum(bool(r.get('interaction_travel')) for r in rows),'manual crossing arcs')


def author_directional_crossings():
    """Known bridge/ladder/belt families, with exact source-side eligibility.

    Native movement operand semantics are already verified in TRAPS.md. This
    bounded reviewer follows only direction dispatch and linear puppet calls;
    story flags, encounters, choices or unsupported movement reject the edge.
    The original scripts remain external. Only hashed factual endpoints ship.
    """
    import sys
    sys.path.insert(0,str(ROOT/'tools'))
    import bake_maps
    path=ROOT/'data/ff6/warp_arrival_overrides.json'
    rows=json.loads(path.read_text(encoding='utf-8'))
    rows=[r for r in rows if not r.get('evidence','').startswith('Reviewed direction-dispatched player movement;')]
    count=0
    for map_id in (383,501,506,556,542,559,564):
        for o in OBJECTS:
            if o['submap']!=MAPS[map_id]['map_name']:continue
            sid=o['props'].get('script_id')
            # Only the reviewed native travel families, not repulsion events,
            # battle/cutscene relocations, exits, traps or arbitrary scripts.
            if not sid or not any(t in o['name'] for t in ('飛び移る','梯子昇降','エスカレーターで進む','ベルトコンベア')):continue
            source=SCRIPTS.get((o['submap'],SCRIPT_NAMES[sid]))
            assert source,(map_id,sid)
            ms=source['mnemonics'];variants={}
            for facing in range(4):
                # These vertical ladders have no walkable east/west approach.
                # Their native default-direction branch is unreachable there;
                # do not manufacture lateral travel from the wall cells.
                if '梯子昇降' in o['name'] and facing not in (0,2):continue
                side=(facing+2)%4
                accept=o['props'].get('accept_action_direction',15) or 15
                if o['type_name']=='Entity' and not (accept&(1<<side)):continue
                pc=0;stack=[];puppets={};executed=[];valid=True
                for _ in range(128):
                    m=ms[pc];pc+=1;op=m['mnemonic'];iv=m['operands'].get('iValues',[])
                    if op=='BranchByPlayerDirection':pc=iv[facing]
                    elif op=='Call':stack.append(pc);pc=iv[0]
                    elif op=='Return':
                        if not stack:break
                        pc=stack.pop()
                    elif op=='Exit':break
                    elif op=='SetPuppet':puppets[iv[0]]=(iv[1],iv[2])
                    elif op=='ExecPuppet':
                        if iv[0]!=0:valid=False;break
                        executed += [entry for actor,entry in puppets.values() if actor==-1001]
                        puppets.clear()
                    elif op not in ('Nop','PlaySE','Wait'):valid=False;break
                else:valid=False
                if not valid or len(executed)!=1:continue
                moves=[]
                for m in ms[executed[0]:executed[0]+128]:
                    if m['mnemonic']=='Exit':break
                    if m['mnemonic'] not in ('Nop','PlaySE','Wait','CueAnim','Lerp','SetPos','Move','MoveTo'):valid=False;break
                    if m['mnemonic'] in ('Lerp','SetPos','Move','MoveTo'):moves.append(m)
                if not valid or not moves:continue
                dx,dy=((0,-1),(1,0),(0,1),(-1,0))[facing]
                # Tile objects store their bottom edge. A player starts beside
                # an examined object, or on the cell of a step-on event.
                sx,sy=o['tx'],bake_maps.tile_row(o)
                if o['type_name']=='Entity':sx-=dx;sy-=dy
                landing=[sx,sy,bake_maps.layer_index(o['layer'])]
                for move in moves:
                    iv=move['operands']['iValues'];op=move['mnemonic']
                    if op in ('SetPos','Lerp') and iv[3]==0:
                        if iv[0]!=-1:landing[0]=iv[0]
                        if iv[1]!=-1:landing[1]=iv[1]
                        if iv[2]>0:landing[2]=iv[2]-1
                    elif op=='Move' and 0<=iv[0]<4:
                        x,y=((0,-1),(1,0),(0,1),(-1,0))[iv[0]]
                        landing[0]+=x*iv[1];landing[1]+=y*iv[1]
                    elif op=='MoveTo' and iv[1]==0 and iv[0] in (0,1):
                        landing[iv[0]]=iv[2]
                        if iv[3]>0:landing[2]=iv[3]-1
                    else:valid=False;break
                if not valid:continue
                # Current reviewed families are single-cell interactions. Do
                # not assume a single arrival for a wide trigger's relative move.
                if o['w']!=16 or o['h']!=16:continue
                variants.setdefault(tuple(landing),0)
                variants[tuple(landing)]|=1<<side
            assert variants,(map_id,sid,o['id'],'unsupported reviewed travel')
            info=json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[o['submap']]
            ox,oy,_,_=bake_maps.map_bounds(info,[b for b in OBJECTS if b['submap']==o['submap']])
            for landing,mask in variants.items():
                manual=o['type_name'] in ('Entity','AnimEntity')
                row=dict(submap=o['submap'],entity_id=o['props'].get('entity_id',0),object_id=o['id'],
                         same_map=True,interaction_travel=manual,approach_mask=mask if manual else 0,
                         destination_map=map_id,original_point=0,continuation=source['name'],
                         continuation_sha256=hashlib.sha256(json.dumps(source,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest(),
                         arrival_cell=list(landing),arrival_baked_cell=[landing[0]-ox,landing[1]-oy,landing[2]],
                         evidence='Reviewed direction-dispatched player movement; source-side mask and native movement operands. docs/re/FF6_STORY_NAVIGATION.md')
                old=next((r for r in rows if r['submap']==o['submap'] and r.get('object_id')==o['id'] and
                          r.get('approach_mask',0)==row['approach_mask']),None)
                if old:old.update(row)
                else:rows.append(row)
                count+=1
    path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Reviewed',count,'direction-aware bridge, ladder and belt arcs')


if __name__=='__main__':
    author_guides()
    author_crossings()
    author_directional_crossings()
