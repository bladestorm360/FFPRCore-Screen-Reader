"""Read narrow external script families; emit factual metadata, never bodies.

Used for semantic review, not an automatic solution or completeness oracle.
Run through the finite resource runner. Arguments: ffN [script-prefix ...].
"""
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
game = sys.argv[1]
assert game in {f'ff{i}' for i in range(1,7)}
prefixes = sys.argv[2:]
source = ROOT.parent/'tools/mapdump'/f'{game}_scripts.jsonl'
for line in source.open(encoding='utf-8'):
    row=json.loads(line)
    if not any(row['name'].startswith(p) or row['submap']==p for p in prefixes):continue
    body=row['mnemonics']
    if not isinstance(body,list):print(row['submap'],row['name'],'unavailable');continue
    flags=[];locals_=[];calls=[];choices=[];maps=[];tests=[];continuations=[]
    for m in body:
        op=m['mnemonic'];v=m.get('operands',{});iv=v.get('iValues',[]);sv=v.get('sValues',[])
        if op in ('SetFlag','ResetFlag') and sv and iv:flags.append([op,sv[0],iv[0]])
        if op=='Calc' and sv and iv and sv[0] in ('MapLocalVariable','AreaLocalVariable'):locals_.append([sv,iv])
        if op=='SysCall' and sv:calls.append(sv)
        if op=='Select':choices.append([sv,iv])
        if op=='ChangeMap':maps.append(iv)
        if op=='Branch' and len(sv)>=3 and len(iv)>=2:
            tests.append(dict(left_bank=sv[0],left_index=iv[0],comparison=sv[1],right_bank=sv[2],right_value=iv[1]))
        if op=='ChangeScript' and sv:continuations.append(sv[0])
    print(json.dumps(dict(locator=row['submap']+'/'+row['name'],instructions=len(body),
        sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),
        flags=flags,local_assignments=locals_,native_calls=calls,choices=choices,maps=maps,
        state_tests=tests,continuations=continuations),ensure_ascii=False))
