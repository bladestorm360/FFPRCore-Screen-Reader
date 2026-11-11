"""Validate explicit candidate dispositions, not infer solutions from labels.

Inventories remain immutable discovery evidence. Decisions are original factual
review records, with source identities/hashes and authored-guide references.
Raw game implementations never enter this output or repository. --game permits
one finite game checkpoint; the final all-six check requires every candidate.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
INVENTORIES=('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json',
             'puzzle_mechanics_continuation_inventory_20261104.json')

def key(game,row):
    return (game,row['submap'],row['group'],row['entity'],row['raw'])

def instance(row):
    return dict(type=row['type'],cell=row['cell'],action=row['action'],script=row['script'])

def instance_key(row):
    return row['type'],tuple(row['cell']),row['action'],row['script']

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--game',choices=[f'ff{i}' for i in range(1,7)])
    parser.add_argument('--check-sources',action='store_true');args=parser.parse_args()
    review=json.loads((ROOT/'docs/re/puzzle_semantic_review_20261101.json').read_text(encoding='utf-8'))
    candidates={};sources={};facts={}
    for name in INVENTORIES:
        inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))
        for game,data in inventory['games'].items():
            if args.game and game!=args.game:continue
            if game in sources:assert sources[game]==data['source_sha256']
            sources[game]=data['source_sha256'];facts.setdefault(game,{}).update(data['script_facts'])
            for row in data['candidates']:
                identity=key(game,row);variants=candidates.setdefault(identity,[])
                assert all(instance_key(old)!=instance_key(row) for old in variants),('Repeated physical discovery record',identity)
                variants.append(row)
    decisions={};guide_ids={}
    for game in sources:
        data=json.loads((ROOT/f'data/{game}/puzzles.json').read_text(encoding='utf-8'))
        guide_ids[game]={p['id'] for section in ('puzzles','story_guides','form_guides') for p in data.get(section,[])}
        if game=='ff5':
            # These existing typed owners consume their own resource sections,
            # rather than the generic recipe arrays. No invented guide ID is
            # allowed to stand in for a missing shipped consumer.
            for section in ('catapult_guide','stone_guide','book_guide','pyramid_guide','jachol_guide','fire_ship_guide'):
                if section in data:
                    owned=data[section];assert owned['id'] and owned['maps']
                    guide_ids[game].add(owned['id'])
            if 'library_policy' in data:
                assert data['library_policy']['version']==1 and 0<len(data['library_policy']['rows'])<=512
                guide_ids[game].add('library_policy')
    for row in review['decisions']:
        game=row['game']
        if args.game and game!=args.game:continue
        identity=key(game,row);assert identity in candidates and identity not in decisions,identity
        variants=candidates[identity];original=variants[0]
        assert all(row['script']==v['script'] for v in variants),identity
        if len(variants)>1 or 'instances' in row:
            expected_instances=sorted((instance(v) for v in variants),key=instance_key)
            assert sorted(row.get('instances',[]),key=instance_key)==expected_instances,('Physical instances omitted or changed',identity)
        expected=facts[game].get(original['submap']+'/'+original['script'],{}).get('sha256')
        assert row['script_sha256']==expected,identity
        assert row['disposition'] and row['reason'] and row['evidence'],identity
        assert all(p in guide_ids[game] for p in row.get('guides',[])),identity
        if row['disposition'] in ('guided mechanic','decorative counterpart'):assert row['guides'],identity
        decisions[identity]=row
    missing=candidates.keys()-decisions.keys()
    assert not missing,('Unreviewed candidates',len(missing),list(sorted(missing))[:8])
    if 'ff2' in sources:
        inventory=json.loads((ROOT/'docs/re/ff2_keyword_mechanics_inventory_20261101.json').read_text(encoding='utf-8'))
        expected={(r['submap'],r['script']):r for r in inventory['scripts']}
        reviewed={}
        for row in review.get('ff2_keyword_decisions',[]):
            identity=row['submap'],row['script']
            assert identity in expected and identity not in reviewed,identity
            original=expected[identity]
            assert all(row[k]==original[k] for k in ('script_sha256','asset_sha256')),identity
            assert row['disposition'] and row['reason'] and row['evidence'],identity
            assert all(p in guide_ids['ff2'] for p in row.get('guides',[])),identity
            if row['disposition']=='guided mechanic':assert row['guides'],identity
            if args.check_sources:
                # Raw native declarations stay in their external source archive.
                source=ROOT.parent/'tools/ff2_keyword_puzzles_20261101/all_scripts'/('_'.join(identity)+'.json')
                assert hashlib.sha256(source.read_text(encoding='utf-8').encode('utf-8')).hexdigest()==row['asset_sha256'],identity
            reviewed[identity]=row
        assert reviewed.keys()==expected.keys(),('Unreviewed FFII keyword bindings',sorted(expected.keys()-reviewed.keys()))
        print('FFII keyword semantic review:',len(reviewed),'exact callers; no missing declarations.')
    if 'ff3' in sources:
        inventory=json.loads((ROOT/'docs/re/ff3_condition_mechanics_inventory_20261101.json').read_text(encoding='utf-8'))
        expected={(r['submap'],r['script']):r for r in inventory['scripts']};reviewed={}
        for row in review.get('ff3_condition_decisions',[]):
            identity=row['submap'],row['script']
            assert identity in expected and identity not in reviewed,identity
            assert row['sha256']==expected[identity]['sha256'],identity
            assert row['disposition'] and row['reason'] and row['evidence'],identity
            assert all(p in guide_ids['ff3'] for p in row.get('guides',[])),identity
            if row['disposition']=='guided mechanic':assert row['guides'],identity
            reviewed[identity]=row
        assert reviewed.keys()==expected.keys(),('Unreviewed FFIII condition callers',expected.keys()-reviewed.keys())
        if args.check_sources:
            actual={}
            for line in (ROOT.parent/'tools/mapdump/ff3_scripts.jsonl').open(encoding='utf-8'):
                native=json.loads(line);identity=native['submap'],native['name']
                if identity in expected:actual[identity]=hashlib.sha256(json.dumps(native['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
            assert all(actual.get(k)==r['sha256'] for k,r in reviewed.items()),'FFIII condition source changed'
        print('FFIII condition semantic review:',len(reviewed),'exact callers; no missing declarations.')
    if 'ff4' in sources:
        inventory=json.loads((ROOT/'docs/re/ff4_item_mechanics_inventory_20261101.json').read_text(encoding='utf-8'))
        expected={(r['submap'],r['script']):r for r in inventory['scripts']};reviewed={}
        for row in review.get('ff4_item_decisions',[]):
            identity=row['submap'],row['script']
            assert identity in expected and identity not in reviewed,identity
            assert row['sha256']==expected[identity]['sha256'],identity
            assert row['disposition'] and row['reason'] and row['evidence'],identity
            assert all(p in guide_ids['ff4'] for p in row.get('guides',[])),identity
            if row['disposition']=='guided mechanic':assert row['guides'],identity
            reviewed[identity]=row
        assert reviewed.keys()==expected.keys(),('Unreviewed FFIV item callers',expected.keys()-reviewed.keys())
        if args.check_sources:
            actual={}
            for line in (ROOT.parent/'tools/mapdump/ff4_scripts.jsonl').open(encoding='utf-8'):
                native=json.loads(line);identity=native['submap'],native['name']
                if identity in expected:actual[identity]=hashlib.sha256(json.dumps(native['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
            assert all(actual.get(k)==r['sha256'] for k,r in reviewed.items()),'FFIV item source changed'
        print('FFIV item semantic review:',len(reviewed),'exact callers; no missing declarations.')
    if args.check_sources:
        for game,paths in sources.items():
            for name,expected in paths.items():
                # Source hashes are labelled by discovery input role, never a
                # user-supplied executable or an arbitrary filesystem target.
                names={'objects':f'{game}_objects.jsonl','scripts':f'{game}_scripts.jsonl',
                    'maps':f'{game}_master/map.txt','script_master':f'{game}_master/script.txt',
                    'areas':f'{game}_master/area.txt','english_names':f'{game}_master/system_en.txt'}
                source=ROOT.parent/'tools/mapdump'/names[name];digest=hashlib.sha256()
                with source.open('rb') as f:
                    for part in iter(lambda:f.read(1024*1024),b''):digest.update(part)
                assert digest.hexdigest()==expected,(game,name,'primary source changed')
    print('Semantic review:',len(decisions),'explicit identity dispositions/',sum(map(len,candidates.values())),'physical candidates in',len(sources),'games; no missing entries in selected scope.')

if __name__=='__main__':main()
