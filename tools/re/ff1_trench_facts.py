"""Original factual locators for FF1's canal/ship gate; no native program output file."""
from pathlib import Path
import collections,csv,hashlib,json
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
def main():
 objects=[];scripts=[]
 for line in (DUMP/'ff1_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']=='Map_10010' and o['type_name']=='CollisionEntity':objects.append(dict(group=o['group'],entity=o['props'].get('entity_id',0),raw=o['name'],layer=o['layer'],cell=[o['tx'],o['ty']],properties=o['props']))
 for line in (DUMP/'ff1_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line)
  if not (r['submap']=='Map_10010' and r['name'].startswith('sc_map_') or r['name'].startswith(('sc_e_0015','sc_e_0004'))):continue
  body=r['mnemonics'];tests=[];flags=[];groups=[];calls=[]
  for m in body:
   op=m['mnemonic'];v=m.get('operands',{});iv=v.get('iValues',[]);sv=v.get('sValues',[])
   if op=='Branch':tests.append(dict(bank=sv[0],index=iv[0],comparison=sv[1],right_bank=sv[2],right_value=iv[1]))
   if op in ('SetFlag','ResetFlag'):flags.append(dict(bank=sv[0],index=iv[0],value=int(op=='SetFlag')))
   if op=='SetEntities':groups.append(sv[0])
   if op=='SysCall':calls.append(sv[0])
  scripts.append(dict(locator=r['submap']+'/'+r['name'],sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),state_predicates=tests,flag_assignments=flags,groups=sorted(set(groups)),native_calls=sorted(set(calls)),operation_counts=dict(collections.Counter(m['mnemonic'] for m in body))))
 report=dict(world_colliders=objects,scripts=scripts)
 (ROOT/'docs/re/ff1_trench_facts_20261101.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FF1 trench facts:',len(objects),'world collider declarations;',len(scripts),'exact script records; implementations remain external.')
 for r in scripts:print(r['locator'],'flags',r['flag_assignments'],'native calls',r['native_calls'])
if __name__=='__main__':main()
