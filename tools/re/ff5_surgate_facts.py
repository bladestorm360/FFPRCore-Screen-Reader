"""Exact Surgate book/control investigation; all native text stays external."""
from pathlib import Path
import contextlib,csv,json,subprocess,uuid,sys
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
def collect(folder):
 maps={int(r['id']):r for r in csv.DictReader((DUMP/'ff5_master/map.txt').open(encoding='utf-8-sig'))};rooms={maps[i]['map_name']:i for i in range(153,162)}
 names={int(r['id']):r['script_name']for r in csv.DictReader((DUMP/'ff5_master/script.txt').open(encoding='utf-8-sig'))};objects=[];wanted=set()
 for line in(DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']not in rooms:continue
  objects.append(r);p=r['props'];script=names.get(p.get('script_id'),'')
  if script:wanted.add((r['submap'],script))
  if r['type_name']in('Entity','NPC','Event','CollisionEntity','GotoMap','PointIn'):
   print('OBJECT',rooms[r['submap']],r['type_name'],p.get('entity_id'),r['layer'],(r['tx'],r['ty']),r.get('w'),r.get('h'),r['group'],r['name'],'script',script,'action',p.get('action_id'),'accept',p.get('accept_action_direction'),'map/point',(p.get('map_id'),p.get('point_id')))
 rows=[]
 for line in(DUMP/'ff5_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']not in rooms or((r['submap'],r['name'])not in wanted and not r['name'].startswith('sc_map_')):continue
  rows.append(r);print('PROGRAM',rooms[r['submap']],r['name'],'segments',r['segments'])
  if r['name']in('sc_e_0241','sc_e_0242','sc_e_0383','sc_e_0385','sc_e_0387_1'):
   for pc,m in enumerate(r.get('mnemonics')or[]):print(pc,m['mnemonic'],m['operands']['iValues'][:5],m['operands']['sValues'][:4])
 for name,rows in(('objects.json',objects),('programs.json',rows)):
  path=folder/name;assert not path.exists(),'Reuse existing exact archive, do not overwrite investigation'
  path.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Surgate primary archive',len(objects),'objects;',len(wanted),'script bindings.')
def main():
 folder=ROOT.parent/'tools/ff5_surgate_20261102';folder.mkdir(exist_ok=True)
 assert subprocess.run(['git','-C',str(folder),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
 report=folder/('inspection_'+uuid.uuid4().hex+'.txt')
 with report.open('x',encoding='utf-8')as output,contextlib.redirect_stdout(output):
  if '--detail'in sys.argv:
   for r in json.loads((folder/'programs.json').read_text(encoding='utf-8')):
    if r['name']not in('sc_e_0382','sc_e_0384','sc_e_0386','sc_e_0387_2','sc_e_0387_9','sc_map_20171_7'):continue
    print('PROGRAM',r['name'],'segments',r['segments'])
    for pc,m in enumerate(r.get('mnemonics')or[]):print(pc,m['mnemonic'],m['operands']['iValues'][:5],m['operands']['sValues'][:4])
  else:collect(folder)
 print('Surgate primary inspection stored outside Git:',report,'; no native source on stdout.')
if __name__=='__main__':main()
