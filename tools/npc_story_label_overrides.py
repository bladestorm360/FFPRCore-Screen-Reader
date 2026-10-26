"""Exact user-reviewed navigation labels that would reveal future story facts.

This is a catalog correction, not a generic NPC visibility/reveal system. Native
identity and eligibility remain untouched. Keep the recipe checkable/replayable
without regenerating unrelated translations or importing proprietary scripts.
"""
from pathlib import Path
import argparse
import json

ROOT = Path(__file__).resolve().parents[1]
LANGS = ('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
# Full-width is the real FFII property; ASCII prevents normalization/fallback
# from exposing the ordinary name for the same reviewed developer variant.
MASKS = {'ff2': ('皇帝（復活後）', '皇帝(復活後)')}


def apply():
    for game, aliases in MASKS.items():
        path = ROOT / 'data' / game / 'translation.json'
        catalog = json.loads(path.read_text(encoding='utf-8-sig'))
        for alias in aliases:
            catalog[alias] = {lang: '???' for lang in LANGS}
            catalog['@npc-mask/'+alias] = {lang: '???' for lang in LANGS}
        path.write_text(json.dumps(catalog, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')


def check():
    for game, aliases in MASKS.items():
        catalog = json.loads((ROOT/'data'/game/'translation.json').read_text(encoding='utf-8-sig'))
        for alias in aliases:
            assert catalog[alias] == {lang:'???' for lang in LANGS}, (game,alias,'reviewed mask drift')
            assert catalog['@npc-mask/'+alias] == catalog[alias], (game,alias,'policy mask drift')
        assert catalog['皇帝']['en'] == 'Emperor', 'ordinary known identity must remain named'
    # Audit real all-six NPC placements, retaining raw identity outside the
    # catalog. A future additional occurrence requires context review; never
    # infer concealment from distance or a broad narrative-word blacklist.
    placements = []
    source = ROOT.parent/'tools/mapdump'
    for n in range(1,7):
        for line in (source/f'ff{n}_objects.jsonl').open(encoding='utf-8-sig'):
            obj = json.loads(line)
            if obj['type_name'] not in ('NPC','ShopNPC'):
                continue
            if obj['name'] in MASKS.get(f'ff{n}', ()):
                placements.append((n,obj['submap'],obj['group'],obj['props']['entity_id'],obj['type']))
    assert placements == [(2,'Map_30141_11','ev_e_0085',9,6)], placements
    print('NPC story labels: exact reviewed variant masked in all12 locales; all-six placement/identity audit passes')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true', help='apply only the reviewed exact aliases')
    args = parser.parse_args()
    if args.apply:
        apply()
    check()
