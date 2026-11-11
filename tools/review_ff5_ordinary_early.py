"""Finite exact story dispositions;physical/vehicle puzzle leads stay pending."""
from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1];D=R.parent/'tools/mapdump';read=lambda p:json.loads(p.read_text(encoding='utf-8'))
AREAS={'Western Valley','Tycoon Meteorite','Wind Shrine','Ship Graveyard',"Xezat's Fleet",'Barrier Tower','Guardian Tree','Karnak Meteorite','Gohn Meteorite','Black Chocobo Forest','Interdimensional Rift','Moogle Forest','Cave','Underground Waterway'}
REASONS={
'sc_e_0007':'Three physical rescue contacts run the same automatic scene and record406. No choice,controlled jump or special battle command is requested. The separate pre-rescue0478 jump contacts remain pending.',
'sc_e_0480':'Three post-rescue contacts perform the full578/0480_1/1/0480_2 return,restore the real party and record6 only at the end. This is automatic story staging,not a player-selected maze answer. The separate0478 jump geometry remains pending.',
'sc_e_0134':'Eight contacts start the same complete243-operation automatic story scene. Its actual final134 receipt,groups and puppet movement are native story progression;there is no choice,ordered control,player timer or prescribed input solution.',
'sc_e_0002':'Native introductory dismount/removal of vehicle3 and story1/reset4 are an automatic scene. The mod must retain these actual ownership receipts;this is not an independent vehicle puzzle or permission to recreate a missing mount.',
'sc_e_0004':'Player-facing branches only stage the same conversation;temporary party removal and item removal precede native3. These are automatic story operations,not directional answers the player must select.',
'sc_e_0005':'Native Boko interaction branches on the real introductory story state,places vehicle3,boards the chocobo and records4. It is ordinary native vehicle interaction;ownership and actual placement remain authoritative.',
'sc_e_0069':'Four physical meteorite-opening contacts run one complete192-operation story scene and record68. Native party staging opens the plot route automatically;there is no manual code,switch order or choice puzzle.',
'sc_e_0070':'The actual adamantite pickup grants item47 and records69 through one complete scene. It is an ordinary visible story-item operation;later encounters/eligibility must retain their own native receipts.',
'sc_e_0035':'Footprint discovery is one automatic37-operation scene ending34. It has no selected track order,hidden directional answer or controlled chase operation.',
'sc_e_0204':'Spring discovery is automatic81-operation staging ending208. It is an ordinary rest-area introduction,not a player-operated passage puzzle.',
'sc_e_0205':'The spring removes conditions,restores native HP/MP and records the ordinary first-use tutorial receipt300. There is no reward answer,required switch sequence or controlled movement mechanic.',
'sc_e_0009':'The complete227-operation pirate observation scene records8 after native staging. It is ordinary story progression without player-controlled puzzle inputs.',
'sc_e_0015':'The minister conversation is one full257-operation introductory scene ending14. Approaching the NPC requires ordinary routing;the scene contains no puzzle answer or ordered interaction.',
'sc_e_0210':'The menu offers optional job/ability/equipment tutorials and normal recovery. It records197 and uses actual125 to return to world1/point5 or map19/point3. These educational choices have no correct puzzle answer;the conditional native return is preserved.',
'sc_e_0027':'The101-operation water reluctance scene automatically stages the party and records26. It does not ask the player to choose a dry route or execute a special command.',
'sc_e_0028':'The complete732-operation drying/rest scene records27 and performs native condition/HP/MP recovery. No player puzzle input is present. The separate chest0029 bridge mechanism remains pending.',
'sc_e_0080':'Both scientist contacts share one ordinary conversation branch and final79 receipt. Neither asks for an ordered response or activates a player-controlled mechanism.',
'sc_e_0082':'Both scientist contacts run the same133-operation scene and record81. This is ordinary plot conversation;native future travel/vehicle eligibility remains separate.',
'sc_e_0091':'The moogle contact is a202-operation automatic scene ending90. It contains no player-controlled puzzle choice or special-input battle solution.',
'sc_e_0092':'Moogle discovery is one102-operation automatic scene ending91. Its motions are native puppet staging rather than an ordered navigation input puzzle.',
'sc_e_0109':'Xezat interaction is a complete368-operation attack scene,including a native temporary controlled-character change and108. No special combat command or selected answer is required by this field script.',
'sc_e_0108':'The ordinary rest confirmation leads to recovery and the native attack scene ending107. Declining postpones resting;there is no riddle or correct response sequence.',
'sc_e_0115_12':'Galuf communication is a32-operation story conversation recording394. It is not a second switch,party-pressure puzzle or timed player action.',
'sc_e_0113':'The complete whisperweed scene grants actual item49,records113 and autosaves. This ordinary key-item handover is not a choice solution or independently operated barrier mechanism.',
'sc_e_0154':'The complete137-operation monster greeting records154 after automatic staging/effects. It contains no riddle,choice order or prescribed player input.',
'sc_e_0343_1':'Before156,this ordinary book examination delegates0156 and real boss505;full0156_1 records156/206. After156,the normal book effect records206 directly. There is no selected answer or required special battle command in the complete field/callback chain.',
}
def digest(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 proof=read(R/'docs/re/ff5_remaining_native_20261104.json');assert len(proof['candidates'])==347 and not proof['unresolved_script_names']
 selected=[r for r in proof['candidates']if any(a in AREAS for a in r['area_names_en'])and r['script']in REASONS];assert len(selected)==42 and {r['script']for r in selected}==set(REASONS)
 for role,name in{'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}.items():assert digest(D/name)==proof['source_sha256'][role]
 wanted={r['submap']+'/'+r['script']for r in selected};wanted.update(('Map_30581_1/sc_e_0156','Map_30581_1/sc_e_0156_1','Map_40002/sc_e_0480_1','Map_10010/sc_e_0480_2'));actual={}
 for line in(D/'ff5_scripts.jsonl').open(encoding='utf-8'):
  s=json.loads(line);k=s['submap']+'/'+s['name']
  if k in wanted:actual[k]=hashlib.sha256(json.dumps(s['mnemonics'],sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()
 assert set(actual)==wanted and all(v==proof['source_script_hashes'][k]for k,v in actual.items())
 key=lambda r:(r['game'],r['submap'],r['group'],r['entity'],r['raw']);authored={}
 for r in selected:
  sha=proof['source_script_hashes'][r['submap']+'/'+r['script']];authored[key(r)]=dict(**{k:r[k]for k in('game','submap','group','entity','raw','script')},script_sha256=sha,disposition='ordinary native story interaction',guides=[],reason=REASONS[r['script']],evidence='docs/re/FF5_ORDINARY_EARLY_20261104.md;complete root/callback bodies and factual actor manifest in ff5_remaining_native_20261104.json. Distinct jump,bridge and moving-vehicle leads remain pending.')
 p=R/'docs/re/puzzle_semantic_review_20261101.json';old=read(p);new=dict(old);new['decisions']=[];seen=set()
 for r in old['decisions']:
  identity=key(r)
  if identity in authored:new['decisions'].append(authored[identity]);seen.add(identity)
  else:new['decisions'].append(r)
 new['decisions'].extend(v for k,v in authored.items()if k not in seen)
 if '--check'in sys.argv:assert new==old
 else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print(f'FF5 early ordinary review:42 exact identities/{len(REASONS)} root classes/{len(actual)} full pinned bodies;five physical/vehicle controls explicitly remain pending;production unchanged.')
if __name__=='__main__':main()
