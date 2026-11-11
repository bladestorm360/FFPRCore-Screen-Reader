"""Exact additional II–IV callers;accepted transport/quest designs stay intact."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 inv=json.loads((R/'docs/re/puzzle_mechanics_continuation_inventory_20261104.json').read_text(encoding='utf-8'));p=R/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));authored={};bodies=0;actors=0
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw'])
 for game,count in(('ff2',9),('ff3',1),('ff4',1)):
  data=inv['games'][game];assert len(data['candidates'])==count;proof=json.loads((R/'docs/re'/(game+'_continuation_native_20261104.json')).read_text(encoding='utf-8'));assert proof['scope_tag']==game+'_ContinuationSupplement'and not proof['unresolved_script_names']
  expected=proof['source_script_hashes'];actual={}
  for line in(R.parent/'tools/mapdump'/(game+'_scripts.jsonl')).open(encoding='utf-8'):
   s=json.loads(line);locator=s['submap']+'/'+s['name']
   if locator in expected:actual[locator]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
  assert actual==expected;bodies+=len(actual);actors+=len(proof['actors'])
  for role,name in{'objects':game+'_objects.jsonl','scripts':game+'_scripts.jsonl','maps':game+'_master/map.txt','script_master':game+'_master/script.txt','areas':game+'_master/area.txt','english_names':game+'_master/system_en.txt'}.items():assert digest(R.parent/'tools/mapdump'/name)==data['source_sha256'][role]
  for row in data['candidates']:
   name=row['script'];locator=row['submap']+'/'+name;sha=data['script_facts'][locator]['sha256'];assert actual[locator]==sha
   if game=='ff2'and name=='sc_e_0152':
    previous,=[r for r in old['ff2_keyword_decisions']if(r['submap'],r['script'])==(row['submap'],name)];assert previous['script_sha256']==sha and previous['disposition']=='ordinary story interaction'
    disposition='ordinary keyword and recruitment interaction';reason='Native Minwu conversation uses the existing Ask/Learn/Item interface. Once8 is known,returning script0010 automatically recruits Minwu,awards the Canoe and writes9. No independent order,riddle or environmental control is missing;the prior exact keyword decision is preserved.'
   elif game=='ff2'and name.startswith(('sc_sea_ship_taxi_','sc_air_ship_taxi_')):
    disposition='ordinary transport menu';reason='Exact sea-service bodies show the fare,accept native confirmation,check payment and update97/98 travel state. Airship service shows the actual station selector,checks money,spawns or withdraws Vehicle7 and updates110/113. Destination/confirmation/payment menus are ordinary transport UI,with no unique puzzle answer or map-control sequence. Both intact/ruined town variants and the distinct Poft groups retain their own identities.'
   elif game=='ff2'and name=='sc_e_0073':
    disposition='ordinary story-return confirmation';reason='Native Yes confirmation restores Ship4 and returns from Leviathan to world1;full callback positions at57,133,boards the Ship,sets66 and autosaves. Declining only moves East. This is a story exit confirmation,with no ordered map/riddle mechanic;existing source-qualified return/vehicle routing remains intact.'
   elif game=='ff3':
    assert name=='sc_e_0087';disposition='automatic story key handover';reason='Full0087→0327 chain automatically awards items44 and43,records treasure422/423,and writes87 only after both staged handovers. Neither body has a choice or puzzle operation. Existing Eureka/Syrcus key-door recipes and native inventory/story validation remain intact;no new gate is inferred from possession or labels.'
   else:
    assert game=='ff4'and name=='sc_npc_20051_9_1';previous,=[r for r in old['ff4_item_decisions']if r['script']=='sc_e_0112'];assert previous['disposition']=='ordinary story interaction'
    disposition='ordinary quest item exchange';reason='Full wife root chooses0111 after76 to award Frying Pan62/write77;after78 it chooses0113,whose native item selector consumes62,awards159 and writes79. Pre/post-story branches are ordinary conversation. This is the return leg of the already-reviewed optional Yang/Sylph item exchange0112,with no separate riddle/order/environmental control. Existing native item-menu initial focus/detail support applies;the accepted quest classification is preserved.'
   out=dict(game=game,**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=sha,disposition=disposition,guides=[],reason=reason,evidence='docs/re/CONTINUATION_PUZZLE_REVIEW_20261104.md;exact full caller/callback proof in '+game+'_continuation_native_20261104.json')
   assert key(out)not in authored;authored[key(out)]=out
 assert len(authored)==11 and bodies==15 and actors==238
 new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  k=key(row)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('II–IV continuation review:11 exact identities/15 full scripts/238 actor facts;existing transport,key and optional quest classifications preserved;no production change.')
if __name__=='__main__':main()
