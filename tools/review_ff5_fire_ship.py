"""Twelve exact identities;full native control/placement/return qualifications."""
from pathlib import Path
import hashlib,json,sys
import pefile
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;P=R/'docs/re/ff5_fire_ship_native_20261105.json';proof=read(P);assert proof['scope_tag']=='ff5_FireShip'and len(proof['source_script_hashes'])==171 and len(proof['actors'])==427
 position=read(R.parent/'tools/ff5_library_20261101/puppet_position_provenance.json');assert len(position['functions'])==4
 dll=Path('C:/Program Files (x86)/Steam/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==position['dll_sha256']and digest(R.parent/'FF5/dump.cs')==position['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in position['functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 if check:assert proof['position_native']==position
 else:assert 'position_native'not in proof or proof['position_native']==position;proof['position_native']=position;P.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 actual={};bodies={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:actual[k]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest();bodies[k]=s['mnemonics']
 assert actual==proof['source_script_hashes']
 for map in(19,21):
  body=bodies[f'Map_30151_{map}/sc_map_30151_{map}'];assert all(body[1+i]['mnemonic']=='ResetFlag'and body[1+i]['operands']['iValues'][0]==45+i and body[1+i]['operands']['sValues'][0]=='ScenarioFlag2'for i in range(7))
 assert len(bodies['Map_30151_6/sc_e_0216'])==37 and bodies['Map_30150/sc_e_0213'][26]['operands']['iValues'][0]==20
 master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if 'Fire-Powered Ship'in r['area_names_en']];assert len(selected)==12
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  sha=proof['source_script_hashes'].get(r['submap']+'/'+r['script']);guides=[]
  if r['submap']=='Map_30151_20':disposition='guided mechanic';guides=['fire_ship_passages'];reason='Eight solid controls/seven native bits and actual facing-dependent player carriers. Complete source-qualified policy covers all200 legal layouts,including the real269 reset/return and full onward271 arrival. Exact cardinal standing points prevent operating a carrier from the wrong side;only native script-ready receipts advance it.'
  elif r['script']=='sc_e_0216':disposition='ordinary native interaction';reason='Control13 hides itself and the door in one native operation. Its ordinary route and activation remain available.'
  elif not r['script']:disposition='ordinary scenery counterpart';reason='Scriptless Entity1 in the opened engine group is a visual counterpart,not a second operable switch.'
  elif r['script']=='sc_e_0054':disposition='ordinary native story interaction';reason='Full Cid scene automatically restores the Fire-Powered Ship through real world1 callback0054_1,sets53,places vehicle10 at84,79 and invokes native boarding. No manual puzzle answer or invented vehicle unlock.'
  else:assert r['script']=='sc_e_0213';disposition='ordinary native story interaction';reason='Complete Cid contact is automatic staging with native party/player restoration and final276 receipt. It has no player-selected code,switch order or special battle command.'
  authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=sha,disposition=disposition,guides=guides,reason=reason,evidence='docs/re/FF5_FIRE_SHIP_20261105.md;171 complete scripts/427 actors/four exact placement helpers;4096 native episodes,actual physical routes and native owner checks.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Ship:12 exact identities/eight-control complete policy/existing engine and two ordinary Cid scenes;171 bodies/427 actors/four actual native helpers verified.')
if __name__=='__main__':main()
