"""Read-only, repeatable all-six condition presentation facts; no asset copies.

Uses installed bundles and existing extracted CSV masters. Output contains only
counts, identifiers, hashes and prefab component topology, never game source or
serialized object dumps. Pair with map_location_native.py's exact native modes.
"""
import csv
import hashlib
import json
from pathlib import Path
import sys

import UnityPy

ROOT = Path(__file__).resolve().parents[2]
TITLES = ('FINAL FANTASY', 'FINAL FANTASY II', 'FINAL FANTASY III',
          'FINAL FANTASY IV', 'FINAL FANTASY V', 'FINAL FANTASY VI')


def rows(path):
    with path.open(encoding='utf-8-sig', newline='') as stream:
        return [r for r in csv.DictReader(stream) if r.get('id')]


def prefab_facts(bundle):
    env = UnityPy.load(str(bundle))
    transforms = {}
    components = {}
    reaction_types = set()
    for obj in env.objects:
        if obj.type.name == 'Transform':
            transforms[obj.path_id] = obj.read()
        if obj.type.name in ('ParticleSystem', 'ParticleSystemRenderer', 'SpriteRenderer', 'MeshRenderer'):
            data = obj.read()
            components.setdefault(data.m_GameObject.path_id, []).append(obj.type.name)
        if obj.type.name == 'MonoBehaviour':
            # Inspect only the known effect marker field. Do not retain the
            # typetree or code/script contents in any output.
            tree = obj.read_typetree()
            if isinstance(tree.get('currentType'), int):
                reaction_types.add(tree['currentType'])
    roots = []
    for transform in transforms.values():
        if transform.m_Father.path_id:
            continue
        counts = {}
        pending = [transform]
        visited = set()
        while pending:
            node = pending.pop()
            identity = node.m_GameObject.path_id
            assert identity not in visited, 'Malformed cyclic prefab'
            visited.add(identity)
            for kind in components.get(identity, []):
                counts[kind] = counts.get(kind, 0) + 1
            pending.extend(transforms[c.path_id] for c in node.m_Children)
        roots.append({'root': transform.m_GameObject.read().m_Name,
                      'objects': len(visited), 'components': counts})
    return {'roots': roots, 'reaction_types': sorted(reaction_types)}


def main():
    output = Path(sys.argv[1]).resolve()
    assert output.is_relative_to(ROOT / 'build_check')
    assert not output.exists(), 'Preserve previous evidence'
    games = []
    for number, title in enumerate(TITLES, 1):
        master = ROOT.parent / 'tools/mapdump' / f'ff{number}_master'
        conditions = rows(master / 'condition.txt')
        assets = {int(r['id']): r['asset_name'] for r in rows(master / 'battle_effect_asset.txt')}
        install = Path('D:/Games/SteamLibrary/steamapps/common') / (title + ' PR')
        bundle_index = {p.name: p for p in install.glob('*_Data/StreamingAssets/aa/StandaloneWindows64/*.bundle')}
        facts = []
        for identity in sorted({int(c['battle_effect_asset_id']) for c in conditions} - {0}):
            asset = assets[identity]
            matches = [p for name, p in bundle_index.items() if name.startswith(asset.lower() + '_assets_all_')]
            assert len(matches) == 1, (number, identity, asset, len(matches))
            bundle = matches[0]
            facts.append({'id': identity, 'asset': asset, 'bundle': bundle.name,
                          'sha256': hashlib.sha256(bundle.read_bytes()).hexdigest(), **prefab_facts(bundle)})
        games.append({'game': number, 'condition_rows': len(conditions),
                      'master_sha256': hashlib.sha256((master / 'condition.txt').read_bytes()).hexdigest(),
                      'conditions': [{'id': int(c['id']), 'type': int(c['condition_type']),
                                      'system': int(c['system_type']), 'icon': int(c['icon_id']),
                                      'effect': int(c['battle_effect_asset_id'])} for c in conditions],
                      'assets': facts})
        print(f'FF{number}: {len(conditions)} conditions, {len(facts)} actual effect bundles inspected')
    output.write_text(json.dumps(games, indent=2) + '\n', encoding='utf-8')


if __name__ == '__main__':
    main()
