"""Pin Death Valley's post-battle and mounted return poses."""
import json
import sys
import hashlib
from pathlib import Path

import pefile

from death_valley_journey import CAVE, VALLEY, WORLD, HASHES, ROOT as RESCUE, END, BOARDING_INIT, destination, verified, extend
from forest_journeys import transfers
from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write

TOPIC = 'docs/re/FF5_DEATH_VALLEY_20261105.md'
PROOF = ROOT/'docs/re/ff5_death_valley_native_20261105.json'
FALLS = {1:('sc_e_0131',12,416),2:('sc_e_0131_3',13,2653),3:('sc_e_0131_4',14,2654)}
REASONS = {
    'sc_e_0132': 'Any of the four initial contacts exposes the same rope and records341.',
    'sc_e_0132_1': 'The first automatic lift returns the player beside the same rope and records342.',
    'sc_e_0490': 'The next automatic lift returns the player beside the same rope and records343.',
    'sc_e_0491': 'The next lift restores the player next to the same rope and records344.',
    'sc_e_0492': 'The final approach runs the native rescue and mounts Boko on the world map after the full scene.',
}
REASONS.update({name:'One of the original fall contacts. The native scene dismounts, runs the encounter, and returns control on the cave floor.'
                for name,_,_ in FALLS.values()})


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if 'Death Valley' in row['area_names_en']]
    assert len(selected) == 11 and {row['script'] for row in selected} == set(REASONS)
    wanted = set(HASHES) | {(row['submap'],row['script']) for row in selected} | {
        (room,'sc_map_'+room[4:]) for room in (CAVE,VALLEY,WORLD)}
    scripts = {}
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            if key in wanted:
                scripts[key] = script
    assert scripts.keys() == wanted
    pins = {'/'.join(key):signature(row['mnemonics']) for key,row in sorted(scripts.items())}
    actors = {}
    with (DUMP/paths['objects']).open(encoding='utf-8') as stream:
        for line in stream:
            actor = json.loads(line)
            if actor['submap'] in (VALLEY,CAVE) and actor['type_name'] == 'Event':
                actors[actor['submap'],actor['id']] = actor

    battle_key = CAVE,'sc_e_0131_1'
    ready_key = CAVE,'sc_e_0131_2'
    boss = verified(scripts,battle_key)[41]
    assert boss['mnemonic'] == 'EncountBoss' and boss['operands']['iValues'][:3] == [475,14,18]
    assert boss['operands']['sValues'][0] == ready_key[1]
    ready = verified(scripts,ready_key)
    assert ready[62]['mnemonic'] == 'SetPos' and ready[62]['operands']['iValues'][:4] == [14,18,2,0]
    overrides = []
    for entity,(name,editor,script_id) in FALLS.items():
        actor = actors[VALLEY,editor]
        assert actor['group'] == 'ev_e_0131' and actor['props']['entity_id'] == entity
        assert actor['props']['script_id'] == script_id and actor['props']['action_id'] == 2
        assert not actor['props'].get('target_transportation_ids')
        assert transfers(verified(scripts,(VALLEY,name))) == [(408,1,battle_key[1])]
        overrides.append(dict(submap=VALLEY,entity_id=entity,object_id=editor,destination_map=408,original_point=1,
                              continuation=battle_key[1],continuation_sha256=signature(scripts[battle_key]),
                              arrival_cell=[14,18,1],trap_transfer=True,evidence=TOPIC))
    final = destination(actors[CAVE,62],scripts[RESCUE],scripts)
    assert final == (19,6,END[1])
    overrides.append(dict(submap=CAVE,entity_id=20,object_id=62,destination_map=19,original_point=6,
                          continuation=END[1],continuation_sha256=signature(scripts[END]),
                          arrival_cell=[121,89,1],evidence=TOPIC))
    prior = read(ROOT.parent/'tools/ff5_library_20261101/puppet_position_provenance.json')
    facts = read(PROOF).get('native_boarding') if PROOF.exists() else None
    if facts is None:
        archive = ROOT/'build_check/broader_audit_20261104'
        facts = read(archive/'death_valley_boarding_facts.json') + read(archive/'death_valley_native_boarding_dispatch_facts.json')
    dll = Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
    assert len(facts) == 5 and digest(dll) == prior['dll_sha256'] and digest(ROOT.parent/'FF5/dump.cs') == prior['dump_sha256']
    image = pefile.PE(str(dll),fast_load=True)
    for method in facts:
        assert hashlib.sha256(image.get_data(method['rva'],method['bytes'])).hexdigest() == method['sha256']
    proof = dict(scope_tag='ff5_DeathValley',source_script_hashes=pins,arrivals=overrides,encounter=475,
                 rope_receipts=[341,342,343,344,132],world_transport=9,native_boarding=facts,
                 dll_sha256=prior['dll_sha256'],dump_sha256=prior['dump_sha256'])

    state_path = ROOT/'data/ff5/mapstate.json'
    state = read(state_path)
    code = state[VALLEY]['code']
    if check:
        assert code[4] == [0]
    elif code[4] != [0]:
        extend(scripts[BOARDING_INIT],'ff5',None,code)
        state_path.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')

    path = ROOT/'data/ff5/warp_arrival_overrides.json'
    old = read(path)
    identities = {(row['submap'],row['entity_id']) for row in overrides}
    existing = [row for row in old if (row['submap'],row['entity_id']) in identities]
    if check:
        assert existing == overrides and read(PROOF) == proof
    else:
        assert not existing or existing == overrides
        if not existing:
            write(path,old+overrides)
        write(PROOF,proof)

    decisions = {}
    for row in selected:
        key = row['submap']+'/'+row['script']
        assert pins[key] == source['source_script_hashes'][key]
        decision = {name:row[name] for name in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=pins[key],disposition='ordinary navigation/story interaction',
                        guides=[],reason=REASONS[row['script']],evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    ledger = read(path)
    rows = [decisions.pop(identity(row),row) for row in ledger['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == ledger['decisions']
    else:
        ledger['decisions'] = rows
        write(path,ledger)
    print(f'Death Valley: {len(selected)} ordinary identities, {len(scripts)} pinned programs, and four playable arrivals.')


if __name__ == '__main__':
    main()
