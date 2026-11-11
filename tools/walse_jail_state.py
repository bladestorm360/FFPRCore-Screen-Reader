"""Project the native Walse jail door's one reviewed absolute placement."""
import hashlib
import json
from pathlib import Path
import sys

import pefile
import bake_maps

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent/'tools/mapdump'
ROOM = 'Map_20041_9'
INITIALIZER = 'sc_map_20041_9'
TOPIC = 'docs/re/FF5_WALSE_JAIL_20261105.md'
PROOF = ROOT/'docs/re/ff5_walse_jail_native_20261105.json'
SOURCE = {
    INITIALIZER: '38ed2b54166efbeef27f037adddc0cc84410ee57aec9e2b4027a2fe91a390d7b',
    'sc_e_0182': '20cb0dc15dda9e989ff1ced06679a606d6c00f9a63f3b4b1e534783efb2ef236',
    'sc_e_0495': 'd79c4949d919071c8365087dbefb7ad51524a0ac4d0f84fef085ebd81e435b08',
    'sc_e_0428': 'abff0d29453e60318ad406a113438103ed9fbfce043777428624a03630c97396',
}
PUPPET = 52
OLD = [10,4,24,1,-2147483648,-2147483648]
PLACEMENT = [10,4,24,1,8,11]


def read(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write(path,value,compact=False):
    encoded = json.dumps(value,ensure_ascii=False,indent=None if compact else 2,
                         separators=(',',':') if compact else None)
    path.write_text(encoded+'\n',encoding='utf-8')


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()


def signature(body):
    encoded = json.dumps(body,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()
    return hashlib.sha256(encoded).hexdigest()


def extend(script,game,region,code):
    if game != 'ff5' or script.get('submap') != ROOM or script.get('name') != INITIALIZER:
        return code
    body = script['mnemonics']
    assert signature(body) == SOURCE[INITIALIZER] and list(region[:2]) == [0,0]
    assert len(body) == 70 and code[PUPPET] == OLD
    assert body[PUPPET]['mnemonic'] == 'SetPuppet' and body[PUPPET]['operands']['iValues'][:3] == [4,24,67]
    assert body[68]['mnemonic'] == 'SetPos' and body[68]['operands']['iValues'][:4] == [8,11,0,0]
    # The closed branch places this off-map collider into the jail entrance.
    # Retain its actual Ground layer. The open branch still uses the native
    # off-map template and never executes this placement.
    code[PUPPET] = PLACEMENT
    return code


def load_sources():
    scripts = {}
    with (DUMP/'ff5_scripts.jsonl').open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            if script['submap'] == ROOM and script['name'] in SOURCE:
                scripts[script['name']] = script
    assert {name:signature(script['mnemonics']) for name,script in scripts.items()} == SOURCE
    objects = []
    with (DUMP/'ff5_objects.jsonl').open(encoding='utf-8') as stream:
        for line in stream:
            row = json.loads(line)
            if row['submap'] == ROOM:
                objects.append(row)
    region = bake_maps.map_bounds(read(DUMP/'ff5_submaps.json')[ROOM],objects)
    gate, = (row for row in objects if row['props'].get('entity_id') == 24 and row['type_name'] == 'Entity')
    assert gate['props']['collision'] and gate['group'] == 'entity_default' and gate['layer'] == 'GroundObject'
    assert (gate['tx'],bake_maps.tile_row(gate)) == (-1,0)
    return scripts,region


def review(check):
    source = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    reasons = {
        'sc_e_0182':'The two native closing contacts set355 and move the jail collider into the door. They are ordinary automatic state changes.',
        'sc_e_0495':'The reachable inside contact clears355 and hides the real blocker. This is one automatic opening interaction.',
        'sc_e_0428':'One normal confirmation releases the prisoner and records169. It has no ordered answer or navigation puzzle.',
    }
    selected = [row for row in source['candidates'] if row['submap'] == ROOM and row['script'] in reasons]
    assert len(selected) == 4
    keys = ('game','submap','group','entity','raw','script')
    decisions = {}
    for row in selected:
        identity = tuple(row[key] for key in keys)
        assert source['source_script_hashes'][ROOM+'/'+row['script']] == SOURCE[row['script']]
        decisions[identity] = dict(zip(keys,identity),script_sha256=SOURCE[row['script']],
                                   disposition='ordinary navigation/story interaction',guides=[],
                                   reason=reasons[row['script']],evidence=TOPIC)
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    data = read(path)
    rows = [decisions.pop(tuple(row[key] for key in keys),row) for row in data['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == data['decisions']
    else:
        data['decisions'] = rows
        write(path,data)


def main():
    check = '--check' in sys.argv
    scripts,region = load_sources()
    position = read(ROOT.parent/'tools/ff5_library_20261101/puppet_position_provenance.json')
    dll = Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')
    assert digest(dll) == position['dll_sha256'] and digest(ROOT.parent/'FF5/dump.cs') == position['dump_sha256']
    image = pefile.PE(str(dll),fast_load=True)
    for method in position['functions']:
        assert hashlib.sha256(image.get_data(method['rva'],method['bytes'])).hexdigest() == method['sha256']
    proof = dict(scope_tag='ff5_WalseJail',source_script_hashes={ROOM+'/'+name:sha for name,sha in SOURCE.items()},
                 native_position=position,collider=24,pose=[1,8,11],locked_scenario=355,opening_contact=27)
    path = ROOT/'data/ff5/mapstate.json'
    data = read(path)
    code = data[ROOM]['code']
    if check:
        assert code[PUPPET] == PLACEMENT and read(PROOF) == proof
    else:
        assert code[PUPPET] in (OLD,PLACEMENT)
        code[PUPPET] = OLD
        extend(scripts[INITIALIZER],'ff5',region,code)
        write(path,data,compact=True)
        write(PROOF,proof)
    review(check)
    print('Walse jail: one exact locked door placement and four ordinary native interactions.')


if __name__ == '__main__':
    main()
