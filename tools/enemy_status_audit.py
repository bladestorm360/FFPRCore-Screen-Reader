"""Audit all six condition presentation tables, retaining facts rather than game code.

The runtime policy only permits an exact condition name for a unique effect asset.
Shared/unknown visual identities use a generic description after renderer proof.
An asset ID is a candidate, never proof that an enemy actually displays it.
"""
import collections
import csv
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools' / 'mapdump'


def build(number):
    path = DUMP / f'ff{number}_master' / 'condition.txt'
    raw = path.read_bytes()
    rows = list(csv.DictReader(line for line in raw.decode('utf-8-sig').splitlines() if line.strip()))
    assets = collections.defaultdict(set)
    facts = []
    for row in rows:
        kind, asset = int(row['condition_type']), int(row['battle_effect_asset_id'])
        if asset > 0:
            assets[asset].add(kind)
        facts.append({'id': int(row['id']), 'type': kind, 'message_key': row['mes_id_name'],
                      'effect_asset': asset, 'native_target_type': int(row['target_type'])})
    policy = {str(asset): {'unique': len(types) == 1, 'types': sorted(types)}
              for asset, types in sorted(assets.items())}
    report = {'game': number, 'source': str(path.relative_to(ROOT.parent)),
              'source_sha256': hashlib.sha256(raw).hexdigest(), 'conditions': facts,
              'critical': [row for row in facts if row['type'] == 4],
              'limits': 'Asset metadata establishes candidates only. Runtime requires actual native enemy effect creation and visible renderer proof. Shared assets have no unique exact-name approval.'}
    return policy, report


def main():
    checking = '--check' in sys.argv
    for n in range(1, 7):
        policy, report = build(n)
        outputs = {ROOT / 'data' / f'ff{n}' / 'condition_visuals.json': policy,
                   ROOT / 'docs' / 're' / f'ff{n}_condition_visual_audit.json': report}
        for path, value in outputs.items():
            if checking:
                assert json.loads(path.read_text(encoding='utf-8')) == value, f'Stale condition evidence: {path}'
            else:
                path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        print(f'FF{n}: {len(report["conditions"])} condition records, {len(policy)} effect assets; Critical effect IDs {[row["effect_asset"] for row in report["critical"]]}')


if __name__ == '__main__':
    main()
