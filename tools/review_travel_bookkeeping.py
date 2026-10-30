"""Pin the complete travel/flag-maintenance families from the current audit.

This is explicit offline authoring, not runtime inference. Only complete source
graphs with no dialogue, battle, actor choreography or opaque native calls pass.
"""
from pathlib import Path
import csv,hashlib,json,struct

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump';OUT=ROOT.parent/'tools/map_event_audit_20261028'
ALLOWED={'Nop','Branch','BranchByPlayerDirection','Jump','Exit','Return','Wait','FadeOut','FadeIn',
         'StopBGM','PlayBGM','StopEnvSe','PauseEnvSe','PlayEnvSe','ChangeMap','ChangeScript','SetVehicle',
         'AddTelepoCache','RemoveTelepoCache','Calc','SetFlag','ResetFlag','AutoSave','ShowMapTitle','SetEntities','ColorFade'}
MOVEMENT=ALLOWED|{'Call','SetPuppet','ExecPuppet','FinishPuppet','SetPos','Move','MoveTo','Lerp',
                  'LookAt','CueAnim','PlayAnim','Blank','SyncSE','PlaySE','StopSE','SetDirection','Show','Hide'}
# Complete manually reviewed graphs. Native -1001 is first party puppet;
# 0 is the field player. No unrelated NPC/actor choreography is allowed here.
PHYSICAL={5:{
    'sc_map_20030_warterfall_2':'Water current','sc_map_20080_warterfall_1':'Water current',
    'sc_20041_15_to_20040':'Passage','sc_e_0233':'Passage','sc_20191_6_to_30161_2':'Cave entrance',
    **{f'sc_30060_jump_{i}':'Jump point' for i in (2,3,4,5,14,15,16,17)},
    'sc_30151_21_carry':'Conveyor belt','sc_e_0257':'Passage','sc_e_0436':'Passage',
    'sc_e_0523':'Slippery floor','sc_e_0523_1':'Slippery floor',
    **{f'sc_e_0316_{i}':'Water current' for i in range(1,6)}},
    6:{'sc_e_1075':'Passage'}}

def verify_movement(n,family,bodies):
    assert family['script'] in PHYSICAL.get(n,{})
    ops={m['mnemonic'] for s in bodies for m in s['mnemonics']}
    actors={m['operands']['iValues'][1] for s in bodies for m in s['mnemonics'] if m['mnemonic']=='SetPuppet'}
    assert ops<=MOVEMENT and -1001 in actors and actors<={-1001,0},(n,family['script'],ops-MOVEMENT,actors)

def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def build():
    path=ROOT/'tools/world_event_reviews.json';reviews=json.loads(path.read_text(encoding='utf8'));before=len(reviews)
    known={(r['game'],r['submap'],r['group'],r['id']) for r in reviews}
    for n in range(1,7):
        source=[json.loads(line) for line in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8-sig')]
        scripts={(s['submap'],s['name']):s for s in source}
        objects={(o['submap'],o['group'],o['id']):o for o in (json.loads(line) for line in (DUMP/f'ff{n}_objects.jsonl').open(encoding='utf-8-sig'))}
        with (DUMP/f'ff{n}_master/map.txt').open(encoding='utf-8-sig') as file:maps={int(r['id']):r for r in csv.DictReader(file) if r['id']}
        blob=(ROOT/f'data/ff{n}/maps.bin').read_bytes();header=struct.unpack_from('<8sIB3s12I',blob)
        strings=[]
        for i in range(header[5]):
            at=struct.unpack_from('<I',blob,header[4]+i*4)[0];strings.append(blob[at:blob.index(0,at)].decode())
        worlds=set()
        for i in range(header[7]):
            room=struct.unpack_from('<IihhBBBB10I',blob,header[6]+i*56)
            if room[4]:worlds.add(strings[room[0]])
        for family in json.loads((OUT/f'ff{n}_context_triage.json').read_text(encoding='utf8'))['families']:
            ops=set(family['operations'])
            physical=PHYSICAL.get(n,{}).get(family['script'])
            if family['gaps'] or (not physical and ('ChangeMap' not in ops or not ops<=ALLOWED)):continue
            if physical:verify_movement(n,family,[scripts[b['submap'],b['script']] for b in family['bodies']])
            root=next(scripts[b['submap'],b['script']] for b in family['bodies'] if sha(scripts[b['submap'],b['script']]['mnemonics'])==family['body_sha256'])
            destinations=[]
            for body in family['bodies']:
                for mnemonic in scripts[body['submap'],body['script']]['mnemonics']:
                    if mnemonic['mnemonic']=='ChangeMap':destinations.append(maps[mnemonic['operands']['iValues'][0]])
            assert destinations or physical
            for occurrence in family['occurrences']:
                key=(n,occurrence['submap'],occurrence['group'],occurrence['editor_id'])
                if key in known:continue
                obj=objects[key[1:]];p=obj.get('props')or{}
                assert obj['type'] in (2,5,10,28) and p.get('action_id')==2,key
                label=physical or ('World map transition' if all(d['map_name'] in worlds for d in destinations) else 'Map entrance' if obj['submap'] in worlds else 'Passage')
                continuations=[];seen={(root['submap'],root['name'])}
                for body in family['bodies']:
                    identity=(body['submap'],body['script'])
                    if identity in seen:continue
                    seen.add(identity);continuations.append(body)
                reviews.append(dict(game=n,submap=obj['submap'],group=obj['group'],id=obj['id'],object_type=obj['type'],
                    action_id=p['action_id'],entity_id=p['entity_id'],raw=obj['name'],script_id=p['script_id'],
                    script_name=root['name'],script_sha256=sha(root),label=label,
                    reason='reviewed player-only physical movement graph' if physical else 'complete travel graph with flag/phase maintenance; no narrative actor, dialogue, battle or opaque call',
                    review_grammar='player_movement' if physical else 'travel_bookkeeping',continuations=continuations))
                known.add(key)
    path.write_text(json.dumps(reviews,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(f'Added {len(reviews)-before} exact travel/maintenance occurrences')

if __name__=='__main__':build()
