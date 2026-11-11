"""Exact hidden-control and native joining-scene counterpart dispositions."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rows={};facts={};sources=None
    rooms={'Map_30700','Map_30710','Map_30711_1','Map_30711_2','Map_30720','Map_30721_1','Map_30730','Map_30731_1','Map_30740','Map_30741_1','Map_30750','Map_30751_1'}
    for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if r['submap']in rooms:rows.setdefault(key(r),[]).append(r)
    authored=[]
    for identity,variants in rows.items():
        assert len(variants)==1;r=variants[0]
        if r['script']=='sc_e_0878':
            assert(r['submap'],r['entity'],r['group'])==('Map_30711_1',5,'ev_e_0878')
            disposition='ordinary native interaction';guides=[];reason='One hidden switch records421 and opens the passage. Chests, the summit battle and descent use ordinary navigation.'
        else:
            assert r['script']=='sc_e_0776'and r['submap']=='Map_30700'and r['group']=='ev_e_0776'and r['entity']in(25,26,27,28,29,30,31,34)
            disposition='native story handoff';guides=[];reason='One-time joining-scene contacts are enabled by native character9 membership while392 is clear. A source-qualified unknown-branch projection retains common barrier absence after392, and actual no-Relm current-floor capture remains traversable before recruitment.'
        authored.append(dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts[r['submap']+'/'+r['script']]['sha256'],disposition=disposition,guides=guides,reason=reason,evidence='docs/re/CULTISTS_TOWER_20261103.md; exact native bodies/branch operand,24 complete production route cases/63 floor continuations and native-ready completion/controller checks.'))
    assert len(authored)==9
    for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
        assert h.hexdigest()==sources[role],role
    p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert new==old
    else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Cultists Tower: nine exact control/native-scene decisions; all six source fingerprints verified.')
if __name__=='__main__':main()
