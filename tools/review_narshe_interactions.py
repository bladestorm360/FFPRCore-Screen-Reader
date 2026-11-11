"""Explicit disposition of43 exact original Narshe interaction discoveries."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(row):return row['submap'],row['group'],row['entity'],row['raw']
def main():
 rows={};facts={};sources=None
 cohort={'Map_20010':(132,145,144,134,135,186,129,130,131,189,124,120,121,122,110,111,112),
  'Map_40008':(7,20),'Map_40009':(9,12),'Map_20011_12':(0,),'Map_20011_13':(19,24),
  'Map_20011_14':(13,),'Map_20011_7':(8,),'Map_20020':(22,20,19,18),
  'Map_20040':(35,28,27,29,30,31,39,40,41,42,43),'Map_30011_3':(18,),'Map_30021_1':(13,)}
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
  assert sources is None or sources==inventory['source_sha256'];sources=inventory['source_sha256'];facts.update(inventory['script_facts'])
  for row in inventory['candidates']:
   if row['entity']in cohort.get(row['submap'],()):rows.setdefault(key(row),[]).append(row)
 # The accepted light-path family is outside these original candidate identities.
 assert len(rows)==43
 chase={'sc_e_0432','sc_e_0433','sc_e_0434','sc_e_0435','sc_e_0436','sc_e_0438','sc_e_0439','sc_e_1055'}
 receipts={'sc_e_0016':12,'sc_e_0019':79,'sc_e_0088':66,'sc_e_0286':154,'sc_e_0286_1':154,'sc_e_0777':534,'sc_e_0093':69,'sc_e_0008':6,'sc_e_0010':8}
 authored=[]
 for identity,variants in rows.items():
  row=variants[0];script=row['script'];guides=[]
  if script in chase:
   kind='guided native chase operation';guides=['narshe_chase']
   reason='Explicit beginning-to-end chase objective/counterpart: treasure-room native entry and208 wait, empty chest209, pursuit210..213, native five-second motionless214 sequence, then Mog rescue215. Alternate actor27/216 is a real mutually exclusive outcome and never mistaken for the requested rescue. Movement-detector39..43 cancels the native timer and is never offered as a goal. The native operation, real collision and exact receipts remain authoritative.'
  elif not script:
   assert row['submap']in('Map_40008','Map_40009')and row['entity']in(7,20,9,12)and row['action']==0
   kind='cinematic display counterpart'
   reason='Exact mysterious-esper NPC/white-blink AnimEntity counterpart in the two early story movies. Both have action0 and no script; the riddle keyword is descriptive, not a player-operated puzzle. Native movie/initializer ownership retained; no invented interaction or route gate.'
  elif script=='sc_e_0089':
   assert row['entity']==132 and row['submap']=='Map_20010'
   kind='single native passage control'
   reason='One manual entrance interaction opens the secret passage, with first-use67 and current-map local1. Native initializer puppet/collision state and the actual passage exits remain authoritative. The control stays a target; its reveal is not approximated by possessing a future story flag. This is one native action, not an omitted multi-step solution.'
  elif script in('sc_e_0873','sc_e_0874'):
   kind='ordinary native NPC interaction'
   reason='World of Ruin shop/house NPC conversation: native0873 presents its ordinary two-choice exchange and stores417;0874 grants its one-time native item and stores418. Neither changes route topology or requires a separate map puzzle. Native choice speech/focus remains the authority; no hidden reward is added to treasure labels.'
  elif script=='sc_e_0778':
   kind='ordinary native party recruitment'
   reason='World of Ruin Mog conversation/recruitment with native393 receipt and name entry; no ordered path or independent puzzle control. The Closed light guide is World of Balance-only and remains unchanged. Do not infer recruitment from proximity or enable that earlier light puzzle after collapse.'
  else:
   assert script in receipts,(identity,script)
   kind='native story conversation or automatic scene'
   reason='Exact story scene/conversation with native receipt'+str(receipts[script])+': early detection, guard refusal, elder conversation, mine entry/door scene, Shadow departure or later town explanation. Main flow consists of native dialogue/puppets and that receipt, rather than a player-selected puzzle sequence. Existing entity-state/collision and native continuation ownership remain; no invented automatic unlock or vehicle restriction.'
  result=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(row['submap']+'/'+script,{}).get('sha256'),disposition=kind,guides=guides,reason=reason,evidence='docs/re/NARSHE_CHASE_GUIDANCE_20261103.md; exact external source facts, actual production guide/model/planner and preserved earlier Narshe defense/light/gate evidence.')
  if len(variants)>1:result['instances']=[{k:v[k]for k in('type','cell','action','script')}for v in variants]
  authored.append(result)
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as stream:assert hashlib.file_digest(stream,'sha256').hexdigest()==sources[role],role
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(row):row for row in authored};seen=set();new['decisions']=[]
 for row in old['decisions']:
  if row['game']=='ff6'and key(row)in replacement:new['decisions'].append(replacement[key(row)]);seen.add(key(row))
  else:new['decisions'].append(row)
 new['decisions'].extend(row for row in authored if key(row)not in seen)
 if'--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Narshe:43 exact original interactions;',sum(map(len,rows.values())),'physical instances; previous decisions retained.')
if __name__=='__main__':main()
