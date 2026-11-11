"""Keep the exact music calls from stopping the Karnak actor projection."""
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
ROOMS = {'Map_20070'} | {f'Map_20071_{floor}' for floor in range(1,17)}
MUSIC = 'BGMテンポアップ設定'

def read(path):
    return json.loads(path.read_text(encoding='utf-8'))

def music_index(script):
    room,name = script['submap'],script['name']
    assert room in ROOMS and name == 'sc_map_'+room[4:]
    source = json.dumps(script['mnemonics'],sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()
    proof = read(ROOT/'docs/re/ff5_karnak_native_20261105.json')
    assert hashlib.sha256(source).hexdigest() == proof['source_script_hashes'][room+'/'+name]
    index, = (pc for pc,row in enumerate(script['mnemonics']) if row['mnemonic'] == 'SysCall'
              and row['operands']['sValues'][0] == MUSIC)
    return index

def extend(script,game,region,code):
    room = script.get('submap','')
    if game != 'ff5' or room not in ROOMS or script.get('name') != 'sc_map_'+room[4:]:
        return code
    index = music_index(script)
    assert code[index] == [9]
    # The exact native audio closure is qualified in the Gogo source record.
    # The real game still runs the audio call. Only the copied actor program
    # skips its presentation effects and continues to the scene groups.
    code[index] = [0]
    return code

def main():
    check = '--check' in sys.argv
    path = ROOT/'data/ff5/mapstate.json'
    records = read(path)
    seen = set()
    with (ROOT.parent/'tools/mapdump/ff5_scripts.jsonl').open(encoding='utf-8') as source:
        for line in source:
            script = json.loads(line)
            room = script['submap']
            if room not in ROOMS or script['name'] != 'sc_map_'+room[4:]:
                continue
            assert room not in seen
            seen.add(room)
            index = music_index(script)
            code = records[room]['code']
            if check:
                assert code[index] == [0]
            else:
                assert code[index] in ([9],[0])
                code[index] = [0]
    assert seen == ROOMS
    if not check:
        path.write_text(json.dumps(records,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
    print('Karnak: 17 exact music-only calls preserve the actual actor and route state.')

if __name__ == '__main__':
    main()
