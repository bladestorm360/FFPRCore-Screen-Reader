"""Read-only native movement/Temple facts; writes metadata, never game source."""
from pathlib import Path
import hashlib
import json
import re

root = Path(__file__).resolve().parents[2]
facts = {}
for number, folder in enumerate(('ff1','ff2','ff3','FF4','FF5','ff6'),1):
    path = root.parent / folder / 'dump.cs'
    data = path.read_text(encoding='utf-8-sig')
    blocks = data.split('// Namespace: ')
    player = next(b for b in blocks if b.startswith('Last.Entity.Field\n') and
                  re.search(r'^public class FieldPlayer :', b, re.M))
    mode = re.search(r'private FieldPlayerConstants.MoveState moveState; // (0x[0-9A-F]+)',player)
    assert mode
    transport = next(b for b in blocks if b.startswith('Last.Map\n') and
                     re.search(r'^public class TransportationInfo ',b,re.M))
    assert 'public int get_StayMapId()' in transport
    enum = next(b for b in blocks if re.search(r'^public enum FieldPlayerConstants.MoveState ',b,re.M))
    values = {name:int(value) for name,value in re.findall(r'public const FieldPlayerConstants.MoveState (\w+) = (\d+);',enum)}
    assert values['Walk'] == 0 and values['Dush'] == 1
    facts[str(number)] = {'dump_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                          'native_move_state_offset':mode[1], 'enum':values,
                          'transportation_map_getter':'Last.Map.TransportationInfo.get_StayMapId()'}

scripts = root.parent / 'tools/mapdump/ff3_scripts.jsonl'
temple = []
for line in scripts.read_text(encoding='utf-8-sig').splitlines():
    row = json.loads(line)
    if row['map'] == 'Map_20191':
        temple.append({'submap':row['submap'],'script':row['name'],
                       'riding_vehicle':row.get('system_flag',{}).get('RidingVehicle')})
assert next(row for row in temple if row['script']=='sc_map_20191_1')['riding_vehicle']==0
assert all(row['riding_vehicle'] in (None,0) for row in temple)
out = root / 'build_check/temple_vehicle_20261027'
out.mkdir(exist_ok=True)
(out/'native_facts.json').write_text(json.dumps({'players':facts,'temple_scripts':temple,
    'ff3_transport_move_modes':{'Content_type5':0,'Submarine_type6':0},
    'scripts_sha256':hashlib.sha256(scripts.read_bytes()).hexdigest()},indent=2)+'\n')
print(f'All six native movement fields/enums verified; Temple entry declares RidingVehicle=0; {sum(r["riding_vehicle"]==0 for r in temple)} explicit values, {sum(r["riding_vehicle"] is None for r in temple)} unspecified.')
