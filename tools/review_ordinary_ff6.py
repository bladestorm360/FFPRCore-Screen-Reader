"""Eighteen ordinary controls; keep the distinct Tzen rescue pending."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];identity=lambda r:(r['submap'],r['group'],r['entity'],r['raw'])
REASONS={
'sc_e_0095':('ordinary shop choice','Two cabin merchant forms open the same native shop through visible choices. Flag70 records the first introduction; later map-local bookkeeping swaps the merchant forms and starts its native move route. It is not a combination, environmental unlock or prescribed story answer.'),
'sc_e_0097':('ordinary optional recruitment','Cabin Shadow naming/introduction is followed by an ordinary Yes/No invitation. Yes joins Shadow and records71; No remains available. No special map mechanism or sequence is required beyond the native dialogue.'),
'sc_e_0150':('ordinary character-sensitive conversation','The cabin elder chooses ordinary dialogue/animation by current leader. Map-local1 advances its repeat conversation, whose actual replacement script is included. No environment gate, item requirement or puzzle solution is introduced.'),
'sc_e_0151':('ordinary one-shot stove vignette','Examining the stove plays an automatic vignette and first-time104 exterior relocation. Subsequent examination does not repeat the transfer. This is not a required puzzle operation or a new route through locked content; the existing guarded transfer and ordinary door remain unchanged.'),
'sc_e_0285':('ordinary paid recruitment','Kohlingen Shadow uses the native quoted-fee Yes/No prompt. A full party or failed payment refuses recruitment; successful payment joins him and records147. These are ordinary party/shop conditions, not a hidden answer sequence. Physical conversation remains valid in every case.'),
'sc_e_0281':('ordinary directional story vignette','The native entrance vignette hides its own marker and uses current Locke/facing to choose a conversation/animation. Its0282 replacement is included; temporary speaker movement returns to its prior position. It opens no environmental lock or puzzle route.'),
'sc_e_0680':('ordinary location hint','The later Kohlingen vignette presents a location hint with temporary speaker movement and map-local bookkeeping. It sets no persistent gate or puzzle receipt and requires no non-navigation solution.'),
'sc_e_0066':('ordinary free-order scenario selection','Choosing Locke starts his scenario, organizes its native party and lands at South Figaro38/13. This is one legitimate scenario choice, not an answer to a puzzle or a requirement to choose it first. Ordinary NPCs are not promoted to automatic travel edges.'),
'sc_e_0087':('ordinary free-order scenario selection','Banon/Terra/Edgar are three physical selectors for the same legitimate scenario. The complete0087_1 continuation includes three normal scripted battles before World1/34, not a controllable landing at349. No branch-choice puzzle occurs and no selector is promoted to an automatic route edge.'),
'sc_e_0094':('ordinary free-order scenario selection','Choosing Sabin organizes the native party and enters World1/35. No particular scenario order is required. The accessible scenario NPC choice remains distinct from an automatic map exit.'),
'sc_e_1052':('ordinary elder conversation','The Esper World elder presents dialogue and records524. There is no Select, input combination, inventory exchange, environmental control or separate solution to add.'),
'sc_e_0158':('ordinary automatic healing scene','The forest marker moves the party to the spring, performs native recovery and hides its one-shot actor with106. The player has no puzzle operation or choice to solve. Existing forest/train travel is preserved.'),
'sc_e_0682':('ordinary tomb-opening introduction','The external tomb marker plays the automatic Setzer introduction, opens the native entrance group and records340. It offers no puzzle choice or combination; the already accepted full Darill guide owns the actual interior puzzle. The native story prerequisite remains intact.'),
'sc_e_0313':('ordinary opera-quest conversation','The director conversation records156 and enables the letter group. Direction branches control staging only. This is ordinary story progress before the accepted Opera sequence, with no solution answer or environmental puzzle.'),
'sc_e_0314':('ordinary round-trip introduction scene','Reading the letter records157 and shows the Setzer introduction on119, whose complete continuation returns to611 and the second branch of0314. This is an automatic round-trip scene, not access to a controllable Blackjack. The bake keeps it as an event without a travel companion.')}
def main():
 proof=json.loads((ROOT/'docs/re/ordinary_ff6_native_20261104.json').read_text(encoding='utf-8'));assert len(proof['discoveries'])==19 and len(proof['source_script_hashes'])==95
 selected={identity(r):r for r in proof['discoveries']if r['script']!='sc_e_0637'};assert len(selected)==18
 assert proof['facts']['Map_20170/sc_e_0314_1']['transfers']==[[18,611,1,'sc_e_0314']]
 assert len(proof['facts']['Map_30400/sc_e_0087_1']['encounters'])==3 and proof['facts']['Map_30400/sc_e_0087_1']['transfers']==[[24,1,34,'']]
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  data=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==data['source_sha256'];sources=data['source_sha256'];facts.update(data['script_facts'])
  for row in data['candidates']:
   if identity(row)in selected:rows.setdefault(identity(row),[]).append(row)
 assert len(rows)==18 and sum(map(len,rows.values()))==18
 authored={}
 for key,variants in rows.items():
  row=variants[0];kind,reason=REASONS[row['script']];locator=row['submap']+'/'+row['script'];wanted=facts[locator]['sha256'];assert proof['source_script_hashes'][locator]==wanted
  authored[key]=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=wanted,disposition=kind,guides=[],reason=reason,evidence='docs/re/ORDINARY_FF6_CONTROLS_20261104.md;95 exact connected source bodies,existing party/shop/dialogue behavior and unchanged mod-data/graph preservation.')
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==sources[role]
 p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);new['decisions']=[];seen=set()
 for row in old['decisions']:
  key=identity(row)
  if row['game']=='ff6'and key in authored:new['decisions'].append(authored[key]);seen.add(key)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 # This review never completes the child as an ordinary interaction. A later
 # full Tzen slice may own it, without reopening these accepted dispositions.
 children=[r for r in new['decisions']if r['game']=='ff6'and r['submap']=='Map_30611_2'and r['script']=='sc_e_0637']
 assert not children or len(children)==1 and children[0]['guides']==['tzen_rescue']and 'TZEN_RESCUE_20261104.md'in children[0]['evidence']
 print('Ordinary FFVI:18 exact dialogue/shop/free-scenario/automatic-scene decisions. The child is excluded from ordinary completion and belongs only to the separate full Tzen guide.')
if __name__=='__main__':main()
