"""Verify the final FFV movement and story-contact scope against native data."""
import hashlib
import json
from pathlib import Path
import sys

import pefile

from review_ff5_remaining_ordinary import ROOT, DUMP, digest, identity, read, signature, write
from western_valley_state import BRANCHES, extend

TOPIC = 'docs/re/FF5_STORY_CONTACTS_20261105.md'
PROOF = ROOT/'docs/re/ff5_story_contacts_native_20261105.json'
VALLEY, FOREST, VILLAGE, HOUSE, WORLD = 'Map_40001','Map_20110','Map_20130','Map_20131_3','Map_10010'
REASONS = {
    'sc_e_0478':'One mounted contact automatically moves Boko through both jumps before returning control.',
    'sc_e_0056':'One forest contact starts recruitment. The subsequent black chocobo conversation automatically runs its complete story scene.',
    'sc_e_0095':'One moogle conversation stages the party and opens the correct house. Native movement and transfer are automatic.',
}
WANTED = {
    VALLEY:('sc_map_40001','sc_e_0477','sc_e_0477_1','sc_e_0478','sc_e_0479_1','sc_e_0007'),
    FOREST:('sc_map_20110','sc_e_0056','sc_e_0364','sc_e_0364_2'),
    WORLD:('sc_e_0364_1','sc_e_0364_3'),
    VILLAGE:('sc_map_20130','sc_e_0094','sc_e_0094_1','sc_e_0094_2','sc_e_0095','sc_e_0096'),
    HOUSE:('sc_map_20131_3',),
}
FLAG_METHODS = (
    ('DataStorage.GetFlag',0xD8E270,208,'8d178c2386d85c8dc549387694b9045ccb6c2d1f00e3d32953886985f26b1501'),
    ('DataStorage.SetFlag',0xD8EB70,304,'edf8430a4ec234afe85b1752a45d89732f3a80599b7dcdbd09a34c9fd07b7bde'),
)


def main():
    check = '--check' in sys.argv
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    for role,path in {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt',
                      'script_master':'ff5_master/script.txt'}.items():
        assert digest(DUMP/path) == source['source_sha256'][role],role
    selected = [row for row in source['candidates'] if row['script'] in REASONS and row['submap'] in WANTED]
    assert len(selected) == 5
    scripts = {}
    with (DUMP/'ff5_scripts.jsonl').open(encoding='utf-8') as stream:
        for line in stream:
            row = json.loads(line)
            if row['name'] in WANTED.get(row['submap'],()):
                scripts[row['submap'],row['name']] = row['mnemonics']
    assert scripts.keys() == {(room,name) for room,names in WANTED.items() for name in names}

    def require(room,name,pc,op,ints,first_string=None):
        command = scripts[room,name][pc]
        assert command['mnemonic'] == op and command['operands']['iValues'][:len(ints)] == ints
        if first_string is not None:
            assert command['operands']['sValues'][0] == first_string

    for pc,point in ((36,52),(37,48),(40,48),(56,45),(57,42),(60,42)):
        require(VALLEY,'sc_e_0478',pc,'MoveTo',[1,0,point])
    require(VALLEY,'sc_e_0478',8,'SetFlag',[418],'ScenarioFlag1')
    require(FOREST,'sc_e_0056',12,'SetFlag',[159],'ScenarioFlag2')
    require(FOREST,'sc_e_0364',19,'ChangeMap',[1,1001],'sc_e_0364_1')
    require(WORLD,'sc_e_0364_1',13,'ChangeMap',[129,1],'sc_e_0364_2')
    require(FOREST,'sc_e_0364_2',52,'ChangeMap',[1,1001],'sc_e_0364_3')
    require(WORLD,'sc_e_0364_3',4,'SetFlag',[108],'ScenarioFlag2')
    require(VILLAGE,'sc_e_0095',30,'SetFlag',[94],'ScenarioFlag1')
    require(VILLAGE,'sc_e_0095',31,'ChangeMap',[139,1],'')

    actors = {}
    with (DUMP/'ff5_objects.jsonl').open(encoding='utf-8') as stream:
        for line in stream:
            obj = json.loads(line)
            if obj['submap'] in WANTED and obj['submap'] != WORLD:
                actors[obj['submap'],obj['props'].get('entity_id'),obj['group']] = obj
    for entity in (13,17,18):
        row = actors[VALLEY,entity,'entity_default']
        assert row['type_name'] == 'Event' and row['props']['target_transportation_ids'] == '3'
    assert actors[FOREST,111,'ev_e_0056']['props']['script_id'] == 166
    assert actors[VILLAGE,29,'ev_e_0095']['type_name'] == 'NPC'
    assert actors[VILLAGE,64,'ev_e_0094']['type_name'] == 'Entity'
    assert actors[VILLAGE,57,'ev_e_OpenTrigger']['type_name'] == 'OpenTrigger'
    for room,entity,other_map,point in ((VILLAGE,26,139,1),(HOUSE,8,136,4)):
        row = actors[room,entity,'entity_default']
        assert row['type_name'] == 'GotoMap' and (row['props']['map_id'],row['props']['point_id']) == (other_map,point)

    hashes = {'/'.join(key):signature(body) for key,body in sorted(scripts.items())}
    prior = read(ROOT.parent/'tools/ff5_library_20261101/puppet_position_provenance.json')
    dll = Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
    assert digest(dll) == prior['dll_sha256']
    image = pefile.PE(str(dll),fast_load=True)
    methods = []
    for name,rva,size,sha in FLAG_METHODS:
        assert hashlib.sha256(image.get_data(rva,size)).hexdigest() == sha,name
        methods.append(dict(method=name,rva=rva,bytes=size,sha256=sha))
    proof = dict(source_script_hashes=hashes,western_valley_transport=3,western_valley_waypoints=[52,48,45,42],
                 story_receipts=[418,415,364,94],moogle_house=[139,1,136,4],
                 flag_methods=methods,dll_sha256=prior['dll_sha256'])
    state_path = ROOT/'data/ff5/mapstate.json'
    state = read(state_path)
    code = state[VALLEY]['code']
    actual = [code[pc] for pc in BRANCHES]
    original = code.copy()
    for pc,(bit,target) in BRANCHES.items():
        original[pc] = [2,0,bit,0,-1,1,target]
    normalized = extend(dict(submap=VALLEY,name='sc_map_40001',mnemonics=scripts[VALLEY,'sc_map_40001']),original)
    if check:
        assert code == normalized
    elif code != normalized:
        assert actual == [[2,0,bit,0,-1,1,target] for bit,target in BRANCHES.values()]
        state[VALLEY]['code'] = normalized
        state_path.write_text(json.dumps(state,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
    decisions = {}
    for row in selected:
        key = row['submap']+'/'+row['script']
        assert hashes[key] == source['source_script_hashes'][key]
        decision = {name:row[name] for name in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=hashes[key],disposition='ordinary navigation/story interaction',
                        guides=[],reason=REASONS[row['script']],evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    ledger = read(path)
    rows = [decisions.pop(identity(row),row) for row in ledger['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == ledger['decisions'] and read(PROOF) == proof
    else:
        ledger['decisions'] = rows
        write(path,ledger)
        write(PROOF,proof)
    print(f'FFV story contacts: {len(selected)} review identities and {len(scripts)} complete program fingerprints.')


if __name__ == '__main__':
    main()
