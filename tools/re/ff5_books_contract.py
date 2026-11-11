"""Exact Surgate primary bindings and available writer census; facts only."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump';EXTERNAL=ROOT.parent/'tools/ff5_surgate_20261102'
def digest(row):return hashlib.sha256(json.dumps(row,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def main():
 archive=json.loads((EXTERNAL/'programs.json').read_text(encoding='utf-8'));expected={(r['submap'],r['name']):digest(r)for r in archive};actual={};writers=[];absent=[];bits=set(range(521,527))
 required={'sc_map_20171_6','sc_map_20171_7','sc_e_0241','sc_e_0382','sc_e_0383','sc_e_0384','sc_e_0385','sc_e_0386'}|{f'sc_e_0387_{i}'for i in range(1,10)}
 assert required<={r['name']for r in archive if r.get('mnemonics')},'Required Surgate body unavailable'
 for line in(DUMP/'ff5_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line);identity=r['submap'],r['name']
  if identity in expected:actual[identity]=digest(r)
  written=set()
  if r.get('mnemonics')is None:absent.append(dict(submap=r['submap'],script=r['name'],error=r.get('error'),segments=r.get('segments')))
  for m in r.get('mnemonics')or[]:
   if m['mnemonic']not in('SetFlag','ResetFlag','Calc'):continue
   values=m['operands']['iValues'];strings=m['operands']['sValues']
   if strings and strings[0]in('ScenarioFlag1','ScenarioFlag2','ScenarioFlag3','ScenarioFlag4'):
    bit=values[0]+256*(int(strings[0][-1])-1)
    if bit in bits:written.add(bit)
  if written:writers.append(dict(submap=r['submap'],script=r['name'],flags=sorted(written),sha256=digest(r['mnemonics'])))
 assert actual==expected,'Surgate archived primary script binding changed'
 objects=json.loads((EXTERNAL/'objects.json').read_text(encoding='utf-8'));rooms={r['submap']for r in objects};primary=[]
 for line in(DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']in rooms:primary.append(r)
 assert sorted(map(digest,primary))==sorted(map(digest,objects)),'Surgate primary object binding changed'
 allowed={'sc_map_20171_7','sc_e_0241','sc_e_0382','sc_e_0383','sc_e_0384','sc_e_0385','sc_e_0386'}|{f'sc_e_0387_{i}'for i in range(2,9)};names={r['script']for r in writers}
 facts=dict(programs=len(expected),objects=len(objects),required_programs=len(required),flag_writers=writers,unexpected_writers=sorted(names-allowed),uncoded_records=absent)
 (ROOT/'build_check/ff5_books_primary_contract.json').write_text(json.dumps(facts,indent=2)+'\n')
 print('Surgate primary bindings:',len(expected),'programs/',len(objects),'objects;',len(required),'required bodies;',len(names),'available flag-writer assets. Uncoded declarations:',absent)
 assert names==allowed,'Surgate writer census differs; inspect factual report'
 print('Exact book/shelf/reset/librarian source bindings and available writers match; uncoded declarations outside this family remain with the broader review. No game/source execution or export.')
if __name__=='__main__':main()
