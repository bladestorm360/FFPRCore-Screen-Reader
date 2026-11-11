"""Sixteen exact remaining Imperial travel/story-control discoveries."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0373':('complete navigation puzzle',['vector_infiltration'],'Actual first-option Yes sets local1 and exposes the crate. Refusal supplies no receipt; guard aftermath resets local1/523. Full guide continues through the real crate landing and first factory entrance.'),
'sc_e_0374':('complete manual navigation puzzle',['vector_infiltration'],'Full player choreography ends at57,34 Ground, not an intermediate Upper pose, and sets523. One exact same-map graph companion preserves the source control and native initializer conditions; full guide reaches396.'),
'sc_e_0405':('ordinary staged story reunion',[],'Three physical aliases own the same automatic Setzer/reunion scene. Native178/179/119/world-camera/395/155/156 continuations lead into controllable Esper World after195. No free general world edge or completed Terra-quest receipt is invented; the Esper World control is a separate frozen discovery.'),
'sc_e_0442':('ordinary party gate',[],'Exact initializer checks active canonical Terra1 and more than one current member before217, including repeat visits until225. Qualified parties select the five allowed contacts. Soldier, missing-Terra and solo variants continue to block onward routing.'),
'sc_e_0426':('native local pushback obstacle',[],'The active guard trigger knocks the player to43,23 or45,23. Its exact physical footprint is a route barrier even with NPC avoidance off. A real alternate port route remains available; native257/284 remove this local group. Neither whole-port closure nor an unlocking puzzle is invented.'),
'sc_e_0512':('ordinary native voyage preparation',[],'Leo intro263 and optional Shadow naming28 are ordinary dialogue. Actual inn-rest continuation264, ship arrival265 and night scene266 precede the later Leo conversation267 and sailing268. Distinct variants and their native arrivals are preserved; approaching this NPC cannot grant later voyage receipts or a free travel edge.'),
'sc_e_0517':('ordinary native voyage conversation',[],'Native ship variant623 appears through the actual265 arrival, night scene266 selects Leo, and conversation sets267. The full following0518/world continuation owns268 after Shadow joins. Repeat267 gives ordinary dialogue; no coin/answer puzzle or general transfer edge is inferred.'),
'sc_e_0347':('ordinary automatic negotiation scene',[],'Both exact captain aliases execute automatic coin negotiation; the complete script has no Select. Party composition changes dialogue, not a Heads/Tails player solution. Full119/575/world1/24 continuation owns178; mere approach does not grant airship ownership or a fixed travel edge.'),
'sc_e_BJoutboard':('native manual dynamic-position flight',[],'Fly resumes at actual saved Blackjack4/world1 pose, never fallback World PointIn1/Narshe. Exact owned-worker continuation plus physical control approach retain final destination and manual first/second Fly option across284. Guest branch uses native SetVehicle22,93 without a menu. Unknown ownership/pose cannot certify travel. Floating Continent option remains its separate accepted guide and exact-three-member native party-selection mechanic.')}

def main():
 proof=json.loads((ROOT/'docs/re/imperial_travel_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==119 and len(proof['actors'])==233
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  data=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==data['source_sha256'];sources=data['source_sha256'];facts.update(data['script_facts'])
  for row in data['candidates']:
   if set(row['area_names_en'])&{'Vector','Imperial Observation Post','Albrook','Blackjack'}:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==16 and sum(map(len,rows.values()))==16
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,guides,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=guides,reason=reason,evidence='docs/re/IMPERIAL_TRAVEL_20261104.md;119 native scripts,nine exact native functions,current owned-reader/controller/route consumers and preservation receipt.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Imperial:16 exact discoveries;complete Vector/party guides,strict factory variants,local Albrook pushback and saved-pose manual flight;ordinary voyage/coin/reunion remain separate.')
if __name__=='__main__':main()
