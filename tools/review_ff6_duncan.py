"""Cabin flavor clues and exact native active-Sabin training availability."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
key=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
 'sc_e_0047':('ordinary native clue interaction','Kettle inspection sets33. Any one of33/34/35 enables the optional exit conversation36; these three clues are alternatives, not three ordered or jointly required puzzle operations.'),
 'sc_e_0048':('ordinary native clue interaction','Two shelf identities share one inspection34. Either shelf is the same optional clue, not a separate receipt or hidden switch. Any one clue enables exit dialogue36.'),
 'sc_e_0049':('ordinary native clue interaction','Flower inspection sets35 and optional native character staging. It is an alternative to the kettle/shelf clue, without a treasure gate, maze or independent puzzle sequence.'),
 'sc_e_0050':('ordinary native exit conversation','With any clue33/34/35, first exit plays the optional Sabin observation and sets36 before the real36/2 arrival. Without clues or after36 it remains the ordinary transfer. No all-three requirement or synthetic locked exit is added.'),
 'sc_e_0051':('ordinary native follow-up conversation','Outdoor old man appears only after36 while37/41 are unset, and his ordinary conversation sets37. Later Mt. Kolts progression41 retires this optional encounter; no independent player puzzle input is present.'),
 'sc_e_0954':('corrected native party-gated region','Later training region18 requires canonical Sabin6 in the active corps and unfinished464. The missing exact initializer branch now reads the existing bounded active-party pass; reserve-only/unknown membership grants no route. A single native entry performs the staged training and ability acquisition, so no invented multi-step puzzle guide is added.')}
def main():
 proof=json.loads((ROOT/'docs/re/duncan_native_20261104.json').read_text(encoding='utf-8'));rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if row['area_names_en']==["Duncan's Cabin"]:rows.setdefault(key(row),[]).append(row)
 assert len(rows)==7 and sum(map(len,rows.values()))==7
 authored={}
 for identity,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof[locator]['sha256']==wanted
  authored[identity]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=[],reason=reason,evidence='docs/re/DUNCAN_PARTY_GATE_20261104.md; exact source/one-record preservation, actual owner snapshot/read-count and training route/state checks.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  identity=key(row)
  if row['game']=='ff6'and identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Duncan:7 exact identities; alternative optional clues and one corrected native active-Sabin gate.')
if __name__=='__main__':main()
