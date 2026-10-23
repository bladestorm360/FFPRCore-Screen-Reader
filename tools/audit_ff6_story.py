"""Source-free coverage inventory of the actual FF6 extracted map/script data.

Only counts, identities, hashes and navigation-relevant factual categories are
saved. Game script bodies, mnemonic streams and disassembly remain external.
This is a repeatable coverage audit, not a claim of complete gameplay testing.
"""
import collections
import csv
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
objects=list(map(json.loads,(DUMP/'ff6_objects.jsonl').read_text(encoding='utf-8').splitlines()))
scripts=list(map(json.loads,(DUMP/'ff6_scripts.jsonl').read_text(encoding='utf-8').splitlines()))
names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff6_master/script.txt').open(encoding='utf-8-sig'))}
maps=collections.defaultdict(list)
for r in csv.DictReader((DUMP/'ff6_master/map.txt').open(encoding='utf-8-sig')):maps[r['map_name']].append(int(r['id']))
source={(s['submap'],s['name']):s for s in scripts}
guides=json.loads((ROOT/'data/ff6/puzzles.json').read_text(encoding='utf-8'))['story_guides']
arrivals=json.loads((ROOT/'data/ff6/warp_arrival_overrides.json').read_text(encoding='utf-8'))
covered={(r['submap'],r.get('object_id')) for r in arrivals}
records=[];contacts=[];puzzle_controls=[]
for o in objects:
    sid=o['props'].get('script_id')
    if not sid:continue
    record=dict(maps=maps[o['submap']],submap=o['submap'],type=o['type_name'],
                entity=o['props'].get('entity_id',0),object=o['id'],group=o['group'],
                script_id=sid,script=names.get(sid,''))
    if o['props'].get('leave_script_id'):
        record['leave_script_id']=o['props']['leave_script_id']
        record['leave_script']=names.get(record['leave_script_id'],'')
    s=source.get((o['submap'],record['script']))
    if s:
        ops=collections.Counter(m['mnemonic'] for m in s['mnemonics'])
        record.update(native_encounters=sum(v for k,v in ops.items() if k.startswith('Encount')),
                      map_changes=ops['ChangeMap'],party_calls=sum(m['mnemonic']=='SysCall' and
                      any(x in (m['operands'].get('sValues') or [''])[0] for x in ('パーティ','RTS')) for m in s['mnemonics']),
                      player_puppets=sum(m['mnemonic']=='SetPuppet' and m['operands']['iValues'][1]==-1001 for m in s['mnemonics']),
                      state_writes=sum(ops[k] for k in ('SetFlag','ResetFlag','Calc')),
                      reviewed_travel=(o['submap'],o['id']) in covered)
    else:record['source_missing']=True
    records.append(record)
    if o['type_name']=='NPC' and o['props'].get('action_id')==9:contacts.append(record)
    if o['type_name']=='SwitchEvent' or any(x in o['name'] for x in ('レバー','スイッチ','スウィッチ')):puzzle_controls.append(record)

report=dict(format=1,scope='Every extracted FF6 scripted routing object; factual metadata only, no game implementation.',
            limitations=['Native battle, timing and audible response require gameplay confirmation.',
                         'A covered guide or reviewed edge is not evidence that every possible playthrough/state was exercised.',
                         'Unmodelled map-initializer control remains conservative in remote route projection.'],
            inputs={name:hashlib.sha256((DUMP/name).read_bytes()).hexdigest() for name in
                    ('ff6_objects.jsonl','ff6_scripts.jsonl','ff6_master/map.txt','ff6_master/script.txt')},
            totals=dict(submaps=len(json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'))),
                        scripts=len(scripts),scripted_objects=len(records),contact_encounters=len(contacts),
                        puzzle_controls=len(puzzle_controls),reviewed_story_travel=sum(r.get('interaction_travel',False) or
                        'direction-dispatched' in r.get('evidence','') for r in arrivals)),
            guides=[dict(id=g['id'],maps=g['maps'],state_rules=len(g['rules'])) for g in guides],
            contact_encounters=contacts,puzzle_controls=puzzle_controls,scripted_objects=records)
path=ROOT/'docs/re/ff6_story_audit.json'
path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report['totals']))
assert len(contacts)==59,'Review contact coverage if the extracted game changes'
