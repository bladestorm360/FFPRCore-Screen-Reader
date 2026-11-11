"""Explicit two arrival-counterpart dispositions; automatic0850 is source-checked separately."""
from pathlib import Path
import hashlib,json,sys
from author_zone_eater_roof import HASHES
ROOT=Path(__file__).resolve().parents[1]
SPECS={('Map_30791_4','sc_e_0850_in1'):1,('Map_30791_6','sc_e_0850_in2'):2}
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rows=[];facts={};sources=None
    for name in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
        if sources is not None:assert sources==d['source_sha256']
        sources=d['source_sha256'];facts.update(d['script_facts']);rows.extend(r for r in d['candidates'] if (r['submap'],r['script'])in SPECS)
    assert len(rows)==2 and len({key(r)for r in rows})==2
    for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
        assert h.hexdigest()==sources[role],role
    authored=[]
    for r in rows:
        name=r['submap']+'/'+r['script'];assert r['entity']==0 and r['type']=='Event'and r['action']==2 and facts[name]['sha256']==HASHES[name]
        point=SPECS[r['submap'],r['script']]
        authored.append(dict(game='ff6',**{k:r[k]for k in ('submap','group','entity','raw','script')},script_sha256=HASHES[name],disposition='automatic passage',guides=['zone_eater_roof'],
            reason=f'Automatic ceiling-room entry to504 Point{point}; no manual switch. Exact arrival, safe-area sequence, native five-second callback, input readiness and actual505 exit are covered by the roof guide.',
            evidence='docs/re/ZONE_EATER_ROOF_20261103.md; actual incoming-floor planner witnesses; author_zone_eater_roof.py pins all34 safe markers and seven script contracts, including unbound0850.'))
    path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(path.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for row in old['decisions']:
        if row['game']=='ff6'and key(row)in replacement:new['decisions'].append(replacement[key(row)]);seen.add(key(row))
        else:new['decisions'].append(row)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if '--check'in sys.argv:assert old==new,'Roof counterpart dispositions changed'
    else:path.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FFVI: two exact automatic roof-entry counterparts reviewed;34 safety markers/automatic callback have separate complete source/route checks.')
if __name__=='__main__':main()
