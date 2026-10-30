"""Finite October29 contextual review, not runtime/general-purpose inference.

Only the original245 inventoried families can enter this authoring pass. Whole
source/continuation hashes pin each decision; game bodies stay outside Git.
Run without --apply to inspect dispositions; --apply writes reviewed metadata.
"""
from pathlib import Path
import collections
import hashlib
import json
import sys

ROOT=Path(__file__).resolve().parents[1]
DUMP=ROOT.parent/'tools/mapdump'
CONTEXT=ROOT.parent/'tools/map_event_audit_20261028'


def sha(value):
    return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()


def decide(n,name):
    if n==1:
        return 'narrative',None,'opening bridge presentation or destruction of the canal barrier; multi-actor story staging'
    if n==2:
        if name.startswith('sc_map_'):
            return 'existing_exit',None,'native physical world exit already indexed separately; arrival can start a narrative scene'
        return 'narrative',None,'airship capture and vehicle loss; silent story staging'
    if n==3:
        if name in {'sc_e_0025','sc_e_0021','sc_e_0353'}:
            return 'narrative',None,'first-visit flight/destruction/abduction scene; full continuation is narrative despite no dialogue'
        if name in {'sc_e_0337','sc_e_0339','sc_e_0341','sc_e_0344'}:
            return 'existing_provider',None,'accepted continent provider owns the named destination; native helper is suppressed'
        if name in {'sc_e_0351','sc_e_0347','sc_e_0349'}:
            return 'existing_exit',None,'accepted mounted underwater entrance already has ordinary/submarine exit flags'
        if name in {'sc_ship_0004','sc_ship_0007'}:
            return 'existing_provider',None,'accepted interior vehicle-return/ladder producer owns the action'
        if name in {'sc_e_0348','sc_e_0352','sc_e_0350'}:
            return 'transfer','World map transition','underwater return; native script restores submarine and world position'
        if name=='sc_e_0281':
            return 'transfer','Map entrance','lake entry movement and teleport-cache maintenance, not the later encounter'
    if n==4:
        if name in {'sc_e_0015','sc_e_0048','sc_e_0049','sc_e_0050'}:
            return 'narrative',None,'bombardment/vehicle attack presentation; multi-actor story staging'
        if name=='sc_ship_0007':
            return 'transfer','World map transition','return from airship interior to the same native vehicle'
        if name=='sc_godown_1':
            return 'existing_exit',None,'already an indexed world exit; arrival includes story staging'
        if name=='sc_e_0031':
            return 'transfer','Passage','ordinary teleport through a transient world scene; complete native chain ends at Baron player6,6'
    if n==5:
        if name.startswith(('sc_map_20070_move_','sc_map_20071_move_')):
            return 'transfer','Passage','ordinary room-to-room door; arrival color/fade and map initializer restore room state'
        if name.startswith(('sc_100','sc_20231_','sc_30031_','sc_30151_1_to_',
                            'sc_30161_1_to_','sc_30190_to_','sc_30191_5_to_',
                            'sc_30401_','sc_30411_','sc_30441_','sc_30450_to_',
                            'sc_30461_6_to_','sc_30521_9_to_','sc_30571_1_to_',
                            'sc_30591_1_to_','sc_30601_')) or name=='sc_world_3_to_20050':
            return 'transfer','Passage','ordinary named map/room transfer with movement, vehicle or map setup continuation'
        if name in {'sc_e_0220_9','sc_e_0250_1','sc_e_0108_3','sc_e_0108_4','sc_e_0108_6',
                    'sc_e_0108_8','sc_e_0108_10','sc_e_0108_12','sc_e_0252_8','sc_e_0252_6',
                    'sc_e_0323_1','sc_e_0351_1','sc_e_0509_1'}:
            return 'transfer','Passage','ordinary door/portal/waterfall transfer and its arrival animation'
        if name.startswith('sc_e_0217_'):
            return 'transfer','Water current','player is carried through a passage; actor is the moving floor/current visual'
        if name in {'sc_e_0304','sc_e_0306','sc_e_0307','sc_e_0308','sc_e_0497','sc_e_0447','sc_e_0448','sc_e_0449'}:
            return 'transfer','Vent','player crawl animation and vent-room transfer'
        if name=='sc_e_0268_1':
            return 'transfer','Passage','player warp and matching effect; no unrelated story participant'
        if name=='sc_e_0290':
            return 'transfer','Quicksand','quicksand movement and native floor transfer'
        if name=='sc_e_0321_1':
            return 'transfer','Passage','mechanical passage to a different floor, with the matching local effect'
        if name.startswith('sc_e_0278_'):
            return 'physical','Checkpoint','lap progress flag and hiding its own checkpoint actor; no story scene or reward disclosure'
        if name=='sc_e_0173_1':
            return 'physical','Service area','pub service cleanup returns the service actor and clears temporary area triggers'
        if name.startswith('sc_map_20050_gate_'):
            return 'physical','Gate','gate tiles animate and disappear; guards face the gate, without dialogue or a story transfer'
        if name.startswith('sc_e_0193_'):
            return 'physical','Puzzle trigger','three local progress flags hide their own contact markers and enable another puzzle marker; no reward name'
        if name=='sc_e_0223' or name.startswith('sc_e_0220_'):
            return 'physical','Bookshelf','bookshelf animation/visibility changes and puzzle flags'
        if name in {'sc_30060_timer_start','sc_30060_timer_stop'}:
            return 'physical','Timer start' if name.endswith('start')else 'Timer stop','native timer starts/finishes and its timer-marker actor changes presence'
        if name=='sc_e_0247_reset':
            return 'physical','Passage','leaving the low passage clears crawl condition and restores the standing actor'
        if name in {'sc_e_0266','sc_e_0266_11','sc_e_0267','sc_e_0437'}:
            return 'physical','Moving passage','floor animation and exact passage collision actors toggle; no narrative actor'
        if name=='sc_e_0438_1':
            return 'physical','Warp Tile','same-floor player movement and matching warp effect; no unrelated actor'
        if name in {'sc_e_0440','sc_e_0300','sc_e_0443','sc_e_0444'}:
            return 'physical','Coffin','coffin animation and local floor/collision reaction; no reward/enemy name'
        if name in {'sc_e_0519','sc_e_0293','sc_e_0294','sc_e_0296'}:
            return 'physical','Trap','repeatable local wall/floor hazard animation; future enemy identity withheld'
        if name in {'sc_e_0301_1','sc_e_0301_2'}:
            return 'physical','Door','exact door collision29, passage collision30 and door animation33 toggle'
        if name in {'sc_e_0043','sc_e_0488','sc_e_0094','sc_e_0095_2','sc_e_0366','sc_e_0367','sc_e_0368',
                    'sc_e_0326','sc_e_0132_1','sc_e_0490','sc_e_0491','sc_e_0132','sc_e_0144',
                    'sc_e_0415','sc_e_0415_1','sc_e_0478','sc_e_0007'}:
            return 'narrative',None,'character encounter/rescue/search/destruction or party-story staging; no plot names exposed'
    if n==6:
        if name in {'sc_e_1047','sc_e_1048','sc_e_1080_1','sc_e_1072_0','sc_e_0052_0','sc_e_0053_0',
                    'sc_e_1502','sc_e_0591','sc_e_0687'}:
            return 'transfer','Passage','ordinary secret passage/door/warp/ride transfer; arrival restores the destination mechanism'
        if name.startswith('sc_e_0092'):
            return 'physical','Light path','local path index, light markers and exact light-polyline start/stop calls; puzzle reaction'
        if name in {'sc_e_0111_on','sc_e_0111_off'}:
            return 'physical','Telescope','scope mask and corresponding far-scene visibility toggle; mask is render-only'
        if name in {f'sc_e_{v:04}'for v in range(595,603)}:
            return 'physical','Trap','repeatable collapsing ground during the existing escape timer; no narrative outcome disclosure'
        if name=='sc_e_0300':
            return 'physical','Switch','wall contact switches local floor/collision animation and local progress'
        if name=='sc_e_0906':
            return 'physical','Treasure Chest','chest movement/animation mechanism; contents remain undisclosed'
        if name in {'sc_e_0764','sc_e_0772'}:
            return 'transfer','Passage','false-door return and ordinary mansion puzzle reset; no dialogue/encounter'
        if name in {'sc_e_0766','sc_e_0766_1'}:
            return 'physical','Passage','local puzzle reposition and direction, retaining its native mechanism'
        if name=='sc_e_1080':
            return 'physical','Hidden passage','draft sound at the bedroom secret entrance; local cue markers clear without narrative dialogue or travel'
        if name=='sc_e_1050':
            return 'physical','Passage barrier','exit obstruction actor moves into place and local contact markers clear; no enemy/story identity disclosed'
        if name in {'sc_e_1078','sc_e_0538'}:
            return 'narrative',None,'one-time party/other-character surprise or escape choreography across the transfer, not an ordinary entry animation'
        if name in {'sc_e_0434','sc_e_0436','sc_e_0370','sc_e_0634','sc_e_0706','sc_e_0085_2','sc_e_0085_1',
                    'sc_e_0054','sc_e_0541','sc_e_0542','sc_e_0903','sc_e_0934'}:
            return 'narrative',None,'story character movement, discovery, warning or memory; no plot names exposed'
    return 'needs_context',None,'not yet reviewed'


def main():
    path=ROOT/'tools/world_event_reviews.json';reviews=json.loads(path.read_text(encoding='utf-8'))
    known={(r['game'],r['submap'],r['group'],r['id']):r for r in reviews}
    decisions=[];new=[]
    for n in range(1,7):
        source=[json.loads(line)for line in (DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8-sig')]
        scripts={(s['submap'],s['name']):s for s in source}
        objects={(o['submap'],o['group'],o['id']):o for o in map(json.loads,(DUMP/f'ff{n}_objects.jsonl').open(encoding='utf-8-sig'))}
        counts=collections.Counter()
        for f in json.loads((CONTEXT/f'ff{n}_context_triage.json').read_text(encoding='utf-8'))['families']:
            if f['status']!='silent_staging_or_physical_context':continue
            assert not f['gaps']
            bodies=[scripts[b['submap'],b['script']]for b in f['bodies']]
            assert all(sha(s['mnemonics'])==b['body_sha256']for s,b in zip(bodies,f['bodies']))
            assert not {m['mnemonic']for s in bodies for m in s['mnemonics']}&{'Msg','Encount','EncountBoss'}
            root=next(s for s in bodies if sha(s['mnemonics'])==f['body_sha256'])
            disposition,label,reason=decide(n,f['script']);counts[disposition]+=1
            decisions.append(dict(game=n,script=f['script'],body_sha256=f['body_sha256'],disposition=disposition,label=label,reason=reason,bodies=f['bodies']))
            if n==5 and f['script']in {'sc_30031_1_to_ship','sc_30151_1_to_fire_ship','sc_30161_1_to_world'}:
                decisions[-1]['disposition']='conditional_transfer'
            if disposition not in {'physical','transfer'}:continue
            for loc in f['occurrences']:
                obj=objects[loc['submap'],loc['group'],loc['editor_id']];p=obj['props'];key=(n,obj['submap'],obj['group'],obj['id'])
                if key in known:continue
                # Action0 is not proof that the named master script executes.
                # Preserve this exact occurrence for native action-owner review.
                inactive=p.get('action_id')!=2
                if inactive:
                    # This dormant marker has no derived Warp in any bake.
                    # Its copied master script ID is not execution evidence.
                    # Name the reliable physical floor context, never promote
                    # it to the unrelated Tycoon transfer of that unused ID.
                    assert (n,obj['submap'],p.get('entity_id'),p.get('action_id'),p.get('script_id'))==(5,'Map_30461_6',12,0,1214)
                actual=scripts.get((obj['submap'],f['script']),root)
                assert sha(actual['mnemonics'])==f['body_sha256']
                children=[b for b in f['bodies']if (b['submap'],b['script'])!=(actual['submap'],actual['name'])]
                row=dict(game=n,submap=obj['submap'],group=obj['group'],id=obj['id'],object_type=obj['type'],
                         action_id=p['action_id'],entity_id=p['entity_id'],raw=obj['name'],script_id=p['script_id'],
                         script_name=actual['name'],script_sha256=sha(actual),label=label,reason=reason,
                         review_grammar='contextual_transfer'if disposition=='transfer'else 'contextual_physical',continuations=children)
                if inactive:
                    row.update(label='Slippery floor',review_grammar='inactive_native_mechanism',reason='inactive Action0 Pyramid floor marker; dormant copied script ID does not establish a map transfer')
                if disposition=='transfer'and not inactive:
                    main=next(s for s in actual['segments']if s['Label']=='Main')
                    changes=[m['operands']for m in actual['mnemonics'][main['EntryPoint']:main['EntryPoint']+main['Count']]if m['mnemonic']=='ChangeMap']
                    assert changes,(n,f['script'])
                    if len({tuple(v['iValues'][:2])for v in changes})!=1:
                        assert n==5 and f['script']in {'sc_30031_1_to_ship','sc_30151_1_to_fire_ship','sc_30161_1_to_world'}
                        decisions[-1]['disposition']='conditional_transfer'
                        row['review_grammar']='contextual_conditional'
                        row['conditional_transfers']=[[*v['iValues'][:2],v['sValues'][0]]for v in changes]
                    else:
                        first=changes[0];row['primary_transfer']=[*first['iValues'][:2],first['sValues'][0]]
                    if n==4 and f['script']=='sc_e_0031':
                        assert row['primary_transfer']==[1,14,'sc_e_0031_1']
                        row['final_transfer']=[29,1,'sc_e_0031_2']
                    if n==5 and f['script']in {'sc_10030_to_30391_2','sc_10030_to_30381_8','sc_10050_to_20211_3',
                                               'sc_10050_to_30511_2','sc_10050_to_30611_14','sc_10050_to_30531_11'}:
                        assert any(m['mnemonic']=='SysCall'and m['operands']['sValues'][0]=='乗り物から降りる【アニメ・BGMなし】'for m in actual['mnemonics'])
                        assert len(actual['mnemonics'])==5
                        row['vehicle_entry']='submarine'
                new.append(row)
        print('FF',n,dict(counts))
    assert len(decisions)==245
    print('Context decisions',len(decisions),'new exact occurrences',len(new))
    for d in decisions:
        if d['disposition'].startswith('needs_'):print('OPEN',d['game'],d['script'],d['reason'])
    if '--apply'in sys.argv:
        reviews.extend(new);path.write_text(json.dumps(reviews,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        (ROOT/'tools/silent_family_decisions.json').write_text(json.dumps(decisions,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


if __name__=='__main__':main()
