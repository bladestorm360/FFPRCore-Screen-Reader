"""Qualify the castle's exact source and preserve ordinary interactions."""
from pathlib import Path
import hashlib
import json
import sys

import pefile

from author_ff5_exdeath import ROOT, DUMP, TOPIC, author, read

BUILD = ROOT / 'build_check/broader_audit_20261104/ff5_Exdeath'
EXTERNAL = ROOT.parent / 'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_Exdeath'
PROOF = ROOT / 'docs/re/ff5_exdeath_native_20261105.json'
DLL = Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
ORDINARY = {
    'sc_e_0247': 'One automatic posture scene from a left contact. It records409 and applies the native posture condition.',
    'sc_e_0247_1': 'The matching right contact runs the same automatic posture scene and receipts.',
    'sc_e_0265': 'One ordinary illusion control hides its own actor and the associated obstruction, then records359.',
    'sc_e_0124_2': 'The initial empty chest records337 and treasure520. A repeat uses the native empty-chest message.',
}

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def main():
    check = '--check' in sys.argv
    guide, arc = author()
    native = read(ROOT.parent / 'tools/ff5_library_20261101/puppet_position_provenance.json')
    assert digest(DLL) == native['dll_sha256']
    assert digest(ROOT.parent / 'FF5/dump.cs') == native['dump_sha256']
    captured = read(PROOF)['native_methods'] if PROOF.exists() else read(BUILD / 'roulette_phase_facts.json')
    assert len(captured) == 5
    image = pefile.PE(str(DLL), fast_load=True)
    for method in captured + native['functions']:
        actual = hashlib.sha256(image.get_data(method['rva'], method['bytes'])).hexdigest()
        assert actual == method['sha256'], method['method']

    master = read(ROOT / 'docs/re/ff5_remaining_native_20261104.json')
    selected = [row for row in master['candidates'] if 'Castle Exdeath' in row['area_names_en']]
    assert len(selected) == 19
    source_files = {'objects':'ff5_objects.jsonl', 'scripts':'ff5_scripts.jsonl', 'maps':'ff5_master/map.txt',
                    'script_master':'ff5_master/script.txt', 'areas':'ff5_master/area.txt', 'english_names':'ff5_master/system_en.txt'}
    for role, name in source_files.items():
        assert digest(DUMP / name) == master['source_sha256'][role], role
    hashes = dict(guide['source_script_hashes'])
    for line in (DUMP / 'ff5_scripts.jsonl').open(encoding='utf-8'):
        script = json.loads(line)
        room, name = script['submap'], script['name']
        if room in ('Map_30221_4','Map_30441_10','Map_30441_19') and (name in ORDINARY or name == 'sc_map_' + room[4:]):
            body = json.dumps(script['mnemonics'], ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode()
            hashes[room + '/' + name] = hashlib.sha256(body).hexdigest()
    assert len(hashes) == 43
    for row in selected:
        locator = row['submap'] + '/' + row['script']
        assert hashes[locator] == master['source_script_hashes'][locator]

    tilemap = EXTERNAL / 'roulette_tilemap.json'
    tiles = read(tilemap)['tilesets'][0]['tiles']
    durations = [tuple(frame['duration'] for frame in tile['animation']) for tile in tiles
                 if len(tile.get('animation', ())) == 16]
    assert len(durations) == 9 and all(d == (50,) * 16 for d in durations)
    bundles = DLL.parent / 'FINAL FANTASY V_Data/StreamingAssets/aa/StandaloneWindows64'
    bundle, = bundles.glob('map_30441_assets_all_*.bundle')
    proof = dict(scope_tag='ff5_Exdeath', source_script_hashes=hashes, native_methods=captured,
                 dll_sha256=native['dll_sha256'], dump_sha256=native['dump_sha256'],
                 floor_asset=dict(bundle=bundle.name, bundle_sha256=digest(bundle), tilemap_sha256=digest(tilemap),
                                  frame_count=16, asset_frame_milliseconds=50, animated_tiles=9),
                 stopped_layouts=9, onward_layouts=[4,5,6], gate_state_preserved=True,
                 passage=arc, retained_position_methods=native['functions'])
    if check:
        assert read(PROOF) == proof
    else:
        PROOF.write_text(json.dumps(proof, indent=2) + '\n', encoding='utf-8')

    def identity(row):
        return tuple(row[key] for key in ('game','submap','group','entity','raw'))

    decisions = {}
    for row in selected:
        ordinary = ORDINARY.get(row['script'])
        if row['submap'] == 'Map_30441_13':
            explanation = 'Actual animation stop selects one of nine native layouts. Three reach the north stair. Every other stopped state has a tested center retry, including after the native encounter.'
        else:
            explanation = 'The collapse and restoration contacts change area state1. Native positions and the exact trap geometry qualify three safe legs to the final stair, including recovery from the lower floor.'
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=hashes[row['submap'] + '/' + row['script']],
                        disposition='ordinary navigation/story interaction' if ordinary else 'guided complex puzzle',
                        guides=[] if ordinary else [guide['id']], reason=ordinary or explanation, evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT / 'docs/re/puzzle_semantic_review_20261101.json'
    previous = read(path)
    remaining = dict(decisions)
    rows = [remaining.pop(identity(row), row) for row in previous['decisions']]
    rows.extend(remaining.values())
    if check:
        assert rows == previous['decisions']
    else:
        previous['decisions'] = rows
        path.write_text(json.dumps(previous, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Castle Exdeath: 19 identities, 43 field programs, five timing and four placement methods, and the exact same-floor passage.')

if __name__ == '__main__':
    main()
