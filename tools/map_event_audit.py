"""All-six read-only bake/event survey. No game execution or shipped-data writes.

Rebuilds navigation data outside Git, compares exact shipped bytes/IR, and emits
factual triage metadata. A mnemonic inventory is not a final scene classification;
candidate scripts must still be read in full before any label/gate correction.
"""
from pathlib import Path
import collections
import csv
import hashlib
import json
import struct
import subprocess
import bake_maps as bake
import bake_map_state as state

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
OUT=ROOT.parent/'tools/map_event_audit_20261028'
OUT.mkdir(exist_ok=True)
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
evidence=ROOT/'build_check/map_event_audit_20261028';evidence.mkdir(exist_ok=True)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def csvrows(path):return list(csv.DictReader(path.open(encoding='utf-8-sig')))
def decode(path):
    data=path.read_bytes();h=struct.unpack_from('<8sIB3s12I',data)
    strings=[]
    for i in range(h[5]):
        at=struct.unpack_from('<I',data,h[4]+i*4)[0];strings.append(data[at:data.index(0,at)].decode())
    rooms={}
    for i in range(h[7]):
        r=struct.unpack_from('<IihhBBBB10I',data,h[6]+i*56)
        objs=[]
        for j in range(r[-1]):
            o=struct.unpack_from('<BBhhHH4i2I',data,r[-2]+j*34)
            objs.append(dict(type=o[0],layer=o[1],x=o[2],y=o[3],w=o[4],h=o[5],a=o[6],b=o[7],id=o[8],d=o[9],raw=strings[o[10]],group=strings[o[11]]))
        rooms[strings[r[0]]]=dict(width=r[2],height=r[3],world=bool(r[4]),layers=r[5],objects=objs)
    return rooms

prior=json.loads((ROOT/'build_check/map_audit_2026_10_14/report.json').read_text())
reports=[]
for n in range(1,7):
    game=f'ff{n}';generated=OUT/'generated';(generated/'data'/game).mkdir(parents=True,exist_ok=True)
    bake.bake_game(game,str(generated/'data'),check=True)
    state.ROOT=generated;state.build(game)
    shipped=ROOT/'data'/game;rebuilt=generated/'data'/game
    assert (rebuilt/'maps.bin').read_bytes()==(shipped/'maps.bin').read_bytes(),game+' bake differs'
    assert json.loads((rebuilt/'mapstate.json').read_text())==json.loads((shipped/'mapstate.json').read_text()),game+' map state differs'
    rooms=decode(shipped/'maps.bin')
    raw=[json.loads(l) for l in (DUMP/f'{game}_objects.jsonl').open(encoding='utf-8-sig')]
    scripts=[json.loads(l) for l in (DUMP/f'{game}_scripts.jsonl').open(encoding='utf-8-sig')]
    by_script={(s['submap'],s['name']):s for s in scripts}
    by_parent=collections.defaultdict(list);by_name=collections.defaultdict(list)
    for s in scripts:
        by_parent[(s['map'],s['name'])].append(s);by_name[s['name']].append(s)
    masters={int(r['id']):r['script_name'] for r in csvrows(DUMP/f'{game}_master/script.txt') if r['id']}
    maps={int(r['id']):r['map_name'] for r in csvrows(DUMP/f'{game}_master/map.txt') if r['id'] and int(r['id'])>0}
    mapped=collections.defaultdict(list)
    for mid,sm in maps.items():mapped[sm].append(mid)
    labels=json.loads((shipped/'eventlabels.json').read_text())
    families={};omitted=[];outside=[];relay=[];doors=[];gates=[];missing_scripts=[]
    for o in raw:
        if o['submap'] not in mapped:continue
        p=o.get('props') or {};room=rooms[o['submap']]
        matches=[b for b in room['objects'] if b['id']==p.get('entity_id',0) and b['group']==o['group'] and b['raw']==o['name']]
        if p.get('action_id') and o['type_name'] not in bake.BAKED_TYPES:
            omitted.append(dict(submap=o['submap'],type=o['type_name'],raw=o['name'],id=p.get('entity_id'),action=p['action_id']))
        if o['type_name']=='RelayInteraction':
            assert matches,'Dropped relay '+str((game,o['submap'],o['id']))
            for b in matches:relay.append(dict(submap=o['submap'],id=b['id'],group=b['group'],rect=[b['layer'],b['x'],b['y'],b['w'],b['h']],link=b['a']))
        if o['type_name']=='GotoMap':
            for b in matches:
                destination=maps.get(b['a']);arrivals=[q for q in rooms.get(destination,{}).get('objects',[]) if q['type']==0 and q['a']==b['b']]
                if len(arrivals)!=1:doors.append(dict(submap=o['submap'],id=b['id'],destination=b['a'],point=b['b'],arrivals=len(arrivals)))
        if o['type_name']=='CollisionEntity':
            for b in matches:
                overlap=[q for q in room['objects'] if q['type'] in (3,5,10,100) and q['layer']==b['layer'] and q['x']<b['x']+b['w'] and q['x']+q['w']>b['x'] and q['y']<b['y']+b['h'] and q['y']+q['h']>b['y']]
                if overlap:gates.append(dict(submap=o['submap'],id=b['id'],group=b['group'],collision=b['a'],through=b['b'],targets=[q['id'] for q in overlap]))
        if o['type'] in (5,6,7,10,17) and p.get('action_id'):
            for b in matches:
                if not (0<=b['x']<room['width'] and 0<=b['y']<room['height']):outside.append(dict(submap=o['submap'],id=b['id'],group=b['group'],raw=b['raw'],cell=[b['layer'],b['x'],b['y']],flags=b['d']))
        if o['type'] not in (2,4,5,10,20,21,28,30):continue
        effective=set()
        for mid in mapped[o['submap']]:
            key=f"@object:{mid}:{p.get('entity_id',-1)}:{o['type']}:{o['name']}"
            effective.add(labels.get(key,labels.get(o['name'],'Event trigger')))
        if 'Scene trigger' not in effective:continue
        name=masters.get(p.get('script_id'),'');s=by_script.get((o['submap'],name))
        if not s:
            candidates=by_parent[(o['map'],name)]
            if len(candidates)==1:s=candidates[0]
        if not s:
            candidates=by_name[name]
            bodies={json.dumps(c.get('mnemonics'),sort_keys=True,ensure_ascii=False,separators=(',',':')) for c in candidates}
            if len(bodies)==1:s=candidates[0] # same global asset body; record actual owner below
        if not s:
            missing_scripts.append(dict(submap=o['submap'],id=p.get('entity_id'),raw=o['name'],script=name));continue
        digest=hashlib.sha256(json.dumps(s.get('mnemonics'),sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
        key=name+':'+digest
        row=families.setdefault(key,dict(script=name,script_id=p.get('script_id'),body_sha256=digest,source_submap=s['submap'],occurrences=[],operations=dict(collections.Counter(m['mnemonic'] for m in s.get('mnemonics') or []))))
        row['occurrences'].append(dict(submap=o['submap'],group=o['group'],id=p.get('entity_id'),editor_id=o['id'],raw=o['name'],baked_types=sorted({b['type'] for b in matches})))
    previous=next(r for r in prior if r['game']==game)
    input_hashes={suffix:sha(DUMP/f'{game}_{suffix}') for suffix in ('submaps.json','grids.json','objects.jsonl','scripts.jsonl')}
    same={suffix:value==previous['source_comparison'][suffix]['retained_sha256'] for suffix,value in input_hashes.items()}
    report=dict(game=game,mapped_maps=len(maps),submaps=len(rooms),objects=sum(len(r['objects']) for r in rooms.values()),bake_sha256=sha(shipped/'maps.bin'),mapstate_sha256=sha(shipped/'mapstate.json'),reproduced=True,input_hashes=input_hashes,same_as_fresh_asset_audit=same,
                scene_families=list(families.values()),missing_scene_scripts=missing_scripts,omitted_action_types=omitted,outside_actors=outside,relays=relay,ambiguous_door_arrivals=doors,gate_target_overlaps=gates)
    reports.append(report)
    (OUT/f'{game}_facts.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(game,len(maps),'maps;',len(families),'scene families;',len(missing_scripts),'unresolved scripts;',len(relay),'relays;',len(doors),'arrival candidates;',len(omitted),'omitted action types',flush=True)
summary=[{k:v for k,v in r.items() if k not in ('scene_families','outside_actors','relays','ambiguous_door_arrivals','gate_target_overlaps','missing_scene_scripts','omitted_action_types')}|{k:len(r[k]) for k in ('scene_families','outside_actors','relays','ambiguous_door_arrivals','gate_target_overlaps','missing_scene_scripts','omitted_action_types')} for r in reports]
(evidence/'data_summary.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
print('PASS all-six navigation bakes and map-state IR reproduce exactly; detailed triage outside Git. No shipped data changed.')
