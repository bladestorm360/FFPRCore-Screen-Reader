"""Five exact later-Opera controls;never replace the accepted earlier performance."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0893':('guided later optional dragon lever','Actual later lever3 is the correct optional dragon trigger. Before434 it falls through292 into boss369 and the full victory callback returns290/native60,44 Ground,records434 and runs shared dragon accounting. After434 its repeat is a different full scene ending290/native66,42 Ground. Only that completed repeat is a known-state manual travel arc;never ordinary access to the intermediate boss arena.'),
'sc_e_0892':('guided later operator','Actual operator9 moves one cell east and sets current-map local2. The guide reaches the real control-room arrival before reading this local,then talks once to clear the approach and warn about wrong switches. Leaving/reloading the floor requires fresh local state;no off-floor local is borrowed.'),
'sc_e_0335':('ordinary wrong-switch sound effect','Lever4 only plays the bark and its temporary animation. No flag,transfer or puzzle progress. Correct later guidance uses3 rather than treating this effect as an unlock.'),
'sc_e_0334':('ordinary wrong-switch round trip with recovery','Lever5 plays its stage cutscene on292 and returns293/player117,27 Ground,operator119,29/local2=1. It is not ordinary travel to292. The guide resumes at correct lever3 without repeating the already-set operator step.'),
'sc_e_0332':('preserved earlier stage-door switch counterpart','Later lever6 retains the original stage-door168/local1 behavior;it is not the later dragon solution. The accepted original performance on112–118 remains unchanged. Actual later guidance uses3 and does not repeat the earlier correct-performance lever as a new dragon prerequisite.')}
def main():
 proof=json.loads((R/'docs/re/late_opera_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff6_Late_Opera'and len(proof['source_script_hashes'])==55 and len(proof['actors'])==130
 archive=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Late_Opera_Context/native_move';native=json.loads((archive/'provenance.json').read_text(encoding='utf-8'));assert native['dll_sha256']=='0029a22ed933aa3b6ea3b1290060502267f514d61ba6e6619308d844557f2ffd'and len(native['functions'])==2
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((R/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==['Opera House']and row['submap']=='Map_20441_3':rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==5 and sum(map(len,rows.values()))==5
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=['late_opera_dragon']if row['script']in('sc_e_0892','sc_e_0893')else[],reason=reason,evidence='docs/re/LATE_OPERA_DRAGON_20261104.md;55 exact scripts/130 actors,two exact movement functions,full production routes and native-owner completion.')
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
 print('Later Opera:five exact controls;actual operator/correct3/full434,wrong-lever recovery and exact manual repeat;accepted early performance preserved.')
if __name__=='__main__':main()
