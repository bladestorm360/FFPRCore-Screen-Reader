"""New exact shelf identities reuse the accepted,unchanged Surgate solution."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];E=R.parent/'tools/ff5_surgate_20261102'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
body_sha=lambda s:hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
def main():
 names=('src/games/ff5/book_model.cpp','src/games/ff5/book_model.h','tools/author_ff5_books.py','tools/ff5_books_route_check.cpp','../tools/ff5_surgate_20261102/programs.json','../tools/ff5_surgate_20261102/objects.json','../tools/ff5_surgate_20261102/comparison_programs.json')
 evidence=R/'docs/re/surgate_continuation_evidence_20261104.json'
 if '--record-evidence'in sys.argv:
  frozen=R/'build_check/ff5_books_20261102';receipt=json.loads((frozen/'completion.json').read_text(encoding='utf-8'));inputs=json.loads((frozen/'inputs.json').read_text(encoding='utf-8'))
  for name,wanted in receipt['reports'].items():assert sha(frozen/name)==wanted
  record=dict(scope='Surgate continuation identities;reuse accepted November2 proof,not a new qualification run',episodes=receipt['episodes'],manual_operations=receipt['manual_operations'],shelf_contacts=receipt['shelf_contacts'],wrong_choices=receipt['wrong_choices'],native_hops=receipt['native_hops'],resets=receipt['resets'],locales=receipt['locales'],archived_report_sha256=receipt['reports'],inputs={n:inputs[n.replace('/','\\')]for n in names})
  assert all(sha(R/n)==v for n,v in record['inputs'].items())
  evidence.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
 # The durable factual record avoids making a normal canonical check depend
 # on ignored build_check files. Frozen reports were hash-verified at capture;
 # current source/primary archives must continue to match their qualified input.
 record=json.loads(evidence.read_text(encoding='utf-8'));assert(record['episodes'],record['manual_operations'],record['wrong_choices'],record['locales'])==(148,974,27,12)and set(record['inputs'])==set(names)
 for name,wanted in record['inputs'].items():assert sha(R/name)==wanted,('Accepted Surgate input changed',name)
 programs=json.loads((E/'programs.json').read_text(encoding='utf-8'));sources={(s['submap'],s['name']):s for s in programs};objects=json.loads((E/'objects.json').read_text(encoding='utf-8'));assert len(programs)==54 and len(objects)==302
 inv=json.loads((R/'docs/re/puzzle_mechanics_continuation_inventory_20261104.json').read_text(encoding='utf-8'))['games']['ff5'];rows=[r for r in inv['candidates']if r['area_names_en']==['Surgate Castle']];assert len(rows)==25
 guide=json.loads((R/'data/ff5/puzzles.json').read_text(encoding='utf-8'))['book_guide'];correct={r['entity']for book in guide['books']for r in book['shelves']};assert correct=={21,35,36,22,50,23,51,52}
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);authored={};identity=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw'])
 for row in rows:
  assert row['submap']=='Map_20171_7';s=sources[row['submap'],row['script']];digest=body_sha(s);assert digest==inv['script_facts'][row['submap']+'/'+row['script']]['sha256']
  obj,=[o for o in objects if(o['submap'],o['group'],o['props'].get('entity_id'),o['name'])==(row['submap'],row['group'],row['entity'],row['raw'])];assert obj['type_name']==row['type']and obj['props']['action_id']==row['action']
  good=row['entity']in correct
  assert row['script']in({'sc_e_0382','sc_e_0384','sc_e_0386'}if good else{'sc_e_0387_'+str(i)for i in range(2,9)})
  reason=('This is one of the eight actual correct shelf-facing contacts for the three-book arrangement. The current guide targets it using native North-only facing and Yes0;all correct placement/carried subsets and final librarian/wall receipt are already covered.'if good else'This is an actual wrong-choice shelf,not scenery. The exact seven wrong-shelf bodies clear carried receipts without placement or restoration. The accepted guide detects lost-book state and routes through the actual exit/re-entry reset;its frozen27 wrong-choice and28 reset witnesses cover this same native behavior.')
  out=dict(game='ff5',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=digest,disposition='guided mechanic'if good else'wrong-choice mechanic with guided recovery',guides=['surgate_books'],reason=reason+' Existing Other toggle,Minimal/entry notice and all12 locales remain intact;no production change.',evidence='docs/re/FF5_SURGATE_BOOKS_20261102.md;unchanged54-program/302-object source contract;148 episodes/974 operations/419 contacts/27 wrong choices;current shared-owner regression retains this leaf.')
  assert identity(out)not in authored;authored[identity(out)]=out
 seen=set();new['decisions']=[]
 for row in old['decisions']:
  key=identity(row)
  if key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Surgate continuation review:25 exact contacts/eight correct+17 wrong;accepted source/model/resources/native comparison evidence unchanged;no reopened design or qualification loop.')
if __name__=='__main__':main()
