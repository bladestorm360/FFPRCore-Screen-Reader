"""Audit all-six applied-player-asset metadata; output facts outside Git only."""
from pathlib import Path
import hashlib
import json
import re

root=Path(__file__).resolve().parents[2]
records=[]
for n,folder in enumerate(('ff1','ff2','ff3','FF4','FF5','ff6'),1):
    dump=root.parent/folder/'dump.cs';text=dump.read_text(encoding='utf-8-sig')
    blocks=text.split('// Namespace: ')
    sprite=next(b for b in blocks if b.startswith('Last.Entity.Field\npublic class FieldSpriteEntity :'))
    info=next(b for b in blocks if b.startswith('Last.Map\npublic class TransportationInfo '))
    fields={}
    reviewed=[('<AssetStructure>k__BackingField',sprite),('<Structure>k__BackingField',info)]
    if n in (3,5,6):reviewed.append(('<FrontStructure>k__BackingField',info))
    else:assert '<FrontStructure>k__BackingField' not in info
    for key,block in reviewed:
        match=re.search(re.escape(key)+r'; // (0x[\dA-F]+)',block);assert match,key
        fields[key]=match[1]
    path=root.parent/f'tools/vehicle_state_20261028/ff{n}'
    native=(path/'native.txt').read_text(encoding='utf-8')
    block=next(c for c in native.split('\n\n') if c.startswith('Last.Map.TransportationController.Last.Map.ITransportation.SetupCharaAsset('))
    # Confirm both native branches apply the selected structure directly; the
    # detailed branch/control-flow review is recorded in VEHICLE_MAPS_20261028.
    assert block.count('FieldSpriteEntity.SetAssetStructure(')==(2 if n in (3,5,6) else 1)
    assert fields['<Structure>k__BackingField']=='0x30' and '+ 0x30]' in block
    if n in (3,5,6):assert fields['<FrontStructure>k__BackingField']=='0x38' and '+ 0x38]' in block
    records.append({'game':n,'dump_sha256':hashlib.sha256(dump.read_bytes()).hexdigest(),
                    'fields':fields,'setup_locator':block.splitlines()[0]})
out=root.parent/'tools/vehicle_state_20261028/reviewed_facts.json'
out.write_text(json.dumps(records,indent=2)+'\n',encoding='utf-8')
print('All six exact native applied-asset fields and both SetupCharaAsset branches verified; facts outside Git.')
