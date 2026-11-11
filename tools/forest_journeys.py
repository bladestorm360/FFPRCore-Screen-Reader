"""Resolve the playable arrival of the seven exact Forest of Moore scenes."""
import hashlib
import json

FIRE = ('Map_30400','sc_e_0119')
FIRE_WORLD = ('Map_10020','sc_e_0119_1')
FIRE_RETURN = ('Map_30420','sc_e_0119_2')
REFUGE_SCENE = ('Map_40029','sc_e_0263_1')
REFUGE_RETURN = ('Map_30421_1','sc_e_0263_2')
HASHES = {
    FIRE: 'be4e0ec70cbccff7fe32b757d0ce2d7cfc300a620dc5db8a203b6f98d7ddc5a2',
    FIRE_WORLD: '71bec381729f5d4cbe36f936c3b08d2289e5696fc88a045e23a900cdc6a21d50',
    FIRE_RETURN: '6e552f3aa9a4332daac23addec6a882a488757478ff38aedfbd1576056c38bc9',
    ('Map_30400','sc_e_0263'): 'e88a687c9474b658cb4bf78c96d13887dbe8a00b08e8615625e97851b01a5e71',
    ('Map_30420','sc_e_0263'): 'fe98142bf1207ed586edd972f4fb5119dd19697f357629d905421e3aa69f5cfe',
    ('Map_30430','sc_e_0263'): '08864612296633e5b21146d71235db6b12078460b86ac580fa277b4ab6ff1fe3',
    REFUGE_SCENE: 'c4cf70f61d7ab525624943d0db78dbc75691160b37a99b9d580a5b4f1a20186e',
    REFUGE_RETURN: '94368a6dc8287ed639b8c1926a3a61833a4eb399bcff4be0e3d276d910d58e90',
}
ROWS = {('Map_30400',41):(41,404,'ev_e_0119','sc_e_0119')}
ROWS.update({(room,editor):(entity,2061,group,'sc_e_0263')
             for room,group,pairs in (('Map_30400','ev_e_0263',((97,43),(98,44))),
                                      ('Map_30420','ev_e_0263',((97,43),(98,44))),
                                      ('Map_30430','entity_default',((27,27),(28,28))))
             for editor,entity in pairs})


def body(scripts,key):
    program = scripts[key]['mnemonics']
    encoded = json.dumps(program,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()
    assert hashlib.sha256(encoded).hexdigest() == HASHES[key],key
    return program


def transfers(program):
    return [(*row['operands']['iValues'][:2],row['operands']['sValues'][0])
            for row in program if row['mnemonic'] == 'ChangeMap']


def destination(obj,script,scripts):
    locator = obj['submap'],obj['id']
    if locator not in ROWS:
        return None
    entity,script_id,group,name = ROWS[locator]
    props = obj['props']
    assert (obj['type_name'],obj['name'],obj['group'],props['entity_id'],props['script_id'],props['action_id']) == (
        'Event',name,group,entity,script_id,2)
    assert script and script['name'] == name
    key = obj['submap'],name
    original = body(scripts,key)
    if key == FIRE:
        final = (381,4,'sc_e_0119_2')
        assert transfers(original) == [(12,8,'sc_e_0119_1')]
        assert transfers(body(scripts,FIRE_WORLD)) == [final]
        end = body(scripts,FIRE_RETURN)
        assert not transfers(end) and end[253]['mnemonic'] == 'SetPos'
        assert end[253]['operands']['iValues'][:4] == [36,11,0,0]
    else:
        final = (382,1,'sc_e_0263_2')
        assert transfers(original) == [(615,1,'sc_e_0263_1')]
        assert transfers(body(scripts,REFUGE_SCENE)) == [final,final]
        end = body(scripts,REFUGE_RETURN)
        assert not transfers(end) and end[92]['operands']['iValues'][:4] == [16,10,2,0]
        # The other branch places the player at 16,6, then walks south twice
        # for two steps each. Both native branches finish on the same tile.
        assert end[102]['operands']['iValues'][:4] == [16,6,0,0]
        assert all(end[pc]['mnemonic'] == 'Move' and end[pc]['operands']['iValues'][:2] == [2,2]
                   for pc in (115,126))
    return final
