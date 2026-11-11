"""Ten exact clock/hidden-reward discoveries; other Zozo records stay separate."""
from pathlib import Path
import json,hashlib,sys,collections
ROOT=Path(__file__).resolve().parents[1]
def key(r):return r['submap'],r['group'],r['entity'],r['raw']
def main():
    rows={};facts={};sources=None
    for name in('puzzle_mechanics_inventory_20261031.json','puzzle_mechanics_animation_inventory_20261031.json'):
        d=json.loads((ROOT/'docs/re'/name).read_text(encoding='utf-8'))['games']['ff6'];assert sources is None or sources==d['source_sha256'];sources=d['source_sha256'];facts.update(d['script_facts'])
        for r in d['candidates']:
            if r['submap']in('Map_30511_3','Map_30511_4'):rows.setdefault(key(r),[]).append(r)
    assert len(rows)==10
    authored=[]
    for identity,variants in rows.items():
        assert len(variants)==1;r=variants[0];room=r['submap'];id=r['entity']
        if room=='Map_30511_4':
            assert id==3 and r['type']=='Entity'and r['action']==4 and not r['script'];kind='optional hint';reason='Message-only clue clock, not the solution control. The actual guide routes to386 native Point1 before resolving its local wall/clock state.'
        elif id in(5,6,7,8,9):
            assert r['type']=='Entity'and r['action']==0 and not r['script'];kind='physical gate counterpart';reason='One exact solid wall actor shown while local1 is not3 and hidden by the successful clock or native inside-opening event. It is retained as a real gate, not an independent interaction; closed-side chest routes fail until opened.'
        elif id==16:
            assert r['type']=='AnimEntity'and r['action']==0 and not r['script'];kind='decorative counterpart';reason='Non-colliding animated wall counterpart moved by the native successful opening; no separate player operation.'
        elif id==11:
            assert r['script']=='sc_e_0299';kind='guided mechanic';reason='Actual three-choice clock. Complete native table proves positions3/1/5 among150 combinations; map-local1=3 opens the wall but does not finish the guide. Shift+G repeats the cached instruction during native choices.'
        elif id==14:
            assert r['type']=='TreasureBox'and r['action']==6 and not r['script'];kind='guided mechanic';reason='Actual hidden reward chest. Native treasure115, not proximity or the opened doorway, completes the puzzle; reward contents stay undisclosed.'
        else:
            assert id==10 and r['script']=='sc_e_0300';kind='native geometry handoff';reason='Automatic inside-wall contact writes local1=3 and opens the same wall. Arrival behind the wall can already reach the chest; the planner must not force a detour to the outside clock.'
        authored.append(dict(game='ff6',**{k:r[k]for k in('submap','group','entity','raw','script')},script_sha256=facts.get(room+'/'+r['script'],{}).get('sha256'),disposition=kind,guides=['zozo_clock_reward'],reason=reason,evidence='docs/re/ZOZO_CLOCK_REWARD_20261103.md; exact native body/choice/property hashes,12 actual route cases/two continuations,real model/controller/input and all150 choice combinations.'))
    for role,name in{'objects':'ff6_objects.jsonl','scripts':'ff6_scripts.jsonl','maps':'ff6_master/map.txt','script_master':'ff6_master/script.txt','areas':'ff6_master/area.txt','english_names':'ff6_master/system_en.txt'}.items():
        h=hashlib.sha256()
        with(ROOT.parent/'tools/mapdump'/name).open('rb')as f:
            for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
        assert h.hexdigest()==sources[role],role
    p=ROOT/'docs/re/puzzle_semantic_review_20261101.json';old=json.loads(p.read_text(encoding='utf-8'));new=dict(old);replacement={key(r):r for r in authored};seen=set();new['decisions']=[]
    for r in old['decisions']:
        if r['game']=='ff6'and key(r)in replacement:new['decisions'].append(replacement[key(r)]);seen.add(key(r))
        else:new['decisions'].append(r)
    new['decisions'].extend(r for r in authored if key(r)not in seen)
    if'--check'in sys.argv:assert new==old
    else:p.write_text(json.dumps(new,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Zozo clock:ten exact control/gate/hint/reward identities; all prior decisions and six other Zozo candidates retained.')
if __name__=='__main__':main()
