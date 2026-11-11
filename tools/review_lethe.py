"""One actual raft mechanic and two ordinary save tutorials,exact sources."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((R/'docs/re/lethe_native_20261104.json').read_text(encoding='utf-8'));assert proof['scope_tag']=='ff6_Lethe'and len(proof['source_script_hashes'])==418
 inv=json.loads((R/'docs/re/puzzle_mechanics_continuation_inventory_20261104.json').read_text(encoding='utf-8'))['games']['ff6'];rows=[r for r in inv['candidates']if r['area_names_en']==['Lethe River']];assert len(rows)==3
 authored={}
 for r in rows:
  sha=inv['script_facts'][r['submap']+'/'+r['script']]['sha256'];assert proof['source_script_hashes'][r['submap']+'/'+r['script']]==sha;raft=r['script']=='sc_e_0063'
  reason='Initial native raft confirmation/first fork,real346 landing,347 Up loop/Left exit,348 landing/final raft and actual474 boss/0065_1→584/0065_2/full50 are all covered. Actual choices use existing native open ownership,Navigation toggle and first/second answers;completed/unknown50 never arms a premature completion.'if raft else'Exact eleven-operation0009 body only flashes the save point,sets tutorial7 and optionally explains saving. It is an ordinary tutorial choice,not a puzzle answer or additional ordered progress step. Existing same-tile native SavePoint consolidation owns its category;the raft helper never answers rest-floor tutorial menus.'
  authored[identity(r)]=dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition='guided mechanic'if raft else'ordinary save-point tutorial',guides=['lethe_river']if raft else[],reason=reason,evidence='docs/re/LETHE_RIVER_20261104.md;complete418-body context/eleven recipe source pins,real physical routes,current owner/full50 and all-guide identities.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Lethe:one complete raft/fork/boss/full50 guide;two ordinary rest-floor tutorials stay distinct.')
if __name__=='__main__':main()
