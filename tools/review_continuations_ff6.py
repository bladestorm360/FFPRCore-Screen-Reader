"""Explicit source-qualified dispositions for the remaining FFVI continuations.

The new Lethe solution owns its three separate decisions. This finite review
does not infer puzzle solutions from names or reopen accepted guide designs.
"""
from pathlib import Path
import collections,hashlib,json,sys
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump'
read=lambda p:json.loads(p.read_text(encoding='utf-8'))
identity=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw'])
body_sha=lambda s:hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()

def disposition(name):
 if name=='sc_e_0009':return('ordinary save-point tutorial',[],
  'Identical eleven-command save tutorial:flash,first-explanation receipt7 and optional explanation choice. It does not change a puzzle gate or provide a puzzle answer. Exact action2/script20 event shares group,layer and tile with native ObjectType9. The accepted save-point consolidator retains the Save Points destination and suppresses only this matching Events companion.')
 if name.startswith('sc_chocobo_'):return('ordinary paid vehicle service',[],
  'Repeatable paid chocobo rental. Confirmation and actual successful native fee removal precede the staged outer-map and mounted-world callbacks. Intro548 is not a one-time ownership receipt;decline or failed payment grants no free transit arc. All three actual source producers are in this closure.')
 if name in{'sc_e_'+str(n).zfill(4)for n in range(414,421)}:return('ordinary party formation choice',[],
  'Native character conversation opens one-party formation after confirmation and returns through actual120/2/0414_1;the225/not284 alternative delegates0462..0468 and returns124/2 with area-local1. Refusal exits. These seven parallel party services contain no ordered puzzle answer or locked treasure passage.')
 rows={
 'sc_e_0077':('optional disguise removal',['south_figaro_infiltration'],'The first option removes both actual disguise conditions665/666 and clears52/53;the second retains them. The accepted infiltration guide reads those native receipts and can recover its required disguise. This optional removal is not an independent puzzle or a permanent gate.'),
 'sc_sty_20111_1':('guided alternate delivery producer',['mobliz_letters'],'A paid successful inn rest delegates its actual branch and0226 letter-delivery callback. The accepted five-letter guide already covers reply delivery through native shop-close callbacks;an inn rest is the documented alternative. No purchase or elapsed-time completion is invented.'),
 'sc_e_0426_0':('ordinary presentation scene',[],'This wrapper sets current-room local1 and delegates0426 for the Albrook blockade/shaking conversation. The complete body has no selectable puzzle answer,item consumption or player-controlled passage operation.'),
 'sc_e_0427':('ordinary optional purchase',[],'Native optional magicite purchase uses fee203,item76 and actual206. Declining or failed payment keeps its native branch. This ordinary visible confirmation is not an ordered puzzle and is independent of progression gates.'),
 'sc_npc_20230_1':('ordinary native story conversation',[],'Actual196/197 select ordinary Esper elder dialogue or0408. The native direction/staging branch records198 and current-room local1;the player is not choosing a spatial answer or moving through an invented story arc.'),
 'sc_npc_20231_5_2':('ordinary native story conversation',[],'Actual196/199 select ordinary dialogue or0410. The complete automatic elder scene sets200 after messages and puppet staging,without a puzzle choice or player action sequence.'),
 'sc_npc_20231_5_3':('ordinary native story conversation',[],'Actual197/199/200/201 select ordinary fairy dialogue and0411. The automatic scene sets201;its BranchByPlayerDirection selects staging for the existing approach,not a puzzle answer. No new guidance solution is needed.'),
 'sc_npc_20271_5_1':('ordinary character introduction',[],'Actual268/not271 delegates0521. The full scene introduces Strago and Relm through native name entry,automatic puppets and271 before returning180/8. Names and confirmations are normal accessible menus,without an undisclosed puzzle rule.'),
 'sc_e_0616':('guided mechanic',['solitary_island'],'The existing complete fish/Cid/raft guide targets191/7. Actual native HP/timer checks consume the real held fish receipts;full survivor and bereavement paths remain distinct and converge only at306 after the full raft landing. This same exact source was qualified by solitary_native_20261104.json;the accepted timer/guide design stays Closed.'),
 'sc_e_0640':('ordinary optional purchase',[],'Later optional magicite purchase uses fee209,item76 and313,with original206 exclusion. The native payment-result branch remains authoritative. It is ordinary shopping rather than an ordered puzzle or a new route gate.'),
 'sc_e_0674':('ordinary native castle transport',[],'Actual336/337/425 choose native castle submerge,surface or intermediate Ancient Castle confirmation. Complete0674/0675/1067/0882 callbacks retain their actual world/castle returns and425 reset/hold outcomes. This is optional repeatable transport;no intermediate animation is promoted to a free walking edge.'),
 'sc_e_0959':('ordinary optional character scene',[],'Native Gau/Sabin/party and469/470 conditions select reunion dialogue,the full0959_1.._5 automatic shopping/return scene,or1070. Party checks select native dialogue variants;there is no right-answer choice,player-controlled minigame or new passage gate.'),
 'sc_e_0118_0':('ordinary native battle scene',[],'The wrapper sets current-room local2 and delegates the existing Imperial Camp Kefka battle/0118_1 continuation. It is an additional producer of the already reviewed ordinary camp story scene,not another mask puzzle. Accepted camp mask and escape semantics remain unchanged.'),
 'sc_e_0394':('ordinary native encounter and conversation',[],'Before184,interaction starts native Ifrit encounter440 and its real0394_1 receipt. Afterwards0395 supplies185 and waits for the independent Shiva conversation187 before the automatic188 transformation. This is an ordinary battle and two NPC conversations,not a hidden choice solution;animated189/190 pickup availability and source-qualified passage routing stay intact.'),
 'sc_e_0757':('guided prerequisite with optional reading choices',['owzers_mansion'],'The accepted mansion guide explicitly targets diary442/20 before its real light switch. Entry immediately sets577 and updates the initializer;five optional page choices only cycle current-room local1. Reading every page or selecting a particular page is not required to operate the light or finish the mansion.'),
 'sc_e_0831':('optional retreat choice',['phoenix_cave'],'Two physical Phoenix landing controls offer the same voluntary return to Falcon268/2. Acceptance restores party1 and clears557;decline exits. This is retreat,not the separate full Locke reunion completion. The accepted multi-party guide reads the actual native switch/party state on a later request.'),
 }
 if name in{'sc_e_'+str(n).zfill(4)for n in range(653,657)}:return('guided mechanic',['gerad_passage'],
  'This is one of the four real pub conversations322..325. The fourth successful talk delegates0657 and sets326;there is no fifth required talk. Accepted guidance targets all four actual actors and continues through the entire follow/ferry/turtle/engine/full336 surfacing journey.')
 if name in{'sc_e_'+str(n)for n in range(1081,1087)}:return('guided mechanic',['terra_defense'],
  'One of the six real approaching-enemy controls318/18..23 already targeted by terra_defense. Native RTS party selection starts encounter18;the result callback restores the correct party. The shared retreat/aftermath is distinct from actual defense completion14;the accepted guide also targets commander9 and preserves all three parties.')
 if name in{'sc_e_'+str(n)for n in range(1098,1101)}:return('optional retreat and reset choice',['kefkas_tower'],
  'One of three voluntary Tower escape controls. Yes calls the actual returning resident reset then Falcon268/2;No exits. The reset clears its exact puzzle receipts and party/local state,not a completion receipt. Existing Tower guidance resumes from current native state;no false final-completion step or automatic input is added.')
 return rows[name]

def main():
 inv=read(R/'docs/re/puzzle_mechanics_continuation_inventory_20261104.json')['games']['ff6'];proof=read(R/'docs/re/ff6_continuation_native_20261104.json')
 assert len(inv['candidates'])==88 and len(proof['source_script_hashes'])==176 and len(proof['actors'])==1486 and not proof['unresolved_script_names']
 for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
  with(D/name).open('rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==inv['source_sha256'][role]
 selected={}
 for line in(D/'ff6_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in proof['source_script_hashes']:assert body_sha(s)==proof['source_script_hashes'][k];selected[k]=s
 assert selected.keys()==proof['source_script_hashes'].keys()
 # Confirm every physical tutorial counterpart,not just a representative room.
 rooms={r['submap']for r in inv['candidates']};objects=collections.defaultdict(list)
 for line in(D/'ff6_objects.jsonl').open(encoding='utf-8'):
  o=json.loads(line)
  if o['submap']in rooms:objects[o['submap']].append(o)
 tutorials=[r for r in inv['candidates']if r['script']=='sc_e_0009'];assert len(tutorials)==42
 for row in tutorials:
  o,=[o for o in objects[row['submap']]if o['group']==row['group']and o['props'].get('entity_id')==row['entity']and o['name']==row['raw']]
  assert o['props']['script_id']==20 and o['props']['action_id']==2
  assert any(q['props'].get('object_type')==9 and(q['group'],q['layer'],q['tx'],q['ty'])==(o['group'],o['layer'],o['tx'],o['ty'])for q in objects[row['submap']])
 guides={g['id']:g for g in read(R/'data/ff6/puzzles.json')['story_guides']}
 for name,map_id,ids in [('gerad_passage',217,{1,2,3,4}),('terra_defense',318,set(range(18,24))),('solitary_island',191,{7}),('owzers_mansion',442,{20})]:
  actual={t.get('entity')for r in guides[name]['rules']for t in r.get('targets',[])if t.get('map')==map_id};assert ids<=actual
 excluded=[r for r in inv['candidates']if r['area_names_en']==['Lethe River']];assert len(excluded)==3
 rows=[r for r in inv['candidates']if r not in excluded];assert len(rows)==85
 authored={}
 for row in rows:
  kind,owned,reason=disposition(row['script']);assert all(g in guides for g in owned)
  digest=inv['script_facts'][row['submap']+'/'+row['script']]['sha256'];assert digest==proof['source_script_hashes'][row['submap']+'/'+row['script']]
  out=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=digest,disposition=kind,guides=owned,reason=reason,evidence='docs/re/FF6_CONTINUATION_REVIEW_20261104.md;176 complete source/callback fingerprints,1486 physical actor facts and42 exact native save-point companions in ff6_continuation_native_20261104.json;accepted source-matched guide/controller evidence reused without reopening Closed designs.')
  assert identity(out)not in authored;authored[identity(out)]=out
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);seen=set();new['decisions']=[]
 for row in old['decisions']:
  k=identity(row)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(row)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFVI continuation review:85 exact identities/176 source bodies/1486 actors/42 save companions;accepted mechanics preserved. Lethe owns its separate three decisions.')
if __name__=='__main__':main()
