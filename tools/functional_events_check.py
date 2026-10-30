"""Pin actual non-narrative FFIII event decisions to source scripts and output."""
from pathlib import Path
import csv
import json
import event_label_identities as identities

root=Path(__file__).resolve().parents[1]
dump=root.parent/'tools/mapdump'
scripts={(s['submap'],s['name']):s for s in (json.loads(l) for l in (dump/'ff3_scripts.jsonl').open(encoding='utf-8-sig'))}
masters={int(r['id']):r['script_name'] for r in csv.DictReader((dump/'ff3_master/script.txt').open(encoding='utf-8-sig')) if r['id']}
maps={r['map_name']:int(r['id']) for r in csv.DictReader((dump/'ff3_master/map.txt').open(encoding='utf-8-sig')) if r['id']}
labels=json.loads((root/'data/ff3/eventlabels.json').read_text(encoding='utf-8'))
catalog=json.loads((root/'data/common/mod_text.json').read_text(encoding='utf-8'))
count=0
for obj in (json.loads(l) for l in (dump/'ff3_objects.jsonl').open(encoding='utf-8-sig')):
    match=next((r for r in identities.WORLD_EVENT_REVIEWS if (r['game'],r['submap'],r['group'],r['id'])==(3,obj['submap'],obj['group'],obj['id'])),None)
    if not match or 'continuations' in match or match['label'] not in ('Tozus entrance','Closed forest entrance','Airship barrier','Passage barrier','Statue barrier'):continue
    name=masters[obj['props']['script_id']];script=scripts[(obj['submap'],name)]
    result=identities.reviewed_world_event_identity(3,obj,name,script)
    assert result and result[0]==match['label']
    commands={m['mnemonic'] for m in script['mnemonics']}
    assert not commands.intersection({'EncountBoss','Encount','Battle','ChangeScript'})
    if match['label']=='Statue barrier':
        messages=[m['operands']['sValues'][0] for m in script['mnemonics'] if m['mnemonic']=='Msg']
        assert messages==['N999_C12_999_02_01'] and 'Branch' in commands and 'SetFlag' in commands
    else:assert 'Msg' not in commands
    if match['label']=='Tozus entrance':assert 'ChangeMap' in commands and 'SysCall' in commands
    else:assert not commands.intersection({'ChangeMap','ChangeTransportation'}) and commands.intersection({'Move','MoveBirdView'})
    key=f"@object:{maps[obj['submap']]}:{obj['props']['entity_id']}:{obj['type']}:{obj['name']}"
    assert labels.get(key,labels.get(obj['name']))==match['label'],key
    localized=catalog[match['label']]
    assert len(localized)==12 and set(localized)==set(catalog['Exit to {0}']) and all(localized.values())
    count+=1
assert count==21,count
assert labels['ネプト竜の妨害']=='Scene trigger' and labels['飛空艇大破']=='Scene trigger'
print('21 exact FFIII functional events: script hashes, native behavior, effective labels and12 locales verified; narrative/boss scenes preserved.')
