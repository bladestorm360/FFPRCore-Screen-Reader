"""Source-qualified Ship Graveyard crossings and the connected puzzle guide."""
import csv
import hashlib
import json
from pathlib import Path
import sys

import pefile

import bake_maps
from author_puzzle_types import expected
from ff5_rule_resources import update_header

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent/'tools/mapdump'
ROOM = 'Map_30060'
MAP = 211
STORY_ROOMS = {'Map_30061_2': (213,7,'sc_e_0027',26,94,[7,16]),
               'Map_30061_9': (220,9,'sc_e_0028',27,726,[8,15])}
TOPIC = 'docs/re/FF5_SHIP_GRAVEYARD_20261105.md'
PROOF = ROOT/'docs/re/ff5_ship_graveyard_native_20261105.json'
DIRECTIONS = ((0,-1),(1,0),(0,1),(-1,0))
# Each entry has a real action start, incoming facing, and final controlled
# landing after all native continuations. Intermediate rock poses are not
# separate player-controlled route steps.
CROSSINGS = {
    102: ((2,17,33,13,51),),
    76: ((0,13,51,17,33),),
    106: ((1,32,33,36,33),),
    107: ((3,36,33,32,33),),
    104: ((0,30,46,30,44),(2,30,44,30,46)),
    105: ((1,31,48,33,48),(3,33,48,31,48)),
    83: ((1,37,49,45,47),),
    108: ((3,45,47,37,49),),
    89: ((2,47,23,47,33),),
    90: ((0,47,33,47,23),),
}


def read(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write(path,value):
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()


def signature(value):
    data = json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()
    return hashlib.sha256(data).hexdigest()


def room_rows(path):
    with path.open(encoding='utf-8') as stream:
        for line in stream:
            row = json.loads(line)
            if row['submap'] == ROOM or row['submap'] in STORY_ROOMS:
                yield row


def player_segment(body,facing):
    slots = {}
    stack = []
    controlled = []
    leader = []
    continuation = ''
    pc = 0
    for _ in range(64):
        command = body[pc]
        op = command['mnemonic']
        values = command['operands']['iValues']
        pc += 1
        if op == 'Call':
            stack.append(pc)
            pc = values[0]
        elif op == 'Return':
            pc = stack.pop()
        elif op == 'BranchByPlayerDirection':
            pc = values[facing]
        elif op == 'SetPuppet':
            slots[values[0]] = values[1:3]
        elif op == 'ExecPuppet':
            controlled.extend(entry for actor,entry in slots.values() if actor == -1001)
            leader.extend(entry for actor,entry in slots.values() if actor == 0)
            slots.clear()
        elif op == 'ChangeScript':
            continuation = command['operands']['sValues'][0]
        elif op == 'Exit':
            assert not stack and len(controlled) == len(leader) == 1
            return controlled[0],leader[0],continuation
        else:
            assert op in ('Nop','PlaySE','Wait'), (pc,op)
    raise AssertionError('Unresolved jump control')


def movement(body,entry,controlled):
    x = y = 0
    layer = 1
    for command in body[entry:entry+40]:
        op = command['mnemonic']
        values = command['operands']['iValues']
        if op == 'Move':
            direction,count = values[:2]
            assert 0 <= direction < 4 and count == 2 and values[2:5] == [0,0,0]
            dx,dy = DIRECTIONS[direction]
            x += dx*count
            y += dy*count
        elif op == 'SetPos':
            assert controlled and values[:2] == [0,0] and values[3:5] == [1,-1001]
            assert values[2] in (2,3)
            layer = values[2]-1
        elif op == 'Exit':
            return x,y,layer
        else:
            assert op in ('Nop','Blank','LookAt','Wait','PlaySE'), op
    raise AssertionError('Jump puppet did not exit')


def replay(bodies,name,facing,x,y):
    chain = []
    layer = 1
    while name:
        assert name not in chain and len(chain) < 8
        chain.append(name)
        body = bodies[name]
        player,leader,name = player_segment(body,facing)
        dx,dy,layer = movement(body,player,True)
        assert movement(body,leader,False)[:2] == (dx,dy)
        x += dx
        y += dy
    return [x,y,layer],chain


def review(guide,hashes,check):
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    locator = ROOM+'/sc_e_0029'
    assert hashes[locator] == source['source_script_hashes'][locator]
    candidate, = (row for row in source['candidates'] if row['submap'] == ROOM and row['script'] == 'sc_e_0029')
    keys = ('game','submap','group','entity','raw','script')
    identity = tuple(candidate[key] for key in keys)
    decision = dict(zip(keys,identity))
    decision.update(script_sha256=hashes[locator],disposition='guided mechanic',guides=[guide['id']],evidence=TOPIC,
                    reason='The single chest interaction clears the bridge during the larger jump sequence. Guidance follows the actual approaches and native28/29 progression.')
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    data = read(path)
    matching = [row for row in data['decisions'] if tuple(row[key] for key in keys) == identity]
    if check or matching:
        assert matching == [decision]
    else:
        data['decisions'].append(decision)
        write(path,data)


def qualified():
    all_scripts = {(row['submap'],row['name']):row for row in room_rows(DUMP/'ff5_scripts.jsonl')}
    scripts = {name:row for (room,name),row in all_scripts.items() if room == ROOM}
    bodies = {name:row['mnemonics'] for name,row in scripts.items()}
    actors = list(room_rows(DUMP/'ff5_objects.jsonl'))
    with (DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig',newline='') as stream:
        names = {int(row['id']):row['script_name'] for row in csv.DictReader(stream)}
    maps = read(DUMP/'ff5_submaps.json')
    bounds = bake_maps.map_bounds(maps[ROOM],[row for row in actors if row['submap'] == ROOM])
    assert bounds[:2] == (0,0)

    def actor(entity,kind,room=ROOM):
        result, = (row for row in actors if row['submap'] == room and row['props'].get('entity_id') == entity
                   and row['type_name'] == kind)
        return result

    def cell(row):
        return [row['tx'],bake_maps.tile_row(row),bake_maps.layer_index(row['layer'])]

    timer = bodies['sc_30060_timer_start'][2]
    assert timer['mnemonic'] == 'SetTimer' and timer['operands']['rValues'][0] == 2
    assert timer['operands']['sValues'][0] == 'sc_30060_stone'
    callback = bodies['sc_30060_stone'][5]
    assert callback['mnemonic'] == 'ChangeScript' and callback['operands']['sValues'][0] == 'sc_30060_jump_1'
    chest = bodies['sc_e_0029'][16]
    assert chest['mnemonic'] == 'SetFlag' and chest['operands']['iValues'][0] == 28
    boss = bodies['sc_e_0030'][106]
    assert boss['mnemonic'] == 'EncountBoss' and boss['operands']['iValues'][0] == 442
    assert boss['operands']['sValues'][0] == 'sc_e_0030_1'
    finish = bodies['sc_e_0030_1'][20]
    assert finish['mnemonic'] == 'SetFlag' and finish['operands']['iValues'][0] == 29

    overrides = []
    facts = []
    for entity,options in CROSSINGS.items():
        timed = entity == 102
        row = actor(entity,'Event' if timed else 'Entity')
        assert row['props']['action_id'] == 2 and row['group'] == 'entity_default'
        name = names[row['props']['script_id']]
        ax,ay,layer = cell(row)
        assert layer == 1
        for facing,sx,sy,x,y in options:
            dx,dy = DIRECTIONS[facing]
            assert (sx,sy) == ((ax,ay) if timed else (ax-dx,ay-dy))
            mask = 1 << ((facing+2)%4)
            if not timed:
                assert row['props']['accept_action_direction'] & mask
            root = 'sc_30060_jump_1' if timed else name
            landing,chain = replay(bodies,root,facing,sx,sy)
            assert landing == [x,y,1], (entity,landing)
            overrides.append(dict(submap=ROOM,entity_id=entity,object_id=row['id'],destination_map=MAP,
                                  original_point=0,continuation=name,continuation_sha256=signature(scripts[name]),
                                  same_map=True,arrival_cell=landing,ordinary_travel=timed,interaction_travel=not timed,
                                  approach_mask=mask,evidence=TOPIC))
            facts.append(dict(entity=entity,facing=facing,start=[sx,sy,1],landing=landing,chain=chain))

    def target(entity,kind,action,text,room=ROOM,map_id=MAP):
        row = actor(entity,kind,room)
        x,y,layer = cell(row)
        left,top,*_ = bake_maps.map_bounds(maps[room],[item for item in actors if item['submap'] == room])
        return dict(map=map_id,entity=entity,raw=row['name'],group=row['group'],layer=layer,x=x-left,y=y-top,kind=action,text=text)

    rules = []
    for room,(map_id,entity,name,receipt,pose_pc,pose) in STORY_ROOMS.items():
        body = all_scripts[room,name]['mnemonics']
        assert any(command['mnemonic'] == 'SetFlag' and command['operands']['iValues'][0] == receipt
                   for command in body)
        assert body[pose_pc]['mnemonic'] == 'SetPos' and body[pose_pc]['operands']['iValues'][:2] == pose
        rules.append(dict(when=[['scenario',receipt,'eq',0]],targets=[
            target(entity,'Event','spot','Cross this trigger, then request guidance again.',room,map_id)]))
    rules.extend([dict(when=[['scenario',28,'eq',0]],targets=[target(17,'Entity','interact','Interact, then request guidance again.')]),
                  dict(when=[['scenario',28,'eq',1]],targets=[target(109,'Entity','interact','Interact, then request guidance again.')])])
    guide = dict(id='ship_graveyard_jumps',maps=list(range(211,226)),when=[['scenario',25,'eq',1]],
                 complete_scenario=29,rules=rules,evidence=TOPIC)
    hashes = {room+'/'+name:signature(row['mnemonics']) for (room,name),row in sorted(all_scripts.items())}
    return overrides,guide,facts,hashes


def main():
    check = '--check' in sys.argv
    overrides,guide,facts,hashes = qualified()
    position = read(ROOT.parent/'tools/ff5_library_20261101/puppet_position_provenance.json')
    motion = read(ROOT/'docs/re/ff5_istory_falls_native_20261105.json')['native_methods']
    dll = Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
    assert digest(dll) == position['dll_sha256'] and digest(ROOT.parent/'FF5/dump.cs') == position['dump_sha256']
    image = pefile.PE(str(dll),fast_load=True)
    methods = position['functions'] + [method for method in motion if method['method'] == 'Puppet.Move']
    assert len(methods) == 5
    for method in methods:
        assert hashlib.sha256(image.get_data(method['rva'],method['bytes'])).hexdigest() == method['sha256']
    proof = dict(scope_tag='ff5_ShipGraveyard',dll_sha256=position['dll_sha256'],dump_sha256=position['dump_sha256'],
                 native_methods=methods,source_script_hashes=hashes,jumps=facts,seconds=2,chest_receipt=28,encounter_receipt=29)
    path = ROOT/'data/ff5/warp_arrival_overrides.json'
    old = read(path)
    found = [row for row in old if row['submap'] == ROOM and row['entity_id'] in CROSSINGS]
    puzzle_path = ROOT/'data/ff5/puzzles.json'
    data = read(puzzle_path)
    guides = [row for row in data['story_guides'] if row['id'] == guide['id']]
    assert len(guides) <= 1
    if check:
        assert read(PROOF) == proof and found == overrides and guides == [guide]
    else:
        write(PROOF,proof)
        if found != overrides:
            assert not found or old[-len(found):] == found
            write(path,old[:len(old)-len(found)]+overrides)
        if guides != [guide]:
            if guides:
                data['story_guides'][data['story_guides'].index(guides[0])] = guide
            else:
                data['story_guides'].append(guide)
            data['guidance_types'] = expected(5,data)
            write(puzzle_path,data)
    update_header(data,check)
    review(guide,hashes,check)
    print(f'Ship Graveyard: {len(hashes)} pinned programs and {len(overrides)} exact jump approaches.')


if __name__ == '__main__':
    main()
