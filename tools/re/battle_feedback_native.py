"""Bounded offline evidence for MP drain, Swap and condition application.

Only the original extraction logic belongs in Git. Matching disassembly and
ABI/layout evidence stay outside every Git tree; no game code is executed.
"""
from pathlib import Path
import hashlib,json,re,sys,subprocess,bisect,struct
import pefile,capstone

ROOT=Path(__file__).resolve().parents[2]
number=int(sys.argv[1]) if len(sys.argv)>1 else 2
assert 1<=number<=6
folder=('ff1','ff2','ff3','FF4','FF5','ff6')[number-1]
title=('FINAL FANTASY','FINAL FANTASY II','FINAL FANTASY III','FINAL FANTASY IV','FINAL FANTASY V','FINAL FANTASY VI')[number-1]
dump=ROOT.parent/folder/'dump.cs'
dll=Path('D:/Games/SteamLibrary/steamapps/common')/(title+' PR')/'GameAssembly.dll'
out=ROOT.parent/'tools/battle_feedback_20261023'/f'ff{number}'
out.mkdir(parents=True,exist_ok=True)
assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
targets={
 'BattleFunction23':{'HitCalc','CreateViewEntity','UpdateParameterSub'},
 'AbsorptionFunction':{'Calc','CreateAttackCalc','UpdateParameter'},'MagicAbsorptionFunction':{'Calc','CreateAttackCalc','UpdateParameter'},
 'BattleFunction4FF2':{'CreateViewEntity','UpdateParameterSub'},
 'BattleBasicFunction':{'CreateDamageView','CreateViewEntity','CreateAfterViewEntity','CreateAttackViewEntity','CreateAfterAttackViewEntity','SetupValueToDisplay','UpdateParameter','UpdateParameterSub'},
 'CalcResult':{'GetHitType','GetSubHitType'},
 'BattleUtility':{'CreateDamageView','ChangeTemporaryParameter'},
 'BattleConditionController':{'Add','AddConditionFunction','CheckAddCondition','UpdateView'},
 'BattleConditionFunction':{'Init'},'BattleStatusControl':{'UpdateTempParamter'},
 'ConditionIconController':{'GetIcon'},
 'UniqueUpdateParameterProvider':{'HitCalc','UpdateParameter','CreateViewEntity','EndViewEntity','GetHitType','CreateDamageView'},
 'CalcExecuteFF2':{'UniqueExection'},
 'BattleFunction4':{'GetWearFunction'},
 'UniqueFunction':{'UpdateParameter'},
 'BattleFunction13':{'PrevCalc','HitCalc'},
 'BattleFunction13Serial':{'GetAbsorptionValues'},
}
if '--parasite-followup' in sys.argv:
 out=ROOT.parent/'tools/parasite_followup_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'BattleFunction13FF2':{'PrevCalc','HitCalc','Init','PhysicsAbs','MagicAbs'},
          'BattleFunction13FF3':{'PrevCalc','HitCalc','Init'},
          'FunctionProvider':{'GetFunctionClass'},
          'ParameterActFunctionManagment':{'GetFunction'}}
if '--three-parts' in sys.argv:
 out=ROOT.parent/'tools/parasite_three_parts_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'BattleBasicFunction':{'AfterViewDamage','ViewEntity','HitCalc','MainCalc','GetSumCalcResult','SetUpFunctions','SetupValueToDisplay','CreateAfterViewEntity','CreateAfterAttackViewEntity'},
          'FuncitonNormalAttack':{'Calc','UpdateParameter'},'CalcExecuteFF2':{'PhysicalExecution','AdditionalAttack'},
          'AbsorptionFunction':{'Calc','CreateAttackCalc'},
          'MagicAbsorptionFunction':{'Calc'},'BattleBaseFunction':{'SetFunction'},
          'CalcResult':{'GetHitType','GetSubHitType','GetValue','GetSubValue','Update'}}
if '--imbibe' in sys.argv:
 out=ROOT.parent/'tools/imbibe_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'SalmonConditionFunction':{'Start'},
          'BattleTempParameter':{'UpdateParameter','SetTempParameter'},
          'BattleConditionFunction':{'Init','GetParameterGroupDic'},
          'GroupMasterUtility':{'GetParameterDic'},
          'AddConditionFunction':{'Calc','UpdateParameter'},
          'AddConditionFunctionFF2':{'Calc','UpdateParameter'},
          'BattleConditionController':{'Add','AddConditionFunction'},
          'BattleBasicFunction':{'CreateViewEntity','ViewEntity'},
          'FunctionProvider':{'GetFunctionClass'}}
if '--stat-effects' in sys.argv:
 out=ROOT.parent/'tools/stat_effects_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'BattleConditionFunction':{'Init','SetParameterDic','GetParameterDic'},
          'BattleConditionController':{'Add','AddConditionFunction'},
          'BattleTempParameter':{'UpdateParameter'},
          'GroupMasterUtility':{'GetParameterDic'},
          'TempParameterFixedFunction':{'UpdateParameter'},
          'TempParameterRangeFunction':{'UpdateParameter'},
          'MultiPersistanceParameterFixedFunction':{'UpdateParameter'},
          'MultiPersistanceParameterRangeFunction':{'UpdateParameter'},
          'PersistanceParameterFixedFunction':{'UpdateParameter'},
          'PersistanceParameterRangeFunction':{'UpdateParameter'},
          'ParameterFixedCoefficientPhysicalFunction':{'Calc','UpdateParameter'},
          'BattleUtility':{'ChangeTemporaryParameter'},
          'BattleActExection':{'InitActionState','EndFinishingState'},
          'CharacterParameterBase':{'ConfirmedMaxHp','ConfirmedMaxMp'},
          'UniqueUpdateParameterProvider':{'UpdateParameter'},
          'BattleBasicFunction':{'UpdateParameter','CreateViewEntity'},
          'BerserkConditionFunction':{'Start'},'HasteConditionFunction':{'Start'},
          'SlowConditionFunction':{'Start'},'ProtectConditionFunction':{'Start'},
          'ShellConditionFunction':{'Start'},'PowerUpConditionFunction':{'Start'},
          'SongConditionFunction':{'Start','Update'},'SingingConditionFunction':{'Start','Update'},
          'TakenSongConditionFunction':{'Start','UpdateValue'},
          'SalmonConditionFunction':{'Start'}}
if '--key-help' in sys.argv:
 out=ROOT.parent/'tools/key_help_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'KeyHelpController':{'Initialize','SetValues','UpdateView','OnDestroy'},
          'KeyIconController':{'SetValues','UpdateView','Initialize'},
          'KeyIconView':{'SetText','SetMessage','SetActive','SetView'}}
if '--removals' in sys.argv or '--removal-callers' in sys.argv:
 out=ROOT.parent/'tools/status_removals_20261024'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'BattleConditionController':{'Remove','RemoveFunction','RemoveConditionFunction','CheckConditionFunction','Recovery','InterruptRemoveCondition','BattleEndRecoveryCondition'},
          'RecoveryConditionFunction':{'UpdateParameter'},'RecoveryConditionMultiFunction':{'UpdateParameter'},
          'ConditionUtility':{'Conflict','Negate','ResolvingConflict'}}
if '--hp-polarity' in sys.argv:
 out=ROOT.parent/'tools/hp_polarity_20261025'/f'ff{number}'
 out.mkdir(parents=True,exist_ok=True)
 assert subprocess.run(['git','-C',str(out),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 targets={'BattleBasicFunction':{'CreateDamageView'},'BattleUtility':{'CreateDamageView'},
          'ViewDamageEntity':{'CreateDamageValue'}}
names={};wanted={}
for block in dump.read_text(encoding='utf-8-sig').split('// Namespace: ')[1:]:
 ns=block.splitlines()[0]
 cls=re.search(r'^(?:public|private|internal).*?\b(?:class|struct) ([\w.<>]+)',block,re.M)
 if not cls:continue
 for match in re.finditer(r'^\s*// RVA: 0x([0-9A-F]+)[^\n]*\n\s*(?:public|private|protected|internal) [^\n]*? ([\w.<>]+)\(([^\n]*)\)',block,re.M):
  rva=int(match[1],16);label=ns+'.'+cls[1]+'.'+match[2]+'('+match[3]+')'
  names.setdefault(rva,[]).append(label)
  if ns.startswith('Last.Battle') or ns.startswith('Last.UI') or ns.startswith('Serial.') or (ns=='Last.Systems' and cls[1] in ('GroupMasterUtility','ConditionUtility')) or (ns=='Last.Data' and cls[1]=='CharacterParameterBase'):
   if match[2] in targets.get(cls[1],set()):wanted[label]=rva
literal_file=dump.parent/'stringliteral.json'
literals={int(row['address'],16):row['value'] for row in json.loads(literal_file.read_text(encoding='utf-8-sig'))}
ordered=sorted(names);pe=pefile.PE(str(dll),fast_load=True)
if '--damage-callers' in sys.argv or '--removal-callers' in sys.argv:
 # Exact direct-call xrefs only; validate instruction boundaries in each named
 # containing function before exporting its bounded body below.
 damage={rva for label,rva in wanted.items() if
         ('.BattleConditionController.RemoveFunction(' in label if '--removal-callers' in sys.argv
          else '.BattleUtility.CreateDamageView(BattleUnitData' in label)}
 for section in pe.sections:
  if not section.Characteristics&0x20000000:continue
  data=section.get_data()
  for i in range(len(data)-5):
   if data[i]!=0xe8:continue
   rva=section.VirtualAddress+i
   if rva+5+struct.unpack_from('<i',data,i+1)[0] not in damage:continue
   start=ordered[bisect.bisect_right(ordered,rva)-1]
   if rva-start>32768:continue
   decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64)
   if any(ins.address==rva and ins.mnemonic=='call' for ins in decoder.disasm(pe.get_data(start,rva-start+5),start)):
    wanted['XREF '+names[start][0]]=start
disasm=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);lines=[]
for label,rva in wanted.items():
 end=next((value for value in ordered if value>rva),rva+16384)
 lines.append('\n'+label+' RVA '+hex(rva))
 for instruction in disasm.disasm(pe.get_data(rva,min(end-rva,32768)),pe.OPTIONAL_HEADER.ImageBase+rva):
  note=''
  if instruction.mnemonic in ('call','jmp') and instruction.op_str.startswith('0x'):
   aliases=names.get(int(instruction.op_str,16)-pe.OPTIONAL_HEADER.ImageBase,[])
   if aliases:note=' ; '+(' / '.join(aliases) if len(aliases)<5 else str(len(aliases))+' folded aliases')
  rip=re.search(r'\[rip \+ (0x[0-9a-f]+)\]',instruction.op_str)
  if rip:
   literal=literals.get(instruction.address+instruction.size+int(rip[1],16)-pe.OPTIONAL_HEADER.ImageBase)
   if literal is not None:note+=' ; literal '+repr(literal)
  lines.append(f'{instruction.address:X} {instruction.mnemonic} {instruction.op_str}{note}')
(out/'native.txt').write_text('\n'.join(lines),encoding='utf-8')
(out/'provenance.json').write_text(json.dumps({'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'dump_sha256':hashlib.sha256(dump.read_bytes()).hexdigest(),'functions':wanted},indent=2)+'\n')
print(f'FF{number}: {len(wanted)} bounded native functions, evidence outside Git at {out}')
