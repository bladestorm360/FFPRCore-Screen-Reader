"""Read exact native initializer facts. Proprietary bytes remain outside Git.

Run map_location_native.py N --chocobo-rules first. This only reads installed
files and existing dumps; it never initializes or executes the game.
"""
from pathlib import Path
import csv,hashlib,json,re,struct,subprocess
ROOT=Path(__file__).resolve().parents[2]
FOLDERS=('ff1','ff2','ff3','FF4','FF5','ff6')
TITLES=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')
for number,(folder,title) in enumerate(zip(FOLDERS,TITLES),1):
    source=ROOT.parent/folder
    out=ROOT.parent/'tools/chocobo_owen_20261025'/f'ff{number}'
    assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    native=(out/'native.txt').read_text(encoding='utf8').split('\n\n')[0]
    lines=native.splitlines()
    assert 'MapConstants..cctor()' in native
    dump=(source/'dump.cs').read_text(encoding='utf-8-sig')
    script=json.loads((source/'script.json').read_text(encoding='utf-8-sig'))
    fields={r['Address']:r['Name'] for r in script['ScriptMetadata']}
    metadata=Path('D:/Games/SteamLibrary/steamapps/common')/(title+' PR')/(title+'_Data/il2cpp_data/Metadata/global-metadata.dat')
    raw=metadata.read_bytes();tables=[];filters=[]
    for index,line in enumerate(lines):
        if 'RuntimeHelpers.InitializeArray' not in line:continue
        field=None;operand=None
        for previous in reversed(lines[max(0,index-9):index]):
            match=re.search(r'^([0-9A-F]+) mov rdx, qword ptr \[rip \+ (0x[0-9a-f]+)\]',previous)
            if match:
                field=fields.get(int(match[1],16)+7+int(match[2],16)-0x180000000);operand=previous;break
        if not field:continue
        name=field.split('.')[-1]
        match=re.search(r'StaticArrayInitTypeSize=(\d+) '+name+r' /\*Metadata offset (0x[0-9A-F]+)\*/',dump)
        if not match:continue
        size=int(match[1]);offset=int(match[2],16);value=raw[offset:offset+size]
        if 21<=size<=32 and all(b in (0,1) for b in value):
            assert all(b in (0,1) for b in value)
            following=lines[index+1:index+16]
            key=next((re.search(r'lea edx, \[(\w+) \+ (0x[0-9a-f]+|\d+)\]',s) for s in following if re.search(r'lea edx, \[(\w+) \+ (0x[0-9a-f]+|\d+)\]',s)),None)
            direct=None
            if key is None:direct=next((re.search(r'mov edx, (0x[0-9a-f]+|\d+)$',s) for s in following if re.search(r'mov edx, (0x[0-9a-f]+|\d+)$',s)),None)
            match_key=key or direct
            if match_key is None:continue
            key_at=next(i for i,s in enumerate(following) if match_key[0] in s)
            if not any('mov r8, rbx' in s for s in following[max(0,key_at-3):key_at+4]):continue
            if key:assert key[1]=='rsi' and 'xor esi, esi' in native
            key_value=int(key[2] if key else direct[1],0)
            tables.append({'type':key_value,'goto_map_enabled':bool(value[3]),'enabled_object_types':[i for i,b in enumerate(value) if b],
                           'field':name,'metadata_offset':offset,'field_load':operand})
        if size==56:filters.append({'field':name,'metadata_offset':offset,'values':struct.unpack('<14i',value)})
    keys={r['type'] for r in tables}
    assert {1,5}.issubset(keys) and keys.issubset({1,5,11,12}) and all(r['goto_map_enabled'] for r in tables),(number,tables)
    rows=csv.DictReader((ROOT.parent/f'tools/mapdump/ff{number}_master/transportation.txt').open(encoding='utf-8-sig'))
    required={int(row['type_id']) for row in rows if row.get('type_id','').isdigit() and int(row['type_id']) in (11,12)}
    assert required.issubset(keys),(number,required,keys)
    expected=(15,0,14,11,7,13,6,9,0,0,0,0,0,0)
    assert any(r['values']==expected for r in filters)
    result={'game':number,'metadata_sha256':hashlib.sha256(raw).hexdigest(),
            'dump_sha256':hashlib.sha256((source/'dump.cs').read_bytes()).hexdigest(),
            'script_metadata_sha256':hashlib.sha256((source/'script.json').read_bytes()).hexdigest(),
            'native_provenance':'provenance.json','explicit_trigger_tables':tables,'int14_tables':filters,
            'default_goto_map_enabled':False,'note':'Default table enables Event2 and TransportationEventAction21 only; verified initializer stores remain in native.txt.'}
    (out/'facts.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(f'FF{number}: native half-collision filter and entry types{sorted(keys)} verified against exact metadata')
