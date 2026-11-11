"""Three exact discovery decisions; jump/contact supplements are source/route checked."""
from pathlib import Path
import hashlib,json,sys
from author_zone_eater_crossings import HASHES
ROOT=Path(__file__).resolve().parents[1]
SPECS={
 ('Map_30791_3','sc_e_0849'):(8,'zone_eater_bridges','Native local1 receipt opens the blocked return; request selects the actual floor switch first, then its exit. Guard contact0833 has a real fall/recovery witness.'),
 ('Map_30791_7','sc_e_0857'):(25,'zone_eater_stepping_stones','Native scenario409 raises the footings and hides the two barriers. Complete manual-jump replays from both physical arrivals reach this switch and the later exit.'),
 ('Map_30791_8','sc_e_0865'):(1,'zone_eater_stepping_stones','Native physical Gogo conversation includes naming/joining. Completion uses scenario410 and field readiness, never proximity or a premature naming-dialogue receipt.')}
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rows=[];facts={};sources=None
    for name in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
        if sources is not None:assert sources==d['source_sha256']
        sources=d['source_sha256'];facts.update(d['script_facts']);rows.extend(r for r in d['candidates']if(r['submap'],r['script'])in SPECS)
    assert len(rows)==3 and len({key(r)for r in rows})==3
    for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
        assert h.hexdigest()==sources[role],role
    authored=[]
    for r in rows:
        entity,guide,reason=SPECS[r['submap'],r['script']];locator=r['submap']+'/'+r['script']
        assert r['entity']==entity and r['action']==2 and facts[locator]['sha256']==HASHES[locator]
        authored.append(dict(game='ff6',**{k:r[k]for k in ('submap','group','entity','raw','script')},script_sha256=HASHES[locator],disposition='guided mechanic',guides=[guide],reason=reason,
            evidence='docs/re/ZONE_EATER_CROSSINGS_20261103.md; actual model/production-route/native-ready checks; author_zone_eater_crossings.py verifies eight state scripts and65 reviewed directional landings.'))
    path=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(path.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for row in old['decisions']:
        if row['game']=='ff6'and key(row)in replacement:new['decisions'].append(replacement[key(row)]);seen.add(key(row))
        else:new['decisions'].append(row)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if '--check'in sys.argv:assert old==new,'Crossing discovery dispositions changed'
    else:path.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FFVI: three exact guided discovery decisions; manual jump/contact supplements independently source/route checked.')
if __name__=='__main__':main()
