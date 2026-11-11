"""Preserve the native live switch state separately from the reload initializer."""
from review_ff5_remaining_ordinary import signature

ROOM = 'Map_30611_12'
INITIALIZER = 'sc_map_30611_12'
SOURCE_HASH = '8d051ea597685c899f1c0d0170f0cebbec4b20b7955cf9c6e4051c2db7bc7cad'


def live_code(script,code):
    assert (script['submap'],script['name']) == (ROOM,INITIALIZER)
    assert signature(script['mnemonics']) == SOURCE_HASH and len(code) == 138
    assert code[11] == [2,0,195,0,-1,1,24] and code[15] == [7,0,195,1]
    assert code[20:23] == [[7,0,192,0],[7,0,193,0],[7,0,194,0]]
    current = code.copy()
    # Loaded rooms already ran the original initializer. Reload-only flag
    # writes cannot open the live chest early or reset real pressed switches.
    # The original area latch and actual success branch still run unchanged.
    current[12] = [3,len(current)]
    for receipt,actor in ((192,8),(193,10),(194,9)):
        current.append([2,0,receipt,0,-1,0,len(current)+3])
        current.append([10,0,actor,0,-2147483648,-2147483648])
        current.append([11,0])
    current.append([6])
    return current
