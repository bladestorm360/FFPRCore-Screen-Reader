"""Exact external dream-train control/initializer equivalence, factual output.

No recovered body or instruction program is written into the repository.
This narrow abstract observer never executes a game or changes game state.
"""
import hashlib,json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
WANTED={'sc_map_30831_3','sc_e_0917','sc_e_0919','sc_e_0921','sc_e_0929'}
FIRST={'sc_e_0904','sc_e_0904_1','sc_e_0904_t'}
scripts={}
for line in (ROOT.parent/'tools/mapdump/ff6_scripts.jsonl').open(encoding='utf-8'):
    row=json.loads(line)
    if (row['submap']=='Map_30831_3' and row['name'] in WANTED or
        row['submap']=='Map_30831_2' and row['name'] in FIRST):scripts[row['name']]=row['mnemonics']
assert scripts.keys()==WANTED|FIRST

def observe(name,flags,locals_=None,positions=None,effects=None):
    positions=dict(positions or {});locals_=list(locals_ or [0]*8);puppets={}
    if effects is None:effects={}
    body=scripts[name]
    def value(bank,index):
        if bank=='imm':return index
        if bank=='MapLocalVariable':return locals_[index]
        if bank.startswith('ScenarioFlag'):return flags[index+256*(int(bank[-1])-1)]
        raise AssertionError(('unreviewed value bank',name,bank))
    def execute(pc,actor=None):
        stack=[]
        for _ in range(4096):
            assert 0<=pc<len(body),(name,pc)
            m=body[pc];pc+=1;op=m['mnemonic'];v=m.get('operands',{});iv=v.get('iValues',[]);sv=v.get('sValues',[])
            if op=='Branch':
                lhs=value(sv[0],iv[0]);rhs=value(sv[2],iv[1]);assert sv[1]=='＝'
                if lhs==rhs:pc=iv[2]
            elif op=='Jump':pc=iv[0]
            elif op=='Call':stack.append(pc);pc=iv[0]
            elif op=='Return':
                if not stack:return
                pc=stack.pop()
            elif op=='Exit':return
            elif op in ('SetFlag','ResetFlag'):
                assert sv[0].startswith('ScenarioFlag');flags[iv[0]+256*(int(sv[0][-1])-1)]=int(op=='SetFlag')
            elif op=='Calc':
                assert sv[:3]==['MapLocalVariable','←','imm'];locals_[iv[0]]=iv[1]
            elif op=='SetPuppet':puppets[iv[0]]=(iv[1],iv[2])
            elif op=='ExecPuppet':
                pending=list(puppets.values());puppets.clear()
                for puppet,entry in pending:execute(entry,puppet)
            elif op=='SetTimer':
                assert name=='sc_e_0904' and v['rValues'][0]==4
                effects['timer_running']=True;effects['timer_seconds']=4
            elif op=='FinishTimer':effects['timer_running']=False
            elif op=='Hide':
                assert actor is not None and actor>0
                effects.setdefault('hidden',set()).add(actor)
            elif op=='SetPos':
                assert actor is not None
                if actor>0 and iv[2]==0 and iv[3]==0:positions[actor]=(iv[0],iv[1])
                # The chest's native relative placement is not maze geometry.
                else:assert name=='sc_e_0929'
            else:
                assert op in {'Nop','Blank','Wait','CueAnim','PlayAnim','PlaySE','PlayEnvSe','ScreenEffect','Shake','SetEntities'},(name,op)
        raise AssertionError(('instruction bound',name))
    execute(0)
    return flags,locals_,positions

rows=[]
for bits in range(8):
    flags=[0]*1024
    for i in range(3):flags[445+i]=(bits>>i)&1
    _,_,initial=observe('sc_map_30831_3',flags.copy())
    for i,name in enumerate(['sc_e_0917','sc_e_0919','sc_e_0921']):
        changed,_,positions=observe(name,flags.copy(),positions=initial)
        result=sum(changed[445+j]<<j for j in range(3));assert result==bits^(1<<i)
        _,_,expected=observe('sc_map_30831_3',changed.copy())
        # All five position-controlled native wall actors must agree, not
        # merely the toggle bit or one visually representative obstacle.
        assert len(expected)==5,(bits,len(expected))
        assert positions==expected,(bits,name,positions,expected)
        rows.append(dict(initial_bits=bits,lever=i,result_bits=result,wall_positions_match=True))
success=[]
for mask in range(64):
    flags=[0]*1024
    changed,_,_=observe('sc_e_0929',flags,[(mask>>(i-1))&1 if i else 0 for i in range(8)])
    if changed[448]:success.append(mask)
assert success==[28],success
flags=[0]*1024;effects={}
_,first,positions=observe('sc_e_0904',flags,positions={28:(27,8),31:(29,8)},effects=effects)
assert first[0]==1 and positions[28]==(27,9) and effects['timer_running'] and effects['timer_seconds']==4
_,second,locked=observe('sc_e_0904',flags,first,positions,effects)
assert second[0]==2 and locked[28]==(27,8) and locked[31]==(29,9) and not effects['timer_running'] and 26 in effects['hidden']
_,third,unchanged=observe('sc_e_0904',flags,second,locked,effects)
assert third==second and unchanged==locked and not effects['timer_running']
_,finished,_=observe('sc_e_0904_1',flags,second,locked,effects)
assert finished[0]==3 and not effects['timer_running'] and {26,106}<=effects['hidden']
_,unstarted,_=observe('sc_e_0904_1',flags,[0]*8)
assert unstarted[0]==0
timeout_effects={'timer_running':True}
_,reset,reset_positions=observe('sc_e_0904_t',flags,first,positions,timeout_effects)
assert reset[0]==0 and reset_positions[28]==(27,8) and not timeout_effects['timer_running']
_,retry,_=observe('sc_e_0904',flags,reset,reset_positions,timeout_effects)
assert retry[0]==1 and timeout_effects['timer_running']
timed_facts=dict(first_pull_local=first[0],timer_seconds=4,second_pull_local=second[0],
    second_pull_stops_timer=True,locked_crates={str(k):list(locked[k]) for k in (28,31)},
    second_repeat_inert=True,success_from_locked_local=finished[0],unstarted_crossing_not_complete=True,
    timeout_resets_to_zero=True,retry_starts_timer=True)
report=dict(source_sha256={name:hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest() for name,body in scripts.items()},
    maze_cases=rows,chest_patterns_checked=64,successful_pattern_masks=success,
    first_car=timed_facts,
    limitation='Native functions are not invoked; actual frame/audio/input timing remains unmeasured.')
(ROOT/'build_check/dream_train_native_report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('Dream train native facts:24 maze comparisons;64 patterns, sole success28; two-pull timer stop/locked crates/success/timeout/retry verified; no recovered source output')
