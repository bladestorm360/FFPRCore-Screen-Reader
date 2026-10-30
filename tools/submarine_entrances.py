"""Pinned FFIII underwater transfers: factual identities/hashes, no game source.

All three complete five-operation scripts disembark inside the entry, then
change map. Their complete arrival scripts contain travel/UI/bookkeeping only.
The player enters aboard transportation14; ordinary surface doors are unchanged.
"""
import hashlib,json

REVIEWS={
    8:(4,370,242,'sc_e_0351','bc2f147a84712b5dd516e879c74148c97d4ecca8bc1f76beb93032b883b81598','23b4f442220c5f54e70e7b9767932bea8dbbe6c0da9c086500982ca18181d760'),
    9:(5,370,242,'sc_e_0351','bc2f147a84712b5dd516e879c74148c97d4ecca8bc1f76beb93032b883b81598','23b4f442220c5f54e70e7b9767932bea8dbbe6c0da9c086500982ca18181d760'),
    10:(6,362,234,'sc_e_0347','105b44ac5e06af24c6f68b338a9ff0cde25b71d0d64388a863707543142de12e','9f4ce8860f0585f98868ec35b6eb221a0c3c0aec7eeadcc8ed2e51643769a28c'),
    11:(7,366,269,'sc_e_0349','03d5a64c92490132599fe18a1a844d269c51b6e4645d5eab3650d5de417798fc','3218e89b4d7e8ec1ccac6851ad1062b3365bd2722e3c72f342e2b15076bd236c'),
}
def digest(script):
    return hashlib.sha256(json.dumps(script,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def reviewed(game,obj,script,scripts,submaps,dest):
    if game!='ff3' or obj['submap']!='Map_10030' or obj['id'] not in REVIEWS:return False
    entity,sid,target,name,body,arrival=REVIEWS[obj['id']]
    prop=obj.get('props')or{}
    assert obj['type_name']=='Event' and prop['entity_id']==entity and prop['script_id']==sid and prop['action_id']==2
    assert script['name']==name and digest(script)==body
    assert dest==(target,1,name+'_1')
    assert digest(scripts[submaps[target],dest[2]])==arrival
    assert len(script['mnemonics'])==5
    return True

if __name__=='__main__':
    from pathlib import Path
    from check_scripted_bakes import decode
    root=Path(__file__).resolve().parents[1];dump=root.parent/'tools/mapdump'
    scripts={(s['submap'],s['name']):s for s in (json.loads(l) for l in (dump/'ff3_scripts.jsonl').open(encoding='utf-8'))}
    import csv
    submaps={int(r['id']):r['map_name'] for r in csv.DictReader((dump/'ff3_master/map.txt').open(encoding='utf-8-sig'))}
    rooms,_=decode(root/'data/ff3/maps.bin');count=0
    for obj in (json.loads(l) for l in (dump/'ff3_objects.jsonl').open(encoding='utf-8')):
        if obj['submap']!='Map_10030' or obj['id'] not in REVIEWS:continue
        entity,sid,target,name,body,arrival=REVIEWS[obj['id']]
        assert reviewed('ff3',obj,scripts['Map_10030',name],scripts,submaps,(target,1,name+'_1'))
        arcs=[o for o in rooms['Map_10030']['objects'] if o[0]==100 and o[8]==entity]
        assert len(arcs)==1 and arcs[0][6:8]==(target,1) and arcs[0][9]&((1<<16)|(1<<18))==((1<<16)|(1<<18))
        count+=1
    assert count==4
    print('FFIII underwater:4 exact native entries, complete root/arrival source hashes and mounted destination flags verified')
