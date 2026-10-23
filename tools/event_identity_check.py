"""Validate reviewed identities through the real all-six native/baked keys.

Reads extracted game data outside Git. Reports contain only counts, locators and
hashes; no scripts, dialogue bodies or recovered implementation is exported.
"""
from pathlib import Path
import collections
import csv
import hashlib
import json
from event_label_identities import REVIEWS, SPRITES

ROOT=Path(__file__).absolute().parent.parent
DUMP=ROOT.parent/'tools/mapdump'
NEUTRAL={'Interactive object','Event trigger','Scene trigger'}

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def check():
    report=[]
    for n in range(1,7):
        source=DUMP/f'ff{n}_objects.jsonl'
        objects=list(map(json.loads,source.read_text(encoding='utf-8').splitlines()))
        labels_path=ROOT/f'data/ff{n}/eventlabels.json'
        labels=json.loads(labels_path.read_text(encoding='utf-8'))
        audit=json.loads((ROOT/f'docs/re/ff{n}_event_label_audit.json').read_text(encoding='utf-8'))
        rows={(r['submap'],r['group'],r['id']):r for r in audit}
        maps=collections.defaultdict(list)
        with (DUMP/f'ff{n}_master/map.txt').open(encoding='utf-8-sig') as f:
            for r in csv.DictReader(l for l in f if l.strip()):
                if r['id']:maps[r['map_name']].append(int(r['id']))
        checked=0;lookups=0;contexts=0
        for o in objects:
            p=o['props']
            if o['type'] not in (5,10) or not(p.get('script_id') or p.get('action_id') or p.get('message_key')):continue
            checked+=1
            expected=rows[(o['submap'],o['group'],o['id'])]['label']
            assert expected not in NEUTRAL, f'FF{n}: unresolved actionable identity {o["submap"]}/{o["group"]}/{o["id"]}'
            assert maps[o['submap']], f'FF{n}: native map alias missing'
            for map_id in maps[o['submap']]:
                lookups+=1
                key=f'@object:{map_id}:{p.get("entity_id",-1)}:{o["type"]}:{o["name"]}'
                remote=labels.get(key) if p.get('entity_id',-1)>=0 else None
                contexts+=bool(remote)
                remote=remote or labels.get(o['name'],'Interactive object')
                # This is the actual copied remote consumer. A correct audit
                # classification alone cannot certify a usable runtime key.
                assert remote==expected, (n,map_id,p.get('entity_id'),o['name'],expected,remote)
                live=SPRITES[n].get(p.get('asset_id',0),remote)
                assert live==expected, f'FF{n}: live sprite/context disagree at {key}: {live}/{expected}'
        record={'game':n,'actionable_entity_candidates':checked,'map_alias_lookups':lookups,
                'context_lookups':contexts,'unresolved':0,'lookup_mismatches':0,
                'object_dump_sha256':sha(source),'eventlabels_sha256':sha(labels_path)}
        report.append(record)
        print(f'FF{n}: {checked} actionable Entity/AnimEntity candidates, {lookups} live/remote aliases, zero unresolved or mismatched keys')
    # An override is always attached to the exact native source it was reviewed
    # against; stale entries must fail generation rather than silently relabel.
    assert len(REVIEWS)==len(json.loads((ROOT/'tools/event_identity_reviews.json').read_text(encoding='utf-8')))
    receipt={'reviewed_occurrences':len(REVIEWS),'games':report,
             'scope':'All extracted type5/10 candidates with an action, script or direct message; inactive/no-action helpers are separate.',
             'limits':'Offline source/key coverage; native runtime activation, cue timing and physical speech remain gameplay checks.'}
    (ROOT/'build_check/event_identity_report.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    return receipt

if __name__=='__main__':check()
