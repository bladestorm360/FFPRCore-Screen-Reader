"""Seven remaining exact identities; ordinary bids/art and independently gated pickups."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
def main():
 proof=json.loads((R/'docs/re/zozo_jidoor_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['source_script_hashes'])==166 and len(proof['actors'])==755
 assert len(proof['auction_contracts'])==11 and len({a['locator'].partition('/')[2]for a in proof['auction_contracts']})==9
 assert all(a['ordinary_displayed_bid_choices']and a['no_map_or_navigation_unlock_effect']for a in proof['auction_contracts'])
 lamp=proof['lamp_contract'];assert(lamp['action'],lamp['handler'],lamp['script_id'],lamp['link'],lamp['default_character_status_change'])==(4,'Talk',0,0,0)
 native=lamp['native_provenance'];assert len(native['functions'])==4
 archive=R.parent/'tools/puzzle_audit_20261030/broader_audit_20261104/ff6_Zozo_Jidoor_Context/native_lamp'
 assert hashlib.sha256((archive/'native.txt').read_bytes()).hexdigest()==lamp['native_export_sha256']
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  d=json.loads((R/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
  for row in d['candidates']:
   if(row['submap']=='Map_20151_8'and row['entity']==14)or row['script']=='sc_e_0422'or(row['submap']=='Map_30511_12'and row['script']in('sc_e_0303','sc_e_0304','sc_e_0305','sc_e_0306')):rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==7 and sum(map(len,rows.values()))==7
 authored={}
 for key,variants in rows.items():
  row=variants[0];script=row['script'];wanted=facts.get(row['submap']+'/'+script,{}).get('sha256')
  if not script:kind='ordinary message-only artwork';reason='Earlier mansion611 lamp14 uses actual action4/Talk,script0,link0,default character-status change0 and a literal lamp description. Four exact FFVI native handlers confirm message-only ownership. The later442 light switch belongs to the accepted complete Owzer guide;this earlier painting is not that mechanism and is not a missing entrance.'
  elif script=='sc_e_0422':
   kind='ordinary optional auction shopping';reason='Complete nine-body/eleven-variant dispatch:actual203 opening/current local1,random lots,displayed participation/bid/decline choices and conditional gil payment. Paid lots record purchase receipts;some lots are intentionally won by another bidder. No map transfer,story-navigation unlock,battle,custom SysCall or timer. Ordinary optional shopping is not a map/story puzzle with a correct solution;do not add hidden random-roll prediction or turn bids into story gates.'
  else:
   kind='ordinary animated pickup with corrected state projection';reason='Actual149 enables all four magicite groups;each separate150..153 receipt retires only its own21..24 actor. After150,the initializer only conditionally hides six party NPCs. Its formerly opaque tail incorrectly discarded guaranteed remaining pickup groups. The exact source-pinned tail now marks only those six NPCs uncertain through existing VM operations;all16 saved pickup profiles and full remote-floor journeys are checked. No guessed membership,new native query,early pickup or treasure-content label.'
  if script:assert proof['source_script_hashes'][row['submap']+'/'+script]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=[],reason=reason,evidence='docs/re/ZOZO_JIDOOR_REMAINDER_20261104.md;166 exact scripts/755 actors,four native Talk functions,all pickup profiles and real multi-floor production routes.')
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
 print('Zozo/Jidoor:seven exact controls;four independent animated receipts,four exact Talk handlers,nine auction bodies;no extra map/story puzzle invented.')
if __name__=='__main__':main()
