"""Current primary binding/writer census for Ghido; hashes and locators only."""
from pathlib import Path
import hashlib,json,re
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump';EXTERNAL=ROOT.parent/'tools/ff5_stone_20261102'
def digest(row):return hashlib.sha256(json.dumps(row,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def main():
 archive=json.loads((EXTERNAL/'programs.json').read_text(encoding='utf-8'));expected={(r['submap'],r['name']):digest(r)for r in archive};actual={};writers=[];absent=[];bits={174,175,176,177,178,179,294}
 assert all(r.get('mnemonics')for r in archive),'Required Ghido program body unavailable'
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
 assert actual==expected,'Ghido primary script binding changed'
 objects=json.loads((EXTERNAL/'objects.json').read_text(encoding='utf-8'));rooms={r['submap']for r in objects};primary=[]
 for line in(DUMP/'ff5_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if r['submap']in rooms:primary.append(r)
 assert sorted(map(digest,primary))==sorted(map(digest,objects)),'Ghido primary object binding changed'
 names={r['script']for r in writers};allowed={f'sc_e_0252_{i}'for i in(1,2,3,4,5,7,9)}|{'sc_e_0254'}
 facts=dict(programs=len(expected),objects=len(objects),flag_writers=writers,unexpected_writers=sorted(names-allowed),uncoded_records=absent,camera_dummy=0)
 (ROOT/'build_check/ff5_stone_primary_contract.json').write_text(json.dumps(facts,indent=2)+'\n')
 print('Ghido primary bindings:',len(expected),'programs/',len(objects),'objects; writer assets',sorted(names))
 print('Uncoded declarations:',absent)
 assert names==allowed,'Ghido flag writer census differs; inspect factual report'
 text=(ROOT.parent/'FF5/dump.cs').read_text(encoding='utf-8');assert re.search(r'kCameraDummyId\s*=\s*0;',text),'Camera dummy metadata changed'
 print('Reviewed Ghido producers match available primary bodies; both return resets and camera metadata retained. Uncoded declarations outside Ghido remain with broader semantic review. No source export or execution.')
if __name__=='__main__':main()
