"""Seven exact remaining script cases and the matching dance-area sibling.

Original source/native data is retained outside Git. This authoring/check tool
does not run scripts or the game. Facts/hashes and mod-owned labels may be saved.
"""
from pathlib import Path
import collections,csv,hashlib,json,subprocess,sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
from check_scripted_bakes import decode
DUMP=ROOT.parent/'tools/mapdump'
RAW=ROOT.parent/'tools/final_script_cases_20261029'
OUT=ROOT/'build_check/final_script_cases_20261029'
def sha(value):return hashlib.sha256(json.dumps(value,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
def read_rows(path):return list(csv.DictReader(path.open(encoding='utf-8-sig')))

def main():
    OUT.mkdir(parents=True,exist_ok=True);RAW.mkdir(parents=True,exist_ok=True)
    assert subprocess.run(['git','-C',str(RAW),'rev-parse','--show-toplevel'],capture_output=True).returncode!=0
    data={}
    for n in(2,4,5,6):
        scripts=list(map(json.loads,(DUMP/f'ff{n}_scripts.jsonl').open(encoding='utf-8-sig')))
        objects=list(map(json.loads,(DUMP/f'ff{n}_objects.jsonl').open(encoding='utf-8-sig')))
        data[n]=dict(scripts=scripts,by_key={(s['submap'],s['name']):s for s in scripts},objects=objects,
                     submaps=json.loads((DUMP/f'ff{n}_submaps.json').read_text(encoding='utf-8')),
                     masters={int(r['id']):r['script_name']for r in read_rows(DUMP/f'ff{n}_master/script.txt')if r['id']},
                     maps={int(r['id']):r for r in read_rows(DUMP/f'ff{n}_master/map.txt')if r['id']},
                     bakes=decode(ROOT/f'data/ff{n}/maps.bin')[0])
    def actor(n,sm,entity):
        found=[o for o in data[n]['objects']if o['submap']==sm and o['type']==2 and o['props'].get('entity_id')==entity]
        assert len(found)==1;return found[0]
    def bank(n,sm,name):
        maps=data[n]['submaps'];local=name in maps[sm]['scripts'];resident=name in maps['Resident']['scripts']
        return 'local'if local else'resident'if resident else'absent'
    def native_case(n,obj,label=None,skip=False):
        matches=[o for o in data[n]['bakes'][obj['submap']]['objects']if o[8]==obj['props']['entity_id']and o[10:]==(obj['name'],obj['group'])]
        assert matches;shape=matches[0];mid=next(i for i,r in data[n]['maps'].items()if r['map_name']==obj['submap'])
        return dict(game=n,map_id=mid,submap=obj['submap'],entity_id=obj['props']['entity_id'],script=obj['props']['script_id'],
                    action=obj['props']['action_id'],raw=obj['name'],cell=[shape[2],shape[3]],label=label,skip=skip)
    report={};cases=[];preserved=[]
    # Missing Fynn helper has a real, exact GotoMap at the same cell. Existing
    # reviewed TransitionHelper owns suppression; the physical exit is intact.
    helper=actor(2,'Map_20041_7',96);assert bank(2,helper['submap'],'sc_transition_20041_7')=='absent'
    shapes=data[2]['bakes'][helper['submap']]['objects'];h=next(o for o in shapes if o[0]==101 and o[8]==96)
    door=next(o for o in shapes if o[0]==3 and o[8]==84)
    assert h[1:6]==door[1:6]and door[6:8]==(40,2)
    report['II_missing_helper']='no local/resident binding; exact native exit84 to40/point2 already preserved'
    cases.append(native_case(2,helper,skip=True));preserved.append(helper)
    # The absent IV prologue root is Action0, not an executable omitted event.
    prologue=actor(4,'Map_20011_1',29);assert prologue['props']['action_id']==0 and bank(4,prologue['submap'],'sc_e_0006')=='absent'
    assert not any(o[0]==100 and o[8]==29 for o in data[4]['bakes'][prologue['submap']]['objects'])
    report['IV_dormant_prologue']='Action0, no local/resident declaration and no derived Warp; narrative label retained'
    cases.append(native_case(4,prologue,'Scene trigger'));preserved.append(prologue)
    save=actor(4,'Map_30021_4',11);assert save['props']['script_id']==853 and bank(4,save['submap'],'sc_savepoint')=='resident'
    source=data[4]['by_key']['Resident','sc_savepoint'];assert len(source['mnemonics'])==7
    other=data[4]['by_key']['Map_40008','sc_savepoint'];assert len(other['mnemonics'])==3
    assert [m['operands']['sValues'][0]for m in source['mnemonics']if m['mnemonic']=='Msg']==[m['operands']['sValues'][0]for m in other['mnemonics']if m['mnemonic']=='Msg']
    report['IV_savepoint']='native local miss selects Resident7; unrelated Map_40008/3 is not the fallback bank'
    cases.append(native_case(4,save,'Save Point'));preserved.append(save)
    dancers=[actor(5,'Map_20051_2',i)for i in(54,55,56)]
    for obj in dancers:
        p=obj['props'];name=data[5]['masters'][p['script_id']]
        assert obj['group']=='ev_e_0126'and p['action_id']==2 and bank(5,obj['submap'],name)=='absent'
        assert not any(o[0]==100 and o[8]==p['entity_id']for o in data[5]['bakes'][obj['submap']]['objects'])
        cases.append(native_case(5,obj,'Dance floor'));preserved.append(obj)
    report['V_dance_missing_roots']='55/56 have no loaded-map/resident roots; sibling54 also lacked a native binding and had an incorrect foreign barrier review; no invented transfer'
    # Native VI handler searches active coincident PropertyEvent actors and
    # returns their positive live script ID. The prompt/chase itself is already
    # narrative; dynamic contact is not an unconditional physical map transfer.
    trigger=data[6]['by_key']['Map_20040','sc_e_1055']
    assert any(m['mnemonic']=='Msg'for m in trigger['mnemonics'])
    changes=[m for m in trigger['mnemonics']if m['mnemonic']=='ChangeScriptAtOnTrigger']
    assert len(changes)==1 and changes[0]['operands']['iValues'][:2]==[0,-1001]
    related=[o for o in data[6]['objects']if o['submap']=='Map_20040'and o['props'].get('script_id')==1163]
    assert len(related)==8
    for obj in related:cases.append(native_case(6,obj,'Scene trigger'))
    report['VI_live_contact']='native IsDuplicateEntityOnTrigger supplies a live positive actor ScriptId to master lookup/Integrator.ChangeScript; false contact returns normally; root prompt/chase remains narrative'
    # Replay only actual Main control transfer/flag flow after successful battles.
    # Puppet movement/animation is not simulated or executed. Source callbacks
    # omitted by the old syntactic cycle report are included through their names.
    maps=data[6]['maps'];assert maps[207]['area_id']==maps[212]['area_id']=='77'
    scc_names={'sc_e_0650','sc_e_0650_1','sc_e_0650_2','sc_e_0650_2_1','sc_e_0650_3'}
    traces=[]
    for initial in(0,1):
        area=initial;current=data[6]['by_key']['Map_20341_8','sc_e_0650'];map_id=212;pc=0;stack=[];seen=set();trace=[];battles=[]
        for step in range(8192):
            key=(current['name'],pc,area,tuple(stack));assert key not in seen,'Actual unchanged control state cycles'
            seen.add(key);m=current['mnemonics'][pc];op=m['mnemonic'];v=m.get('operands')or{};iv=v.get('iValues')or[];sv=v.get('sValues')or[]
            if op=='Branch':
                assert sv[:3]==['AreaLocalVariable','＝','imm']and iv[:2]==[2,1]
                pc=iv[2]if area==1 else pc+1;continue
            if op=='Calc':assert sv[:3]==['AreaLocalVariable','←','imm']and iv[:2]==[2,1];area=1
            if op=='Call':stack.append(pc+1);pc=iv[0];continue
            if op=='Return':assert stack;pc=stack.pop();continue
            if op in('ChangeMap','EncountBoss'):
                name=sv[0];assert name in scc_names
                if op=='ChangeMap':assert maps[iv[0]]['area_id']=='77';map_id=iv[0]
                else:battles.append(iv[0])
                trace.append(dict(source=current['name'],node=pc,kind=op,target=name,area2=area))
                current=data[6]['by_key'][maps[map_id]['map_name'],name];pc=0;stack=[];continue
            if op=='Exit':assert not stack and current['name']=='sc_e_0650_3';break
            pc+=1
        else:raise AssertionError('Native scene proof exceeds its finite instruction bound')
        assert battles==([421,422]if initial==0 else[422])and area==1
        traces.append(dict(initial_area2=initial,instructions=step+1,battles=battles,final=current['name'],trace=trace))
    boss=actor(6,'Map_20341_8',44);cases.append(native_case(6,boss,'Scene trigger'));preserved.append(boss)
    report['VI_cycle']='five source bodies; flag AreaLocal2 changes0→1 in shared area77; post-win paths terminate at sc_e_0650_3 after2 or1 battles, not an unchanged-state cycle'
    raw=dict(ownership_actors=preserved,savepoint_sources=[source,other],live_contact=trigger,scc_sources=[s for s in data[6]['scripts']if s['name']in scc_names],scc_traces=traces)
    (RAW/'contexts.json').write_text(json.dumps(raw,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    (OUT/'cases.json').write_text(json.dumps(cases,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    if '--apply'in sys.argv:
        path=ROOT/'tools/world_event_reviews.json';reviews=json.loads(path.read_text(encoding='utf-8'))
        bad=[r for r in reviews if (r['game'],r['submap'],r['entity_id'])==(5,'Map_20051_2',54)]
        if bad:
            assert len(bad)==1 and bad[0]['script_name']=='sc_map_10020_barrier'
            reviews.remove(bad[0]);(OUT/'retired_foreign_binding.json').write_text(json.dumps(bad,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        key=(4,save['submap'],save['group'],save['id'])
        if not any((r['game'],r['submap'],r['group'],r['id'])==key for r in reviews):
            p=save['props'];reviews.append(dict(game=4,submap=save['submap'],group=save['group'],id=save['id'],object_type=2,action_id=2,
                entity_id=p['entity_id'],raw=save['name'],script_id=853,script_name='sc_savepoint',script_sha256=sha(source),
                label='Save Point',reason='native local/resident lookup selects the complete Resident save-point message and effect; unrelated map variant is inaccessible',continuations=[]))
        path.write_text(json.dumps(reviews,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        path=ROOT/'tools/event_identity_reviews.json';identities=json.loads(path.read_text(encoding='utf-8'))
        for obj in dancers:
            p=obj['props'];key=(5,obj['submap'],obj['group'],obj['id'])
            if any((r['game'],r['submap'],r['group'],r['id'])==key for r in identities):continue
            identities.append(dict(game=5,submap=obj['submap'],group=obj['group'],id=obj['id'],raw=obj['name'],entity_id=p['entity_id'],
                script_id=p['script_id'],message_key=p.get('message_key',''),asset_id=p.get('asset_id',0),label='Dance floor',layer=obj['layer'],
                evidence=dict(kind='exact native dance-collision area; master name is not bound in loaded or resident scripts',object_sha256=sha(obj))))
        path.write_text(json.dumps(identities,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    (OUT/'source_report.json').write_text(json.dumps(dict(cases=report,post_win_traces=[{k:v for k,v in t.items()if k!='trace'}for t in traces],native_cases=len(cases)),indent=2)+'\n',encoding='utf-8')
    print('Seven fixed cases plus matching dancer54: native owner facts,8 contact triggers and two finite post-win paths verified; native cases',len(cases))

if __name__=='__main__':main()
