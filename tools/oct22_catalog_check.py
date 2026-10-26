"""Check changed localization/identity data against real consumers and sources."""
from pathlib import Path
import json
import re
from modtext_oct22 import extend
from event_label_identities import SPRITES

ROOT=Path(__file__).absolute().parent.parent

def main():
    common=json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf8'))
    recipes=[];extend(recipes)
    langs={'en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt'}
    for _,key,entry,_ in recipes:
        assert set(entry)==langs and all(entry.values()), key
        assert common[key]==entry, f'Catalog/recipe drift: {key}'
        placeholders=set(re.findall(r'\{\d+\}',key))
        assert all(set(re.findall(r'\{\d+\}',value))==placeholders for value in entry.values()), key
    physical=0
    for n in range(1,7):
        labels=json.loads((ROOT/f'data/ff{n}/eventlabels.json').read_text(encoding='utf8'))
        override=json.loads((ROOT/f'data/ff{n}/mod_text.json').read_text(encoding='utf8'))
        names=json.loads((ROOT/f'data/ff{n}/translation.json').read_text(encoding='utf8'))
        for asset,label in SPRITES[n].items():
            assert labels[f'@sprite:{asset}']==label
        for raw,label in labels.items():
            if label=='@route-only':
                # Reviewed navigation metadata is filtered before speech. Its
                # defensive formatter yields the existing Event trigger key.
                assert n==3 and raw=='世界一周', (n,raw,label)
                assert set(common['Event trigger'])==langs
                continue
            if label.startswith('@name:'):
                entry=names.get(label[6:])
                assert entry and all(entry.get(lang) for lang in langs-{'ja'}), (n,label)
                continue
            # Every generated mod label has real catalog coverage in every UI language.
            translated=override.get(label,common.get(label))
            assert translated and set(translated)==langs and all(translated.values()), (n,label)
        audit=json.loads((ROOT/f'docs/re/ff{n}_event_label_audit.json').read_text(encoding='utf8'))
        physical+=sum(row['reason'].startswith(('reviewed physical','reviewed mapobject','physical-object','bare interactible')) for row in audit)
    gate=json.loads((ROOT/'data/ff2/eventlabels.json').read_text(encoding='utf8'))
    assert gate['鉄格子']=='Iron gate' and gate['@sprite:72']=='Iron gate'
    print(f'October22 catalogs: {len(recipes)} entries x12 locales; {sum(map(len,SPRITES.values()))} reviewed sprite IDs; {physical} physical identity rows; passed')

if __name__=='__main__': main()
