"""Generate the Great Sea Trench's native switch and chest guidance."""
import json
import sys

import bake_maps
import trench_state
from author_puzzle_types import expected
from ff5_rule_resources import update_header
from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write

ROOM = 'Map_30611_12'
TABLET = 'Map_30611_13'
TOPIC = 'docs/re/FF5_GREAT_SEA_TRENCH_20261105.md'
PROOF = ROOT/'docs/re/ff5_trench_native_20261105.json'
SWITCHES = ((192,8,'sc_e_0314_2'),(193,10,'sc_e_0314_3'),(194,9,'sc_e_0314_4'))
INTERACT = 'Interact, then request guidance again.'
REASONS = {
    'sc_e_0313':'An ordinary lamp-restoration contact uses the native possession receipt.',
    'sc_e_0315':'The skull control opens the switch room route and records the area-local latch.',
    'sc_e_0314_2':'One of three independent switch receipts. The guide directs the player to switches still needed before opening the chest.',
    'sc_e_0314_3':'One of three independent switch receipts. Its activation order is free.',
    'sc_e_0314_4':'One of three independent switch receipts. Its activation order is free.',
    'sc_e_0314_1':'Opening the chest early causes a six-tile knockback and resets all three switch receipts. All three together open the actual tablet passage.',
    'sc_e_0148':'The real end contact starts the encounter. Its native return records148 and grants the tablet.',
    '':'The actor is a staged member of the tablet encounter. It has no separate player action.',
    'sc_e_0309':'One ordinary switch opens its nearby path using area-local state2.',
    'sc_e_0310':'One ordinary switch closes the first section and changes a second path using area-local state3.',
    'sc_e_0311':'One independent optional chest passage control uses area-local state1.',
}
GUIDED = {'sc_e_0315','sc_e_0314_2','sc_e_0314_3','sc_e_0314_4','sc_e_0314_1','sc_e_0148'}


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if 'Great Sea Trench' in row['area_names_en']]
    assert len(selected) == 12 and {row['script'] for row in selected} == set(REASONS)
    rooms = {row['submap'] for row in selected}
    wanted = {(row['submap'],row['script']) for row in selected if row['script']}
    wanted |= {(room,'sc_map_'+room[4:]) for room in rooms} | {(TABLET,'sc_e_0148_1')}
    scripts = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            if key in wanted:
                scripts[key] = script
    assert scripts.keys() == wanted
    bodies = {key:script['mnemonics'] for key,script in scripts.items()}
    pins = {'/'.join(key):signature(body) for key,body in sorted(bodies.items())}
    chest = bodies[ROOM,'sc_e_0314_1']
    for index,receipt in enumerate((192,193,194)):
        assert chest[1+index*3]['mnemonic'] == 'Branch' and chest[1+index*3]['operands']['iValues'][:2] == [receipt,1]
        assert chest[17+index]['mnemonic'] == 'ResetFlag' and chest[17+index]['operands']['iValues'][0] == receipt
    assert chest[51]['mnemonic'] == 'Move' and chest[51]['operands']['iValues'][:2] == [2,6]
    assert chest[24]['mnemonic'] == 'SetFlag' and chest[24]['operands']['iValues'][0] == 195
    for receipt,_,name in SWITCHES:
        row = bodies[ROOM,name][3]
        assert row['mnemonic'] == 'SetFlag' and row['operands']['iValues'][0] == receipt
    reward = bodies[TABLET,'sc_e_0148_1'][14]
    assert reward['mnemonic'] == 'SetFlag' and reward['operands']['iValues'][0] == 148

    state_path = ROOT/'data/ff5/mapstate.json'
    state = read(state_path)
    record = state[ROOM]
    current_code = trench_state.live_code(scripts[ROOM,trench_state.INITIALIZER],record['code'])
    if check:
        assert record['current_code'] == current_code
    elif record.get('current_code') != current_code:
        record['current_code'] = current_code
        state_path.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')

    actors = []
    with (DUMP/paths['objects']).open(encoding='utf-8') as stream:
        for line in stream:
            actor = json.loads(line)
            if actor['submap'] in (ROOM,TABLET):
                actors.append(actor)
    drop, = (actor for actor in actors if actor['submap'] == ROOM and actor['type_name'] == 'ToLayer'
             and actor['props'].get('entity_id') == 53)
    assert (drop['tx'],bake_maps.tile_row(drop),drop['props']['layer']) == (17,16,0)
    bounds = read(DUMP/'ff5_submaps.json')

    def target(entity,room=ROOM):
        actor, = (row for row in actors if row['submap'] == room and row['type_name'] == 'Entity'
                  and row['props'].get('entity_id') == entity)
        left,top,*_ = bake_maps.map_bounds(bounds[room],[row for row in actors if row['submap'] == room])
        return dict(map=473 if room == ROOM else 474,entity=entity,raw=actor['name'],group=actor['group'],
                    layer=bake_maps.layer_index(actor['layer']),x=actor['tx']-left,y=bake_maps.tile_row(actor)-top,
                    kind='interact',text=INTERACT)

    def condition(bank,index,value):
        return [bank,index,'eq',value]

    rules = [dict(when=[condition('scenario',195,1)],targets=[target(1,TABLET)]),
             dict(when=[condition('area',0,0)],targets=[target(1)])]
    for mask in range(8):
        when = [condition('scenario',receipt,int(bool(mask & (1<<index))))
                for index,(receipt,_,_) in enumerate(SWITCHES)]
        targets = [target(entity) for index,(_,entity,_) in enumerate(SWITCHES) if not mask & (1<<index)]
        rules.append(dict(when=when,targets=targets or [target(11)]))
    guide = dict(id='trench_chest_switches',maps=[473,474],complete_scenario=148,rules=rules,evidence=TOPIC)
    proof = dict(scope_tag='ff5_GreatSeaTrench',source_script_hashes=pins,identities=len(selected),
                 switches=[list(row[:2]) for row in SWITCHES],chest=11,success=195,completion=148,
                 early_chest_push_south=6,reset_pose=[0,17,17],reset_receipts=[192,193,194],
                 live_projection_rows=len(current_code))
    puzzle_path = ROOT/'data/ff5/puzzles.json'
    data = read(puzzle_path)
    guides = [row for row in data['story_guides'] if row['id'] == guide['id']]
    assert not guides or guides == [guide]
    if check:
        assert guides == [guide] and read(PROOF) == proof
    else:
        if not guides:
            data['story_guides'].append(guide)
            data['guidance_types'] = expected(5,data)
            write(puzzle_path,data)
        write(PROOF,proof)
    update_header(data,check)

    decisions = {}
    for row in selected:
        name = row['script']
        key = row['submap']+'/'+name
        assert pins.get(key) == source['source_script_hashes'].get(key)
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=pins.get(key),disposition='guided mechanic' if name in GUIDED else 'ordinary navigation/story interaction',
                        guides=[guide['id']] if name in GUIDED else [],reason=REASONS[name],evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    ledger = read(path)
    rows = [decisions.pop(identity(row),row) for row in ledger['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == ledger['decisions']
    else:
        ledger['decisions'] = rows
        write(path,ledger)
    print(f'Great Sea Trench: {len(selected)} identities, {len(bodies)} full programs, and all eight native switch states.')


if __name__ == '__main__':
    main()
