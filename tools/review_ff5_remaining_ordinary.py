"""Pin the remaining ordinary FFV scene, reward and encounter families."""
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent/'tools/mapdump'
TOPIC = 'docs/re/FF5_ORDINARY_STORY_20261105.md'
PROOF = ROOT/'docs/re/ff5_ordinary_story_native_20261105.json'

REASONS = {
    'sc_e_0498': 'One normal conversation with the old man. Facing determines the native approach and pose.',
    'sc_e_0236': 'The meal restores the party and records the native meal and reward receipts.',
    'sc_e_0400': 'The first Kelger conversation is a native staged scene. Later talk uses the existing dialogue branch.',
    'sc_e_0379': 'The trade checks the captured Kornago and Gil and asks for one confirmation. This is a normal service prerequisite.',
    'sc_e_0097': 'The Krile scene stages the party and records96 on its first visit.',
    'sc_e_0102': 'The automatic Bal return and greeting record101.',
    'sc_e_0103': 'The first dragon scene uses native actor presence to stage dialogue, then records102.',
    'sc_e_0103_1': 'The alternate dragon contact ends at the same102 story receipt.',
    'sc_e_0040': 'The king selects his normal story conversation from39 and47.',
    'sc_e_0094': 'The scattering moogles and timer callbacks are automatic scene presentation. Their later conversation is independent of the timer phase.',
    'sc_e_0095_2': 'Five contacts stage one moogle exit scene and hide the alternative contacts after the native receipt.',
    'sc_e_0232': 'The moogle grants the reward when the party has used the costume. The two actors share the same outfit and treasure branches.',
    'sc_e_0231': 'Examine the costume and confirm to put it on. This is a single preparation action for the moogle reward.',
    'sc_e_0376': 'This conversation sets the native moogle receipt. Facing changes only the actor choreography.',
    'sc_e_0274': 'The Brave Blade is a free alternative reward. Its pickup spends the shared choice and starts the native conclusion.',
    'sc_e_0274_1': 'The Chicken Knife is the other free reward choice. There is no required choice sequence.',
    'sc_e_0439': 'The old man appears and the full native conclusion records the ordinary story receipt.',
    'sc_e_0011': 'The first pirate observation contact runs one automatic story scene ending10.',
    'sc_e_0012': 'The pirate encounter is a staged conversation that records11.',
    'sc_e_0286': 'The Syldra scene grants its native reward and records the story receipt.',
    'sc_npc_30021_4_1': 'The ordinary pirate conversation grants its reward once and autosaves after receipt80.',
    'sc_e_0036': 'Boko appears in an automatic story scene. Native74 selects dialogue and placement, and35 ends the scene.',
    'sc_e_0348': 'Two contacts stage the same kiss scene and record248. The actual player movement is automatic.',
}
REASONS.update({f'sc_e_{number:04}':
                'One of five ordinary Walse crystal pickups. Any order is legal. The final receipt starts the full native escape scene.'
                for number in range(470,475)})
REASONS.update({f'sc_e_0249_{number}':
                'An optional Gil pickup with its own native index and a native Gil Turtle encounter roll. There is no correct order or safe-tile answer.'
                for number in range(1,14)})
AREAS = {'Quelb','Castle of Bal','Castle Walse','Tower of Walse','Moogle Village','Moore',"Pirates' Hideout",'Gil Cave','The Rift'}
CONTINUATIONS = {'ChangeScript','ChangeMap','Encount','EncountBoss','SetTimer'}


def read(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write(path,value):
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()


def signature(body):
    data = json.dumps(body,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()
    return hashlib.sha256(data).hexdigest()


def identity(row):
    return tuple(row[key] for key in ('game','submap','group','entity','raw'))


def main():
    check = '--check' in sys.argv
    master = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    paths = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt',
             'script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}
    for role,name in paths.items():
        assert digest(DUMP/name) == master['source_sha256'][role], role
    selected = [row for row in master['candidates'] if set(row['area_names_en']) & AREAS and row['script'] in REASONS]
    assert len(selected) == 47 and {row['script'] for row in selected} == set(REASONS)
    bodies = {}
    by_name = defaultdict(list)
    with (DUMP/paths['scripts']).open(encoding='utf-8') as stream:
        for line in stream:
            script = json.loads(line)
            key = script['submap'],script['name']
            bodies[key] = script['mnemonics']
            by_name[script['name']].append(key)
    todo = [(row['submap'],row['script']) for row in selected]
    signatures = {}
    while todo:
        key = todo.pop()
        locator = '/'.join(key)
        if locator in signatures:
            continue
        body = bodies[key]
        signatures[locator] = signature(body)
        if locator in master['source_script_hashes']:
            assert signatures[locator] == master['source_script_hashes'][locator], locator
        for command in body:
            if command['mnemonic'] not in CONTINUATIONS:
                continue
            name = (command['operands'].get('sValues') or [''])[0]
            if name:
                child = key[0],name
                matches = [child] if child in bodies else by_name[name]
                assert matches, (locator,name)
                todo.extend(matches)
    proof = dict(scope_tag='ff5_OrdinaryStory47',source_script_hashes=dict(sorted(signatures.items())),identities=len(selected))
    decisions = {}
    for row in selected:
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=signatures[row['submap']+'/'+row['script']],
                        disposition='ordinary navigation/story interaction',guides=[],
                        reason=REASONS[row['script']],evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    data = read(path)
    rows = [decisions.pop(identity(row),row) for row in data['decisions']]
    rows.extend(decisions.values())
    if check:
        assert read(PROOF) == proof and rows == data['decisions']
    else:
        write(PROOF,proof)
        data['decisions'] = rows
        write(path,data)
    print(f'FFV ordinary story: {len(selected)} exact identities, {len(signatures)} full roots and callbacks.')


if __name__ == '__main__':
    main()
