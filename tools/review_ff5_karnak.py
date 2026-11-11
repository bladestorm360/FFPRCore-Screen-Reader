"""Pin the complete timed escape, world return, and ordinary prelude."""
import hashlib
import json
from pathlib import Path
import sys

import pefile

from author_ff5_karnak import ROOT, DUMP, TOPIC, author, read, write

PROOF = ROOT/'docs/re/ff5_karnak_native_20261105.json'
EXTERNAL = ROOT.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_Karnak'
BUILD = ROOT/'build_check/broader_audit_20261104/ff5_Karnak'
DLL = Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll')

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()

def identity(row):
    return tuple(row[key] for key in ('game','submap','group','entity','raw'))

def main():
    check = '--check' in sys.argv
    guide,arrivals = author()
    native = read(ROOT.parent/'tools/ff5_library_20261101/puppet_position_provenance.json')
    assert digest(DLL) == native['dll_sha256'] and digest(ROOT.parent/'FF5/dump.cs') == native['dump_sha256']
    music = read(ROOT/'docs/re/ff5_gogo_native_20261104.json')
    assert music['dll_sha256'] == native['dll_sha256'] and music['dump_sha256'] == native['dump_sha256']
    assert len(music['native_functions']) == 6
    image = pefile.PE(str(DLL),fast_load=True)
    for method in native['functions']+music['native_functions']:
        assert hashlib.sha256(image.get_data(method['rva'],method['bytes'])).hexdigest() == method['sha256']
    asset = read(PROOF)['battle_timer'] if PROOF.exists() else read(BUILD/'timer_asset.json')
    battle = EXTERNAL/(asset['name']+'.json')
    bundle = DLL.parent/'FINAL FANTASY V_Data/StreamingAssets/aa/StandaloneWindows64'/asset['bundle']
    assert digest(battle) == asset['sha256'] and digest(bundle) == asset['bundle_sha256']
    mnemonics = read(battle)['Mnemonics']
    assert len(mnemonics) == asset['operations'] == 38
    assert mnemonics[27]['operands']['sValues'][0] == 'FinishBattleByTimeOut'
    assert mnemonics[35]['mnemonic'] == 'BattleForceExitOnSave'

    master = read(ROOT/'docs/re/ff5_remaining_native_20261104.json')
    source_files = {'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt',
                    'script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}
    for role,name in source_files.items():
        assert digest(DUMP/name) == master['source_sha256'][role],role
    selected = [row for row in master['candidates'] if 'Karnak Castle' in row['area_names_en']]
    assert len(selected) == 6
    proof = dict(scope_tag='ff5_Karnak',source_script_hashes=guide['source_script_hashes'],battle_timer=asset,
                 native_position=native,timer_seconds=600,start=[102,1,4,7],final=[1,2,83,78],
                 final_scenario_receipts=[47,992],exits=arrivals,
                 retained_music_methods=music['native_functions'],music_projection_count=17)
    if check:
        assert read(PROOF) == proof
    else:
        write(PROOF,proof)
    decisions = {}
    reasons = {
        'sc_e_0044':'Ordinary automatic wolf encounter at the entrance. The native scene records43 and does not start the timer.',
        'sc_e_0043':'The three prison contacts trigger the same ordinary automatic scene and its native receipt353.',
        'sc_e_0043_1':'Ordinary Cid scene records42 after its full dialogue and actor movements. Timed guidance starts later at the actual102 landing.',
        'sc_e_0048':'The courtyard contact owns native battle446. The complete timed guide connects the actual102 start, stairs, encounter, three exit contacts and ready world return after47 and992.',
    }
    for row in selected:
        locator = row['submap']+'/'+row['script']
        signature = proof['source_script_hashes'][locator]
        assert signature == master['source_script_hashes'][locator]
        guided = row['script'] == 'sc_e_0048'
        decision = {key:row[key] for key in ('game','submap','group','entity','raw','script')}
        decision.update(script_sha256=signature,disposition='guided complex puzzle' if guided else 'ordinary navigation/story interaction',
                        guides=[guide['id']] if guided else [],reason=reasons[row['script']],evidence=TOPIC)
        decisions[identity(row)] = decision
    path = ROOT/'docs/re/puzzle_semantic_review_20261101.json'
    data = read(path)
    rows = [decisions.pop(identity(row),row) for row in data['decisions']]
    rows.extend(decisions.values())
    if check:
        assert rows == data['decisions']
    else:
        data['decisions'] = rows
        write(path,data)
    print('Karnak: six identities, 120 field programs, the native battle timer, and four placement methods.')

if __name__ == '__main__':
    main()
