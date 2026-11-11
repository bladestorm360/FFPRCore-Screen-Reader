"""Bind scene, spring and boss roles to reviewed native event occurrences."""

import csv
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
SCENE_TRANSFER = 1 << 26
SCENES = {
    ('Map_30020', 8): (25, 'sc_e_0008', 192, 1, 'sc_e_0008_1'),
    ('Map_30020', 14): (415, 'sc_e_0130', 19, 19, 'sc_e_0130_1'),
    ('Map_30020', 17): (2000, 'sc_e_0203', 192, 1, 'sc_e_0203_1'),
    ('Map_20010', 58): (62, 'sc_e_0023', 1, 1, 'sc_e_0023_1'),
    ('Map_20010', 100): (464, 'sc_e_0167', 1, 4, 'sc_e_0167_1'),
}
# Exact encounter contact, map object asset (zero for a proxy), encounter and localized name.
BOSSES = {
    (5, 'Map_20041_15', 'ev_e_0183', 12): (4, 10, 43, 'sc_e_0183', 498, 'シヴァ'),
    (5, 'Map_20191_6', 'ev_e_0285', 6): (5, 5, 0, 'sc_e_0285', 482, 'オーディン'),
    (5, 'Map_30041_4', 'ev_e_0016', 50): (17, 10, 79, 'sc_e_0016', 440, 'ウイングラプター'),
    (5, 'Map_30060', 'entity_default', 116): (109, 5, 0, 'sc_e_0030', 442, 'セイレーン'),
    (5, 'Map_30141_2', 'ev_e_0081', 25): (11, 5, 0, 'sc_e_0081', 455, 'タイタン'),
    (5, 'Map_30190', 'ev_e_0073', 10): (10, 5, 0, 'sc_e_0073', 452, 'ソル・カノン'),
    (5, 'Map_30190', 'ev_e_0073', 11): (11, 5, 0, 'sc_e_0073', 452, 'ソル・カノン'),
    (5, 'Map_30441_17', 'ev_e_0269', 21): (60, 5, 0, 'sc_e_0269', 472, 'カーバンクル'),
    (5, 'Map_30531_10', 'ev_e_0166', 13): (8, 5, 0, 'sc_e_0166', 489, 'ものまね師　ゴゴ'),
    (5, 'Map_30601_4', 'ev_e_0352', 12): (5, 10, 154, 'sc_e_0352', 609, 'ネクロフォビア'),
    (6, 'Map_20210', 'ev_e_0428', 36): (1, 5, 106, 'sc_e_0428', 446, 'ガーディアン'),
}
SPRINGS = {
    (5, 'Map_30021_1', 'entity_default', 23): (5, 'sc_e_0205'),
    (5, 'Map_30421_1', 'entity_default', 19): (19, 'sc_30421_1_healing_spring'),
}


def scene_transfer(game, obj, script, destination):
    key = obj['submap'], obj['props'].get('entity_id')
    if game != 'ff5' or key not in SCENES:
        return 0
    script_id, name, target, point, continuation = SCENES[key]
    assert obj['type'] == 2 and obj['props']['script_id'] == script_id and obj['props']['action_id'] == 2
    assert script and script['name'] == name and destination == (target, point, continuation)
    main = next(segment for segment in script['segments'] if segment['Label'] == 'Main')
    body = script['mnemonics'][main['EntryPoint']:main['EntryPoint'] + main['Count']]
    change = next(index for index, command in enumerate(body) if command['mnemonic'] == 'ChangeMap')
    assert any(command['mnemonic'] == 'Msg' for command in body[:change]), 'Reviewed scene no longer leads with dialogue'
    return SCENE_TRANSFER


def apply_labels(game, labels, audit, objects, scripts, masters, maps):
    if game not in (5, 6):
        return
    expected = {key for key in BOSSES.keys() | SPRINGS.keys() if key[0] == game}
    audit_rows = {(row['submap'], row['group'], row['id']): row for row in audit}
    for obj in objects:
        local = obj['submap'], obj['group'], obj['id']
        key = game, *local
        if key not in expected:
            continue
        expected.remove(key)
        prop = obj['props']
        script_name = masters[prop['script_id']]
        script = scripts[obj['submap'], script_name]
        assert prop['action_id'] == 2 and obj['visible'] and maps[obj['submap']]
        commands = script['mnemonics']
        if key in BOSSES:
            actor, native_type, asset, name, boss, translation = BOSSES[key]
            assert (prop['entity_id'], obj['type'], prop['asset_id'], script_name) == (actor, native_type, asset, name)
            assert any(c['mnemonic'] == 'EncountBoss' and c['operands']['iValues'][0] == boss for c in commands)
            assert not any(c['mnemonic'] == 'ChangeMap' for c in commands)
            for map_id in maps[obj['submap']]:
                labels[f'@boss:{map_id}:{actor}:{native_type}:{obj["name"]}'] = '@name:' + translation
        else:
            actor, name = SPRINGS[key]
            assert (prop['entity_id'], obj['type'], script_name) == (actor, 2, name)
            assert {'RecoveryHP', 'RecoveryMP', 'RemoveCondition'} <= {c['mnemonic'] for c in commands}
            assert not any(c['mnemonic'] in {'ChangeMap', 'EncountBoss'} for c in commands)
            label = '@name:回復の泉'
            for map_id in maps[obj['submap']]:
                labels[f'@object:{map_id}:{actor}:2:{obj["name"]}'] = label
            audit_rows[local].update(label=label, reason='reviewed native healing spring event')
    assert not expected, expected


def read_rows(path):
    with path.open(encoding='utf-8-sig', newline='') as source:
        return list(csv.DictReader(line for line in source if line.strip()))


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def main():
    write = '--write' in sys.argv
    for game in (5, 6):
        title = f'ff{game}'
        master = DUMP / f'{title}_master'
        masters = {int(row['id']): row['script_name'] for row in read_rows(master / 'script.txt')}
        maps = {}
        for row in read_rows(master / 'map.txt'):
            maps.setdefault(row['map_name'], []).append(int(row['id']))
        objects = [json.loads(line) for line in (DUMP / f'{title}_objects.jsonl').read_text(encoding='utf-8').splitlines()]
        scripts = {(row['submap'], row['name']): row for row in (
            json.loads(line) for line in (DUMP / f'{title}_scripts.jsonl').read_text(encoding='utf-8').splitlines())}
        labels_path = ROOT / 'data' / title / 'eventlabels.json'
        audit_path = ROOT / 'docs/re' / f'{title}_event_label_audit.json'
        labels, audit = read_json(labels_path), read_json(audit_path)
        apply_labels(game, labels, audit, objects, scripts, masters, maps)
        translations = read_json(ROOT / 'data' / title / 'translation.json')
        for label in [labels[key] for key in labels if key.startswith('@boss:')] + (['@name:回復の泉'] if game == 5 else []):
            entry = translations[label.removeprefix('@name:')]
            assert all(entry.get(lang) for lang in ('en', 'ja', 'de', 'es', 'fr', 'it', 'ko', 'pt', 'ru', 'th', 'zhc', 'zht'))
        for path, value in ((labels_path, labels), (audit_path, audit)):
            if write:
                if value != read_json(path):
                    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
            else:
                assert value == read_json(path), path
    print(f'Verified {len(BOSSES)} scripted boss contacts and {len(SPRINGS)} healing spring event identities.')


if __name__ == '__main__':
    main()
