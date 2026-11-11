"""Explicit semantic links for52 candidates around four completed FFV families.

No name-based catch-all classifier or new puzzle solution is introduced. Every
accepted script/physical counterpart is pinned below; unknown records fail.
Primary game bodies remain external and are never emitted.
"""
from collections import Counter
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
AREAS={'Library of the Ancients','Catapult',"Ghido's Cave",'Surgate Castle'}
LIBRARY={'Map_20221_4':{'sc_e_0220'}|{f'sc_e_0220_{i}'for i in range(1,9)},
 'Map_20221_6':{'sc_e_0221'},'Map_20221_7':{'sc_e_0222','sc_e_0222_1','sc_e_0430'},
 'Map_20221_10':{'sc_e_0223','sc_e_0223_1','sc_e_0223_2'}}
def identity(r):return r['submap'],r['group'],r['entity'],r['raw']
def disposition(r):
 room=r['submap'];script=r['script'];actor=r['entity']
 if script in LIBRARY.get(room,set()):
  return 'guided mechanic',['library_policy'],'Exact shelf/fire-seal operation in the completed native-visibility Library guide; all entries, reversals, contact ordering and full Mid completion are qualified.'
 if room=='Map_20231_4'and script in{f'sc_e_0224_{i}'for i in range(1,6)}:
  return 'guided mechanic',['catapult_clues'],'One of the five ordered native clue/switch operations, with exact309–313 receipts and current staged-actor recovery in the completed Catapult guide.'
 if(room=='Map_30381_2'and script=='sc_e_0254')or(room=='Map_30381_3'and script in{f'sc_e_0252_{i}'for i in range(1,6)}):
  return 'guided mechanic',['ghido_stone_chests'],'Exact stone/chest or opening-switch operation in the completed Ghido guide, including center176 inversion, incorrect placements, resets and both required opened passages.'
 if room=='Map_20171_7'and script in{'sc_e_0241','sc_e_0383','sc_e_0385','sc_e_0387_1'}:
  return 'guided mechanic',['surgate_books'],'Exact book pickup or librarian completion in the qualified Surgate arrangement guide; correct shelves, wrong placements, reset/re-entry and reward-room route are covered.'
 if not script and room=='Map_20221_4'and(r['group'],actor)in{('ev_e_0224',23),('ev_e_0224',29)}:
  return 'decorative counterpart',['library_policy'],'Unbound staged book/switch counterpart, not an additional script operation. Real shelf operations and their physical layouts are covered by the native Library episodes.'
 if not script and room=='Map_20221_7'and(r['group'],actor)in{('ev_e_0049',11),('ev_e_0220',11)}:
  return 'decorative counterpart',['library_policy'],'Unbound blocking-bookcase counterpart. The actual fire-seal/shelf scripts and collision-preserving routes are covered by the completed Library guide.'
 if not script and room=='Map_20231_4'and actor==42:
  assert r['action']==0 and r['type']=='NPC'
  return 'decorative counterpart',['catapult_clues'],'Non-interactive frog actor with no action or bound script; no additional puzzle step is available.'
 if room=='Map_20231_4'and script=='sc_e_0224':
  return 'optional hint',['catapult_clues'],'Repeatable initial switch hint; the source contract proves no persistent puzzle mutation. The completed guide supplies the required clue sequence directly.'
 if not script and room=='Map_20231_4'and actor==13:
  assert r['type']=='TreasureBox'and r['action']==6
  return 'ordinary reward',['catapult_clues'],'Ordinary built-in chest. Solved Catapult313 opens access; the guide covers its five prerequisites and ordinary treasure routing owns collection. Contents remain undisclosed by the mod.'
 if(room,script)in{('Map_20171_2','sc_e_0240'),('Map_20221_3','sc_e_0287')}:
  return 'ordinary reward',[],'Single song-reward interaction, with native GetItem and one persistent receipt; no ordered map-control puzzle, local puzzle state or choice sequence.'
 if room=='Map_20171_7'and script=='sc_e_0362_2'and actor in(64,67):
  return 'ordinary reward',[],'Closed/opened scripted item-container variants, guarded by native432 and GetItem. Ordinary collection rather than a separate puzzle sequence; both identities remain explicit.'
 if(room,script)in{('Map_20231_4','sc_e_0077'),('Map_20231_7','sc_e_0068'),('Map_20231_7','sc_e_0369'),('Map_30381_7','sc_e_0117'),('Map_30381_7','sc_e_0436')}:
  return 'ordinary story scene',[],'Single letter, conversation or automatic progression scene with its own persistent receipt and staged movement. No puzzle choice, local-control sequence or additional puzzle mechanic.'
 if room=='Map_20170'and script in{'sc_e_0239_1','sc_e_0239_2','sc_e_0381'}:
  return 'ordinary gate interaction',[],'One native door-opening interaction or automatic switch-removal companion, receipt520; no ordered puzzle state or item/keyword/form selection. Door availability belongs to the existing map/story-gate model.'
 if room=='Map_20171_7'and script=='sc_e_0242':
  return 'ordinary examination',[],'Conditional bookshelf messages only: no item, persistent/local mutation, choice or geometry operation in the complete native body.'
 raise AssertionError(('Unreviewed completed-family candidate',identity(r),script))
def main():
 rows=[];facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  data=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff5']
  if sources is not None:assert sources==data['source_sha256']
  sources=data['source_sha256'];facts.update(data['script_facts'])
  rows.extend(r for r in data['candidates']if AREAS.intersection(r['area_names_en']))
 assert len(rows)==52 and len({identity(r)for r in rows})==52
 names={'objects':'ff5_objects.jsonl','scripts':'ff5_scripts.jsonl','maps':'ff5_master/map.txt','script_master':'ff5_master/script.txt','areas':'ff5_master/area.txt','english_names':'ff5_master/system_en.txt'}
 for role,name in names.items():
  digest=hashlib.sha256()
  with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
   for chunk in iter(lambda:f.read(1024*1024),b''):digest.update(chunk)
  assert digest.hexdigest()==sources[role],(role,'primary source changed')
 authored=[]
 for r in rows:
  kind,guides,reason=disposition(r)
  authored.append(dict(game='ff5',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(r['submap']+'/'+r['script'],{}).get('sha256'),disposition=kind,guides=guides,reason=reason,
   evidence='Exact immutable discovery identity/current six primary hashes;44 body hashes and operation counts in the completed-family checkpoint; closed Library/Catapult/Ghido/Surgate native/route evidence indexed by docs/re/FF5_PUZZLE_GUIDES_20261101.md.'))
 counts=Counter(r['disposition']for r in authored)
 assert counts=={'guided mechanic':31,'decorative counterpart':5,'optional hint':1,'ordinary reward':5,'ordinary story scene':5,'ordinary gate interaction':4,'ordinary examination':1},counts
 path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';review=json.loads(path.read_text(encoding='utf-8'));expected=dict(review)
 replacements={identity(r):r for r in authored};seen=set();expected['decisions']=[]
 for old in review['decisions']:
  if old['game']=='ff5'and identity(old)in replacements:
   assert identity(old)not in seen;seen.add(identity(old));expected['decisions'].append(replacements[identity(old)])
  else:expected['decisions'].append(old)
 expected['decisions'].extend(r for r in authored if identity(r)not in seen)
 if '--check'in sys.argv:assert review==expected,'Completed-family dispositions differ'
 else:path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFV52 explicit candidate dispositions, six current primary hashes:',dict(counts))
if __name__=='__main__':main()
