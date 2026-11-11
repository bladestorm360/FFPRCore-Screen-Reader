"""Explicit78-identity camp/waterfall dispositions; source bodies stay external."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(row):return row['submap'],row['group'],row['entity'],row['raw']
def main():
 rows={};facts={};sources=None
 for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
  inventory=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==inventory['source_sha256'];sources=inventory['source_sha256'];facts.update(inventory['script_facts'])
  for row in inventory['candidates']:
   if row['submap']in('Map_30410','Map_30470'):rows.setdefault(key(row),[]).append(row)
 assert len(rows)==78
 proof=json.loads((ROOT/'docs/re/sabin_scenario_native_20261103.json').read_text(encoding='utf-8'));authored=[]
 for identity,variants in rows.items():
  row=variants[0];script=row['script'];locator=row['submap']+'/'+script;digest=facts[locator]['sha256'];assert digest==proof['source_script_hashes'][locator]
  if script in('sc_e_0111_on','sc_e_0111_off'):
   assert row['submap']=='Map_30410'and row['type']=='Event'and row['action']==2
   kind='native visual-mask region counterpart';reason='Thirty physical on/off regions alter the same visual scope/local3 and exchange trigger visibility. The Closed exact native four-command mask projection already qualifies rendering-only operations and both dog visibility/pose branches. These are automatic presentation regions, not thirty ordered puzzle steps. Native collision/visibility and bounded current observation remain unchanged.'
  elif script=='sc_e_0111':
   kind='ordinary native chest choice';reason='One native three-choice chest interaction: selection0 starts the dog encounter and its continuation82, selection1 has its different scene/83 receipt, selection2 leaves. Choices are already accessible native prompts, not a spatial/order puzzle or a story unlock prerequisite. Keep type-only treasure labels and the actual native outcome/collision; do not invent an automatic successful choice.'
  elif script in('sc_e_0114','sc_e_0116'):
   kind='ordinary native story interaction';reason='One current scene/NPC conversation advances84/86 and the subsequent native encounter scripts. Existing native actor availability, contact/interaction, battle and scene continuations own progression. No independent ordered map control or vehicle predicate exists in these two operations.'
  elif script=='sc_e_0133':
   kind='native story exit counterpart';reason='Three equivalent escape regions perform the native Magitek dismount/party scene, set97 and leave to world1/PointIn11. Optional Shadow branches remain native-owned. This is a story-stage exit, not an additional vehicle requirement; no ability, collision, group or transfer metadata is changed by this semantic review.'
  elif script=='sc_e_0216':
   kind='automatic native story conversation';reason='Ten physical regions trigger the same waterfall conversation/optional Shadow departure and124 receipt. They are counterparts of one automatic scene, not ten independent puzzles or ten required visits. The earlier/later group guards and native party operation remain authoritative.'
  elif script=='sc_e_0218':
   kind='first-visit native battle exit counterpart';reason='Sixteen first-visit jump/reset regions share the same two-choice exit. The first choice starts native encounter80, whose0218_1 victory continuation changes to376/PointIn1 and0218_2. That native beach/name scene sets125 before returning control on376. It is not an immediate world1 transfer and is never replaced by the later return-visit exit. Decline/reset area1 and native trigger visibility remain unchanged; no missing multi-control puzzle is inferred from the duplicate regions.'
  else:
   assert script=='sc_e_0269'
   kind='return-visit native exit counterpart';reason='Sixteen later jump/reset regions use the same ordinary native prompt and return directly to world1/PointIn37. Source initializer selects this family only after125; first-visit groups remain separate. Preserve area1 decline/reset semantics and the actual first choice, rather than granting this transfer before the mandatory first-visit battle/scene.'
  result=dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=digest,disposition=kind,guides=[],reason=reason,evidence='docs/re/SABIN_SCENARIO_REVIEW_20261103.md; exact current source/initializer guards and continuation hashes, preserved native data, Closed mask producer evidence/current actual mask consumer.')
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
 print('Imperial Camp/Baren Falls:78 exact original identities;',sum(map(len,rows.values())),'physical instances; all prior decisions retained; no guide/runtime/data change.')
if __name__=='__main__':main()
