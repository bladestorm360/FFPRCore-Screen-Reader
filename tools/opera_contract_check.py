"""Pin reviewed Opera choice identities and failure branches to external evidence.

This is original verification code. No native body or lyric is copied into the
repository. The source archive and literal metadata remain outside every Git tree.
"""
import json
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
NATIVE=ROOT.parent/'tools/ff6_story_20261023/native.txt'
def main():
 lines=NATIVE.read_text(encoding='utf-8').splitlines()
 ins={int(m[1],16):(m[2],m[3].split(' ;')[0]) for line in lines
      if (m:=re.match(r'([0-9A-F]+) (\w+) (.*)',line))}
 literals={int(v['address'],16):v['value'] for v in
           json.loads((ROOT.parent/'ff6/stringliteral.json').read_text(encoding='utf-8-sig'))}
 def literal(rva):
  op,args=ins[0x180000000+rva]
  m=re.search(r'\[rip \+ 0x([a-f0-9]+)\]',args)
  assert op=='mov' and m,(hex(rva),'not a literal load')
  return literals[rva+7+int(m[1],16)]
 for q,(a,b,bad) in enumerate(((0xD1FBC4,0xD1FC08,0xD1FD06),
                             (0xD1FE7D,0xD1FEC1,0xD20156),
                             (0xD202E1,0xD20325,0xD2036E))):
  assert [literal(a),literal(b)]==[f'E0318_0{q}_118_c_01',f'E0318_0{q}_118_c_02']
  assert literal(bad)==f'E0318_0{q}_118_c_0{1 if q==1 else 2}'
 # Equality with each reviewed bad ID is the branch reaching the single shared
 # failure factory; inequality continues the performance. The middle question
 # takes its failure branch directly; the outer two first check animation time.
 for rva,op,dest in ((0xD1FD23,'je',0xD1FD9F),(0xD2016B,'jne',0xD20202),
                     (0xD20209,'jmp',0xD1FD36),(0xD2038E,'je',0xD203A6),
                     (0xD203A1,'jmp',0xD1FD36),(0xD1FD3C,'call',0xD18C60)):
  assert ins[0x180000000+rva]==(op,hex(0x180000000+dest)),hex(rva)
 provenance=json.loads((ROOT.parent/'tools/puzzle_audit_20261030/ff6_opera/provenance.json').read_text())
 assert provenance['dll_sha256']=='0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd'
 assert provenance['dump_sha256']=='38aa9c54abe1c712ffd3c6e88a893ea6428f6df7001718b754b21afba7f4a61b'
 print('Opera external source contract: three ordered two-option arrays, failure IDs/branches, correct positions1/2/1; native source remains external')
if __name__=='__main__':main()
