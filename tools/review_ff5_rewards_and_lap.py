"""Verify the independent tablet rewards and the mounted world checkpoints."""
import json
import sys

from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write

TOPIC = 'docs/re/FF5_REWARDS_AND_LAP_20261105.md'
PROOF = ROOT/'docs/re/ff5_rewards_and_lap_native_20261105.json'
WEAPONS = tuple(f'sc_e_{number:04}' for number in range(402,414))
LAP = tuple(f'sc_e_0278_{number}' for number in range(1,5))
REASONS = {
    ('Map_30311_4',''): 'The tablet actor is the ordinary display counterpart of the native seal control. It has no input script.',
    ('Map_30311_4','sc_e_0326'): 'The third claim uses this native scene to consume the active tablet opportunity and close the seal.',
    ('Map_30311_1','sc_e_0324'): 'The native service scene places the recovery pot and records its one-time receipt.',
    ('Map_30311_4','sc_e_0152'): 'One native seal interaction checks the real tablet receipts and uses the corresponding item. It then enables three free reward choices.',
    ('Map_30311_4','sc_e_0248'): 'An ordinary arrival scene introduces the weapons and records241.',
    ('Map_20201_4','sc_e_0278'): 'The Phantom Village reward checks the matching set of four mounted checkpoints and the native world receipt. Their order is free.',
}
REASONS.update({('Map_30311_4',name):
                'One free choice among unclaimed weapons after the native seal opens. The corresponding pickup receipt preserves a single claim.'
                for name in WEAPONS})
REASONS.update({('Map_10040',name):
                'A native route checkpoint records only its own lap progress. Only Boko transportation9 can trigger it. No earlier checkpoint is required.'
                for name in LAP})


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if (row['submap'],row['script']) in REASONS]
    assert len(selected) == 22
    wanted = {key for key in REASONS if key[1]} | {
        ('Map_30311_4','sc_map_30311_4'),('Map_30311_1','sc_map_30311_1'),('Map_10040','sc_map_10040')}
    bodies = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            if key in wanted:
                bodies[key] = script['mnemonics']
    assert bodies.keys() == wanted
    signatures = {'/'.join(key):signature(body) for key,body in sorted(bodies.items())}

    seal = bodies['Map_30311_4','sc_e_0152']
    assert [cmd['operands']['iValues'][0] for cmd in seal if cmd['mnemonic'] == 'UseItem'] == [52,53,54,55]
    for index,name in enumerate(WEAPONS):
        body = bodies['Map_30311_4',name]
        assert body[3]['operands']['iValues'][:3] == [203,1,5]
        assert body[4]['operands']['iValues'][:3] == [71,1,7]
        assert body[22]['mnemonic'] == 'SetFlag' and body[22]['operands']['iValues'][0] == 180+index
    native_lap = {}
    with (DUMP/paths['objects']).open(encoding='utf-8') as stream:
        for line in stream:
            actor = json.loads(line)
            entity = actor['props'].get('entity_id')
            if actor['submap'] == 'Map_10040' and entity in (1003,1004,1005,1006):
                assert actor['type_name'] == 'Event' and actor['props']['target_transportation_ids'] == '9'
                native_lap[entity] = actor
    assert len(native_lap) == 4
    state = read(ROOT/'data/ff5/mapstate.json')['Map_10040']['transport']
    for entity,actor in native_lap.items():
        assert state[f'2:{entity}:1:1:1'] == {actor['group']:{actor['name']:[9]}}
    labels = read(ROOT/'tools/world_event_reviews.json')
    assert {row['entity_id']:row['label'] for row in labels if row['game'] == 5 and row['submap'] == 'Map_10040'
            and row['entity_id'] in native_lap} == dict.fromkeys(native_lap,'Checkpoint')

    decisions = {}
    for row in selected:
        key = row['submap'],row['script']
        sha = signatures.get('/'.join(key))
        assert sha == source['source_script_hashes'].get('/'.join(key))
        decision = {name:row[name] for name in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=sha,disposition='ordinary navigation/story interaction',guides=[],
                        reason=REASONS[key],evidence=TOPIC)
        decisions[identity(row)] = decision
    proof = dict(scope_tag='ff5_RewardsAndLap',source_script_hashes=signatures,identities=len(selected),
                 tablet_items=[52,53,54,55],pickup_receipts=list(range(180,192)),
                 checkpoint_entities=sorted(native_lap),checkpoint_transport=9)
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
    print(f'FFV rewards and lap: {len(selected)} ordinary identities and {len(bodies)} pinned programs. All four checkpoints preserve Boko ownership.')


if __name__ == '__main__':
    main()
