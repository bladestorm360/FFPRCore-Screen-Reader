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
             and o['type_name'] in ('Entity', 'AnimEntity', 'NPC', 'ShopNPC', 'Event', 'RandomEvent', 'SwitchEvent', 'GotoMap', 'TreasureBox')
             and (group is None or o['group'] == group)
             and (script is None or o['props'].get('script_id') == script)]
    assert len(found) == 1, (map_id, entity, group, script, len(found))
    return found[0]


def target(map_id, entity, text, group=None, script=None, spot=False):
    o = physical(map_id, entity, group, script)
    return dict(map=map_id, entity=entity, raw=o['name'], group=o['group'],
                kind='spot' if spot or o['type_name'] in ('Event', 'RandomEvent', 'SwitchEvent') else 'interact', text=text)


def entrance(map_id, point):
    """A real native arrival is a waypoint, never a claim that an unknown
    remote-map local leaves its control visible. Observe that floor on entry.
    """
    import bake_maps
    room=MAPS[map_id]['map_name']
    found=[o for o in OBJECTS if o['submap']==room and o['type_name']=='PointIn'
           and o['props'].get('object_id')==point]
    assert len(found)==1,(map_id,point)
    actor=found[0]
    info=json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[room]
    ox,oy,_,_=bake_maps.map_bounds(info,[o for o in OBJECTS if o['submap']==room])
    return dict(map=map_id,x=actor['tx']-ox,y=bake_maps.tile_row(actor)-oy,
                layer=bake_maps.layer_index(actor['layer']),kind='spot',label='Passage',
                text='Continue through the passage.')


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


# Full reviewed native bodies remain external; only source identity is stored.
PHOENIX_SCRIPT_SHA = {
    "Map_30781_1/sc_e_0800": "62c0b4700752d4b9fed16a1a0a66731f4d4b2702e0d796b1869548dec8cba336",
    "Map_30781_1/sc_e_0800_1": "6edc72c3075639eb0480e3567c98cf2e6927abdcfa671a10c1101aae81fa8246",
    "Map_30781_1/sc_e_0801": "bc49c97fc03e9db06c64c451795ad81e076b06fa47cfe9a00e2f1b4ef9a0bb93",
    "Map_30781_1/sc_e_0801_1": "c91f3fe8d03c263aef5eb8ade55d4c508fbd372b2b729a86a9713384f96bc692",
    "Map_30781_1/sc_e_0799_2": "4a02a79e8c00f456a0d65ab53af489ae837ba006b95e9e98869f2617b6055441",
    "Map_30781_2/sc_e_0811": "5f9b2a4452e10418068295a0d32f19ea203c2854546cfafde652cbb6c5d5f9f1",
    "Map_30781_2/sc_e_0816": "5da10d119efce06b3b59c3c97510e90c119c918099dfe3e3e2e6c9a286bcfdd2",
    "Map_30781_2/sc_e_0817": "be8abfb442df9dddfbb94d273fbf552d16c447c583b6262d1415fc552086dbed",
    "Map_30781_2/sc_e_0828": "709b05d28833b5fb11df20956703a3a670abe7dcd37d7007299669f985332e9c",
    "Map_40012/sc_e_0828_1": "29399a6983b080c0e584746f4a84421fb9eca038d39d4e3cb8f03bed52e10278",
    "Map_20381_5/sc_e_0828_2": "e2d5c456f5cc017a40f9d7ca96c1e4511d3b475479a717734a905cdbc8ae3e23",
    "Map_20381_4/sc_e_0828_3": "349470455d9c258f2131c865f40a64e488d17fd13cdfae64034cf0bd25c56d34",
    "Map_20380/sc_e_0828_4": "1e1e42e55346f0240d8437eb4f57f06cbfca65f0fb5ecb05f6dd16b33b5283e6",
    "Map_30781_1/sc_map_30781_1": "960447177634cdd21fa17b203f32d28403570ee54c8bf854c69ebe034b7af9e2",
    "Map_30781_2/sc_map_30781_2": "2521dcc47b90173b5058c1709e1ba138a07771fe88b3ce782bad08bcd561304f",
}


def author_guides():
    for locator,expected in PHOENIX_SCRIPT_SHA.items():
        room,name=locator.split('/',1)
        body=SCRIPTS[room,name]['mnemonics']
        actual=hashlib.sha256(json.dumps(body,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
        assert actual==expected, ('Phoenix native source changed; recheck handoffs/poses/reunion',locator)
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
    phoenix = []
    for entity,occupied in ((40,401),(41,400)):
        actor=physical(498,entity)
        leave=SCRIPT_NAMES[actor['props']['leave_script_id']]
        body=SCRIPTS[actor['submap'],leave]['mnemonics']
        writes=[m for m in body if m['mnemonic'] in ('SetFlag','ResetFlag')]
        assert len(writes)==1 and writes[0]['mnemonic']=='ResetFlag' and \
            writes[0]['operands']['iValues'][0]==occupied-256 and \
            writes[0]['operands']['sValues'][0]=='ScenarioFlag2'
        # A manual premature arrival at a final plate must not strand the
        # other party on6. Release this exact plate and finish the7 handoff.
        phoenix.append(dict(when=[flag(402),condition('area',6,1),condition('area',7)],
                            at=target(498,entity,''),release=['scenario',occupied,0],leave_script=leave,
                            note='Stay on this switch. Change parties and guide the other party across the rocks.'))
        phoenix += [hold(498,entity,402,[condition('area',6)]),
                    hold(498,entity,402,[condition('area',6,1),condition('area',7,1)])]
    # The two lower rock toggles precede the upper-floor handoff. The moving
    # rock passage needs6 until the other party reaches7;7 then supplies the
    # jumping stones for the drain approach.
    phoenix += [dict(when=[flag(402),condition('area',6,1),condition('area',7)],
                     at=target(498,49,''),note='Hold switch. Switch parties.'),
                dict(when=[flag(402),flag(400),flag(401),condition('area',7,1)],
                     at=target(498,44,''),note='Hold switch. Switch parties.')]
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
    phoenix.append(dict(when=[flag(402,1),flag(408)],
                        targets=[target(499,50,'Reach {0} to reunite the parties and advance the Phoenix Cave story.',spot=True)],
                        finish_on_exit=262))
    phoenix += [rule([flag(399)], target(498, 56, 'Operate {0} to drain the water and cool the lava.')),
                rule([flag(403)], target(498, 98, 'Step on {0} to extend the bridge.', spot=True)),
                rule([flag(405)], target(499, 66, 'Step on {0} to raise the stepping stones.', spot=True))]
    # These two native events set406/reset407 and set407/reset406. Both can
    # precede the drain; neither needs a holder. Only the initial all-zero state
    # requests58, so the later407 state cannot oscillate back to that objective.
    phoenix += [rule([flag(405,1),flag(399),flag(406),flag(407)],target(499,58,switch,spot=True)),
                rule([flag(406,1)],target(499,55,switch,spot=True))]
    # This native save prompt lies across the upper room's northward passage.
    # Crossing it is a separate, explicit event objective, not permission to
    # route through arbitrary active events on the way to a distant switch.
    phoenix.append(rule([flag(405,1),flag(399),flag(406,1),
                         condition('map',0,498),condition('player_y',0,29,'ge')],
                        target(498,143,'Cross the save point. Close any prompt, then use Shift+G.',
                               'entity_default',20,spot=True)))
    # Complete reachable permanent mechanisms before revisiting released
    # temporary plates. The full episode otherwise cycles backwards forever.
    for local, entity in ((1, 108), (2, 94), (3, 86), (4, 77), (5, 61), (6, 49), (7, 44)):
        next_plate=target(498, entity, plate, spot=True)
        phase=[flag(405)] if local<=4 else [flag(399)] if local==5 else \
            [flag(402),flag(400),flag(401)] if local==6 else [flag(402)]
        phoenix.append(rule([*phase,condition('area', local)], next_plate))
    for entity,missing,other in ((41,400,401),(40,401,400)):
        phoenix += [rule([flag(399,1),flag(402),flag(missing),condition('area',7,1)],target(498,entity,plate)),
                    rule([flag(399,1),flag(402),flag(missing),flag(other,1),condition('area',7),condition('area',6)],target(498,entity,plate))]
    guides.append(guide('phoenix_cave', [497, 498, 499],
        'Phoenix Cave uses two parties. Keep a party on pressure switches while the other advances. The guide follows current switch and rock state.',
        phoenix, [flag(408)]))

    tower_maps = list(range(542, 572)) + [590, 595, 596]
    tower = [hold(549, i, 483) for i in (37, 38)]
    for entity, completed in ((37,486),(38,487)):
        tower.append(dict(when=[flag(483,1),flag(completed)],at=target(549,entity,''),
                          leave_switch=True,leave_direction=[0,1],
                          note='The door is open. Move off this switch so another party can push the weight.'))
    # The central party steps over its later three-party plate on the way to
    # the weights. Holding there immediately strands both outer parties below.
    # Until both weights fall, prove departures against the actual leave flag;
    # afterwards each party holds its final plate until the shared latch.
    for entity,occupied in ((40,488),(41,489),(44,490)):
        actor=physical(565,entity)
        leave=SCRIPT_NAMES[actor['props']['leave_script_id']]
        body=SCRIPTS[actor['submap'],leave]['mnemonics']
        assert any(m['mnemonic']=='ResetFlag' and m['operands']['iValues'][0]==occupied-256
                   and m['operands']['sValues'][0]=='ScenarioFlag2' for m in body), (entity,leave)
        for pending in ([flag(486)],[flag(486,1),flag(487)]):
            tower.append(dict(when=[flag(491),*pending],at=target(565,entity,''),
                              release=['scenario',occupied,0],leave_script=leave,
                              note='Stay on this switch. Change parties and guide another party to the remaining switch. Leave only after the shared passage opens.'))
        tower.append(hold(565,entity,491,[flag(486,1),flag(487,1)]))
    tower += [hold(570, i, 501) for i in (14, 15, 16)]
    # Guardian leaves the player south of a mandatory save-point event. Its
    # coordinates in EncountBoss focus the visual effect, not the player.
    # Guide that exact informational event as a waypoint, then continue from
    # its cell; never relax the safety policy for arbitrary active events.
    import bake_maps
    for map_id,entity,group,completed in ((566,51,'ev_e_0992_1',492),
                                        (567,25,'ev_e_0996_1',495),
                                        (568,22,'ev_e_0997_1',496),
                                        (569,0,'entity_default',494)):
        save=physical(map_id,entity,group,20)
        assert save['layer']=='GroundObject' and save['w']==save['h']==16
        save_script=SCRIPTS[save['submap'],SCRIPT_NAMES[20]]
        assert {m['mnemonic'] for m in save_script['mnemonics']} <= {
            'Nop','PlaySE','Flash','Branch','SetFlag','Msg','Select','Exit'}
        writes=[m for m in save_script['mnemonics'] if m['mnemonic']=='SetFlag']
        assert len(writes)==1 and writes[0]['operands']['iValues'][0]==7 and \
            writes[0]['operands']['sValues'][0]=='ScenarioFlag1'
        info=json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[save['submap']]
        _,oy,_,_=bake_maps.map_bounds(info,[o for o in OBJECTS if o['submap']==save['submap']])
        save_y=bake_maps.tile_row(save)-oy
        # Goddess resumes on Upper and descends to the Ground save point.
        # Other native rooms use Ground; existing route geometry owns entry.
        tower.append(rule([flag(completed,1),condition('map',0,map_id),
                           condition('player_layer',0,1,'ge' if map_id==568 else 'eq'),
                           condition('player_y',0,save_y+1,'ge')],
                          target(map_id,entity,'Cross the save point. Close any prompt, then use Shift+G.',
                                 group,20,spot=True)))
        if map_id==569:
            # The preceding playable room must target this intermediate prompt,
            # rather than asking a strict final-switch route to cross it unseen.
            tower.append(rule([flag(completed,1),condition('map',0,542)],
                              target(map_id,entity,'Cross the save point. Close any prompt, then use Shift+G.',
                                     group,20,spot=True)))
    # The complete episode must resume from each victory scene's real final
    # player pose, not from the preceding interaction approach. Pin those
    # factual coordinates/layers and the PointIn anchors used for crop offsets.
    for map_id,script_name,ending,point,anchor in (
            (542,'sc_e_0994_1',(30,20),8,(9,17)),
            (567,'sc_e_0996_1',(65,14),2,(64,11)),
            (568,'sc_e_0997_1',(12,33),2,(12,32))):
        body=SCRIPTS[MAPS[map_id]['map_name'],script_name]['mnemonics']
        entries=[m['operands']['iValues'][2] for m in body
                 if m['mnemonic']=='SetPuppet' and m['operands']['iValues'][1]==-1001]
        moves=[]
        for m in body[entries[-1]:]:
            if m['mnemonic']=='Exit':break
            if m['mnemonic'] in ('SetPos','Lerp'):moves.append(m)
        lerps=[m for m in moves if m['mnemonic']=='Lerp']
        assert tuple(lerps[-1]['operands']['iValues'][:2])==ending
        if map_id==542:
            assert moves[-1]['mnemonic']=='SetPos' and moves[-1]['operands']['iValues'][:5]==[0,0,2,1,-1001]
        if map_id==567:
            assert moves[0]['mnemonic']=='SetPos' and moves[0]['operands']['iValues'][:3]==[64,15,2]
        points=[o for o in OBJECTS if o['submap']==MAPS[map_id]['map_name'] and o['type_name']=='PointIn'
                and o['props']['object_id']==point]
        assert len(points)==1 and (points[0]['tx'],points[0]['ty'])==anchor, (map_id,point,[(o['tx'],o['ty']) for o in points])
    # This glowing chest is a required control, not ordinary treasure. The
    # native sc_e_0973 latches Flag2[219], opening the left route onward.
    for map_id,entity,script_name,index in ((542,88,'sc_e_0973',219),(560,32,'sc_e_0980',224)):
        actor=physical(map_id,entity)
        body=SCRIPTS[actor['submap'],script_name]['mnemonics']
        writes=[m for m in body if m['mnemonic']=='SetFlag']
        assert len(writes)==1 and writes[0]['operands']['iValues'][0]==index and writes[0]['operands']['sValues'][0]=='ScenarioFlag2'
    tower.append(rule([flag(475)],target(542,88,switch,'ev_e_0973')))
    tower.append(rule([flag(480)],target(560,32,switch)))
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
                                           (542, 93, 494, None), (567, 2, 495, None),
                                           (568, 2, 496, 'ev_e_0997'), (568, 12, 496, 'ev_e_0997')):
        tower.append(rule([flag(completed)], target(map_id, entity, 'Reach {0} and defeat the boss to open this route.', group)))
    # Goddess has two native interaction actors for the same battle, one on
    # each walkway layer. An Upper-only objective omits the Ground approach.
    for entity,layer in ((2,'UpperObject'),(12,'GroundObject')):
        actor=physical(568,entity,'ev_e_0997',1025)
        assert actor['layer']==layer and actor['props']['accept_action_direction']==4
    tower += [rule([flag(501), flag(f)], target(570, i, plate)) for i, f in ((15, 498), (16, 499), (14, 500))]
    guides.append(guide('kefkas_tower', tower_maps,
        'Kefka’s Tower uses three parties. Hold the outer switches to open the central route, move clear before dropping weights, then place all three parties on the final switches.',
        tower, [flag(501)]))

    #160/162/163 count failures;165 is actual performance success. The native
    # retry conversation sets164 and reuses the same stage event/group0318.
    success=SCRIPTS[MAPS[113]['map_name'],'sc_e_0320_1']['mnemonics']
    assert any(m['mnemonic']=='SetFlag' and m['operands']['sValues'][0]=='ScenarioFlag1' and
               m['operands']['iValues'][0]==165 for m in success)
    assert not any(m['mnemonic']=='SetFlag' and m['operands']['sValues'][0]=='ScenarioFlag1' and
                   m['operands']['iValues'][0]==160 for m in success)
    opera = [rule([flag(158)],target(112,21,'Speak to {0} to prepare or retry the performance.','ev_e_0315'))]
    for entity,conditions,group in (
            (20,[flag(160,1),flag(161)],'ev_e_0322'),
            (19,[flag(160,1),flag(161,1),flag(162,1),flag(163),condition('area',6)],'ev_e_0325'),
            (18,[flag(160,1),flag(161,1),flag(162,1),flag(163,1),condition('area',6)],'ev_e_0327')):
        opera.append(rule([condition('map',0,112),*conditions],
                          target(112,entity,'Speak to {0} to prepare or retry the performance.',group)))
    opera += [rule([flag(158,1),flag(159),flag(165)], target(113,6,'Speak to {0} in the dressing room.','ev_e_0316')),
              rule([flag(159,1),flag(160),flag(165)],target(113,7,'Enter the stage through {0}. Follow the spoken lyric choices and native performance prompts.','ev_e_0318',spot=True)),
              rule([flag(159,1),flag(160,1),flag(164,1),flag(165)],target(113,7,'Enter the stage through {0}. Follow the spoken lyric choices and native performance prompts.','ev_e_0318',spot=True)),
              rule([flag(161,1),flag(164),flag(165)], target(113,12,'Speak to {0} to prepare the retry.','ev_e_0323')),
              rule([flag(165,1),flag(174),flag(166)],target(113,13,'Read {0} to discover the sabotage.','ev_e_0329')),
              rule([flag(165,1),flag(174),flag(166,1),flag(167)],target(114,10,'Speak to {0} to continue.','ev_e_0330')),
              rule([flag(175,1),flag(176)],target(114,10,'Speak to {0} to continue.','ev_e_0330')),
              rule([flag(167,1),flag(168),condition('map',0,115,'ne')],entrance(115,1)),
              rule([flag(167,1),flag(168),condition('map',0,115)],target(115,6,'Pull {0}, the far-right lever. The other three levers are traps.')),
              rule([flag(167,1),flag(168,1)],target(117,5,'Reach {0} across the rafters before the timer expires.','ev_e_0344'))]
    # The exterior letter is enabled after the director's first retry, or after
    # the later retry's native area6 receipt. The first timed failure still
    # requires the normal director conversation; it must not jump straight here.
    for ready in ([flag(162)],[flag(162,1),condition('area',6,1)]):
        opera.append(rule([flag(174,1),flag(175),flag(160,1),flag(161,1),*ready],
                          target(112,17,'Read {0} to discover the sabotage.','ev_e_0342')))
    for entity, done in zip((13, 12, 11, 10, 9), range(169, 174)):
        opera.append(rule([flag(167,1),flag(168,1),flag(done)], target(117,entity,encounter)))
    opera.append(dict(when=[condition('map', 0, 576)],
                      note='Follow the spoken lyric choices and native performance prompts. Use the movement prompts on stage; the game controls the performance timer.'))
    guides.append(guide('opera', list(range(112, 119)) + [576],
        'Opera guidance follows the current performance and sabotage phase. The timer continues normally.', opera, [flag(157,1),flag(177)]))
    guides[-1].update(complete_scenario=177,finish_on_completion=True,inactive='No puzzle guide for this map')

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

    # The memory puzzle checks ordered regions, not shortest travel to its exit.
    # Pin every accepted predecessor directly to the matching external script.
    def light_target(entity,text,group=None):
        row=target(320,entity,text,group,spot=True)
        row['avoid_other_events']=True
        return row
    light=[dict(when=[flag(559,1)],note='Press a direction to resolve the light trap, then use Shift+G.'),
           dict(when=[flag(559),condition('local',1,99,'ge')],note='Leave and re-enter this room to restart the light path.'),
           rule([flag(559),condition('local',1,0)],light_target(17,'Start the light demonstration.'))]
    for value,entity in enumerate((20,33,29,28,30,24,23,18,19,22,35,31,32),2):
        actor=physical(320,entity)
        script=SCRIPTS[actor['submap'],SCRIPT_NAMES[actor['props']['script_id']]]
        assert script['mnemonics'][1]['mnemonic']=='Calc' and script['mnemonics'][1]['operands']['iValues'][:2]==[2,value]
        assert script['mnemonics'][1]['operands']['sValues'][:3]==['MapLocalVariable','←','imm']
        assert script['mnemonics'][2]['mnemonic']=='Branch' and script['mnemonics'][2]['operands']['iValues'][:2]==[1,value-1]
        assert script['mnemonics'][2]['operands']['sValues'][:3]==['MapLocalVariable','＝','imm']
        assert script['mnemonics'][12]['mnemonic']=='Calc' and script['mnemonics'][12]['operands']['iValues'][:2]==[1,2]
        assert script['mnemonics'][12]['operands']['sValues'][:3]==['MapLocalVariable','←','MapLocalVariable']
        light.append(rule([flag(559),condition('local',1,value-1)],light_target(entity,'Follow the light path.')))
    exit_actor=physical(320,4,'ev_e_0609')
    assert exit_actor['type_name']=='GotoMap' and exit_actor['props']['map_id']==317
    exit_target=light_target(4,'Leave through the northern exit.','ev_e_0609')
    exit_target['kind']='transition'
    light.append(dict(when=[flag(559),condition('local',1,14)],targets=[exit_target],finish_on_exit=317))
    guides.append(guide('narshe_light_path',[320],'',light,[flag(67,1),flag(297)]))
    guides[-1]['inactive']='No puzzle guide for this map'
    guides[-1]['blocked']='Leave and re-enter this room to restart the light path.'

    # The requested guide is brief: route first, then the action/required switch.
    # Threat interception is ranked from captured moving actors by the worker;
    # otherwise the actual event-completion actor is the first objective.
    for identity,map_id,actor,group,warning in (
            ('terra_defense',318,32,'ev_e_0018','Terra in danger.'),
            ('narshe_battle',19,64,'ev_e_0257','Banon in danger.')):
        entry=next(g for g in guides if g['id']==identity)
        entry.update(complete_scenario=14 if identity=='terra_defense' else 138,
                     inactive='No puzzle guide for this map')
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
        if entry['id'] in ('phoenix_cave','kefkas_tower'):
            entry['blocked']='Switch parties.'
        for row in entry['rules']:
            if 'note' in row:row['note']=short.get(row['note'],row['note'])
            for candidate in row.get('targets',[]):
                candidate['text']=short.get(candidate['text'],candidate['text'])
                if entry['id'] in ('phoenix_cave','kefkas_tower') or \
                   (entry['id']=='opera' and candidate.get('map')==117):
                    # The timed rafters have actual contact-encounter goals.
                    # If a rat blocks Ultros, choose a reachable fight rather
                    # than accepting a generic wait prefix as the objective.
                    candidate['avoid_other_events']=True
                    candidate['allow_scripted_crossings']=True
    from extend_phantom_train_guide import extend as phantom_stages
    import sys
    guides=[phantom_stages(sys.modules[__name__],g)if g['id']=='phantom_train'else g for g in guides]
    path = ROOT/'data/ff6/puzzles.json'
    data = json.loads(path.read_text(encoding='utf-8'))
    data.setdefault('_comment', ('Original factual navigation recipes. Static puzzles retain their original authorship. '
                        'Story guides use native completion flags and reachable live targets, not proximity completion. '
                        'See docs/re/FF6_STORY_NAVIGATION.md for source locators, controls, limits and pending gameplay confirmation.'))
    from author_dream_train_guide import author as dream_train
    guides.append(dream_train(target,guide,entrance))
    from author_zone_eater_roof import author as zone_roof
    guides.append(zone_roof(physical,SCRIPTS,OBJECTS))
    from author_zone_eater_crossings import author as zone_crossings
    import sys
    guides.extend(zone_crossings(sys.modules[__name__]))
    from author_darills_tomb_guide import author as darill
    guides.append(darill(sys.modules[__name__]))
    from author_owzers_mansion_guide import author as owzers_mansion
    guides.append(owzers_mansion(sys.modules[__name__]))
    from author_zozo_clock_guide import author as zozo_clock
    guides.append(zozo_clock(sys.modules[__name__]))
    from author_sealed_gate_guide import author as sealed_gate
    guides.append(sealed_gate(sys.modules[__name__]))
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
    for map_id in (383,498,501,506,556,542,559,564,567):
        for o in OBJECTS:
            if o['submap']!=MAPS[map_id]['map_name']:continue
            sid=o['props'].get('script_id')
            # Only the reviewed native travel families, not repulsion events,
            # battle/cutscene relocations, exits, traps or arbitrary scripts.
            doom_stairs=map_id==567 and sid==1111 and o['props'].get('entity_id')==0 and \
                o['group']=='entity_default' and o['name']=='sc_e_1105:（共通）階段を上る・降りる'
            rock_passage=map_id==498 and sid==1108 and o['props'].get('entity_id') in (205,206) and \
                o['type_name']=='Event' and o['group']=='entity_default' and o['name']=='sc_e_0800_1:動く岩を通り過ぎる'
            if not sid or not (doom_stairs or rock_passage or any(t in o['name'] for t in ('飛び移る','梯子昇降','エスカレーターで進む','ベルトコンベア'))):continue
            source=SCRIPTS.get((o['submap'],SCRIPT_NAMES[sid]))
            assert source,(map_id,sid)
            ms=source['mnemonics'];variants={}
            for facing in range(4):
                # This exact stair has no east/west Ground approach in the
                # native collision grid. Those script directions cannot be
                # operated here; the real-geometry suite rejected both arcs.
                if doom_stairs and facing in (1,3):continue
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
                         same_map=True,interaction_travel=manual,approach_mask=mask if manual or rock_passage else 0,
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

def author_scene_crossings():
    from scripted_transition_identity import tower_destination
    import bake_maps
    path=ROOT/'data/ff6/warp_arrival_overrides.json'
    rows=json.loads(path.read_text(encoding='utf-8'))
    for map_id,entity,final_map,point,continuation,landing in (
            (546,12,544,3,'sc_e_0970_2',[48,24,1]),
            (544,13,546,2,'sc_e_0971_2',[9,13,2])):
        actor=physical(map_id,entity)
        assert tower_destination(actor,SCRIPT_NAMES[actor['props']['script_id']],SCRIPTS)==(final_map,point,continuation)
        body=SCRIPTS[MAPS[final_map]['map_name'],continuation]
        # The reviewed player puppet finishes at this absolute Lerp, after
        # the duct's explicit Upper SetPos. Camera movement is a separate actor.
        player_entries=[m['operands']['iValues'][2] for m in body['mnemonics']
                        if m['mnemonic']=='SetPuppet' and m['operands']['iValues'][1]==-1001]
        assert player_entries
        last=[]
        for entry in player_entries:
            segment=[]
            for m in body['mnemonics'][entry:]:
                if m['mnemonic']=='Exit':break
                segment.append(m)
            last+=segment
        lerps=[m for m in last if m['mnemonic']=='Lerp']
        assert lerps[-1]['operands']['iValues'][:2]==landing[:2]
        if entity==13:
            assert any(m['mnemonic']=='SetPos' and m['operands']['iValues'][:3]==[6,13,3] for m in last)
        info=json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))[MAPS[final_map]['map_name']]
        ox,oy,_,_=bake_maps.map_bounds(info,[o for o in OBJECTS if o['submap']==MAPS[final_map]['map_name']])
        row=dict(submap=actor['submap'],entity_id=entity,object_id=actor['id'],interaction_travel=True,
                 destination_map=final_map,original_point=point,continuation=continuation,
                 continuation_sha256=hashlib.sha256(json.dumps(body,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest(),
                 arrival_cell=landing,arrival_baked_cell=[landing[0]-ox,landing[1]-oy,landing[2]],
                 evidence='Exact three-script roller/duct transfer through movie545; final player Lerp/layer. docs/re/SHIFTG_PUZZLE_AUDIT_20261030.md')
        old=next((r for r in rows if r['submap']==actor['submap'] and r.get('object_id')==actor['id']),None)
        if old:old.update(row)
        else:rows.append(row)
    path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Reviewed two Tower roller/duct scene chains')


if __name__=='__main__':
    author_guides()
    author_crossings()
    author_directional_crossings()
    author_scene_crossings()
