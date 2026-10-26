"""Check physical Ur destinations against native placements, without game source.

The well is a GotoMap entrance; the stash and outdoor pickups are distinct native
TreasureBox occurrences. Do not synthesize objects from dialogue or label chests
with their contents. Inputs remain in the outside-Git map extraction directory.
"""
import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parent / 'tools/mapdump'
OUT = ROOT / 'build_check/ur_well_20261025'


def main():
    import npc_speaker_labels
    npc_speaker_labels.check()
    OUT.mkdir(parents=True, exist_ok=True)
    paths = [SOURCE/'ff3_objects.jsonl', SOURCE/'ff3_master/map.txt',
             SOURCE/'ff3_master/story_mes_en.txt']
    objects = [json.loads(line) for line in paths[0].read_text(encoding='utf-8').splitlines()]
    maps = {int(row['id']): row for row in csv.DictReader(paths[1].open(encoding='utf-8-sig'))}
    titles = json.loads((ROOT/'data/ff3/map_titles.json').read_text(encoding='utf-8'))
    expected = {336: {'text': 'Shed'}, 337: [{'text': 'Shed'}, {'text': 'Back Room'}],
                338: {'text': 'Well'}}
    entrances = []
    for source, target in ((5, 336), (336, 337), (5, 338)):
        candidates = [o for o in objects if o['submap'] == maps[source]['map_name']
                      and o['type'] == 3 and o['props'].get('map_id') == target]
        assert len(candidates) == 1, (source, target, 'native entrance changed')
        o = candidates[0]
        assert o['props']['action_id'] == 3 and not o['props'].get('script_id')
        assert titles[str(target)] == expected[target]
        assert maps[target]['map_title'] == 'None' and maps[target]['floor'] == '0'
        entrances.append({'source_map': source, 'destination_map': target,
                          'entity_id': o['props']['entity_id'], 'type': o['type'],
                          'native_point': o['props']['point_id'], 'authored_title': expected[target]})
    # Exact native identity and independent collection flags matter: two outdoor
    # pickups must never be mistaken for the three separate chests below the well.
    treasure = {}
    for map_id, expected_flags, pickup in ((5, {366, 367}, True),
                                         (337, {372, 373, 374, 375, 376}, False),
                                         (338, {377, 378, 379}, False)):
        rows = [o for o in objects if o['submap'] == maps[map_id]['map_name'] and o['type'] == 7]
        assert {o['props']['flag_id'] for o in rows} == expected_flags
        assert all(o['props']['pick_up'] == pickup for o in rows)
        treasure[map_id] = [{'entity_id': o['props']['entity_id'], 'flag_id': o['props']['flag_id'],
                             'pick_up': pickup} for o in rows]
    common = json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))
    for key in ('Shed', 'Well', 'Back Room'):
        assert set(common[key]) == {'en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt'}
        assert all(common[key].values())
    # Audit the actual physical well/fountain class in every game, especially I
    # and II. An automatic scene near a spring isn't itself a physical object.
    physical = {}
    for n in range(1, 7):
        audit = json.loads((ROOT/f'docs/re/ff{n}_event_label_audit.json').read_text(encoding='utf-8'))
        physical[n] = [{'submap': r['submap'], 'group': r['group'], 'id': r['id'], 'label': r['label']}
                       for r in audit if r['type'] in (5, 10) and r['label'] in ('Well', 'Fountain')]
    assert len(physical[1]) == 6 and len(physical[2]) == 1
    report = {'sources': {str(p.relative_to(ROOT.parent)): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in paths}, 'entrances': entrances, 'treasure_occurrences': treasure,
              'physical_well_fountain_audit': physical, 'new_synthetic_objects': 0,
              'treasure_contents_in_labels': False, 'native_game_run': False}
    (OUT/'native_locations.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print('Ur: three native entrance labels; two outdoor pickups, five shed and three well chests;')
    print('all12 locales; all-six existing physical well/fountain identities checked; passed')


if __name__ == '__main__':
    main()
