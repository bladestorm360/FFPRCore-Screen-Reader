"""Pin ordinary FFV door, switch and hidden passage decisions."""
import json
import sys

from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write

TOPIC = 'docs/re/FF5_SHORT_PASSAGES_20261105.md'
PROOF = ROOT/'docs/re/ff5_short_passages_native_20261105.json'
REASONS = {
    ('Map_20191_1','sc_e_0435'): 'One treasury control sets the area-local latch, clears its blocker and enables the ordinary merchant scene.',
    ('Map_20191_1','sc_e_0235'): 'The automatic merchant scene follows the treasury opening and records316.',
    ('Map_20191_6','sc_e_0516'): 'The door can be approached on the castle side. Facing north reports the lock. The other side opens it and sets275.',
    ('Map_30021_4','sc_e_0207'): 'The two exact actors share one ordinary door interaction, whose native receipt527 clears the blocker.',
    ('Map_20201_6','sc_e_0280'): 'One box interaction opens the nearby passage and records the map-local latch.',
    ('Map_30191_11','sc_e_0228'): 'One switch clears the nearby door collider and records438. The rest of the Ronka route retains its actual stairs and pits.',
    ('Map_30590','sc_e_0345'): 'The ordinary exterior door interaction clears its collider and records440.',
    ('Map_30591_2','sc_e_0349'): 'One interior control uses the shared440 receipt and clears the matching corridor blocker.',
}


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    wanted = set(REASONS) | {(room,'sc_map_'+room[4:]) for room,_ in REASONS}
    scripts = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            if key in wanted:
                scripts[key] = signature(script['mnemonics'])
    assert scripts.keys() == wanted
    selected = [row for row in source['candidates'] if (row['submap'],row['script']) in REASONS]
    assert len(selected) == 10
    decisions = {}
    for row in selected:
        key = row['submap'],row['script']
        assert scripts[key] == source['source_script_hashes']['/'.join(key)]
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=scripts[key],disposition='ordinary navigation/story interaction',
                        guides=[],reason=REASONS[key],evidence=TOPIC)
        decisions[identity(row)] = decision
    proof = dict(scope_tag='ff5_ShortPassages',source_script_hashes={'/'.join(key):sha for key,sha in sorted(scripts.items())},
                 identities=len(selected),rooms=len({room for room,_ in wanted}))
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    review = read(path)
    rows = [decisions.pop(identity(row),row) for row in review['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == review['decisions'] and read(PROOF) == proof
    else:
        review['decisions'] = rows
        write(path,review)
        write(PROOF,proof)
    print(f'FFV short passages: {len(selected)} ordinary identities and {len(scripts)} full native programs.')


if __name__ == '__main__':
    main()
