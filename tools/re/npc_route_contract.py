"""Verify the six exact native instruction-controller contracts offline."""
from pathlib import Path
import hashlib,json,re
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT.parent/'tools/npc_geometry_context_20261029'
result=[]
for n in range(1,7):
 source=(OUT/f'ff{n}_native/native.txt').read_text()
 provenance=json.loads((OUT/f'ff{n}_native/provenance.json').read_text())
 prefix='\nLast.Map.FieldNonPlayerRouteMoveInstructionController.'
 def body(method):return source.split(prefix+method)[1].split('\n\n')[0]
 moving=body('get_IsMovingType()');ctor=body('.ctor(');destroy=body('OnDestroy()');supported=n in (3,5,6)
 assert re.search(r' mov al, 1\n',moving),(n,'native moving return contract')
 writes=re.findall(r'mov dword ptr \[rdx \+ (0x[0-9a-f]+)\], 0x66',ctor)
 assert bool(writes)==supported,(n,'active MoveType102 assignment')
 if supported:
  assert len(writes)==1 and ' + 0x88]'in ctor and ' + 0x88]'in destroy
  assert re.search(r'mov dword ptr \[[^\]]+ \+ '+writes[0]+r'\], r10d',destroy),(n,'restore original type')
  advance=body('NextInstruction()');update=body('UpdateController(')
  # Completion clears the instruction pointer, not the persistent102 marker.
  # IsMovingType remains constant true even in that completed state. Preserve
  # exact locators/raw bodies outside Git; these are evidence assertions only.
  for code in (advance,update):
   assert '0x90]'in code and '0x84]'in code and '0x8c]'in code,(n,'instruction lifecycle fields')
   assert re.search(r'mov qword ptr \[[^\]]+\], 0\n',code),(n,'completion pointer clear')
   assert '.OnDestroy('not in code,(n,'completion does not call destruction')
 script_file=ROOT.parent/'tools/mapdump'/f'ff{n}_scripts.jsonl';assignments=0
 with script_file.open(encoding='utf-8-sig')as file:
  for line in file:assignments+=sum(m['mnemonic']=='AssignMoveRoute'for m in json.loads(line).get('mnemonics')or[])
 if n in (1,2,4):assert assignments==0,(n,'older source command applicability',assignments)
 if n in (5,6):assert assignments>0,(n,'assigned patrol source usage',assignments)
 result.append(dict(game=n,moving=True,assigns_active_type102=supported,restores_original_type=supported,
                    completion_requires_current_instruction=supported,
                    source_assignments=assignments,script_source_sha256=hashlib.sha256(script_file.read_bytes()).hexdigest(),
                    property_move_type_offset=writes[0]if writes else None,**provenance))
 print(f'FF{n}: controller moving=True; active102/restore={bool(writes)}; source AssignMoveRoute count={assignments}')
(OUT/'native_route_contract.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print('All-six native contracts verified; no game run or source/body copied into Git')
