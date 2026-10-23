"""Source-free receipt for reviewed FF5/FF6 multipart identities.

Reads extracted masters/assets outside Git; writes hashes and original factual
identity decisions only. No extracted game scripts or images enter this repo.
"""
import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXTERNAL = ROOT.parent / 'tools'
EVIDENCE = EXTERNAL / 'boss_parts_20261023'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    rows = []
    for game, identities in (
        (5, ((387, 'Center', '中央'), (388, 'Lower right', '右下'),
             (389, 'Upper right', '右上'), (390, 'Lower left', '左下'))),
        (6, ((257, 'Shell', '102_C00'), (309, 'Head', '103_C00'),
             (258, 'Shell', '102_C01'), (310, 'Head', '104_C00'),
             (287, 'Goddess', '127_C00'), (329, 'Demon', '144_C00'))),
    ):
        master = EXTERNAL / f'mapdump/ff{game}_master/monster.txt'
        monsters = {int(row['id']): row for row in csv.DictReader(master.open(encoding='utf-8-sig'))}
        for identifier, descriptor, proof in identities:
            monster = monsters[identifier]
            if game == 5:
                evidence = EVIDENCE / f'ff5_scripts/sc_ai_{identifier}_NeoExdeath.txt'
                title = json.loads(evidence.read_text(encoding='utf-8'))['Title']['main']
                assert proof in title, (identifier, 'part title mismatch')
                kind = 'Native AI title explicitly identifies this anatomical area'
            else:
                evidence = EVIDENCE / f'mn_ff6_{proof[:3]}_MN_FF6_{proof}.png'
                kind = 'Actual extracted sprite visually reviewed by Sol'
            assert evidence.is_file(), evidence
            rows.append(dict(game=game, monster=identifier, message=monster['mes_id_name'],
                             descriptor=descriptor, asset=int(monster['monster_asset_id']),
                             evidence=str(evidence), evidence_sha256=digest(evidence),
                             master=str(master), master_sha256=digest(master), method=kind))
    report = dict(date='2026-10-23', identities=rows,
                  additional_review=dict(
                      ff5='Soul Cannon already names Launchers separately; Neo Exdeath dummy511 and Stalker copies receive no inferred identity.',
                      ff6='Crane/Tentacle have positional variants without a verified anatomical adapter; Erebus clones remain ordinary duplicate letters.'),
                  limits='Target/status reads use current native master plus message identity. Opening roster and bestiary retain native display names. No gameplay confirmation.')
    path = ROOT / 'docs/re/boss_part_identities.json'
    path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'{len(rows)} reviewed boss-part identities; source assets remain outside Git')


if __name__ == '__main__':
    main()
