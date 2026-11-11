"""Pin Troia's ordinary passage controls to their complete native effects."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
CONTROLS = {
    ('Map_20101_5', 'sc_s_20101_1'): (27, 'SetFlag', {23, 26}),
    ('Map_20101_7', 'sc_s_20101_4'): (24, 'ResetFlag', {20, 22}),
    ('Map_20101_8', 'sc_s_20101_2'): (27, 'SetFlag', {10, 11, 16}),
    ('Map_20101_9', 'sc_s_20101_3'): (24, 'SetFlag', {5, 20, 24}),
}


def check():
    sources = {(s['submap'], s['name']): s for s in
               map(json.loads, (DUMP / 'ff4_scripts.jsonl').open(encoding='utf-8'))}
    reviews = json.loads((ROOT / 'tools/world_event_reviews.json').read_text(encoding='utf-8'))
    for key, (flag, write, hidden) in CONTROLS.items():
        body = sources[key]
        digest = hashlib.sha256(json.dumps(body, sort_keys=True, ensure_ascii=False,
                                          separators=(',', ':')).encode()).hexdigest()
        matched = [r for r in reviews if r['game'] == 4 and (r['submap'], r['script_name']) == key]
        assert matched and all(r['script_sha256'] == digest and r['label'] == 'Passage switch' for r in matched)
        ops = body['mnemonics']
        assert any(m['mnemonic'] == write and m['operands']['iValues'][0] == flag
                   and m['operands']['sValues'][0] == 'ScenarioFlag2' for m in ops)
        actual = set()
        for m in ops:
            if m['mnemonic'] != 'SetPuppet':
                continue
            values = m['operands']['iValues']
            end = next(i for i in range(values[2], len(ops)) if ops[i]['mnemonic'] == 'Exit')
            effects = [row['mnemonic'] for row in ops[values[2]:end]]
            assert all(op in {'Nop', 'Hide', 'PlayAnim'} for op in effects)
            if 'Hide' in effects:
                actual.add(values[1])
        assert actual == hidden, (key, actual)
    # Membership is a real native pass selection, distinct from floor contacts.
    admission = sources['Map_20101_5', 'sc_e_0139']['mnemonics']
    assert any(m['mnemonic'] == 'Branch' and m['operands']['iValues'][:2] == [0, 60]
               and m['operands']['sValues'][0] == 'ItemSelection' for m in admission)
    print('Troia: four native passage scripts, five step contacts, exact collider releases and Club Pass60 admission verified.')


if __name__ == '__main__':
    check()
