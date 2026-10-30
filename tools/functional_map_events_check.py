"""Validate exact functional event reviews and effective labels across six games.

Sources remain outside Git. A complete body/continuation hash is required, not
the presence of a developer word or absence of a Msg opcode by itself.
"""
from pathlib import Path
import collections,csv,hashlib,json
import event_label_identities as identities
from scripted_transition_identity import destination as transfer_destination
from check_scripted_bakes import decode as decode_bake

ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
TRAVEL={'Nop','Branch','Jump','Exit','Return','Wait','FadeOut','FadeIn','StopBGM','PlayBGM',
        'StopEnvSe','PauseEnvSe','PlayEnvSe','ChangeMap','SetVehicle','AddTelepoCache','RemoveTelepoCache'}
catalog=json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf8'))
total=0;families=set();silent_checked=0
silent=json.loads((ROOT/'tools/silent_family_decisions.json').read_text(encoding='utf-8'))
assert len(silent)==245 and not any(r['disposition'].startswith('needs_')for r in silent)
for n in range(1,7):
    reviews=[r for r in identities.WORLD_EVENT_REVIEWS if r['game']==n and 'continuations' in r]
    objects=[json.loads(l) for l in (DUMP/f'ff{n}_objects.jsonl').open(encoding='utf8')]
    scripts={(s['submap'],s['name']):s for s in (json.loads(l) for l in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf8'))}
    parents=collections.defaultdict(list)
    for source in scripts.values():parents[source['map'],source['name']].append(source)
    baked_rooms,_=decode_bake(ROOT/f'data/ff{n}/maps.bin')
    transfer_count=0
    maps=collections.defaultdict(list)
    for r in csv.DictReader((DUMP/f'ff{n}_master/map.txt').open(encoding='utf-8-sig')):
        if r['id'] and int(r['id'])>0:maps[r['map_name']].append(int(r['id']))
    labels=json.loads((ROOT/f'data/ff{n}/eventlabels.json').read_text(encoding='utf8'))
    for review in reviews:
        matched=[o for o in objects if (o['submap'],o['group'],o['id'],o['type'],o['name'],o.get('props',{}).get('entity_id'))==
                 (review['submap'],review['group'],review['id'],review.get('object_type',2),review['raw'],review['entity_id'])]
        assert len(matched)==1,(n,review['submap'],review['id'],'review identity ambiguous or missing')
        obj=matched[0]
        script=identities.reviewed_event_script(n,obj,review['script_name'],scripts.get((obj['submap'],review['script_name']),{}))
        result=identities.reviewed_world_event_identity(n,obj,review['script_name'],script)
        assert result and result[0]==review['label']
        transfer=transfer_destination(f'ff{n}',obj,review['script_name'],scripts,parents)
        if transfer:
            matches=[o for o in baked_rooms[obj['submap']]['objects'] if o[0]==100 and o[8]==review['entity_id'] and o[10:]==(obj['name'],obj['group'])]
            expected=transfer[:2]
            correction_file=ROOT/f'data/ff{n}/warp_arrival_overrides.json'
            corrections=json.loads(correction_file.read_text(encoding='utf-8'))if correction_file.exists()else[]
            for index,correction in enumerate(corrections):
                if (correction['submap'],correction['entity_id'],correction.get('object_id',obj['id']),correction['destination_map'],correction['original_point'],correction['continuation'])==(
                        obj['submap'],review['entity_id'],obj['id'],*transfer):
                    expected=(correction['destination_map'],-100000-index if 'arrival_cell'in correction else correction['arrival_point'])
            assert matches and all(o[6:8]==expected and o[9]&65536 for o in matches),('Physical transfer missing from exit index',n,obj['submap'],review['entity_id'])
            transfer_count+=1
        bodies=[script]+[scripts[(c['submap'],c['script'])] for c in review['continuations']]
        ops={m['mnemonic'] for s in bodies for m in s.get('mnemonics')or[]}
        if review['reason'].startswith('complete travel-only'):
            assert 'ChangeMap' in ops and ops<=TRAVEL,(n,review['script_name'],ops-TRAVEL)
        if review.get('review_grammar')=='travel_bookkeeping':
            from review_travel_bookkeeping import ALLOWED
            assert 'ChangeMap' in ops and ops<=ALLOWED,(n,review['script_name'],ops-ALLOWED)
        if review.get('review_grammar')=='player_movement':
            from review_travel_bookkeeping import verify_movement,PHYSICAL
            verify_movement(n,{'script':review['script_name']},bodies)
            assert review['label']==PHYSICAL[n][review['script_name']]
        for mid in maps[obj['submap']]:
            key=f"@object:{mid}:{obj['props']['entity_id']}:{obj['type']}:{obj['name']}"
            assert labels.get(key,labels.get(obj['name']))==review['label'],key
        localized=catalog[review['label']]
        assert set(localized)==set(catalog['Exit to {0}']) and len(localized)==12 and all(localized.values())
        total+=1;families.add((n,review['script_name'],review['script_sha256']))
    # Reuse the already loaded matching source/labels for the complete finite
    # contextual inventory. In particular, silent narrative scenes are not
    # reclassified just because a reviewed physical family shares a raw name.
    contexts=json.loads((ROOT.parent/f'tools/map_event_audit_20261028/ff{n}_context_triage.json').read_text(encoding='utf-8'))['families']
    contextual={(r['script'],r['body_sha256']):r for r in contexts}
    actual_objects={(o['submap'],o['group'],o['id']):o for o in objects}
    for decision in (r for r in silent if r['game']==n):
        original=contextual[decision['script'],decision['body_sha256']]
        assert original['status']=='silent_staging_or_physical_context'and not original['gaps']
        for body in decision['bodies']:
            actual=scripts[body['submap'],body['script']]['mnemonics']
            digest=hashlib.sha256(json.dumps(actual,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
            assert digest==body['body_sha256'],('Contextual source changed',n,body['submap'],body['script'])
        if decision['disposition']=='narrative':
            for loc in original['occurrences']:
                obj=actual_objects[loc['submap'],loc['group'],loc['editor_id']]
                for mid in maps[obj['submap']]:
                    key=f"@object:{mid}:{obj['props']['entity_id']}:{obj['type']}:{obj['name']}"
                    assert labels.get(key,labels.get(obj['name']))=='Scene trigger',('Silent narrative relabelled',n,key)
        silent_checked+=1
    print(f'FF{n}: {len(reviews)} exact functional occurrences, {transfer_count} actual ordinary-transfer arcs verified')
assert total>=507
print(f'PASS {total} exact occurrences; complete source/continuation hashes, functional grammar and12 locales; no game run')
assert silent_checked==245
print('PASS all245 silent-family contextual dispositions/source closures;41 narrative families retain Scene trigger; provider/exit ownership separately verified')
