"""Two exact controls,complete cave and both native rescue-return outcomes."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((R/'docs/re/veldt_cave_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==124 and len(proof['actors'])==444
 assert proof['facts']['Map_30761_4/sc_e_0743']['flags']==[[1,'ScenarioFlag3',21,1]]
 assert proof['facts']['Map_20461_6/sc_e_0744_6']['flags'][:1]==[[12,'ScenarioFlag2',116,1]]and proof['facts']['Map_20461_6/sc_e_0745_6']['flags'][:1]==[[12,'ScenarioFlag2',117,1]]
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((R/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Cave on the Veldt']:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==2 and sum(map(len,rows.values()))==2
 authored={}
 for key,variants in rows.items():
  row=variants[0];script=row['script'];locator=row['submap']+'/'+script;wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  reason='The automatic Interceptor entrance488/1 records371 and leaves the player in place. 'if script=='sc_e_0741'else'One ordinary switch491/21 records533, hides the switch and moves the blocking rock20 to43,11. '
  reason+='Native298 selects Shadow372 or Relm373. Initial rescue branches only pass491 during an uncontrollable sequence ending on311. The completed branch is the known-state ordinary repeat edge491/3 Upper. The physical rescue trigger remains reachable before completion.'
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition='ordinary native interaction',guides=[],reason=reason,evidence='docs/re/VELDT_CAVE_GUIDANCE_20261104.md;124 exact scripts/444 actors,both complete native return chains,production routes and native-owner completion checks.')
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
 print('Veldt cave:two exact controls;complete371/533/rescue/battle/voyage/dream/full372-or373 return;only native repeat is ordinary floor travel.')
if __name__=='__main__':main()
