"""Validate the Forest of Moore's ordinary interactions and full scene arrivals."""
import json
import sys

from check_scripted_bakes import decode
from forest_journeys import HASHES, ROWS, destination
from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write

TOPIC = 'docs/re/FF5_FOREST_OF_MOORE_20261105.md'
PROOF = ROOT/'docs/re/ff5_forest_native_20261105.json'
REASONS = {
    'sc_e_0118': 'An automatic forest introduction changes the original story receipts. The movement is native staging.',
    'sc_e_0260': 'One tree interaction records437 and clears its approach. The native initializer selects the tunnel group after117.',
    'sc_e_0261': 'One tree interaction opens its tunnel and records438. There is no separate ordered switch challenge.',
    'sc_e_0263_3': 'Any of the twenty native rescue contacts runs the shared scene. Receipt339 reveals the refuge entrances without a required trigger order.',
    'sc_30421_1_healing_spring': 'One normal recovery interaction clears conditions and restores the party.',
    'sc_e_0264': 'One conversation records the refuge rest. Its player staging and return remain native.',
    'sc_e_0120': 'The automatic post-refuge scene records120 and its native continuation receipt.',
}


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if 'Forest of Moore' in row['area_names_en']]
    assert len(selected) == 26 and {row['script'] for row in selected} == set(REASONS)
    rooms = {row['submap'] for row in selected}
    wanted = set(HASHES) | {(row['submap'],row['script']) for row in selected} | {
        (room,'sc_map_'+room[4:]) for room in rooms}
    scripts = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            if key in wanted:
                scripts[key] = script
    assert scripts.keys() == wanted
    pins = {'/'.join(key):signature(row['mnemonics']) for key,row in sorted(scripts.items())}
    baked,_ = decode(ROOT/'data/ff5/maps.bin')
    arrivals = []
    with (DUMP/paths['objects']).open(encoding='utf-8') as stream:
        for line in stream:
            actor = json.loads(line)
            key = actor['submap'],actor['id']
            if key not in ROWS:
                continue
            entity,_,group,name = ROWS[key]
            map_id,point,continuation = destination(actor,scripts[key[0],name],scripts)
            arc, = (row for row in baked[key[0]]['objects'] if row[0] == 100 and row[8] == entity
                    and row[10:] == (name,group))
            assert arc[6:8] == (map_id,point) and arc[9] == 1
            arrivals.append(dict(submap=key[0],entity=entity,map=map_id,point=point,continuation=continuation))
    assert len(arrivals) == len(ROWS) == 7

    decisions = {}
    for row in selected:
        key = row['submap']+'/'+row['script']
        assert pins[key] == source['source_script_hashes'][key]
        decision = {name:row[name] for name in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=pins[key],disposition='ordinary navigation/story interaction',
                        guides=[],reason=REASONS[row['script']],evidence=TOPIC)
        decisions[identity(row)] = decision
    proof = dict(scope_tag='ff5_ForestOfMoore',source_script_hashes=pins,identities=len(selected),arrivals=arrivals)
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    ledger = read(path)
    rows = [decisions.pop(identity(row),row) for row in ledger['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == ledger['decisions'] and read(PROOF) == proof
    else:
        ledger['decisions'] = rows
        write(path,ledger)
        write(PROOF,proof)
    print(f'Forest of Moore: {len(selected)} ordinary identities, {len(scripts)} full programs, and seven exact playable arrivals.')


if __name__ == '__main__':
    main()
