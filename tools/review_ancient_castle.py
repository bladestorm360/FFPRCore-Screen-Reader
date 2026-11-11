"""Exact Ancient Castle mechanism/gate/clue counterpart dispositions."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rows={};facts={};sources=None
    for filename in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6']
        if sources is not None:assert sources==d['source_sha256']
        sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if r['submap'].startswith('Map_30811_'):rows.setdefault(key(r),[]).append(r)
    old=json.loads((ROOT/'docs/re/puzzle_semantic_review_20261101.json').read_text(encoding='utf-8'));existing={key(r):r for r in old['decisions']if r['game']=='ff6'};authored=[]
    for identity,variants in rows.items():
        if identity in existing and'ANCIENT_CASTLE_20261103.md'not in existing[identity]['evidence']:continue
        assert len(variants)==1;row=variants[0];script=row['script']
        if script=='sc_e_0886':
            assert row['submap']=='Map_30811_5'and row['entity']in(2,30)
            disposition='guided mechanic';reason='The enabling statue records428 and makes the final interaction available. NPC2 and side trigger30 share the same native operation; the guide targets NPC2, not a second invented action.'
        elif script=='sc_e_0887':
            assert row['entity']==12;disposition='guided mechanic';reason='Hidden switch records429, animates the actual lower passage via518 and returns control through the native callback. Guide then selects the real final statue, not the unreachable construction-scenery cell.'
        elif script=='sc_e_0890':
            assert row['submap']=='Map_30811_7'and row['entity']==10
            disposition='guided mechanic';reason='Actual final statue requires enabling428 and records432. Root432 and field readiness finish the complete objective; proximity, opened stairs or entering the last room are insufficient.'
        elif script=='sc_e_0888':
            assert row['entity']==6;disposition='optional clue';reason='Native diary430 supplies the story clue. It is not a prerequisite for432; the guide gives the actual hidden-switch solution and full statue objective directly.'
        else:
            assert not script and row['action']==0 and row['group']in('ev_e_0887','ev_e_0887_1'),identity
            disposition='gate counterpart';reason='Native hidden-stair construction/collision counterpart, with no separate action script. The old14,32 construction waypoint is unreachable; actual Ground/Bottom passage and all return/resume routes use native429 geometry without a bake change.'
        authored.append(dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(row['submap']+'/'+script,{}).get('sha256'),disposition=disposition,guides=['ancient_castle_hidden_stairs'],reason=reason,evidence='docs/re/ANCIENT_CASTLE_20261103.md; eight pinned native source bodies, real three-operation recipe, full physical/resume/negative/floor continuations and actual generic-controller receipt/readiness checks.'))
    assert len(authored)==26,('Exact original discoveries',len(authored))
    for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
        assert h.hexdigest()==sources[role],role
    replacement={key(r):r for r in authored};new=dict(old);new['decisions']=[];seen=set()
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert new==old
    else:(ROOT/'docs/re/puzzle_semantic_review_20261101.json').write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Ancient Castle:26 exact mechanic/gate/clue dispositions; no unrelated prior decision changed.')
if __name__=='__main__':main()
