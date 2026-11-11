"""Factual all-six native layer-switch provenance; no recovered bodies in Git."""
import collections
import hashlib
import json
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def native_acceptance(n,text):
 dump=ROOT.parent/('FF4' if n==4 else 'FF5' if n==5 else f'ff{n}')/'dump.cs'
 source=dump.read_text(encoding='utf-8-sig')
 block=source[source.index('public class SwitchLayerEventEntity :'):]
 block=block[:block.index('// Properties')]
 offsets={name:int(re.search(r'\b'+name+r'; // 0x([0-9A-F]+)',block)[1],16)
          for name in ('targetEnterLayerList','targetStayLayerList','layerType')}
 match=re.search(r'^Last.Entity.Field.SwitchLayerEventEntity.Init\(.*?\n(.*?)(?=\nLast\.|\Z)',text,re.M|re.S)
 code=[]
 for line in match[1].splitlines():
  if m:=re.match(r'([0-9A-F]+) (\w+) (.*)',line):code.append((int(m[1],16),m[2],m[3].split(' ;')[0]))
 by_address={row[0]:i for i,row in enumerate(code)}
 # Start after the verified native property getter stores the target enum.
 start=next(i+1 for i,(_,op,args) in enumerate(code) if op=='mov' and
            re.fullmatch(r'dword ptr \[rbx \+ 0x'+f'{offsets["layerType"]:x}'+r'\], eax',args))
 rows=[]
 for target in range(3):
  pc=start;eax=target;edx=0;rcx='';zero=False;accepted={'enter':[],'stay':[]}
  for _ in range(128):
   _,op,args=code[pc];pc+=1
   if op=='test':zero=(eax==0) if args=='eax, eax' else False
   elif op=='sub' and args.startswith('eax, '):eax-=int(args.split(', ')[1],0);zero=eax==0
   elif op=='cmp' and args.startswith('eax, '):zero=eax==int(args.split(', ')[1],0)
   elif op=='mov' and args.startswith('edx, '):edx=int(args.split(', ')[1],0)
   elif op=='lea' and args.startswith('edx, [rax + '):edx=eax+int(args.split(' + ')[1].rstrip(']'),0)
   elif op=='mov' and args.startswith('rcx, qword ptr [rbx + '):
    off=int(args.split(' + ')[1].rstrip(']'),0)
    rcx='enter' if off==offsets['targetEnterLayerList'] else 'stay' if off==offsets['targetStayLayerList'] else ''
    assert rcx,'Unreviewed native list source'
   elif op in ('je','jne','jmp'):
    jump=op=='jmp' or (zero if op=='je' else not zero)
    if jump:
     address=int(args,16)
     if address in by_address:pc=by_address[address]
     else:
      assert op=='jmp' and rcx and edx in (9,10,11)
      accepted[rcx].append(edx-9);break
   elif op=='call':
    assert rcx and edx in (9,10,11),'Unexpected call in native acceptance tail'
    accepted[rcx].append(edx-9)
   elif op=='ret':break
  else:raise AssertionError('Native acceptance tail exceeded its bound')
  rows.append(accepted)
 expected=[{'enter':[],'stay':[1,2]},{'enter':[0],'stay':[2]},{'enter':[0,1],'stay':[]}]
 assert rows==expected,(n,rows)
 return rows

def main():
 result=[]
 for n in range(1,7):
  folder=ROOT.parent/'tools/puzzle_audit_20261030'/f'ff{n}_layer_contract'
  provenance=json.loads((folder/'provenance.json').read_text())
  text=(folder/'native.txt').read_text()
  methods={}
  for block in text.split('\nLast.Entity.Field.SwitchLayerEventEntity.')[1:]:
   title,*lines=block.splitlines();name=title.split('(')[0]
   start=int(re.search(r'RVA 0x([0-9a-f]+)',title)[1],16)+0x180000000
   normalized=[]
   for line in lines:
    m=re.match(r'([0-9A-F]+) (\w+) (.*)',line)
    if not m:continue
    op,args=m[2],m[3].split(' ;')[0]
    args=re.sub(r'\[rip [+-] 0x[0-9a-f]+\]','[rip]',args)
    if op=='call':args='external'
    elif op.startswith('j') and args.startswith('0x'):
     address=int(args,16);args=str(address-start) if start<=address<start+16384 else 'external'
    normalized.append(op+' '+args)
   methods[name]=dict(instructions=len(normalized),signature=hashlib.sha256('\n'.join(normalized).encode()).hexdigest())
  result.append(dict(game=n,inputs={k:provenance[k] for k in ('dll_sha256','dump_sha256')},methods=methods,
                     native_acceptance=native_acceptance(n,text)))
 for method in ('Init','OnTriggerEnterCustom','OnTriggerStayCustom','PerfectCollisionHit'):
  print(method,[(row['game'],row['methods'][method]['instructions'],row['methods'][method]['signature'][:16]) for row in result])
 # Detect genuine conflicts before choosing a metadata cell's target layer.
 import bake_maps
 for n in range(1,7):
  grouped=collections.defaultdict(set)
  for obj in map(json.loads,(ROOT.parent/f'tools/mapdump/ff{n}_objects.jsonl').read_text(encoding='utf-8').splitlines()):
   if obj['type_name']=='ToLayer':grouped[obj['submap'],obj['tx'],bake_maps.tile_row(obj)].add(obj['props']['layer'])
  conflicts={str(k):sorted(v) for k,v in grouped.items() if len(v)>1}
  assert not conflicts, f'ff{n}: conflicting native target layers at {conflicts}'
  print(f'ff{n}: {len(grouped)} layer-switch cells, {len(conflicts)} conflicting target cells')
  result[n-1]['conflicts']=conflicts
 (ROOT/'build_check/layer_contract.json').write_text(json.dumps(result,indent=2)+'\n')
 print('All six native Init tails: every other layer is accepted; ascent on enter, descent at exact-position stay')
if __name__=='__main__':main()
