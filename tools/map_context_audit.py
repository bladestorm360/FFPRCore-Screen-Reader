"""Finish factual candidate triage without changing bakes or launching games.

Full script bodies remain outside Git. Reports retain source locators, hashes,
counts and explicit unknowns; sample flags are never certified story states.
"""
from pathlib import Path
import collections,csv,hashlib,json,struct,subprocess
import bake_map_state

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
OUT=ROOT.parent/'tools/map_event_audit_20261028'
EVIDENCE=ROOT/'build_check/map_event_audit_20261028'
assert subprocess.run(['git','-C',str(OUT),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0

def sha(value):
    return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def rows(path):
    with path.open(encoding='utf-8-sig') as file:return list(csv.DictReader(file))

def decode(path):
    data=path.read_bytes();header=struct.unpack_from('<8sIB3s12I',data);strings=[];rooms={}
    for i in range(header[5]):
        at=struct.unpack_from('<I',data,header[4]+i*4)[0];strings.append(data[at:data.index(0,at)].decode())
    for i in range(header[7]):
        room=struct.unpack_from('<IihhBBBB10I',data,header[6]+i*56);objects=[]
        for j in range(room[-1]):
            item=struct.unpack_from('<BBhhHH4i2I',data,room[-2]+j*34)
            objects.append(dict(type=item[0],layer=item[1],x=item[2],y=item[3],a=item[6],id=item[8],group=strings[item[11]]))
        rooms[strings[room[0]]]=objects
    return rooms

reviews=json.loads((ROOT/'tools/world_event_reviews.json').read_text(encoding='utf8'))
reviewed={(r['game'],r['submap'],r['group'],r['id']) for r in reviews}
travel={'Nop','Branch','BranchByPlayerDirection','Jump','Exit','Return','Wait','FadeOut','FadeIn',
        'StopBGM','PlayBGM','StopEnvSe','PauseEnvSe','PlayEnvSe','ChangeMap','SetVehicle',
        'AddTelepoCache','RemoveTelepoCache','Calc','SetFlag','ResetFlag','AutoSave','ShowMapTitle'}
summary=[]
for n in range(1,7):
    source=[json.loads(line) for line in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8-sig')]
    exact={(s['submap'],s['name']):s for s in source};named=collections.defaultdict(list)
    for s in source:named[s['name']].append(s)
    maps={int(r['id']):r['map_name'] for r in rows(DUMP/f'ff{n}_master/map.txt') if r['id'] and int(r['id'])>0}
    master={int(r['id']):r['script_name'] for r in rows(DUMP/f'ff{n}_master/script.txt') if r['id']}
    raw=[json.loads(line) for line in (DUMP/f'ff{n}_objects.jsonl').open(encoding='utf-8-sig')]
    facts=json.loads((OUT/f'ff{n}_facts.json').read_text(encoding='utf8'))
    records=json.loads((ROOT/f'data/ff{n}/mapstate.json').read_text(encoding='utf8'))

    def resolve(sm,name):
        if (sm,name) in exact:return exact[sm,name]
        candidates=named[name]
        return candidates[0] if candidates and len({sha(s['mnemonics']) for s in candidates})==1 else None

    def closure(script,seen=None):
        seen=set(seen or ());identity=(script['submap'],script['name'])
        if identity in seen:return [],['script_cycle:'+script['name']]
        seen.add(identity);bodies=[script];gaps=[]
        for m in script.get('mnemonics') or []:
            op=m['mnemonic'];v=m.get('operands') or {};ss=v.get('sValues') or [];ii=v.get('iValues') or []
            if op=='ChangeMap' and ss and ss[0]:owner=maps.get(ii[0],'');name=ss[0]
            elif op=='ChangeScript' and ss and ss[0]:owner=script['submap'];name=ss[0]
            elif op=='ChangeScriptAtOnTrigger':gaps.append('native_trigger_script_lookup');continue
            else:continue
            child=resolve(owner,name)
            if child:
                found,unknown=closure(child,seen);bodies+=found;gaps+=unknown
            else:gaps.append('missing_or_ambiguous:'+name)
        return bodies,sorted(set(gaps))

    families=[]
    for family in json.loads((OUT/f'ff{n}_scene_review.json').read_text(encoding='utf8')):
        original=next(s for s in named[family['script']] if sha(s['mnemonics'])==family['body_sha256'])
        bodies,gaps=closure(original);ops=collections.Counter(m['mnemonic'] for s in bodies for m in s.get('mnemonics') or [])
        complete=all((n,o['submap'],o['group'],o['editor_id']) in reviewed for o in family['occurrences'])
        if complete:status='reviewed_functional_labels'
        elif gaps:status='source_or_continuation_gap'
        elif set(ops)&{'Msg','MsgFunfare','Encount','EncountBoss'}:status='dialogue_or_encounter_context'
        elif ops.get('ChangeMap') and set(ops)<=travel:status='additional_travel_candidate'
        else:status='silent_staging_or_physical_context'
        families.append(dict(script=family['script'],body_sha256=family['body_sha256'],status=status,
                             operations=dict(ops),gaps=gaps,occurrences=family['occurrences'],
                             bodies=[dict(submap=s['submap'],script=s['name'],body_sha256=sha(s['mnemonics'])) for s in bodies]))

    guards=[]
    for sm,record in records.items():
        for guard in record.get('events',[]):
            script=exact[sm,guard['source']];compiled=bake_map_state.entry_guard(script,f'ff{n}')
            assert compiled and all(compiled[k]==guard[k] for k in ('entry','code'))
            assert guard['type']!=3,'A physical GotoMap must not be consumed by a story objective predicate'
            guards.append(dict(submap=sm,id=guard['id'],type=guard['type'],raw=guard['raw'],script=guard['source'],
                               body_sha256=sha(script['mnemonics']),prefix_nodes=len(guard['code']),
                               classification='exact_native_prefix_before_first_action'))

    rooms=decode(ROOT/f'data/ff{n}/maps.bin');arrivals=[]
    for candidate in facts['ambiguous_door_arrivals']:
        target=maps[candidate['destination']]
        points=[o for o in rooms[target] if o['type']==0 and o['a']==candidate['point']]
        arrivals.append(dict(**candidate,target_submap=target,points=points,
                            classification='same_cell_variants' if len({(p['layer'],p['x'],p['y']) for p in points})==1 else 'conditional_or_dynamic_arrival'))
    omitted=[]
    for o in raw:
        p=o.get('props') or {}
        if o['type_name']!='EffectEntity' or not p.get('action_id') or o['submap'] not in set(maps.values()):continue
        name=master.get(p.get('script_id'), '')
        omitted.append(dict(submap=o['submap'],group=o['group'],id=p.get('entity_id'),raw=o['name'],
                            collision=p.get('collision'),script=name,
                            classification='collision_script_needs_native_observer_check' if p.get('collision') and name else 'visual_effect_without_script'))

    details=dict(game=n,families=families,entry_guards=guards,ambiguous_arrivals=arrivals,omitted_effects=omitted)
    (OUT/f'ff{n}_context_triage.json').write_text(json.dumps(details,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    row=dict(game=n,scene_families=len(families),family_status=dict(collections.Counter(f['status'] for f in families)),
             exact_entry_guards=len(guards),arrival_status=dict(collections.Counter(a['classification'] for a in arrivals)),
             omitted_effect_status=dict(collections.Counter(a['classification'] for a in omitted)))
    summary.append(row);print(json.dumps(row),flush=True)

with (EVIDENCE/'state_geometry.tsv').open(encoding='utf8') as file:geometry=list(csv.DictReader(file,delimiter='\t'))
result=dict(games=summary,geometry_total=len(geometry),geometry_status=dict(collections.Counter(r['classification'] for r in geometry)),
            geometry_scope='Production Resolver/obstacles/goals/flood with at most64 copied threshold samples per room; not exhaustive valid story states.',
            game_launched_or_probed=False,shipped_data_changed=False)
(EVIDENCE/'context_summary.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS exact22 entry-prefix metadata records; arrival/effect/family triage recorded without changing shipped data.')
