"""Exact six native Locke locks; original navigation metadata, no game body."""
from pathlib import Path
import csv,hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
def author():
 import bake_maps
 objects=[]
 with(DUMP/'ff6_objects.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   o=json.loads(line)
   if o['submap']=='Map_20010':objects.append(o)
 names={int(r['id']):r['script_name']for r in csv.DictReader((DUMP/'ff6_master/script.txt').open(encoding='utf-8-sig'))}
 scripts={}
 with(DUMP/'ff6_scripts.jsonl').open(encoding='utf-8')as stream:
  for line in stream:
   s=json.loads(line)
   if s['submap']=='Map_20010'and s['name']in('sc_e_0867','sc_e_0868','sc_e_0869','sc_e_0870','sc_e_0871','sc_e_0872'):scripts[s['name']]=s
 infos=json.loads((DUMP/'ff6_submaps.json').read_text(encoding='utf-8'));ox,oy,*_=bake_maps.map_bounds(infos['Map_20010'],objects)
 proof=json.loads((ROOT/'docs/re/narshe_party_gates_native_20261103.json').read_text(encoding='utf-8'));rows=[]
 for index,entity in enumerate((115,116,117,118,119,114)):
  actor,=[o for o in objects if o['type_name']=='Entity'and o['props']['entity_id']==entity]
  props=actor['props'];name=names[props['script_id']];assert name=='sc_e_'+str(867+index).zfill(4)
  s=scripts[name];digest=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
  assert digest==proof['script_sha256']['Map_20010/'+name]
  assert props['collision']and props['action_id']==2
  rows.append(dict(id=entity,group=actor['group'],raw=actor['name'],
                   cell=[bake_maps.layer_index(actor['layer']),actor['tx']-ox,bake_maps.tile_row(actor)-oy],
                   receipt=411+index,character_status=2))
 return rows
if __name__=='__main__':
 rows=author();p=ROOT/'data/ff6/mapstate.json';d=json.loads(p.read_text(encoding='utf-8'))
 if'--check'in sys.argv:assert d['Map_20010'].get('party_gates')==rows
 else:d['Map_20010']['party_gates']=rows;p.write_text(json.dumps(d,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
 print('Narshe:six exact active-Locke locks,411..416; no consumables or native writes.')
