"""Nineteen exact ordinary town contacts;complete timer/ejection continuations."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
AREAS={'Tule','Carwen','Crescent','Lix','Regole'}
REASONS={
'sc_e_0018':'Automatic party/condition/item cleanup,normal Faris party departure and native17/273/274 story branches. The full controlled scene has no player-selected code,timed input,ordered switch or special battle action.',
'sc_e_0021':'Full automatic exterior party conversation records20. Staging,dialogue and controlled return are native;there is no separate player puzzle solution.',
'sc_npc_20011_1_1':'Ordinary beginner-house question. The novice answer records513 and hides the tutorial decorations;the experienced answer automatically ejects to30 through full sc_npc_20010_5,or gives later125 job advice. It is a tutorial confirmation with no code,ordered choices or progression puzzle. Actual native ejection/return is retained.',
'sc_e_0173':'Ordinary fully automatic special-service dance/music scene. Native SyncBGM owns its presentation and area variable0 becomes1;there is no manual dance input,note sequence or timing challenge.',
'sc_e_0173_1':'Three equivalent contacts finish the ordinary stage presentation and set area variable0 to2. No player answer or puzzle input is required;native movement/presentation owns the scene.',
'sc_e_0020':'Full automatic Faris-room conversation and party staging records19. No input solution,maze control or selected answer occurs.',
'sc_e_0176':'Direction guard skips one outward-facing traversal;the other entries run a complete automatic empty-house party conversation and record267. This is an optional narrative sensor,not an unlockable barrier or a special-facing puzzle.',
'sc_e_0031':'Native rumor conversation records30 with normal33 gating. A later135 conversation has one ordinary confirmation;leader branches only select character-specific dialogue and the same352 receipt. There is no correct-answer sequence or map/puzzle unlock.',
'sc_e_0362_1':'Two physical unopened/opened counterparts use one ordinary one-shot container body. Before442 it follows native169,optionally grants item13,then records442. This is a normal pickup with story-dependent contents,not a puzzle;treasure naming/reveal policy is unchanged.',
'sc_e_0032':'Native31/33/139 branches choose the ordinary mountain rumor dialogue and record31. The complete controlled conversation has no special player input or puzzle operation.',
'sc_e_0055':'Three equivalent contacts trigger the automatic ship-sinking/earthquake story scene and full54. Native six-second callback0055_1 repeats jump/facing/quake presentation only;it re-arms itself without any additional story or inventory receipt. It is not a player-timed puzzle and must not be omitted as an unresolved timer continuation.',
'sc_e_0140':'Ordinary music-box examination plays a fully automatic hometown memory and records140. No note selection,timed performance or navigation control is required.',
'sc_e_0201':'Ordinary bard dialogue/song gift after140;native207 prevents duplicate item468. The music-box memory is the normal conversation prerequisite,not a player-selected melody or code. There is no additional puzzle input or map gate.',
'sc_npc_20151_2_1':'Ordinary direction-dependent conversation:East/West describe the neighboring shops;North/South talk to the girl. After125,the same normal conversation grants Ribbon/item203 once and records282. Native player-facing dialogue confirms this is a gift,not a shop,hidden switch,answer sequence or separate facing puzzle. All conversation sides remain valid;no artificial door/story gate is added.',
}
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 check='--check'in sys.argv;master=read(R/'docs/re/ff5_remaining_native_20261104.json');selected=[r for r in master['candidates']if any(a in AREAS for a in r['area_names_en'])and r['script']in REASONS];assert len(selected)==19 and {r['script']for r in selected}==set(REASONS)
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==master['source_sha256'][role]
 extras={'Map_20100/sc_e_0055_1','Map_20010/sc_map_20010','Map_20100/sc_map_20100'}
 wanted={r['submap']+'/'+r['script']for r in selected}|extras|{'Map_20010/sc_npc_20010_5'};actual={};bodies={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);key=s['submap']+'/'+s['name']
  if key in wanted:actual[key]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest();bodies[key]=s['mnemonics']
 assert set(actual)==wanted and len(actual)==18
 for key in wanted-extras:assert actual[key]==master['source_script_hashes'][key]
 timer=bodies['Map_20100/sc_e_0055_1'];assert len(timer)==83 and {m['mnemonic']for m in timer}<={'Nop','Shake','Call','Wait','BranchByPlayerDirection','Jump','SetTimer','Exit','SetPuppet','ExecPuppet','Return','PlayAnim','LookAt'}
 assert timer[19]['mnemonic']=='SetTimer'and timer[19]['operands']['rValues'][0]==6 and timer[19]['operands']['sValues'][0]=='sc_e_0055_1'
 proof=dict(identities=19,root_classes=14,complete_scripts=18,source_script_hashes=actual,source_sha256=master['source_sha256'],dialogue_source_sha256=digest(D/'ff5_master/story_mes_en.txt'),dialogue_locators=['N107_C01_143_01_01','N107_C01_143_01_02','N107_C01_143_01_03','N107_C01_143_01_04','E0282_00_143_a_14','E0282_00_143_a_15','E0169_00_031_a_01'],timer_callback=dict(seconds=6,operations=83,story_or_inventory_effects=False),ordinary_direction_contacts=['Map_20011_9/sc_e_0176','Map_20151_2/sc_npc_20151_2_1'])
 p=R/'docs/re/ff5_town_contacts_native_20261105.json'
 if check:assert read(p)==proof
 else:p.write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=actual[r['submap']+'/'+r['script']],disposition='ordinary native town interaction',guides=[],reason=REASONS[r['script']],evidence='docs/re/FF5_TOWN_CONTACTS_20261105.md;18 complete pinned bodies/14 root classes,full timer/ejection callbacks/two native initializers and actual dialogue locators;production unchanged.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  k=key(r)
  if k in authored:new['decisions'].append(authored[k]);seen.add(k)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if check:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Town contacts:19 exact identities/14 root classes/18 complete bodies;ordinary confirmations,gifts and automatic scenes;timer/ejection/direction cases fully qualified,no production change.')
if __name__=='__main__':main()
