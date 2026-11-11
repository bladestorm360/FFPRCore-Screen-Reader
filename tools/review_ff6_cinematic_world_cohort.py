"""163 fixed discovery identities; no area/name-based completeness shortcut."""
from pathlib import Path
import json,hashlib,sys,collections,csv
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    from check_scripted_bakes import decode
    manifest=json.loads((ROOT/'docs/re/ff6_cinematic_world_cohort_20261103.json').read_text(encoding='utf-8'))
    specs=manifest['identities'];assert len(specs)==len({key(r)for r in specs})==163
    for role,name in{'objects':'ff6_objects.jsonl','script_master':'ff6_master/script.txt','maps':'ff6_master/map.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
        assert h.hexdigest()==manifest['source_sha256'][role],role
    maps={int(r['id']):r for r in csv.DictReader((ROOT.parent/'tools/mapdump/ff6_master/map.txt').open(encoding='utf-8-sig'))}
    for expected in manifest['native_movie_maps']:
        native=maps[expected['map']]
        assert native['map_name']==expected['name'] and int(native['moving_availability'])==expected['moving_availability']==0
    candidates={}
    for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        for r in json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6']['candidates']:
            candidates.setdefault(key(r),[]).append(r)
    baked,_=decode(ROOT/'data/ff6/maps.bin');authored=[];counts=collections.Counter()
    for spec in specs:
        found=candidates[key(spec)];assert len(found)==1;r=found[0]
        assert all(r[k]==spec[k]for k in('type','action','script','cell'))
        if r['type']=='AnimEntity':
            assert r['submap']in('Map_40006','Map_40006_1')and r['action']==0 and r['script']==''
            assert r['raw']in('天井照明','ライト','ライト反転')and not spec['properties']['collision']
            assert spec['properties']['script_id']==spec['properties']['action_id']==spec['properties']['interactive_icon_type']==0
            kind='cinematic scenery';guides=[];reason='Exact non-interactive cinematic lighting prop: no action/script/icon or collision, in native movement-disabled movie map578/586. This is neither a separate player-operated puzzle nor a story barrier. Its source identity and position remain explicit.'
        else:
            assert (r['submap'],r['entity'])in(('Map_10010',121),('Map_10020',99))and r['type']=='GotoMap'and r['action']==3 and not r['script']
            p=spec['properties'];assert(p['map_id'],p['point_id'])in((112,1),(290,1))
            assert any(o[0]==3 and o[6:9]==(p['map_id'],p['point_id'],r['entity'])and o[10:]==(r['raw'],r['group'])for o in baked[r['submap']]['objects'])
            kind='ordinary indexed exit';guides=['opera']if p['map_id']==112 else[];reason='Exact native action3 GotoMap, with no independent script or puzzle operation. The ordinary baked exit already retains its native destination/point; arrival and applicable story gates remain owned by the existing route/field state model. The ruined-world Opera entrance is distinct from the earlier opera performance.'
        counts[kind]+=1
        authored.append(dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=None,disposition=kind,guides=guides,reason=reason,evidence='docs/re/FF6_CINEMATIC_WORLD_COHORT_20261103.md; fixed163-identity manifest, exact object/script-master/map hashes, native movement-disabled map records and both baked exit tuples.'))
    assert counts=={'cinematic scenery':161,'ordinary indexed exit':2},counts
    p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert new==old
    else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FFVI exact cinematic/world cohort:161 non-interactive lights/two existing exits; all positions/identities retained.')
if __name__=='__main__':main()
