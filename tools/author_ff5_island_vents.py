"""Verify Island Shrine's playable vent landings and ordinary interactions."""
from collections import defaultdict
import json
import sys

from bake_maps import tile_row
from event_label_identities import WORLD_EVENT_REVIEWS
from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write
from scripted_transition_identity import ISLAND_VENT_DESTINATIONS, destination

TOPIC = 'docs/re/FF5_ISLAND_SHRINE_20261105.md'
PROOF = ROOT/'docs/re/ff5_island_vents_native_20261105.json'
LANDINGS = {
    # map: (native room, starting cell, controlled-puppet move instruction)
    429:('Map_30471_3',(10,4),26),
    430:('Map_30471_4',(7,14),14),
    431:('Map_30471_5',(7,4),14),
    432:('Map_30471_6',(6,3),14),
    433:('Map_30471_7',(11,5),14),
}
REASONS = {
    'sc_e_0142':'Ordinary tablet pickup and boss encounter 488.',
    'sc_e_0493':'Automatic entry scene records scenario receipts 240 and 439.',
    'sc_30471_1_door':'Either contact opens the same door through receipt 439.',
    'sc_30471_16_door':'Either contact opens the same door through map local 1.',
    'sc_e_0305':'Changes the first vent lever and selects the native group. Guide scope remains pending.',
    'sc_e_0305_1':'Changes the second vent lever and selects the native group. Guide scope remains pending.',
}
REASONS.update({name:'Return vent runs through the animation floor and resumes in the switch room.'
                for name in ('sc_e_0304','sc_e_0447','sc_e_0448','sc_e_0449')})


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if 'Island Shrine' in row['area_names_en']]
    assert len(selected) == 12 and {row['script'] for row in selected} == REASONS.keys()
    vents = [row for row in WORLD_EVENT_REVIEWS if row['game'] == 5 and
             (row['submap'],row['id']) in ISLAND_VENT_DESTINATIONS]
    assert len(vents) == 8
    wanted = {(row['submap'],row['script']) for row in selected}
    for vent in vents:
        wanted.add((vent['submap'],vent['script_name']))
        wanted.update((child['submap'],child['script']) for child in vent['continuations'])
    wanted.add(('Map_30471_16','sc_e_0142_1'))
    scripts = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            row = json.loads(line)
            key = row['submap'],row['name']
            if key in wanted:
                scripts[key] = row
    assert scripts.keys() == wanted
    actors = {}
    room_points = defaultdict(dict)
    with (DUMP/paths['objects']).open(encoding='utf-8') as stream:
        for line in stream:
            row = json.loads(line)
            room = row['submap']
            key = room,row['id']
            if key in ISLAND_VENT_DESTINATIONS:
                actors[key] = row
            if room in {landing[0] for landing in LANDINGS.values()}:
                assert row['type_name'] != 'ToLayer',room
                if row['type_name'] == 'PointIn':
                    assert row['layer'] == 'GroundObject'
                    room_points[room][row['props']['object_id']] = (row['tx'],tile_row(row))
    overrides = []
    for vent in vents:
        locator = vent['submap'],vent['id']
        map_id,point,callback = destination('ff5',actors[locator],vent['script_name'],scripts,{})
        room,start,move_pc = LANDINGS[map_id]
        assert room_points[room][point] == start
        final = scripts[room,callback]
        body = final['mnemonics']
        # Both controlled poses move together. The 00 branch first sets its
        # own position. These rooms have no native layer changes along the move.
        if map_id == 429:
            assert all(body[pc]['mnemonic'] == 'SetPos' and body[pc]['operands']['iValues'][:2] == [10,5]
                       for pc in (15,18))
            start = (10,5)
        heading,steps = body[move_pc]['operands']['iValues'][:2]
        assert heading in (0,2) and steps > 0
        assert all(body[pc]['mnemonic'] == 'Move' and body[pc]['operands']['iValues'][:2] == [heading,steps]
                   for pc in (move_pc,move_pc+3))
        arrival = [start[0],start[1]+steps*(1 if heading == 2 else -1),1]
        overrides.append(dict(submap=locator[0],entity_id=vent['entity_id'],object_id=locator[1],
                              destination_map=map_id,original_point=point,continuation=callback,
                              continuation_sha256=signature(final),arrival_cell=arrival,evidence=TOPIC))

    proof = dict(source_script_hashes={'/'.join(key):signature(row['mnemonics'])
                                      for key,row in sorted(scripts.items())},arrivals=overrides)
    path = ROOT/'data/ff5/warp_arrival_overrides.json'
    old = read(path)
    owned = {(row['submap'],row['object_id']) for row in overrides}
    existing = [row for row in old if (row['submap'],row.get('object_id')) in owned]
    if check:
        assert existing == overrides and read(PROOF) == proof
    else:
        assert not existing or existing == overrides
        if not existing:
            write(path,old+overrides)
        write(PROOF,proof)

    decisions = {}
    for row in selected:
        key = row['submap']+'/'+row['script']
        sha = proof['source_script_hashes'][key]
        assert sha == source['source_script_hashes'][key]
        decision = {name:row[name] for name in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=sha,disposition='ordinary navigation/story interaction',
                        guides=[],reason=REASONS[row['script']],evidence=TOPIC)
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
    print(f'Island Shrine: {len(selected)} review identities, {len(scripts)} pinned programs and {len(overrides)} playable vent landings.')


if __name__ == '__main__':
    main()
