"""Verify the exact fixed-border exception from external matching game data.

Only factual counts/hashes/locators are emitted. No game source or script
stream is copied into the repository. This never launches or calls the game.
"""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DUMP = ROOT.parent / 'tools/mapdump'
objects_path = DUMP / 'ff3_objects.jsonl'
scripts_path = DUMP / 'ff3_scripts.jsonl'
objects = [json.loads(line) for line in objects_path.read_text(encoding='utf-8').splitlines()]
border = [row for row in objects if row['submap'] == 'Map_10010' and
          row['type_name'] == 'Event' and row['props'].get('script_id') in (341, 345, 349)]
assert len(border) == 1524
ids = {row['props']['entity_id'] for row in border}
assert len(ids) == 1524
for row in border:
    assert row['group'] == 'entity_default'
    assert row['w'] == row['h'] == 16
    assert row['props']['action_id'] == 2
    assert not row['props'].get('leave_script_id')
scripts = [json.loads(line) for line in scripts_path.read_text(encoding='utf-8').splitlines()]
references = []
for row in scripts:
    if row['submap'] != 'Map_10010':
        continue
    for operation in row['mnemonics']:
        if operation['mnemonic'] == 'SetPuppet' and operation['operands']['iValues'][1] in ids:
            references.append(row['name'])
assert not references, 'A fixed border acquired script-owned movement/visibility; retain ordinary observation'
native = ROOT.parent / 'tools/map_transport_audit_20261028/ff3_trigger_lifetime/provenance.json'
provenance = json.loads(native.read_text(encoding='utf-8'))
dll = Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY III PR/GameAssembly.dll')
assert hashlib.sha256(dll.read_bytes()).hexdigest() == provenance['dll_sha256']
assert hashlib.sha256((ROOT.parent/'ff3/dump.cs').read_bytes()).hexdigest() == provenance['dump_sha256']
print(json.dumps(dict(fixed_default_border_actors=len(border), puppet_references=len(references),
    native_power_and_cache4_contract='source-matched existing native archive',
    inputs={str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in
            (objects_path, scripts_path, native, dll)}), indent=2))
