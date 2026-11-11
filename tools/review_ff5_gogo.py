"""Exact air identity plus independently discovered Gogo combat puzzle."""
from pathlib import Path
import csv,hashlib,json,sys
import pefile
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;P=R/'docs/re/ff5_gogo_native_20261104.json';proof=read(P);assert proof['scope_tag']=='ff5_Gogo'and len(proof['source_script_hashes'])==47 and len(proof['actors'])==59 and len(proof['native_functions'])==6
 dll=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/GameAssembly.dll');assert digest(dll)==proof['dll_sha256']and digest(R.parent/'FF5/dump.cs')==proof['dump_sha256'];pe=pefile.PE(str(dll),fast_load=True)
 for f in proof['native_functions']:assert hashlib.sha256(pe.get_data(f['rva'],f['bytes'])).hexdigest()==f['sha256']
 mechanic_files=('monster_party.txt','monster.txt','monster_ai_script.txt','ability.txt','script.txt','map.txt','map_script.txt','story_mes_en.txt');wanted={name:digest(D/'ff5_master'/name)for name in mechanic_files}
 if check:assert proof['mechanic_sources']==wanted
 else:assert 'mechanic_sources'not in proof or proof['mechanic_sources']==wanted;proof['mechanic_sources']=wanted
 E=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff5_Gogo_Context';aa=Path('D:/Games/SteamLibrary/steamapps/common/FINAL FANTASY V PR/FINAL FANTASY V_Data/StreamingAssets/aa/StandaloneWindows64')
 for name,row in proof['battle_scripts'].items():assert digest(E/(name+'.json'))==row['asset_sha256']and digest(aa/row['bundle'])==row['bundle_sha256']
 ai=read(E/'sc_ai_353_FamedMimicGogo.json')['Mnemonics'];assert len(ai)==148 and ai[37]['mnemonic']=='StartTable'and all(m['mnemonic']=='Act'and m['operands']['iValues'][0]==608 for m in ai[38:63])and ai[63]['operands']['iValues'][0]==635
 assert proof['battle_references']['489']==dict(script='sc_b_timer',monsters=[353])and proof['battle_references']['632']==dict(script='',monsters=[353])
 actual={};bodies={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:actual[k]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest();bodies[k]=s['mnemonics']
 assert actual==proof['source_script_hashes'];root=bodies['Map_30531_10/sc_e_0166'];end=bodies['Map_30531_10/sc_e_0166_1'];air=bodies['Map_30531_6/sc_e_0333'];outside=bodies['Map_30531_11/sc_map_30531_11']
 assert root[3]['operands']['iValues'][:3]==[0,0,6]and root[5]['operands']['iValues'][0]==489 and root[5]['operands']['sValues'][0]=='sc_e_0166_1'
 assert end[8]['mnemonic']=='SysCall'and end[8]['operands']['sValues'][0]=='ジョブ開放：ものまね師'and end[9]['mnemonic']=='SetFlag'and end[9]['operands']['iValues'][0]==166
 assert air[6]['operands']['iValues'][0]==91 and air[7]['mnemonic']=='ResetTimer'and outside[3]['mnemonic']=='FinishTimer'
 key=lambda r:(r['submap'],r['group'],r['entity'],r['raw']);candidates={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json','puzzle_mechanics_continuation_inventory_20261104.json'):
  inv=read(R/'docs/re'/name)['games']['ff5'];assert sources is None or sources==inv['source_sha256'];sources=inv['source_sha256']
  for r in inv['candidates']:candidates[key(r)]=r
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==sources[role]
 candidate,=[r for r in candidates.values()if r['submap']=='Map_30531_6'and r['entity']==3];a,=[a for a in proof['actors']if a['room']=='Map_30531_10'and a['entity']==8];independent=(a['room'],a['group'],a['entity'],a['raw']);assert independent not in candidates
 addition=dict(game='ff5',submap=a['room'],group=a['group'],entity=a['entity'],raw=a['raw'],script=a['script'],script_sha256=actual[a['room']+'/'+a['script']],guides=['gogo_sunken_tower'],discovery='Independent native special-battle/field callback review;not included in the three immutable map-mechanism inventories.',evidence='docs/re/FF5_GOGO_20261104.md;actual489/148-operation AI,full166 and native safe622 return.')
 if check:assert proof['additional_discoveries']==[addition]
 else:assert 'additional_discoveries'not in proof or proof['additional_discoveries']==[addition];proof['additional_discoveries']=[addition];P.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 authored=dict(game='ff5',**{k:candidate[k]for k in('submap','group','entity','raw','script')},script_sha256=actual[candidate['submap']+'/'+candidate['script']],disposition='guided timer-reset and special combat journey',guides=['gogo_sunken_tower'],reason='Actual air control records347 and resets the native seven-minute timer. Full guidance reaches its vine-only alcove,then Gogo8 on494;choose the second warning answer and take no battle actions. Native full Mime receipt166 changes the objective to the upper exit;only the full safe622 landing finishes. Two exact music-only projections and four full-player vine arrivals restore real reachability without opening terrain.',evidence='docs/re/FF5_GOGO_20261104.md;47 complete field scripts/59 actors/six exact native functions,real battle assets,all physical journeys/native owner/entry and preservation checks.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=False
 for row in old['decisions']:
  if row['game']=='ff5'and key(row)==key(authored):new['decisions'].append(authored);seen=True
  else:new['decisions'].append(row)
 if not seen:new['decisions'].append(authored)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Gogo:one original air identity and one independent combat discovery;actual air/reset,second choice,inaction/full166 and safe622 return;source/AI/native pins verified.')
if __name__=='__main__':main()
