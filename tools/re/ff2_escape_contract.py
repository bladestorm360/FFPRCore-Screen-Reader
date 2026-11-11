"""Factual native escape-chain validation; no script bodies in Git/output."""
import csv,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];DUMP=ROOT.parent/'tools/mapdump'

def main():
    maps={int(r['id']):r['map_name'] for r in csv.DictReader((DUMP/'ff2_master/map.txt').open(encoding='utf-8-sig'))}
    scripts={}
    for line in (DUMP/'ff2_scripts.jsonl').open(encoding='utf-8'):
        row=json.loads(line)
        if row['name'].startswith('sc_e_0039'):scripts[row['submap'],row['name']]=row
    map_id=175;name='sc_e_0039';seen=[];facts=[]
    for _ in range(12):
        row=scripts[maps[map_id],name];body=row['mnemonics'];seen.append(map_id)
        facts.append(dict(map=map_id,script=maps[map_id]+'/'+name,
            sha256=hashlib.sha256(json.dumps(body,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()))
        done=any(m['mnemonic']=='SetFlag' and m['operands'].get('sValues',[None])[0]=='ScenarioFlag1' and m['operands']['iValues'][0]==35 for m in body)
        if done:assert map_id==1 and name=='sc_e_0039_8';break
        changes=[m['operands'] for m in body if m['mnemonic']=='ChangeMap'];assert len(changes)==1,(map_id,name)
        change=changes[0];continuations=[s for s in change.get('sValues',[]) if s.startswith('sc_e_0039')];assert len(continuations)==1
        map_id=change['iValues'][0];name=continuations[0]
    else:raise AssertionError('Native escape chain exceeded its finite bound')
    assert seen==[175,174,173,170,172,171,170,164,1],seen
    (ROOT/'build_check/ff2_escape_chain.json').write_text(json.dumps(dict(maps=seen,final_scenario=35,scripts=facts),indent=2)+'\n',encoding='utf-8')
    print('Dreadnought: eight exact automatic transfers from engine175 to world1; only final0039_8 sets completion35.')

if __name__=='__main__':main()
