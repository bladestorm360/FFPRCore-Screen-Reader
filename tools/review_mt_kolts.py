"""Exact remaining sightings,connected mandatory battle and full recruitment."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((R/'docs/re/mt_kolts_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff6_Mt_Kolts'and len(proof['source_script_hashes'])==51 and len(proof['actors'])==164
 external=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Mt_Kolts_Context/battle'
 native=json.loads((external.parent/'native_placement/provenance.json').read_text(encoding='utf-8'));assert native['dll_sha256']=='0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd'and len(native['functions'])==3 and native['functions']['Last.Interpreter.Instructions.Puppet.MoveEntityLayerWithWorldPosition(FieldEntity e, Vector3 p, int layer)']==0x2e8850
 for name,wanted in proof.get('mechanic_sources',{}).items():assert hashlib.sha256((R.parent/'tools/mapdump/ff6_master'/name).read_bytes()).hexdigest()==wanted
 for name,row in proof['battle_scripts'].items():
  body=(external/(name+'.json')).read_bytes();assert hashlib.sha256(body).hexdigest()==row['asset_sha256']
 ai=json.loads((external/'sc_ai_103_Vargas.json').read_text(encoding='utf-8'))['Mnemonics'];assert ai[7]['mnemonic']=='IsRecevieAbility'and ai[7]['operands']['iValues'][:2]==[132,54]and ai[56]['mnemonic']=='ModeSet'and ai[56]['operands']['iValues'][0]==3
 battle=json.loads((external/'sc_b_0055.json').read_text(encoding='utf-8'))['Mnemonics'];assert battle[145]['mnemonic']=='BattleForceExitOnSave'
 authored={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((R/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256']
  for row in d['candidates']:
   if row['area_names_en']!=['Mt. Kolts']:continue
   locator=row['submap']+'/'+row['script'];assert row['script']=='sc_e_0054'and row['entity']in(1,10);sha=d['script_facts'][locator]['sha256'];assert proof['source_script_hashes'][locator]==sha
   authored[identity(row)]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition='guided sightings and mandatory combat mechanic',guides=['mt_kolts_vargas'],reason='Both native sightings set40 and hide after their staged movement. The connected mandatory436 Vargas encounter requires Sabin’s Blitz ability132;the native visible tutorial specifies Raging Fist and Left/Right/Left/Confirm. Guidance covers the mountain approach,actual moving boss and full ready41 recruitment;the sighting is not completion or a new travel gate. Other-off blocks only the combat instruction,without skipping the earlier navigation stage.',evidence='docs/re/MT_KOLTS_VARGAS_20261104.md;51 exact field scripts/164 actors/three exact battle and AI assets,production routes/native-owner readiness.')
 assert len(authored)==2
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(R.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Mt. Kolts:two exact sightings/full mandatory Blitz/recruitment;no native geometry/gate change.')
if __name__=='__main__':main()
