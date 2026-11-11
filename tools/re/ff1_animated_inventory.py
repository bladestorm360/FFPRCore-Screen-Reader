"""Factual FF1 animated-object identities; no native programs are copied."""
from pathlib import Path
import csv,hashlib,json
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'

def main():
 maps={}
 for row in csv.DictReader((DUMP/'ff1_master/map.txt').open(encoding='utf-8-sig')):
  maps.setdefault(row['map_name'],[]).append(int(row['id']))
 scripts={int(row['id']):row['script_name'] for row in csv.DictReader((DUMP/'ff1_master/script.txt').open(encoding='utf-8-sig'))}
 rows=[]
 for line in (DUMP/'ff1_objects.jsonl').open(encoding='utf-8'):
  obj=json.loads(line)
  if obj['type_name']!='AnimEntity':continue
  props=obj['props'];sid=props.get('script_id',0)
  rows.append(dict(submap=obj['submap'],maps=maps.get(obj['submap'],[]),entity=props.get('entity_id',0),group=obj['group'],raw=obj['name'],layer=obj['layer'],cell=[obj['tx'],obj['ty']],script=scripts.get(sid,''),action=props.get('action_id',0),collision=bool(props.get('collision',False))))
 transfers=[]
 for line in (DUMP/'ff1_scripts.jsonl').open(encoding='utf-8'):
  script=json.loads(line)
  if (script['submap'],script['name']) not in {('Map_20130','sc_e_0031'),('Map_30081_3','sc_e_0032')}:continue
  body=script['mnemonics'];destinations=[m.get('operands',{}).get('iValues',[])[:2] for m in body if m['mnemonic']=='ChangeMap']
  transfers.append(dict(submap=script['submap'],script=script['name'],destinations=destinations,native_calls=sorted({m.get('operands',{}).get('sValues',[''])[0] for m in body if m['mnemonic']=='SysCall'}),sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
 report=dict(source_sha256=hashlib.sha256((DUMP/'ff1_objects.jsonl').read_bytes()).hexdigest(),objects=rows,submarine_transfers=transfers)
 (ROOT/'docs/re/ff1_animated_inventory_20261101.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FF1 animated source census:',len(rows),'objects across',len({r['submap'] for r in rows}),'submaps.')
 print('Submarine transfer facts:',json.dumps(transfers,ensure_ascii=False))
 for row in rows:
  if row['script'] or 'クリスタル' in row['raw']:print(json.dumps(row,ensure_ascii=False))

if __name__=='__main__':main()
