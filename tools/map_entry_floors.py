"""Bake/check native floor-title keys without copying proprietary game source.

Inputs are offline object/master extracts outside Git. Only factual message/map
identifiers enter the existing embedded map-title resource. Authored entries and
explicit master titles retain precedence. Ambiguous, area-only, empty and named
non-floor entrance titles are deliberately excluded rather than guessed.
"""
from collections import defaultdict
import csv
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parent / 'tools/mapdump'
EVIDENCE = ROOT / 'build_check/map_floor_20261025'


def text_table(path):
    return dict(line.split('\t', 1) for line in
                path.read_text(encoding='utf-8-sig').splitlines() if '\t' in line)


def derive(maps, objects, messages):
    """Require all entrances to agree; never use map-id or floor arithmetic."""
    points = defaultdict(list)
    for row in objects:
        if row['type'] == 0:
            points[row['submap']].append(row.get('props') or {})
    result, records = {}, []
    for row in maps:
        if row['map_title'].lower() not in ('none', ''):
            continue
        entrances = points.get(row['map_name'], [])
        if not entrances or any(p.get('title_type') not in (1, 4) or
                                not p.get('title_id') for p in entrances):
            continue
        keys = {p['title_id'] for p in entrances}
        if len(keys) != 1:
            continue
        key = next(iter(keys))
        english = messages.get(key, '')
        if not re.fullmatch(r'(?:[1-9][0-9]*F|B[1-9][0-9]*)', english):
            continue
        result[row['id']] = key
        floor = int(row['floor'])
        old = f'{floor}F' if floor > 0 else f'B{-floor}' if floor < 0 else ''
        records.append({'map_id': int(row['id']), 'submap': row['map_name'],
                        'native_key': key, 'native_en': english,
                        'master_floor': floor, 'old_fallback': old,
                        'entrances': len(entrances), 'changed_en': old != english})
    return dict(sorted(result.items(), key=lambda pair: int(pair[0]))), records


def main():
    apply = '--apply' in sys.argv
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    report = {}
    for number in range(1, 7):
        game = f'ff{number}'
        master = SOURCE / f'{game}_master'
        paths = [master/'map.txt', SOURCE/f'{game}_objects.jsonl', master/'system_en.txt']
        with paths[0].open(encoding='utf-8-sig', newline='') as stream:
            maps = list(csv.DictReader(stream))
        objects = (json.loads(line) for line in paths[1].read_text(encoding='utf-8').splitlines())
        keys, rows = derive(maps, objects, text_table(paths[2]))
        target = ROOT / f'data/{game}/map_titles.json'
        document = json.loads(target.read_text(encoding='utf-8-sig'))
        # Preserve every existing authored map key. Generated metadata has its
        # own namespace, read only after authored and master title resolution.
        if apply:
            document['@entryFloors'] = keys
            target.write_text(json.dumps(document, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        assert document.get('@entryFloors') == keys, f'{game}: stale native floor-title data'
        for row in rows:
            row['authored_override'] = str(row['map_id']) in document
        active = [r for r in rows if not r['authored_override']]
        if '--locales' in sys.argv:
            import official_text
            official_text.INSTALL['ff6'] = ('FINAL FANTASY VI PR', 'FINAL FANTASY VI_Data')
            system = official_text.load(game)['system']
            for locale in official_text.LANGS:
                assert all(system.get(locale, {}).get(key) for key in keys.values()), (game, locale, 'missing native floor message')
            print(f'{game}: every generated key exists in all12 native language tables')
        report[game] = {'eligible': len(rows), 'active': len(active),
                        'changed_en': sum(r['changed_en'] for r in active),
                        'sources': {str(p.relative_to(ROOT.parent)): hashlib.sha256(p.read_bytes()).hexdigest()
                                    for p in paths}, 'maps': rows}
        if '--locales' in sys.argv:
            report[game]['verified_languages'] = official_text.LANGS
            bundles = list(Path(official_text.aa_path(game)).glob('message_assets_all_*.bundle'))
            assert len(bundles) == 1, (game, 'ambiguous native message bundle')
            report[game]['locale_bundle'] = {'path': str(bundles[0]),
                'sha256': hashlib.sha256(bundles[0].read_bytes()).hexdigest()}
        print(f'{game}: {len(rows)} native floor keys, {len(active)} active, '
              f'{report[game]["changed_en"]} corrected English floor labels')
    panda = {int(k): v for k, v in json.loads((ROOT/'data/ff2/map_titles.json').read_text(encoding='utf-8'))['@entryFloors'].items()
             if 314 <= int(k) <= 324}
    assert panda == {314: 'MSGARA_NAME_01', 315: 'MSGARA_NAME_09', 316: 'MSGARA_NAME_10',
                     317: 'MSGARA_NAME_02', 318: 'MSGARA_NAME_03', 319: 'MSGARA_NAME_04',
                     320: 'MSGARA_NAME_05', 321: 'MSGARA_NAME_05', 322: 'MSGARA_NAME_06',
                     323: 'MSGARA_NAME_07', 324: 'MSGARA_NAME_08'}
    # Adversarial input: mixed floors, unlabelled entrance, area-only types,
    # non-floor strings and explicit master names must never create a label.
    sample = {'id': '1', 'map_name': 'Fixture', 'map_title': 'None', 'floor': '13'}
    point = {'type': 0, 'submap': 'Fixture', 'props': {'title_id': 'floor', 'title_type': 1}}
    import copy
    for field, value in [('title_id', 'other'), ('title_id', ''), ('title_type', 0), ('title_type', 2), ('title_type', 5)]:
        second = copy.deepcopy(point)
        second['props'][field] = value
        assert not derive([sample], [point, second], {'floor': '1F', 'other': '2F'})[0]
    assert not derive([sample], [point], {'floor': 'Entrance'})[0]
    assert not derive([{**sample, 'map_title': 'official'}], [point], {'floor': '1F'})[0]
    name = 'entry_floor_audit_locales.json' if '--locales' in sys.argv else 'entry_floor_audit.json'
    (EVIDENCE/name).write_text(json.dumps(report, indent=2)+'\n')
    print('PASS all-six data, Pandaemonium11 maps and ambiguous/area-only/empty/master-title exclusions')


if __name__ == '__main__':
    main()
