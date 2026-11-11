"""Exact tomb control/counterpart dispositions; never a label-only classifier."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
REASONS={
 'sc_e_0683':'Gravestone records341 and opens the real first-water passage. Proximity never completes the native operation.',
 'sc_e_0684':'First water control records342; the native first turtle and actual Upper/Ground returns are part of the full journey.',
 'sc_e_0685':'Second gravestone records343 and removes the native passage obstruction. The guide continues through water/turtle/boss and final story stages.',
 'sc_e_0688':'Native toggle raises344/clears345 or lowers345/clears344. The guide requests the raised state and resumes correctly after a prior lowering.',
 'sc_e_0690':'Native forward turtle records area1=1 and finishes at raw79,8 Ground. Exact default actors22/23 supply reviewed manual arcs; the guide explicitly requests the ride before its next fresh state.',
 'sc_e_0691':'Native return turtle records area1=0 and finishes at raw71,13 Ground. Its manual arc supplies complete return routes without invented walking across water.',
 'sc_e_0692':'First distinct native inscription records346; identity includes its raw name because all four stones use entity0.',
 'sc_e_0693':'Second distinct native inscription records347; it is not collapsed into another entity0 stone.',
 'sc_e_0694':'Third distinct native inscription records348; all four readings precede the actual native answer menu.',
 'sc_e_0695':'Fourth distinct native inscription records349; all four receipts are necessary before the native answer menu can open.',
 'sc_e_0696':'Native answer requires346..349 and records350 only for the verified answer. All24 native table entries prove original options4/1/3/2, shrinking menus4/1/2/1. Guide continues to actual chest169 and supports a late puzzle visit after story353.',
 'sc_e_0701':'The moving guide character is a real subsequent objective, not a fixed scenery waypoint. Native527/528 advance the memory interactions; real copied NPC positions21,12 and8,24 remain reachable. Only the subsequent native353/ready world2 ends the story; movie world1 cannot finish it.'}
def main():
    rows={};facts={};sources=None
    for filename in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/filename).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if r['submap'].startswith('Map_30651_')or r['submap']=='Map_40005':rows.setdefault(key(r),[]).append(r)
    authored=[]
    for identity,variants in rows.items():
        assert len(variants)==1;row=variants[0];script=row['script']
        if script:
            assert script in REASONS,identity
            disposition='guided mechanic';reason=REASONS[script]
            if row['group']in('ev_e_0690','ev_e_0691'):
                disposition='gate counterpart';reason+=' These alternate authored group copies are not separate required controls; the actual initializer selects default/ev_e_0688, with real actor poses and area ownership.'
        else:
            assert(row['submap'],row['group'],row['entity'],row['action'])==('Map_30651_3','ev_e_0683',4,0)
            disposition='gate counterpart';reason='Non-action first-gravestone collision counterpart. Native341 and its actual group remove this obstruction; no separate door interaction or fabricated completion.'
        authored.append(dict(game='ff6',**{k:row[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(row['submap']+'/'+script,{}).get('sha256'),disposition=disposition,guides=['darills_tomb'],reason=reason,evidence='docs/re/DARILLS_TOMB_20261103.md; exact native scripts/table values, full production physical/model/controller/choice-repeat cases.'))
    assert len(authored)==17,len(authored)
    for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
        assert h.hexdigest()==sources[role],role
    path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(path.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert new==old
    else:path.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Darill:17 exact control/counterpart/moving-guide decisions; all six source roles unchanged.')
if __name__=='__main__':main()
