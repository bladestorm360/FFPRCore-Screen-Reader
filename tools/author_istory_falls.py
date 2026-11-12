"""Qualify the six native waterfall moves without adding puzzle guidance."""
from pathlib import Path
import hashlib
import json
import sys

import pefile
import bake_maps

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
PROOF = ROOT / 'docs/re/ff5_istory_falls_native_20261105.json'
TOPIC = 'docs/re/FF5_ISTORY_FALLS_20261105.md'
ROOM = 'Map_30521_3'
CURRENT = 1 << 24
SHAPES = {37:(1,16,11,2,17), 38:(1,9,8,2,45), 41:(0,20,10,1,None),
          42:(0,16,12,1,None), 43:(0,12,10,1,None), 44:(0,9,9,1,None)}

def read(path):
    return json.loads(path.read_text(encoding='utf-8'))

def write(path, value, *, compact=False):
    encoded = json.dumps(value, ensure_ascii=False, separators=(',', ':')) if compact else json.dumps(value, ensure_ascii=False, indent=2)
    path.write_text(encoded + '\n', encoding='utf-8')

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def json_digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, ensure_ascii=False, separators=(',', ':')).encode()).hexdigest()

def qualified():
    rooms = {'Map_30521_1',ROOM,'Map_30521_6','Map_30521_8','Map_30521_9'}
    programs = {}
    for line in (DUMP / 'ff5_scripts.jsonl').open(encoding='utf-8'):
        row = json.loads(line)
        if row['submap'] in rooms:
            programs[row['submap'] + '/' + row['name']] = row
    hashes = {name:json_digest(row['mnemonics']) for name,row in programs.items()}
    objects = []
    for line in (DUMP / 'ff5_objects.jsonl').open(encoding='utf-8'):
        row = json.loads(line)
        if row['submap'] == ROOM:
            objects.append(row)
    left, top, *_ = bake_maps.map_bounds(read(DUMP / 'ff5_submaps.json')[ROOM], objects)
    assert (left,top) == (0,0)
    overrides = []
    facts = []
    for entity,(layer,x,y,steps,to_layer) in SHAPES.items():
        actor, = (row for row in objects if row['type_name'] == 'Event' and row['props'].get('entity_id') == entity)
        assert actor['group'] == 'entity_default'
        assert (bake_maps.layer_index(actor['layer']), actor['tx'], bake_maps.tile_row(actor)) == (layer,x,y)
        assert actor['props']['script_id'] == (2340 if steps == 1 else 2341)
        name = 'sc_e_0316_4' if steps == 1 else 'sc_e_0316_5'
        source = programs[ROOM + '/' + name]
        body = source['mnemonics']
        assert len(body) == 22 and body[5]['operands']['iValues'][:3] == [0,-1001,9]
        assert body[10]['mnemonic'] == 'Move' and body[10]['operands']['iValues'][:3] == [2,steps,4]
        assert body[20]['mnemonic'] == 'LookAt'
        if to_layer is not None:
            switch, = (row for row in objects if row['type_name'] == 'ToLayer' and row['props'].get('entity_id') == to_layer)
            assert switch['tx'] == x and switch['props']['layer'] == 0
            assert y < bake_maps.tile_row(switch) <= y+steps
        landing = [x,y+steps,0]
        overrides.append(dict(submap=ROOM, entity_id=entity, destination_map=478, original_point=0,
                              continuation=name, continuation_sha256=json_digest(source), same_map=True,
                              arrival_cell=landing, ordinary_travel=True, automatic_current=True, evidence=TOPIC))
        facts.append(dict(entity=entity, editor=actor['id'], source=[layer,x,y], landing=landing,
                          south_steps=steps, layer_contact=to_layer, timer_controls_actor=(entity==38)))
    timer = programs[ROOM + '/sc_e_0317']['mnemonics']
    expiry = programs[ROOM + '/sc_e_0317_1']['mnemonics']
    assert timer[5]['mnemonic'] == 'SetTimer' and timer[5]['operands']['rValues'][0] == 2
    assert expiry[2]['mnemonic'] == 'ResetFlag' and expiry[5]['mnemonic'] == 'ChangeScriptAtOnTrigger'
    current, = (row for row in objects if row['props'].get('entity_id') == 38 and row['type_name'] == 'Event')
    guard = dict(id=38,type=2,group=current['group'],raw=current['name'],cell=[1,9,8],map_ids=[478],area_id=69,
                 entry=0,require_known=True,warp_mask=[CURRENT,CURRENT],destination=[-1,None],
                 code=[[2,1,102,0,-1,0,2],[6],[1,'@action'],[6]],
                 review='Only the exact timed waterfall arc is inactive while native358 is set. ' + TOPIC)
    # The generated point index is filled from its stable appended override.
    return overrides, guard, facts, hashes

def main():
    check = '--check' in sys.argv
    overrides, guard, facts, hashes = qualified()
    position = read(ROOT.parent / 'tools/ff5_library_20261101/puppet_position_provenance.json')
    captured = read(PROOF)['native_methods'] if PROOF.exists() else read(ROOT / 'build_check/broader_audit_20261104/ff5_IstoryFalls/motion_facts.json')
    assert len(captured) == 10
    dll = Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
    assert digest(dll) == position['dll_sha256'] and digest(ROOT.parent/'FF5/dump.cs') == position['dump_sha256']
    image = pefile.PE(str(dll), fast_load=True)
    for method in captured:
        assert hashlib.sha256(image.get_data(method['rva'],method['bytes'])).hexdigest() == method['sha256']

    arrival_path = ROOT / 'data/ff5/warp_arrival_overrides.json'
    arrivals = read(arrival_path)
    existing = [row for row in arrivals if row['submap'] == ROOM and row['entity_id'] in SHAPES]
    assert not existing or existing == overrides
    if not existing:
        assert not check
        arrivals.extend(overrides)
    index, = (i for i,row in enumerate(arrivals) if row['submap'] == ROOM and row['entity_id'] == 38)
    guard['destination'][1] = -100000-index
    state_path = ROOT / 'data/ff5/mapstate.json'
    state = read(state_path)
    events = state[ROOM].get('events', [])
    guards = [row for row in events if row.get('warp_mask') == [CURRENT,CURRENT]]
    proof = dict(scope_tag='ff5_IstoryFalls',source_script_hashes=hashes,native_methods=captured,
                 dll_sha256=position['dll_sha256'],dump_sha256=position['dump_sha256'],current_contacts=facts,
                 spike_control=20,exit_enable_scenario=404,ordinary_interactions=True)
    if check:
        assert guards == [guard] and read(PROOF) == proof
    else:
        assert not guards or guards == [guard]
        if not guards:
            state[ROOM]['events'] = events + [guard]
            write(state_path,state,compact=True)
        if not existing:
            write(arrival_path,arrivals)
        write(PROOF,proof)
    print('Istory Falls: six exact native current landings, one timed arc guard, and ordinary spike control.')

if __name__ == '__main__':
    main()
