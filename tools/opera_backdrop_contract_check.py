"""Exact Opera backdrop binding/renderer provenance; native bodies stay external."""
import hashlib
import json
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ARCHIVE=ROOT.parent/'tools/puzzle_audit_20261030/ff6_opera_backdrop'
ENTRY=ROOT.parent/'tools/npc_geometry_context_20261029/ff6_entry_native'

def main():
    expected=('0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd',
              '38aa9c54abe1c712ffd3c6e88a893ea6428f6df7001718b754b21afba7f4a61b')
    for folder in (ARCHIVE,ENTRY):
        provenance=json.loads((folder/'provenance.json').read_text())
        assert (provenance['dll_sha256'],provenance['dump_sha256'])==expected,folder
    bindings={0x25e8e10:('ChangeDistantViewSkyNormal',0x28a5e0,'背景変更：飛空艇（通常）'),
              0x25e8de0:('ChangeDistantViewSkyOther',0x28a710,'背景変更：飛空艇（特殊）')}
    metadata={}
    with (ROOT.parent/'ff6/script.json').open(encoding='utf-8-sig') as source:
        for line in source:
            match=re.search(r'"Address": (\d+)',line)
            if not match or int(match[1]) not in bindings:continue
            address=int(match[1]);record={}
            for _ in range(5):
                row=next(source)
                if '"Name"' in row:record['name']=json.loads(row.split(':',1)[1].rstrip(',\n'))
                if match:=re.search(r'"MethodAddress": (\d+)',row):record['rva']=int(match[1]);break
            name,rva,_=bindings[address]
            assert record=={'name':'Method$Last.Interpreter.Instructions.SystemCall.Current.'+name+'()','rva':rva}
            metadata[address]=record
            if len(metadata)==2:break
    assert len(metadata)==2
    cctor=(ENTRY/'native.txt').read_text(encoding='utf-8').splitlines()
    for address,(name,rva,label) in bindings.items():
        index=next(i for i,line in enumerate(cctor) if "; literal '"+label+"'" in line)
        load=next(line for line in reversed(cctor[max(0,index-12):index]) if 'mov r8, qword ptr [rip +' in line)
        match=re.match(r'([0-9A-F]+) mov r8, qword ptr \[rip \+ 0x([0-9a-f]+)\]',load)
        assert match and int(match[1],16)+7+int(match[2],16)-0x180000000==address,name
    native=(ARCHIVE/'native.txt').read_text(encoding='utf-8')
    facts=[]
    for name,pairs in (('ChangeDistantViewSkyNormal',[(82,1),(83,2),(83,3)]),
                       ('ChangeDistantViewSkyOther',[(82,2),(83,1),(83,3)])):
        body=re.search(r'^Last\.Interpreter\.Instructions\.SystemCall\.Current\.'+name+r'\(.*?\n(.*?)(?=\nLast\.|\Z)',native,re.M|re.S)[1]
        scenery=slot=None;actual=[]
        for line in body.splitlines():
            match=re.match(r'[0-9A-F]+ (\w+) (.*?)(?: ;.*)?$',line)
            if not match:continue
            op,args=match.groups()
            if op=='mov' and args.startswith('r9d, '):scenery=int(args.split(', ')[1],0)
            if op=='lea' and args.startswith('ecx, [r9 + '):slot=scenery+int(args.split(' + ')[1].rstrip(']'),0)
            if op=='call' and args=='0x1800e6ce0':actual.append((slot,scenery))
        assert actual==pairs,(name,actual)
        facts.append({'method':name,'scenery_calls':pairs,'body_sha256':hashlib.sha256(body.encode()).hexdigest()})
    provenance=json.loads((ARCHIVE/'provenance.json').read_text())
    assert len(provenance['functions'])==11
    output=ROOT/'build_check/opera_backdrop_contract.json'
    output.write_text(json.dumps({'dll_sha256':expected[0],'dump_sha256':expected[1],
                                 'native_functions':provenance['functions'],'bindings':metadata,'handlers':facts},ensure_ascii=False,indent=2)+'\n')
    print('Opera backdrop: two exact registrations, scenery slots82/83 and11 reviewed renderer bodies; no native code in repository')

if __name__=='__main__':main()
