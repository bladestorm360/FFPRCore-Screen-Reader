"""Primary Ghido room/control facts; native programs stay outside Git."""
from pathlib import Path
import collections,csv,hashlib,json,subprocess,shutil,sys,contextlib,uuid
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
def main():
 folder=ROOT.parent/'tools/ff5_stone_20261102';folder.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[i]['map_name']:i for i in range(370,376)}
 names={int(r['id']):r['script_name']for r in csv.DictReader((DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig'))};objects=[];wanted=set()
 for line in(DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']not in rooms:continue
  objects.append(r);p=r['props'];script=names.get(p.get('script_id'),'')
  if script:wanted.add((r['submap'],script))
  if r['type_name']in('PointIn','GotoMap','Entity','NPC','TreasureBox','Event','CollisionEntity'):
   print('OBJECT',rooms[r['submap']],r['type_name'],p.get('entity_id'),r['layer'],(r['tx'],r['ty']),r['group'],r['name'],'script',script,'collision',p.get('collision'),'map/point',(p.get('map_id'),p.get('point_id')))
 rows=[]
 for line in(DUMP/'ff5_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']not in rooms:continue
  if (r['submap'],r['name'])not in wanted and not r['name'].startswith(('sc_map_','sc_e_0252_')):continue
  rows.append(r);main=next(s for s in r['segments']if s['Label']=='Main');count=main['Count'];start=main['EntryPoint']
  print('PROGRAM',rooms[r['submap']],r['name'],'Main',start,count,'sha',hashlib.sha256(json.dumps(r['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest())
  if r['name'].startswith(('sc_e_0252','sc_e_0254'))or r['name'].startswith('sc_map_'):
   for pc,m in enumerate(r['mnemonics'][start:start+count],start):
    if m['mnemonic']in('Branch','SetFlag','ResetFlag','Call','Jump','ChangeMap','SysCall','BranchByPlayerDirection','Select','SetEntities','SetPuppet','ExecPuppet','Calc','Exit'):
     print(pc,m['mnemonic'],m['operands']['iValues'][:4],m['operands']['sValues'][:3])
  if '--detail'in sys.argv and r['name']in('sc_e_0254','sc_e_0255','sc_e_0252_7','sc_e_0252_9'):
   print('DETAIL',r['name'])
   for pc,m in enumerate(r['mnemonics']):
    print(pc,m['mnemonic'],m['operands']['iValues'][:4],m['operands']['sValues'][:3])
 (folder/'objects.json').write_text(json.dumps(objects,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 old=folder/'programs_initial19_20261102.json'
 if(folder/'programs.json').exists()and not old.exists():shutil.copy2(folder/'programs.json',old)
 (folder/'programs.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('External primary Ghido archive',len(objects),'objects',len(rows),'programs; no game execution.')
if __name__=='__main__':
 # Native instructions must never escape into a caller's repository-local
 # redirected build log. Store inspection text directly in verified external
 # storage; stdout carries only its locator, never implementation content.
 folder=ROOT.parent/'tools/ff5_stone_20261102';folder.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 report=folder/('inspection_'+uuid.uuid4().hex+'.txt')
 with report.open('x',encoding='utf-8')as output,contextlib.redirect_stdout(output):main()
 print('Ghido primary inspection is external:',report,'; no native source on stdout or in Git.')
