"""Normalize Western Valley's three native absolute flag operands at bake time."""
ROOM = 'Map_40001'
INITIALIZER = 'sc_map_40001'
BRANCHES = {8:(417,10),12:(418,14),16:(419,18)}


def extend(script, code):
    assert (script['submap'],script['name']) == (ROOM,INITIALIZER)
    body = script['mnemonics']
    assert len(body) == len(code) == 107
    for pc,(bit,target) in BRANCHES.items():
        row = body[pc]
        assert row['mnemonic'] == 'Branch'
        assert row['operands']['iValues'][:3] == [bit,1,target]
        assert row['operands']['sValues'][:3] == ['ScenarioFlag1','＝','imm']
        assert code[pc] == [2,0,bit,0,-1,1,target]
        # Native DataStorage bounds-checks the resulting absolute bit after
        # adding the segment offset. Store the equivalent canonical bank pair.
        code[pc] = [2,bit//256,bit%256,0,-1,1,target]
    return code
