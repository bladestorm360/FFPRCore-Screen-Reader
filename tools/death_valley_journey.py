"""Resolve Death Valley's playable world return after its final scene."""
from forest_journeys import transfers
from review_ff5_remaining_ordinary import signature

CAVE = 'Map_30451_1'
VALLEY = 'Map_30450'
WORLD = 'Map_10040'
ROOT = (CAVE,'sc_e_0492')
MID = (VALLEY,'sc_e_0492_1')
END = (WORLD,'sc_e_0492_2')
BOARDING_INIT = (VALLEY,'sc_map_30450')
HASHES = {
    ROOT:'c78b2e20623fc506ec2051d573e61831543f5e45e8d84ef19d84bb0d63242713',
    MID:'f6903ff781f9fb9046846f475ca8dec747f9e95545ef5b4ef7f178e310990c4c',
    END:'15948cf12bef924e39ab0ee6ad1093b06ec652b9b844c33840b7f47e1c15c078',
    (VALLEY,'sc_e_0131'):'86cfe4bf9e63be370fc3e3ef68f3240721b4968a32395195a1dc749ef3abfafa',
    (VALLEY,'sc_e_0131_3'):'cf271a022eda1c8be186310bf3a64393a08072dfd922794c095e1efd722a4820',
    (VALLEY,'sc_e_0131_4'):'58aee3c84e748c50817b9a31ee6ab8b1f27d56de8c9e325ea75ee7c5f07ccdf7',
    (CAVE,'sc_e_0131_1'):'923e3ff0e9c1839641adeac159a18b2a4456bfb4feb708eb863caeb44bc335db',
    (CAVE,'sc_e_0131_2'):'a9e6d4d767e9af15f3a6eee52cd4a90f71fa9b8ff9f4e479a83a06e1b8ce274b',
    BOARDING_INIT:'47545fdd4d2887aa7ff502463ec448ca821978a40145130c64fe9d0d326ad653',
}


def verified(scripts,key):
    body = scripts[key]['mnemonics']
    assert signature(body) == HASHES[key],key
    return body


def destination(obj,script,scripts):
    if (obj['submap'],obj['id']) != (CAVE,62):
        return None
    props = obj['props']
    assert (obj['type_name'],obj['group'],props['entity_id'],props['script_id'],props['action_id']) == (
        'Event','entity_default',20,420,2)
    assert script and script['name'] == ROOT[1]
    assert transfers(verified(scripts,ROOT)) == [(407,2,MID[1])]
    final = (19,6,END[1])
    assert transfers(verified(scripts,MID)) == [final]
    body = verified(scripts,END)
    assert not transfers(body) and body[1]['mnemonic'] == 'SysCall'
    assert all(body[pc]['mnemonic'] == 'SetPos' and body[pc]['operands']['iValues'][:2] == [121,89]
               for pc in (16,21,24))
    return final


def extend(script,game,region,code):
    if game != 'ff5' or (script.get('submap'),script.get('name')) != BOARDING_INIT:
        return code
    body = script['mnemonics']
    assert signature(body) == HASHES[BOARDING_INIT] and len(body) == 16
    assert body[4]['mnemonic'] == 'SysCall' and body[4]['operands']['sValues'][0] == '乗り物に乗る：チョコボ(通常)'
    assert code[4] == [9]
    # This exact call boards transport9 and applies its transport selectors.
    # Routing already owns those facts. The three fall actors are unrestricted
    # and their prior native group must survive the copied initializer.
    code[4] = [0]
    return code
