"""Promote completely reviewed physical transfers to ordinary map destinations.

This is offline bake metadata, never runtime script parsing. Whole source and
continuation hashes stay pinned to the existing functional reviews. Narrative
scripts are not identified by the mere presence of ChangeMap.
"""
import hashlib
import json
from event_label_identities import WORLD_EVENT_REVIEWS
from review_travel_bookkeeping import ALLOWED
import phantom_train_exits
import south_figaro_passages
import serpent_passages
import vector_passages

_reviews={(r['game'],r['submap'],r['group'],r['id'],r.get('object_type',2)):r
          for r in WORLD_EVENT_REVIEWS if (r['label'] in
          {'Map entrance','World map transition','Passage'} or 'primary_transfer'in r) and 'continuations' in r}

# These vents alone use floor 428 as transit during their animation. The
# ordinary stairs on that floor retain their own playable routes.
ISLAND_VENT_DESTINATIONS = {
    ('Map_30471_3',6):(430,5,'sc_e_0304_2'),
    ('Map_30471_4',16):(429,2,'sc_e_0497_2'),
    ('Map_30471_4',17):(431,1,'sc_e_0306_2'),
    ('Map_30471_4',18):(432,1,'sc_e_0307_2'),
    ('Map_30471_4',19):(433,1,'sc_e_0308_2'),
    ('Map_30471_5',5):(430,5,'sc_e_0304_2'),
    ('Map_30471_6',7):(430,5,'sc_e_0304_2'),
    ('Map_30471_7',7):(430,5,'sc_e_0304_2'),
}

def entry_flags(game,obj):
    if game == 'ff5' and (obj['submap'], obj['id']) in _METEORITE_TRANSFERS:
        return 1 << 29
    if game=='ff6':
        import blackjack_passage
        if flags:=blackjack_passage.entry_flags(obj):return flags
    if game=='ff6'and(flags:=serpent_passages.entry_flags(obj)):return flags
    if game=='ff6'and(flags:=phantom_train_exits.entry_flags(obj)):return flags
    # Called only after destination has validated this exact review's complete
    # source graph. These six native underwater entries dismount inside their
    # script; requiring a ground landing first would make them unreachable.
    row=_reviews.get((int(game.removeprefix('ff')),obj['submap'],obj['group'],obj['id'],obj['type']))
    return 1<<18 if row and row.get('vehicle_entry')=='submarine'else 0

def conditional_destinations(game,obj,script):
    if game=='ff6'and(variants:=vector_passages.variants(obj,script)):return variants
    row=_reviews.get((int(game.removeprefix('ff')),obj['submap'],obj['group'],obj['id'],obj['type']))
    if not row or row.get('review_grammar')!='contextual_conditional':return []
    assert script and _sha(script)==row['script_sha256'],'Conditional source body changed'
    prop=obj['props']
    assert (prop['action_id'],prop['entity_id'],prop['script_id'],obj['name'])==(row['action_id'],row['entity_id'],row['script_id'],row['raw'])
    main=next(s for s in script['segments']if s['Label']=='Main')
    actual=[[*m['operands']['iValues'][:2],m['operands']['sValues'][0]]for m in
            script['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]if m['mnemonic']=='ChangeMap']
    assert actual==row['conditional_transfers'] and len(actual)==2,'Conditional destination structure changed'
    return [tuple(v)for v in actual]

def _sha(value):
    return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,
                                    separators=(',',':')).encode()).hexdigest()

def karnak_pub_destination(obj, script_name, scripts):
    variants = {44: 'entity_default', 31: 'ev_e_0052'}
    if obj['submap'] != 'Map_20061_5' or obj['id'] not in variants:
        return None
    prop = obj['props']
    assert (obj['type_name'], obj['group'], prop['entity_id'], prop['script_id'], prop['action_id'], script_name) == (
        'Event', variants[obj['id']], 33, 146, 2, 'sc_e_0052')
    source = scripts[('Map_20061_5', script_name)]
    assert _sha(source) == '0eafe4c2c5b2bca822b43f4adc17b6bc72fc7d1aa5cae7b5fcbd05ad4f8c0439'
    assert _sha(scripts[('Map_20061_6', 'sc_e_0052_1')]) == '500b9ec3981ef76e2a7649c08d83c374d9ca1e399b643641d713b3d6c74ef7a0'
    assert _sha(scripts[('Map_20061_6', 'sc_e_0190')]) == 'd8ac50944011d66b9bb67c4a113749519e8894ade738ef67a205c5b09de9bb2c'
    # The two story scenes play after the same native 2F arrival as plain entry.
    transfers = [(row['operands']['iValues'][:2], row['operands']['sValues'][0])
                 for row in source['mnemonics'] if row['mnemonic'] == 'ChangeMap']
    assert transfers == [([93, 1], ''), ([93, 1], 'sc_e_0052_1'), ([93, 1], 'sc_e_0190')]
    return 93, 1, ''

# The meteor flight is staged on the world map. The other interior is the
# first playable landing. Use the PointIn at the final player pose, even
# where native ChangeMap first places the cutscene at the door.
_METEORITE_TRANSFERS = {
    ('Map_30131_2', 10): (
        'entity_default', 2, 118, 'sc_e_0041',
        'a03821859cae256c20771e4a2c1942543719bc4dc35263eb5f527160235ec18a',
        'sc_e_0041_1', 'b258e1eeb1b5854dd278eb8e926f77e493624861b807554d6660b1dc69cbf5b5',
        247, 1, 'Map_30141_2', 'sc_e_0041_2',
        '74927a58d9cacdb5c8d67b9952c2a2274f1300c9623807622ab5804c3b723c6c', (14, 14), (16, 25)),
    ('Map_30141_2', 16): (
        'ev_e_0001', 8, 356, 'sc_e_0041_3',
        '9ba49db86a708f47ed2897fdb10bb88a1032a7a9a40603380976170ad17a11bb',
        'sc_e_0041_4', 'a35b477bb9ffee055cf6290f6525c1d042066119bb8d14913b05e37178a36d13',
        246, 2, 'Map_30131_2', 'sc_e_0041_5',
        'aae77742ecb62507d3526c8ae86561ba9a218a185db64761244854ef2bdd0a3f', (15, 14), (15, 24)),
}

def meteorite_destination(obj, script_name, scripts):
    row = _METEORITE_TRANSFERS.get((obj['submap'], obj['id']))
    if row is None:
        return None
    group, entity, script_id, departure, root_hash, flight, flight_hash, map_id, point, room, arrival, arrival_hash, pose, placements = row
    prop = obj['props']
    assert (obj['type_name'], obj['group'], prop['entity_id'], prop['script_id'], prop['action_id'], script_name) == (
        'Event', group, entity, script_id, 2, departure)
    root = scripts[(obj['submap'], departure)]
    transit = scripts[('Map_10010', flight)]
    landing = scripts[(room, arrival)]
    assert (_sha(root), _sha(transit), _sha(landing)) == (root_hash, flight_hash, arrival_hash)
    changes = lambda body: {(*op['operands']['iValues'][:2], op['operands']['sValues'][0])
                            for op in body['mnemonics'] if op['mnemonic'] == 'ChangeMap'}
    assert changes(root) == {(1, 1, flight)}
    assert changes(transit) == {(map_id, 1, arrival)} and not changes(landing)
    assert all(landing['mnemonics'][i]['mnemonic'] == 'SetPos' and
               tuple(landing['mnemonics'][i]['operands']['iValues'][:2]) == pose for i in placements)
    return map_id, point, arrival

# Tower roller/duct travel uses movie-only map545 between two playable rooms.
# Pin all three complete bodies. Returning the first transfer strands routing
# in a scene with no player-controlled exit; the final continuation/landing is
# authored separately in warp_arrival_overrides. No generic cutscene collapse.
_TOWER_PASSAGES=(
 ('Map_30871_4',56,12,998,'sc_e_0970','ローラーで別マップへ',
  (('Map_30871_4','sc_e_0970','b2f33fdbc2899c16d68fc208c6e624b6ce28837c59fe57a7ab3fc8fe486fdb89'),
   ('Map_30871_3','sc_e_0970_1','ca819692050687e415f5795a81f9be7a95398fdf1a6dcc8fd8f9012eaf9d4e76'),
   ('Map_30871_2','sc_e_0970_2','0acc00388ed50e56a4fa2e343e9f2b216e400d94bb828b93ce483c7c838fd514')),
  (545,1,'sc_e_0970_1'),(544,3,'sc_e_0970_2')),
 ('Map_30871_2',23,13,999,'sc_e_0971','ダクトで別マップへ',
  (('Map_30871_2','sc_e_0971','e943a916c8681f497bfc04fe2e2a32ec570d27c7072beddff6f7719654c4028b'),
   ('Map_30871_3','sc_e_0971_1','2548f8a485e2642b20374c5bc93c45e21e9f62111e65902b827f5b0d1affe92d'),
   ('Map_30871_4','sc_e_0971_2','5463b6220a3d1657dd98151173057d1708038ed5c0144d7d3631e446519af826')),
  (545,2,'sc_e_0971_1'),(546,2,'sc_e_0971_2')),
)

def tower_destination(obj,script_name,scripts):
    for submap,object_id,entity,script_id,name,raw,chain,first,final in _TOWER_PASSAGES:
        if (obj['submap'],obj['id'])!=(submap,object_id):continue
        prop=obj['props']
        assert (obj['type_name'],obj['group'],prop['entity_id'],prop['script_id'],prop['action_id'],obj['name'],script_name)==\
            ('Entity','entity_default',entity,script_id,2,raw,name),'Tower passage identity changed'
        bodies=[]
        for room,label,digest in chain:
            body=scripts.get((room,label));assert body and _sha(body)==digest,'Tower passage body changed'
            bodies.append(body)
        def transfers(body):
            main=next(s for s in body['segments'] if s['Label']=='Main')
            return [tuple([*m['operands']['iValues'][:2],m['operands']['sValues'][0]])
                    for m in body['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]
                    if m['mnemonic']=='ChangeMap']
        assert transfers(bodies[0])==[first] and transfers(bodies[1])==[final] and not transfers(bodies[2])
        return final
    return None

def destination(game,obj,script_name,scripts,parents):
    """Return one proven native destination/point/continuation, or no promotion.

    A multi-destination or multi-arrival script keeps its previous classification
    until its native branch/arrival behavior has been resolved. Exact native
    actor identity is required even when a developer name happens to match.
    """
    n=int(game.removeprefix('ff'))
    if n == 5 and (meteorite := meteorite_destination(obj, script_name, scripts)):
        return meteorite
    if n == 5 and (pub := karnak_pub_destination(obj, script_name, scripts)):
        return pub
    if n==6:
        import solitary_passage
        if passage:=solitary_passage.destination(obj,script_name,scripts):return passage
    if n==6:
        import cyan_passage
        if passage:=cyan_passage.destination(obj,script_name,scripts):return passage
    if n==6:
        import veldt_cave_passage
        if passage:=veldt_cave_passage.destination(obj,script_name,scripts):return passage
    if n==6:
        import late_opera_passage
        if passage:=late_opera_passage.destination(obj,script_name,scripts):return passage
    if n==6:
        import blackjack_passage
        if passage:=blackjack_passage.destination(obj,script_name,scripts):return passage
    if n==6 and(passage:=vector_passages.destination(obj,script_name,scripts)):return passage
    if n==6 and(passage:=serpent_passages.destination(obj,script_name,scripts)):return passage
    if n==6 and(train:=phantom_train_exits.destination(obj,script_name,scripts)):return train
    if n==6 and(passage:=south_figaro_passages.destination(obj,script_name,scripts)):return passage
    if n==6 and (tower:=tower_destination(obj,script_name,scripts)):return tower
    row=_reviews.get((n,obj['submap'],obj['group'],obj['id'],obj['type']))
    if not row or obj['type_name'] not in ('Event','RandomEvent'):return None
    if row.get('review_grammar')not in ('travel_bookkeeping','contextual_transfer','contextual_conditional') and not row['reason'].startswith('complete travel-only'):return None
    prop=obj.get('props')or{}
    assert (row['raw'],row['entity_id'],row['script_id'],row.get('action_id',2),row['script_name'])==(
        obj['name'],prop.get('entity_id'),prop.get('script_id'),prop.get('action_id'),script_name), 'Reviewed transfer identity changed'
    root=scripts.get((obj['submap'],script_name))
    if root is None:
        candidates=[s for s in parents.get((obj['map'],script_name),[]) if _sha(s)==row['script_sha256']]
        if not candidates:
            candidates=[s for (sub,name),s in scripts.items() if name==script_name and _sha(s)==row['script_sha256']]
        assert candidates,'Reviewed transfer root is missing or changed'
        root=candidates[0]
    assert _sha(root)==row['script_sha256'],'Reviewed transfer root changed'
    bodies=[root]
    for child in row['continuations']:
        body=scripts.get((child['submap'],child['script']))
        assert body and _sha(body['mnemonics'])==child['body_sha256'],'Reviewed transfer continuation changed'
        bodies.append(body)
    if row.get('review_grammar')=='contextual_conditional':
        # Both native branches remain manual event transfers. Baking a second
        # real arc and destination-specific availability does not invent an
        # unconditional entrance or guess its current destination at capture.
        conditional_destinations(game,obj,root)
        return None
    if row.get('review_grammar')=='contextual_transfer':
        # Exact contextual review, not a generalized no-dialogue heuristic.
        # Preserve the existing first Main transfer and arrival script. Room
        # initializers can have other exits in unused segments; those are not
        # additional destinations of the selected entrance. Differing native
        # Main destinations require a separate branch/arrival proof.
        main=next(s for s in root['segments']if s['Label']=='Main')
        changes=[m['operands']for m in root['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]if m['mnemonic']=='ChangeMap']
        assert changes and len({tuple(v['iValues'][:2])for v in changes})==1,'Conditional transfer needs exact native branch evidence'
        first=changes[0];actual=[*first['iValues'][:2],first['sValues'][0]]
        assert actual==row['primary_transfer'],'Reviewed primary transfer changed'
        if n == 5 and (locator := (obj['submap'],obj['id'])) in ISLAND_VENT_DESTINATIONS:
            final = ISLAND_VENT_DESTINATIONS[locator]
            assert actual[0] == 428 and bodies[1]['submap'] == 'Map_30471_2' and bodies[1]['name'] == actual[2]
            assert bodies[2]['name'] == final[2]
            moves = [(*m['operands']['iValues'][:2],m['operands']['sValues'][0])
                     for m in bodies[1]['mnemonics'] if m['mnemonic'] == 'ChangeMap']
            assert moves == [final] and not any(m['mnemonic'] == 'ChangeMap' for m in bodies[2]['mnemonics'])
            return final
        if final:=row.get('final_transfer'):
            # One exact reviewed two-stage teleport. The intermediate world is
            # not player-controlled; arrival override owns the final SetPos.
            assert n==4 and row['script_name']=='sc_e_0031'and final==[29,1,'sc_e_0031_2']
            assert any(m['mnemonic']=='ChangeMap'and [*m['operands']['iValues'][:2],m['operands']['sValues'][0]]==final
                       for s in bodies[1:]for m in s['mnemonics'])
            return tuple(final)
        return tuple(actual)
    commands={m['mnemonic'] for s in bodies for m in s['mnemonics']}
    assert commands<=ALLOWED,'Reviewed physical transfer now contains unreviewed behavior'
    targets=set()
    for source in bodies:
        for m in source['mnemonics']:
            if m['mnemonic']!='ChangeMap':continue
            operands=m.get('operands')or{}
            ints=operands.get('iValues')or[];strings=operands.get('sValues')or[]
            if len(ints)<2 or ints[0]<=0:return None
            targets.add((int(ints[0]),int(ints[1]),strings[0] if strings else ''))
    return next(iter(targets)) if len(targets)==1 else None
