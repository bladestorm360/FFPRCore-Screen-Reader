"""Finite primary-source census of FFIII Mini/Frog script predicates.

Emit only factual identities/hashes/owners, never native implementation bodies.
"""
from pathlib import Path
import csv,hashlib,json
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
CALLS={'状態異常：小人かどうか':401,'状態異常：カエルかどうか':402}
def main():
 maps={}
 for row in csv.DictReader((DUMP/'ff3_master/map.txt').open(encoding='utf-8-sig')):maps.setdefault(row['map_name'],[]).append(int(row['id']))
 names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff3_master/script.txt').open(encoding='utf-8-sig'))}
 rows=[];owners={}
 for line in (DUMP/'ff3_scripts.jsonl').open(encoding='utf-8'):
  row=json.loads(line);body=row['mnemonics']
  predicates=[dict(pc=i,condition=CALLS[m['operands']['sValues'][0]]) for i,m in enumerate(body)
   if m['mnemonic']=='SysCall' and m['operands'].get('sValues',[None])[0] in CALLS]
  if predicates:
   rows.append(dict(submap=row['submap'],script=row['name'],maps=maps.get(row['submap'],[]),predicates=predicates,
    sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
   owners[row['submap'],row['name']]=[]
 for line in (DUMP/'ff3_objects.jsonl').open(encoding='utf-8'):
  row=json.loads(line);props=row['props'];key=row['submap'],names.get(props.get('script_id'))
  if key in owners:owners[key].append(dict(entity=props.get('entity_id'),raw=row['name'],group=row['group'],type=row['type_name']))
 for row in rows:row['owners']=owners[row['submap'],row['script']]
 (ROOT/'docs/re/ff3_condition_mechanics_inventory_20261101.json').write_text(json.dumps(dict(about='Exact Mini/Frog SysCall census; factual discovery, not automatic guide completeness.',scripts=rows),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 for row in rows:print(row['maps'],row['script'],row['predicates'],row['owners'])
 print('FFIII condition census:',len(rows),'scripts;',sum(len(r['predicates']) for r in rows),'predicates.')
if __name__=='__main__':main()
