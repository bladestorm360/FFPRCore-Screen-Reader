"""Independent native choice-table proof; original facts only, no game code."""
from pathlib import Path
import sys,json,itertools,hashlib
ROOT=Path(__file__).resolve().parents[1]
if __name__=='__main__':
    import author_ff6_story_guides as o
    manifest=json.loads((ROOT/'docs/re/zozo_clock_native_20261103.json').read_text(encoding='utf-8'));s=o.SCRIPTS['Map_30511_3','sc_e_0299'];ms=s['mnemonics']
    assert hashlib.sha256(json.dumps(ms,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==manifest['source_script_hashes']['Map_30511_3/sc_e_0299']
    valid=[];counts=[]
    for pc in [i for i,m in enumerate(ms)if m['mnemonic']=='Select']:
        options=[v for v in ms[pc]['operands']['sValues']if v];counts.append(len(options));end=next(i for i in range(pc+1,len(ms))if ms[i]['mnemonic']=='Calc');rejected=[]
        for m in ms[pc+1:end]:
            v=m['operands'];assert m['mnemonic']=='Branch'and v['sValues'][:3]==['MessageSelection','＝','imm']and v['iValues'][0]==0 and v['iValues'][2]==end+1;rejected.append(v['iValues'][1])
        v=ms[end]['operands'];assert v['sValues'][:3]==['MapLocalVariable','＋','imm']and v['iValues'][:2]==[1,1]
        accepted=set(range(len(options)))-set(rejected);assert len(accepted)==1;valid.append(next(iter(accepted)))
    assert ms[1]['operands']['sValues'][:3]==['MapLocalVariable','←','imm']and ms[1]['operands']['iValues'][:2]==[1,0]
    assert ms[27]['operands']['sValues'][:3]==['MapLocalVariable','＝','imm']and ms[27]['operands']['iValues'][:3]==[1,3,30]
    outcomes=[dict(selection=list(choice),correct=sum(a==b for a,b in zip(choice,valid)))for choice in itertools.product(*(range(n)for n in counts))]
    assert counts==[6,5,5]and valid==[2,0,4]and len(outcomes)==150 and [r['selection']for r in outcomes if r['correct']==3]==[[2,0,4]]
    if'--report'in sys.argv:Path(sys.argv[sys.argv.index('--report')+1]).write_text(json.dumps(outcomes,indent=2)+'\n',encoding='utf-8')
    print('Zozo native choices:all150 combinations; only positions3/1/5 reach map-local1=3. Chest115 is a separate native receipt.')
