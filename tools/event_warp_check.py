"""Verify exact reviewed warp sources and live/remote naming keys offline."""
from pathlib import Path
import csv
import hashlib
import json
from event_label_identities import WARP_REVIEWS, reviewed_warp_identity

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent/'tools/mapdump'


def check():
    found=set()
    for n in range(1,7):
        path=DUMP/f'ff{n}_objects.jsonl'
        objects=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        scripts={(s['submap'],s['name']):s for s in
                 map(json.loads,(DUMP/f'ff{n}_scripts.jsonl').read_text(encoding='utf-8').splitlines())}
        with (DUMP/f'ff{n}_master/script.txt').open(encoding='utf-8-sig') as stream:
            names={int(r['id']):r['script_name'] for r in csv.DictReader(stream) if r['id']}
        aliases={}
        with (DUMP/f'ff{n}_master/map.txt').open(encoding='utf-8-sig') as stream:
            for row in csv.DictReader(stream):aliases.setdefault(row['map_name'],[]).append(int(row['id']))
        labels=json.loads((ROOT/f'data/ff{n}/eventlabels.json').read_text(encoding='utf-8'))
        count=0
        for obj in objects:
            key=(n,obj['submap'],obj['group'],obj['id'])
            if key not in WARP_REVIEWS:continue
            p=obj['props'];name=names[p['script_id']];script=scripts[(obj['submap'],name)]
            assert WARP_REVIEWS[key]['map_id'] in aliases[obj['submap']]
            assert reviewed_warp_identity(n,obj,name,script)[0]=='Warp Tile'
            for map_id in aliases[obj['submap']]:
                context=f'@object:{map_id}:{p["entity_id"]}:{obj["type"]}:{obj["name"]}'
                assert (labels.get(context) or labels.get(obj['name']))=='Warp Tile',(map_id,obj['name'])
            # Reject a future changed body and a similarly named unreviewed
            # occurrence. Do not treat every ChangeMap as a warp label.
            assert reviewed_warp_identity(n,{**obj,'id':-123456},name,script) is None
            try:reviewed_warp_identity(n,obj,name,{**script,'unknown_new_action':True})
            except AssertionError:pass
            else:raise AssertionError('changed reviewed script was silently accepted')
            found.add(key);count+=1
        if n==2:
            assert labels['パンデモニウムの入り口']=='Warp Tile'
            assert labels['パンデモニウム8へ']=='Warp Tile'
            assert labels['パンデモニウム10へ']=='Warp Tile'
        print(f'FF{n}: {count} exact travel pads verified; unknown occurrences and changed scripts rejected')
    assert found==set(WARP_REVIEWS) and len(found)==16
    catalog=json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))
    assert set(catalog['Warp Tile'])=={'en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt'}
    assert all(catalog['Warp Tile'].values())
    print('PASS16 reviewed pads, all-six naming-key coverage, all12 existing localized labels; native activation unchanged')


if __name__=='__main__':check()
