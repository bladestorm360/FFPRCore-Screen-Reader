"""Promote completely reviewed physical transfers to ordinary map destinations.

This is offline bake metadata, never runtime script parsing. Whole source and
continuation hashes stay pinned to the existing functional reviews. Narrative
scripts are not identified by the mere presence of ChangeMap.
"""
import hashlib
import json
from event_label_identities import WORLD_EVENT_REVIEWS
from review_travel_bookkeeping import ALLOWED

_reviews={(r['game'],r['submap'],r['group'],r['id'],r.get('object_type',2)):r
          for r in WORLD_EVENT_REVIEWS if (r['label'] in
          {'Map entrance','World map transition','Passage'} or 'primary_transfer'in r) and 'continuations' in r}

def entry_flags(game,obj):
    # Called only after destination has validated this exact review's complete
    # source graph. These six native underwater entries dismount inside their
    # script; requiring a ground landing first would make them unreachable.
    row=_reviews.get((int(game.removeprefix('ff')),obj['submap'],obj['group'],obj['id'],obj['type']))
    return 1<<18 if row and row.get('vehicle_entry')=='submarine'else 0

def conditional_destinations(game,obj,script):
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

def destination(game,obj,script_name,scripts,parents):
    """Return one proven native destination/point/continuation, or no promotion.

    A multi-destination or multi-arrival script keeps its previous classification
    until its native branch/arrival behavior has been resolved. Exact native
    actor identity is required even when a developer name happens to match.
    """
    n=int(game.removeprefix('ff'))
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
