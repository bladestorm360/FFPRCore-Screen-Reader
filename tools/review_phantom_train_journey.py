"""Explicit eighteen train-discovery dispositions after complete stage proof."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
SCRIPTS={
 'sc_e_0160':('guided mechanic','Actual platform introduction107 precedes boarding; guide selects its real Event1.'),
 'sc_e_0163':('guided mechanic','Actual departure trigger109 is reached from the native post-boarding29,11 position, not falsely completed at its initial PointIn.'),
 'sc_e_0186':('guided mechanic','Native pursuit sets111 and moves the player one north from125,3; the next guide uses that actual roof pose.'),
 'sc_e_0187':('guided mechanic','Native jump sets112 and lands the player at97,5; guide then reaches the next detachment event with one west.'),
 'sc_e_0188':('guided mechanic','Exact native planning/return variants:113 arms the first detachment lever; after114 the default return event leads back for its second operation115.'),
 'sc_e_0189':('guided mechanic','First native lever phase visits624 Point12 and records114; second phase0190 records115. The guide preserves both operations before the original engine solution.'),
 'sc_e_0208':('guided mechanic','Original left engine toggle120; retained1/0/1 solution, all eight engine states and real control/boss routes pass.'),
 'sc_e_0210':('guided mechanic','Original middle engine toggle121 must be off; all eight engine states and the real toggle route pass.'),
 'sc_e_0212':('guided mechanic','Original right engine toggle122 must be on; all eight engine states and the real toggle route pass.'),
 'sc_e_0172':('automatic scene','Sabin operates the side-car lever in native choreography and records518. The trigger is an automatic scene, not a separate manual lever order; native narration and ordinary physical event routing retain it.'),
 'sc_e_1050':('guided mechanic','Native automatic blockade records519 and enables the actual ghost conversation6. The complete entrance route reaches its trigger, then guides that physical encounter before the pursuit.'),
 'sc_e_1078_0':('automatic scene','Automatic hatch-opening marker records map-local1; it has no separate manual combination or persistent story completion. Native hatch/door geometry remains authoritative.'),
 'sc_e_0196':('ordinary recovery scene','Optional first dining-car meal records116 and restores the party through native choreography; it is a recovery interaction rather than an ordered map/story puzzle.')}
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    candidates={};facts={};sources=None
    for name in ('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']
        if sources is not None:assert sources==d['source_sha256']
        sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if any(m in range(355,373)or m==624 for m in r['maps']):candidates.setdefault(key(r),[]).append(r)
    old=json.loads((ROOT/'docs/re/puzzle_semantic_review_20261101.json').read_text(encoding='utf-8'))
    existing={key(r):r for r in old['decisions']if r['game']=='ff6'};authored=[]
    for k,variants in candidates.items():
        if k in existing and'PHANTOM_TRAIN_JOURNEY_20261103.md'not in existing[k]['evidence']:continue
        assert len(variants)==1;krow=variants[0];assert krow['script']in SCRIPTS and krow['action']==2
        disposition,reason=SCRIPTS[krow['script']]
        authored.append(dict(game='ff6',**{k:krow[k]for k in ('submap','group','entity','raw','script')},script_sha256=facts[krow['submap']+'/'+krow['script']]['sha256'],disposition=disposition,guides=['phantom_train'],reason=reason,evidence='docs/re/PHANTOM_TRAIN_JOURNEY_20261103.md;16 stage/engine/boss/departure witnesses with complete intermediate-floor replays, native-ready world1 completion and exact native source/landing/projection checks.'))
    assert len(authored)==18
    for role,name in {'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
        assert h.hexdigest()==sources[role],role
    replacement={key(r):r for r in authored};new=dict(old);seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert old==new
    else:(ROOT/'docs/re/puzzle_semantic_review_20261101.json').write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Eighteen exact Phantom Train discoveries reviewed; automatic/recovery markers stay distinct from native guide operations.')
if __name__=='__main__':main()
