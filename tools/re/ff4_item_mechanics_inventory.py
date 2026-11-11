"""Independent factual item-selector census; native bodies remain external."""
from pathlib import Path
import csv,hashlib,json
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
def main():
 maps={}
 for r in csv.DictReader((DUMP/'ff4_master/map.txt').open(encoding='utf-8-sig')):maps.setdefault(r['map_name'],[]).append(int(r['id']))
 names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/'ff4_master/script.txt').open(encoding='utf-8-sig'))}
 scripts={}
 for line in (DUMP/'ff4_scripts.jsonl').open(encoding='utf-8'):
  r=json.loads(line);body=r['mnemonics']
  if any(m['mnemonic']=='SysCall' and 'アイテム選択肢表示' in m.get('operands',{}).get('sValues',[''])[0] for m in body):
   items=sorted({m['operands']['iValues'][1] for m in body if m['mnemonic']=='Branch' and m.get('operands',{}).get('sValues',[''])[0]=='ItemSelection' and m['operands']['iValues'][1]>0})
   scripts[r['submap'],r['name']]=dict(submap=r['submap'],script=r['name'],maps=maps.get(r['submap'],[]),items=items,
    sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),owners=[])
 for line in (DUMP/'ff4_objects.jsonl').open(encoding='utf-8'):
  r=json.loads(line);p=r['props'];key=r['submap'],names.get(p.get('script_id'))
  if key in scripts:scripts[key]['owners'].append(dict(entity=p.get('entity_id'),raw=r['name'],group=r['group'],type=r['type_name']))
 rows=[r for r in scripts.values() if r['owners']]
 (ROOT/'docs/re/ff4_item_mechanics_inventory_20261101.json').write_text(json.dumps(dict(about='Independent native item-selector callers; factual identities/hashes, not automatic puzzle certification.',scripts=rows),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 for r in rows:print(r['maps'],r['script'],'items',r['items'],[(o['entity'],o['group'],o['type']) for o in r['owners']])
 print('FFIV exact item callers:',len(rows))
if __name__=='__main__':main()
