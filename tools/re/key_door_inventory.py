"""Factual keyed-door declarations/callers only; no native bodies emitted."""
from pathlib import Path
import collections,csv,hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'
def main():
 result={}
 for game in ('ff1','ff3'):
  names={int(r['id']):r['script_name'] for r in csv.DictReader((DUMP/f'{game}_master/script.txt').open(encoding='utf-8-sig'))}
  rows=[];script_keys=set()
  for line in (DUMP/f'{game}_objects.jsonl').open(encoding='utf-8'):
   o=json.loads(line);p=o['props']
   if o['type_name'] not in ('Entity','AnimEntity','OpenTrigger','CollisionEntity'):continue
   if not (re.search('鍵|扉|ドア|door|lock|key',o['name'],re.I) or (game=='ff1' and o['submap']=='Map_20011_1')):continue
   script=names.get(p.get('script_id',0),'');script_keys.add((o['submap'],script))
   rows.append(dict(submap=o['submap'],group=o['group'],entity=p.get('entity_id'),raw=o['name'],type=o['type_name'],layer=o['layer'],cell=[o['tx'],o['ty']],props=p,script=script))
  calls=[]
  for line in (DUMP/f'{game}_scripts.jsonl').open(encoding='utf-8'):
   r=json.loads(line);body=r['mnemonics']
   if not isinstance(body,list):continue
   native=[m['operands'].get('sValues',[]) for m in body if m['mnemonic']=='SysCall']
   if (r['submap'],r['name']) not in script_keys and not any(any(re.search('鍵|扉|ドア|key|lock',str(s),re.I) for s in call) for call in native):continue
   # Retain semantic declarations, never raw operand arrays/jump targets or an
   # ordered implementation. Full primary bodies remain in the external dump.
   tests=[];receipts=[]
   for m in body:
    v=m['operands'];iv=v.get('iValues',[]);sv=v.get('sValues',[])
    if m['mnemonic']=='Branch' and len(iv)>=2 and len(sv)>=3:tests.append(dict(bank=sv[0],index=iv[0],comparison=sv[1],right_bank=sv[2],right_value=iv[1]))
    if m['mnemonic'] in ('SetFlag','ResetFlag') and iv and sv:receipts.append(dict(bank=sv[0],index=iv[0],value=int(m['mnemonic']=='SetFlag')))
   calls.append(dict(submap=r['submap'],script=r['name'],sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest(),operation_counts=dict(collections.Counter(m['mnemonic'] for m in body)),native_calls=[sv[0] for sv in native if sv],state_predicates=tests,receipts=receipts))
  result[game]=dict(objects=rows,callers=calls)
 path=ROOT/'docs/re/key_door_inventory_20261101.json';rendered=json.dumps(result,ensure_ascii=False,indent=2)+'\n'
 if '--check' in sys.argv:assert path.read_text(encoding='utf-8')==rendered,'Key-door source inventory changed'
 else:path.write_text(rendered,encoding='utf-8')
 print('Key-door declarations:',[(g,len(r['objects']),len(r['callers'])) for g,r in result.items()])
if __name__=='__main__':main()
