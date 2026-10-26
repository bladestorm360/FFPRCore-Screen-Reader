"""Reviewed generic NPC labels from their own direct dialogue speaker table.

Only factual locators/names enter the catalog. No scripts or dialogue bodies are
copied. A map-scoped alias is safe here because this map has one generic NPC.
Ambiguous maps require an entity-specific resolver; never replace global 'NPC'.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parent/'tools/mapdump'
KEY = 'N008_C03_011_01_01'
ALIAS = '@map/11/NPC'
LANGS = ('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')


def check(apply=False):
    found = []
    sources = {}
    for n in range(1, 7):
        paths = [SOURCE/f'ff{n}_objects.jsonl', SOURCE/f'ff{n}_master/story_cha_en.txt']
        sources.update({str(p.relative_to(ROOT.parent)): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths})
        speakers = dict(line.split('\t',1) for line in paths[1].read_text(encoding='utf-8-sig').splitlines() if '\t' in line)
        for line in paths[0].read_text(encoding='utf-8').splitlines():
            o = json.loads(line)
            if o['type'] != 6 or o['name'].lower() not in ('npc','fieldnpc',''):
                continue
            key = o['props'].get('message_key','')
            speaker = speakers.get(key, '')
            if speaker:
                found.append((n,o['submap'],o['group'],o['props']['entity_id'],o['name'],key,speaker))
    assert found == [(3,'Map_20011_6','entity_default',11,'NPC',KEY,'Dahn')], 'Generic named NPC placements changed; review context before extending aliases'
    objects = [json.loads(line) for line in (SOURCE/'ff3_objects.jsonl').read_text(encoding='utf-8').splitlines()]
    generic = [o for o in objects if o['submap']=='Map_20011_6' and o['type']==6 and o['name']=='NPC']
    assert len(generic)==1 and generic[0]['props']['message_key']==KEY
    maps = list(csv.DictReader((SOURCE/'ff3_master/map.txt').open(encoding='utf-8-sig')))
    assert [int(m['id']) for m in maps if m['map_name']=='Map_20011_6']==[11]
    path = ROOT/'data/ff3/translation.json'
    catalog = json.loads(path.read_text(encoding='utf-8'))
    import official_text
    tables = official_text.load('ff3')['story_cha']
    names = {lang: tables[lang][KEY] for lang in LANGS}
    assert all(names.values()) and names['en']=='Dahn'
    if apply:
        catalog[ALIAS] = names
        path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    assert catalog[ALIAS] == names, 'Dahn must use actual native speaker names in all12 languages'
    out = ROOT/'build_check/ur_well_20261025';out.mkdir(parents=True,exist_ok=True)
    bundle = next(Path(official_text.aa_path('ff3')).glob('message_assets_all_*.bundle'))
    report = {'sources': sources, 'named_generic_npcs': found, 'alias': ALIAS,
              'official_names': names, 'message_bundle_sha256': hashlib.sha256(bundle.read_bytes()).hexdigest(),
              'native_game_run': False, 'raw_entity_identity_changed': False}
    (out/'npc_speaker_names.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('All-six direct-speaker audit: Dahn is the only bare generic named NPC; map11 alias checked in12 native locales')


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply',action='store_true')
    check(parser.parse_args().apply)
