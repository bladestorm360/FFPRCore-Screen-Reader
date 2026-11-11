"""Eight exact original mansion discoveries; native source stays external."""
from pathlib import Path
import json,hashlib,sys
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rooms={'Map_30631_'+str(i)for i in range(1,6)};rows={};facts={};sources=None
    for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if r['submap']in rooms:rows.setdefault(key(r),[]).append(r)
    reasons={15:'Flower-painting battle381 is one stage, not completion of the entire mansion.',19:'Native warning379 enables the diary577, then lights380 and the painting corridor.',18:'Native diary577 enables this reversible light control; on380 is required before the painting journey.',
        56:'Exact automatic painting transfer ends at Ground6,24, writes area1 and local1, and leads to the four hanging-chest encounters and door painting.',57:'Actual painting doorway transfer ends at Ground20,18. Its source cell is blocked by the final painting until389; full continuation then reaches the boss room.',
        88:'Native start trigger arms the one-second three-door cycle. Shift+G routes to the left staging cell and requires explicit readiness before a native phase0 Go cue.',55:'The true left door stops the native timer and transfers to Ground25,32. Closed collider85 is retained; neither wrong door is substituted.',
        8:'Native bookshelf391 joins Relm and returns to the town. Boss390 alone is not completion; independent385-388 hanging-chest receipts remain guidable on a later visit.'}
    authored=[]
    for identity,variants in rows.items():
        assert len(variants)==1;r=variants[0];assert r['entity']in reasons
        authored.append(dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts[r['submap']+'/'+r['script']]['sha256'],disposition='guided mechanic',guides=['owzers_mansion'],reason=reasons[r['entity']],evidence='docs/re/OWZERS_MANSION_20261103.md; exact native scripts/collider/timer evidence, production routes and actual controller checks.'))
    assert len(authored)==8
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
    print('Owzer: eight exact original discoveries; matching six source fingerprints and prior decisions retained.')
if __name__=='__main__':main()
