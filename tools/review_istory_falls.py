"""Record the ordinary controls and exact source dispositions for Istory Falls."""
import json
import sys

from author_istory_falls import ROOT, DUMP, PROOF, TOPIC, digest, read, write

REASONS = {
    'sc_e_0317': 'One optional switch stops waterfall contact38 for two seconds. The exact current is guarded by native358. The timer callback restores it and invokes the trigger under the player. The main exit remains reachable without using the switch.',
    'sc_e_0318': 'One independent spike toggle for the nearby directional chest. Both states allow access to the main exit control.',
    'sc_e_0319': 'One independent spike toggle for the nearby directional chest. Both states allow access to the main exit control.',
    'sc_e_0320': 'One ordinary exit control changes404. All three native arrivals reach it in every layout, and only this control enables the two final drops.',
    'sc_e_0321_1': 'The two native exit contacts use the same fall to484/1. The complete destination callback ends the controlled player at the existing Ground12,36 landing.',
    'sc_e_0453': 'The unopened and opened chest actors share one direction-sensitive interaction. The existing treasure identity and receipts remain authoritative.',
    'sc_e_0457': 'The unopened and opened chest actors share one direction-sensitive interaction. The existing treasure identity and receipts remain authoritative.',
    'sc_30521_6_to_30521_8_1': 'The actual trap resets402 through404 before transfer and lands at483 Ground19,13 after the complete callback.',
    'sc_30521_6_to_30521_8_3': 'The actual trap resets402 through404 before transfer and lands at483 Ground6,18 after the complete callback.',
    'sc_e_0446': 'Ordinary seal-opening scene uses the native story prerequisite and records293.',
    'sc_e_0150': 'Ordinary fourth tablet reward and full native story receipts.',
    'sc_e_0151': 'Ordinary native encounter and its story receipts. This does not add a puzzle decision.',
    'sc_e_0524': 'Ordinary lamp pickup or restoration with the native possession and treasure receipts.',
}

def identity(row):
    return tuple(row[key] for key in ('game','submap','group','entity','raw'))

def main():
    check = '--check' in sys.argv
    proof = read(PROOF)
    master = read(ROOT / 'docs/re/ff5_remaining_native_20261104.json')
    source_files = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt',
                    'script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}
    for role, name in source_files.items():
        assert digest(DUMP/name) == master['source_sha256'][role], role
    selected = [row for row in master['candidates'] if 'Istory Falls' in row['area_names_en']]
    assert len(selected) == 16
    decisions = {}
    for row in selected:
        locator = row['submap'] + '/' + row['script']
        signature = proof['source_script_hashes'][locator]
        assert signature == master['source_script_hashes'][locator]
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=signature, disposition='ordinary navigation/story interaction', guides=[],
                        reason=REASONS[row['script']], evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT / 'docs/re/puzzle_semantic_review_20261101.json'
    old = read(path)
    rows = [decisions.pop(identity(row),row) for row in old['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == old['decisions']
    else:
        old['decisions'] = rows
        write(path,old)
    print('Istory Falls: 16 exact source dispositions. One ordinary exit switch and no new puzzle guide.')

if __name__ == '__main__':
    main()
