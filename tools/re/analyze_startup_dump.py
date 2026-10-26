"""Offline minidump register/stack candidates, never a live process attachment.

Candidate return addresses are not an unwind or proof of a call chain. Keep the
snapshot and any native-memory contents outside Git; export only factual locators.
"""
from pathlib import Path
import struct,json,sys,bisect
import pefile

directory=Path(sys.argv[1]).resolve()
receipt=json.loads((directory/'capture.json').read_text(encoding='utf-8-sig'))
raw=Path(receipt['dump']).read_bytes()
assert raw[:4]==b'MDMP'
def u32(at):return struct.unpack_from('<I',raw,at)[0]
def u64(at):return struct.unpack_from('<Q',raw,at)[0]
streams={u32(p):(u32(p+4),u32(p+8)) for p in range(u32(12),u32(12)+12*u32(8),12)}
mods=json.loads((directory/'modules.json').read_text(encoding='utf-8-sig'))
def module(address):return next((m for m in mods if m['base']<=address<m['base']+m['ModuleMemorySize']),None)
game=next(m for m in mods if m['ModuleName']=='GameAssembly.dll')
metadata=json.loads((Path(__file__).resolve().parents[3]/'ff3/script.json').read_text(encoding='utf-8-sig'))
names={}
for row in metadata['ScriptMethod']:names.setdefault(row['Address'],[]).append(row['Name'])
ordered=sorted(names)
def label(address):
    m=module(address)
    if not m:return hex(address)
    rva=address-m['base'];s=m['ModuleName']+'+'+hex(rva)
    if m==game:
        index=bisect.bisect_right(ordered,rva)-1
        if index>=0 and rva-ordered[index]<0x10000:s+=' '+','.join(names[ordered[index]][:3])+'+'+hex(rva-ordered[index])
    return s
images={}
def call_before(address,m):
    # Conservative direct-call return-site check; indirect calls are omitted.
    key=m['FileName']
    if key not in images:
        try:images[key]=pefile.PE(key,fast_load=True)
        except (OSError,pefile.PEFormatError):images[key]=None
    pe=images[key]
    if not pe:return False
    try:return pe.get_data(address-m['base']-5,1)==b'\xe8'
    except pefile.PEFormatError:return False
at=streams[3][1];threads=[]
for offset in range(at+4,at+4+u32(at)*48,48):
    tid=u32(offset);stack_address=u64(offset+24);size=u32(offset+32);stack_rva=u32(offset+36);ctx=u32(offset+44)
    rsp=u64(ctx+152);rip=u64(ctx+248);candidates=[]
    for i in range(0,size-7,8):
        value=u64(stack_rva+i);m=module(value)
        if m and call_before(value,m):candidates.append({'slot':hex(stack_address+i),'return':label(value)})
    row={'id':tid,'rip':label(rip),'rsp':hex(rsp),'stack_bytes':size,'direct_call_candidates':candidates[:36]};threads.append(row)
(directory/'thread_candidates.json').write_text(json.dumps(threads,indent=2)+'\n')
for row in threads:
    relevant=[x for x in row['direct_call_candidates'] if 'GameAssembly.dll+' in x['return'] or 'HID.DLL+' in x['return']]
    if relevant or row['id']==4020:
        print(json.dumps({'id':row['id'],'rip':row['rip'],'candidates':relevant[:12]}))
print(str(len(threads))+' thread contexts parsed; candidate addresses are not unwound stacks.')
